/*
 * Copyright 2000 Computing Research Labs, New Mexico State University
 * Copyright 2001 Francesco Zappa Nardelli
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COMPUTING RESEARCH LAB OR NEW MEXICO STATE UNIVERSITY BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
 * OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef __BDF_H__
#define __BDF_H__

/*
 * $Id$
 */

#include <string.h>

#include <ft2build.h>
#include FT_INTERNAL_OBJECTS_H
#include FT_INTERNAL_STREAM_H


FT_BEGIN_HEADER
/* Imported from bdfP.h */

#ifndef MYABS
#define MYABS(xx) ((xx) < 0 ? -(xx) : (xx))
#endif

#define _bdf_glyph_modified(map, e) ((map)[(e) >> 5] & (1 << ((e) & 31)))
#define _bdf_set_glyph_modified(map, e) (map)[(e) >> 5] |= (1 << ((e) & 31))
#define _bdf_clear_glyph_modified(map, e) (map)[(e) >> 5] &= ~(1 << ((e) & 31))

/* end of bdfP.h */

/**************************************************************************
 *
 * BDF font options macros and types.
 *
 **************************************************************************/

#define BDF_UNIX_EOL 1           /* Save fonts with Unix LF.              */
#define BDF_DOS_EOL  2           /* Save fonts with DOS CRLF.             */
#define BDF_MAC_EOL  3           /* Save fonts with Mac CR.               */

#define BDF_CORRECT_METRICS 0x01 /* Correct invalid metrics when loading. */
#define BDF_KEEP_COMMENTS   0x02 /* Preserve the font comments.           */
#define BDF_KEEP_UNENCODED  0x04 /* Keep the unencoded glyphs.            */
#define BDF_PROPORTIONAL    0x08 /* Font has proportional spacing.        */
#define BDF_MONOWIDTH       0x10 /* Font has mono width.                  */
#define BDF_CHARCELL        0x20 /* Font has charcell spacing.            */

#define BDF_ALL_SPACING (BDF_PROPORTIONAL|BDF_MONOWIDTH|BDF_CHARCELL)

#define BDF_DEFAULT_LOAD_OPTIONS \
    (BDF_CORRECT_METRICS|BDF_KEEP_COMMENTS|BDF_KEEP_UNENCODED|BDF_PROPORTIONAL)

typedef struct {
    int ttf_hint;
    int correct_metrics;
    int keep_unencoded;
    int keep_comments;
    int pad_cells;
    int font_spacing;
    long point_size;
    unsigned long resolution_x;
    unsigned long resolution_y;
    int bits_per_pixel;
    int eol;
} bdf_options_t;

/*
 * Callback function type for unknown configuration options.
 */
typedef int (*bdf_options_callback_t) (bdf_options_t *opts,
                                          char **params,
                                          unsigned long nparams,
                                          void *client_data);

/**************************************************************************
 *
 * BDF font property macros and types.
 *
 **************************************************************************/

#define BDF_ATOM     1
#define BDF_INTEGER  2
#define BDF_CARDINAL 3

/*
 * This structure represents a particular property of a font.
 * There are a set of defaults and each font has their own.
 */
typedef struct {
    char *name;         /* Name of the property.                        */
    int format;         /* Format of the property.                      */
    int builtin;        /* A builtin property.                          */
    union {
        char *atom;
        long int32;
        unsigned long card32;
    } value;            /* Value of the property.                       */
} bdf_property_t;

/**************************************************************************
 *
 * BDF font metric and glyph types.
 *
 **************************************************************************/

/*
 * A general bitmap type, mostly used when the glyph bitmap is being edited.
 */
typedef struct {
    short x;
    short y;
    unsigned short width;
    unsigned short height;
    unsigned short bpp;
    unsigned short pad;
    unsigned char *bitmap;
    unsigned long bytes;
} bdf_bitmap_t;

typedef struct {
    int font_spacing;
    unsigned short swidth;
    unsigned short dwidth;
    unsigned short width;
    unsigned short height;
    short x_offset;
    short y_offset;
    short ascent;
    short descent;
} bdf_metrics_t;

typedef struct {
    unsigned short width;
    unsigned short height;
    short x_offset;
    short y_offset;
    short ascent;
    short descent;
} bdf_bbx_t;

typedef struct {
    char *name;                 /* Glyph name.                          */
    long encoding;              /* Glyph encoding.                      */
    unsigned short swidth;      /* Scalable width.                      */
    unsigned short dwidth;      /* Device width.                        */
    bdf_bbx_t bbx;              /* Glyph bounding box.                  */
    unsigned char *bitmap;      /* Glyph bitmap.                        */
    unsigned short bytes;       /* Number of bytes used for the bitmap. */
} bdf_glyph_t;

typedef struct {
    char *key;
    void *data;
} _hashnode, *hashnode;

typedef struct {
    int limit;
    int size;
    int used;
    hashnode *table;
} hashtable;

typedef struct {
    unsigned short pad;         /* Pad to 4-byte boundary.              */
    unsigned short bpp;         /* Bits per pixel.                      */
    long start;                 /* Beginning encoding value of glyphs.  */
    long end;                   /* Ending encoding value of glyphs.     */
    bdf_glyph_t *glyphs;        /* Glyphs themselves.                   */
    unsigned long glyphs_size;  /* Glyph structures allocated.          */
    unsigned long glyphs_used;  /* Glyph structures used.               */
    bdf_bbx_t bbx;              /* Overall bounding box of glyphs.      */
} bdf_glyphlist_t;

