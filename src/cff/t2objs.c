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
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_DEF
  FT_Error  T2_Init_Face( FT_Stream      stream,
                          T2_Face        face,
                          FT_Int         face_index,
                          FT_Int         num_params,
                          FT_Parameter*  params )
  {
    FT_Error         error;
    SFNT_Interface*  sfnt;


    sfnt = (SFNT_Interface*)FT_Get_Module_Interface(
             face->root.driver->root.library, "sfnt" );
    if ( !sfnt )
      goto Bad_Format;

    /* create input stream from resource */
    if ( FILE_Seek( 0 ) )
      goto Exit;

    /* check that we have a valid OpenType file */
    error = sfnt->init_face( stream, face, face_index, num_params, params );
    if ( error )
      goto Exit;

    if ( face->format_tag != 0x4F54544FL )  /* `OTTO'; OpenType/CFF font */
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

    /* now, load the CFF part of the file */
    error = face->goto_table( face, TTAG_CFF, stream, 0 );
    if ( error )
      goto Exit;

    {
      CFF_Font*  cff;
      FT_Memory  memory = face->root.memory;
      FT_Face    root;


      if ( ALLOC( cff, sizeof ( *cff ) ) )
        goto Exit;

      face->extra.data = cff;
      error = T2_Load_CFF_Font( stream, face_index, cff );
      if ( error )
        goto Exit;

      /* Complement the root flags with some interesting information. */
      /* Note that for OpenType/CFF, there is no need to do this, but */
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
    FT_Memory        memory = face->root.memory;
    SFNT_Interface*  sfnt   = (SFNT_Interface*)face->sfnt;


    if ( sfnt )
      sfnt->done_face( face );

    {
      CFF_Font*  cff = (CFF_Font*)face->extra.data;


      if ( cff )
      {
        T2_Done_CFF_Font( cff );
        FREE( face->extra.data );
      }
    }
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

    FT_UNUSED( driver );

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

#else

    FT_UNUSED( driver );

#endif
  }


/* END */
