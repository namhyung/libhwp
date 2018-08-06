/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ghwp-file-v5.c
 *
 * Copyright (C) 2012-2013 Hodong Kim <cogniti@gmail.com>
 * 
 * This library is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * This software have been developed with reference to
 * the HWP file format open specification by Hancom, Inc.
 * http://www.hancom.co.kr/userofficedata.userofficedataList.do?menuFlag=3
 * 한글과컴퓨터의 한/글 문서 파일(.hwp) 공개 문서를 참고하여 개발하였습니다.
 */

#include <glib.h>
#include <glib-object.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <gsf/gsf-input-impl.h>
#include <gsf/gsf-input-memory.h>
#include <gsf/gsf-msole-utils.h>
#include <gsf/gsf-input-stdio.h>
#include <gsf/gsf-infile-impl.h>
#include <gsf/gsf-doc-meta-data.h>
#include <gsf/gsf-meta-names.h>
#include <gsf/gsf-timestamp.h>

#include "gsf-input-stream.h"
#include "ghwp-document.h"
#include "ghwp-file-v5.h"
#include "ghwp-parse.h"
#include "config.h"

G_DEFINE_TYPE (GHWPFileV5, ghwp_file_v5, GHWP_TYPE_FILE);

#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))
#define _g_array_free0(var) ((var == NULL) ? NULL : (var = (g_array_free (var, TRUE), NULL)))
#define _g_error_free0(var) ((var == NULL) ? NULL : (var = (g_error_free (var), NULL)))
#define _g_free0(var) (var = (g_free (var), NULL))

static gpointer _g_object_ref0 (gpointer obj)
{
    return obj ? g_object_ref (obj) : NULL;
}

static void _ghwp_file_v5_parse_doc_info (GHWPDocument *doc, GError **error)
{
    g_return_if_fail (doc != NULL);

    GHWPFileV5   *file    = GHWP_FILE_V5(doc->file);
    GInputStream *stream  = file->doc_info_stream;
    GHWPContext  *context = ghwp_context_new (stream);
    gint          n_bindata = 0;
    gint          n_fonts = 0;
    gint          n_char_shapes = 0;
    gint          n_para_shapes = 0;

    context->version[0] = file->major_version;
    context->version[1] = file->minor_version;
    context->version[2] = file->micro_version;
    context->version[3] = file->extra_version;

    while (ghwp_context_pull (context, error)) {
        switch (context->tag_id) {
        case GHWP_TAG_DOCUMENT_PROPERTIES:
            ghwp_parse_document_property (doc, context);
            break;
        case GHWP_TAG_ID_MAPPINGS:
            ghwp_parse_document_id_mapping (doc, context);
            break;
        case GHWP_TAG_BIN_DATA:
            ghwp_parse_document_bin_data (doc, context, n_bindata++);
            break;
        case GHWP_TAG_FACE_NAME:
            ghwp_parse_document_font_face (doc, context, n_fonts++);
            break;
        case GHWP_TAG_CHAR_SHAPE:
            ghwp_parse_document_char_shape (doc, context, n_char_shapes++);
            break;
        case GHWP_TAG_PARA_SHAPE:
            ghwp_parse_document_para_shape (doc, context, n_para_shapes++);
            break;
        default:
            dbg("%s:%d: %s not implemented\n", __FILE__, __LINE__,
                _ghwp_get_tag_name (context->tag_id));
            break;
        }
    }

    g_object_unref (context);
}

/* enum의 최대값은 ?? */
typedef enum
{
    CTRL_ID_TABLE		= GUINT32_FROM_LE(MAKE_CTRL_ID('t', 'b', 'l', ' ')),
    CTRL_ID_SEC_DEF		= GUINT32_FROM_LE(MAKE_CTRL_ID('s', 'e', 'c', 'd')),
    CTRL_ID_COL_DEF		= GUINT32_FROM_LE(MAKE_CTRL_ID('c', 'o', 'l', 'd')),
} CtrlID;

/* TODO fsm parser, nautilus에서 파일 속성만 보는 경우가 있으므로 속도 문제
 * 때문에 get_n_pages 로 옮겨갈 필요가 있다. */
