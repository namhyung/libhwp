/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ghwp-models.c
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

#include <glib/gprintf.h>

#include "ghwp-models.h"
#include "ghwp-parse.h"

#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))
#define _g_free0(var) (var = (g_free (var), NULL))

void ghwp_parse_common_object (GHWPObject *obj, GHWPContext *ctx)
{
    /* must be called after reading ctrl id */
    context_read_uint32 (ctx, &obj->attr);
    context_read_hwp_unit (ctx, &obj->v_offset);
    context_read_hwp_unit (ctx, &obj->h_offset);
    context_read_hwp_unit (ctx, &obj->width);
    context_read_hwp_unit (ctx, &obj->height);
    context_read_int32 (ctx, &obj->z_order);
    context_read_hwp_unit16 (ctx, &obj->l_spacing);
    context_read_hwp_unit16 (ctx, &obj->r_spacing);
    context_read_hwp_unit16 (ctx, &obj->t_spacing);
    context_read_hwp_unit16 (ctx, &obj->b_spacing);
    context_read_uint32 (ctx, &obj->instance_id);
    context_read_int32 (ctx, &obj->page_split);
    context_read_uint16 (ctx, &obj->n_desc);

    gunichar2 ch; /* guint16 */
    GString  *text = g_string_new ("");
    guint     i;

    for (i = 0; i < obj->n_desc; i++)
    {
        context_read_uint16 (ctx, &ch);
        g_string_append_unichar (text, ch);
    }

    obj->desc = g_string_free (text, FALSE);
}

void ghwp_parse_list_header (GHWPListHeader *hdr, GHWPContext *ctx)
{
    g_return_if_fail (ctx != NULL);

    context_read_int16 (ctx, &hdr->n_paragraphs);
    context_read_uint32 (ctx, &hdr->attr);
    context_read_int16 (ctx, &hdr->unknown);
}

/** GHWPText *****************************************************************/

G_DEFINE_TYPE (GHWPText, ghwp_text, G_TYPE_OBJECT);

GHWPText *ghwp_text_new (const gchar *text)
{
    g_return_val_if_fail (text != NULL, NULL);
    GHWPText *ghwp_text = (GHWPText *) g_object_new (GHWP_TYPE_TEXT, NULL);
    ghwp_text->text = g_strdup (text);
    return ghwp_text;
}

GHWPText *ghwp_text_append (GHWPText *ghwp_text, const gchar *text)
{
    g_return_val_if_fail (ghwp_text != NULL, NULL);

    gchar *tmp;
    tmp = g_strdup (ghwp_text->text);
    g_free (ghwp_text->text);
    ghwp_text->text = g_strconcat (tmp, text, NULL);
    g_free (tmp);
    return ghwp_text;
}

static void ghwp_text_finalize (GObject *obj)
{
    GHWPText *ghwp_text = GHWP_TEXT(obj);
    _g_free0 (ghwp_text->text);
    G_OBJECT_CLASS (ghwp_text_parent_class)->finalize (obj);
}

static void ghwp_text_class_init (GHWPTextClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    object_class->finalize     = ghwp_text_finalize;
}

static void ghwp_text_init (GHWPText *ghwp_text)
{
}

/** GHWPParagraph ************************************************************/

G_DEFINE_TYPE (GHWPParagraph, ghwp_paragraph, G_TYPE_OBJECT);

GHWPParagraph *ghwp_paragraph_new (void)
{
    return (GHWPParagraph *) g_object_new (GHWP_TYPE_PARAGRAPH, NULL);
}

static void ghwp_paragraph_finalize (GObject *obj)
{
    GHWPParagraph *paragraph = GHWP_PARAGRAPH (obj);

    _g_object_unref0 (paragraph->ghwp_text);
    _g_object_unref0 (paragraph->table);

    g_array_unref (paragraph->char_shapes);
    g_array_unref (paragraph->range_tags);
    g_array_unref (paragraph->line_segs);
    G_OBJECT_CLASS (ghwp_paragraph_parent_class)->finalize (obj);
}

