/***************************************************************************/
/*                                                                         */
/*  pshfit.h                                                               */
/*                                                                         */
/*    Postscript (Type 1/CFF) outline grid-fitter                          */
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
/*  process the hints provided by the Postscript hints recorder. This      */
/*  means sorting and scaling the hints, calling the "optimiser" to        */
/*  process them, then grid-fitting the glyph outline based on the         */
/*  optimised hints information.                                           */
/*                                                                         */
/*  (the real hinting "intelligence" is in "pshoptim.h", not here)         */
/*                                                                         */
/***************************************************************************/

#ifndef __PS_HINTER_FITTER_H__
#define __PS_HINTER_FITTER_H__

#include "pshrec.h"

FT_BEGIN_HEADER

  typedef struct PSH_HintRec_*   PSH_Hint;

  typedef enum
  {
    PSH_HINT_FLAG_GHOST  = PS_HINT_FLAG_GHOST,
    PSH_HINT_FLAG_BOTTOM = PS_HINT_FLAG_BOTTOM,
    PSH_HINT_FLAG_ACTIVE = 4
  
  } PSH_Hint_Flags;

#define  psh_hint_is_active(x)  (((x)->flags  & PSH_HINT_FLAG_ACTIVE) != 0)
#define  psh_hint_is_ghost(x)   (((x)->flags  & PSH_HINT_FLAG_GHOST ) != 0)  

#define  psh_hint_activate(x)     (x)->flags |= PSH_HINT_FLAG_ACTIVE
#define  psh_hint_deactivate(x)   (x)->flags &= ~PSH_HINT_FLAG_ACTIVE

  typedef struct PSH_HintRec_
  {
    FT_Int    org_pos;
    FT_Int    org_len;
    FT_Pos    cur_pos;
    FT_Pos    cur_len;
    
    FT_UInt   flags;
    
    PSH_Hint  parent;
    FT_Int    order;
  
  } PSH_HintRec;


 /* this is an interpolation zone used for strong points   */
 /* weak points are interpolated according to their strong */
 /* neighbours..                                           */
  typedef struct PSH_ZoneRec_
  {
    FT_Fixed  scale;
    FT_Fixed  delta;
    FT_Pos    min;
    FT_Pos    max;
    
  } PSH_ZoneRec, *PSH_Zone;


  typedef struct PSH_Hint_TableRec_
  {
    FT_UInt        max_hints;
    FT_UInt        num_hints;
    PSH_Hint       hints;
    PSH_Hint*      sort;
    PSH_Hint*      sort_global;
    FT_UInt        num_zones;
    PSH_Zone       zones;
    PSH_Zone       zone;
    PS_Mask_Table  hint_masks;
    PS_Mask_Table  counter_masks;
    
  } PSH_Hint_TableRec, *PSH_Hint_Table;


  FT_LOCAL FT_Error
  ps_hints_apply( PS_Hints     ps_hints,
                  FT_Outline*  outline,
                  PSH_Globals  globals );


#ifdef DEBUG_VIEW
  extern  int             ps_debug_no_horz_hints;
  extern  int             ps_debug_no_vert_hints;
  extern  PSH_Hint_Table  ps_debug_hint_table;

  typedef void  (*PSH_HintFunc)( PSH_Hint  hint, FT_Bool vertical );
  extern  PSH_HintFunc    ps_debug_hint_func;
#endif

FT_END_HEADER

#endif /* __PS_HINTER_FITTER_H__ */
