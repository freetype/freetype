/***************************************************************************/
/*                                                                         */
/*  ftcsbits.c                                                             */
/*                                                                         */
/*    FreeType sbits manager (body).                                       */
/*                                                                         */
/*  Copyright 2000-2001 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#include <ft2build.h>
#include FT_CACHE_H
#include FT_CACHE_SMALL_BITMAPS_H
#include FT_CACHE_INTERNAL_CHUNK_H
#include FT_INTERNAL_OBJECTS_H
#include FT_INTERNAL_DEBUG_H
#include FT_ERRORS_H

#include "ftcerror.h"

#include <string.h>         /* memcmp() */


#define FTC_SBIT_ITEMS_PER_NODE  16


 /* handle to sbit set */
  typedef struct FTC_SBitSetRec_*   FTC_SBitSet;

 /* sbit set structure */
  typedef struct  FTC_SBitSetRec_
  {
    FTC_ChunkSetRec  cset;
    FTC_Image_Desc   desc;

  } FTC_SBitSetRec;

#define  FTC_SBIT_SET(x)  ((FTC_SBitSet)(x))

#define  FTC_SBIT_SET_MEMORY(x)  FTC_CHUNK_SET_MEMORY(&(x)->cset)


  typedef struct FTC_SBitQueryRec_
  {
    FTC_ChunkQueryRec  chunk;
    FTC_Image_Desc     desc;

  } FTC_SBitQueryRec, *FTC_SBitQuery;


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                     SBIT CACHE NODES                          *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/


  FT_CALLBACK_DEF( void )
  ftc_sbit_node_done( FTC_ChunkNode  cnode )
  {
    FTC_ChunkSet  cset   = cnode->cset;
    FT_Memory     memory = cset->ccache->cache.memory;
    FT_UInt       count  = cnode->item_count;
    FTC_SBit      sbit   = (FTC_SBit) cnode->items;

    if ( sbit )
    {
      for ( ; count > 0; sbit++, count-- )
        FREE( sbit->buffer );

      FREE( cnode->items );
    }

    ftc_chunk_node_done( cnode );
  }


  static FT_Error
  ftc_sbit_set_bitmap( FTC_SBit    sbit,
                       FT_Bitmap*  bitmap,
                       FT_Memory   memory )
  {
    FT_Error  error;
    FT_Int    pitch = bitmap->pitch;
    FT_ULong  size;


    if ( pitch < 0 )
      pitch = -pitch;

    size = (FT_ULong)( pitch * bitmap->rows );

    if ( !ALLOC( sbit->buffer, size ) )
      MEM_Copy( sbit->buffer, bitmap->buffer, size );

    return error;
  }



  static FT_Error
  ftc_sbit_node_load( FTC_ChunkNode  cnode,
                      FT_UInt        gindex,
                      FT_ULong      *asize )
  {
    FT_Error       error;
    FTC_ChunkSet   cset    = cnode->cset;
    FTC_SBitSet    sbitset = FTC_SBIT_SET(cset);
    FT_Memory      memory  = FTC_SBIT_SET_MEMORY(sbitset);
    FT_Face        face;
    FT_Size        size;

    FTC_SBit       sbit;

    if ( gindex <  (FT_UInt)cnode->item_start                     ||
         gindex >= (FT_UInt)cnode->item_start + cnode->item_count )
    {
      FT_ERROR(( "FreeType.cache.sbit_load: invalid glyph index" ));
      return FTC_Err_Invalid_Argument;
    }

    sbit = (FTC_SBit)cnode->items + (gindex - cnode->item_start);

    error = FTC_Manager_Lookup_Size( cset->ccache->cache.manager,
                                     &sbitset->desc.font,
                                     &face, &size );
    if ( !error )
    {
      FT_UInt   load_flags  = FT_LOAD_DEFAULT;
      FT_UInt   image_type  = sbitset->desc.image_type;


      /* determine load flags, depending on the font description's */
      /* image type                                                */

      if ( FTC_IMAGE_FORMAT( image_type ) == ftc_image_format_bitmap )
      {
        if ( image_type & ftc_image_flag_monochrome )
          load_flags |= FT_LOAD_MONOCHROME;

        /* disable embedded bitmaps loading if necessary */
        if ( image_type & ftc_image_flag_no_sbits )
          load_flags |= FT_LOAD_NO_BITMAP;
      }
      else
      {
        FT_ERROR(( "FreeType.cache.sbit_load: cannot load scalable glyphs in an"
                   " sbit cache, please check your arguments!\n" ));
        error = FTC_Err_Invalid_Argument;
        goto Exit;
      }

      /* always render glyphs to bitmaps */
      load_flags |= FT_LOAD_RENDER;

      if ( image_type & ftc_image_flag_unhinted )
        load_flags |= FT_LOAD_NO_HINTING;

      if ( image_type & ftc_image_flag_autohinted )
        load_flags |= FT_LOAD_FORCE_AUTOHINT;

      /* by default, indicates a `missing' glyph */
      sbit->buffer = 0;

      error = FT_Load_Glyph( face, gindex, load_flags );
      if ( !error )
      {
        FT_Int        temp;
        FT_GlyphSlot  slot   = face->glyph;
        FT_Bitmap*    bitmap = &slot->bitmap;
        FT_Int        xadvance, yadvance;


        /* check that our values fit into 8-bit containers!       */
        /* If this is not the case, our bitmap is too large       */
        /* and we will leave it as `missing' with sbit.buffer = 0 */

#define CHECK_CHAR( d )  ( temp = (FT_Char)d, temp == d )
#define CHECK_BYTE( d )  ( temp = (FT_Byte)d, temp == d )

        /* XXX: FIXME: add support for vertical layouts maybe */

        /* horizontal advance in pixels */
        xadvance = ( slot->metrics.horiAdvance + 32 ) >> 6;
        yadvance = ( slot->metrics.vertAdvance + 32 ) >> 6;

        if ( CHECK_BYTE( bitmap->rows  )     &&
             CHECK_BYTE( bitmap->width )     &&
             CHECK_CHAR( bitmap->pitch )     &&
             CHECK_CHAR( slot->bitmap_left ) &&
             CHECK_CHAR( slot->bitmap_top  ) &&
             CHECK_CHAR( xadvance )          &&
             CHECK_CHAR( yadvance )          )
        {
          sbit->width    = (FT_Byte) bitmap->width;
          sbit->height   = (FT_Byte) bitmap->rows;
          sbit->pitch    = (FT_Char) bitmap->pitch;
          sbit->left     = (FT_Char) slot->bitmap_left;
          sbit->top      = (FT_Char) slot->bitmap_top;
          sbit->xadvance = (FT_Char) xadvance;
          sbit->yadvance = (FT_Char) yadvance;
          sbit->format   = (FT_Byte) bitmap->pixel_mode;

          /* grab the bitmap when possible - this is a hack !! */
          if ( slot->flags & ft_glyph_own_bitmap )
          {
            slot->flags &= ~ft_glyph_own_bitmap;
            sbit->buffer = bitmap->buffer;
          }
          else
          {
            /* copy the bitmap into a new buffer -- ignore error */
            error = ftc_sbit_set_bitmap( sbit, bitmap, memory );
          }

          /* now, compute size */
          if ( asize )
            *asize = ABS(sbit->pitch) * sbit->height;
          
        }  /* glyph dimensions ok */

      } /* glyph loading successful */

      /* ignore the errors that might have occurred --        */
      /* we recognize unloaded glyphs with `sbit.buffer == 0' */
      /* and 'width == 255', 'height == 0'                    */
      /*                                                      */
      if ( error )
      {
        sbit->width  = 255;
        error        = 0;
        /* sbit->buffer == NULL too !! */
      }
    }

  Exit:
    return error;
  }


  FT_CALLBACK_DEF( FT_Error )
  ftc_sbit_node_init( FTC_ChunkNode   cnode,
                      FTC_ChunkQuery  query )
  {
    FT_Error       error;

    error = ftc_chunk_node_init( cnode,
                                 query->cset,
                                 query->gindex,
                                 TRUE );
    if ( !error )
    {
      error = ftc_sbit_node_load( cnode, query->gindex, NULL );

      if ( error )
        ftc_chunk_node_done( cnode );
    }

    return error;
  }



  /* this function is important because it is both part of */
  /* an FTC_ChunkSet_Class and an FTC_CacheNode_Class      */
  /*                                                       */
  FT_CALLBACK_DEF( FT_ULong )
  ftc_sbit_node_weight( FTC_ChunkNode  cnode )
  {
    FT_ULong      size;
    FTC_ChunkSet  cset  = cnode->cset;
    FT_UInt       count = cnode->item_count;
    FT_Int        pitch;
    FTC_SBit      sbit  = (FTC_SBit) cnode->items;


    /* the node itself */
    size  = sizeof ( *cnode );

    /* the sbit records */
    size += cnode->item_count * sizeof ( FTC_SBitRec );

    for ( ; count > 0; count--, sbit++ )
    {
      if ( sbit->buffer )
      {
        pitch = sbit->pitch;
        if ( pitch < 0 )
          pitch = -pitch;

        /* add the size of a given glyph image */
        size += pitch * sbit->height;
      }
    }

    return size;
  }


  FT_CALLBACK_DEF( FT_Bool )
  ftc_sbit_node_compare( FTC_ChunkNode  cnode,
                         FTC_SBitQuery  query,
                         FTC_Cache      cache )
  {
    FTC_ChunkQuery  creq  = &query->chunk;
    FT_UInt         gindex = query->chunk.gindex;
    FT_UInt         offset = (FT_UInt)(gindex - cnode->item_start);
    FT_Bool         result;

    result = FT_BOOL( offset < (FT_UInt)cnode->item_count &&
                      creq->cset == cnode->cset );
    if ( result )
    {
      /* check if we need to load the glyph bitmap now */
      FTC_SBit  sbit = (FTC_SBit)cnode->items + offset;
      
      if ( sbit->buffer == NULL && sbit->width != 255 )
      {
        FT_ULong  size;
        ftc_sbit_node_load( cnode, gindex, &size );
        cache->manager->cur_weight += size;
      }
    }
    return result;
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                     SBIT CHUNK SETS                           *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/


  FT_CALLBACK_DEF( FT_Error )
  ftc_sbit_set_init( FTC_SBitSet    sset,
                     FTC_SBitQuery  query,
                     FT_LruList     lru )
  {
    FTC_ChunkCache  ccache  = lru->user_data;
    FTC_Manager     manager = ccache->cache.manager;
    FT_Error        error;
    FT_Face         face;

    sset->desc = query->desc;

    /* we need to compute "cquery.item_total" now */
    error = FTC_Manager_Lookup_Face( manager,
                                     query->desc.font.face_id,
                                     &face );
    if ( !error )
    {
      ftc_chunk_set_init( FTC_CHUNK_SET(sset),
                          sizeof( FTC_SBitRec ),
                          FTC_SBIT_ITEMS_PER_NODE,
                          face->num_glyphs,
                          FTC_CHUNK_CACHE(lru->user_data) );

      /* now compute hash from description - this is _very_ important */
      /* for good performance..                                       */
      sset->cset.hash   = FTC_IMAGE_DESC_HASH( &sset->desc );
      query->chunk.cset = &sset->cset;
    }

    return error;
  }



  FT_CALLBACK_DEF( FT_Bool )
  ftc_sbit_set_compare( FTC_SBitSet    sset,
                        FTC_SBitQuery  query )
  {
    FT_Bool  result;

    /* we need to set the "cquery.cset" field or our query for */
    /* faster glyph comparisons in ftc_sbit_node_compare..         */
    /*                                                             */
    result = FT_BOOL( FTC_IMAGE_DESC_COMPARE( &sset->desc, &query->desc ) );
    if ( result )
      query->chunk.cset = &sset->cset;

    return result;
  }



  FT_CALLBACK_TABLE_DEF
  const FT_LruList_ClassRec  ftc_sbit_set_class =
  {
    sizeof( FT_LruListRec ),
    (FT_LruList_InitFunc)     NULL,
    (FT_LruList_DoneFunc)     NULL,

    sizeof( FTC_SBitSetRec ),
    (FT_LruNode_InitFunc)     ftc_sbit_set_init,
    (FT_LruNode_DoneFunc)     ftc_chunk_set_done,
    (FT_LruNode_FlushFunc)    NULL,
    (FT_LruNode_CompareFunc)  ftc_sbit_set_compare,
  };



  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                     SBITS CACHE                               *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/


  FT_CALLBACK_DEF( FT_Error )
  ftc_sbit_cache_init( FTC_SBit_Cache  scache )
  {
    return ftc_chunk_cache_init( FTC_CHUNK_CACHE(scache),
                                 &ftc_sbit_set_class );
  }

  FT_CALLBACK_TABLE_DEF
  const FTC_Cache_ClassRec  ftc_sbit_cache_class =
  {
    sizeof( FTC_ChunkCacheRec ),
    (FTC_Cache_InitFunc)  ftc_sbit_cache_init,
    (FTC_Cache_DoneFunc)  ftc_chunk_cache_done,

    sizeof( FTC_ChunkNodeRec ),
    (FTC_Node_InitFunc)     ftc_sbit_node_init,
    (FTC_Node_WeightFunc)   ftc_sbit_node_weight,
    (FTC_Node_CompareFunc)  ftc_sbit_node_compare,
    (FTC_Node_DoneFunc)     ftc_sbit_node_done
  };


  /* documentation is in ftcsbits.h */

  FT_EXPORT_DEF( FT_Error )
  FTC_SBit_Cache_New( FTC_Manager      manager,
                      FTC_SBit_Cache  *acache )
  {
    return FTC_Manager_Register_Cache(
             manager,
             &ftc_sbit_cache_class,
             (FTC_Cache*) acache );
  }


  /* documentation is in ftcsbits.h */

  FT_EXPORT_DEF( FT_Error )
  FTC_SBit_Cache_Lookup( FTC_SBit_Cache   cache,
                         FTC_Image_Desc*  desc,
                         FT_UInt          gindex,
                         FTC_SBit        *ansbit )
  {
    FT_Error          error;
    FTC_ChunkCache    ccache = (FTC_ChunkCache) cache;
    FTC_ChunkNode     node;
    FTC_SBitQueryRec  query;


    /* argument checks delayed to FTC_Chunk_Cache_Lookup */
    if ( !ansbit )
      return FTC_Err_Invalid_Argument;

    *ansbit = NULL;

    query.chunk.gindex = gindex;
    query.chunk.cset   = NULL;
    query.desc         = *desc;
    
    error = ftc_chunk_cache_lookup( ccache, &query.chunk, &node );
    if ( !error )
    {
      *ansbit = (FTC_SBit) node->items + (gindex - node->item_start);
    }
    return error;
  }


/* END */
