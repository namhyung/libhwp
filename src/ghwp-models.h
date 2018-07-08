/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ghwp-table.c
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

/*
 * This software have been developed with reference to
 * the HWP file format open specification by Hancom, Inc.
 * http://www.hancom.co.kr/userofficedata.userofficedataList.do?menuFlag=3
 * 한글과컴퓨터의 한/글 문서 파일(.hwp) 공개 문서를 참고하여 개발하였습니다.
 */

#ifndef __GHWP_MODELS_H__
#define __GHWP_MODELS_H__

#include <glib-object.h>
#include "ghwp.h"

G_BEGIN_DECLS

#define OBJ_ATTR_LIKE_TEXT              (1U << 0)
#define OBJ_ATTR_RESERVED               (1U << 1)
#define OBJ_ATTR_AFFECT_LINE            (1U << 2)
#define OBJ_ATTR_VERT_REL_TO_MASK       (3U << 3)
#define OBJ_ATTR_VERT_REL_ARRANGE_MASK  (7U << 5)
#define OBJ_ATTR_HORZ_REL_TO_MASK       (3U << 8)
#define OBJ_ATTR_HORZ_REL_ARRANGE_MASK  (7U << 10)
#define OBJ_ATTR_ALLOW_OVERLAP          (1U << 14)
#define OBJ_ATTR_WIDTH_START_MASK       (7U << 15)
#define OBJ_ATTR_HEIGHT_START_MASK      (3U << 18)
#define OBJ_ATTR_TEXT_FLOW_MASK         (7U << 21)
#define OBJ_ATTR_TEXT_SIDE_MASK         (3U << 24)
#define OBJ_ATTR_CATEGORY_MASK          (3U << 26)

/* use with OBJ_ATTR_VERT_REL_TO_MASK */
#define OBJ_ATTR_VERT_REL_TO_PAPER  (0U << 3)
#define OBJ_ATTR_VERT_REL_TO_PAGE   (1U << 3)
#define OBJ_ATTR_VERT_REL_TO_PARA   (2U << 3)

/* use with OBJ_ATTR_HORZ_REL_TO_MASK */
#define OBJ_ATTR_HORZ_REL_TO_PAPER  (0U << 8)
#define OBJ_ATTR_HORZ_REL_TO_PAGE   (1U << 8)
#define OBJ_ATTR_HORZ_REL_TO_COLUMN (2U << 8)
#define OBJ_ATTR_HORZ_REL_TO_PARA   (3U << 8)

/* use with OBJ_ATTR_WIDTH_START_MASK */
#define OBJ_ATTR_WIDTH_START_PAPER  (0U << 15)
#define OBJ_ATTR_WIDTH_START_PAGE   (1U << 15)
#define OBJ_ATTR_WIDTH_START_COLUMN (2U << 15)
#define OBJ_ATTR_WIDTH_START_PARA   (3U << 15)
#define OBJ_ATTR_WIDTH_START_ABS    (4U << 15)

/* use with OBJ_ATTR_HEIGHT_START_MASK */
#define OBJ_ATTR_HEIGHT_START_PAPER  (0U << 18)
#define OBJ_ATTR_HEIGHT_START_PAGE   (1U << 18)
#define OBJ_ATTR_HEIGHT_START_ABS    (2U << 18)

/* use with OBJ_ATTR_TEXT_FLOW_MASK */
#define OBJ_ATTR_TEXT_FLOW_SQUARE   (0U << 21)
#define OBJ_ATTR_TEXT_FLOW_TIGHT    (1U << 21)
#define OBJ_ATTR_TEXT_FLOW_THROUGH  (2U << 21)
#define OBJ_ATTR_TEXT_FLOW_TOP_BOT  (3U << 21)
#define OBJ_ATTR_TEXT_FLOW_BEHIND   (4U << 21)
#define OBJ_ATTR_TEXT_FLOW_FRONT    (5U << 21)

/* use with OBJ_ATTR_TEXT_SIDE_MASK */
#define OBJ_ATTR_TEXT_SIDE_BOTH     (0U << 24)
#define OBJ_ATTR_TEXT_SIDE_LEFT     (1U << 24)
#define OBJ_ATTR_TEXT_SIDE_RIGHT    (2U << 24)
#define OBJ_ATTR_TEXT_SIDE_LARGEST  (3U << 24)

/* use with OBJ_ATTR_CATEGORY_MASK */
#define OBJ_ATTR_CATEGORY_NONE      (0U << 26)
#define OBJ_ATTR_CATEGORY_FIGURE    (1U << 26)
#define OBJ_ATTR_CATEGORY_TABLE     (2U << 26)
#define OBJ_ATTR_CATEGORY_EQUATION  (3U << 26)

