/*  bdfdriver.c

    FreeType font driver for bdf files

    Copyright (C) 2001-2002 by
    Francesco Zappa Nardelli 

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <ft2build.h>

#include FT_INTERNAL_DEBUG_H
#include FT_INTERNAL_STREAM_H
#include FT_INTERNAL_OBJECTS_H

#include "bdf.h"
#include "bdfdriver.h"

#include "bdferror.h"


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_bdf


FT_CALLBACK_DEF( FT_Error )
BDF_Face_Done( BDF_Face  face )
{
  FT_Memory  memory = FT_FACE_MEMORY( face );

  bdf_free_font(face->bdffont); 
  
  FT_FREE( face->en_table ); 

  FT_FREE( face->charset_encoding);
  FT_FREE( face->charset_registry);
  FT_FREE( face->root.family_name );
  
  FT_FREE( face->root.available_sizes );
  FT_FREE( face->bdffont );

  FT_TRACE4(("bdf: done face\n"));
  
  return FT_Err_Ok;
}


FT_CALLBACK_DEF( FT_Error )
BDF_Face_Init( FT_Stream      stream,
               BDF_Face       face,
               FT_Int         face_index,
               FT_Int         num_params,
               FT_Parameter*  params )
{
  FT_Error       error  = FT_Err_Ok;
  FT_Memory      memory = FT_FACE_MEMORY( face );
  bdf_font_t*    font;
  bdf_options_t  options;

  FT_UNUSED( num_params );
  FT_UNUSED( params );
  FT_UNUSED( face_index );

  (void) FT_STREAM_SEEK( 0 );

  options.correct_metrics = 1;   /* FZ XXX : options semantics */
  options.keep_unencoded  = 1;
  options.pad_cells       = 1;

  font = bdf_load_font( stream, memory, &options, 0, 0 );
  if ( font == NULL )
  {
    FT_TRACE2(("[not a valid BDF file]\n"));
    goto Fail;
  }

  /* we have a bdf font: let's construct the face object */
  face->bdffont = font;
  {
    FT_Face          root = FT_FACE( face );
    bdf_property_t*  prop = NULL;

    FT_TRACE4(("glyph %d - %d, unencoded %d %d\n",font->glyphs_size,
               font->glyphs_used, font->unencoded_size, font->unencoded_used));


    root->num_faces  = 1;
    root->face_index = 0;
    root->face_flags =  FT_FACE_FLAG_FIXED_SIZES |
                        FT_FACE_FLAG_HORIZONTAL  |
                        FT_FACE_FLAG_FAST_GLYPHS ;

    prop = bdf_get_font_property (font,"SPACING");
    
    if ( prop && prop->format == BDF_ATOM )
    { 
      if ( (*(prop->value.atom) == 'M') ||
           (*(prop->value.atom) == 'C') )
        {
          root->face_flags |= FT_FACE_FLAG_FIXED_WIDTH;
        }
      }
    }

    /* FZ XXX : TO DO : FT_FACE_FLAGS_VERTICAL  */
    /* FZ XXX : I need a font to implement this */

    root->style_flags = 0;
    
    prop = bdf_get_font_property (font,"SLANT");
    
    if ( prop && prop->format == BDF_ATOM )
    { 
      if ( (*(prop->value.atom) == 'O' ) ||
           (*(prop->value.atom) == 'I' ) )
      {
        root->style_flags |= FT_STYLE_FLAG_ITALIC;
      }
    }

    prop = bdf_get_font_property (font,"WEIGHT_NAME");
    
    if ( prop && prop->format == BDF_ATOM )
    {
      if ( *(prop->value.atom) == 'B' )
        root->style_flags |= FT_STYLE_FLAG_BOLD;
    }


    prop = bdf_get_font_property (font,"FAMILY_NAME");
    if (prop != NULL) {
      int l = strlen(prop->value.atom) + 1;
      if ( FT_ALLOC( root->family_name, l * sizeof(char)) )
        goto Fail;
      strcpy(root->family_name, prop->value.atom);
    } else root->family_name = 0;

    root->style_name = (char *)"Regular";
    if ( root->style_flags & FT_STYLE_FLAG_BOLD )
      {
        if ( root->style_flags & FT_STYLE_FLAG_ITALIC )
          root->style_name = (char *)"Bold Italic";
        else
          root->style_name = (char *)"Bold";
      }
    else if ( root->style_flags & FT_STYLE_FLAG_ITALIC )
      root->style_name = (char *)"Italic";

    root->num_glyphs = font->glyphs_size ; /* unencoded included */
    
    root->num_fixed_sizes = 1;
    if ( FT_ALLOC_ARRAY( root->available_sizes, 1,
                      FT_Bitmap_Size ) )
      goto Fail;
    
    prop = bdf_get_font_property(font,"PIXEL_SIZE");
    if (prop != NULL) {
      bdf_property_t *xres = 0, *yres = 0;

      xres = bdf_get_font_property(font,"RESOLUTION_X");
      yres = bdf_get_font_property(font,"RESOLUTION_Y");
      if ((xres != NULL) && (yres != NULL)) {
        FT_TRACE4(("prop %d %d %d\n",prop->value.int32, xres->value.int32, 
                   yres->value.int32));
        root->available_sizes->width = 
          prop->value.int32 * 75 / xres->value.int32;
        root->available_sizes->height = 
          prop->value.int32 * 75 / yres->value.int32;
      }
    } else { 
      /* some fonts have broken SIZE declaration (jiskan24.bdf) */
      FT_ERROR(("BDF Warning: reading size\n"));
      root->available_sizes->width = font->point_size ;
      root->available_sizes->height = font->point_size ;
    }      
 
    /* encoding table */
    {
      bdf_glyph_t *cur = font->glyphs;
      int n;

      if ( FT_ALLOC ( face->en_table , 
                   font->glyphs_size * sizeof(BDF_encoding_el ) ) )
        goto Fail;

      for (n = 0; n<font->glyphs_size ; n++) {
        (face->en_table[n]).enc = cur[n].encoding ;
        FT_TRACE4(("enc n: %d, val %ld\n",n,cur[n].encoding));
        (face->en_table[n]).glyph = n;
      }
    }
    
    /* charmaps */
    {
      bdf_property_t *charset_registry = 0, *charset_encoding = 0;
      
      charset_registry = bdf_get_font_property(font,"CHARSET_REGISTRY");
      charset_encoding = bdf_get_font_property(font,"CHARSET_ENCODING");
      if ((charset_registry != NULL) && (charset_encoding != NULL)) {
        if ((charset_registry->format == BDF_ATOM) && 
            (charset_encoding->format == BDF_ATOM)) {
          if (FT_ALLOC(face->charset_encoding, 
                    (strlen(charset_encoding->value.atom)+1) * sizeof(char))) 
            goto Exit;
          if (FT_ALLOC(face->charset_registry, 
                    (strlen(charset_registry->value.atom)+1) * sizeof(char))) 
            goto Exit;
          strcpy(face->charset_registry,charset_registry->value.atom);
          strcpy(face->charset_encoding,charset_encoding->value.atom);
            
          face->charmap.encoding = ft_encoding_none;
          face->charmap.platform_id = 0;
          face->charmap.encoding_id = 0;
          face->charmap.face = root; 
          face->charmap_handle = &face->charmap;
          root->charmap = face->charmap_handle;
          goto Exit;
        }  
      }

      /* otherwise assume adobe standard encoding */
      face->charmap.encoding = ft_encoding_adobe_standard;
      face->charmap.platform_id = 7; /* taken from t1objs.c */
      face->charmap.encoding_id = 0;
      face->charmap.face = root; 
      face->charmap_handle = &face->charmap;
      root->charmap = face->charmap_handle;
    }
  }
 
 Exit:
  return FT_Err_Ok;
  
 Fail:
  BDF_Face_Done( face );
  return FT_Err_Unknown_File_Format;
}

