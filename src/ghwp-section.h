/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ghwp-section.h
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

/*
 * 한글과컴퓨터의 한/글 문서 파일(.hwp) 공개 문서를 참고하여 개발하였습니다.
 */

#ifndef __GHWP_SECTION_H__
#define __GHWP_SECTION_H__

#include <glib-object.h>

#include "ghwp.h"

G_BEGIN_DECLS

#define GHWP_TYPE_SECTION             (ghwp_section_get_type ())
#define GHWP_SECTION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GHWP_TYPE_SECTION, GHWPSection))
#define GHWP_SECTION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GHWP_TYPE_SECTION, GHWPSectionClass))
#define GHWP_IS_SECTION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GHWP_TYPE_SECTION))
#define GHWP_IS_SECTION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GHWP_TYPE_SECTION))
#define GHWP_SECTION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GHWP_TYPE_SECTION, GHWPSectionClass))

typedef struct _GHWPSectionClass   GHWPSectionClass;
typedef struct _GHWPSectionDef     GHWPSectionDef;
typedef struct _GHWPPageDef        GHWPPageDef;
typedef struct _GHWPColumnDef      GHWPColumnDef;

struct _GHWPSectionDef
{
    guint32      attr;
    ghwp_unit16  col_spacing;
    ghwp_unit16  v_align;
    ghwp_unit16  h_align;
    ghwp_unit    default_tab_size;
    guint16      num_para_shape_id;
    guint16      page_num;
    guint16      image_num;
    guint16      table_num;
    guint16      math_num;
    guint16      lang;
};

struct _GHWPPageDef
{
    ghwp_unit  h_size;
    ghwp_unit  v_size;
    ghwp_unit  l_margin;  /* 왼쪽 여백 */
    ghwp_unit  r_margin;  /* 왼쪽 여백 */
    ghwp_unit  t_margin;  /* 위쪽 여백 */
    ghwp_unit  b_margin;  /* 아래 여백 */
    ghwp_unit  header;    /* 머리말 여백 */
    ghwp_unit  footer;    /* 꼬리말 여백 */
    ghwp_unit  binding;   /* 제본 여백 */
    guint32    attr;
};

#define COL_ATTR_SAME_WIDTH  (1U << 12)

struct _GHWPColumnDef
{
    guint32      attr;
    ghwp_unit16  spacing;
    gint         n_cols;
    guint8       border_kind;
    guint8       border_weight;
    ghwp_color   border_color;
    guint16     *col_widths;
};

struct _GHWPSection
{
    GObject         parent_instance;
    GHWPSectionDef  def_info;
    GHWPPageDef     page_info;
    GHWPColumnDef   col_info;
};

struct _GHWPSectionClass
{
    GObjectClass parent_class;
};

GType        ghwp_section_get_type   (void) G_GNUC_CONST;
GHWPSection *ghwp_section_new        (void);

gboolean ghwp_parse_section_def      (GHWPSection *sec,
                                      GHWPContext *ctx);
gboolean ghwp_parse_page_def         (GHWPSection *sec,
                                      GHWPContext *ctx);
gboolean ghwp_parse_column_def       (GHWPSection *sec,
                                      GHWPContext *ctx);

G_END_DECLS

#endif /* __GHWP_SECTION_H__ */
