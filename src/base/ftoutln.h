#ifndef FTOUTLN_H
#define FTOUTLN_H

#include <ftobjs.h>

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
  EXPORT_DEF
  FT_Error  FT_Outline_Copy( FT_Outline*  source,
                             FT_Outline*  target );

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
  EXPORT_DEF
  FT_Error  FT_Outline_Get_Bitmap( FT_Library   library,
                                   FT_Outline*  outline,
                                   FT_Bitmap*   map );


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
  EXPORT_DEF
  void  FT_Outline_Transform( FT_Outline*  outline,
                              FT_Matrix*   matrix );


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
  EXPORT_DEF
  void  FT_Vector_Transform( FT_Pos*     x,
                             FT_Pos*     y,
                             FT_Matrix*  matrix );


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
  EXPORT_DEF
  void  FT_Matrix_Multiply( FT_Matrix*  a,
                            FT_Matrix*  b );


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
  EXPORT_DEF
  FT_Error  FT_Matrix_Invert( FT_Matrix*  matrix );

#endif /* FTOUTLN_H */
