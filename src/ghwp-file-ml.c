/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ghwp-file-ml.c
 *
 * Copyright (C) 2013 Hodong Kim <cogniti@gmail.com>
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

#include <libxml/xmlreader.h>
#include <string.h>
#include "ghwp-file-ml.h"
#include <math.h>

G_DEFINE_TYPE (GHWPFileML, ghwp_file_ml, GHWP_TYPE_FILE);

GHWPFileML *ghwp_file_ml_new_from_uri (const gchar *uri, GError **error)
{
    g_return_val_if_fail (uri != NULL, NULL);

    GHWPFileML *file = g_object_new (GHWP_TYPE_FILE_ML, NULL);
    file->priv->uri  = g_strdup (uri);

    return file;
}

GHWPFileML *ghwp_file_ml_new_from_filename (const gchar *filename,
                                            GError     **error)
{
    g_return_val_if_fail (filename != NULL, NULL);

    GHWPFileML *file = g_object_new (GHWP_TYPE_FILE_ML, NULL);
    file->priv->uri  = g_filename_to_uri (filename, NULL, error);

    return file;
}

gchar *ghwp_file_ml_get_hwp_version_string (GHWPFile *file)
{
    return NULL;
}

void ghwp_file_ml_get_hwp_version (GHWPFile *file,
                                    guint8   *major_version,
                                    guint8   *minor_version,
                                    guint8   *micro_version,
                                    guint8   *extra_version)
{

}

enum HWPParseStateFlags {
    HWP_PARSE_NORMAL = 0,
    HWP_PARSE_P      = 1 << 0,
    HWP_PARSE_TEXT   = 1 << 1,
    HWP_PARSE_CHAR   = 1 << 2
};

static int   hwp_parse_state = HWP_PARSE_NORMAL;
static guint tag_p_count = 0;

static void _ghwp_file_ml_parse_node(GHWPDocument    *doc,
                                     xmlTextReaderPtr reader)
{
    xmlChar *name, *value;
    int node_type = 0;

    name = xmlTextReaderName(reader);
    value = xmlTextReaderValue(reader);
    node_type = xmlTextReaderNodeType(reader);

    gchar *tag_name = g_utf8_casefold ((const char*) name,
                                       strlen ((const char*) name));
    gchar *tag_p    = g_utf8_casefold ("P", strlen("P"));
    gchar *tag_text = g_utf8_casefold ("TEXT", strlen("TEXT"));
    gchar *tag_char = g_utf8_casefold ("CHAR", strlen("CHAR"));

    switch (node_type) {
        case XML_READER_TYPE_ELEMENT:
            /* paragraph */
            if (g_utf8_collate (tag_name, tag_p) == 0) {
                hwp_parse_state |= HWP_PARSE_P;
                tag_p_count++;
                if (tag_p_count > 1) {
                    GHWPParagraph *paragraph = ghwp_paragraph_new ();
                    GHWPText *ghwp_text = ghwp_text_new ();
                    ghwp_paragraph_set_ghwp_text (paragraph, ghwp_text);
                    g_array_append_val (doc->paragraphs, paragraph);
                }
            /* char */
            } else if (g_utf8_collate (tag_name, tag_char) == 0) {
                hwp_parse_state |= HWP_PARSE_CHAR;
            }
            break;
        case XML_READER_TYPE_TEXT:
            if ((hwp_parse_state & HWP_PARSE_CHAR) == HWP_PARSE_CHAR) {
                GHWPParagraph *paragraph = g_array_index (doc->paragraphs,
                                                          GHWPParagraph *,
                                                          doc->paragraphs->len - 1);
                ghwp_text_append (paragraph->ghwp_text, (const gchar *) value);
            }
            break;
        case XML_READER_TYPE_END_ELEMENT:
            if ((g_utf8_collate (tag_name, tag_p) == 0) && (tag_p_count > 1)) {
                GHWPParagraph *paragraph = g_array_index (doc->paragraphs,
                                                          GHWPParagraph *,
                                                          doc->paragraphs->len - 1);

                /* 높이 계산 */
                static gdouble y   = 0.0;
                static guint   len = 0;
                len = g_utf8_strlen (paragraph->ghwp_text->text, -1);
                y += 18.0 * ceil (len / 33.0);

                if (y > 842.0 - 80.0) {
                    g_array_append_val (doc->pages, GHWP_FILE_ML (doc->file)->page);
                    GHWP_FILE_ML (doc->file)->page = ghwp_page_new ();
                    g_array_append_val (GHWP_FILE_ML (doc->file)->page->paragraphs, paragraph);
                    y = 0.0;
                } else {
                    g_array_append_val (GHWP_FILE_ML (doc->file)->page->paragraphs, paragraph);
                } /* if */
            } else if (g_utf8_collate (tag_name, tag_char) == 0) {
                hwp_parse_state &= ~HWP_PARSE_CHAR;
            }
            break;
        default:
            break;
    }

    g_free (tag_name);
    g_free (tag_p);
    g_free (tag_text);
    g_free (tag_char);

    xmlFree(name);
    xmlFree(value);
}

static void _ghwp_file_ml_parse (GHWPDocument *doc, GError **error)
{
    g_return_if_fail (doc != NULL);

    gchar *uri = GHWP_FILE_ML (doc->file)->priv->uri;
    
    xmlTextReaderPtr reader;
    int              ret;

    reader = xmlNewTextReaderFilename (uri);

    if (reader != NULL) {
        while ((ret = xmlTextReaderRead(reader)) == 1) {
            _ghwp_file_ml_parse_node (doc, reader);
        }
        /* 마지막 페이지 더하기 */
        g_array_append_val (doc->pages, GHWP_FILE_ML (doc->file)->page);
        xmlFreeTextReader(reader);
        if (ret != 0) {
            g_warning ("%s : failed to parse\n", uri);
        }
    } else {
        g_warning ("Unable to open %s\n", uri);
    }
}

GHWPDocument *ghwp_file_ml_get_document (GHWPFile *file, GError **error)
{
    g_return_val_if_fail (GHWP_IS_FILE_ML (file), NULL);
    GHWPDocument *doc = ghwp_document_new();
    doc->file = GHWP_FILE(file);
    _ghwp_file_ml_parse (doc, error);
    return doc;
}

static void ghwp_file_ml_init (GHWPFileML *file)
{
    file->priv = G_TYPE_INSTANCE_GET_PRIVATE (file, GHWP_TYPE_FILE_ML,
                                                    GHWPFileMLPrivate);
    file->page = ghwp_page_new ();
}

static void ghwp_file_ml_finalize (GObject *object)
{
    GHWPFileML *file = GHWP_FILE_ML(object);
    g_free (file->priv->uri);
    G_OBJECT_CLASS (ghwp_file_ml_parent_class)->finalize (object);
}

static void ghwp_file_ml_class_init (GHWPFileMLClass *klass)
{
    GObjectClass  *object_class   = G_OBJECT_CLASS (klass);
    g_type_class_add_private (klass, sizeof (GHWPFileMLPrivate));
    GHWPFileClass *hwp_file_class = GHWP_FILE_CLASS (klass);
    hwp_file_class->get_document  = ghwp_file_ml_get_document;
    hwp_file_class->get_hwp_version_string = ghwp_file_ml_get_hwp_version_string;
    hwp_file_class->get_hwp_version = ghwp_file_ml_get_hwp_version;

    object_class->finalize = ghwp_file_ml_finalize;
}