struct _GHWPObject {
    guint32      ctrl_id;
    guint32      attr;        /* 표 70 및 위의 OBJ_ATTR 매크로 정의 참조 */
    ghwp_unit    v_offset;
    ghwp_unit    h_offset;
    ghwp_unit    width;
    ghwp_unit    height;
    gint32       z_order;
    ghwp_unit16  l_spacing;  /* left */
    ghwp_unit16  r_spacing;  /* right */
    ghwp_unit16  t_spacing;  /* top */
    ghwp_unit16  b_spacing;  /* bottom */
    guint32      instance_id;
    gint32       page_split;
    guint16      n_desc;
    gchar       *desc;
};

typedef struct _GHWPText      GHWPText;
typedef struct _GHWPTable     GHWPTable;
typedef struct _GHWPTableCell GHWPTableCell;
typedef struct _GHWPObject    GHWPObject;

void ghwp_parse_common_object (GHWPObject *obj, GHWPContext *ctx);

/** GHWPParagraph ************************************************************/

#define GHWP_TYPE_PARAGRAPH             (ghwp_paragraph_get_type ())
#define GHWP_PARAGRAPH(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GHWP_TYPE_PARAGRAPH, GHWPParagraph))
#define GHWP_PARAGRAPH_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GHWP_TYPE_PARAGRAPH, GHWPParagraphClass))
#define GHWP_IS_PARAGRAPH(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GHWP_TYPE_PARAGRAPH))
#define GHWP_IS_PARAGRAPH_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GHWP_TYPE_PARAGRAPH))
#define GHWP_PARAGRAPH_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GHWP_TYPE_PARAGRAPH, GHWPParagraphClass))

typedef struct _GHWPParagraph       GHWPParagraph;
typedef struct _GHWPParagraphClass  GHWPParagraphClass;
typedef struct _GHWPParagraphHeader GHWPParagraphHeader;
typedef struct _GHWPCharShapeRef    GHWPCharShapeRef;
typedef struct _GHWPLineSeg         GHWPLineSeg;
typedef struct _GHWPRangeTag        GHWPRangeTag;

struct _GHWPParagraphHeader {
    guint32    n_chars;
    guint32    control_mask;
    guint16    para_shape_id;
    guint8     para_style_id;
    guint8     col_split;
    guint16    n_char_shapes;
    guint16    n_range_tags;
    guint16    n_line_segs;
    guint32    para_id;
    guint16    history_merge;  /* 변경추적 병합 문단여부 (5.0.3.2 버전 이상) */
};

struct _GHWPCharShapeRef
{
    guint32  pos;
    guint32  id;
};

#define LINESEG_TAG_PAGE_START    (1U << 0)
#define LINESEG_TAG_COL_START     (1U << 1)
#define LINESEG_TAG_EMPTY         (1U << 16)
#define LINESEG_TAG_LINE_START    (1U << 17)
#define LINESEG_TAG_LINE_END      (1U << 18)
#define LINESEG_TAG_HYPHEN        (1U << 19)
#define LINESEG_TAG_INDENT        (1U << 20)
#define LINESEG_TAG_PARA_HEADING  (1U << 21)
#define LINESEG_TAG_PROPERTY      (1U << 31)

struct _GHWPLineSeg
{
    guint32  text_start;
    gint32   v_pos;
    gint32   line_height;
    gint32   text_height;
    gint32   base_line;
    gint32   line_spacing;
    gint32   col_offset;
    gint32   segment_width;
    guint32  tag;
};

struct _GHWPRangeTag
{
    guint32  start;
    guint32  end;
    guint32  tag;
};

struct _GHWPParagraph
{
    GObject              parent_instance;
    GHWPParagraphHeader  header;
    GArray              *char_shapes;
    GArray              *range_tags;
    GArray              *line_segs;

    GHWPText            *ghwp_text;
    GHWPTable           *table;

    /* 문단이 다음 페이지로 이어지는 경우 */
    GHWPParagraph       *link;
    gint                 line_start;
    gint                 line_end;
};

struct _GHWPParagraphClass
{
    GObjectClass parent_class;
};

GType          ghwp_paragraph_get_type          (void) G_GNUC_CONST;
GHWPParagraph *ghwp_paragraph_new               (void);
void           ghwp_paragraph_set_ghwp_text     (GHWPParagraph *paragraph,
                                                 GHWPText      *ghwp_text);
GHWPText      *ghwp_paragraph_get_ghwp_text     (GHWPParagraph *paragraph);
GHWPTable     *ghwp_paragraph_get_table         (GHWPParagraph *paragraph);
void           ghwp_paragraph_set_table         (GHWPParagraph *paragraph,
                                                 GHWPTable     *table);
void           ghwp_paragraph_add_link          (GHWPParagraph *paragraph,
                                                 GHWPParagraph *link,
                                                 gint           line);
void           ghwp_parse_paragraph_header      (GHWPParagraph *paragraph,
                                                 GHWPContext *ctx);
void           ghwp_parse_paragraph_char_shape  (GHWPParagraph *paragraph,
                                                 GHWPContext *ctx);
void           ghwp_parse_paragraph_line_seg    (GHWPParagraph *paragraph,
                                                 GHWPContext *ctx);
void           ghwp_parse_paragraph_range_tag   (GHWPParagraph *paragraph,
                                                 GHWPContext *ctx);

