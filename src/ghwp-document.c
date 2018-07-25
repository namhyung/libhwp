/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ghwp-document.c
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

#include <string.h>

#include "config.h"
#include "ghwp-document.h"
#include "ghwp-parse.h"

G_DEFINE_TYPE (GHWPDocument, ghwp_document, G_TYPE_OBJECT);

/* private function */
static void   ghwp_document_finalize               (GObject      *obj);

#define _g_array_free0(var) ((var == NULL) ? NULL : (var = (g_array_free (var, TRUE), NULL)))
#define _g_free0(var) (var = (g_free (var), NULL))
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))
#define _g_error_free0(var) ((var == NULL) ? NULL : (var = (g_error_free (var), NULL)))

static gpointer _g_object_ref0 (gpointer obj)
{
    return obj ? g_object_ref (obj) : NULL;
}

/**
 * ghwp_document_new_from_uri:
 * @uri: uri of the file to load
 * @error: (allow-none): Return location for an error, or %NULL
 * 
 * Creates a new #GHWPDocument.  If %NULL is returned, then @error will be
 * set. Possible errors include those in the #GHWP_ERROR and #G_FILE_ERROR
 * domains.
 * 
 * Return value: A newly created #GHWPDocument, or %NULL
 **/
GHWPDocument *ghwp_document_new_from_uri (const gchar *uri, GError **error)
{
    g_return_val_if_fail (uri != NULL, NULL);

    gchar        *filename = g_filename_from_uri (uri, NULL, error);
    GHWPDocument *document = ghwp_document_new_from_filename (filename, error);
    _g_free0 (filename);
    return document;
}

GHWPDocument *
ghwp_document_new_from_filename (const gchar *filename, GError **error)
{
    g_return_val_if_fail (filename != NULL, NULL);

    GHWPFile *file = ghwp_file_new_from_filename (filename, error);

    if (file == NULL) {
        return NULL;
    }

    if (*error) return NULL;

    return ghwp_file_get_document (file, error);
}

guint ghwp_document_get_n_pages (GHWPDocument *doc)
{
    g_return_val_if_fail (doc != NULL, 0U);
    return doc->pages->len;
}

/**
 * ghwp_document_get_page:
 * @doc: a #GHWPDocument
 * @n_page: the index of the page to get
 *
 * Returns a #GHWPPage representing the page at index
 *
 * Returns: (transfer none): a #GHWPPage
 *     DO NOT FREE the page.
 */
GHWPPage *ghwp_document_get_page (GHWPDocument *doc, gint n_page)
{
    g_return_val_if_fail (doc != NULL, NULL);
    GHWPPage *page = g_array_index (doc->pages, GHWPPage *, (guint) n_page);
    return _g_object_ref0 (page);
}

/**
 * ghwp_document_new:
 * 
 * Creates a new #GHWPDocument.
 * 
 * Return value: A newly created #GHWPDocument
 **/
GHWPDocument *ghwp_document_new (void)
{
    return (GHWPDocument*) g_object_new (GHWP_TYPE_DOCUMENT, NULL);
}

static void ghwp_document_class_init (GHWPDocumentClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    object_class->finalize     = ghwp_document_finalize;
}

static void ghwp_document_init (GHWPDocument *doc)
{
    doc->paragraphs = g_array_new (TRUE, TRUE, sizeof (GHWPParagraph *));
    doc->pages      = g_array_new (TRUE, TRUE, sizeof (GHWPPage *));
    doc->sections   = g_array_new (TRUE, TRUE, sizeof (GHWPSection *));
}

static void ghwp_document_finalize (GObject *obj)
{
    GHWPDocument *doc = GHWP_DOCUMENT(obj);
    _g_object_unref0 (doc->file);
    _g_free0 (doc->prv_text);
    _g_array_free0 (doc->paragraphs);
    _g_array_free0 (doc->pages);
    _g_array_free0 (doc->sections);
    _g_object_unref0 (doc->summary_info);
    G_OBJECT_CLASS (ghwp_document_parent_class)->finalize (obj);
}

/**
 * ghwp_document_get_title:
 * @document: A #GHWPDocument
 *
 * Returns the document's title
 *
 * Return value: a new allocated string containing the title
 *               of @document, or %NULL
 *
 * Since: 0.1.2
 **/
gchar *
ghwp_document_get_title (GHWPDocument *document)
{
    g_return_val_if_fail (GHWP_IS_DOCUMENT (document), NULL);
    return g_strdup (document->title);
}

/**
 * ghwp_document_get_keywords:
 * @document: A #GHWPDocument
 *
 * Returns the keywords associated to the document
 *
 * Return value: a new allocated string containing keywords associated
 *               to @document, or %NULL
 *
 * Since: 0.1.2
 **/
gchar *
ghwp_document_get_keywords (GHWPDocument *document)
{
    g_return_val_if_fail (GHWP_IS_DOCUMENT (document), NULL);
    return g_strdup (document->keywords);
}

