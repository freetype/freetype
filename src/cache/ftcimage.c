/***************************************************************************/
/*                                                                         */
/*  ftcimage.c                                                             */
/*                                                                         */
/*    XXX                                                                  */
/*                                                                         */
/*  Copyright 2000 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#include <cache/ftcimage.h>

  static
  void  ftc_done_glyph_image( FTC_Image_Queue  queue,
                              FTC_ImageNode    node )
  {
    FT_UNUSED( queue );

    FT_Done_Glyph( FTC_IMAGENODE_GET_GLYPH( node ) );
  }


  static
  FT_ULong  ftc_size_bitmap_image( FTC_Image_Queue  queue,
                                   FTC_ImageNode    node )
  {
    FT_Long         pitch;
    FT_BitmapGlyph  glyph;
    
    FT_UNUSED( queue );


    glyph = (FT_BitmapGlyph)FTC_IMAGENODE_GET_GLYPH(node);
    pitch = glyph->bitmap.pitch;
    if ( pitch < 0 )
      pitch = -pitch;
      
    return (FT_ULong)(pitch * glyph->bitmap->rows + sizeof ( *glyph ) );
  }


  static
  FT_ULong  ftc_size_outline_image( FTC_Image_Queue  queue,
                                    FTC_ImageNode    node )
  {
    FT_Long          pitch;
    FT_OutlineGlyph  glyph;
    FT_Outline*      outline;
    
    FT_UNUSED( queue );


    glyph   = (FT_OutlineGlyph)FTC_IMAGENODE_GET_GLYPH( node );
    outline = &glyph->outline;
    
    return (FT_ULong)( 
      outline->n_points *  ( sizeof ( FT_Vector ) + sizeof ( FT_Byte ) ) +
      outline->n_contours * sizeof ( FT_Short )                          +
      sizeof( *glyph ) );
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****              MONOCHROME BITMAP CALLBACKS                      *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  static
  FT_Error  ftc_init_mono_image( FTC_Image_Queue  queue,
                                 FTC_ImageNode    node )
  {  
    FT_Face   face;
    FT_Size   size;
    FT_Error  error;

    
    error = FTC_Manager_Lookup_Size( queue->manager,
                                     &queue->size_rec,
                                     &face, &size );
    if ( !error )
    {
      FT_UInt  glyph_index = FTC_IMAGENODE_GINDEX( node );

      
      error = FT_Load_Glyph( face, glyph_index,
                             FT_LOAD_RENDER | FT_LOAD_MONOCHROME );
      if ( !error )
      {
        if ( face->glyph->format            != ft_image_format_bitmap ||
             face->glyph->bitmap.pixel_mode != ft_pixel_mode_mono     )
        {
          /* there is no monochrome glyph for this font! */
          error = FT_Err_Invalid_Glyph_Index;
        }
        else
        {
          /* ok, copy it */
          FT_Glyph  glyph;
          
          
          error = FT_Get_Glyph( face->glyph, &glyph );
          if ( !error )
            FTC_IMAGENODE_SET_GLYPH( node, glyph );
        }
      }
    }
    return error;
  }


  static
  FT_Error  ftc_init_gray_image( FTC_Image_Queue  queue,
                                 FTC_ImageNode    node )
  {  
    FT_Face   face;
    FT_Size   size;
    FT_Error  error;
    

    error = FTC_Manager_Lookup_Size( queue->manager,
                                     &queue->size_rec,
                                     &face, &size );
    if ( !error )
    {
      FT_UInt  glyph_index = FTC_IMAGENODE_GINDEX( node );
      

      error = FT_Load_Glyph( face, glyph_index,
                             FT_LOAD_RENDER );
      if ( !error )
      {
        if ( face->glyph->format            != ft_image_format_bitmap ||
             face->glyph->bitmap.pixel_mode != ft_pixel_mode_grays )
        {
          /* there is no monochrome glyph for this font! */
          error = FT_Err_Invalid_Glyph_Index;
        }
        else
        {
          /* ok, copy it */
          FT_Glyph  glyph;
          
          
          error = FT_Get_Glyph( face->glyph, &glyph );
          if ( !error )
            FTC_IMAGENODE_SET_GLYPH( node, glyph );
        }
      }
    }
    return error;
  }


/* END */