static void _ghwp_file_v5_parse_body_text (GHWPDocument *doc, GError **error)
{
    g_return_if_fail (doc != NULL);
    guint32 ctrl_id = 0;
    guint   index;
    GHWPFileV5    *file = GHWP_FILE_V5(doc->file);
    GHWPPage      *page = NULL;
    GHWPSection   *section;
    GHWPParagraph *paragraph = NULL;

    for (index = 0; index < file->section_streams->len; index++) {
        GInputStream  *section_stream;
        GHWPContext   *context;
        GHWPTable     *table = NULL;
        GHWPTableCell *cell = NULL;
        GHWPListHeader lhdr;

        section_stream = g_array_index (file->section_streams,
                                        GInputStream *,
                                        index);
        section_stream = _g_object_ref0 (section_stream);

        context = ghwp_context_new (section_stream);
        context->version[0] = file->major_version;
        context->version[1] = file->minor_version;
        context->version[2] = file->micro_version;
        context->version[3] = file->extra_version;

        section = ghwp_section_new ();
        section->document = doc;
        g_array_append_val (doc->sections, section);

        while (ghwp_context_pull(context, error)) {
            GHWPContextStatus *curr_status = &context->status[context->level];

            dbg ("%*stag = %u [%s] (size: %u)\n", context->level*3, "",
                 context->tag_id, _ghwp_get_tag_name (context->tag_id),
                 context->data_len);

            switch (context->tag_id) {
            case GHWP_TAG_PARA_HEADER:
                paragraph = ghwp_paragraph_new ();
                ghwp_parse_paragraph_header (paragraph, context);

                if (context->level == 0) {
                    ghwp_section_add_paragraph (section, paragraph);
                } else if (curr_status->s == STATE_TABLE) {
                    cell  = curr_status->p;
                    ghwp_table_cell_add_paragraph (cell, paragraph);
                }

                /* do not update current state, but next state is set */
                context->status[context->level + 1].p = paragraph;
                context->status[context->level + 1].s = STATE_PARAGRAPH;
                break;

            case GHWP_TAG_PARA_TEXT:
            case GHWP_TAG_PARA_CHAR_SHAPE:
            case GHWP_TAG_PARA_LINE_SEG:
            case GHWP_TAG_PARA_RANGE_TAG:
                if (curr_status->s != STATE_PARAGRAPH)
                    g_warning ("invalid paragraph data");

                paragraph = curr_status->p;

                if (context->tag_id == GHWP_TAG_PARA_TEXT)
                    ghwp_parse_paragraph_text (paragraph, context);
                else if (context->tag_id == GHWP_TAG_PARA_CHAR_SHAPE)
                    ghwp_parse_paragraph_char_shape (paragraph, context);
                else if (context->tag_id == GHWP_TAG_PARA_LINE_SEG)
                    ghwp_parse_paragraph_line_seg (paragraph, context);
                else if (context->tag_id == GHWP_TAG_PARA_RANGE_TAG)
                    ghwp_parse_paragraph_range_tag (paragraph, context);

                break;

            case GHWP_TAG_CTRL_HEADER:
                context_read_uint32 (context, &ctrl_id);

                dbg ("%*s ctrl: "CTRL_ID_FMT"\n", context->level * 3, "",
                     CTRL_ID_PRINT (ctrl_id));

                switch (ctrl_id) {
                case CTRL_ID_TABLE:
                    table = ghwp_table_new ();
                    table->obj.ctrl_id = ctrl_id;
                    ghwp_parse_common_object (&table->obj, context);

                    paragraph = curr_status->p;
                    ghwp_paragraph_set_table (paragraph, table);

                    curr_status->s = STATE_CTRL_TABLE;
                    curr_status->p = table;
                    break;
                case CTRL_ID_SEC_DEF:
                    ghwp_parse_section_def (section, context);
                    break;
                case CTRL_ID_COL_DEF:
                    ghwp_parse_column_def (section, context);
                    break;
                default:
                    curr_status->s = STATE_NORMAL;
                    break;
                }
                break;

            case GHWP_TAG_TABLE:
                table = context->status[context->level - 1].p;
                ghwp_parse_table_attr (table, context);

                curr_status->s = STATE_TABLE;
                break;

            case GHWP_TAG_LIST_HEADER:
                ghwp_parse_list_header (&lhdr, context);
                /* TODO ctrl_id 에 따른 객체를 생성한다 */
                switch (curr_status->s) {
                /* table에 cell을 추가한다 */
                case STATE_CTRL_TABLE:
                    /* caption */
                    break;
                case STATE_TABLE:
                    table = context->status[context->level - 1].p;
                    cell  = ghwp_table_cell_new ();
                    ghwp_parse_table_cell_attr (cell, context);
                    memcpy (&cell->header, &lhdr, sizeof (lhdr));
                    /* FIXME 테이블 내에서 페이지가 나누어지는 경우 처리 */
                    ghwp_table_add_cell (table, cell);
                    curr_status->p = cell;
                    break;
                default:
                    break;
                }
                break;

            case GHWP_TAG_PAGE_DEF:
                ghwp_parse_page_def (section, context);
                break;

            default:
                break;
            } /* switch */
        } /* while */

        _g_object_unref0 (context);
        _g_object_unref0 (section_stream);
    } /* for */

    /* create pages */
    for (index = 0; index < doc->sections->len; index++) {
        section = g_array_index (doc->sections, GHWPSection *, index);
        page    = NULL;

        gint i, n;
        for (i = 0; i < section->paragraphs->len; i++) {
            paragraph = g_array_index (section->paragraphs, GHWPParagraph *, i);

            for (n = 0; n < paragraph->header.n_line_segs; n++) {
                GHWPLineSeg *line;

                line = g_array_index (paragraph->line_segs, GHWPLineSeg *, n);
                if (line->v_pos != 0)
                    continue;

                if (n != 0) {  /* 문단 내에서 페이지가 바뀌는 경우 */
                    if (page == NULL)
                        g_warning("invalid line seg?");

                    GHWPParagraph *link_paragraph = ghwp_paragraph_new ();
                    ghwp_paragraph_add_link (paragraph, link_paragraph, n);

                    ghwp_page_add_paragraph (page, paragraph);
                    paragraph = link_paragraph;
                }

                page = ghwp_page_new ();
                ghwp_page_set_section (page, section);

                g_array_append_val (doc->pages, page);
            }
            ghwp_page_add_paragraph (page, paragraph);
        }
    }
}

