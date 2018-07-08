/*
 * ghwp.h
 *
 * Copyright (C) 2012  Hodong Kim <cogniti@gmail.com>
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

#ifndef __GHWP_H__
#define __GHWP_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define GHWP_UPI  7200   /* units  per inch */
#define GHWP_PPU  100.0  /* pixels per unit */

typedef guint32  ghwp_unit;
typedef guint16  ghwp_unit16;

GQuark ghwp_error_quark (void);

#define GHWP_ERROR ghwp_error_quark ()

/**
 * GHWPError:
 * @GHWP_ERROR_INVALID: Generic error when a document opration fails
 * @GHWP_ERROR_ENCRYPTED: Document is encrypted
 * @GHWP_ERROR_OPEN_FILE: File could not be opened for writing when saving document
 * @GHWP_ERROR_DAMAGED: Document is damaged
 *
 * Error codes returned by #GHWPDocument
 *
 * Since: 0.1.2
 */
typedef enum
{
    GHWP_ERROR_INVALID,
    GHWP_ERROR_ENCRYPTED,
    GHWP_ERROR_OPEN_FILE,
    GHWP_ERROR_DAMAGED
} GHWPError;

/**
 * GHWPSelectionStyle:
 * @GHWP_SELECTION_GLYPH: glyph is the minimum unit for selection
 * @GHWP_SELECTION_WORD: word is the minimum unit for selection
 * @GHWP_SELECTION_LINE: line is the minimum unit for selection
 *
 * Selection styles
 */
typedef enum
{
    GHWP_SELECTION_GLYPH,
    GHWP_SELECTION_WORD,
    GHWP_SELECTION_LINE
} GHWPSelectionStyle;


#define GHWP_TAG_BEGIN                    16
typedef enum
{
    GHWP_TAG_DOCUMENT_PROPERTIES       =  16,
    GHWP_TAG_ID_MAPPINGS               =  17,
    GHWP_TAG_BIN_DATA                  =  18,
    GHWP_TAG_FACE_NAME                 =  19,
    GHWP_TAG_BORDER_FILL               =  20,
    GHWP_TAG_CHAR_SHAPE                =  21,
    GHWP_TAG_TAB_DEF                   =  22,
    GHWP_TAG_NUMBERING                 =  23,
    GHWP_TAG_BULLET                    =  24,
    GHWP_TAG_PARA_SHAPE                =  25,
    GHWP_TAG_STYLE                     =  26,
    GHWP_TAG_DOC_DATA                  =  27,
    GHWP_TAG_DISTRIBUTE_DOC_DATA       =  28,
    GHWP_TAG_RESERVED_29               =  29,
    GHWP_TAG_COMPATIBLE_DOCUMENT       =  30,
    GHWP_TAG_LAYOUT_COMPATIBILITY      =  31,
    GHWP_TAG_DOC_INFO_32               =  32,

    GHWP_TAG_PARA_HEADER               =  66,
    GHWP_TAG_PARA_TEXT                 =  67,
    GHWP_TAG_PARA_CHAR_SHAPE           =  68,
    GHWP_TAG_PARA_LINE_SEG             =  69,
    GHWP_TAG_PARA_RANGE_TAG            =  70,
    GHWP_TAG_CTRL_HEADER               =  71,
    GHWP_TAG_LIST_HEADER               =  72,
    GHWP_TAG_PAGE_DEF                  =  73,
    GHWP_TAG_FOOTNOTE_SHAPE            =  74,
    GHWP_TAG_PAGE_BORDER_FILL          =  75,
    GHWP_TAG_SHAPE_COMPONENT           =  76,
    GHWP_TAG_TABLE                     =  77,
    GHWP_TAG_SHAPE_COMPONENT_LINE      =  78,
    GHWP_TAG_SHAPE_COMPONENT_RECTANGLE =  79,
    GHWP_TAG_SHAPE_COMPONENT_ELLIPSE   =  80,
    GHWP_TAG_SHAPE_COMPONENT_ARC       =  81,
    GHWP_TAG_SHAPE_COMPONENT_POLYGON   =  82,
    GHWP_TAG_SHAPE_COMPONENT_CURVE     =  83,
    GHWP_TAG_SHAPE_COMPONENT_OLE       =  84,
    GHWP_TAG_SHAPE_COMPONENT_PICTURE   =  85,
    GHWP_TAG_SHAPE_COMPONENT_CONTAINER =  86,
    GHWP_TAG_CTRL_DATA                 =  87,
    GHWP_TAG_EQEDIT                    =  88,
    GHWP_TAG_RESERVED_89               =  89,
    GHWP_TAG_SHAPE_COMPONENT_TEXTART   =  90,
    GHWP_TAG_FORM_OBJECT               =  91,
    GHWP_TAG_MEMO_SHAPE                =  92,
    GHWP_TAG_MEMO_LIST                 =  93,
    GHWP_TAG_FORBIDDEN_CHAR            =  94,
    GHWP_TAG_CHART_DATA                =  95,

    GHWP_TAG_SHAPE_COMPONENT_UNKNOWN   = 115
} GHWPTag;

GType ghwp_error_get_type           (void) G_GNUC_CONST;
GType ghwp_selection_style_get_type (void) G_GNUC_CONST;
GType ghwp_tag_get_type             (void) G_GNUC_CONST;
#define GHWP_TYPE_ERROR             (ghwp_error_get_type ())
#define GHWP_TYPE_SELECTION_STYLE   (ghwp_selection_style_get_type ())
#define GHWP_TYPE_TAG               (ghwp_tag_get_type ())

const char  *ghwp_get_version  (void);
const char *_ghwp_get_tag_name (guint tag_id);

typedef struct _GHWPColor     GHWPColor;
typedef struct _GHWPDocument  GHWPDocument;
typedef struct _GHWPFile      GHWPFile;
typedef struct _GHWPFileML    GHWPFileML;
typedef struct _GHWPFileV5    GHWPFileV5;
typedef struct _GHWPFileV3    GHWPFileV3;
typedef struct _GHWPPage      GHWPPage;
typedef struct _GHWPRectangle GHWPRectangle;

G_END_DECLS

#define __GHWP_H_INSIDE__

#include "ghwp-document.h"
#include "ghwp-file.h"
#include "ghwp-models.h"
#include "ghwp-page.h"
#include "ghwp-version.h"
#include "gsf-input-stream.h"

#undef __GHWP_H_INSIDE__

#endif /*__GHWP_H__ */
