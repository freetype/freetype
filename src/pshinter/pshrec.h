/***************************************************************************/
/*                                                                         */
/*  pshrec.h                                                               */
/*                                                                         */
/*    Postscript (Type1/Type2) hints recorder.                             */
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
/*                                                                         */
/*  The functions defined here are called from the Type 1, CID and CFF     */
/*  font drivers to record the hints of a given character/glyph.           */
/*                                                                         */
/*  The hints are recorded in a unified format, and are later processed    */
/*  by the "optimiser" and "fitter" to adjust the outlines to the pixel    */
/*  grid.                                                                  */
/*                                                                         */
/***************************************************************************/

#ifndef __PS_HINTER_RECORD_H__
#define __PS_HINTER_RECORD_H__

#include <ft2build.h>
#include FT_INTERNAL_POSTSCRIPT_HINTS_H
#include "pshglob.h"

FT_BEGIN_HEADER

 /**********************************************************************/
 /**********************************************************************/
 /*****                                                            *****/
 /*****                 GLYPH HINTS RECORDER INTERNALS             *****/
 /*****                                                            *****/
 /**********************************************************************/
 /**********************************************************************/

 /* handle to hint record */
  typedef struct PS_HintRec_*   PS_Hint;

 /* hint types */
 typedef enum
 {
   PS_HINT_TYPE_1 = 1,
   PS_HINT_TYPE_2 = 2
   
 } PS_Hint_Type;
 

 /* hint flags */
  typedef enum
  {
    PS_HINT_FLAG_GHOST  = 1,
    PS_HINT_FLAG_BOTTOM = 2

  } PS_Hint_Flags;


 /* hint descriptor */
  typedef struct PS_HintRec_
  {
    FT_Int    pos;
    FT_Int    len;
    FT_UInt   flags;

  } PS_HintRec;


#define  ps_hint_is_active(x)  ((x)->flags & PS_HINT_FLAG_ACTIVE)
#define  ps_hint_is_ghost(x)   ((x)->flags & PS_HINT_FLAG_GHOST)
#define  ps_hint_is_bottom(x)  ((x)->flags & PS_HINT_FLAG_BOTTOM)


 /* hints table descriptor */
  typedef struct PS_Hint_TableRec_
  {
    FT_UInt   num_hints;
    FT_UInt   max_hints;
    PS_Hint   hints;

  } PS_Hint_TableRec, *PS_Hint_Table;


 /* hint and counter mask descriptor */
  typedef struct PS_MaskRec_
  {
    FT_UInt    num_bits;
    FT_UInt    max_bits;
    FT_Byte*   bytes;
    FT_UInt    end_point;

  } PS_MaskRec, *PS_Mask;


 /* masks and counters table descriptor */
  typedef struct PS_Mask_TableRec_
  {
    FT_UInt    num_masks;
    FT_UInt    max_masks;
    PS_Mask    masks;

  } PS_Mask_TableRec, *PS_Mask_Table;


 /* dimension-specific hints descriptor */
  typedef struct PS_DimensionRec_
  {
    PS_Hint_TableRec  hints;
    PS_Mask_TableRec  masks;
    PS_Mask_TableRec  counters;

  } PS_DimensionRec, *PS_Dimension;


 /* magic value used within PS_HintsRec */
#define PS_HINTS_MAGIC  0x68696e74   /* "hint" */


 /* glyph hints descriptor */
  typedef struct PS_HintsRec_
  {
    FT_Memory          memory;
    FT_Error           error;
    FT_UInt32          magic;
    PS_Hint_Type       hint_type;
    PS_DimensionRec    dimension[2];

  } PS_HintsRec, *PS_Hints;

 /* */

 /* initialise hints recorder */
  FT_LOCAL FT_Error
  ps_hints_init( PS_Hints   hints,
                 FT_Memory  memory );

 /* finalize hints recorder */
  FT_LOCAL void
  ps_hints_done( PS_Hints  hints );

 /* initialise Type1 hints recorder interface */  
  FT_LOCAL void
  t1_hints_funcs_init( T1_Hints_FuncsRec*  funcs );
  
 /* initialise Type2 hints recorder interface */
  FT_LOCAL void
  t2_hints_funcs_init( T2_Hints_FuncsRec*  funcs );

 /* */
 
FT_END_HEADER

#endif /* __PS_HINTER_RECORD_H__ */