/** GHWPText *****************************************************************/

#define GHWP_TYPE_TEXT             (ghwp_text_get_type ())
#define GHWP_TEXT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GHWP_TYPE_TEXT, GHWPText))
#define GHWP_TEXT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GHWP_TYPE_TEXT, GHWPTextClass))
#define GHWP_IS_TEXT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GHWP_TYPE_TEXT))
#define GHWP_IS_TEXT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GHWP_TYPE_TEXT))
#define GHWP_TEXT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GHWP_TYPE_TEXT, GHWPTextClass))

typedef struct _GHWPText        GHWPText;
typedef struct _GHWPTextClass   GHWPTextClass;
typedef struct _GHWPTextPrivate GHWPTextPrivate;

struct _GHWPText
{
    GObject          parent_instance;
    GHWPTextPrivate *priv;
    gchar           *text;
};

struct _GHWPTextClass
{
    GObjectClass parent_class;
};

GType     ghwp_text_get_type (void) G_GNUC_CONST;
GHWPText *ghwp_text_new      (const     gchar *text);
GHWPText *ghwp_text_append   (GHWPText *ghwp_text, const gchar *text);

/** GHWPTable ****************************************************************/

#define GHWP_TYPE_TABLE             (ghwp_table_get_type ())
#define GHWP_TABLE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GHWP_TYPE_TABLE, GHWPTable))
#define GHWP_TABLE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GHWP_TYPE_TABLE, GHWPTableClass))
#define GHWP_IS_TABLE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GHWP_TYPE_TABLE))
#define GHWP_IS_TABLE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GHWP_TYPE_TABLE))
#define GHWP_TABLE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GHWP_TYPE_TABLE, GHWPTableClass))

typedef struct _GHWPTable      GHWPTable;
typedef struct _GHWPTableClass GHWPTableClass;

struct _GHWPTableClass
{
    GObjectClass parent_class;
};

struct _GHWPTable
{
    GObject      parent_instance;
    GHWPObject   obj;
    guint32      flags;
    guint16      n_rows; /* 행 개수 */
    guint16      n_cols; /* 열 개수 */
    ghwp_unit16  cell_spacing; /* 셀과 셀 사이의 간격 */
    ghwp_unit16  l_margin;
    ghwp_unit16  r_margin;
    ghwp_unit16  t_margin;
    ghwp_unit16  b_margin;

    ghwp_unit16 *row_sizes;
    guint16      border_fill_id;
    guint16      valid_zone_info_size;
    guint16     *zones;

    GArray      *cells;
};

GType          ghwp_table_get_type         (void) G_GNUC_CONST;
GHWPTable     *ghwp_table_new              (void);
void           ghwp_parse_table_attr       (GHWPTable     *table,
                                            GHWPContext   *context);
GHWPTableCell *ghwp_table_get_last_cell    (GHWPTable     *table);
void           ghwp_table_add_cell         (GHWPTable     *table,
                                            GHWPTableCell *cell);

/** GHWPTableCell ************************************************************/

#define GHWP_TYPE_TABLE_CELL             (ghwp_table_cell_get_type ())
#define GHWP_TABLE_CELL(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GHWP_TYPE_TABLE_CELL, GHWPTableCell))
#define GHWP_TABLE_CELL_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GHWP_TYPE_TABLE_CELL, GHWPTableCellClass))
#define GHWP_IS_TABLE_CELL(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GHWP_TYPE_TABLE_CELL))
#define GHWP_IS_TABLE_CELL_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GHWP_TYPE_TABLE_CELL))
#define GHWP_TABLE_CELL_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GHWP_TYPE_TABLE_CELL, GHWPTableCellClass))

typedef struct _GHWPTableCell      GHWPTableCell;
typedef struct _GHWPTableCellClass GHWPTableCellClass;

struct _GHWPTableCell
{
    GObject parent_instance;

    /* XXX ? */
    guint16 n_paragraphs;
    guint32 flags;
    guint16 unknown;

    guint16 col_addr;
    guint16 row_addr;
    guint16 col_span;
    guint16 row_span;

    ghwp_unit width;
    ghwp_unit height;

    ghwp_unit16 l_margin;
    ghwp_unit16 r_margin;
    ghwp_unit16 t_margin;
    ghwp_unit16 b_margin;

    guint16 border_fill_id;

    GArray *paragraphs;
};

struct _GHWPTableCellClass
{
    GObjectClass parent_class;
};

GType          ghwp_table_cell_get_type           (void) G_GNUC_CONST;
GHWPTableCell *ghwp_table_cell_new                (void);
GHWPParagraph *ghwp_table_cell_get_last_paragraph (GHWPTableCell *cell);
void           ghwp_table_cell_add_paragraph      (GHWPTableCell *cell,
                                                   GHWPParagraph *paragraph);
GHWPTableCell *ghwp_parse_table_cell_attr         (GHWPTableCell *cell,
                                                   GHWPContext   *context);

G_END_DECLS

#endif /* __GHWP_MODELS_H__ */