static void _ghwp_file_v5_parse_prv_text (GHWPDocument *doc)
{
    g_return_if_fail (doc != NULL);

    GsfInputStream *gis   = _g_object_ref0 (GHWP_FILE_V5(doc->file)->prv_text_stream);
    gssize          size  = gsf_input_stream_size (gis);
    guchar         *buf   = g_new (guchar, size);
    GError         *error = NULL;

    g_input_stream_read ((GInputStream*) gis, buf, size, NULL, &error);

    if (error != NULL) {
        g_warning("%s:%d: %s\n", __FILE__, __LINE__, error->message);
        _g_free0 (doc->prv_text);
        g_clear_error (&error);
        buf = (g_free (buf), NULL);
        _g_object_unref0 (gis);
        return;
    }

    /* g_convert() can be used to convert a byte buffer of UTF-16 data of
       ambiguous endianess. */
    doc->prv_text = g_convert ((const gchar*) buf, (gssize) size,
                               "UTF-8", "UTF-16LE", NULL, NULL, &error);

    if (error != NULL) {
        g_warning("%s:%d: %s\n", __FILE__, __LINE__, error->message);
        _g_free0 (doc->prv_text);
        g_clear_error (&error);
        buf = (g_free (buf), NULL);
        _g_object_unref0 (gis);
        return;
    }

    buf = (g_free (buf), NULL);
    _g_object_unref0 (gis);
}

