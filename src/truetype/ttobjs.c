/***************************************************************************/
/*                                                                         */
/*  ttobjs.c                                                               */
/*                                                                         */
/*    Objects manager (body).                                              */
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


#include <freetype.h>
#include <ftdebug.h>
#include <ftcalc.h>
#include <ftstream.h>
#include <ttnameid.h>
#include <tttags.h>

#include <sfnt.h>
#include <psnames.h>
#include <ttobjs.h>

#include <ttpload.h>
#include <tterrors.h>

#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER
#include <ttinterp.h>
#endif

/* required by tracing mode */
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
  /*    Get_Name                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Returns a given ENGLISH name record in ASCII.                      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face   :: A handle to the source face object.                      */
  /*                                                                       */
  /*    nameid :: The name id of the name record to return.                */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Char string.  NULL if no name is present.                          */
  /*                                                                       */
  static
  FT_String*  Get_Name( TT_Face    face,
                        TT_UShort  nameid )
  {
    FT_Memory    memory = face->root.memory;
    TT_UShort    n;
    TT_NameRec*  rec;
    TT_Bool      wide_chars = 1;

    /* first pass, look for a given name record */
    rec = face->name_table.names;
    for ( n = 0; n < face->name_table.numNameRecords; n++, rec++ )
    {
      if ( rec->nameID == nameid )
      {
        /* found the name - now create an ASCII string from it */
        TT_Bool  found = 0;

        /* Test for Microsoft English language */
        if ( rec->platformID == TT_PLATFORM_MICROSOFT &&
             rec->encodingID <= TT_MS_ID_UNICODE_CS   &&
             (rec->languageID & 0x3FF) == 0x009 )
          found = 1;

        /* Test for Apple Unicode encoding */
        else if ( rec->platformID == TT_PLATFORM_APPLE_UNICODE )
          found = 1;
        
        /* Test for Apple Roman */
        else if ( rec->platformID == TT_PLATFORM_MACINTOSH &&
                  rec->languageID == TT_MAC_ID_ROMAN       )
        {
          found      = 1;
          wide_chars = 0;
        }

        /* Found a Unicode Name */
        if ( found )
        {
          TT_String*  string;
          TT_UInt     len;

          if ( wide_chars )
          {
            TT_UInt   m;
            
            len = (TT_UInt)rec->stringLength / 2;
            if ( MEM_Alloc( string, len + 1 ) )
              return NULL;
    
            for ( m = 0; m < len; m ++ )
              string[m] = rec->string[2*m + 1];
          }
          else
          {
            len = rec->stringLength;
            if ( MEM_Alloc( string, len + 1 ) )
              return NULL;

            MEM_Copy( string, rec->string, len );
          }

          string[len] = '\0';
          return string;
        }
      }
    }
    return NULL;
  }


#undef  LOAD_
#define LOAD_(x)   ( (error = sfnt->load_##x( face, stream )) != TT_Err_Ok )


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Init_Face                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Initializes a given TrueType face object.                          */
  /*                                                                       */
  /* <Input>                                                               */
  /*    resource   :: The source font resource.                            */
  /*    face_index :: The index of the font face in the resource.          */
  /*    face       :: The newly built face object.                         */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_DEF
  TT_Error  TT_Init_Face( FT_Stream  stream,
                          TT_Long    face_index,
                          TT_Face    face )
  {
    TT_Error           error;
    TT_ULong           format_tag;
    SFNT_Interface*    sfnt;
    PSNames_Interface* psnames;

    sfnt = (SFNT_Interface*)face->sfnt;
    if (!sfnt)
    {
      /* look-up the SFNT driver */
      FT_Driver  sfnt_driver;
      
      sfnt_driver = FT_Get_Driver( face->root.driver->library, "sfnt" );
      if (!sfnt_driver)
        return FT_Err_Invalid_File_Format;
        
      sfnt = (SFNT_Interface*)(sfnt_driver->interface.format_interface);
      if (!sfnt)
        return FT_Err_Invalid_File_Format;
        
      face->sfnt       = sfnt;
      face->goto_table = sfnt->goto_table;
    }
    
    psnames = (PSNames_Interface*)face->psnames;
    if (!psnames)
    {
      /* look-up the PSNames driver */
      FT_Driver  psnames_driver;
      
      psnames_driver = FT_Get_Driver( face->root.driver->library, "psnames" );
      if (psnames_driver)
        face->psnames = (PSNames_Interface*)
                            (psnames_driver->interface.format_interface);
    }
    
    /* create input stream from resource */
    if ( FILE_Seek(0) )
      goto Exit;

    /* check that we have a valid TrueType file */
    error = sfnt->load_format_tag( face, stream, face_index, &format_tag );
    if (error) goto Exit;
    
    /* We must also be able to accept Mac/GX fonts, as well as OT ones */
    if ( format_tag != 0x00010000 &&    /* MS fonts  */
         format_tag != TTAG_true  )     /* Mac fonts */
    {
      FT_TRACE2(( "[not a valid TTF font]" ));
      error = TT_Err_Invalid_File_Format;
      goto Exit;
    }

    /* Load font directory */
    error = sfnt->load_directory( face, stream, face_index );
    if ( error ) goto Exit;

    face->root.num_faces = face->ttc_header.DirCount;
    if ( face->root.num_faces < 1 )
      face->root.num_faces = 1;

    /* If we're performing a simple font format check, exit immediately */
    if ( face_index < 0 )
      return TT_Err_Ok;

    /* Load tables */

    if ( LOAD_( header )        ||
         LOAD_( max_profile )   ||

         (error = sfnt->load_metrics( face, stream, 0 )) != TT_Err_Ok  ||
         /* load the `hhea' & `hmtx' tables at once */

         (error = sfnt->load_metrics( face, stream, 1 )) != TT_Err_Ok ||
         /* try to load the `vhea' & `vmtx' at once if present */

         LOAD_( charmaps )      ||
         LOAD_( names )         ||
         LOAD_( os2 )           ||
         LOAD_( psnames )       )
     goto Exit;

    /* the optional tables */
    
    /* embedded bitmap support. */
#ifdef TT_CONFIG_OPTION_EMBEDDED_BITMAPS
    if (sfnt->load_sbits && LOAD_(sbits)) goto Exit;
#endif

    if ( LOAD_( hdmx )          ||
         LOAD_( gasp )          ||
         LOAD_( kerning )       ||

         (error = TT_Load_Locations( face, stream )) != TT_Err_Ok ||
         (error = TT_Load_CVT      ( face, stream )) != TT_Err_Ok ||
         (error = TT_Load_Programs ( face, stream )) != TT_Err_Ok )

      goto Exit;

#ifdef TT_CONFIG_OPTION_EXTEND_ENGINE
    if ( ( error = TT_Extension_Create( face ) ) != TT_Err_Ok )
      goto Exit;
#endif

    face->root.family_name = Get_Name( face, TT_NAME_ID_FONT_FAMILY );
    face->root.style_name  = Get_Name( face, TT_NAME_ID_FONT_SUBFAMILY );

  Exit:
    return error;
  }


