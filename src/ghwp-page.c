/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ghwp-page.c
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

#include "ghwp-page.h"

G_DEFINE_TYPE (GHWPPage, ghwp_page, G_TYPE_OBJECT);

#define _g_free0(var) (var = (g_free (var), NULL))

void ghwp_page_get_size (GHWPPage *page,
                         gdouble  *width,
                         gdouble  *height)
{
    g_return_if_fail (page != NULL);

    *width  = page->section->page_info.h_size / GHWP_UPP;
    *height = page->section->page_info.v_size / GHWP_UPP;
}

void ghwp_page_set_section(GHWPPage    *page,
                           GHWPSection *sec)
{
    g_return_if_fail (page != NULL);

    page->section = sec;
}

void ghwp_page_add_paragraph(GHWPPage      *page,
                             GHWPParagraph *para)
{
    g_return_if_fail (page != NULL);

    g_array_append_val (page->paragraphs, para);
}

#include <cairo-ft.h>
#include <ft2build.h>
#include FT_FREETYPE_H

static FT_Library ft_lib;
static FT_Face    ft_face;
/*한 번만 초기화, 로드 */
static void
once_ft_init_and_new (void)
{
    static gsize ft_init = 0;

    if (g_once_init_enter (&ft_init)) {

        FT_Init_FreeType (&ft_lib);
        FT_New_Face (ft_lib, "/usr/share/fonts/truetype/nanum/NanumGothic.ttf",
                     0, &ft_face);

        g_once_init_leave (&ft_init, (gsize)1);
    }
}

static void draw_text_line (cairo_t             *cr,
                            cairo_scaled_font_t *font,
                            double               x,
                            double               y,
                            const gchar         *text,
                            gint                 start,
                            gint                 end)
{
    gchar *str;
    int    num_glyphs;
    cairo_glyph_t *glyphs = NULL; /* NULL로 지정하면 자동 할당됨 */

    if (start >= end)
        return;

    str = g_utf8_substring (text, start, end);

    cairo_scaled_font_text_to_glyphs (font, x, y, str, -1,
                                      &glyphs, &num_glyphs,
                                      NULL, NULL, NULL);
    cairo_show_glyphs (cr, glyphs, num_glyphs);

    cairo_glyph_free (glyphs);
    _g_free0 (str);
}