/**
 * ghwp_document_get_subject:
 * @document: A #GHWPDocument
 *
 * Returns the subject of the document
 *
 * Return value: a new allocated string containing the subject
 *               of @document, or %NULL
 *
 * Since: 0.1.2
 **/
gchar *
ghwp_document_get_subject (GHWPDocument *document)
{
    g_return_val_if_fail (GHWP_IS_DOCUMENT (document), NULL);
    return g_strdup (document->subject);
}

/**
 * ghwp_document_get_creator:
 * @document: A #GHWPDocument
 *
 * Returns the creator of the document.
 *
 * Return value: a new allocated string containing the creator
 *               of @document, or %NULL
 *
 * Since: 0.1.2
 **/
gchar *
ghwp_document_get_creator (GHWPDocument *document)
{
    g_return_val_if_fail (GHWP_IS_DOCUMENT (document), NULL);
    return g_strdup (document->creator);
}

/**
 * ghwp_document_get_creation_date:
 * @document: A #GHWPDocument
 *
 * Returns the date the document was created as seconds since the Epoch
 *
 * Return value: the date the document was created, or -1
 *
 * Since: 0.1.2
 **/
GTime
ghwp_document_get_creation_date (GHWPDocument *document)
{
    g_return_val_if_fail (GHWP_IS_DOCUMENT (document), (GTime)-1);
    return document->creation_date;
}

/**
 * ghwp_document_get_modification_date:
 * @document: A #GHWPDocument
 *
 * Returns the date the document was most recently modified as seconds since the Epoch
 *
 * Return value: the date the document was most recently modified, or -1
 *
 * Since: 0.1.2
 **/
GTime
ghwp_document_get_modification_date (GHWPDocument *document)
{
    g_return_val_if_fail (GHWP_IS_DOCUMENT (document), (GTime)-1);
    return document->mod_date;
}

/**
 * ghwp_document_get_hwp_format:
 * @document: A #GHWPDocument
 *
 * Returns the HWP format of @document as a string (e.g. HWP v5.0.0.6)
 *
 * Return value: a new allocated string containing the HWP format
 *               of @document, or %NULL
 *
 * Since: 0.1.2
 **/
gchar *
ghwp_document_get_format (GHWPDocument *document)
{
    gchar *format;

    g_return_val_if_fail (GHWP_IS_DOCUMENT (document), NULL);

    format = g_strdup_printf ("HWP v%s",
        ghwp_document_get_hwp_version_string (document));
  return format;
}

/**
 * ghwp_document_get_hwp_version_string:
 * @document: A #GHWPDocument
 *
 * Returns the HWP version of @document as a string (e.g. 5.0.0.6)
 *
 * Return value: a new allocated string containing the HWP version
 *               of @document, or %NULL
 *
 * Since: 0.1.2
 **/
gchar *
ghwp_document_get_hwp_version_string (GHWPDocument *document)
{
    g_return_val_if_fail (GHWP_IS_DOCUMENT (document), NULL);
    return ghwp_file_get_hwp_version_string(document->file);
}

/**
 * ghwp_document_get_hwp_version:
 * @document: A #GHWPDocument
 * @major_version: (out) (allow-none): return location for the HWP major version number
 * @minor_version: (out) (allow-none): return location for the HWP minor version number
 * @micro_version: (out) (allow-none): return location for the HWP micro version number
 * @extra_version: (out) (allow-none): return location for the HWP extra version number
 *
 * Returns: the major and minor and micro and extra HWP version numbers
 *
 * Since: 0.1.2
 **/
void
ghwp_document_get_hwp_version (GHWPDocument *document,
                               guint8       *major_version,
                               guint8       *minor_version,
                               guint8       *micro_version,
                               guint8       *extra_version)
{
    g_return_if_fail (GHWP_IS_DOCUMENT (document));

    ghwp_file_get_hwp_version (document->file,
                               major_version,
                               minor_version,
                               micro_version,
                               extra_version);
}

void ghwp_parse_document_property (GHWPDocument *doc,
                                   GHWPContext  *ctx)
{
    GHWPDocumentProperty *prop;

    g_return_if_fail (GHWP_IS_DOCUMENT (doc));
    g_return_if_fail (GHWP_IS_CONTEXT (ctx));

    prop = &doc->info_v5.prop;

    context_read_uint16 (ctx, &prop->n_sections);
    context_read_uint16 (ctx, &prop->start_page_num);
    context_read_uint16 (ctx, &prop->start_footnote_num);
    context_read_uint16 (ctx, &prop->start_endnote_num);
    context_read_uint16 (ctx, &prop->start_picture_num);
    context_read_uint16 (ctx, &prop->start_table_num);
    context_read_uint16 (ctx, &prop->start_math_num);
    context_read_uint32 (ctx, &prop->list_id);
    context_read_uint32 (ctx, &prop->paragraph_id);
    context_read_uint32 (ctx, &prop->char_unit_pos);
}