/* 알려지지 않은 것을 감지하기 위해 이렇게 작성함 */
static void
_ghwp_metadata_hash_func (gpointer k, gpointer v, gpointer user_data)
{
    gchar        *name  = (gchar        *) k;
    GsfDocProp   *prop  = (GsfDocProp   *) v;
    GHWPDocument *doc   = (GHWPDocument *) user_data;
    GValue const *value = gsf_doc_prop_get_val (prop);

    if ( g_str_equal(name, GSF_META_NAME_CREATOR) ) {

        doc->creator = g_value_get_string (value);

    } else if ( g_str_equal(name, GSF_META_NAME_DATE_MODIFIED) ) {
        GsfTimestamp *ts    = g_value_get_boxed (value);
        doc->mod_date = (GTime) ts->timet;

    } else if ( g_str_equal(name, GSF_META_NAME_DESCRIPTION) ) {
        doc->desc = g_value_get_string (value);

    } else if ( g_str_equal(name, GSF_META_NAME_KEYWORDS) ) {
        doc->keywords = g_value_get_string (value);

    } else if ( g_str_equal(name, GSF_META_NAME_SUBJECT) ) {
        doc->subject = g_value_get_string (value);

    } else if ( g_str_equal(name, GSF_META_NAME_TITLE) ) {
        doc->title = g_value_get_string (value);

    } else if ( g_str_equal(name, GSF_META_NAME_LAST_PRINTED) ) {
        GsfTimestamp *ts    = g_value_get_boxed (value);
        doc->last_printed   = (GTime) ts->timet;

    } else if ( g_str_equal(name, GSF_META_NAME_LAST_SAVED_BY) ) {
        doc->last_saved_by = g_value_get_string (value);

    } else if ( g_str_equal(name, GSF_META_NAME_DATE_CREATED) ) {
        GsfTimestamp *ts    = g_value_get_boxed (value);
        doc->creation_date  = (GTime) ts->timet;

    } else if ( g_str_equal(name, GSF_META_NAME_REVISION_COUNT) ) {
        doc->revision_count = g_value_get_string (value);

    } else if ( g_str_equal(name, GSF_META_NAME_PAGE_COUNT) ) {
        /* not correct n_pages == 0 ?? */
        doc->n_pages = g_value_get_int (value);

    } else {
        g_warning("%s:%d:%s not implemented\n", __FILE__, __LINE__, name);
    }
}

/*
typedef enum {
    INFO_ID_CREATOR,
    INFO_ID_DATE_MODIFIED,
    INFO_ID_DESCRIPTION,
    INFO_ID_KEYWORDS,
    INFO_ID_SUBJECT,
    INFO_ID_TITLE,
    INFO_ID_LAST_PRINTED,
    INFO_ID_LAST_SAVED_BY,
    INFO_ID_DATE_CREATED,
    INFO_ID_REVISION_COUNT,
    INFO_ID_PAGE_COUNT,
} InfoID;

typedef struct {
    InfoID id;
    const  gchar *name;
} SummaryInfo;

SummaryInfo info[]   = {
        { INFO_ID_CREATOR,        GSF_META_NAME_CREATOR       },
        { INFO_ID_DATE_MODIFIED,  GSF_META_NAME_DATE_MODIFIED },
        { INFO_ID_DESCRIPTION,    GSF_META_NAME_DESCRIPTION   },
        { INFO_ID_KEYWORDS,       GSF_META_NAME_KEYWORDS      },
        { INFO_ID_SUBJECT,        GSF_META_NAME_SUBJECT       },
        { INFO_ID_TITLE,          GSF_META_NAME_TITLE         },
        { INFO_ID_LAST_PRINTED,   GSF_META_NAME_LAST_PRINTED  },
        { INFO_ID_LAST_SAVED_BY,  GSF_META_NAME_LAST_SAVED_BY },
        { INFO_ID_DATE_CREATED,   GSF_META_NAME_DATE_CREATED  },
        { INFO_ID_REVISION_COUNT, GSF_META_NAME_REVISION_COUNT},
        { INFO_ID_PAGE_COUNT,     GSF_META_NAME_PAGE_COUNT    }
};
*/