static void ghwp_paragraph_class_init (GHWPParagraphClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    object_class->finalize     = ghwp_paragraph_finalize;
}

static void ghwp_paragraph_init (GHWPParagraph *paragraph)
{
    paragraph->ghwp_text = NULL;
    paragraph->table     = NULL;
}

void
ghwp_paragraph_set_ghwp_text (GHWPParagraph *paragraph, GHWPText *ghwp_text)
{
    g_return_if_fail (paragraph != NULL);
    g_return_if_fail (ghwp_text != NULL);
    paragraph->ghwp_text = ghwp_text;
}

GHWPText *ghwp_paragraph_get_ghwp_text (GHWPParagraph *paragraph)
{
    g_return_val_if_fail (paragraph != NULL, NULL);
    return paragraph->ghwp_text;
}

void ghwp_paragraph_set_table (GHWPParagraph *paragraph, GHWPTable *table)
{
    g_return_if_fail (paragraph != NULL);
    g_return_if_fail (table     != NULL);
    paragraph->table = table;
}

void ghwp_paragraph_add_link (GHWPParagraph *paragraph,
                              GHWPParagraph *link,
                              gint line)
{
    g_return_if_fail (paragraph != NULL);
    g_return_if_fail (link      != NULL);

    memcpy (&link->header, &paragraph->header, sizeof (paragraph->header));

    if (paragraph->ghwp_text)
        link->ghwp_text = g_object_ref (paragraph->ghwp_text);
    if (paragraph->table)
        link->table = g_object_ref (paragraph->table);

    link->char_shapes = g_array_ref (paragraph->char_shapes);
    link->range_tags  = g_array_ref (paragraph->range_tags);
    link->line_segs   = g_array_ref (paragraph->line_segs);

    /* XXX only support a link between 2 pages */
    paragraph->link = link;
    link->link = paragraph;

    paragraph->line_start = 0;
    paragraph->line_end   = line;

    link->line_start = line;
    link->line_end   = link->header.n_line_segs;
}

GHWPTable *ghwp_paragraph_get_table (GHWPParagraph *paragraph)
{
    g_return_val_if_fail (paragraph != NULL, NULL);
    return paragraph->table;
}

void ghwp_parse_paragraph_header (GHWPParagraph *paragraph,
                                  GHWPContext *ctx)
{
    g_return_if_fail (paragraph != NULL);

    context_read_uint32 (ctx, &paragraph->header.n_chars);
    context_read_uint32 (ctx, &paragraph->header.control_mask);
    context_read_uint16 (ctx, &paragraph->header.para_shape_id);
    context_read_uint8  (ctx, &paragraph->header.para_style_id);
    context_read_uint8  (ctx, &paragraph->header.col_split);
    context_read_uint16 (ctx, &paragraph->header.n_char_shapes);
    context_read_uint16 (ctx, &paragraph->header.n_range_tags);
    context_read_uint16 (ctx, &paragraph->header.n_line_segs);
    context_read_uint32 (ctx, &paragraph->header.para_id);

    if (context_check_version(ctx, 5, 0, 3, 2)) {
        context_read_uint16 (ctx, &paragraph->header.history_merge);
    }

    paragraph->char_shapes = g_array_sized_new (TRUE, TRUE, sizeof (GHWPCharShapeRef *),
                                                paragraph->header.n_char_shapes);
    paragraph->range_tags  = g_array_sized_new (TRUE, TRUE, sizeof (GHWPRangeTag *),
                                                paragraph->header.n_range_tags);
    paragraph->line_segs   = g_array_sized_new (TRUE, TRUE, sizeof (GHWPLineSeg *),
                                                paragraph->header.n_line_segs);

    paragraph->line_start = 0;
    paragraph->line_end = paragraph->header.n_line_segs;
}

