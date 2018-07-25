/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ghwp-file-v5.h
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

#ifndef _GHWP_FILE_V5_H_
#define _GHWP_FILE_V5_H_

#include <glib-object.h>
#include <gio/gio.h>
#include <gsf/gsf-infile-msole.h>

#include "ghwp.h"

G_BEGIN_DECLS

#define GHWP_TYPE_FILE_V5             (ghwp_file_v5_get_type ())
#define GHWP_FILE_V5(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GHWP_TYPE_FILE_V5, GHWPFileV5))
#define GHWP_FILE_V5_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GHWP_TYPE_FILE_V5, GHWPFileV5Class))
#define GHWP_IS_FILE_V5(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GHWP_TYPE_FILE_V5))
#define GHWP_IS_FILE_V5_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GHWP_TYPE_FILE_V5))
#define GHWP_FILE_V5_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GHWP_TYPE_FILE_V5, GHWPFileV5Class))

typedef struct _GHWPFileV5Class   GHWPFileV5Class;
typedef struct _GHWPFileV5Private GHWPFileV5Private;

struct _GHWPFileV5
{
    GHWPFile           parent_instance;
    GHWPFileV5Private *priv;

    GArray            *section_streams;
    GArray            *bindata_streams;
    GInputStream      *prv_text_stream;
    GInputStream      *prv_image_stream;
    GInputStream      *file_header_stream;
    GInputStream      *doc_info_stream;
    GInputStream      *summary_info_stream;

    gchar   *signature;
    guint8   major_version;
    guint8   minor_version;
    guint8   micro_version;
    guint8   extra_version;
    gboolean is_compress;
    gboolean is_encrypt;
    gboolean is_distribute;
    gboolean is_script;
    gboolean is_drm;
    gboolean is_xml_template;
    gboolean is_history;
    gboolean is_sign;
    gboolean is_certificate_encrypt;
    gboolean is_sign_spare;
    gboolean is_certificate_drm;
    gboolean is_ccl;
};

struct _GHWPFileV5Class
{
    GHWPFileClass parent_class;
};

struct _GHWPFileV5Private
{
    GsfInfileMSOle *olefile;
    GInputStream   *section_stream;
};

GType         ghwp_file_v5_get_type               (void) G_GNUC_CONST;
GHWPFileV5   *ghwp_file_v5_new_from_uri           (const gchar *uri,
                                                   GError     **error);
GHWPFileV5   *ghwp_file_v5_new_from_filename      (const gchar *filename,
                                                   GError     **error);
gchar        *ghwp_file_v5_get_hwp_version_string (GHWPFile    *file);
void          ghwp_file_v5_get_hwp_version        (GHWPFile    *file,
                                                   guint8      *major_version,
                                                   guint8      *minor_version,
                                                   guint8      *micro_version,
                                                   guint8      *extra_version);
GHWPDocument *ghwp_file_v5_get_document           (GHWPFile    *file,
                                                   GError     **error);

G_END_DECLS

#endif /* _GHWP_FILE_V5_H_ */
