/***************************************************************************/
/*                                                                         */
/*  pshints.h                                                              */
/*                                                                         */
/*    Interface to Postscript-specific (Type 1 and Type 2) hints           */
/*    recorders. These are used to support native T1/T2 hints              */
/*    in the "type1", "cid" and "cff" font drivers                         */
/*                                                                         */
/*  Copyright 2001 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

#ifndef __PSHINTS_H__
#define __PSHINTS_H__

#include <ft2build.h>
#include FT_TYPES_H
#include FT_INTERNAL_POSTSCRIPT_GLOBALS_H

FT_BEGIN_HEADER

 /**********************************************************************/
 /**********************************************************************/
 /*****                                                            *****/
 /*****               EXTERNAL REPRESENTATION OF GLOBALS           *****/
 /*****                                                            *****/
 /**********************************************************************/
 /**********************************************************************/

 /****************************************************************
  *
  * @constant: PS_GLOBALS_MAX_BLUE_ZONES
  *
  * @description:
  *   the maximum number of blue zones in a font global hints
  *   structure. See @PS_Globals_BluesRec
  */                                             
#define  PS_GLOBALS_MAX_BLUE_ZONES  16

 /****************************************************************
  *
  * @constant: PS_GLOBALS_MAX_STD_WIDTHS
  *
  * @description:
  *   the maximum number of standard and snap widths in either the
  *   horizontal or vertical direction. See @PS_Globals_WidthsRec
  */                                             