void ghwp_parse_paragraph_char_shape (GHWPParagraph *paragraph,
                                      GHWPContext *ctx)
{
    guint i;

    g_return_if_fail (paragraph != NULL);

    for (i = 0; i < paragraph->header.n_char_shapes; i++) {
        GHWPCharShapeRef *char_shape = malloc (sizeof (*char_shape));

        context_read_uint32 (ctx, &char_shape->pos);
        context_read_uint32 (ctx, &char_shape->id);

        g_array_insert_val (paragraph->char_shapes, i, char_shape);
    }
}

void ghwp_parse_paragraph_line_seg (GHWPParagraph *paragraph,
                                    GHWPContext *ctx)
{
    guint i;

    g_return_if_fail (paragraph != NULL);

    for (i = 0; i < paragraph->header.n_line_segs; i++) {
        GHWPLineSeg *line_seg = malloc (sizeof (*line_seg));

        context_read_uint32 (ctx, &line_seg->text_start);
        context_read_int32  (ctx, &line_seg->v_pos);
        context_read_int32  (ctx, &line_seg->line_height);
        context_read_int32  (ctx, &line_seg->text_height);
        context_read_int32  (ctx, &line_seg->base_line);
        context_read_int32  (ctx, &line_seg->line_spacing);
        context_read_int32  (ctx, &line_seg->col_offset);
        context_read_int32  (ctx, &line_seg->segment_width);
        context_read_uint32 (ctx, &line_seg->tag);

        g_array_insert_val (paragraph->line_segs, i, line_seg);
    }
}

void ghwp_parse_paragraph_range_tag (GHWPParagraph *paragraph,
                                     GHWPContext *ctx)
{
    guint i;

    g_return_if_fail (paragraph != NULL);

    for (i = 0; i < paragraph->header.n_char_shapes; i++) {
        GHWPRangeTag *range_tag = malloc (sizeof (*range_tag));

        context_read_uint32 (ctx, &range_tag->start);
        context_read_uint32 (ctx, &range_tag->end);
        context_read_uint32 (ctx, &range_tag->tag);

        g_array_insert_val (paragraph->range_tags, i, range_tag);
    }
}

/** GHWPTable ****************************************************************/

G_DEFINE_TYPE (GHWPTable, ghwp_table, G_TYPE_OBJECT);

GHWPTable *ghwp_table_new (void)
{
    return (GHWPTable *) g_object_new (GHWP_TYPE_TABLE, NULL);
}

#include <stdio.h>

void hexdump(guint8 *data, guint16 data_len)
{
    int i = 0;

    printf("data_len = %d\n", data_len);
    printf("-----------------------------------------------\n");
    printf("00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15\n");
    printf("-----------------------------------------------\n");

    for (i = 0; i < data_len; i++) {
        if ( (i != 0) && (i % 16 == 0))
           printf("\n");
        printf("%02x ", data[i]);
    }
    printf("\n-----------------------------------------------\n");
}

void ghwp_parse_table_attr (GHWPTable *table, GHWPContext *context)
{
    g_return_if_fail (context != NULL);
    int        i;

    context_read_uint32 (context, &table->flags);
    context_read_uint16 (context, &table->n_rows);
    context_read_uint16 (context, &table->n_cols);
    context_read_hwp_unit16 (context, &table->cell_spacing);
    context_read_hwp_unit16 (context, &table->l_margin);
    context_read_hwp_unit16 (context, &table->r_margin);
    context_read_hwp_unit16 (context, &table->t_margin);
    context_read_hwp_unit16 (context, &table->b_margin);

    table->row_sizes = g_malloc0_n (table->n_rows, 2);

    for (i = 0; i < table->n_rows; i++) {
        context_read_uint16 (context, &(table->row_sizes[i]));
    }

    context_read_uint16 (context, &table->border_fill_id);

    if (context_check_version (context, 5, 0, 1, 0)) {
        context_read_uint16 (context, &table->valid_zone_info_size);

        table->zones = g_malloc0_n (table->valid_zone_info_size, 2);

        for (i = 0; i < table->valid_zone_info_size; i++) {
            context_read_uint16 (context, &(table->zones[i]));
        }
    }

    if (context->data_count != context->data_len) {
        g_warning ("%s:%d: table size mismatch\n", __FILE__, __LINE__);
    }
}

