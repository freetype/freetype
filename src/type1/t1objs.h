/*******************************************************************
 *
 *  t1objs.h                                                    1.0
 *
 *    Type1 objects definition.         
 *
 *  Copyright 1996-1999 by
 *  David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 *  This file is part of the FreeType project, and may only be used
 *  modified and distributed under the terms of the FreeType project
 *  license, LICENSE.TXT.  By continuing to use, modify, or distribute
 *  this file you indicate that you have read the license and
 *  understand and accept it fully.
 *
 ******************************************************************/

#ifndef T1OBJS_H
#define T1OBJS_H

#include <ftobjs.h>
#include <ftconfig.h>

#include <t1errors.h>
#include <t1types.h>

#ifdef __cplusplus
  extern "C" {
#endif

  /* The following structures must be defined by the hinter */
  typedef struct T1_Size_Hints_   T1_Size_Hints;
  typedef struct T1_Glyph_Hints_  T1_Glyph_Hints;

  /***********************************************************************/
  /*                                                                     */
  /* <Type> T1_Driver                                                    */
  /*                                                                     */
  /* <Description>                                                       */
  /*    A handle to a Type 1 driver object.                              */
  /*                                                                     */
  typedef struct T1_DriverRec_   *T1_Driver;


  /***********************************************************************/
  /*                                                                     */
  /* <Type> T1_Size                                                      */
  /*                                                                     */
  /* <Description>                                                       */
  /*    A handle to a Type 1 size object.                                */
  /*                                                                     */
  typedef struct T1_SizeRec_*  T1_Size;


  /***********************************************************************/
  /*                                                                     */
  /* <Type> T1_GlyphSlot                                                 */
  /*                                                                     */
  /* <Description>                                                       */
  /*    A handle to a Type 1 glyph slot object.                          */
  /*                                                                     */
  typedef struct T1_GlyphSlotRec_*  T1_GlyphSlot;


  /***********************************************************************/
  /*                                                                     */
  /* <Type> T1_CharMap                                                   */
  /*                                                                     */
  /* <Description>                                                       */
  /*    A handle to a Type 1 character mapping object.                   */
  /*                                                                     */
  /* <Note>                                                              */
  /*    The Type 1 format doesn't use a charmap but an encoding table.   */
  /*    The driver is responsible for making up charmap objects          */
  /*    corresponding to these tables..                                  */
  /*                                                                     */
  typedef struct T1_CharMapRec_*   T1_CharMap;



 /**************************************************************************/
 /*                                                                        */
 /*    NOW BEGINS THE TYPE1 SPECIFIC STUFF ..............................  */
 /*                                                                        */
 /**************************************************************************/
 

  /***************************************************/
  /*                                                 */
  /*  T1_Size :                                      */
  /*                                                 */
  /*    Type 1 size record..                         */
  /*                                                 */
  
  typedef struct T1_SizeRec_
  {
    FT_SizeRec      root;
    T1_Bool         valid;
    T1_Size_Hints*  hints;  /* defined in the hinter. This allows */
                            /* us to experiment with different    */
                            /* hinting schemes without having to  */
                            /* change 't1objs' each time..        */
  } T1_SizeRec;



  /***************************************************/
  /*                                                 */
  /*  T1_GlyphSlot :                                 */
  /*                                                 */
  /*    TrueDoc glyph record..                       */
  /*                                                 */
  
  typedef struct T1_GlyphSlotRec_
  {
    FT_GlyphSlotRec  root;

    T1_Bool          hint;
    T1_Bool          scaled;
    
    T1_Int           max_points;
    T1_Int           max_contours;

    FT_Fixed         x_scale;
    FT_Fixed         y_scale;

    T1_Glyph_Hints*  hints;  /* defined in the hinter */

  } T1_GlyphSlotRec;


/*******************************************************************
 *
 *  <Function>  T1_Init_Face
 *
 *  <Description>
 *     Initialise a given Type 1 face object
 *
 *  <Input>
 *     face_index :: index of font face in resource
 *     resource   :: source font resource
 *     face       ::  face record to build
 *
 *  <Return>
 *     Error code.
 *
 ******************************************************************/

  LOCAL_DEF
  T1_Error  T1_Init_Face( FT_Stream    stream,
                          FT_Int       face_index,
                          T1_Face      face );



/*******************************************************************
 *
 *  <Function> T1_Done_Face
 *
 *  <Description>
 *     Finalise a given face object
 *
 *  <Input>
 *     face  :: handle  to the face object to destroy
 *
 ******************************************************************/

  LOCAL_DEF
  void  T1_Done_Face( T1_Face  face );



/*******************************************************************
 *
 *  <Function> T1_Init_Size
 *
 *  <Description>
 *     Initialise a new Type 1 size object
 *
 *  <Input>
 *     size  :: handle to size object
 *
 *  <Return>
 *     Type 1 error code. 0 means success.
 *
 ******************************************************************/

  LOCAL_DEF
  T1_Error  T1_Init_Size( T1_Size  size );



/*******************************************************************
 *
 * <Function>  T1_Done_Size
 *
 * <Description>
 *    The Type 1 size object finaliser.
 *
 * <Input>
 *    size   :: handle to the target size object.
 *
 ******************************************************************/

  LOCAL_DEF
  void  T1_Done_Size( T1_Size  size );


/*******************************************************************
 *
 * <Function>  T1_Reset_Size
 *
 * <Description>
 *    Reset a Type 1 size when resolutions and character dimensions
 *    have been changed..
 *
 * <Input>
 *    size   :: handle to the target size object.
 *
 ******************************************************************/

  LOCAL_DEF
  T1_Error  T1_Reset_Size( T1_Size  size );



/*******************************************************************
 *
 *  <Function> T1_Init_GlyphSlot
 *
 *  <Description> The TrueType glyph slot initialiser
 *
 *  <Input>  glyph ::  glyph record to build.
 *
 *  <Output> Error code.
 *
 ******************************************************************/

  LOCAL_DEF
  T1_Error  T1_Init_GlyphSlot( T1_GlyphSlot  slot );



/*******************************************************************
 *
 *  <Function> T1_Done_GlyphSlot
 *
 *  <Description> The Type 1 glyph slot finaliser
 *
 *  <Input>  glyph  :: handle to glyph slot object
 *
 *  <Output>  Error code.                       
 *
 ******************************************************************/

  LOCAL_DEF
  void  T1_Done_GlyphSlot( T1_GlyphSlot  slot );



/*******************************************************************
 *
 *  <Function>  T1_Init_Driver
 *
 *  <Description>
 *     Initialise a given Type 1 driver object
 *
 *  <Input>
 *     driver ::  handle to target driver object
 *
 *  <Return>
 *     Error code.
 *
 ******************************************************************/

  LOCAL_DEF
  T1_Error  T1_Init_Driver( T1_Driver  driver );



/*******************************************************************
 *
 *  <Function> T1_Done_Driver
 *
 *  <Description>
 *     finalise a given Type 1 driver
 *
 *  <Input>
 *     driver  :: handle to target Type 1 driver
 *
 ******************************************************************/

  LOCAL_DEF
  void  T1_Done_Driver( T1_Driver  driver );

#ifdef __cplusplus
  }
#endif

#endif /* T1OBJS_H */


/* END */
