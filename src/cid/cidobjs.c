/***************************************************************************/
/*                                                                         */
/*  cidobjs.c                                                              */
/*                                                                         */
/*    CID objects manager (body).                                          */
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
#include <freetype/internal/ftstream.h>

#include <cidgload.h>
#include <cidload.h>
#include <freetype/internal/psnames.h>
#include <cidafm.h>


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
#undef   FT_COMPONENT
#define  FT_COMPONENT  trace_cidobjs


  /*************************************************************************/
  /*                                                                       */
  /*                           SIZE  FUNCTIONS                             */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    CID_Done_Size                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The CID size object finalizer.                                     */
  /*                                                                       */
  /* <Input>                                                               */
  /*    size :: A handle to the target size object.                        */
  /*                                                                       */
  LOCAL_FUNC
  void  CID_Done_Size( T1_Size  size )
  {
    UNUSED( size );
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    CID_Init_Size                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Initializes a new CID size object.                                 */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    size :: A handle to the size object.                               */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Type1 error code.  0 means success.                                */
  /*                                                                       */
  LOCAL_DEF
  FT_Error  CID_Init_Size( T1_Size  size )
  {
    size->valid = 0;

    return T1_Err_Ok;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    CID_Reset_Size                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Resets a OpenType size when resolutions and character dimensions   */
  /*    have been changed.                                                 */
  /*                                                                       */
  /* <Input>                                                               */
  /*    size :: A handle to the target size object.                        */
  /*                                                                       */
  /* <Output>                                                              */
  /*    Type1 error code.  0 means success.                                */
  /*                                                                       */
  LOCAL_FUNC
  FT_Error  CID_Reset_Size( T1_Size  size )
  {
    /* recompute ascender, descender, etc. */
    CID_Face          face    = (CID_Face)size->root.face;
    FT_Size_Metrics*  metrics = &size->root.metrics;


    if ( metrics->x_ppem < 1 || metrics->y_ppem < 1 )
      return T1_Err_Invalid_Argument;

    /* Compute root ascender, descender, test height, and max_advance */
    metrics->ascender = ( FT_MulFix( face->root.ascender,
                                     metrics->y_scale ) + 32 ) & -64;

    metrics->descender = ( FT_MulFix( face->root.descender,
                                      metrics->y_scale ) + 32 ) & -64;

    metrics->height = ( FT_MulFix( face->root.height,
                                   metrics->y_scale ) + 32 ) & -64;

    metrics->max_advance = ( FT_MulFix( face->root.max_advance_width,
                                        metrics->x_scale ) + 32 ) & -64;

    return T1_Err_Ok;
  }


  /*************************************************************************/
  /*                                                                       */
  /*                           FACE  FUNCTIONS                             */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    CID_Done_Face                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Finalizes a given face object.                                     */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face :: A pointer to the face object to destroy.                   */
  /*                                                                       */
  LOCAL_FUNC
  void  CID_Done_Face( CID_Face  face )
  {
    FT_Memory  memory;


    if ( face )
    {
      CID_Info*     cid  = &face->cid;
      T1_FontInfo*  info = &cid->font_info;


      memory = face->root.memory;

      /* release FontInfo strings */
      FREE( info->version );
      FREE( info->notice );
      FREE( info->full_name );
      FREE( info->family_name );
      FREE( info->weight );

      /* release font dictionaries */
      FREE( cid->font_dicts );
      cid->num_dicts = 0;

      /* release other strings */
      FREE( cid->cid_font_name );
      FREE( cid->registry );
      FREE( cid->ordering );

      face->root.family_name = 0;
      face->root.style_name  = 0;
    }
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    CID_Init_Face                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Initializes a given CID face object.                               */
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
  /*    Type1 error code.  0 means success.                                */
  /*                                                                       */
  LOCAL_FUNC
  FT_Error  CID_Init_Face( FT_Stream      stream,
                           CID_Face       face,
                           FT_Int         face_index,
                           FT_Int         num_params,
                           FT_Parameter*  params )
  {
    FT_Error            error;
    PSNames_Interface*  psnames;

    UNUSED( num_params );
    UNUSED( params );
    UNUSED( face_index );
    UNUSED( stream );


    face->root.num_faces = 1;

    psnames = (PSNames_Interface*)face->psnames;
    if ( !psnames )
    {
      /* look-up the PSNames driver */
      FT_Driver  psnames_driver;


      psnames_driver = FT_Get_Driver( face->root.driver->library, "psnames" );
      if ( psnames_driver )
        face->psnames = (PSNames_Interface*)
                          (psnames_driver->interface.format_interface);
    }

    /* open the tokenizer; this will also check the font format */
    if ( FILE_Seek( 0 ) )
      goto Exit;

    error = CID_Open_Face( face );
    if ( error )
      goto Exit;

    /* if we just wanted to check the format, leave successfully now */
    if ( face_index < 0 )
      goto Exit;

    /* check the face index */
    if ( face_index != 0 )
    {
      FT_ERROR(( "CID_Init_Face: invalid face index\n" ));
      error = T1_Err_Invalid_Argument;
      goto Exit;
    }

    /* Now, load the font program into the face object */
    {
      /* Init the face object fields */
      /* Now set up root face fields */
      {
        FT_Face  root = (FT_Face)&face->root;


        root->num_glyphs   = face->cid.cid_count;
        root->num_charmaps = 0;

        root->face_index = face_index;
        root->face_flags = FT_FACE_FLAG_SCALABLE;

        root->face_flags |= FT_FACE_FLAG_HORIZONTAL;

        if ( face->cid.font_info.is_fixed_pitch )
          root->face_flags |= FT_FACE_FLAG_FIXED_WIDTH;

        /* XXX: TO DO - add kerning with .afm support */

        /* get style name - be careful, some broken fonts only */
        /* have a /FontName dictionary entry!                  */
        root->family_name = face->cid.font_info.family_name;
        if ( root->family_name )
        {
          char*  full   = face->cid.font_info.full_name;
          char*  family = root->family_name;

          while ( *family && *full == *family )
          {
            family++;
            full++;
          }

          root->style_name = ( *full == ' ' ) ? full + 1
                                              : "Regular";
        }
        else
        {
          /* do we have a `/FontName'? */
          if ( face->cid.cid_font_name )
          {
            root->family_name = face->cid.cid_font_name;
            root->style_name  = "Regular";
          }
        }

        /* no embedded bitmap support */
        root->num_fixed_sizes = 0;
        root->available_sizes = 0;

        root->bbox         = face->cid.font_bbox;
        root->units_per_EM = 1000;
        root->ascender     =  (FT_Short)face->cid.font_bbox.yMax;
        root->descender    = -(FT_Short)face->cid.font_bbox.yMin;
        root->height       = ( ( root->ascender + root->descender ) * 12 )
                             / 10;


#if 0

        /* now compute the maximum advance width */

        root->max_advance_width = face->type1.private_dict.standard_width[0];

        /* compute max advance width for proportional fonts */
        if ( !face->type1.font_info.is_fixed_pitch )
        {
          FT_Int  max_advance;


          error = T1_Compute_Max_Advance( face, &max_advance );

          /* in case of error, keep the standard width */
          if ( !error )
            root->max_advance_width = max_advance;
          else
            error = 0;   /* clear error */
        }

        root->max_advance_height = root->height;

#endif /* 0 */

        root->underline_position  = face->cid.font_info.underline_position;
        root->underline_thickness = face->cid.font_info.underline_thickness;

        root->max_points   = 0;
        root->max_contours = 0;
      }
    }

#if 0

    /* charmap support - synthetize unicode charmap when possible */
    {
      FT_Face      root    = &face->root;
      FT_CharMap   charmap = face->charmaprecs;


      /* synthesize a Unicode charmap if there is support in the "psnames" */
      /* module                                                            */
      if ( face->psnames )
      {
        PSNames_Interface*  psnames = (PSNames_Interface*)face->psnames;


        if ( psnames->unicode_value )
        {
          error = psnames->build_unicodes(
                             root->memory,
                             face->type1.num_glyphs,
                             (const char**)face->type1.glyph_names,
                             &face->unicode_map );
          if ( !error )
          {
            root->charmap        = charmap;
            charmap->face        = (FT_Face)face;
            charmap->encoding    = ft_encoding_unicode;
            charmap->platform_id = 3;
            charmap->encoding_id = 1;
            charmap++;
          }

          /* simply clear the error in case of failure (which really */
          /* means that out of memory or no unicode glyph names)     */
          error = 0;
        }
      }

      /* now, support either the standard, expert, or custom encodings */
      charmap->face        = (FT_Face)face;
      charmap->platform_id = 7;  /* a new platform id for Adobe fonts? */

      switch ( face->type1.encoding_type )
      {
      case t1_encoding_standard:
        charmap->encoding    = ft_encoding_adobe_standard;
        charmap->encoding_id = 0;
        break;

      case t1_encoding_expert:
        charmap->encoding    = ft_encoding_adobe_expert;
        charmap->encoding_id = 1;
        break;

      default:
        charmap->encoding    = ft_encoding_adobe_custom;
        charmap->encoding_id = 2;
        break;
      }

      root->charmaps     = face->charmaps;
      root->num_charmaps = charmap - face->charmaprecs + 1;
      face->charmaps[0]  = &face->charmaprecs[0];
      face->charmaps[1]  = &face->charmaprecs[1];
    }

#endif /* 0 */

  Exit:
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    CID_Done_GlyphSlot                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The CID glyph slot finalizer.                                      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    slot :: A handle to the glyph slot object.                         */
  /*                                                                       */
  LOCAL_FUNC
  void  CID_Done_GlyphSlot( T1_GlyphSlot  glyph )
  {
    FT_Memory  memory  = glyph->root.face->memory;
    FT_Library library = glyph->root.face->driver->library;


	/* the bitmaps are created on demand */
	FREE( glyph->root.bitmap.buffer );
    FT_Outline_Done( library, &glyph->root.outline );

    return;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    CID_Init_GlyphSlot                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The CID glyph slot initializer.                                    */
  /*                                                                       */
  /* <Input>                                                               */
  /*    slot :: The glyph record to build.                                 */
  /*                                                                       */
  /* <Output>                                                              */
  /*    Type1 error code.  0 means success.                                */
  /*                                                                       */
  LOCAL_FUNC
  FT_Error  CID_Init_GlyphSlot( T1_GlyphSlot  glyph )
  {
    FT_Library  library = glyph->root.face->driver->library;


    glyph->max_points         = 0;
    glyph->max_contours       = 0;
    glyph->root.bitmap.buffer = 0;

    return FT_Outline_New( library, 0, 0, &glyph->root.outline );
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    CID_Init_Driver                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Initializes a given CID driver object.                             */
  /*                                                                       */
  /* <Input>                                                               */
  /*    driver :: A handle to the target driver object.                    */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Type1 error code.  0 means success.                                */
  /*                                                                       */
  LOCAL_FUNC
  FT_Error  CID_Init_Driver( T1_Driver  driver )
  {
    UNUSED( driver );

    return T1_Err_Ok;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    CID_Done_Driver                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Finalizes a given CID driver.                                      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    driver :: A handle to the target CID driver.                       */
  /*                                                                       */
  LOCAL_DEF
  void  CID_Done_Driver( T1_Driver  driver )
  {
    UNUSED( driver );
  }


/* END */