static void _ghwp_file_v5_parse_summary_info (GHWPDocument *doc)
{
    g_return_if_fail (doc != NULL);

    GsfInputStream *gis;
    gssize          size;
    guint8         *buf = NULL;
    GsfInputMemory *summary;
    GsfDocMetaData *meta;
    GError         *error = NULL;

    gis  = _g_object_ref0 (GHWP_FILE_V5(doc->file)->summary_info_stream);
    size = gsf_input_stream_size (gis);
    buf  = g_malloc(size);

    g_input_stream_read ((GInputStream*) gis, buf, (gsize) size, NULL, &error);

    if (error != NULL) {
        buf = (g_free (buf), NULL);
        _g_object_unref0 (gis);
        g_warning("%s:%d: %s\n", __FILE__, __LINE__, error->message);
        g_clear_error (&error);
        return;
    }

    /* changwoo's solution, thanks to changwoo.
     * https://groups.google.com/forum/#!topic/libhwp/gFDD7UMCXBc
     * https://github.com/changwoo/gnome-hwp-support/blob/master/properties/props-data.c
     * Trick the libgsf's MSOLE property set parser, by changing
     * its GUID. The \005HwpSummaryInformation is compatible with
     * the summary property set.
     */
    guint8 component_guid [] = {
        0xe0, 0x85, 0x9f, 0xf2, 0xf9, 0x4f, 0x68, 0x10,
        0xab, 0x91, 0x08, 0x00, 0x2b, 0x27, 0xb3, 0xd9
    };

    if (size >= sizeof(component_guid) + 28) {
        memcpy (buf + 28, component_guid, (gsize) sizeof(component_guid));
    } else {
        buf = (g_free (buf), NULL);
        _g_object_unref0 (GHWP_FILE_V5(doc->file)->summary_info_stream);
        _g_object_unref0 (gis);
        g_warning("%s:%d: file corrupted\n", __FILE__, __LINE__);
        return;
    }
    summary = (GsfInputMemory*) gsf_input_memory_new (buf, size, FALSE);

    meta = gsf_doc_meta_data_new ();

#ifdef HAVE_GSF_DOC_META_DATA_READ_FROM_MSOLE
    /* since libgsf 1.14.24 */
    gsf_doc_meta_data_read_from_msole (meta, (GsfInput*) summary);
#else
    /* NOTE gsf_msole_metadata_read: deprecated since libgsf 1.14.24 */
    gsf_msole_metadata_read ((GsfInput*) summary, meta);
#endif

    gsf_doc_meta_data_foreach (meta, _ghwp_metadata_hash_func, doc);

    _g_object_unref0 (doc->summary_info);
    doc->summary_info = _g_object_ref0 (meta);
    _g_object_unref0 (meta);
    _g_object_unref0 (summary);
    buf = (g_free (buf), NULL);
    _g_object_unref0 (gis);
}

static void _ghwp_file_v5_parse (GHWPDocument *doc, GError **error)
{
    g_return_if_fail (doc != NULL);

    _ghwp_file_v5_parse_doc_info (doc, error);
    if (*error) return;
    _ghwp_file_v5_parse_body_text (doc, error);
    if (*error) return;
    _ghwp_file_v5_parse_prv_text (doc);
    _ghwp_file_v5_parse_summary_info (doc);
}

GHWPDocument *ghwp_file_v5_get_document (GHWPFile *file, GError **error)
{
    g_return_val_if_fail (GHWP_IS_FILE_V5 (file), NULL);
    GHWPDocument *doc = ghwp_document_new();
    doc->file = GHWP_FILE(file);
    _ghwp_file_v5_parse (doc, error);
    return doc;
}

void
ghwp_file_v5_get_hwp_version (GHWPFile *file,
                              guint8   *major_version,
                              guint8   *minor_version,
                              guint8   *micro_version,
                              guint8   *extra_version)
{
    g_return_if_fail (GHWP_IS_FILE_V5 (file));

    if (major_version) *major_version = GHWP_FILE_V5(file)->major_version;
    if (minor_version) *minor_version = GHWP_FILE_V5(file)->minor_version;
    if (micro_version) *micro_version = GHWP_FILE_V5(file)->micro_version;
    if (extra_version) *extra_version = GHWP_FILE_V5(file)->extra_version;
}

