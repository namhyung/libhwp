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
#include <fontconfig/fontconfig.h>

static FT_Library  ft_lib;
static FT_Face    *ft_face;

static FcConfig              *fc_config;
static cairo_scaled_font_t  **scaled_fonts;

static void load_font_face (GHWPFontFace *ghwp_face, FT_Face *face)
{
    FcPattern* pat;
    FcChar8* file = NULL;
    FcPattern *font;
    FcResult result;

    pat = FcNameParse ((const FcChar8 *)ghwp_face->name);
    FcConfigSubstitute (fc_config, pat, FcMatchPattern);
    FcDefaultSubstitute (pat);

    font = FcFontMatch (fc_config, pat, &result);
	if (font) {
        if (FcPatternGetString (font, FC_FILE, 0, &file) == FcResultMatch) {
            //we found the font, now load it.
            //This might be a fallback font
            dbg ("loading (%s) %s\n", ghwp_face->name, file);
            FT_New_Face (ft_lib, (const gchar *)file, 0, face);
        }
	}
    FcPatternDestroy (pat);
}

static void scale_cairo_font (GHWPCharShape *shape, cairo_t *cr,
                              cairo_scaled_font_t **font)
{
    FT_Face              *face;
    cairo_font_face_t    *font_face;
    cairo_matrix_t        font_matrix;
    cairo_matrix_t        ctm;
    cairo_font_options_t *font_options;

    face = &ft_face[shape->face_id[CHAR_SHAPE_LANG_KO]];

    font_face = cairo_ft_font_face_create_for_ft_face (*face, 0);
    cairo_matrix_init_identity (&font_matrix);
    cairo_matrix_scale (&font_matrix, shape->def_size / GHWP_UPP,
                        shape->def_size / GHWP_UPP);
    cairo_get_matrix (cr, &ctm);

    font_options = cairo_font_options_create ();
    cairo_get_font_options (cr, font_options);
    *font = cairo_scaled_font_create (font_face, &font_matrix, &ctm, font_options);
    cairo_font_options_destroy (font_options);
}

/*한 번만 초기화, 로드 */
static void
once_ft_init_and_new (GHWPDocument *doc, cairo_t *cr)
{
    static gsize ft_init = 0;

    if (g_once_init_enter (&ft_init)) {
        guint32 i;
        guint32 n_fonts;

        FT_Init_FreeType (&ft_lib);
        fc_config = FcInitLoadConfigAndFonts ();

        n_fonts = doc->info_v5.id_maps.num[ID_KOREAN_FONTS];
        ft_face = calloc (n_fonts, sizeof (*ft_face));

        for (i = 0; i < n_fonts; i++) {
            load_font_face (&doc->info_v5.fonts_korean[i], &ft_face[i]);
        }

        n_fonts = doc->info_v5.id_maps.num[ID_CHAR_SHAPES];
        scaled_fonts = calloc (n_fonts, sizeof (*scaled_fonts));

        for (i = 0; i < n_fonts; i++) {
            scale_cairo_font (&doc->info_v5.char_shapes[i], cr, &scaled_fonts[i]);
        }

        g_once_init_leave (&ft_init, (gsize)1);
    }
}

static gdouble draw_text_line (cairo_t             *cr,
                               cairo_scaled_font_t *font,
                               double               x,
                               double               y,
                               GHWPText            *text,
                               gint                 start,
                               gint                 end)
{
    gchar *str;
    int    num_glyphs;
    cairo_glyph_t *glyphs = NULL; /* NULL로 지정하면 자동 할당됨 */
    cairo_text_extents_t extents;
    GString  *strbuf = g_string_new ("");
    gunichar2 ch;
    gint i;

    if (start >= end)
        return x;

    for (i = start; i < end; i++) {
        ch = text->buf[i];

        if (ch >= GHWP_NUM_CC || ghwp_control_char_type[ch] == GHWP_CC_TYPE_CHAR) {
            g_string_append_unichar (strbuf, ch);
            continue;
        }

        /* TODO: handle control characters if needed */
        i += 7;
    }
    str = g_string_free (strbuf, FALSE);

    cairo_scaled_font_text_to_glyphs (font, x / GHWP_UPP, y / GHWP_UPP, str, -1,
                                      &glyphs, &num_glyphs, NULL, NULL, NULL);
    cairo_show_glyphs (cr, glyphs, num_glyphs);
    cairo_glyph_extents (cr, glyphs, num_glyphs, &extents);

    cairo_glyph_free (glyphs);
    _g_free0 (str);

    return x + extents.x_advance * GHWP_UPP;
}

