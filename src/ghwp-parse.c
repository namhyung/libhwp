/*
 * ghwp-parse.c
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

#include "config.h"
#include <glib.h>
#include <glib-object.h>
#include <glib/gi18n-lib.h>
#include <gio/gio.h>

#include "ghwp.h"
#include "ghwp-parse.h"

G_DEFINE_TYPE (GHWPContext, ghwp_context, G_TYPE_OBJECT);

#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))
#define _g_error_free0(var) ((var == NULL) ? NULL : (var = (g_error_free (var), NULL)))

#define GHWP_CONTEXT_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), GHWP_TYPE_CONTEXT, GHWPContextPrivate))

static void ghwp_context_finalize (GObject* obj);

gboolean context_skip (GHWPContext *context, guint16 count)
{
    g_return_val_if_fail (context != NULL, FALSE);

    gboolean is_success = FALSE;
    guint8  *buf        = g_malloc (count);
    /* FIXME g_input_stream_skip 가 작동하지 않아 아래처럼 처리했다. */
    is_success = g_input_stream_read_all (context->stream, buf, (gsize) count,
                                          &context->priv->bytes_read,
                                          NULL, NULL);
    g_free (buf);

    if ((is_success == FALSE) || (context->priv->bytes_read != (gsize) count))
    {
        g_warning ("%s:%d:skip size mismatch\n", __FILE__, __LINE__);
        g_input_stream_close (context->stream, NULL, NULL);
        return FALSE;
    }

    context->data_count += count;
    return TRUE;
}

gboolean context_read_int8 (GHWPContext *context, gint8 *i)
{
    g_return_val_if_fail (context != NULL, FALSE);
    g_return_val_if_fail (context->data_count <= context->data_len - 1, FALSE);

    gboolean is_success = FALSE;
    is_success = g_input_stream_read_all (context->stream, i, 1,
                                          &context->priv->bytes_read,
                                          NULL, NULL);
    if ((is_success == FALSE) ||
        (context->priv->bytes_read != 1))
    {
        *i = 0;
        g_input_stream_close (context->stream, NULL, NULL);
        return FALSE;
    }
    context->data_count += 1;
    return TRUE;
}

gboolean context_read_int16 (GHWPContext *context, gint16 *i)
{
    g_return_val_if_fail (context != NULL, FALSE);
    g_return_val_if_fail (context->data_count <= context->data_len - 2, FALSE);

    gboolean is_success = FALSE;
    is_success = g_input_stream_read_all (context->stream, i, 2,
                                          &context->priv->bytes_read,
                                          NULL, NULL);
    if ((is_success == FALSE) ||
        (context->priv->bytes_read != 2))
    {
        *i = 0;
        g_input_stream_close (context->stream, NULL, NULL);
        return FALSE;
    }
    *i = GINT16_FROM_LE(*i);
    context->data_count += 2;
    return TRUE;
}

gboolean context_read_int32 (GHWPContext *context, gint32 *i)
{
    g_return_val_if_fail (context != NULL, FALSE);
    g_return_val_if_fail (context->data_count <= context->data_len - 4, FALSE);

    gboolean is_success = FALSE;
    is_success = g_input_stream_read_all (context->stream, i, 4,
                                          &context->priv->bytes_read,
                                          NULL, NULL);
    if ((is_success == FALSE) ||
        (context->priv->bytes_read != 4))
    {
        *i = 0;
        g_input_stream_close (context->stream, NULL, NULL);
        return FALSE;
    }
    *i = GINT32_FROM_LE(*i);
    context->data_count += 4;
    return TRUE;
}

gboolean context_read_uint8 (GHWPContext *context, guint8 *i)
{
    g_return_val_if_fail (context != NULL, FALSE);
    g_return_val_if_fail (context->data_count <= context->data_len - 1, FALSE);

    gboolean is_success = FALSE;
    is_success = g_input_stream_read_all (context->stream, i, 1,
                                          &context->priv->bytes_read,
                                          NULL, NULL);
    if ((is_success == FALSE) ||
        (context->priv->bytes_read != 1))
    {
        *i = 0;
        g_input_stream_close (context->stream, NULL, NULL);
        return FALSE;
    }
    context->data_count += 1;
    return TRUE;
}

