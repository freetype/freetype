/***************************************************************************/
/*                                                                         */
/*  ttinterp.h                                                             */
/*                                                                         */
/*    TrueType bytecode interpreter (specification).                       */
/*                                                                         */
/*  Copyright 1996-2000 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef TTINTERP_H
#define TTINTERP_H

#include <ttobjs.h>


#ifdef __cplusplus
  extern "C" {
#endif


#ifndef TT_CONFIG_OPTION_STATIC_INTEPRETER  /* indirect implementation */

#define EXEC_OP_   TT_ExecContext  exc,
#define EXEC_OP    TT_ExecContext  exc
#define EXEC_ARG_  exc,
#define EXEC_ARG   exc

#else                                       /* static implementation */

#define EXEC_OP_   /* void */
#define EXEC_OP    /* void */
#define EXEC_ARG_  /* void */
#define EXEC_ARG   /* void */

#endif /* TT_CONFIG_OPTION_STATIC_INTERPRETER */


  /*************************************************************************/
  /*                                                                       */
  /* Rounding mode constants.                                              */
  /*                                                                       */
#define TT_Round_Off             5
#define TT_Round_To_Half_Grid    0
#define TT_Round_To_Grid         1
#define TT_Round_To_Double_Grid  2
#define TT_Round_Up_To_Grid      4
#define TT_Round_Down_To_Grid    3
#define TT_Round_Super           6
#define TT_Round_Super_45        7


  /*************************************************************************/
  /*                                                                       */
  /* Function types used by the interpreter, depending on various modes    */
  /* (e.g. the rounding mode, whether to render a vertical or horizontal   */
  /* line etc).                                                            */
  /*                                                                       */
  /*************************************************************************/

  /* Rounding function */
  typedef TT_F26Dot6  (*TT_Round_Func)( EXEC_OP_ TT_F26Dot6  distance,
                                                 TT_F26Dot6  compensation );

  /* Point displacement along the freedom vector routine */
  typedef void  (*TT_Move_Func)( EXEC_OP_ FT_GlyphZone*  zone,
                                          TT_UInt        point,
                                          TT_F26Dot6     distance );

  /* Distance projection along one of the projection vectors */
  typedef TT_F26Dot6  (*TT_Project_Func)( EXEC_OP_ TT_Vector*  v1,
                                                   TT_Vector*  v2 );

  /* reading a cvt value.  Take care of non-square pixels if necessary */
  typedef TT_F26Dot6  (*TT_Get_CVT_Func)( EXEC_OP_ TT_ULong  index );

  /* setting or moving a cvt value.  Take care of non-square pixels  */
  /* if necessary                                                    */
  typedef void  (*TT_Set_CVT_Func)( EXEC_OP_ TT_ULong    index,
                                             TT_F26Dot6  value );


  /*************************************************************************/
  /*                                                                       */
  /* This structure defines a call record, used to manage function calls.  */
  /*                                                                       */
  typedef struct  TT_CallRec_
  {
    TT_Int   Caller_Range;
    TT_Long  Caller_IP;
    TT_Long  Cur_Count;
    TT_Long  Cur_Restart;

  } TT_CallRec, *TT_CallStack;


  /*************************************************************************/
  /*                                                                       */
  /* The main structure for the interpreter which collects all necessary   */
  /* variables and states.                                                 */
  /*                                                                       */
  typedef struct  TT_ExecContextRec_
  {
    TT_Face         face;
    TT_Size         size;
    FT_Memory       memory;

    /* instructions state */

    TT_Error        error;      /* last execution error */

    TT_Long         top;        /* top of exec. stack   */

    TT_UInt         stackSize;  /* size of exec. stack  */
    TT_Long*        stack;      /* current exec. stack  */

    TT_Long         args;
    TT_UInt         new_top;    /* new top after exec.  */

    FT_GlyphZone    zp0,        /* zone records */
                    zp1,
                    zp2,
                    pts,
                    twilight;

    FT_Size_Metrics  metrics;
    TT_Size_Metrics  tt_metrics; /* size metrics */

    TT_GraphicsState  GS;         /* current graphics state */

    TT_Int          curRange;  /* current code range number   */
    TT_Byte*        code;      /* current code range          */
    TT_Long         IP;        /* current instruction pointer */
    TT_Long         codeSize;  /* size of current range       */

    TT_Byte         opcode;    /* current opcode              */
    TT_Int          length;    /* length of current opcode    */

    TT_Bool         step_ins;  /* true if the interpreter must */
                               /* increment IP after ins. exec */
    TT_Long         cvtSize;
    TT_Long*        cvt;

    TT_UInt         glyphSize; /* glyph instructions buffer size */
    TT_Byte*        glyphIns;  /* glyph instructions buffer */

    TT_UInt         numFDefs;  /* number of function defs         */
    TT_UInt         maxFDefs;  /* maximum number of function defs */
    TT_DefArray     FDefs;     /* table of FDefs entries          */

    TT_UInt         numIDefs;  /* number of instruction defs */
    TT_UInt         maxIDefs;  /* maximum number of ins defs */
    TT_DefArray     IDefs;     /* table of IDefs entries     */

    TT_UInt         maxFunc;   /* maximum function index     */
    TT_UInt         maxIns;    /* maximum instruction index  */

    TT_Int          callTop,    /* top of call stack during execution */
                    callSize;   /* size of call stack */
    TT_CallStack    callStack;  /* call stack */

    TT_UShort       maxPoints;    /* capacity of this context's `pts' */
    TT_Short        maxContours;  /* record, expressed in points and  */
                                  /* contours.                        */

    TT_CodeRangeTable  codeRangeTable;  /* table of valid code ranges */
                                        /* useful for the debugger   */

    TT_UShort       storeSize;  /* size of current storage */
    TT_Long*        storage;    /* storage area            */

    TT_F26Dot6      period;     /* values used for the */
    TT_F26Dot6      phase;      /* `SuperRounding'     */
    TT_F26Dot6      threshold;

    /* this seems to be unused */
#if 0
    TT_Int          cur_ppem;       /* ppem along the current proj vector */
#endif
    TT_Bool         instruction_trap;  /* If `True', the interpreter will */
                                       /* exit after each instruction     */

    TT_GraphicsState  default_GS;      /* graphics state resulting from    */
                                       /* the prep program                 */
    TT_Bool          is_composite;     /* true if the glyph is composite   */
    TT_Bool          pedantic_hinting; /* true for pedantic interpretation */

    /* latest interpreter additions */

    TT_Long            F_dot_P;    /* dot product of freedom and projection */
                                   /* vectors                               */
    TT_Round_Func      func_round; /* current rounding function             */

    TT_Project_Func    func_project,   /* current projection function */
                       func_dualproj,  /* current dual proj. function */
                       func_freeProj;  /* current freedom proj. func  */

    TT_Move_Func       func_move;      /* current point move function */

    TT_Get_CVT_Func    func_read_cvt;  /* read a cvt entry              */
    TT_Set_CVT_Func    func_write_cvt; /* write a cvt entry (in pixels) */
    TT_Set_CVT_Func    func_move_cvt;  /* incr a cvt entry (in pixels)  */

    TT_ULong           loadSize;
    TT_SubGlyph_Stack  loadStack;      /* loading subglyph stack */

  } TT_ExecContextRec;


  LOCAL_DEF
  const TT_GraphicsState  tt_default_graphics_state;


  LOCAL_DEF
  TT_Error  TT_Goto_CodeRange( TT_ExecContext  exec,
                               TT_Int          range,
                               TT_Long         IP );

  LOCAL_DEF
  TT_Error  TT_Set_CodeRange( TT_ExecContext  exec,
                              TT_Int          range,
                              void*           base,
                              TT_Long         length );

  LOCAL_DEF
  TT_Error  TT_Clear_CodeRange( TT_ExecContext  exec,
                                TT_Int          range );

  FT_EXPORT_DEF( TT_ExecContext )  TT_New_Context( TT_Face  face );

  LOCAL_DEF
  TT_Error  TT_Done_Context( TT_ExecContext  exec );

  LOCAL_DEF
  TT_Error  TT_Destroy_Context( TT_ExecContext  exec,
                                FT_Memory       memory );

  LOCAL_DEF
  TT_Error  TT_Load_Context( TT_ExecContext  exec,
                             TT_Face         face,
                             TT_Size         size );

  LOCAL_DEF
  TT_Error  TT_Save_Context( TT_ExecContext  exec,
                             TT_Size         ins );

  LOCAL_DEF
  TT_Error  TT_Run_Context( TT_ExecContext  exec,
                            TT_Bool         debug );

  FT_EXPORT_DEF( TT_Error )  TT_RunIns( TT_ExecContext  exec );


#ifdef __cplusplus
  }
#endif

#endif /* TTINTERP_H */


/* END */
