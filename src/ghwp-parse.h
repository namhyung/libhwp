/*
 * ghwp-parse.h
 *
 * Copyright (C) 2012-2013  Hodong Kim <cogniti@gmail.com>
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

#ifndef _GHWP_CONTEXT_H_
#define _GHWP_CONTEXT_H_

#include <glib-object.h>
#include <gio/gio.h>

#include "ghwp.h"

G_BEGIN_DECLS

#define GHWP_TYPE_CONTEXT             (ghwp_context_get_type ())
#define GHWP_CONTEXT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GHWP_TYPE_CONTEXT, GHWPContext))
#define GHWP_CONTEXT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GHWP_TYPE_CONTEXT, GHWPContextClass))
#define GHWP_IS_CONTEXT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GHWP_TYPE_CONTEXT))
#define GHWP_IS_CONTEXT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GHWP_TYPE_CONTEXT))
#define GHWP_CONTEXT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GHWP_TYPE_CONTEXT, GHWPContextClass))

/*typedef enum*/
/*{*/
/*    GHWP_PARSE_ERROR_UNKNOWN_TAG*/
/*} GHWPParseError;*/

typedef enum
{
    STATE_NORMAL,
    STATE_INSIDE_TABLE
} GHWPParseState;

typedef struct _GHWPContext        GHWPContext;
typedef struct _GHWPContextClass   GHWPContextClass;
typedef struct _GHWPContextPrivate GHWPContextPrivate;

struct _GHWPContext {
    GObject             parent_instance;
    GHWPContextPrivate *priv;
    GInputStream       *stream;
    guint16             tag_id;
    guint16             level;
    guint16             data_len;
    guint16             data_count;
    guint8              status;
};

struct _GHWPContextClass {
    GObjectClass parent_class;
};

struct _GHWPContextPrivate {
    guint32           header;
    gsize             bytes_read;
    gboolean          ret;
};

GType        ghwp_context_get_type   (void) G_GNUC_CONST;
GHWPContext *ghwp_context_new        (GInputStream *stream);
gboolean     ghwp_context_pull       (GHWPContext  *context,
				      GError **error);
gboolean     context_read_int8       (GHWPContext  *context,
                                      gint8        *i);
gboolean     context_read_int16      (GHWPContext  *context,
                                      gint16       *i);
gboolean     context_read_int32      (GHWPContext  *context,
                                      gint32       *i);
gboolean     context_read_uint8      (GHWPContext  *context,
                                      guint8       *i);
gboolean     context_read_uint16     (GHWPContext  *context,
                                      guint16      *i);
gboolean     context_read_uint32     (GHWPContext  *context,
                                      guint32      *i);
gboolean     context_read_hwp_unit   (GHWPContext  *context,
                                      ghwp_unit    *i);
gboolean     context_read_hwp_unit16 (GHWPContext  *context,
                                      ghwp_unit16  *i);
gboolean     context_read_hwp_color  (GHWPContext  *context,
                                      ghwp_color   *i);
gboolean     context_skip            (GHWPContext  *context,
                                      guint16       count);

G_END_DECLS

#endif /* _GHWP_CONTEXT_H_ */
