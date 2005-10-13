/***************************************************************************/
/*                                                                         */
/*  ftcimage.c                                                             */
/*                                                                         */
/*    FreeType Image cache (body).                                         */
/*                                                                         */
/*  Copyright 2000-2001, 2003, 2004 by                                     */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

#include "ftcint.h"
#include FT_CACHE_INTERNAL_IMAGE_H
#include FT_INTERNAL_MEMORY_H

#include "ftcerror.h"


  /* finalize a given glyph image node */
  FT_EXPORT_DEF( void )
  FTC_INode_Free( FTC_INode   inode,
                  FTC_GCache  cache )
  {
    FT_Memory  memory = FTC_CACHE__MEMORY(cache);


    if ( inode->glyph )
    {
      FT_Done_Glyph( inode->glyph );
      inode->glyph = NULL;
    }

    FTC_GNode_Done( FTC_GNODE( inode ) );
    FT_FREE( inode );
  }


  /* initialize a new glyph image node */
  FT_EXPORT_DEF( FT_Error )
  FTC_INode_New( FTC_INode   *pinode,
                 FTC_GNode    gquery,
                 FTC_GCache   cache )
  {
    FT_Memory  memory = FTC_CACHE__MEMORY(cache);
    FT_Error   error;
    FTC_INode  inode;


    if ( !FT_NEW( inode ) )
    {
      FTC_GNode         gnode  = FTC_GNODE( inode );
      FTC_Family        family = gquery->family;
      FT_UInt           gindex = gquery->gindex;
      FTC_ICacheClass   clazz  = FTC_ICACHE__CLASS( cache );


      /* initialize its inner fields */
      FTC_GNode_Init( gnode, gindex, family );

      /* we will now load the glyph image */
      error = clazz->fam_load_glyph( family, gindex,
                                     FTC_CACHE__MANAGER(cache),
                                     &inode->glyph );
      if ( error )
      {
        FTC_INode_Free( inode, cache );
        inode = NULL;
      }
    }

    *pinode = inode;
    return error;
  }



  FT_EXPORT_DEF( FT_ULong )
  FTC_INode_Weight( FTC_INode  inode )
  {
    FT_ULong   size  = 0;
    FT_Glyph   glyph = inode->glyph;


    switch ( glyph->format )
    {
    case FT_GLYPH_FORMAT_BITMAP:
      {
        FT_BitmapGlyph  bitg;


        bitg = (FT_BitmapGlyph)glyph;
        size = bitg->bitmap.rows * labs( bitg->bitmap.pitch ) +
               sizeof ( *bitg );
      }
      break;

    case FT_GLYPH_FORMAT_OUTLINE:
      {
        FT_OutlineGlyph  outg;


        outg = (FT_OutlineGlyph)glyph;
        size = outg->outline.n_points *
                 ( sizeof ( FT_Vector ) + sizeof ( FT_Byte ) ) +
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


/* END */