gboolean context_read_uint16 (GHWPContext *context, guint16 *i)
{
    g_return_val_if_fail (context != NULL, FALSE);
    g_return_val_if_fail (context->data_count <= context->data_len - 2, FALSE);

    gboolean is_success = FALSE;
    is_success = g_input_stream_read_all (context->stream, i, 2,
                                          &context->priv->bytes_read,
                                          NULL, NULL);
    if ((is_success == FALSE) ||
        (context->priv->bytes_read != 2))
    {
        *i = 0;
        g_input_stream_close (context->stream, NULL, NULL);
        return FALSE;
    }
    *i = GUINT16_FROM_LE(*i);
    context->data_count += 2;
    return TRUE;
}

gboolean context_read_uint32 (GHWPContext *context, guint32 *i)
{
    g_return_val_if_fail (context != NULL, FALSE);
    g_return_val_if_fail (context->data_count <= context->data_len - 4, FALSE);

    gboolean is_success = FALSE;
    is_success = g_input_stream_read_all (context->stream, i, 4,
                                          &context->priv->bytes_read,
                                          NULL, NULL);
    if ((is_success == FALSE) ||
        (context->priv->bytes_read != 4))
    {
        *i = 0;
        g_input_stream_close (context->stream, NULL, NULL);
        return FALSE;
    }
    *i = GUINT32_FROM_LE(*i);
    context->data_count += 4;
    return TRUE;
}

gboolean context_read_hwp_unit16 (GHWPContext *context, ghwp_unit16 *i)
{
    g_return_val_if_fail (context != NULL, FALSE);
    g_return_val_if_fail (context->data_count <= context->data_len - 2, FALSE);

    gboolean is_success = FALSE;
    is_success = g_input_stream_read_all (context->stream, i, 2,
                                          &context->priv->bytes_read,
                                          NULL, NULL);
    if ((is_success == FALSE) ||
        (context->priv->bytes_read != 2))
    {
        *i = 0;
        g_input_stream_close (context->stream, NULL, NULL);
        return FALSE;
    }
    *i = GUINT16_FROM_LE(*i);
    context->data_count += 2;
    return TRUE;
}

gboolean context_read_hwp_unit (GHWPContext *context, ghwp_unit *i)
{
    g_return_val_if_fail (context != NULL, FALSE);
    g_return_val_if_fail (context->data_count <= context->data_len - 4, FALSE);

    gboolean is_success = FALSE;
    is_success = g_input_stream_read_all (context->stream, i, 4,
                                          &context->priv->bytes_read,
                                          NULL, NULL);
    if ((is_success == FALSE) ||
        (context->priv->bytes_read != 4))
    {
        *i = 0;
        g_input_stream_close (context->stream, NULL, NULL);
        return FALSE;
    }
    *i = GUINT32_FROM_LE(*i);
    context->data_count += 4;
    return TRUE;
}

gboolean context_read_hwp_color (GHWPContext *context, ghwp_color *i)
{
    g_return_val_if_fail (context != NULL, FALSE);
    g_return_val_if_fail (context->data_count <= context->data_len - 4, FALSE);

    gboolean is_success = FALSE;
    is_success = g_input_stream_read_all (context->stream, i, 4,
                                          &context->priv->bytes_read,
                                          NULL, NULL);
    if ((is_success == FALSE) ||
        (context->priv->bytes_read != 4))
    {
        *i = 0;
        g_input_stream_close (context->stream, NULL, NULL);
        return FALSE;
    }
    *i = GUINT32_FROM_LE(*i);
    context->data_count += 4;
    return TRUE;
}

gboolean context_check_version (GHWPContext  *context,
				guint8        major,
				guint8        minor,
				guint8        micro,
				guint8        extra)
{
    if (context->version[0] < major)
        return FALSE;
    if (context->version[1] < minor)
        return FALSE;
    if (context->version[2] < micro)
        return FALSE;
    if (context->version[3] < extra)
        return FALSE;

    return TRUE;
}