static
FT_Error  BDF_Set_Pixel_Size( FT_Size  size )
{
  BDF_Face face = (BDF_Face)FT_SIZE_FACE( size );
  FT_Face  root = FT_FACE( face );

  FT_TRACE4(("rec %d - pres %d\n",size->metrics.y_ppem,
      root->available_sizes->height));
  if (size->metrics.y_ppem == root->available_sizes->height) {
   
    size->metrics.ascender = face->bdffont->bbx.ascent << 6; 
    size->metrics.descender = face->bdffont->bbx.descent * (-64);
    size->metrics.height = face->bdffont->bbx.height <<6;
      
    return FT_Err_Ok;
  }
  else {
    return FT_Err_Invalid_Pixel_Size;
  }
}

static
FT_Error  BDF_Glyph_Load( FT_GlyphSlot  slot,
                          FT_Size       size,
                          FT_UInt       glyph_index,
                          FT_Int        load_flags )
{
  BDF_Face face = (BDF_Face)FT_SIZE_FACE( size );
  FT_Error error = FT_Err_Ok;
  FT_Bitmap *bitmap = &slot->bitmap;
  bdf_glyph_t glyph;
  int i;
  FT_Memory memory = face->bdffont->memory;
  
  if (!face) {
    error = FT_Err_Invalid_Argument;
    goto Exit;
  }
  
  /* slot, bitmap => freetype, glyph => bdflib */
  glyph = face->bdffont->glyphs[glyph_index];
  
  bitmap->pitch = (glyph.bbx.width + 7) >> 3;
  bitmap->rows = glyph.bbx.height;
  bitmap->width = glyph.bbx.width;
  bitmap->num_grays = 1;              /* unused */
  bitmap->pixel_mode = ft_pixel_mode_mono;
  
  if ( FT_ALLOC ( bitmap->buffer , glyph.bytes) )
    return FT_Err_Out_Of_Memory;
  FT_MEM_SET( bitmap->buffer , 0 , glyph.bytes );
  for (i=0 ; i<glyph.bytes ; i++) {
    bitmap->buffer[i] = glyph.bitmap[i];
  }

  slot->bitmap_left = 0;
  slot->bitmap_top = glyph.bbx.ascent ; 

  /*  FZ TO DO : vertical metrics  */
  slot->metrics.horiAdvance = glyph.dwidth << 6;
  slot->metrics.horiBearingX = glyph.bbx.x_offset << 6 ;
  slot->metrics.horiBearingY = glyph.bbx.y_offset << 6 ;
  slot->metrics.width = bitmap->width << 6 ;
  slot->metrics.height = bitmap->rows << 6;
  
  slot->linearHoriAdvance = (FT_Fixed)glyph.dwidth << 16;
  slot->format = ft_glyph_format_bitmap;
  slot->flags = FT_GLYPH_OWN_BITMAP;
    
 Exit:
  return error; 
}