static void draw_paragraph_texts (cairo_t       *cr,
                                  GHWPDocument  *document,
                                  GHWPParagraph *paragraph,
                                  gdouble        start_x,
                                  gdouble        start_y)
{
    gint      i, k = 0;
    GHWPText *ghwp_text = paragraph->ghwp_text;

    for (i = paragraph->line_start; i < paragraph->line_end; i++) {
        GHWPLineSeg *line = NULL;
        GHWPCharShapeRef *shape_ref = NULL;
        gint text_start;
        gint text_end;
        gint shape_start;
        gint shape_end;
        gdouble x = start_x;
        gdouble y = start_y;

        line  = g_array_index (paragraph->line_segs, GHWPLineSeg *, i);
        text_start = line->text_start;

        if (i == paragraph->line_segs->len - 1) {
            text_end = ghwp_text->n_chars;
        } else {
            GHWPLineSeg *next_line  = g_array_index (paragraph->line_segs,
                                                     GHWPLineSeg *, i + 1);
            text_end = next_line->text_start;
        }

        for (k = paragraph->char_shapes->len - 1; k >=0; k--) {
            shape_ref  = g_array_index (paragraph->char_shapes,
                                        GHWPCharShapeRef *, k);
            if (shape_ref->pos <= text_start)
                break;
        }

        shape_start = text_start;

        do {
            GHWPCharShape *shape;
            cairo_scaled_font_t *font;

            shape = &document->info_v5.char_shapes[shape_ref->id];
            font  = scaled_fonts[shape_ref->id];
            k++;

            if (k == paragraph->char_shapes->len) {
                shape_end = text_end;
            } else {
                shape_ref  = g_array_index (paragraph->char_shapes,
                                            GHWPCharShapeRef *, k);
                shape_end = shape_ref->pos;
            }

            cairo_set_scaled_font(cr, font);
            cairo_set_source_rgb (cr, GHWP_COLOR_R(shape->char_color),
                                  GHWP_COLOR_G(shape->char_color),
                                  GHWP_COLOR_B(shape->char_color));

            x = draw_text_line(cr, font, x + line->col_offset, y + line->v_pos,
                               ghwp_text, shape_start, shape_end);

            shape_start = shape_end;
        } while (shape_end < text_end);
    }
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

    double x = 20.0;
    double y = 40.0;

    /* create scaled font */
    once_ft_init_and_new (page->section->document, cr); /*한 번만 초기화, 로드*/

    page_info = &page->section->page_info;

    for (i = 0; i < page->paragraphs->len; i++) {
        paragraph = g_array_index (page->paragraphs, GHWPParagraph *, i);
        ghwp_text = paragraph->ghwp_text;

        /* draw text */
        if ((ghwp_text != NULL) && !(g_str_equal(ghwp_text->text, "\n\r"))) {
            draw_paragraph_texts (cr, page->section->document, paragraph,
                                  page_info->l_margin,
                                  page_info->t_margin + page_info->header);
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
                        draw_paragraph_texts (cr, page->section->document, paragraph,
                                              x + cell->l_margin,
                                              y + line->line_height - cell->b_margin);
                    }
                }

                cell_width  = cell->width;
            }
        }
    }

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
