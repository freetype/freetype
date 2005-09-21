/***************************************************************************/
/*                                                                         */
/*  ftcbasic.c                                                             */
/*                                                                         */
/*    The FreeType basic cache interface (body).                           */
/*                                                                         */
/*  Copyright 2003, 2004, 2005 by                                          */
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
#include FT_CACHE_INTERNAL_GLYPH_H
#include FT_CACHE_INTERNAL_IMAGE_H
#include FT_CACHE_INTERNAL_SBITS_H
#include FT_INTERNAL_MEMORY_H

#include "ftcerror.h"


  /*
   *  Basic Families
   *
   */
  typedef struct  FTC_BasicFamilyRec_
  {
    FTC_FamilyRec   family;
    FTC_ScalerRec   scaler;
    FT_UInt         load_flags;

  } FTC_BasicFamilyRec, *FTC_BasicFamily;


#define  FTC_BASIC_FAMILY_HASH(f)  \
     ( FTC_SCALER_HASH( &(f)->scaler ) + 31*(f)->load_flags )


  FT_CALLBACK_DEF( FT_Bool )
  ftc_basic_family_equal( FTC_BasicFamily  family,
                          FTC_BasicFamily  query )
  {
    return ( FTC_SCALER_COMPARE( &(family)->scaler, &(query)->scaler ) &&
             family->load_flags == query->load_flags                   );
  }


  FT_CALLBACK_DEF( FT_Bool )
  ftc_basic_family_equal_faceid( FTC_BasicFamily  family,
                                 FTC_FaceID       face_id )
  {
    return FT_BOOL( family->scaler.face_id == face_id );
  }


  FT_CALLBACK_DEF( FT_UInt )
  ftc_basic_family_get_count( FTC_BasicFamily  family,
                              FTC_Manager      manager )
  {
    FT_Error  error;
    FT_Face   face;
    FT_UInt   result = 0;


    error = FTC_Manager_LookupFace( manager, family->scaler.face_id, &face );
    if ( !error )
      result = face->num_glyphs;

    return result;
  }


  FT_CALLBACK_DEF( FT_Error )
  ftc_basic_family_load_bitmap( FTC_BasicFamily  family,
                                FT_UInt          gindex,
                                FTC_Manager      manager,
                                FT_Face         *aface )
  {
    FT_Error         error;
    FT_Size          size;


    error = FTC_Manager_LookupSize( manager, &family->scaler, &size );
    if ( !error )
    {
      FT_Face  face = size->face;


      error = FT_Load_Glyph( face, gindex,
                             family->load_flags | FT_LOAD_RENDER );
      if ( !error )
        *aface = face;
    }

    return error;
  }


  FT_CALLBACK_DEF( FT_Error )
  ftc_basic_family_load_glyph( FTC_BasicFamily  family,
                               FT_UInt          gindex,
                               FTC_Manager      manager,
                               FT_Glyph        *aglyph )
  {
    FT_Error    error;
    FTC_Scaler  scaler = &family->scaler;
    FT_Face     face;
    FT_Size     size;


    /* we will now load the glyph image */
    error = FTC_Manager_LookupSize( manager, scaler, &size );
    if ( !error )
    {
      face = size->face;

      error = FT_Load_Glyph( face, gindex, family->load_flags );
      if ( !error )
      {
        if ( face->glyph->format == FT_GLYPH_FORMAT_BITMAP  ||
             face->glyph->format == FT_GLYPH_FORMAT_OUTLINE )
        {
          /* ok, copy it */
          FT_Glyph  glyph;


          error = FT_Get_Glyph( face->glyph, &glyph );
          if ( !error )
          {
            *aglyph = glyph;
            goto Exit;
          }
        }
        else
          error = FTC_Err_Invalid_Argument;
      }
    }

  Exit:
    return error;
  }


 /*
  *
  * basic image cache
  *
  */

  FT_CALLBACK_TABLE_DEF
  const FTC_ICacheClassRec  ftc_basic_image_cache_class =
  FTC_DEFINE_ICACHE_CLASS(
      FTC_DEFINE_GCACHE_CLASS(
          FTC_DEFINE_CACHE_CLASS(
              FTC_INode_New,
              FTC_INode_Weight,
              FTC_GNode_Equal,
              FTC_GNode_EqualFaceID,
              FTC_INode_Free,
              FTC_GCacheRec,
              FTC_GCache_Init,
              FTC_GCache_Done 
           ),
           FTC_DEFINE_FAMILY_CLASS(
              FTC_BasicFamilyRec,
              0 /* init */,
              0 /* done */,
              ftc_basic_family_equal,
              ftc_basic_family_equal_faceid
           )
      ),
      ftc_basic_family_load_glyph
  );

  /* documentation is in ftcache.h */

  FT_EXPORT_DEF( FT_Error )
  FTC_ImageCache_New( FTC_Manager      manager,
                      FTC_ImageCache  *acache )
  {
    return FTC_Manager_RegisterCache( manager,
                          (FTC_CacheClass) &ftc_basic_image_cache_class,
                          (FTC_Cache*)acache );
  }


  /* documentation is in ftcache.h */

  FT_EXPORT_DEF( FT_Error )
  FTC_ImageCache_Lookup( FTC_ImageCache  cache,
                         FTC_ImageType   type,
                         FT_UInt         gindex,
                         FT_Glyph       *aglyph,
                         FTC_Node       *anode )
  {
    FTC_BasicFamilyRec  key_family;
    FTC_GNodeRec        key;
    FTC_Node            node = 0;  /* make compiler happy */
    FT_Error            error;
    FT_UInt32           hash;


    /* some argument checks are delayed to FTC_Cache_Lookup */
    if ( !aglyph )
    {
      error = FTC_Err_Invalid_Argument;
      goto Exit;
    }

    *aglyph = NULL;
    if ( anode )
      *anode  = NULL;

    key_family.scaler.face_id = type->face_id;
    key_family.scaler.width   = type->width;
    key_family.scaler.height  = type->height;
    key_family.scaler.pixel   = 1;
    key_family.scaler.x_res   = 0;  /* make compilers happy */
    key_family.scaler.y_res   = 0;
    key_family.load_flags     = type->flags;

    hash = FTC_BASIC_FAMILY_HASH( &key_family );

    FTC_GCACHE_GET_FAMILY( cache, ftc_basic_family_equal,
                           hash, &key_family, &key.family, error );
    if ( !error )
    {
      hash      += gindex;
      key.gindex = gindex;

      FTC_CACHE_LOOKUP_CMP( cache, FTC_GNODE_EQUAL, hash,
                            &key, node, error );
      if ( !error )
      {
        *aglyph = FTC_INODE( node )->glyph;

        if ( anode )
          *anode = FTC_NODE_REF( node );
      }

      FTC_Family_Unref( FTC_FAMILY(key.family) );
    }

  Exit:
    return error;
  }


 /*
  *
  * basic small bitmap cache
  *
  */
  FT_CALLBACK_TABLE_DEF
  const FTC_SCacheClassRec  ftc_basic_sbit_cache_class =
  FTC_DEFINE_SCACHE_CLASS(
      FTC_DEFINE_GCACHE_CLASS(
          FTC_DEFINE_CACHE_CLASS(
              FTC_SNode_New,
              FTC_SNode_Weight,
              FTC_SNode_Equal,
              FTC_GNode_EqualFaceID,
              FTC_SNode_Free,
              FTC_GCacheRec,
              FTC_GCache_Init,
              FTC_GCache_Done
          ),
          FTC_DEFINE_FAMILY_CLASS(
              FTC_BasicFamilyRec,
              0 /* init */,
              0 /* done */,
              ftc_basic_family_equal,
              ftc_basic_family_equal_faceid
          )
      ),
      ftc_basic_family_get_count,
      ftc_basic_family_load_bitmap
  );

  /* documentation is in ftcache.h */

  FT_EXPORT_DEF( FT_Error )
  FTC_SBitCache_New( FTC_Manager     manager,
                     FTC_SBitCache  *acache )
  {
    return FTC_Manager_RegisterCache( manager,
                          (FTC_CacheClass) &ftc_basic_sbit_cache_class,
                          (FTC_Cache*)acache );
  }


  /* documentation is in ftcache.h */

  FT_EXPORT_DEF( FT_Error )
  FTC_SBitCache_Lookup( FTC_SBitCache  cache,
                        FTC_ImageType  type,
                        FT_UInt        gindex,
                        FTC_SBit      *ansbit,
                        FTC_Node      *anode )
  {
    FTC_BasicFamilyRec  key_family;
    FTC_GNodeRec        key;
    FT_Error            error;
    FTC_Node            node = 0; /* make compiler happy */
    FT_UInt32           hash;


    if ( anode )
      *anode = NULL;

    /* other argument checks delayed to FTC_Cache_Lookup */
    if ( !ansbit )
      return FTC_Err_Invalid_Argument;

    *ansbit = NULL;

    key_family.scaler.face_id = type->face_id;
    key_family.scaler.width   = type->width;
    key_family.scaler.height  = type->height;
    key_family.scaler.pixel   = 1;
    key_family.scaler.x_res   = 0;  /* make compilers happy */
    key_family.scaler.y_res   = 0;
    key_family.load_flags     = type->flags;

    hash = FTC_BASIC_FAMILY_HASH( &key_family );

    FTC_GCACHE_GET_FAMILY( cache, ftc_basic_family_equal,
                           hash, &key_family, &key.family, error );
    if ( !error )
    {
      /* beware, the hash must be the same for all glyph ranges */
      hash += gindex/FTC_SBIT_ITEMS_PER_NODE;

      key.gindex = gindex;

      FTC_CACHE_LOOKUP_CMP( cache, FTC_SNODE_EQUAL, hash,
                            &key, node, error );
      if ( !error )
      {
        *ansbit = FTC_SNODE(node)->sbits + ( gindex - FTC_GNODE(node)->gindex );

        if ( anode )
          *anode = FTC_NODE_REF(node);
      }

      FTC_Family_Unref( FTC_FAMILY(key.family) );
    }

    return error;
  }

/* END */
