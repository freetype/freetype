/***************************************************************************/
/*                                                                         */
/*  fi.c                                                                   */
/*                                                                         */
/*    2 characters ligature test program for AAT/TrueTypeGX font driver.   */
/*                                                                         */
/*  Copyright 2004 by                                                      */
/*  Masatake YAMATO and Redhat K.K.                                        */
/*                                                                         */
/*  This file may only be used,                                            */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

/***************************************************************************/
/* Development of the code in this file is support of                      */
/* Information-technology Promotion Agency, Japan.                         */
/***************************************************************************/

/* "fi" is the typical alphabet ligature.      *
 * "fl" is another good candidate to be tried. */

const char * target = "fi";

#include <ft2build.h>
#include FT_LAYOUT_H
#include FT_GXLAYOUT_H
#include FT_INTERNAL_OBJECTS_H	/* for FT_FACE_MEMORY */

#include <stdio.h>

FT_Error try_lig(FT_Face face, char a, char b);

int
main(int argc, char ** argv)
{
  FT_Error error;
  int result = 0;
  FT_Library library;
  FT_Face face;
  
  if ( argc != 2 )
    {
      fprintf(stderr, "Usage: %s fontfile\n", argv[0]);
      return 1;
    }
  
  error = FT_Init_FreeType ( &library );
  if ( error )
    {
      fprintf(stderr, "Abort: Fail to initialize FT2\n");
      result = 1;
      goto Fail;
    }
  
  error = FT_New_Face (library, argv[1], 0, &face);  
  if ( error )
    {
      fprintf(stderr, "Abort: Fail to open the file\n");
      result = 1;
      goto Fail;
    }
  if ( !( face->face_flags & FT_FACE_FLAG_GLYPH_SUBSTITUTION) )
    {
      fprintf(stderr, "Abort: No substitution table for the face\n");
      result = 1;
      goto Fail;
    }
  
  fprintf(stdout, "-------------------\n");
  fprintf(stdout,
	  "File: %s\nName: %s\n", 
	  argv[1], 
	  ((FT_Face)face)->family_name);
  
  error = try_lig( face, target[0], target[1] );
  if ( error )
    {
      fprintf(stderr, "Abort: Fail to substitute\n");
      result = 1;
    }
  
  FT_Done_Face ( face );

 Fail:
  FT_Done_FreeType  (library);
  return result;
} 

FT_Error 
try_lig( FT_Face face, char c1, char c2 )
{
  FT_Error error = FT_Err_Ok;
  FTL_EngineType engine_type;
  FTL_GlyphArray in, out;
  FTL_FeaturesRequest request;

  /* Get the engine type */
  if (( error = FTL_Query_EngineType( face, &engine_type ) ))
    goto Fail;
  
  /* Ignore if the engine type is not GX.  */
  if ( engine_type != FTL_TRUETYPEGX_ENGINE )
    {
      fprintf(stderr, "Abort: Not GX font.\n");
      goto Fail;
    }

  /* Allocate input and output glyphs arrays. 
     The lenght for input has already been known: 2 */
  FTL_New_Glyphs_Array ( FT_FACE_MEMORY(face), &in );
  FTL_New_Glyphs_Array ( FT_FACE_MEMORY(face), &out );
  FTL_Set_Glyphs_Array_Length ( in, 2 );

  /* Get glyph id for c1 and c2 */
  in->glyphs[0].gid = FT_Get_Char_Index( face, c1 );
  in->glyphs[1].gid = FT_Get_Char_Index( face, c2 );
  fprintf(stdout, "[IN]%c: %u, %c: %u\n", 
	  c1, in->glyphs[0].gid,
	  c2, in->glyphs[1].gid);
  
  /* Create a features-request. */
  FTL_New_FeaturesRequest ( face, &request );
  
  /* -------------------------------------
   * YOU CAN SET SOME LAYOUT SETTINGS HERE 
   * -------------------------------------
   * In this program, just use default.
   * -------------------------------------
   */ 
  
  /* Activeate the features request */
  FTL_Activate_FeaturesRequest( request );

  /* Do substitute the glyphs */
  FTL_Substitute_Glyphs( face, in, out );

  /* Free the features-request */
  FTL_Done_FeaturesRequest ( request );
  
  fprintf(stdout, 
	  "[OUT]%c: %u, %c: %u%s\n", 
	  c1, out->glyphs[0].gid,
	  c2, out->glyphs[1].gid, 
	  (out->glyphs[1].gid == 0xFFFF)? "<empty>": "");

  /* Free glyphs arrays */
  FTL_Done_Glyphs_Array ( in );
  FTL_Done_Glyphs_Array ( out );    

 Fail:
  return error;
}


/* END */
