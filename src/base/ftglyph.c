/***************************************************************************/
/*                                                                         */
/*  ftglyph.c                                                              */
/*                                                                         */
/*    FreeType convenience functions to handle glyphs..                    */
/*                                                                         */
/*  Copyright 1996-1999 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used        */
/*  modified and distributed under the terms of the FreeType project       */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/*  This file contains the definition of several convenience functions     */
/*  that can be used by client applications to easily retrieve glyph       */
/*  bitmaps and outlines from a given face.                                */
/*                                                                         */
/*  These functions should be optional if you're writing a font server     */
/*  or text layout engine on top of FreeType. However, they are pretty     */
/*  handy for many other simple uses of the library..                      */
/*                                                                         */
/***************************************************************************/

#include <ftglyph.h>
#include <ftobjs.h>

  static
  void ft_prepare_glyph( FT_Glyph  glyph,
                         FT_Face   face,
                         FT_Bool   vertical )
{
    FT_Glyph_Metrics*  metrics = &face->glyph->metrics;
    
    glyph->memory   = face->memory;
    glyph->width    = metrics->width;
    glyph->height   = metrics->height;
    
    if (vertical)
    {
      glyph->bearingX = metrics->vertBearingX;
      glyph->bearingY = metrics->vertBearingY;
      glyph->advance  = metrics->vertAdvance;
    }
    else
    {
      glyph->bearingX = metrics->horiBearingX;
      glyph->bearingY = metrics->horiBearingY;
      glyph->advance  = metrics->horiAdvance;
    }
  }

 /***********************************************************************
  *
  * <Function>
  *    FT_Get_Glyph_Bitmap
  *
  * <Description>
  *    A function used to directly return a monochrome bitmap glyph image
  *    from a face.
  *
  * <Input>
  *    face        :: handle to source face object
  *    glyph_index :: glyph index in face
  *    load_flags  :: load flags, see FT_LOAD_FLAG_XXXX constants..
  *    grays       :: number of gray levels for anti-aliased bitmaps,
  *                   set to 0 if you want to render a monochrome bitmap
  *    origin      :: a pointer to the origin's position. Set to 0
  *                   if the current transform is the identity..
  *
  * <Output>
  *    bitglyph :: pointer to the new bitmap glyph
  *
  * <Return>
  *    Error code. 0 means success.
  *
  * <Note>
  *    If the font contains glyph outlines, these will be automatically
  *    converted to a bitmap according to the value of "grays"
  *
  *    If "grays" is set to 0, the result is a 1-bit monochrome bitmap
  *    otherwise, it is an 8-bit gray-level bitmap
  *
  *    The number of gray levels in the result anti-aliased bitmap might
  *    not be "grays", depending on the current scan-converter implementation
  *
  *    Note that it is not possible to generate 8-bit monochrome bitmaps
  *    with this function. Rather, use FT_Get_Glyph_Outline, then
  *    FT_Glyph_Render_Outline and provide your own span callbacks..
  *
  *    When the face doesn't contain scalable outlines, this function will
  *    fail if the current transform is not the identity, or if the glyph
  *    origin's phase to the pixel grid is not 0 in both directions !!
  *
  ***********************************************************************/

  EXPORT_FUNC
  FT_Error  FT_Get_Glyph_Bitmap( FT_Face         face,
                                 FT_UInt         glyph_index,
                                 FT_UInt         load_flags,
                                 FT_Int          grays,
                                 FT_Vector*      origin,
                                 FT_BitmapGlyph  *abitglyph )
  {
    FT_Error         error;
    FT_Memory        memory;
    FT_BitmapGlyph   bitglyph;
    FT_Glyph         glyph;
    FT_Pos           origin_x = 0;
    FT_Pos           origin_y = 0;

    *abitglyph = 0;
    
    if (origin)
    {
      origin_x = origin->x & 63;
      origin_y = origin->y & 63;
    }
    
    /* check arguments if the face's format is not scalable */
    if ( !(face->face_flags & FT_FACE_FLAG_SCALABLE) && face->transform_flags )
    {
      /* we can't transform bitmaps, so return an error */
      error = FT_Err_Unimplemented_Feature;
      goto Exit;
    }

    /* check that NO_SCALE and NO_RECURSE are not set */
    if (load_flags & (FT_LOAD_NO_SCALE|FT_LOAD_NO_RECURSE))
    {
      error = FT_Err_Invalid_Argument;
      goto Exit;
    }

    /* disable embedded bitmaps for transformed images */
    if ( face->face_flags & FT_FACE_FLAG_SCALABLE && face->transform_flags )
      load_flags |= FT_LOAD_NO_BITMAP;
      
    error = FT_Load_Glyph( face, glyph_index, load_flags );
    if (error) goto Exit;
    
    /* now, handle bitmap and outline glyph images */
    memory = face->memory;
    switch ( face->glyph->format )
    {
      case ft_glyph_format_bitmap:
        {
          FT_Long     size;
          FT_Bitmap*  source;
          
          if ( ALLOC( bitglyph, sizeof(*bitglyph) ) )
            goto Exit;
            
          glyph             = (FT_Glyph)bitglyph;
          glyph->glyph_type = ft_glyph_type_bitmap;
          ft_prepare_glyph( glyph, face, 0 );
          
          source = &face->glyph->bitmap;
          size   = source->rows * source->pitch;
          if (size < 0) size = -size;
          
          bitglyph->bitmap = *source;
          if ( ALLOC( bitglyph->bitmap.buffer, size ) )
            goto Fail;
            
          /* copy the content of the source glyph */
          MEM_Copy( bitglyph->bitmap.buffer, source->buffer, size );
        }
        break;

      case ft_glyph_format_outline:
        {
          FT_BBox  cbox;
          FT_Int   width, height, pitch;
          FT_Long  size;
          
          /* transform the outline - note that the original metrics are NOT */
          /* transformed by this.. only the outline points themselves..     */
          FT_Outline_Transform( &face->glyph->outline, &face->transform_matrix );
          FT_Outline_Translate( &face->glyph->outline,
                                face->transform_delta.x + origin_x,
                                face->transform_delta.y + origin_y );
                                
          /* compute the size in pixels of the outline */
          FT_Outline_Get_CBox( &face->glyph->outline, &cbox );
          cbox.xMin &= -64;
          cbox.yMin &= -64;
          cbox.xMax  = (cbox.xMax+63) & -64;
          cbox.yMax  = (cbox.yMax+63) & -64;
          
          width  = (cbox.xMax - cbox.xMin) >> 6;
          height = (cbox.yMax - cbox.yMin) >> 6;

          /* allocate the pixel buffer for the glyph bitmap */
          if (grays) pitch = (width+3) & -4;  /* some raster implementation need this */
                else pitch = (width+7) >> 3;
            
          size  = pitch * height;
          if ( ALLOC( bitglyph, sizeof(*bitglyph) ) )
            goto Exit;
            
          glyph             = (FT_Glyph)bitglyph;
          glyph->glyph_type = ft_glyph_type_bitmap;
          ft_prepare_glyph( glyph, face, 0 );
          
          if ( ALLOC( bitglyph->bitmap.buffer, size ) )
            goto Fail;
          
          bitglyph->bitmap.width      = width;
          bitglyph->bitmap.rows       = height;
          bitglyph->bitmap.pitch      = pitch;
          bitglyph->bitmap.pixel_mode = grays ? ft_pixel_mode_grays
                                              : ft_pixel_mode_mono;
          bitglyph->bitmap.num_grays  = (short)grays;
          
          bitglyph->left = (cbox.xMin >> 6);
          bitglyph->top  = (cbox.yMax >> 6);
          
          /* render the monochrome outline into the target buffer */
          FT_Outline_Translate( &face->glyph->outline, -cbox.xMin, -cbox.yMin );
          error = FT_Outline_Get_Bitmap( face->driver->library,
                                         &face->glyph->outline,
                                         &bitglyph->bitmap );
          if (error)
          {
            FREE( bitglyph->bitmap.buffer );
            goto Fail;
          }
        }
        break;
      
      default:
        error = FT_Err_Invalid_Glyph_Index;
        goto Exit;
    }
    
    *abitglyph = bitglyph;
  Exit:
    return error;
    
  Fail:
    FREE( glyph );
    goto Exit;
  }

  
 /***********************************************************************
  *
  * <Function>
  *    FT_Get_Glyph_Outline
  *
  * <Description>
  *    A function used to directly return a bitmap glyph image from a
  *    face. This is faster than calling FT_Load_Glyph+FT_Get_Outline_Bitmap..
  *
  * <Input>
  *    face        :: handle to source face object
  *    glyph_index :: glyph index in face
  *    load_flags  :: load flags, see FT_LOAD_FLAG_XXXX constants..
  * 
  * <Output>
  *    vecglyph :: pointer to the new outline glyph
  *
  * <Return>
  *    Error code. 0 means success.
  *
  * <Note>
  *    This function will fail if the load flags FT_LOAD_NO_OUTLINE and
  *    FT_LOAD_NO_RECURSE are set..
  *
  ***********************************************************************/
  
  EXPORT_FUNC
  FT_Error  FT_Get_Glyph_Outline( FT_Face           face,
                                  FT_UInt           glyph_index,
                                  FT_UInt           load_flags,
                                  FT_OutlineGlyph  *vecglyph )
  {
    FT_Error         error;
    FT_Memory        memory;
    FT_OutlineGlyph  glyph;

    *vecglyph = 0;
        
    /* check that NO_OUTLINE and NO_RECURSE are not set */
    if (load_flags & (FT_LOAD_NO_OUTLINE|FT_LOAD_NO_RECURSE))
    {
      error = FT_Err_Invalid_Argument;
      goto Exit;
    }
    
    /* disable the loading of embedded bitmaps */
    load_flags |= FT_LOAD_NO_BITMAP;
    
    error = FT_Load_Glyph( face, glyph_index, load_flags );
    if (error) goto Exit;
    
    /* check that we really loaded an outline */
    if ( face->glyph->format != ft_glyph_format_outline )
    {
      error = FT_Err_Invalid_Glyph_Index;
      goto Exit;
    }
    
    /* transform the outline - note that the original metrics are NOT */
    /* transformed by this.. only the outline points themselves..     */
    if ( face->transform_flags )
    {
      FT_Outline_Transform( &face->glyph->outline, &face->transform_matrix );
      FT_Outline_Translate( &face->glyph->outline,
                            face->transform_delta.x,
                            face->transform_delta.y );
    }
    
    /* now, create a new outline glyph and copy everything there */
    memory = face->memory;
    if ( ALLOC( glyph, sizeof(*glyph) ) )
      goto Exit;

    ft_prepare_glyph( (FT_Glyph)glyph, face, 0 );
    glyph->metrics.glyph_type = ft_glyph_type_outline;
    
    error = FT_Outline_New( face->driver->library,
                            face->glyph->outline.n_points,
                            face->glyph->outline.n_contours,
                            &glyph->outline );
    if (!error)
      error = FT_Outline_Copy( &face->glyph->outline, &glyph->outline );
      
    if (error) goto Fail;
    
    *vecglyph = glyph;
  Exit:
    return error;
    
  Fail:
    FREE( glyph );
    goto Exit;
  }

 /***********************************************************************
  *
  * <Function>
  *    FT_Set_Transform
  *
  * <Description>
  *    A function used to set the transform that is applied to glyph images
  *    just after they're loaded in the face's glyph slot, and before they're 
  *    returned by either FT_Get_Glyph_Bitmap or FT_Get_Glyph_Outline
  *
  * <Input>
  *    face   :: handle to source face object
  *    matrix :: pointer to the transform's 2x2 matrix. 0 for identity
  *    delta  :: pointer to the transform's translation. 0 for null vector
  *
  * <Note>
  *    The transform is only applied to glyph outlines when they are found
  *    in a font face. It is unable to transform embedded glyph bitmaps
  *
  ***********************************************************************/
  
  EXPORT_FUNC
  void FT_Set_Transform( FT_Face     face,
                         FT_Matrix*  matrix,
                         FT_Vector*  delta )
  {
    face->transform_flags = 0;
    
    if (!matrix)
    {
      face->transform_matrix.xx = 0x10000L;
      face->transform_matrix.xy = 0;
      face->transform_matrix.yx = 0L;
      face->transform_matrix.yy = 0x10000L;
      matrix = &face->transform_matrix;
    }
    else
      face->transform_matrix = *matrix;
    
    /* set transform_flags bit flag 0 if delta isn't the null vector */
    if ( (matrix->xy | matrix->yx) ||
         matrix->xx != 0x10000L    ||
         matrix->yy != 0x10000L    )
      face->transform_flags |= 1;

    if (!delta)
    {
      face->transform_delta.x = 0;
      face->transform_delta.y = 0;
      delta = &face->transform_delta;
    }
    else
      face->transform_delta = *delta;
      
    /* set transform_flags bit flag 1 if delta isn't the null vector */
    if ( delta->x | delta->y )
      face->transform_flags |= 2;
  }

 /***********************************************************************
  *
  * <Function>
  *    FT_Done_Glyph
  *
  * <Description>
  *    Destroys a given glyph..
  *
  * <Input>
  *    glyph  :: handle to target glyph object 
  *
  ***********************************************************************/
  
  EXPORT_FUNC
  void  FT_Done_Glyph( FT_Glyph  glyph )
  {
    if (glyph)
    {
      FT_Memory  memory = glyph->memory;
      
      if ( glyph->glyph_type == ft_glyph_type_bitmap )
      {
        FT_BitmapGlyph  bit = (FT_BitmapGlyph)glyph;
        FREE( bit->bitmap.buffer );
      }
      else if ( glyph->glyph_type == ft_glyph_type_outline )
      {
        FT_OutlineGlyph  out = (FT_OutlineGlyph)glyph;
        if (out->outline.flags & ft_outline_owner)
        {
          FREE( out->outline.points );
          FREE( out->outline.contours );
          FREE( out->outline.tags );
        }
      }
      
      FREE( glyph );
    }
  }


 /***********************************************************************
  *
  * <Function>
  *    FT_Glyph_Get_Box
  *
  * <Description>
  *    Returns the glyph image's bounding box in pixels.
  *
  * <Input>
  *    glyph :: handle to target glyph object 
  *
  * <Output>
  *    box   :: the glyph bounding box. Coordinates are expressed in
  *             _integer_ pixels, with exclusive max bounds
  *
  * <Note>
  *    Coordinates are relative to the glyph origin, using the Y-upwards
  *    convention..
  *
  *    The width of the box in pixels is box.xMax-box.xMin
  *    The height is box.yMax - box.yMin
  *
  ***********************************************************************/
  
  EXPORT_DEF
  void  FT_Glyph_Get_Box( FT_Glyph  glyph,
                          FT_BBox  *box )
  {
    box->xMin = box->xMax = 0;
    box->yMin = box->yMax = 0;
    
    if (glyph) switch (glyph->glyph_type)
    {
      case ft_glyph_type_bitmap:
        {
          FT_BitmapGlyph  bit = (FT_BitmapGlyph)glyph;
          box->xMin = bit->left;
          box->xMax = box->xMin + bit->bitmap.width;
          box->yMax = bit->top;
          box->yMin = box->yMax - bit->bitmap.rows;
        }
        break;
        
      case ft_glyph_type_outline:
        {
          FT_OutlineGlyph  out = (FT_OutlineGlyph)glyph;
          
          FT_Outline_Get_CBox( &out->outline, box );
          box->xMin >>= 6;
          box->yMin >>= 6;
          box->xMax  = (box->xMax+63) >> 6;
          box->yMax  = (box->yMax+63) >> 6;
        }
        break;
        
      default:
        ;
    }
  }
