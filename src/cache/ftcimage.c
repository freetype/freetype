/***************************************************************************/
/*                                                                         */
/*  ftcimage.c                                                             */
/*                                                                         */
/*    FreeType Image cache (body).                                         */
/*                                                                         */
/*  Copyright 2000 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifdef FT_FLAT_COMPILE
#  include "ftcimage.h"
#else
#  include <cache/ftcimage.h>
#endif

#include <string.h>

#include <freetype/internal/ftmemory.h>


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                    GLYPH IMAGE NODES                          *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/


  /* this is a simple glyph image destructor, which is called exclusively */
  /* from the CacheQueue object                                           */
  LOCAL_FUNC_X
  void  ftc_glyph_image_node_destroy( FTC_GlyphImage   node,
                                      FTC_Glyph_Queue  queue )
  {
    FT_Memory  memory = queue->memory;


    FT_Done_Glyph( node->ft_glyph );
    FREE( node );
  }


  LOCAL_FUNC_X
  FT_Error  ftc_glyph_image_node_new( FTC_Glyph_Queue  queue,
                                      FT_UInt          glyph_index,
                                      FTC_GlyphImage  *anode )
  {
    FT_Memory        memory   = queue->memory;
    FTC_Image_Queue  imageq = (FTC_Image_Queue)queue;
    FT_Error         error;
    FTC_GlyphImage   node = 0;
    FT_Face          face;
    FT_Size          size;


    /* allocate node */
    if ( ALLOC( node, sizeof ( *node ) ) )
      goto Exit;

    /* init its inner fields */
    FTC_GlyphNode_Init( FTC_GLYPHNODE(node), queue, glyph_index );

    /* we will now load the glyph image */
    error = FTC_Manager_Lookup_Size( queue->manager,
                                     &imageq->description.font,
                                     &face, &size );
    if ( !error )
    {
      FT_UInt  glyph_index = node->root.glyph_index;
      FT_UInt  load_flags  = FT_LOAD_DEFAULT;
      FT_UInt  image_type  = imageq->description.image_type;


      if ( FTC_IMAGE_FORMAT( image_type ) == ftc_image_format_bitmap )
      {
        load_flags |= FT_LOAD_RENDER;
        if ( image_type & ftc_image_flag_monochrome )
          load_flags |= FT_LOAD_MONOCHROME;

        /* disable embedded bitmaps loading if necessary */
        if ( image_type & ftc_image_flag_no_sbits )
          load_flags |= FT_LOAD_NO_BITMAP;
      }
      else if ( FTC_IMAGE_FORMAT( image_type ) == ftc_image_format_outline )
      {
        /* disable embedded bitmaps loading */
        load_flags |= FT_LOAD_NO_BITMAP;

        if ( image_type & ftc_image_flag_unscaled )
          load_flags |= FT_LOAD_NO_SCALE;
      }

      if ( image_type & ftc_image_flag_unhinted )
        load_flags |= FT_LOAD_NO_HINTING;

      if ( image_type & ftc_image_flag_autohinted )
        load_flags |= FT_LOAD_FORCE_AUTOHINT;

      error = FT_Load_Glyph( face, glyph_index, load_flags );
      if ( !error )
      {
        if ( face->glyph->format == ft_glyph_format_bitmap  ||
             face->glyph->format == ft_glyph_format_outline )
        {
          /* ok, copy it */
          FT_Glyph  glyph;


          error = FT_Get_Glyph( face->glyph, &glyph );
          if ( !error )
            node->ft_glyph = glyph;
        }
        else
          error = FT_Err_Invalid_Argument;
      }
    }

  Exit:
    if ( error && node )
      FREE( node );

    *anode = node;
    return error;
  }


  /* this function is important because it is both part of */
  /* a FTC_Glyph_Queue_Class and a FTC_CacheNode_Class     */
  /*                                                       */
  LOCAL_FUNC_X
  FT_ULong  ftc_glyph_image_node_size( FTC_GlyphImage  node )
  {
    FT_ULong  size  = 0;
    FT_Glyph  glyph = node->ft_glyph;


    switch ( glyph->format )
    {
    case ft_glyph_format_bitmap:
      {
        FT_BitmapGlyph  bitg;


        bitg = (FT_BitmapGlyph)glyph;
        size = bitg->bitmap.rows * labs( bitg->bitmap.pitch ) +
               sizeof ( *bitg );
      }
      break;

    case ft_glyph_format_outline:
      {
        FT_OutlineGlyph  outg;


        outg = (FT_OutlineGlyph)glyph;
        size = outg->outline.n_points *
                 ( sizeof( FT_Vector ) + sizeof ( FT_Byte ) ) +
               outg->outline.n_contours * sizeof ( FT_Short ) +
               sizeof ( *outg );
      }
      break;

    default:
      ;
    }

    size += sizeof ( *node );
    return size;
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                    GLYPH IMAGE QUEUES                         *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/


  LOCAL_FUNC_X
  FT_Error  ftc_image_queue_init( FTC_Image_Queue  queue,
                                  FTC_Image_Desc*  type )
  {
    queue->description = *type;
    return 0;
  }


  LOCAL_FUNC_X
  FT_Bool  ftc_image_queue_compare( FTC_Image_Queue  queue,
                                    FTC_Image_Desc*  type )
  {
    return !memcmp( &queue->description, type, sizeof ( *type ) );
  }


  FT_CPLUSPLUS( const FTC_Glyph_Queue_Class )  ftc_glyph_image_queue_class =
  {
    sizeof( FTC_Image_QueueRec ),

    (FTC_Glyph_Queue_InitFunc)       ftc_image_queue_init,
    (FTC_Glyph_Queue_DoneFunc)       0,
    (FTC_Glyph_Queue_CompareFunc)    ftc_image_queue_compare,

    (FTC_Glyph_Queue_NewNodeFunc)    ftc_glyph_image_node_new,
    (FTC_Glyph_Queue_SizeNodeFunc)   ftc_glyph_image_node_size,
    (FTC_Glyph_Queue_DestroyNodeFunc)ftc_glyph_image_node_destroy
  };


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                    GLYPH IMAGE CACHE                          *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/


  FT_CPLUSPLUS( const FTC_Glyph_Cache_Class )  ftc_glyph_image_cache_class =
  {
    {
      sizeof( FTC_Glyph_CacheRec ),
      (FTC_Cache_InitFunc)FTC_Glyph_Cache_Init,
      (FTC_Cache_DoneFunc)FTC_Glyph_Cache_Done
    },
    (FTC_Glyph_Queue_Class*)&ftc_glyph_image_queue_class
  };


  FT_EXPORT_FUNC( FT_Error )  FTC_Image_Cache_New( FTC_Manager       manager,
                                                   FTC_Image_Cache*  acache )
  {
    return FTC_Manager_Register_Cache(
              manager,
              (FTC_Cache_Class*)&ftc_glyph_image_cache_class,
              (FTC_Cache*)acache );
  }


  FT_EXPORT_DEF( FT_Error )
  FTC_Image_Cache_Lookup( FTC_Image_Cache  cache,
                          FTC_Image_Desc*  desc,
                          FT_UInt          gindex,
                          FT_Glyph*        aglyph )
  {
    FT_Error         error;
    FTC_Glyph_Queue  queue;
    FTC_GlyphNode    node;
    FTC_Manager      manager;

    FTC_Image_Queue  img_queue;


    /* check for valid `desc' delayed to FT_Lru_Lookup() */

    if ( !cache || !aglyph )
      return FT_Err_Invalid_Argument;

    *aglyph   = 0;
    queue     = cache->root.last_queue;
    img_queue = (FTC_Image_Queue)queue;
    if ( !queue || memcmp( &img_queue->description, desc, sizeof ( *desc ) ) )
    {
      error = FT_Lru_Lookup( cache->root.queues_lru,
                             (FT_LruKey)desc,
                             (FT_Pointer*)&queue );
      cache->root.last_queue = queue;
      if ( error )
        goto Exit;
    }

    error = FTC_Glyph_Queue_Lookup_Node( queue, gindex, &node );
    if ( error )
      goto Exit;

    /* now compress the manager's cache pool if needed */
    manager = cache->root.root.manager;
    if ( manager->num_bytes > manager->max_bytes )
    {
      FTC_GlyphNode_Ref   ( node );
      FTC_Manager_Compress( manager );
      FTC_GlyphNode_Unref ( node );
    }

    *aglyph = ((FTC_GlyphImage)node)->ft_glyph;

  Exit:
    return error;
  }


/* END */