typedef struct {
    char *name;                 /* Name of the font.                     */
    bdf_bbx_t bbx;              /* Font bounding box.                    */

    long point_size;            /* Point size of the font.               */
    unsigned long resolution_x; /* Font horizontal resolution.           */
    unsigned long resolution_y; /* Font vertical resolution.             */

    int hbf;                    /* Font came from an HBF font.           */

    int spacing;                /* Font spacing value.                   */

    unsigned short monowidth;   /* Logical width for monowidth font.     */

    long default_glyph;         /* Encoding of the default glyph.        */

    long font_ascent;           /* Font ascent.                          */
    long font_descent;          /* Font descent.                         */

    long glyphs_size;           /* Glyph structures allocated.           */
    long glyphs_used;           /* Glyph structures used.                */
    bdf_glyph_t *glyphs;        /* Glyphs themselves.                    */

    long unencoded_size;        /* Unencoded glyph structures allocated. */
    long unencoded_used;        /* Unencoded glyph structures used.      */
    bdf_glyph_t *unencoded;     /* Unencoded glyphs themselves.          */

    unsigned long props_size;   /* Font properties allocated.            */
    unsigned long props_used;   /* Font properties used.                 */
    bdf_property_t *props;      /* Font properties themselves.           */

    char *comments;             /* Font comments.                        */
    unsigned long comments_len; /* Length of comment string.             */

    char *acmsgs;               /* Auto-correction messages.             */
    unsigned long acmsgs_len;   /* Length of auto-correction messages.   */

    bdf_glyphlist_t overflow;   /* Storage used for glyph insertion.     */

    void *internal;             /* Internal data for the font.           */

    unsigned long nmod[2048];   /* Bitmap indicating modified glyphs.    */
    unsigned long umod[2048];   /* Bitmap indicating modified unencoded. */

    unsigned short modified;    /* Boolean indicating font modified.     */
    unsigned short bpp;         /* Bits per pixel.                       */

  FT_Memory memory;
  bdf_property_t *user_props;
  unsigned long nuser_props;
  hashtable proptbl;

} bdf_font_t;


/**************************************************************************
 *
 * Types for load/save callbacks.
 *
 **************************************************************************/

/*
 * Callback reasons.
 */
#define BDF_LOAD_START       1
#define BDF_LOADING          2
#define BDF_SAVE_START       3
#define BDF_SAVING           4
#define BDF_TRANSLATE_START  5
#define BDF_TRANSLATING      6
#define BDF_ROTATE_START     7
#define BDF_ROTATING         8
#define BDF_SHEAR_START      9
#define BDF_SHEARING         10
#define BDF_GLYPH_NAME_START 11
#define BDF_GLYPH_NAME       12
#define BDF_EXPORT_START     13
#define BDF_EXPORTING        14
#define BDF_EMBOLDEN_START   15
#define BDF_EMBOLDENING      16
#define BDF_WARNING          20
#define BDF_ERROR            21

/*
 * Error codes.
 */
#define BDF_OK                 0
#define BDF_MISSING_START     -1
#define BDF_MISSING_FONTNAME  -2
#define BDF_MISSING_SIZE      -3
#define BDF_MISSING_FONTBBX   -4
#define BDF_MISSING_CHARS     -5
#define BDF_MISSING_STARTCHAR -6
#define BDF_MISSING_ENCODING  -7
#define BDF_MISSING_BBX       -8

#define BDF_NOT_CONSOLE_FONT  -10
#define BDF_NOT_MF_FONT       -11
#define BDF_NOT_PSF_FONT      -12

#define BDF_OUT_OF_MEMORY     -20

#define BDF_EMPTY_FONT        -99
#define BDF_INVALID_LINE      -100

typedef struct {
    unsigned long reason;
    unsigned long current;
    unsigned long total;
    unsigned long errlineno;
} bdf_callback_struct_t;

typedef void (*bdf_callback_t) (bdf_callback_struct_t *call_data,
                                   void *client_data);

/**************************************************************************
 *
 * BDF font API.
 *
 **************************************************************************/

/*
 * Startup and shutdown functions are no more needed
 */

/*
 * Font options functions.
 */
/*extern void bdf_default_options (bdf_options_t *opts);*/

/*
 * Font load, create, save and free functions.
 */

FT_LOCAL( bdf_font_t* )  bdf_load_font (FT_Stream stream, FT_Memory memory,
                                        bdf_options_t *opts,
                                        bdf_callback_t callback, void *data);


FT_LOCAL( void )         bdf_free_font (bdf_font_t *font);

/*
 * Font property functions.
 */
/* extern void bdf_create_property (char *name, int type, bdf_font_t *font); */
FT_LOCAL( bdf_property_t* )  bdf_get_property (char *name, bdf_font_t *font);
FT_LOCAL( unsigned long )    bdf_property_list (bdf_property_t **props);

FT_LOCAL( void )             bdf_add_font_property (bdf_font_t *font,
                                                    bdf_property_t *property);
                                                    
FT_LOCAL( void )             bdf_delete_font_property (bdf_font_t *font, char *name);

FT_LOCAL( bdf_property_t* )  bdf_get_font_property (bdf_font_t *font,
                                                    char *name);
                                                    
FT_LOCAL( unsigned long )    bdf_font_property_list (bdf_font_t *font,
                                                      bdf_property_t **props);

/*
 * Font comment functions.
 */
FT_LOCAL( int )              bdf_replace_comments (bdf_font_t *font, char *comments,
                                                   unsigned long comments_len);

/*
 * Other miscellaneous functions.
 */
FT_LOCAL( void )             bdf_set_default_metrics (bdf_font_t *font);

/* */

FT_END_HEADER

#endif /* _h_bdf */