gboolean ghwp_page_render (GHWPPage *page, cairo_t *cr)
{
    g_return_val_if_fail (page != NULL, FALSE);
    g_return_val_if_fail (cr   != NULL, FALSE);
    cairo_save (cr);

    guint          i, j, k;
    GHWPParagraph *paragraph;
    GHWPText      *ghwp_text;
    GHWPTable     *table;
    GHWPTableCell *cell;
    GHWPPageDef   *page_info;
    GHWPLineSeg   *line;

    cairo_scaled_font_t  *scaled_font;
    cairo_font_face_t    *font_face;
    cairo_matrix_t        font_matrix;
    cairo_matrix_t        ctm;
    cairo_font_options_t *font_options;

    double x = 20.0;
    double y = 40.0;

    /* create scaled font */
    once_ft_init_and_new(); /*한 번만 초기화, 로드*/
    font_face = cairo_ft_font_face_create_for_ft_face (ft_face, 0);
    cairo_matrix_init_identity (&font_matrix);
    cairo_matrix_scale (&font_matrix, 12.0, 12.0);
    cairo_get_matrix (cr, &ctm);
    font_options = cairo_font_options_create ();
    cairo_get_font_options (cr, font_options);
    scaled_font = cairo_scaled_font_create (font_face,
                                           &font_matrix, &ctm, font_options);
    cairo_font_options_destroy (font_options);

    cairo_set_scaled_font(cr, scaled_font); /* 요 문장 없으면 fault 떨어짐 */
    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);

    page_info = &page->section->page_info;

    for (i = 0; i < page->paragraphs->len; i++) {
        paragraph = g_array_index (page->paragraphs, GHWPParagraph *, i);
        ghwp_text = paragraph->ghwp_text;

        /* draw text */
        if ((ghwp_text != NULL) && !(g_str_equal(ghwp_text->text, "\n\r"))) {
            for (j = paragraph->line_start; j < paragraph->line_end; j++) {
                gint text_end;
                line = g_array_index (paragraph->line_segs, GHWPLineSeg *, j);

                if (j == paragraph->header.n_line_segs - 1)
                    text_end = g_utf8_strlen(ghwp_text->text, -1);
                else {
                    GHWPLineSeg *next_line = g_array_index (paragraph->line_segs,
                                                            GHWPLineSeg *, j + 1);
                    text_end = next_line->text_start;
                }

                draw_text_line(cr, scaled_font, (page_info->l_margin + line->col_offset) / GHWP_UPP,
                               (page_info->t_margin + page_info->header + line->v_pos) / GHWP_UPP,
                               ghwp_text->text, line->text_start, text_end);
            }
        }
        /* draw table */
        table = ghwp_paragraph_get_table (paragraph);
        if (table != NULL) {
            line = g_array_index (paragraph->line_segs, GHWPLineSeg *, 0);

            x = page_info->l_margin;
            y = page_info->t_margin + page_info->header + line->v_pos;

            cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
            cairo_set_line_width (cr, 0.3);
            cairo_rectangle (cr, x / GHWP_UPP, y / GHWP_UPP, table->obj.width / GHWP_UPP,
                             table->obj.height / GHWP_UPP);
            cairo_stroke (cr);

            guint l;
            guint cell_width = 0;

            for (j = 0; j < table->cells->len; j++) {
                cell = g_array_index(table->cells, GHWPTableCell *, j);

                if (cell->col_addr == 0) {
                    x = page_info->l_margin;

                    if (j != 0)  /* FIXME */
                        y += table->obj.height / table->n_rows;
                } else {
                    x += cell_width;
                }

                cairo_set_line_width (cr, 0.2);
                cairo_rectangle (cr, x / GHWP_UPP, y / GHWP_UPP, cell_width / GHWP_UPP,
                                 (table->obj.height / table->n_rows) / GHWP_UPP);
                cairo_stroke (cr);

                for (k = 0; k < cell->paragraphs->len; k++) {
                    paragraph = g_array_index(cell->paragraphs,
                                              GHWPParagraph *, k);
                    if (paragraph->ghwp_text) {
                        for (l = paragraph->line_start; l < paragraph->line_end; l++) {
                            gint text_end;

                            ghwp_text = paragraph->ghwp_text;
                            line = g_array_index (paragraph->line_segs, GHWPLineSeg *, l);

                            if (l == paragraph->header.n_line_segs - 1) {
                                text_end = g_utf8_strlen(ghwp_text->text, -1);
                            } else {
                                GHWPLineSeg *next_line = g_array_index (paragraph->line_segs,
                                                                        GHWPLineSeg *, l + 1);
                                text_end = next_line->text_start;
                            }

                            draw_text_line(cr, scaled_font, (x + cell->l_margin) / GHWP_UPP,
                                           (y + cell->t_margin + line->line_height) / GHWP_UPP,
                                           ghwp_text->text, line->text_start, text_end);
                        }
                    }
                }

                cell_width  = cell->width;
            }
        }
    }

    cairo_scaled_font_destroy (scaled_font);

    cairo_restore (cr);
    return TRUE;
}

GHWPPage *ghwp_page_new (void)
{
    return (GHWPPage *) g_object_new (GHWP_TYPE_PAGE, NULL);
}

static void ghwp_page_finalize (GObject *obj)
{
    GHWPPage *page = GHWP_PAGE(obj);
    g_array_free (page->paragraphs, TRUE);
    G_OBJECT_CLASS (ghwp_page_parent_class)->finalize (obj);
}

static void ghwp_page_class_init (GHWPPageClass * klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    object_class->finalize     = ghwp_page_finalize;
}

static void ghwp_page_init (GHWPPage *page)
{
    page->paragraphs = g_array_new (TRUE, TRUE, sizeof (GHWPParagraph *));
}

/* experimental */
void
ghwp_page_render_selection (GHWPPage           *page,
                            cairo_t            *cr,
                            GHWPRectangle      *selection,
                            GHWPRectangle      *old_selection,
                            GHWPSelectionStyle  style, 
                            GHWPColor          *glyph_color,
                            GHWPColor          *background_color)
{
    g_return_if_fail (page != NULL);
    /* TODO */
}

/* experimental */
char *
ghwp_page_get_selected_text (GHWPPage          *page,
                             GHWPSelectionStyle style,
                             GHWPRectangle     *selection)
{
    g_return_val_if_fail (page != NULL, NULL);
    /* TODO */
    return NULL;
}

/* experimental */
GList *
ghwp_page_get_selection_region (GHWPPage          *page,
                                gdouble            scale,
                                GHWPSelectionStyle style,
                                GHWPRectangle     *selection)
{
    g_return_val_if_fail (page != NULL, NULL);
    /* TODO */
    return NULL;
}

/**
 * ghwp_rectangle_free:
 * @rectangle: a #GHWPRectangle
 *
 * Frees the given #GHWPRectangle
 */
void
ghwp_rectangle_free (GHWPRectangle *rectangle)
{
    g_return_if_fail (rectangle != NULL);
    g_slice_free (GHWPRectangle, rectangle);
}
