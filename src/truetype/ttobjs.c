/***************************************************************************/
/*                                                                         */
/*  ttobjs.c                                                               */
/*                                                                         */
/*    Objects manager (body).                                              */
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


#include <freetype/internal/ftdebug.h>
#include <freetype/internal/ftcalc.h>
#include <freetype/internal/ftstream.h>
#include <freetype/ttnameid.h>
#include <freetype/tttags.h>

#include <freetype/internal/sfnt.h>
#include <freetype/internal/psnames.h>
#include <ttobjs.h>

#include <ttpload.h>
#include <freetype/internal/tterrors.h>

#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER
#include <ttinterp.h>
#endif


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
#undef   FT_COMPONENT
#define  FT_COMPONENT  trace_ttobjs


  /*************************************************************************/
  /*                                                                       */
  /*                       GLYPH ZONE FUNCTIONS                            */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Init_Face                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Initializes a given TrueType face object.                          */
  /*                                                                       */
  /* <Input>                                                               */
  /*    stream     :: The source font stream.                              */
  /*                                                                       */
  /*    face_index :: The index of the font face in the resource.          */
  /*                                                                       */
  /*    num_params :: Number of additional generic parameters.  Ignored.   */
  /*                                                                       */
  /*    params     :: Additional generic parameters.  Ignored.             */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    face       :: The newly built face object.                         */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_DEF
  TT_Error  TT_Init_Face( FT_Stream      stream,
                          TT_Face        face,
                          TT_Int         face_index,
                          TT_Int         num_params,
                          FT_Parameter*  params )
  {
    TT_Error         error;
    FT_Driver        sfnt_driver;
    SFNT_Interface*  sfnt;


    sfnt_driver = FT_Get_Driver( face->root.driver->library, "sfnt" );
    if ( !sfnt_driver )
      goto Bad_Format;

    sfnt = (SFNT_Interface*)(sfnt_driver->interface.format_interface);
    if ( !sfnt )
      goto Bad_Format;

    /* create input stream from resource */
    if ( FILE_Seek( 0 ) )
      goto Exit;

    /* check that we have a valid TrueType file */
    error = sfnt->init_face( stream, face, face_index, num_params, params );
    if ( error )
      goto Exit;

    /* We must also be able to accept Mac/GX fonts, as well as OT ones */
    if ( face->format_tag != 0x00010000L &&    /* MS fonts  */
         face->format_tag != TTAG_true   )     /* Mac fonts */
    {
      FT_TRACE2(( "[not a valid TTF font]\n" ));
      goto Bad_Format;
    }

    /* If we are performing a simple font format check, exit immediately */
    if ( face_index < 0 )
      return TT_Err_Ok;

    /* Load font directory */
    error = sfnt->load_face( stream, face, face_index, num_params, params );
    if ( error )
      goto Exit;

    error = TT_Load_Locations( face, stream ) ||
            TT_Load_CVT      ( face, stream ) ||
            TT_Load_Programs ( face, stream );

  Exit:
    return error;
     
  Bad_Format:
    error = FT_Err_Unknown_File_Format;
    goto Exit;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Done_Face                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Finalizes a given face object.                                     */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face :: A pointer to the face object to destroy.                   */
  /*                                                                       */
  LOCAL_DEF
  void  TT_Done_Face( TT_Face  face )
  {
    FT_Memory  memory = face->root.memory;
    FT_Stream  stream = face->root.stream;

    SFNT_Interface*  sfnt = face->sfnt;


    if ( sfnt )
      sfnt->done_face( face );

    /* freeing the locations table */
    FREE( face->glyph_locations );
    face->num_locations = 0;

    /* freeing the CVT */
    FREE( face->cvt );
    face->cvt_size = 0;

    /* freeing the programs */
    RELEASE_Frame( face->font_program );
    RELEASE_Frame( face->cvt_program );
    face->font_program_size = 0;
    face->cvt_program_size  = 0;
  }


  /*************************************************************************/
  /*                                                                       */
  /*                           SIZE  FUNCTIONS                             */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Init_Size                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Initializes a new TrueType size object.                            */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    size :: A handle to the size object.                               */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_DEF
  TT_Error  TT_Init_Size( TT_Size  size )
  {
    TT_Error   error = 0;

#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER

    TT_Face    face   = (TT_Face)size->root.face;
    FT_Memory  memory = face->root.memory;
    TT_Int     i;

    TT_ExecContext  exec;
    TT_UShort       n_twilight;
    TT_MaxProfile*  maxp = &face->max_profile;


    size->ttmetrics.valid = FALSE;

    size->max_function_defs    = maxp->maxFunctionDefs;
    size->max_instruction_defs = maxp->maxInstructionDefs;

    size->num_function_defs    = 0;
    size->num_instruction_defs = 0;

    size->max_func = 0;
    size->max_ins  = 0;

    size->cvt_size     = face->cvt_size;
    size->storage_size = maxp->maxStorage;

    /* Set default metrics */
    {
      FT_Size_Metrics*  metrics  = &size->root.metrics;
      TT_Size_Metrics*  metrics2 = &size->ttmetrics;


      metrics->x_ppem = 0;
      metrics->y_ppem = 0;

      metrics2->rotated   = FALSE;
      metrics2->stretched = FALSE;

      /* set default compensation (all 0) */
      for ( i = 0; i < 4; i++ )
        metrics2->compensations[i] = 0;
    }

    /* allocate function defs, instruction defs, cvt, and storage area */
    if ( ALLOC_ARRAY( size->function_defs,
                      size->max_function_defs,
                      TT_DefRecord )                ||

         ALLOC_ARRAY( size->instruction_defs,
                      size->max_instruction_defs,
                      TT_DefRecord )                ||

         ALLOC_ARRAY( size->cvt,
                      size->cvt_size, TT_Long )     ||

         ALLOC_ARRAY( size->storage,
                      size->storage_size, TT_Long ) )

      goto Fail_Memory;

    /* reserve twilight zone */
    n_twilight = maxp->maxTwilightPoints;
    error = FT_New_GlyphZone( memory, n_twilight, 0, &size->twilight );
    if ( error )
      goto Fail_Memory;

    size->twilight.n_points = n_twilight;

    /* set `face->interpreter' according to the debug hook present */
    {
      FT_Library  library = face->root.driver->library;


      face->interpreter = (TT_Interpreter)
                            library->debug_hooks[FT_DEBUG_HOOK_TRUETYPE];
      if ( !face->interpreter )
        face->interpreter = (TT_Interpreter)TT_RunIns;
    }

    /* Fine, now execute the font program! */
    exec = size->context;
    /* size objects used during debugging have their own context */
    if ( !size->debug )
      exec = TT_New_Context( face );

    if ( !exec )
    {
      error = TT_Err_Could_Not_Find_Context;
      goto Fail_Memory;
    }

    size->GS = tt_default_graphics_state;
    TT_Load_Context( exec, face, size );

    exec->callTop   = 0;
    exec->top       = 0;

    exec->period    = 64;
    exec->phase     = 0;
    exec->threshold = 0;

    {
      FT_Size_Metrics*  metrics    = &exec->metrics;
      TT_Size_Metrics*  tt_metrics = &exec->tt_metrics;


      metrics->x_ppem   = 0;
      metrics->y_ppem   = 0;
      metrics->x_scale  = 0;
      metrics->y_scale  = 0;

      tt_metrics->ppem  = 0;
      tt_metrics->scale = 0;
      tt_metrics->ratio = 0x10000L;
    }

    exec->instruction_trap = FALSE;

    exec->cvtSize = size->cvt_size;
    exec->cvt     = size->cvt;

    exec->F_dot_P = 0x10000L;

    /* allow font program execution */
    TT_Set_CodeRange( exec,
                      tt_coderange_font,
                      face->font_program,
                      face->font_program_size );

    /* disable CVT and glyph programs coderange */
    TT_Clear_CodeRange( exec, tt_coderange_cvt );
    TT_Clear_CodeRange( exec, tt_coderange_glyph );

    if ( face->font_program_size > 0 )
    {
      error = TT_Goto_CodeRange( exec, tt_coderange_font, 0 );
      if ( !error )
        error = face->interpreter( exec );

      if ( error )
        goto Fail_Exec;
    }
    else
      error = TT_Err_Ok;

    TT_Save_Context( exec, size );

    if ( !size->debug )
      TT_Done_Context( exec );

#endif /* TT_CONFIG_OPTION_BYTECODE_INTERPRETER */

    size->ttmetrics.valid = FALSE;
    return error;

#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER

  Fail_Exec:
    if ( !size->debug )
      TT_Done_Context( exec );

  Fail_Memory:

#endif

    TT_Done_Size( size );
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Done_Size                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The TrueType size object finalizer.                                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    size :: A handle to the target size object.                        */
  /*                                                                       */
  LOCAL_FUNC
  void  TT_Done_Size( TT_Size  size )
  {

#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER

    FT_Memory  memory = size->root.face->memory;


    if ( size->debug )
    {
      /* the debug context must be deleted by the debugger itself */
      size->context = NULL;
      size->debug   = FALSE;
    }

    FREE( size->cvt );
    size->cvt_size = 0;

    /* free storage area */
    FREE( size->storage );
    size->storage_size = 0;

    /* twilight zone */
    FT_Done_GlyphZone( &size->twilight );

    FREE( size->function_defs );
    FREE( size->instruction_defs );

    size->num_function_defs    = 0;
    size->max_function_defs    = 0;
    size->num_instruction_defs = 0;
    size->max_instruction_defs = 0;

    size->max_func = 0;
    size->max_ins  = 0;

#endif

    size->ttmetrics.valid = FALSE;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Reset_Size                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Resets a TrueType size when resolutions and character dimensions   */
  /*    have been changed.                                                 */
  /*                                                                       */
  /* <Input>                                                               */
  /*    size :: A handle to the target size object.                        */
  /*                                                                       */
  LOCAL_DEF
  TT_Error  TT_Reset_Size( TT_Size  size )
  {
    TT_Face   face;
    TT_Error  error = TT_Err_Ok;

    FT_Size_Metrics*  metrics;


    if ( size->ttmetrics.valid )
      return TT_Err_Ok;

    face = (TT_Face)size->root.face;

    metrics = &size->root.metrics;

    if ( metrics->x_ppem < 1 || metrics->y_ppem < 1 )
      return TT_Err_Invalid_PPem;

    /* compute new transformation */
    if ( metrics->x_ppem >= metrics->y_ppem )
    {
      size->ttmetrics.scale   = metrics->x_scale;
      size->ttmetrics.ppem    = metrics->x_ppem;
      size->ttmetrics.x_ratio = 0x10000L;
      size->ttmetrics.y_ratio = FT_MulDiv( metrics->y_ppem,
                                           0x10000L,
                                           metrics->x_ppem );
    }
    else
    {
      size->ttmetrics.scale   = metrics->y_scale;
      size->ttmetrics.ppem    = metrics->y_ppem;
      size->ttmetrics.x_ratio = FT_MulDiv( metrics->x_ppem,
                                           0x10000L,
                                           metrics->y_ppem );
      size->ttmetrics.y_ratio = 0x10000L;
    }

    /* Compute root ascender, descender, test height, and max_advance */
    metrics->ascender = ( FT_MulFix( face->root.ascender,
                                     metrics->y_scale ) + 32 ) & -64;

    metrics->descender = ( FT_MulFix( face->root.descender,
                                      metrics->y_scale ) + 32 ) & -64;

    metrics->height = ( FT_MulFix( face->root.height,
                                   metrics->y_scale ) + 32 ) & -64;

    metrics->max_advance = ( FT_MulFix( face->root.max_advance_width,
                                        metrics->x_scale ) + 32 ) & -64;

#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER

    {
      TT_ExecContext    exec;
      TT_UInt  i, j;


      /* Scale the cvt values to the new ppem.          */
      /* We use by default the y ppem to scale the CVT. */
      for ( i = 0; i < size->cvt_size; i++ )
        size->cvt[i] = FT_MulFix( face->cvt[i], size->ttmetrics.scale );

      /* All twilight points are originally zero */
      for ( j = 0; j < size->twilight.n_points; j++ )
      {
        size->twilight.org[j].x = 0;
        size->twilight.org[j].y = 0;
        size->twilight.cur[j].x = 0;
        size->twilight.cur[j].y = 0;
      }

      /* clear storage area */
      for ( i = 0; i < size->storage_size; i++ )
        size->storage[i] = 0;

      size->GS = tt_default_graphics_state;

      /* get execution context and run prep program */
      if ( size->debug )
        exec = size->context;
      else
        exec = TT_New_Context( face );
      /* debugging instances have their own context */

      if ( !exec )
        return TT_Err_Could_Not_Find_Context;

      TT_Load_Context( exec, face, size );

      TT_Set_CodeRange( exec,
                        tt_coderange_cvt,
                        face->cvt_program,
                        face->cvt_program_size );

      TT_Clear_CodeRange( exec, tt_coderange_glyph );

      exec->instruction_trap = FALSE;

      exec->top     = 0;
      exec->callTop = 0;

      if ( face->cvt_program_size > 0 )
      {
        error = TT_Goto_CodeRange( exec, tt_coderange_cvt, 0 );
        if ( error )
          goto Fin;

        if ( !size->debug )
          error = face->interpreter( exec );
      }
      else
        error = TT_Err_Ok;

      size->GS = exec->GS;
      /* save default graphics state */

    Fin:
      TT_Save_Context( exec, size );

      if ( !size->debug )
        TT_Done_Context( exec );
      /* debugging instances keep their context */
    }

#endif /* TT_CONFIG_OPTION_BYTECODE_INTERPRETER */

    if ( !error )
      size->ttmetrics.valid = TRUE;

    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Init_GlyphSlot                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The TrueType glyph slot initializer.                               */
  /*                                                                       */
  /* <Input>                                                               */
  /*    slot :: The glyph record to build.                                 */
  /*                                                                       */
  /* <Output>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_FUNC
  TT_Error  TT_Init_GlyphSlot( TT_GlyphSlot  slot )
  {
    /* allocate the outline space */
    FT_Face     face    = slot->face;
    FT_Library  library = face->driver->library;


    FT_TRACE4(( "TT_Init_GlyphSlot: Creating outline maxp = %d, maxc = %d\n",
                face->max_points, face->max_contours ));

    return FT_Outline_New( library,
                           face->max_points + 2,
                           face->max_contours,
                           &slot->outline );
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Done_GlyphSlot                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The TrueType glyph slot finalizer.                                 */
  /*                                                                       */
  /* <Input>                                                               */
  /*    slot :: A handle to the glyph slot object.                         */
  /*                                                                       */
  LOCAL_FUNC
  void  TT_Done_GlyphSlot( TT_GlyphSlot  slot )
  {
    FT_Library  library = slot->face->driver->library;
    FT_Memory   memory  = library->memory;


    if ( slot->flags & ft_glyph_own_bitmap )
      FREE( slot->bitmap.buffer );

    FT_Outline_Done( library, &slot->outline );
    return;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Init_Driver                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Initializes a given TrueType driver object.                        */
  /*                                                                       */
  /* <Input>                                                               */
  /*    driver :: A handle to the target driver object.                    */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_FUNC
  TT_Error  TT_Init_Driver( TT_Driver  driver )
  {
    FT_Memory  memory = driver->root.memory;
    TT_Error   error;


    error = FT_New_GlyphZone( memory, 0, 0, &driver->zone );
    if ( error )
      return error;

    /* init extension registry if needed */

#ifdef TT_CONFIG_OPTION_EXTEND_ENGINE
    return TT_Init_Extensions( driver );
#else
    return TT_Err_Ok;
#endif
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Done_Driver                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Finalizes a given TrueType driver.                                 */
  /*                                                                       */
  /* <Input>                                                               */
  /*    driver :: A handle to the target TrueType driver.                  */
  /*                                                                       */
  LOCAL_FUNC
  void  TT_Done_Driver( TT_Driver  driver )
  {
    /* destroy extensions registry if needed */

#ifdef TT_CONFIG_OPTION_EXTEND_ENGINE
    TT_Done_Extensions( driver );
#endif

    /* remove the loading glyph zone */
    FT_Done_GlyphZone( &driver->zone );

#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER

    /* destroy the execution context */
    if ( driver->context )
    {
      TT_Destroy_Context( driver->context, driver->root.memory );
      driver->context = NULL;
    }

#endif
  }


/* END */