GHWPContext* ghwp_context_new (GInputStream* stream)
{
    g_return_val_if_fail (stream != NULL, NULL);
    GHWPContext *context = g_object_new (GHWP_TYPE_CONTEXT, NULL);
    context->stream = g_object_ref (stream);
    return context;
}

/* 에러일 경우 FALSE 반환, error 설정,
 * 성공일 경우 TRUE 반환,
 * end-of-stream 일 경우 FALSE 반환, error 설정 안 함 */
gboolean ghwp_context_pull (GHWPContext *context, GError **error)
{
    g_return_val_if_fail (context != NULL, FALSE);
    gboolean is_success = TRUE;
    if (context->data_len - context->data_count > 0)
        context_skip (context, context->data_len - context->data_count);
    /* 4바이트 읽기 */
    is_success = g_input_stream_read_all (context->stream,
                                          &context->priv->header,
                                          (gsize) 4,
                                          &context->priv->bytes_read,
                                          NULL, error);

    if (is_success == FALSE) {
        /* g_input_stream_read_all이 에러를 설정했으므로
         * 따로 에러 설정할 필요 없다. */
        g_input_stream_close (context->stream, NULL, NULL);
        return FALSE;
    }
    /* 스트림의 끝, 에러가 아님 */
    if (context->priv->bytes_read == ((gsize) 0)) {
        g_input_stream_close (context->stream, NULL, error);
        return FALSE;
    }
    /* 비정상 */
    if (context->priv->bytes_read != ((gsize) 4)) {
        g_set_error_literal (error, GHWP_ERROR, GHWP_ERROR_INVALID,
                             _("File corrupted"));
        g_input_stream_close (context->stream, NULL, NULL);
        return FALSE;
    }

    /* Converts a guint32 value from little-endian to host byte order. */
    context->priv->header = GUINT32_FROM_LE(context->priv->header);
    /* 4바이트 헤더 디코딩하기 */
    context->tag_id   = (guint16) ( context->priv->header        & 0x3ff);
    context->level    = (guint16) ((context->priv->header >> 10) & 0x3ff);
    context->data_len = (guint16) ((context->priv->header >> 20) & 0xfff);
    /* 비정상 */
    if (context->data_len == 0) {
        g_set_error_literal (error, GHWP_ERROR, GHWP_ERROR_INVALID,
                             _("File corrupted"));
        g_input_stream_close (context->stream, NULL, NULL);
        return FALSE;
    }
    /* data_len == 0xfff 이면 다음 4바이트는 data_len 이다 */
    if (context->data_len == 0xfff) {
        is_success = g_input_stream_read_all (context->stream,
                                              &context->data_len, (gsize) 4,
                                              &context->priv->bytes_read,
                                              NULL, error);

        if (is_success == FALSE) {
            /* g_input_stream_read_all이 에러를 설정했으므로
             * 따로 에러 설정할 필요 없다. */
            g_input_stream_close (context->stream, NULL, NULL);
            return FALSE;
        }

        /* 비정상 */
        if (context->priv->bytes_read != ((gsize) 4)) {
            g_set_error_literal (error, GHWP_ERROR, GHWP_ERROR_INVALID,
                                 _("File corrupted"));
            g_input_stream_close (context->stream, NULL, NULL);
            return FALSE;
        }

        context->data_len = GUINT32_FROM_LE(context->data_len);
    }

    context->data_count = 0;

    return TRUE;
}


static void ghwp_context_class_init (GHWPContextClass * klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    g_type_class_add_private (klass, sizeof (GHWPContextPrivate));
    object_class->finalize = ghwp_context_finalize;
}


static void ghwp_context_init (GHWPContext * context)
{
    context->status[0].s = STATE_NORMAL;
    context->status[0].p = NULL;
    context->priv       = GHWP_CONTEXT_GET_PRIVATE (context);
}


static void ghwp_context_finalize (GObject *obj)
{
    GHWPContext *context = GHWP_CONTEXT(obj);
    g_input_stream_close (context->stream, NULL, NULL);
    g_object_unref (context->stream);
/*    context->data = (g_free (context->data), NULL);*/
    G_OBJECT_CLASS (ghwp_context_parent_class)->finalize (obj);
}
