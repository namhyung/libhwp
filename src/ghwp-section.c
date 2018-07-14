/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ghwp-section.c
 *
 * Copyright (C) 2018 Namhyung Kim <namhyung@gmail.com>
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

#include "ghwp-section.h"
#include "ghwp-parse.h"

G_DEFINE_TYPE (GHWPSection, ghwp_section, G_TYPE_OBJECT);

GHWPSection *ghwp_section_new (void)
{
    return (GHWPSection *) g_object_new (GHWP_TYPE_SECTION, NULL);
}

static void ghwp_section_finalize (GObject *obj)
{
    G_OBJECT_CLASS (ghwp_section_parent_class)->finalize (obj);
}

static void ghwp_section_class_init (GHWPSectionClass * klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    object_class->finalize     = ghwp_section_finalize;
}

static void ghwp_section_init (GHWPSection *sec)
{
}

gboolean ghwp_parse_section_def (GHWPSection *sec, GHWPContext *ctx)
{
    g_return_val_if_fail (sec != NULL, FALSE);

    context_read_uint32 (ctx, &sec->def_info.attr);
    context_read_hwp_unit16 (ctx, &sec->def_info.col_spacing);
    context_read_hwp_unit16 (ctx, &sec->def_info.v_align);
    context_read_hwp_unit16 (ctx, &sec->def_info.h_align);
    context_read_hwp_unit (ctx, &sec->def_info.default_tab_size);
    context_read_uint16 (ctx, &sec->def_info.num_para_shape_id);
    context_read_uint16 (ctx, &sec->def_info.page_num);
    context_read_uint16 (ctx, &sec->def_info.image_num);
    context_read_uint16 (ctx, &sec->def_info.table_num);
    context_read_uint16 (ctx, &sec->def_info.math_num);
    context_read_uint16 (ctx, &sec->def_info.lang);

    return TRUE;
}

gboolean ghwp_parse_page_def (GHWPSection *sec, GHWPContext *ctx)
{
    g_return_val_if_fail (sec != NULL, FALSE);

    context_read_hwp_unit (ctx, &sec->page_info.h_size);
    context_read_hwp_unit (ctx, &sec->page_info.v_size);
    context_read_hwp_unit (ctx, &sec->page_info.l_margin);
    context_read_hwp_unit (ctx, &sec->page_info.r_margin);
    context_read_hwp_unit (ctx, &sec->page_info.t_margin);
    context_read_hwp_unit (ctx, &sec->page_info.b_margin);
    context_read_hwp_unit (ctx, &sec->page_info.header);
    context_read_hwp_unit (ctx, &sec->page_info.footer);
    context_read_hwp_unit (ctx, &sec->page_info.binding);
    context_read_uint32 (ctx, &sec->page_info.attr);

    return TRUE;
}

gboolean ghwp_parse_column_def (GHWPSection *sec, GHWPContext *ctx)
{
    guint16 attr;

    g_return_val_if_fail (sec != NULL, FALSE);

    context_read_uint16 (ctx, &attr);
    sec->col_info.attr = attr;

    sec->col_info.n_cols = (sec->col_info.attr >> 2) & 0xff;

    sec->col_info.col_widths = NULL;
    if (sec->col_info.attr & COL_ATTR_SAME_WIDTH) {
        gint i;

        sec->col_info.col_widths = calloc(sec->col_info.n_cols,
                                          sizeof (guint16));

        for (i = 0; i < sec->col_info.n_cols; i++)
            context_read_uint16 (ctx, &sec->col_info.col_widths[i]);
    }

    context_read_uint16 (ctx, &attr);
    sec->col_info.attr |= ((guint32) attr) << 16;

    context_read_uint8 (ctx, &sec->col_info.border_kind);
    context_read_uint8 (ctx, &sec->col_info.border_weight);
    context_read_hwp_color (ctx, &sec->col_info.border_color);

    return TRUE;
}