#undef LOAD_


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
    TT_UShort  n;
    FT_Memory  memory = face->root.memory;

    SFNT_Interface*  sfnt = face->sfnt;
    
    if (sfnt)
    {
      /* destroy the postscript names table if it is supported */
      if (sfnt->free_psnames)
        sfnt->free_psnames( face );
        
      /* destroy the embedded bitmaps table if it is supported */
      if (sfnt->free_sbits)
        sfnt->free_sbits( face );
    }

    /* freeing the kerning table */
    FREE( face->kern_pairs );
    face->num_kern_pairs = 0;

    /* freeing the collection table */
    FREE( face->ttc_header.TableDirectory );
    face->ttc_header.DirCount = 0;

    /* freeing table directory */
    FREE( face->dir_tables );
    face->num_tables = 0;

    /* freeing the locations table */
    FREE( face->glyph_locations );
    face->num_locations = 0;

    /* freeing the character mapping tables */
    if (sfnt && sfnt->load_charmaps )
    {
      for ( n = 0; n < face->num_charmaps; n++ )
        sfnt->free_charmap( face, &face->charmaps[n].cmap );
    }

    FREE( face->charmaps );
    face->num_charmaps = 0;

    FREE( face->root.charmaps );
    face->root.num_charmaps = 0;
    face->root.charmap      = 0;

    /* freeing the CVT */
    FREE( face->cvt );
    face->cvt_size = 0;

    /* freeing the horizontal metrics */
    FREE( face->horizontal.long_metrics );
    FREE( face->horizontal.short_metrics );

    /* freeing the vertical ones, if any */
    if ( face->vertical_info )
    {
      FREE( face->vertical.long_metrics  );
      FREE( face->vertical.short_metrics );
      face->vertical_info = 0;
    }

    /* freeing the programs */
    FREE( face->font_program );
    FREE( face->cvt_program );
    face->font_program_size = 0;
    face->cvt_program_size  = 0;

    /* freeing the gasp table */
    FREE( face->gasp.gaspRanges );
    face->gasp.numRanges = 0;

    /* freeing the name table */
    sfnt->free_names( face );

    /* freeing the hdmx table */
    sfnt->free_hdmx( face );

    /* freeing family and style name */
    FREE( face->root.family_name );
    FREE( face->root.style_name );
    
    face->sfnt = 0;
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

    /* allocate function defs, instruction defs, cvt and storage area */
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
                            library->debug_hooks[ FT_DEBUG_HOOK_TRUETYPE ];
      if (!face->interpreter)
        face->interpreter = (TT_Interpreter)TT_RunIns;
    }

    /* Fine, now execute the font program! */
    exec = size->context;
    if (!size->debug)
      exec = TT_New_Context( face );
    /* size objects used during debugging have their own context */

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


      metrics->x_ppem    = 0;
      metrics->y_ppem    = 0;
      metrics->x_scale   = 0;
      metrics->y_scale   = 0;

      tt_metrics->ppem   = 0;
      tt_metrics->scale  = 0;
      tt_metrics->ratio  = 0x10000;
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
      size->ttmetrics.x_ratio = 0x10000;
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
      size->ttmetrics.y_ratio = 0x10000;
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


    FT_TRACE4(( "TT.Init_GlyphSlot: Creating outline maxp = %d, maxc = %d\n",
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
    FT_Error   error;
    
    error = FT_New_GlyphZone( memory, 0, 0, &driver->zone );
    if (error) return error;
    
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