static
FT_UInt  BDF_Get_Char_Index( FT_CharMap  charmap,
                             FT_ULong    char_code )
{
  BDF_Face face = ((BDF_Face)charmap->face);
  BDF_encoding_el *en_table = face->en_table;
  int low, high, mid;

  FT_TRACE4(("get_char_index %ld\n", char_code));
  
  low = 0;
  high = face->bdffont->glyphs_used - 1;   
  while (low <= high) {
    mid = (low+high) / 2;
    if (char_code < en_table[mid].enc)
      high = mid - 1;
    else if (char_code > en_table[mid].enc)
      low = mid + 1;
    else return en_table[mid].glyph;
  }
  
  return face->bdffont->default_glyph;
}

FT_CALLBACK_TABLE_DEF
const FT_Driver_ClassRec  bdf_driver_class =
{
  {
    ft_module_font_driver,
    sizeof ( FT_DriverRec ),
    
    "bdf",
    0x10000L,
    0x20000L,
      
    0,
    
    (FT_Module_Constructor)0,
    (FT_Module_Destructor) 0,
    (FT_Module_Requester)  0
  },
  
  sizeof( BDF_FaceRec ),
  sizeof( FT_SizeRec ),
  sizeof( FT_GlyphSlotRec ),
 
    (FT_Face_InitFunc)        BDF_Face_Init,
    (FT_Face_DoneFunc)        BDF_Face_Done,
    (FT_Size_InitFunc)        0,
    (FT_Size_DoneFunc)        0,
    (FT_Slot_InitFunc)        0,
    (FT_Slot_DoneFunc)        0,

    (FT_Size_ResetPointsFunc) BDF_Set_Pixel_Size,
    (FT_Size_ResetPixelsFunc) BDF_Set_Pixel_Size,

    (FT_Slot_LoadFunc)        BDF_Glyph_Load,

#ifndef FT_CONFIG_OPTION_USE_CMAPS    
   (FT_CharMap_CharIndexFunc)0,
#else
    (FT_CharMap_CharIndexFunc)0,
#endif

    (FT_Face_GetKerningFunc)  0,
    (FT_Face_AttachFunc)      0,
    (FT_Face_GetAdvancesFunc) 0,

#ifndef FT_CONFIG_OPTION_USE_CMAPS
    (FT_CharMap_CharNextFunc) 0, /*PCF_Char_Get_Next,*/
#else
    (FT_CharMap_CharNextFunc) 0
#endif    
  };

 /*  
  (FTDriver_initFace)     BDF_Init_Face,
  (FTDriver_doneFace)     BDF_Done_Face,
  (FTDriver_initSize)     0,
  (FTDriver_doneSize)     0,
  (FTDriver_initGlyphSlot)0,
  (FTDriver_doneGlyphSlot)0,
  
  (FTDriver_setCharSizes) BDF_Set_Pixel_Size,
  (FTDriver_setPixelSizes)BDF_Set_Pixel_Size,

  (FTDriver_loadGlyph)    BDF_Load_Glyph,
  (FTDriver_getCharIndex) BDF_Get_Char_Index,
  
  (FTDriver_getKerning)   0,
  (FTDriver_attachFile)   0,
  (FTDriver_getAdvances)  0
  */



#ifdef FT_CONFIG_OPTION_DYNAMIC_DRIVERS


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    getDriverClass                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This function is used when compiling the TrueType driver as a      */
  /*    shared library (`.DLL' or `.so').  It will be used by the          */
  /*    high-level library of FreeType to retrieve the address of the      */
  /*    driver's generic interface.                                        */
  /*                                                                       */
  /*    It shouldn't be implemented in a static build, as each driver must */
  /*    have the same function as an exported entry point.                 */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The address of the TrueType's driver generic interface.  The       */
  /*    format-specific interface can then be retrieved through the method */
  /*    interface->get_format_interface.                                   */
  /*                                                                       */
  FT_EXPORT_DEF( const FT_Driver_Class* )  
  getDriverClass( void )
  {
    return &bdf_driver_class;
  }


#endif /* FT_CONFIG_OPTION_DYNAMIC_DRIVERS */


/* END */