gchar *ghwp_file_v5_get_hwp_version_string (GHWPFile *file)
{
    g_return_val_if_fail (GHWP_IS_FILE_V5 (file), NULL);

    return g_strdup_printf ("%d.%d.%d.%d", GHWP_FILE_V5(file)->major_version,
                                           GHWP_FILE_V5(file)->minor_version,
                                           GHWP_FILE_V5(file)->micro_version,
                                           GHWP_FILE_V5(file)->extra_version);
}

GHWPFileV5* ghwp_file_v5_new_from_uri (const gchar* uri, GError** error)
{
    g_return_val_if_fail (uri != NULL, NULL);

    gchar      *filename = g_filename_from_uri (uri, NULL, error);
    GHWPFileV5 *file     = ghwp_file_v5_new_from_filename (filename, error);
    _g_free0 (filename);
    return file;
}

/* TODO 에러 감지/전파 코드 있어야 한다. */
static void ghwp_file_v5_decode_file_header (GHWPFileV5 *file)
{
    g_return_if_fail (file != NULL);

    GsfInputStream *gis;
    gssize          size;
    gsize           bytes_read;
    guint8         *buf;
    guint32         prop = 0;

    gis  = (GsfInputStream *) g_object_ref (file->file_header_stream);
    size = gsf_input_stream_size (gis);
    buf  = g_malloc (size);

    g_input_stream_read_all ((GInputStream*) gis, buf, (gsize) size,
                             &bytes_read, NULL, NULL);
    g_object_unref (gis);

    if (bytes_read >= 40) {
        file->signature = g_strndup ((const gchar *)buf, 32); /* null로 끝남 */
        file->major_version = buf[35];
        file->minor_version = buf[34];
        file->micro_version = buf[33];
        file->extra_version = buf[32];

        memcpy (&prop, buf + 36, 4);
        prop = GUINT32_FROM_LE(prop);

        if (prop & (1 <<  0)) file->is_compress            = TRUE;
        if (prop & (1 <<  1)) file->is_encrypt             = TRUE;
        if (prop & (1 <<  2)) file->is_distribute          = TRUE;
        if (prop & (1 <<  3)) file->is_script              = TRUE;
        if (prop & (1 <<  4)) file->is_drm                 = TRUE;
        if (prop & (1 <<  5)) file->is_xml_template        = TRUE;
        if (prop & (1 <<  6)) file->is_history             = TRUE;
        if (prop & (1 <<  7)) file->is_sign                = TRUE;
        if (prop & (1 <<  8)) file->is_certificate_encrypt = TRUE;
        if (prop & (1 <<  9)) file->is_sign_spare          = TRUE;
        if (prop & (1 << 10)) file->is_certificate_drm     = TRUE;
        if (prop & (1 << 11)) file->is_ccl                 = TRUE;
    }

    buf = (g_free (buf), NULL);
}


static int get_order (char *a)
{
    if (g_str_equal (a, "FileHeader"))
        return 0;
    if (g_str_equal (a, "DocInfo"))
        return 1;
    if (g_str_equal (a, "BodyText"))
        return 2;
    if (g_str_equal (a, "ViewText"))
        return 3;
    if (g_str_equal (a, "\005HwpSummaryInformation"))
        return 4;
    if (g_str_equal (a, "BinData"))
        return 5;
    if (g_str_equal (a, "PrvText"))
        return 6;
    if (g_str_equal (a, "PrvImage"))
        return 7;
    if (g_str_equal (a, "DocOptions"))
        return 8;
    if (g_str_equal (a, "Scripts"))
        return 9;
    if (g_str_equal (a, "XMLTemplate"))
        return 10;
    if (g_str_equal (a, "DocHistory"))
        return 11;

    return 100;
}

static gint compare_entries (gconstpointer a, gconstpointer b)
{
    int i, j;
    i = get_order (*(char **)a);
    j = get_order (*(char **)b);
    return i - j;
}

