/***************************************************************************/
/*                                                                         */
/*  ftcimage.c                                                             */
/*                                                                         */
/*    FreeType Image cache (body).                                         */
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
#include FT_CACHE_IMAGE_H
#include FT_CACHE_INTERNAL_GLYPH_H
#include FT_INTERNAL_MEMORY_H

#include "ftcerror.h"

#include <string.h>     /* memcmp() */
#include <stdlib.h>     /* labs()   */


  /* the FT_Glyph image node type */
  typedef struct  FTC_ImageNodeRec_
  {
    FTC_GlyphNodeRec  gnode;
    FT_Glyph          glyph;

  } FTC_ImageNodeRec, *FTC_ImageNode;

#define  FTC_IMAGE_NODE(x)         ((FTC_ImageNode)(x))
#define  FTC_IMAGE_NODE_GINDEX(x)  FTC_GLYPH_NODE_GINDEX(x)

  /* the glyph image set type */
  typedef struct  FTC_ImageSetRec_
  {
    FTC_GlyphSetRec  gset;
    FTC_Image_Desc   description;

  } FTC_ImageSetRec, *FTC_ImageSet;

#define  FTC_IMAGE_SET(x)  ((FTC_ImageSet)(x))

#define  FTC_IMAGE_SET_MEMORY(x)  FTC_GLYPH_SET_MEMORY(&(x)->gset)


  typedef struct FTC_ImageQueryRec_
  {
    FTC_GlyphQueryRec  glyph;
    FTC_Image_Desc     desc;
  
  } FTC_ImageQueryRec, *FTC_ImageQuery;


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                    GLYPH IMAGE NODES                          *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/


 /* finalize a given glyph image node */
  FT_CALLBACK_DEF( void )
  ftc_image_node_done( FTC_ImageNode  inode )
  {
    if (inode->glyph)
    {
      FT_Done_Glyph( inode->glyph );
      inode->glyph = NULL;
    }
  }


 /* initialize a new glyph image node */
  FT_CALLBACK_DEF( FT_Error )
  ftc_image_node_init( FTC_ImageNode   inode,
                       FTC_GlyphQuery  query )
  {
    FTC_ImageSet    iset   = FTC_IMAGE_SET( query->gset );
    FT_Memory       memory = FTC_IMAGE_SET_MEMORY( iset );
    FT_Error        error;
    FT_Face         face;
    FT_Size         size;

    /* initialize its inner fields */
    ftc_glyph_node_init( FTC_GLYPH_NODE(inode), query->gindex, query->gset );

    /* we will now load the glyph image */
    error = FTC_Manager_Lookup_Size( iset->gset.gcache->cache.manager,
                                     &iset->description.font,
                                     &face, &size );
    if ( !error )
    {
      FT_UInt  gindex      = FTC_GLYPH_NODE_GINDEX(inode);
      FT_UInt  load_flags  = FT_LOAD_DEFAULT;
      FT_UInt  image_type  = iset->description.image_type;


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

      error = FT_Load_Glyph( face, gindex, load_flags );
      if ( !error )
      {
        if ( face->glyph->format == ft_glyph_format_bitmap  ||
             face->glyph->format == ft_glyph_format_outline )
        {
          /* ok, copy it */
          FT_Glyph  glyph;


          error = FT_Get_Glyph( face->glyph, &glyph );
          if ( !error )
          {
            inode->glyph = glyph;
            goto Exit;
          }
        }
        else
          error = FTC_Err_Invalid_Argument;
      }
    }
    
    /* in case of error */
    ftc_glyph_node_done( FTC_GLYPH_NODE(inode) );

  Exit:
    return error;
  }


  FT_CALLBACK_DEF( FT_ULong )
  ftc_image_node_weight( FTC_ImageNode  inode )
  {
    FT_ULong  size  = 0;
    FT_Glyph  glyph = inode->glyph;

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

    size += sizeof ( *inode );
    return size;
  }


 /* this function assumes that the desired node's glyph set has been */
 /* set by a previous call to ftc_image_set_compare..                */
 /*                                                                  */
  FT_CALLBACK_DEF( FT_Bool )
  ftc_image_node_compare( FTC_ImageNode   inode,
                          FTC_ImageQuery  iquery )
  {
    FTC_ImageSet  iset = FTC_IMAGE_SET(inode->gnode.gset);

   /* only if same glyph index and image set description */
    return FT_BOOL( iquery->glyph.gindex == FTC_IMAGE_NODE_GINDEX(inode) &&
                    iquery->glyph.gset   == inode->gnode.gset );
  }



  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                    GLYPH IMAGE SETS                           *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/


  FT_CALLBACK_DEF( FT_Error )
  ftc_image_set_init( FTC_ImageSet     iset,
                      FTC_ImageQuery   query,
                      FT_LruList       lru )
  {
    ftc_glyph_set_init( &iset->gset, lru );
    iset->description = query->desc;
    
    /* now compute hash from description - this is _very_ important */
    iset->gset.hash   = FTC_IMAGE_DESC_HASH(&query->desc);
    query->glyph.gset = FTC_GLYPH_SET(iset);

    return 0;
  }


  FT_CALLBACK_DEF( FT_Bool )
  ftc_image_set_compare( FTC_ImageSet    iset,
                         FTC_ImageQuery  iquery )
  {
    FT_Bool  result;

    /* we must set iquery.glyph.gset for faster glyph node comparisons */
    result = FT_BOOL( FTC_IMAGE_DESC_COMPARE( &iset->description, &iquery->desc ) );
    if ( result )
      iquery->glyph.gset = &iset->gset;
    
    return result;
  }


  FT_CALLBACK_TABLE_DEF
  const FT_LruList_ClassRec  ftc_image_set_class =
  {
    sizeof( FT_LruListRec ),
    (FT_LruList_InitFunc) NULL,
    (FT_LruList_DoneFunc) NULL,
    
    sizeof( FTC_ImageSetRec ),
    (FT_LruNode_InitFunc)    ftc_image_set_init,
    (FT_LruNode_DoneFunc)    ftc_glyph_set_init,
    (FT_LruNode_FlushFunc)   NULL,
    (FT_LruNode_CompareFunc) ftc_image_set_compare
  };


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                    GLYPH IMAGE CACHE                          *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/


  FT_CALLBACK_DEF( FT_Error )
  ftc_image_cache_init( FTC_Image_Cache  cache )
  {
    return ftc_glyph_cache_init( (FTC_GlyphCache) cache, &ftc_image_set_class );
  }
  

  FT_CALLBACK_TABLE_DEF
  const FTC_Cache_ClassRec  ftc_image_cache_class =
  {
    sizeof( FTC_GlyphCacheRec ),
    (FTC_Cache_InitFunc)    ftc_image_cache_init,
    (FTC_Cache_DoneFunc)    ftc_glyph_cache_done,
    
    sizeof( FTC_ImageNodeRec ),
    (FTC_Node_InitFunc)     ftc_image_node_init,
    (FTC_Node_WeightFunc)   ftc_image_node_weight,
    (FTC_Node_CompareFunc)  ftc_image_node_compare,
    (FTC_Node_DoneFunc)     ftc_image_node_done
  };


  /* documentation is in ftcimage.h */

  FT_EXPORT_DEF( FT_Error )
  FTC_Image_Cache_New( FTC_Manager       manager,
                       FTC_Image_Cache  *acache )
  {
    return FTC_Manager_Register_Cache(
             manager,
             (FTC_Cache_Class) &ftc_image_cache_class,
             FTC_CACHE_P(acache) );
  }


  /* documentation is in ftcimage.h */

  FT_EXPORT_DEF( FT_Error )
  FTC_Image_Cache_Acquire( FTC_Image_Cache  cache,
                           FTC_Image_Desc*  desc,
                           FT_UInt          gindex,
                           FT_Glyph        *aglyph,
                           FTC_Node        *anode )
  {
    FTC_ImageQueryRec  query;
    FTC_ImageNode      node;
    FT_Error           error;
    
    /* some argument checks are delayed to ftc_glyph_cache_lookup */
    if ( !cache || !desc || !aglyph )
      return FTC_Err_Invalid_Argument;

    *aglyph = NULL;
    
    if ( anode )
      *anode  = NULL;

    query.glyph.gindex = gindex;
    query.glyph.gset   = NULL;
    query.desc         = *desc;
    error = ftc_glyph_cache_lookup( FTC_GLYPH_CACHE(cache),
                                    &query.glyph,
                                    (FTC_GlyphNode*) &node );
    if (!error)
    {
      *aglyph = node->glyph;
      
      if (anode)
      {
        *anode = (FTC_Node) node;
        FTC_NODE(node)->ref_count++;
      }
    }

    return error;
  }                           


  FT_EXPORT_DEF( void )
  FTC_Image_Cache_Release( FTC_Image_Cache  icache,
                           FTC_Node         node )
  {
    ftc_node_unref( node, FTC_CACHE(icache) );
  }




  FT_EXPORT_DEF( FT_Error )
  FTC_Image_Cache_Lookup( FTC_Image_Cache  icache,
                          FTC_Image_Desc*  desc,
                          FT_UInt          gindex,
                          FT_Glyph        *aglyph )
  {
    return FTC_Image_Cache_Acquire( icache, desc, gindex, aglyph, NULL );
  }



/* END */
