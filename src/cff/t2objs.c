/***************************************************************************/
/*                                                                         */
/*  t2objs.c                                                               */
/*                                                                         */
/*    OpenType objects manager (body).                                     */
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
#include <t2objs.h>

#include <t2load.h>
#include <freetype/internal/t2errors.h>


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_t2objs


  /*************************************************************************/
  /*                                                                       */
  /*                           FACE  FUNCTIONS                             */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    T2_Init_Face                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Initializes a given OpenType face object.                          */
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
  /*    FreeTrue error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_DEF
  FT_Error  T2_Init_Face( FT_Stream      stream,
                          T2_Face        face,
                          FT_Int         face_index,
                          FT_Int         num_params,
                          FT_Parameter*  params )
  {
    FT_Error         error;
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

    /* check that we have a valid OpenType file */
    error = sfnt->init_face( stream, face, face_index, num_params, params );
    if ( error )
      goto Exit;

    if ( face->format_tag != 0x4f54544fL )     /* OpenType/CFF font */
    {
      FT_TRACE2(( "[not a valid OpenType/CFF font]\n" ));
      goto Bad_Format;
    }

    /* If we are performing a simple font format check, exit immediately */
    if ( face_index < 0 )
      return T2_Err_Ok;

    /* Load font directory */
    error = sfnt->load_face( stream, face, face_index, num_params, params );
    if ( error )
      goto Exit;

    /* now, load the CFF part of the file.. */
    error = face->goto_table( face, TTAG_CFF, stream, 0 );
    if ( error )
      goto Exit;

    {
      CFF_Font*  cff;
      FT_Memory  memory = face->root.memory;
      FT_Face    root;


      if ( ALLOC( cff, sizeof ( *cff ) ) )
        goto Exit;

      face->other = cff;
      error = T2_Load_CFF_Font( stream, face_index, cff );
      if ( error )
        goto Exit;

      /* Complement the root flags with some interesting information. */
      /* note that for OpenType/CFF, there is no need to do this, but */
      /* this will be necessary for pure CFF fonts through.           */
      root = &face->root;
    }

  Exit:
    return error;

  Bad_Format:
    error = FT_Err_Unknown_File_Format;
    goto Exit;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    T2_Done_Face                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Finalizes a given face object.                                     */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face :: A pointer to the face object to destroy.                   */
  /*                                                                       */
  LOCAL_DEF
  void  T2_Done_Face( T2_Face  face )
  {
    FT_Memory  memory = face->root.memory;
#if 0
    FT_Stream  stream = face->root.stream;
#endif

    SFNT_Interface*  sfnt = face->sfnt;


    if ( sfnt )
      sfnt->done_face( face );

    {
      CFF_Font*  cff = (CFF_Font*)face->other;


      if ( cff )
      {
        T2_Done_CFF_Font( cff );
        FREE( face->other );
      }
    }
  }


  /*************************************************************************/
  /*                                                                       */
  /*                           SIZE  FUNCTIONS                             */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    T2_Init_Size                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Initializes a new OpenType size object.                            */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    size :: A handle to the size object.                               */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_DEF
  FT_Error  T2_Init_Size( T2_Size  size )
  {
    UNUSED( size );

    return 0;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    T2_Done_Size                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The OpenType size object finalizer.                                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    size :: A handle to the target size object.                        */
  /*                                                                       */
  LOCAL_FUNC
  void  T2_Done_Size( T2_Size  size )
  {
    UNUSED( size );
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    T2_Reset_Size                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Resets a OpenType size when resolutions and character dimensions   */
  /*    have been changed.                                                 */
  /*                                                                       */
  /* <Input>                                                               */
  /*    size :: A handle to the target size object.                        */
  /*                                                                       */
  /* <Output>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_DEF
  FT_Error  T2_Reset_Size( T2_Size  size )
  {
    T2_Face           face    = (T2_Face)size->face;
    FT_Size_Metrics*  metrics = &size->metrics;


    if ( metrics->x_ppem < 1 || metrics->y_ppem < 1 )
      return T2_Err_Invalid_PPem;

    /* Compute root ascender, descender, test height, and max_advance */
    metrics->ascender = ( FT_MulFix( face->root.ascender,
                                     metrics->y_scale ) + 32 ) & -64;

    metrics->descender = ( FT_MulFix( face->root.descender,
                                      metrics->y_scale ) + 32 ) & -64;

    metrics->height = ( FT_MulFix( face->root.height,
                                   metrics->y_scale ) + 32 ) & -64;

    metrics->max_advance = ( FT_MulFix( face->root.max_advance_width,
                                        metrics->x_scale ) + 32 ) & -64;

    return T2_Err_Ok;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    T2_Init_GlyphSlot                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The OpenType glyph slot initializer.                               */
  /*                                                                       */
  /* <Input>                                                               */
  /*    slot :: The glyph record to build.                                 */
  /*                                                                       */
  /* <Output>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_FUNC
  FT_Error  T2_Init_GlyphSlot( T2_GlyphSlot  slot )
  {
    FT_Library  library = slot->root.face->driver->library;


    slot->max_points         = 0;
    slot->max_contours       = 0;
    slot->root.bitmap.buffer = 0;

    return FT_Outline_New( library, 0, 0, &slot->root.outline );
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    T2_Done_GlyphSlot                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The OpenType glyph slot finalizer.                                 */
  /*                                                                       */
  /* <Input>                                                               */
  /*    slot :: A handle to the glyph slot object.                         */
  /*                                                                       */
  LOCAL_FUNC
  void  T2_Done_GlyphSlot( T2_GlyphSlot  slot )
  {
    FT_Library  library = slot->root.face->driver->library;
    FT_Memory   memory  = library->memory;


    if ( slot->root.flags & ft_glyph_own_bitmap )
      FREE( slot->root.bitmap.buffer );

    FT_Outline_Done( library, &slot->root.outline );
    return;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    T2_Init_Driver                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Initializes a given OpenType driver object.                        */
  /*                                                                       */
  /* <Input>                                                               */
  /*    driver :: A handle to the target driver object.                    */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_FUNC
  FT_Error  T2_Init_Driver( T2_Driver  driver )
  {
    /* init extension registry if needed */
#ifdef TT_CONFIG_OPTION_EXTEND_ENGINE
    return TT_Init_Extensions( driver );
#else
    return T2_Err_Ok;
#endif
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    T2_Done_Driver                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Finalizes a given OpenType driver.                                 */
  /*                                                                       */
  /* <Input>                                                               */
  /*    driver :: A handle to the target OpenType driver.                  */
  /*                                                                       */
  LOCAL_FUNC
  void  T2_Done_Driver( T2_Driver  driver )
  {
    /* destroy extensions registry if needed */
#ifdef TT_CONFIG_OPTION_EXTEND_ENGINE
    TT_Done_Extensions( driver );
#endif
  }


/* END */