static GInputStream *_ghwp_make_stream_single (GHWPFileV5 *file, char *name,
                                               gboolean compress)
{
    GsfInput *input   = gsf_infile_child_by_name ((GsfInfile*) file->priv->olefile,
                                                  name);
    GInputStream *gis;
    gint num_children;

    input = _g_object_ref0 (input);
    num_children = gsf_infile_num_children ((GsfInfile*) input);

    if (num_children > 0) {
        g_warning ("invalid input stream");
        _g_object_unref0 (input);
        return NULL;
    }

    gis = G_INPUT_STREAM (gsf_input_stream_new (input));

    if (compress && file->is_compress) {
        GZlibDecompressor *zd  = g_zlib_decompressor_new (G_ZLIB_COMPRESSOR_FORMAT_RAW);
        GInputStream      *cis = g_converter_input_stream_new (gis, (GConverter*) zd);

        _g_object_unref0 (zd);
        _g_object_unref0 (gis);

        gis = cis;
    }

    _g_object_unref0 (input);
    return gis;
}

static GArray *_ghwp_make_stream_array (GHWPFileV5 *file, char *name,
                                        gboolean compress)
{
    GsfInfile *olefile = (GsfInfile*) file->priv->olefile;
    GsfInfile *infile = (GsfInfile*) gsf_infile_child_by_name (olefile, name);
    gint num_children;

    infile = _g_object_ref0 (infile);
    num_children = gsf_infile_num_children (infile);

    if (num_children == 0) {
        g_warning ("nothing in %s", name);
        _g_object_unref0 (infile);
        return NULL;
    }

    GArray *stream_array = g_array_new (TRUE, TRUE, sizeof (GInputStream*));
    gint j;
    for (j = 0; j < num_children; j++) {
        GsfInput  *input = gsf_infile_child_by_index (infile, j);
        GsfInfile *child = _g_object_ref0 (input);
        gint num_grand_children = gsf_infile_num_children (child);
        GInputStream *gis;

        if (num_grand_children > 0) {
            fprintf (stderr, "invalid child stream\n");
            _g_object_unref0 (child);
            continue;
        }

        gis = G_INPUT_STREAM (gsf_input_stream_new (input));

        if (compress && file->is_compress) {
            GZlibDecompressor *zd  = g_zlib_decompressor_new (G_ZLIB_COMPRESSOR_FORMAT_RAW);
            GInputStream      *cis = g_converter_input_stream_new (gis, (GConverter*) zd);

            _g_object_unref0 (zd);
            _g_object_unref0 (gis);

            gis = cis;
        }

        gis = _g_object_ref0 (gis);
        g_array_append_val (stream_array, gis);
        _g_object_unref0 (child);
    } /* for */
    _g_object_unref0 (infile);

    return stream_array;
}

/* FIXME streams 배열과 enum을 이용하여 코드 재적성 바람 */
static void _ghwp_file_v5_make_stream (GHWPFileV5 *file)
{
    g_return_if_fail (file != NULL);

    const gchar *name = NULL;
    gint  n_children;
    n_children = gsf_infile_num_children ((GsfInfile*) file->priv->olefile);

    if (n_children < 1) {
        fprintf (stderr, "invalid hwp file\n");
        return;
    }

    /* 스펙이 명확하지 않고, 추후 예고없이 스펙이 변할 수 있기 때문에
     * 이를 감지하고자 코드를 이렇게 작성하였다. */
    GArray *entries = g_array_new (TRUE, TRUE, sizeof(char *));
    gint i;
    for (i = 0; i < n_children; i++) {
        name = gsf_infile_name_by_index ((GsfInfile*) file->priv->olefile, i);
        g_array_append_val (entries, name);
    }
    g_array_sort(entries, compare_entries);

    for (i = 0; i < n_children; i++) {
        char     *entry = g_array_index (entries, char *, i);

        if (g_str_equal (entry, "FileHeader")) {
            _g_object_unref0 (file->file_header_stream);
            file->file_header_stream = _ghwp_make_stream_single (file, entry, FALSE);
            ghwp_file_v5_decode_file_header (file);
        } else if (g_str_equal (entry, "DocInfo")) {
            _g_object_unref0 (file->doc_info_stream);
            file->doc_info_stream = _ghwp_make_stream_single (file, entry, TRUE);
        } else if (g_str_equal(entry, "BodyText") ||
                   g_str_equal(entry, "ViewText")) {
            _g_array_free0 (file->section_streams);
            file->section_streams = _ghwp_make_stream_array (file, entry, TRUE);
        } else if (g_str_equal (entry, "\005HwpSummaryInformation")) {
            _g_object_unref0 (file->summary_info_stream);
            file->summary_info_stream = _ghwp_make_stream_single (file, entry, FALSE);
        } else if (g_str_equal(entry, "BinData")) {
            _g_array_free0 (file->bindata_streams);
            file->bindata_streams = _ghwp_make_stream_array (file, entry, TRUE);
        } else if (g_str_equal (entry, "PrvText")) {
            _g_object_unref0 (file->prv_text_stream);
            file->prv_text_stream = _ghwp_make_stream_single (file, entry, FALSE);
        } else if (g_str_equal (entry, "PrvImage")) {
            _g_object_unref0 (file->prv_image_stream);
            file->prv_image_stream = _ghwp_make_stream_single (file, entry, FALSE);
        } else {
            g_warning("%s:%d: %s not implemented\n", __FILE__, __LINE__, entry);
        } /* if */
    } /* for */
    g_array_free (entries, TRUE);
    g_array_unref (entries);
}

