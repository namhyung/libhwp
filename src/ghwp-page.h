/*
 * ghwp-page.h
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

#ifndef __GHWP_PAGE_H__
#define __GHWP_PAGE_H__

#include <glib-object.h>

#include <cairo.h>
#include "ghwp.h"

G_BEGIN_DECLS

#define GHWP_TYPE_PAGE             (ghwp_page_get_type ())
#define GHWP_PAGE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GHWP_TYPE_PAGE, GHWPPage))
#define GHWP_PAGE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GHWP_TYPE_PAGE, GHWPPageClass))
#define GHWP_IS_PAGE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GHWP_TYPE_PAGE))
#define GHWP_IS_PAGE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GHWP_TYPE_PAGE))
#define GHWP_PAGE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GHWP_TYPE_PAGE, GHWPPageClass))

typedef struct _GHWPPageClass   GHWPPageClass;

struct _GHWPPage
{
    GObject       parent_instance;
    GArray       *paragraphs;
    GHWPSection  *section;
};

struct _GHWPPageClass
{
    GObjectClass parent_class;
};

GType     ghwp_page_get_type      (void) G_GNUC_CONST;
GHWPPage *ghwp_page_new           (void);
void      ghwp_page_get_size      (GHWPPage *page,
                                   gdouble  *width,
                                   gdouble  *height);
void      ghwp_page_set_section   (GHWPPage    *page,
                                   GHWPSection *sec);
void      ghwp_page_add_paragraph (GHWPPage      *page,
                                   GHWPParagraph *para);

gboolean  ghwp_page_render     (GHWPPage *page, cairo_t *cr);
/* experimental */
void
ghwp_page_render_selection     (GHWPPage           *page,
                                cairo_t            *cr,
                                GHWPRectangle      *selection,
                                GHWPRectangle      *old_selection,
                                GHWPSelectionStyle  style, 
                                GHWPColor          *glyph_color,
                                GHWPColor          *background_color);
char *
ghwp_page_get_selected_text    (GHWPPage          *page,
                                GHWPSelectionStyle style,
                                GHWPRectangle     *selection);
GList *
ghwp_page_get_selection_region (GHWPPage          *page,
                                gdouble            scale,
                                GHWPSelectionStyle style,
                                GHWPRectangle     *selection);
void
ghwp_rectangle_free            (GHWPRectangle     *rectangle);

struct _GHWPColor
{
    guint16 red;
    guint16 green;
    guint16 blue;
};

struct _GHWPRectangle
{
    gdouble x1;
    gdouble y1;
    gdouble x2;
    gdouble y2;
};

G_END_DECLS

#endif /* __GHWP_PAGE_H__ */
