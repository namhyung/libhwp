/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ghwp-document.h
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

#ifndef __GHWP_DOCUMENT_H__
#define __GHWP_DOCUMENT_H__

#include <glib-object.h>
#include <gsf/gsf-doc-meta-data.h>

#include "ghwp.h"

G_BEGIN_DECLS

#define GHWP_TYPE_DOCUMENT             (ghwp_document_get_type ())
#define GHWP_DOCUMENT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GHWP_TYPE_DOCUMENT, GHWPDocument))
#define GHWP_DOCUMENT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GHWP_TYPE_DOCUMENT, GHWPDocumentClass))
#define GHWP_IS_DOCUMENT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GHWP_TYPE_DOCUMENT))
#define GHWP_IS_DOCUMENT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GHWP_TYPE_DOCUMENT))
#define GHWP_DOCUMENT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GHWP_TYPE_DOCUMENT, GHWPDocumentClass))

typedef struct _GHWPDocument           GHWPDocument;
typedef struct _GHWPDocumentClass      GHWPDocumentClass;
typedef struct _GHWPDocumentPrivate    GHWPDocumentPrivate;
typedef struct _GHWPDocumentProperty   GHWPDocumentProperty;

struct _GHWPDocumentProperty {
    guint16  n_sections;
    guint16  start_page_num;
    guint16  start_footnote_num;  // 각주 시작 번호
    guint16  start_endnote_num;   // 미주 시작 번호
    guint16  start_picture_num;
    guint16  start_table_num;
    guint16  start_math_num;
    guint32  list_id;
    guint32  paragraph_id;
    guint32  char_unit_pos;
};

struct _GHWPDocument {
    GObject              parent_instance;
    GHWPDocumentPrivate *priv;
    GHWPFile            *file;
    gchar               *prv_text;
    GArray              *paragraphs;
    GArray              *pages;
    GArray              *sections;
    GsfDocMetaData      *summary_info;

    struct {
        GHWPDocumentProperty  prop;
    } info_v5;

    /* ev info */
    const gchar         *title;
    gchar               *format;
    const gchar         *author;
    const gchar         *subject;
    const gchar         *keywords;
    gchar               *layout;
    gchar               *start_mode;
    gchar               *permissions;
    gchar               *ui_hints;
    const gchar         *creator;
    gchar               *producer;
    GTime                creation_date;
    GTime                mod_date;
    gchar               *linearized;
    guint                n_pages; /* FIXME duplicate */
    gchar               *security;
    gchar               *paper_size;
    gchar               *license;
    /* hwp info */
    const gchar         *desc;
    GTime                last_printed;
    const gchar         *last_saved_by;
    const gchar         *revision_count;
};

struct _GHWPDocumentClass {
    GObjectClass parent_class;
};

GType         ghwp_document_get_type           (void) G_GNUC_CONST;
GHWPDocument *ghwp_document_new                (void);
GHWPDocument *ghwp_document_new_from_uri       (const gchar  *uri,
                                                GError      **error);
GHWPDocument *ghwp_document_new_from_filename  (const gchar  *filename,
                                                GError      **error);
guint     ghwp_document_get_n_pages            (GHWPDocument *doc);
GHWPPage *ghwp_document_get_page               (GHWPDocument *doc, gint n_page);
/* meta data */
gchar    *ghwp_document_get_title              (GHWPDocument *document);
gchar    *ghwp_document_get_keywords           (GHWPDocument *document);
gchar    *ghwp_document_get_subject            (GHWPDocument *document);
gchar    *ghwp_document_get_creator            (GHWPDocument *document);
GTime     ghwp_document_get_creation_date      (GHWPDocument *document);
GTime     ghwp_document_get_modification_date  (GHWPDocument *document);
gchar    *ghwp_document_get_format             (GHWPDocument *document);
gchar    *ghwp_document_get_hwp_version_string (GHWPDocument *document);
void      ghwp_document_get_hwp_version        (GHWPDocument *document,
                                                guint8       *major_version,
                                                guint8       *minor_version,
                                                guint8       *micro_version,
                                                guint8       *extra_version);

void      ghwp_parse_document_property         (GHWPDocument *document,
                                                GHWPContext  *context);
G_END_DECLS

#endif /* __GHWP_DOCUMENT_H__ */
