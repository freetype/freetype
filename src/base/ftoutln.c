/***************************************************************************/
/*                                                                         */
/*  ftoutln.c                                                              */
/*                                                                         */
/*    FreeType outline management (body).                                  */
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
/***************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* All functions are declared in freetype.h.                             */
  /*                                                                       */
  /*************************************************************************/


#include <freetype.h>
#include <ftconfig.h>
#include <ftobjs.h>
#include <ftimage.h>
#include <ftoutln.h>

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Outline_Copy                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Copies an outline into another one.  Both objects must have the    */
  /*    same sizes (number of points & number of contours) when this       */
  /*    function is called.                                                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    source :: A handle to the source outline.                          */
  /*    target :: A handle to the target outline.                          */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  BASE_FUNC
  FT_Error  FT_Outline_Copy( FT_Outline*  source,
                             FT_Outline*  target )
  {
    FT_Int  is_owner;
    
    if ( !source            || !target            ||
         source->n_points   != target->n_points   ||
         source->n_contours != target->n_contours )
      return FT_Err_Invalid_Argument;

    MEM_Copy( target->points, source->points,
              source->n_points * 2 * sizeof ( FT_Pos ) );

    MEM_Copy( target->tags, source->tags,
              source->n_points * sizeof ( FT_Byte ) );

    MEM_Copy( target->contours, source->contours,
              source->n_contours * sizeof ( FT_Short ) );

    /* copy all flags, except the "ft_outline_owner" one */
    is_owner = target->flags & ft_outline_owner;
    target->flags = source->flags;
    
    target->flags &= ~ft_outline_owner;
    target->flags |= is_owner;
    return FT_Err_Ok;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Outline_Get_Bitmap                                              */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Renders an outline within a bitmap.  The outline's image is simply */
  /*    or-ed to the target bitmap.                                        */
  /*                                                                       */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library :: A handle to a FreeType library object.                  */
  /*    outline :: A pointer to the source outline descriptor.             */
  /*    map     :: A pointer to the target bitmap descriptor.              */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <MT-Note>                                                             */
  /*    YES.  Rendering is synchronized, so that concurrent calls to the   */
  /*    scan-line converter will be serialized.                            */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This function does NOT CREATE the bitmap, it only renders an       */
  /*    outline image within the one you pass to it!                       */
  /*                                                                       */
  /*    It will use the raster correponding to the default glyph format.   */
  /*                                                                       */
  BASE_FUNC
  FT_Error  FT_Outline_Get_Bitmap( FT_Library   library,
                                   FT_Outline*  outline,
                                   FT_Bitmap*   map )
  {
    FT_Error          error;
    FT_Glyph_Format*  format;

    error  = FT_Err_Invalid_Glyph_Format;
    format = FT_Get_Glyph_Format( library, ft_glyph_format_outline );
    if (!format) goto Exit;

    error = FT_Err_Invalid_Glyph_Format;
    if (!format->raster) goto Exit;

    error = format->raster_interface->render( format->raster, outline, map );
  Exit:
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Outline_Transform                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Applies a simple 2x2 matrix to all of an outline's points.  Useful */
  /*    for applying rotations, slanting, flipping, etc.                   */
  /*                                                                       */
  /* <Input>                                                               */
  /*    outline :: A pointer to the target outline descriptor.             */
  /*    matrix  :: A pointer to the transformation matrix.                 */
  /*                                                                       */
  /* <MT-Note>                                                             */
  /*    Yes.                                                               */
  /*                                                                       */
  /* <Note>                                                                */
  /*    You can use FT_Outline_Translate() if you need to translate the    */
  /*    outline's points.                                                  */
  /*                                                                       */
  BASE_FUNC
  void  FT_Outline_Transform( FT_Outline*  outline,
                              FT_Matrix*   matrix )
  {
    FT_UShort   n;
    FT_Vector*  vec;


    vec = outline->points;
    for ( n = 0; n < outline->n_points; n++ )
    {
      FT_Pos  x, y;


      x = FT_MulFix( vec->x, matrix->xx ) +
          FT_MulFix( vec->y, matrix->xy );

      y = FT_MulFix( vec->x, matrix->yx ) +
          FT_MulFix( vec->y, matrix->yy );

      vec->x = x;
      vec->y = y;
      vec++;
    }
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Vector_Transform                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Transforms a single vector through a 2x2 matrix.                   */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    x      :: The horizontal vector coordinate.                        */
  /*    y      :: The vertical vector coordinate.                          */
  /*                                                                       */
  /* <Input>                                                               */
  /*    matrix :: A pointer to the source 2x2 matrix.                      */
  /*                                                                       */
  /* <MT-Note>                                                             */
  /*    Yes.                                                               */
  /*                                                                       */
  BASE_FUNC
  void  FT_Vector_Transform( FT_Pos*     x,
                             FT_Pos*     y,
                             FT_Matrix*  matrix )
  {
    FT_Pos xz, yz;


    xz = FT_MulFix( *x, matrix->xx ) +
         FT_MulFix( *y, matrix->xy );

    yz = FT_MulFix( *x, matrix->yx ) +
         FT_MulFix( *y, matrix->yy );

    *x = xz;
    *y = yz;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Matrix_Multiply                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Performs the matrix operation `b = a*b'.                           */
  /*                                                                       */
  /* <Input>                                                               */
  /*    a :: A pointer to matrix `a'.                                      */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    b :: A pointer to matrix `b'.                                      */
  /*                                                                       */
  /* <MT-Note>                                                             */
  /*    Yes.                                                               */
  /*                                                                       */
  BASE_FUNC
  void  FT_Matrix_Multiply( FT_Matrix*  a,
                            FT_Matrix*  b )
  {
    FT_Fixed  xx, xy, yx, yy;


    xx = FT_MulFix( a->xx, b->xx ) + FT_MulFix( a->xy, b->yx );
    xy = FT_MulFix( a->xx, b->xy ) + FT_MulFix( a->xy, b->yy );
    yx = FT_MulFix( a->yx, b->xx ) + FT_MulFix( a->yy, b->yx );
    yy = FT_MulFix( a->yx, b->xy ) + FT_MulFix( a->yy, b->yy );

    b->xx = xx;  b->xy = xy;
    b->yx = yx;  b->yy = yy;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Matrix_Invert                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Inverts a 2x2 matrix.  Returns an error if it can't be inverted.   */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    matrix :: A pointer to the target matrix.  Remains untouched in    */
  /*              case of error.                                           */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <MT-Note>                                                             */
  /*    Yes.                                                               */
  /*                                                                       */
  BASE_FUNC
  FT_Error  FT_Matrix_Invert( FT_Matrix*  matrix )
  {
    FT_Pos  delta, xx, yy;


    /* compute discriminant */
    delta = FT_MulFix( matrix->xx, matrix->yy ) -
            FT_MulFix( matrix->xy, matrix->yx );

    if ( !delta )
      return FT_Err_Invalid_Argument;  /* matrix can't be inverted */

    matrix->xy = - FT_DivFix( matrix->xy, delta );
    matrix->yx = - FT_DivFix( matrix->yx, delta );

    xx = matrix->xx;
    yy = matrix->yy;

    matrix->xx = FT_DivFix( yy, delta );
    matrix->yy = FT_DivFix( xx, delta );

    return FT_Err_Ok;
  }


/* END */