static void
ghwp_table_init (GHWPTable *table)
{
    table->cells = g_array_new (TRUE, TRUE, sizeof (GHWPTableCell *));
}

static void
ghwp_table_finalize (GObject *object)
{
    GHWPTable *table = GHWP_TABLE(object);
    _g_free0 (table->row_sizes);
    _g_free0 (table->zones);
    g_array_free (table->cells, TRUE);
    G_OBJECT_CLASS (ghwp_table_parent_class)->finalize (object);
}

static void
ghwp_table_class_init (GHWPTableClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    object_class->finalize     = ghwp_table_finalize;
}

GHWPTableCell *ghwp_table_get_last_cell (GHWPTable *table)
{
    g_return_val_if_fail (table != NULL, NULL);
    return g_array_index (table->cells, GHWPTableCell *,
                          table->cells->len - 1);
}

void ghwp_table_add_cell (GHWPTable *table, GHWPTableCell *cell)
{
    g_return_if_fail (table != NULL);
    g_return_if_fail (cell  != NULL);
    g_array_append_val (table->cells, cell);
}

/** GHWPTableCell ************************************************************/

G_DEFINE_TYPE (GHWPTableCell, ghwp_table_cell, G_TYPE_OBJECT);

static void
ghwp_table_cell_init (GHWPTableCell *cell)
{
    cell->paragraphs = g_array_new (TRUE, TRUE, sizeof (GHWPParagraph *));
}

static void
ghwp_table_cell_finalize (GObject *object)
{
    GHWPTableCell *cell = GHWP_TABLE_CELL(object);
    g_array_free (cell->paragraphs, TRUE);
    G_OBJECT_CLASS (ghwp_table_cell_parent_class)->finalize (object);
}

static void
ghwp_table_cell_class_init (GHWPTableCellClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    object_class->finalize     = ghwp_table_cell_finalize;
}

GHWPTableCell *ghwp_table_cell_new (void)
{
    return (GHWPTableCell *) g_object_new (GHWP_TYPE_TABLE_CELL, NULL);
}

GHWPTableCell *ghwp_parse_table_cell_attr (GHWPTableCell *table_cell,
                                           GHWPContext *context)
{
    g_return_val_if_fail (context != NULL, NULL);

    /* 표 75 */
    context_read_uint16 (context, &table_cell->col_addr);
    context_read_uint16 (context, &table_cell->row_addr);
    context_read_uint16 (context, &table_cell->col_span);
    context_read_uint16 (context, &table_cell->row_span);

    context_read_hwp_unit (context, &table_cell->width);
    context_read_hwp_unit (context, &table_cell->height);

    context_read_hwp_unit16 (context, &table_cell->l_margin);
    context_read_hwp_unit16 (context, &table_cell->r_margin);
    context_read_hwp_unit16 (context, &table_cell->t_margin);
    context_read_hwp_unit16 (context, &table_cell->b_margin);

    context_read_uint16 (context, &table_cell->border_fill_id);

    if (context->data_count != context->data_len) {
        //g_printf ("%s:%d: table cell size mismatch\n", __FILE__, __LINE__);
    }
    return table_cell;
}

GHWPParagraph *ghwp_table_cell_get_last_paragraph (GHWPTableCell *cell)
{
    g_return_val_if_fail (cell != NULL, NULL);
    return g_array_index (cell->paragraphs, GHWPParagraph *,
                          cell->paragraphs->len - 1);
}

void
ghwp_table_cell_add_paragraph (GHWPTableCell *cell, GHWPParagraph *paragraph)
{
    g_return_if_fail (cell       != NULL);
    g_return_if_fail (paragraph  != NULL);
    g_array_append_val (cell->paragraphs, paragraph);
}