GHWPFileV5* ghwp_file_v5_new_from_filename (const gchar* filename, GError** error)
{
    g_return_val_if_fail (filename != NULL, NULL);
    GFile *gfile = g_file_new_for_path (filename);

    GsfInputStdio* input;
    GsfInfileMSOle* olefile;

    gchar *path = g_file_get_path(gfile);
    _g_object_unref0 (gfile);
    input = (GsfInputStdio*) gsf_input_stdio_new (path, error);
    _g_free0 (path);

    if (input == NULL) {
        g_warning("%s:%d: %s\n", __FILE__, __LINE__, (*error)->message);
        return NULL;
    }

    olefile = (GsfInfileMSOle*) gsf_infile_msole_new ((GsfInput*) input,
                                                      error);

    if (olefile == NULL) {
        g_warning("%s:%d: %s\n", __FILE__, __LINE__, (*error)->message);
        _g_object_unref0 (input);
        return NULL;
    }

    GHWPFileV5 *file = g_object_new (GHWP_TYPE_FILE_V5, NULL);
    file->priv->olefile = olefile;
    _g_object_unref0 (input);
    _ghwp_file_v5_make_stream (file);

    return file;
}

static void ghwp_file_v5_finalize (GObject* obj)
{
    GHWPFileV5 *file = GHWP_FILE_V5(obj);
    _g_object_unref0 (file->priv->olefile);
    _g_object_unref0 (file->prv_text_stream);
    _g_object_unref0 (file->prv_image_stream);
    _g_object_unref0 (file->file_header_stream);
    _g_object_unref0 (file->doc_info_stream);
    _g_array_free0 (file->section_streams);
    _g_object_unref0 (file->priv->section_stream);
    _g_object_unref0 (file->summary_info_stream);
    g_free (file->signature);
    G_OBJECT_CLASS (ghwp_file_v5_parent_class)->finalize (obj);
}

static void ghwp_file_v5_class_init (GHWPFileV5Class * klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    g_type_class_add_private (klass, sizeof (GHWPFileV5Private));
    GHWP_FILE_CLASS (klass)->get_document = ghwp_file_v5_get_document;
    GHWP_FILE_CLASS (klass)->get_hwp_version_string = ghwp_file_v5_get_hwp_version_string;
    GHWP_FILE_CLASS (klass)->get_hwp_version = ghwp_file_v5_get_hwp_version;
    object_class->finalize = ghwp_file_v5_finalize;
}

static void ghwp_file_v5_init (GHWPFileV5 *file)
{
    file->priv = G_TYPE_INSTANCE_GET_PRIVATE (file, GHWP_TYPE_FILE_V5,
                                                    GHWPFileV5Private);
}
