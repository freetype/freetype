/***************************************************************************/
/*                                                                         */
/*  ttobjs.h                                                               */
/*                                                                         */
/*    Objects manager (specification).                                     */
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


#ifndef TTOBJS_H
#define TTOBJS_H


#include <ftobjs.h>
#include <tttypes.h>
#include <tterrors.h>


#ifdef __cplusplus
  extern "C" {
#endif


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    TT_Driver                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to a TrueType driver object.                              */
  /*                                                                       */
  typedef struct TT_DriverRec_*  TT_Driver;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    TT_Instance                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to a TrueType size object.                                */
  /*                                                                       */
  typedef struct TT_SizeRec_*  TT_Size;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    TT_GlyphSlot                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to a TrueType glyph slot object.                          */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This is a direct typedef of FT_GlyphSlot, as there is nothing      */
  /*    specific about the TrueType glyph slot.                            */
  /*                                                                       */
  typedef FT_GlyphSlot  TT_GlyphSlot;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_GraphicsState                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The TrueType graphics state used during bytecode interpretation.   */
  /*                                                                       */
  typedef struct  TT_GraphicsState_
  {
    TT_UShort      rp0;
    TT_UShort      rp1;
    TT_UShort      rp2;

    TT_UnitVector  dualVector;
    TT_UnitVector  projVector;
    TT_UnitVector  freeVector;

    TT_Long        loop;
    TT_F26Dot6     minimum_distance;
    TT_Int         round_state;

    TT_Bool        auto_flip;
    TT_F26Dot6     control_value_cutin;
    TT_F26Dot6     single_width_cutin;
    TT_F26Dot6     single_width_value;
    TT_Short       delta_base;
    TT_Short       delta_shift;

    TT_Byte        instruct_control;
    TT_Bool        scan_control;
    TT_Int         scan_type;

    TT_UShort      gep0;
    TT_UShort      gep1;
    TT_UShort      gep2;

  } TT_GraphicsState;


  /*************************************************************************/
  /*                                                                       */
  /*  EXECUTION SUBTABLES                                                  */
  /*                                                                       */
  /*  These sub-tables relate to instruction execution.                    */
  /*                                                                       */
  /*************************************************************************/


#define TT_MAX_CODE_RANGES  3


  /*************************************************************************/
  /*                                                                       */
  /* There can only be 3 active code ranges at once:                       */
  /*   - the Font Program                                                  */
  /*   - the CVT Program                                                   */
  /*   - a glyph's instructions set                                        */
  /*                                                                       */
  typedef enum  TT_CodeRange_Tag_
  {
    tt_coderange_none = 0,
    tt_coderange_font,
    tt_coderange_cvt,
    tt_coderange_glyph

  } TT_CodeRange_Tag;


  typedef struct  TT_CodeRange_
  {
    TT_Byte*  base;
    TT_ULong  size;

  } TT_CodeRange;

  typedef TT_CodeRange  TT_CodeRangeTable[TT_MAX_CODE_RANGES];


  /*************************************************************************/
  /*                                                                       */
  /* Defines a function/instruction definition record.                     */
  /*                                                                       */
  typedef struct  TT_DefRecord_
  {
    TT_Int   range;      /* in which code range is it located? */
    TT_Long  start;      /* where does it start?               */
    TT_UInt  opc;        /* function #, or instruction code    */
    TT_Bool  active;     /* is it active?                      */

  } TT_DefRecord, *TT_DefArray;



  /*************************************************************************/
  /*                                                                       */
  /* Subglyph transformation record.                                       */
  /*                                                                       */
  typedef struct  TT_Transform_
  {
    TT_Fixed    xx, xy;     /* transformation matrix coefficients */
    TT_Fixed    yx, yy;
    TT_F26Dot6  ox, oy;     /* offsets        */

  } TT_Transform;


  /*************************************************************************/
  /*                                                                       */
  /* Subglyph loading record.  Used to load composite components.          */
  /*                                                                       */
  typedef struct  TT_SubglyphRec_
  {
    TT_Long       index;        /* subglyph index; initialized with -1 */
    TT_Bool       is_scaled;    /* is the subglyph scaled?             */
    TT_Bool       is_hinted;    /* should it be hinted?                */
    TT_Bool       preserve_pps; /* preserve phantom points?            */

    TT_Long       file_offset;

    TT_BBox       bbox;
    TT_Pos        left_bearing;
    TT_Pos        advance;

    FT_GlyphZone  zone;

    TT_Long       arg1;         /* first argument                      */
    TT_Long       arg2;         /* second argument                     */

    TT_UShort     element_flag; /* current load element flag           */

    TT_Transform  transform;    /* transformation matrix               */

    TT_Vector     pp1, pp2;     /* phantom points                      */

  } TT_SubGlyphRec, *TT_SubGlyph_Stack;


  /*************************************************************************/
  /*                                                                       */
  /* A note regarding non-squared pixels:                                  */
  /*                                                                       */
  /* (This text will probably go into some docs at some time, for now, it  */
  /*  is kept here to explain some definitions in the TIns_Metrics         */
  /*  record).                                                             */
  /*                                                                       */
  /* The CVT is a one-dimensional array containing values that control     */
  /* certain important characteristics in a font, like the height of all   */
  /* capitals, all lowercase letter, default spacing or stem width/height. */
  /*                                                                       */
  /* These values are found in FUnits in the font file, and must be scaled */
  /* to pixel coordinates before being used by the CVT and glyph programs. */
  /* Unfortunately, when using distinct x and y resolutions (or distinct x */
  /* and y pointsizes), there are two possible scalings.                   */
  /*                                                                       */
  /* A first try was to implement a `lazy' scheme where all values were    */
  /* scaled when first used.  However, while some values are always used   */
  /* in the same direction, some others are used under many different      */
  /* circumstances and orientations.                                       */
  /*                                                                       */
  /* I have found a simpler way to do the same, and it even seems to work  */
  /* in most of the cases:                                                 */
  /*                                                                       */
  /* - All CVT values are scaled to the maximum ppem size.                 */
  /*                                                                       */
  /* - When performing a read or write in the CVT, a ratio factor is used  */
  /*   to perform adequate scaling.  Example:                              */
  /*                                                                       */
  /*     x_ppem = 14                                                       */
  /*     y_ppem = 10                                                       */
  /*                                                                       */
  /*   We choose ppem = x_ppem = 14 as the CVT scaling size.  All cvt      */
  /*   entries are scaled to it.                                           */
  /*                                                                       */
  /*     x_ratio = 1.0                                                     */
  /*     y_ratio = y_ppem/ppem (< 1.0)                                     */
  /*                                                                       */
  /*   We compute the current ratio like:                                  */
  /*                                                                       */
  /*   - If projVector is horizontal,                                      */
  /*       ratio = x_ratio = 1.0                                           */
  /*                                                                       */
  /*   - if projVector is vertical,                                        */
  /*       ratio = y_ratio                                                 */
  /*                                                                       */
  /*   - else,                                                             */
  /*       ratio = sqrt( (proj.x * x_ratio) ^ 2 + (proj.y * y_ratio) ^ 2 ) */
  /*                                                                       */
  /*   Reading a cvt value returns                                         */
  /*     ratio * cvt[index]                                                */
  /*                                                                       */
  /*   Writing a cvt value in pixels:                                      */
  /*     cvt[index] / ratio                                                */
  /*                                                                       */
  /*   The current ppem is simply                                          */
  /*     ratio * ppem                                                      */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* Metrics used by the TrueType size and context objects.                */
  /*                                                                       */
  typedef struct  TT_Size_Metrics_
  {
    /* for non-square pixels */
    TT_Long     x_ratio;
    TT_Long     y_ratio;

    TT_UShort   ppem;               /* maximum ppem size              */
    TT_Long     ratio;              /* current ratio                  */
    TT_Fixed    scale;

    TT_F26Dot6  compensations[4];   /* device-specific compensations  */

    TT_Bool     valid;

    TT_Bool     rotated;            /* `is the glyph rotated?'-flag   */
    TT_Bool     stretched;          /* `is the glyph stretched?'-flag */

  } TT_Size_Metrics;


  /*************************************************************************/
  /*                                                                       */
  /* FreeType execution context type.                                      */
  /*                                                                       */
  /* This is a forward declaration; the full specification is in the file  */
  /* `ttinterp.h'.                                                         */
  /*                                                                       */
  typedef struct TT_ExecContextRec_*  TT_ExecContext;


  /***********************************************************************/
  /*                                                                     */
  /* TrueType size class.                                                */
  /*                                                                     */
  typedef struct  TT_SizeRec_
  {
    FT_SizeRec         root;

    TT_Size_Metrics    ttmetrics;

#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER
    TT_UInt            num_function_defs; /* number of function definitions */
    TT_UInt            max_function_defs;
    TT_DefArray        function_defs;     /* table of function definitions  */

    TT_UInt            num_instruction_defs;  /* number of ins. definitions */
    TT_UInt            max_instruction_defs;
    TT_DefArray        instruction_defs;      /* table of ins. definitions  */

    TT_UInt            max_func;
    TT_UInt            max_ins;

    TT_CodeRangeTable  codeRangeTable;

    TT_GraphicsState   GS;

    TT_ULong           cvt_size;      /* the scaled control value table */
    TT_Long*           cvt;

    TT_UShort          storage_size; /* The storage area is now part of */
    TT_Long*           storage;      /* the instance                    */

    FT_GlyphZone       twilight;     /* The instance's twilight zone    */

    /* debugging variables */

    /* When using the debugger, we must keep the */
    /* execution context tied to the instance    */
    /* object rather than asking it on demand.   */

    TT_Bool            debug;
    TT_ExecContext     context;

#endif /* TT_CONFIG_OPTION_BYTECODE_INTERPRETER */

  } TT_SizeRec;


  /***********************************************************************/
  /*                                                                     */
  /* TrueType driver class.                                              */
  /*                                                                     */
  typedef struct  TT_DriverRec_
  {
    FT_DriverRec    root;
    TT_ExecContext  context;  /* execution context        */
    FT_GlyphZone    zone;     /* glyph loader points zone */

    void*           extension_component;

  } TT_DriverRec;


 /*************************************************************************/
 /*  Face Funcs                                                           */ 

  LOCAL_DEF TT_Error  TT_Init_Face( FT_Stream  stream,
                                    TT_Long    face_index,
                                    TT_Face    face );

  LOCAL_DEF void      TT_Done_Face( TT_Face  face );


 /*************************************************************************/
 /*  Size funcs                                                           */ 

  LOCAL_DEF TT_Error  TT_Init_Size ( TT_Size  size );
  LOCAL_DEF void      TT_Done_Size ( TT_Size  size );
  LOCAL_DEF TT_Error  TT_Reset_Size( TT_Size  size );


 /*************************************************************************/
 /*  GlyphSlot funcs                                                      */ 

  LOCAL_DEF TT_Error  TT_Init_GlyphSlot( TT_GlyphSlot  slot );
  LOCAL_DEF void      TT_Done_GlyphSlot( TT_GlyphSlot  slot );


 /*************************************************************************/
 /*  Driver funcs                                                         */ 

  LOCAL_DEF  TT_Error  TT_Init_Driver( TT_Driver  driver );
  LOCAL_DEF  void      TT_Done_Driver( TT_Driver  driver );


#ifdef __cplusplus
  }
#endif


#endif /* TTOBJS_H */


/* END */