#define  PS_GLOBALS_MAX_STD_WIDTHS  16

 /****************************************************************
  *
  * @type: PS_Globals
  *
  * @description:
  *   a handle to a @PS_GlobalsRec structure used to
  *   describe the global hints of a given font
  */                                             
  typedef struct PS_GlobalsRec_*    PS_Globals;
                                             
 /****************************************************************
  *
  * @struct: PS_Globals_BluesRec
  *
  * @description:
  *   a structure used to model the global blue zones of a given
  *   font
  *
  * @fields:
  *   count        :: number of blue zones
  *   zones        :: an array of (count*2) coordinates describing the zones
  *
  *   count_family :: number of family blue zones
  *   zones_family :: an array of (count_family*2) coordinates describing
  *                   the family blue zones
  *
  *   scale  :: the blue scale to be used (fixed float)
  *   shift  :: the blue shift to be used
  *   fuzz   :: the blue fuzz to be used
  *
  * @note:
  *   each blue zone is modeled by a (reference,overshoot) coordinate pair
  *   in the table. zones can be placed in any order..
  */                                             
  typedef struct PS_Globals_BluesRec
  {                    
    FT_UInt    count;
    FT_Int16   zones[ 2*PS_GLOBALS_MAX_BLUE_ZONES ];
    
    FT_UInt    count_family;
    FT_Int16   zones_family[ 2*PS_GLOBALS_MAX_BLUE_ZONES ];

    FT_Fixed   scale;
    FT_Int16   shift;
    FT_Int16   fuzz;
  
  } PS_Globals_BluesRec, *PS_Globals_Blues;


 /****************************************************************
  *
  * @type: PS_Global_Widths;
  *
  * @description:
  *   a handle to a @PS_Globals_WidthsRec structure used to model
  *   the global standard and snap widths in a given direction
  */                                             
  typedef struct PS_Globals_WidthsRec_*  PS_Globals_Widths;
  

 /****************************************************************
  *
  * @struct: PS_Globals_WidthsRec
  *
  * @description:
  *   a structure used to model the global standard and snap widths
  *   in a given font
  *
  * @fields:
  *   count  :: number of widths
  *   widths :: an array of 'count' widths in font units.
  *
  * @note:
  *   'widths[0]' must be the standard width or height, while
  *   remaining elements of the array are snap widths or heights
  */                                             
  typedef struct PS_Globals_WidthsRec_
  {                     
    FT_UInt    count;
    FT_Int16   widths[ PS_GLOBALS_MAX_STD_WIDTHS ];
  
  } PS_Globals_WidthsRec;

  
 /****************************************************************
  *
  * @struct: PS_Globals_GlobalsRec
  *
  * @description:
  *   a structure used to model the global hints for a given font
  *
  * @fields:
  *   horizontal :: horizontal widths
  *   vertical   :: vertical heights
  *   blues      :: blue zones
  */                                             
  typedef struct PS_GlobalsRec_
  {
    PS_Globals_WidthsRec  horizontal;
    PS_Globals_WidthsRec  vertical;
    PS_Globals_BluesRec   blues;
  
  } PS_GlobalsRec;
 

 /**********************************************************************/
 /**********************************************************************/
 /*****                                                            *****/
 /*****               INTERNAL REPRESENTATION OF GLOBALS           *****/
 /*****                                                            *****/
 /**********************************************************************/
 /**********************************************************************/

  typedef struct PSH_GlobalsRec_*   PSH_Globals;
  
  typedef FT_Error  (*PSH_Globals_NewFunc)( FT_Memory     memory,
                                            PSH_Globals*  aglobals );

  typedef FT_Error  (*PSH_Globals_ResetFunc)( PSH_Globals   globals,
                                              PS_Globals    ps_globals );
  
  typedef FT_Error  (*PSH_Globals_SetScaleFunc)( PSH_Globals  globals,
                                                 FT_Fixed     x_scale,
                                                 FT_Fixed     y_scale,
                                                 FT_Fixed     x_delta,
                                                 FT_Fixed     y_delta );
                                              
  typedef void      (*PSH_Globals_DestroyFunc)( PSH_Globals  globals );                                          

  typedef struct
  {
    PSH_Globals_NewFunc       create;
    PSH_Globals_ResetFunc     reset;
    PSH_Globals_SetScaleFunc  set_scale;
    PSH_Globals_DestroyFunc   destroy;
    
  } PSH_Globals_FuncsRec, *PSH_Globals_Funcs;

 /**********************************************************************/
 /**********************************************************************/
 /*****                                                            *****/
 /*****                  PUBLIC TYPE 1 HINTS RECORDER              *****/
 /*****                                                            *****/
 /**********************************************************************/
 /**********************************************************************/

 /************************************************************************
  *
  * @type: T1_Hints
  *
  * @description:
  *   this is a handle to an opaque structure used to record glyph
  *   hints from a Type 1 character glyph character string.
  *
  *   the methods used to operate on this object are defined by the
  *   @T1_Hints_FuncsRec structure. Recording glyph hints is normally
  *   achieved through the following scheme:
  *
  *     - open a new hint recording session by calling the "open"
  *       method. This will rewind the recorder and prepare it for
  *       new input
  *
  *     - for each hint found in the glyph charstring, call the
  *       corresponding method ("stem", "stem3" or "reset").
  *       note that these functions do not return an error code
  *
  *     - close the recording session by calling the "close" method
  *       it will return an error code if the hints were invalid or
  *       something strange happened (e.g. memory shortage)
  *  
  *  the hints accumulated in the object can later be used by the
  *  Postscript hinter
  */
  typedef struct T1_HintsRec_*   T1_Hints;
  
 /************************************************************************
  *
  * @type: T1_Hints_Funcs
  *
  * @description:
  *   a pointer to the @T1_Hints_FuncsRec structure that defines the
  *   API of a given @T1_Hints object
  */
  typedef const struct T1_Hints_FuncsRec_*  T1_Hints_Funcs;
  

 /************************************************************************
  *
  * @functype: T1_Hints_OpenFunc
  *
  * @description:
  *   a method of the @T1_Hints class used to prepare it for a new
  *   Type 1 hints recording session
  *
  * @input:
  *   hints :: handle to Type 1 hints recorder
  *
  * @note:
  *  You should always call the @T1_Hints_CloseFunc method in order
  *  to close an opened recording session
  */
  typedef void      (*T1_Hints_OpenFunc)      ( T1_Hints  hints );

 /************************************************************************
  *
  * @functype: T1_Hints_SetStemFunc
  *
  * @description:
  *   a method of the @T1_Hints class used to record a new horizontal or
  *   vertical stem. This corresponds to the Type 1 "hstem" and "vstem"
  *   operators
  *
  * @input:
  *   hints     :: handle to Type 1 hints recorder
  *   dimension :: 0 for horizontal stems (hstem), 1 for vertical ones (vstem)
  *   coords    :: array of 2 integers, used as (position,length) stem descriptor
  *
  * @note:
  *   use vertical coordinates   (y) for horizontal stems (dim=0)
  *   use horizontal coordinates (x) for vertical   stems (dim=1)
  *
  *   "coords[0]" is the absolute stem position (lowest coordinate)
  *   "coords[1]" is the length.
  *
  *   the length can be negative, in which case it must be either
  *   -20 or -21 in order and will be interpreted as a "ghost" stem,
  *   according to the Type 1 specification.
  *
  *   if the length is -21 (corresponding to a bottom ghost stem), then
  *   the real stem position is "coords[0]+coords[1]"
  */
  typedef void      (*T1_Hints_SetStemFunc)   ( T1_Hints  hints,
                                                FT_UInt   dimension,
                                                FT_Int*   coords );

 /************************************************************************
  *
  * @functype: T1_Hints_SetStem3Func
  *
  * @description:
  *   a method of the @T1_Hints class used to record three counter-controlled
  *   horizontal or vertical stems at once
  *
  * @input:
  *   hints     :: handle to Type 1 hints recorder
  *   dimension :: 0 for horizontal stems, 1 for vertical ones
  *   coords    :: array of 6 integers, i.e. 3 (position,length) pairs
  *                for the counter-controlled stems
  *
  * @note:
  *   use vertical coordinates   (y) for horizontal stems (dim=0)
  *   use horizontal coordinates (x) for vertical   stems (dim=1)
  *
  *   the lengths cannot be negative (ghost stems are never counter-controlled)
  */
  typedef void      (*T1_Hints_SetStem3Func)  ( T1_Hints  hints,
                                                FT_UInt   dimension,
                                                FT_Int*   coords );

 /************************************************************************
  *
  * @functype: T1_Hints_ResetFunc
  *
  * @description:
  *   a method of the @T1_Hints class used to reset the stems hints
  *   in a recording session. This is equivalent to the Type 1 ...
  *
  * @input:
  *   hints     :: handle to Type 1 hints recorder
  *   end_point :: index of last point in the input glyph in which
  *                the previously defined hints apply
  */
  typedef void      (*T1_Hints_ResetFunc)( T1_Hints  hints,
                                           FT_UInt   end_point );

 /************************************************************************
  *
  * @functype: T1_Hints_CloseFunc
  *
  * @description:
  *   a method of the @T1_Hints class used to close a hint recording
  *   session.
  *
  * @input:
  *   hints     :: handle to Type 1 hints recorder
  *   end_point :: index of last point in the input glyph
  *
  * @return:
  *   error code. 0 means success
  *
  * @note:
  *   the error code will be set to indicate that an error occured
  *   during the recording session
  */
  typedef FT_Error  (*T1_Hints_CloseFunc)( T1_Hints  hints,
                                           FT_UInt   end_point );


 /************************************************************************
  *
  * @functype: T1_Hints_ApplyFunc
  *
  * @description:
  *   a method of the @T1_Hints class used to apply hints to the
  *   corresponding glyph outline. Must be called once all hints
  *   have been recorded.
  *
  * @input:
  *   hints     :: handle to Type 1 hints recorder
  *   outline   :: pointer to target outline descriptor
  *   globals   :: the hinter globals for this font
  *
  * @return:
  *   error code. 0 means success
  *
  * @note:
  *   on input, all points within the outline are in font coordinates.
  *   on output, they're in 1/64th of pixels.
  *
  *   the scaling transform is taken from the "globals" object, which
  *   must correspond to the same font than the glyph
  */
  typedef FT_Error  (*T1_Hints_ApplyFunc)( T1_Hints     hints,
                                           FT_Outline*  outline,
                                           PSH_Globals  globals );
  

 /************************************************************************
  *
  * @struct: T1_Hints_FuncsRec
  *
  * @description:
  *   the structure used to provide the API to @T1_Hints objects
  *
  * @fields:
  *   hints  :: handle to T1 Hints recorder
  *   open   :: open recording session
  *   close  :: close recording session
  *   stem   :: set simple stem
  *   stem3  :: set counter-controlled stems
  *   reset  :: reset stem hints
  *   apply  :: apply the hints to the corresponding glyph outline
  */
  typedef struct T1_Hints_FuncsRec_
  {
    T1_Hints                hints;
    T1_Hints_OpenFunc       open;
    T1_Hints_CloseFunc      close;
    T1_Hints_SetStemFunc    stem;
    T1_Hints_SetStem3Func   stem3;
    T1_Hints_ResetFunc      reset;
    T1_Hints_ApplyFunc      apply;
  
  } T1_Hints_FuncsRec;


 /**********************************************************************/
 /**********************************************************************/
 /*****                                                            *****/
 /*****                  PUBLIC TYPE 2 HINTS RECORDER              *****/
 /*****                                                            *****/
 /**********************************************************************/
 /**********************************************************************/


 /************************************************************************
  *
  * @type: T2_Hints
  *
  * @description:
  *   this is a handle to an opaque structure used to record glyph
  *   hints from a Type 2 character glyph character string.
  *
  *   the methods used to operate on this object are defined by the
  *   @T2_Hints_FuncsRec structure. Recording glyph hints is normally
  *   achieved through the following scheme:
  *
  *     - open a new hint recording session by calling the "open"
  *       method. This will rewind the recorder and prepare it for
  *       new input
  *
  *     - for each hint found in the glyph charstring, call the
  *       corresponding method ("stems", "hintmask", "counters").
  *       note that these functions do not return an error code
  *
  *     - close the recording session by calling the "close" method
  *       it will return an error code if the hints were invalid or
  *       something strange happened (e.g. memory shortage)
  *  
  *  the hints accumulated in the object can later be used by the
  *  Postscript hinter
  */
  typedef struct T2_HintsRec_*   T2_Hints;

 /************************************************************************
  *
  * @type: T2_Hints_Funcs
  *
  * @description:
  *   a pointer to the @T1_Hints_FuncsRec structure that defines the
  *   API of a given @T2_Hints object
  */
  typedef const struct T2_Hints_FuncsRec_*  T2_Hints_Funcs;

 /************************************************************************
  *
  * @functype: T2_Hints_OpenFunc
  *
  * @description:
  *   a method of the @T2_Hints class used to prepare it for a new
  *   Type 2 hints recording session
  *
  * @input:
  *   hints :: handle to Type 2 hints recorder
  *
  * @note:
  *  You should always call the @T2_Hints_CloseFunc method in order
  *  to close an opened recording session
  */
  typedef void      (*T2_Hints_OpenFunc)   ( T2_Hints  hints );  


 /************************************************************************
  *
  * @functype: T2_Hints_StemsFunc
  *
  * @description:
  *   a method of the @T2_Hints class used to set the table of stems
  *   in either the vertical or horizontal dimension. Equivalent to the
  *   "hstem", "vstem", "hstemhm" and "vstemhm" Type 2 operators
  *
  * @input:
  *   hints       :: handle to Type 2 hints recorder
  *   dimension   :: 0 for horizontal stems (hstem), 1 for vertical ones (vstem)
  *   count       :: number of stems
  *   coordinates :: an array of "count" (position,length) pairs
  *
  * @note:
  *   use vertical coordinates   (y) for horizontal stems (dim=0)
  *   use horizontal coordinates (x) for vertical   stems (dim=1)
  *
  *   there are "2*count" elements in the "coordinates" array. Each
  *   even element is an absolute position in font units, each odd
  *   element is a length in font units
  *
  *   a length can be negative, in which case it must be either
  *   -20 or -21 in order and will be interpreted as a "ghost" stem,
  *   according to the Type 1 specification.
  */
  typedef void      (*T2_Hints_StemsFunc)  ( T2_Hints   hints,
                                             FT_UInt    dimension,
                                             FT_UInt    count,
                                             FT_Fixed*  coordinates );
                                                
 /************************************************************************
  *
  * @functype: T2_Hints_MaskFunc
  *
  * @description:
  *   a method of the @T2_Hints class used to set a given hintmask
  *   (correspond to the "hintmask" Type 2 operator)
  *
  * @input:
  *   hints       :: handle to Type 2 hints recorder
  *   end_point   :: glyph index of the last point to which the previously
  *                  defined/active hints apply.
  *   bit_count   :: number of bits in the hint mask.
  *   bytes       :: an array of bytes modelling the hint mask
  *                  
  * @note:
  *   if the hintmask starts the charstring (before any glyph point
  *   definition), the value of "end_point" should be 0
  *
  *   "bit_count" is the number of meaningful bits in the "bytes" array,
  *   and must be equal to the total number of hints defined so far
  *   (i.e. horizontal+verticals)
  *
  *   the "bytes" array can come directly from the Type 2 charstring
  *   and respect the same format.
  */
  typedef void      (*T2_Hints_MaskFunc)   ( T2_Hints        hints,
                                             FT_UInt         end_point,
                                             FT_UInt         bit_count,
                                             const FT_Byte*  bytes );

 /************************************************************************
  *
  * @functype: T2_Hints_CounterFunc
  *
  * @description:
  *   a method of the @T2_Hints class used to set a given counter
  *   mask (correspond to the "hintmask" Type 2 operator)
  *
  * @input:
  *   hints       :: handle to Type 2 hints recorder
  *   end_point   :: glyph index of the last point to which the previously
  *                  defined/active hints apply.
  *   bit_count   :: number of bits in the hint mask.
  *   bytes       :: an array of bytes modelling the hint mask
  *                  
  * @note:
  *   if the hintmask starts the charstring (before any glyph point
  *   definition), the value of "end_point" should be 0
  *
  *   "bit_count" is the number of meaningful bits in the "bytes" array,
  *   and must be equal to the total number of hints defined so far
  *   (i.e. horizontal+verticals)
  *
  *   the "bytes" array can come directly from the Type 2 charstring
  *   and respect the same format.
  */
  typedef void      (*T2_Hints_CounterFunc)( T2_Hints        hints,
                                             FT_UInt         bit_count,
                                             const FT_Byte*  bytes );

 /************************************************************************
  *
  * @functype: T2_Hints_CloseFunc
  *
  * @description:
  *   a method of the @T2_Hints class used to close a hint recording
  *   session.
  *
  * @input:
  *   hints     :: handle to Type 2 hints recorder
  *   end_point :: index of last point in the input glyph
  *
  * @return:
  *   error code. 0 means success
  *
  * @note:
  *   the error code will be set to indicate that an error occured
  *   during the recording session
  */
  typedef FT_Error  (*T2_Hints_CloseFunc)  ( T2_Hints        hints,
                                             FT_UInt         end_point );


 /************************************************************************
  *
  * @functype: T2_Hints_ApplyFunc
  *
  * @description:
  *   a method of the @T2_Hints class used to apply hints to the
  *   corresponding glyph outline. Must be called after the "close" method
  *
  * @input:
  *   hints     :: handle to Type 2 hints recorder
  *   outline   :: pointer to target outline descriptor
  *   globals   :: the hinter globals for this font
  *
  * @return:
  *   error code. 0 means success
  *
  * @note:
  *   on input, all points within the outline are in font coordinates.
  *   on output, they're in 1/64th of pixels.
  *
  *   the scaling transform is taken from the "globals" object, which
  *   must correspond to the same font than the glyph
  */
  typedef FT_Error  (*T2_Hints_ApplyFunc)( T2_Hints     hints,
                                           FT_Outline*  outline,
                                           PSH_Globals  globals );
  

 /************************************************************************
  *
  * @struct: T2_Hints_FuncsRec
  *
  * @description:
  *   the structure used to provide the API to @T2_Hints objects
  *
  * @fields:
  *   hints    :: handle to T2 hints recorder object
  *   open     :: open recording session
  *   close    :: close recording session
  *   stems    :: set dimension's stems table
  *   hintmask :: set hint masks
  *   counter  :: set counter masks
  *   apply    :: apply the hints on the corresponding glyph outline
  */
  typedef struct T2_Hints_FuncsRec_
  {
    T2_Hints              hints;
    T2_Hints_OpenFunc     open;
    T2_Hints_CloseFunc    close;
    T2_Hints_StemsFunc    stems;
    T2_Hints_MaskFunc     hintmask;
    T2_Hints_CounterFunc  counter;
    T2_Hints_ApplyFunc    apply;
  
  } T2_Hints_FuncsRec;


  /* */
  
  typedef struct PSHinter_Interface_
  {
    PSH_Globals_Funcs  (*get_globals_funcs)( FT_Module  module );
    T1_Hints_Funcs     (*get_t1_funcs)     ( FT_Module  module );
    T2_Hints_Funcs     (*get_t2_funcs)     ( FT_Module  module );
    
  } PSHinter_Interface, *PSHinter_InterfacePtr;

FT_END_HEADER

#endif /* __PSHINTS_H__ */
