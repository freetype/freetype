#include <cache/ftcsbits.h>

  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                    GLYPH IMAGE NODES                          *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/


  LOCAL_FUNC_X
  void  ftc_sbit_chunk_node_destroy( FTC_ChunkNode  node )
  {
    FTC_ChunkSet   cset   = node->cset;
    FT_Memory      memory = cset->memory;
    FT_UInt        count  = node->num_elements;
    FTC_SBit       sbit   = (FTC_SBit)node->elements;
    
    for ( ; count > 0; sbit++, count-- )
      FREE( sbit->buffer );

    FREE( node );
  }


  static
  FT_Error  ftc_bitmap_copy( FT_Memory   memory,
                             FT_Bitmap*  source,
                             FTC_SBit    target )
  {
    FT_Error  error;
    FT_Int    pitch = source->pitch;
    FT_ULong  size;

    if ( pitch < 0 )
      pitch = -pitch;

    size = (FT_ULong)( pitch * source->rows );

    if ( !ALLOC( target->buffer, size ) )
      MEM_Copy( target->buffer, source->buffer, size );

    return error;
  }


  LOCAL_FUNC_X
  FT_Error  ftc_sbit_chunk_node_new( FTC_ChunkSet   cset,
                                     FT_UInt        index,
                                     FTC_SBitChunk *anode )
  {
    FT_Error         error;
    FT_Memory        memory  = cset->memory;
    FTC_SBitSet      sbitset = (FTC_SBitSet)cset;
    FTC_SBitChunk    node    = 0;
    FT_Face          face;
    FT_Size          size;


    /* allocate node */
    if ( ALLOC( node, sizeof ( *node ) ) )
      goto Exit;

    /* init its inner fields */
    error = FTC_ChunkNode_Init( FTC_CHUNKNODE(node), cset, index, 1 );
    if (error)
      goto Exit;

    /* we will now load all glyph images */
    error = FTC_Manager_Lookup_Size( cset->manager,
                                     &sbitset->description.font,
                                     &face, &size );
    if ( !error )
    {
      FT_UInt  glyph_index = index * cset->chunk_size;
      FT_UInt  load_flags  = FT_LOAD_DEFAULT;
      FT_UInt  image_type  = sbitset->description.image_type;
      FT_UInt  count       = node->num_elements;
      FTC_SBit sbit        = (FTC_SBit)node->elements;


      if ( FTC_IMAGE_FORMAT( image_type ) == ftc_image_format_bitmap )
      {
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
        {
          FT_ERROR(( "FTC_SBit_Cache: cannot load vector outlines in a"
                     " sbit cache, please check your arguments !!\n" ));
          error = FT_Err_Bad_Argument;
          goto Exit;
        }
      }

      /* always render glyphs to bitmaps */
      load_flags |= FT_LOAD_RENDER;
        
      if ( image_type & ftc_image_flag_unhinted )
        load_flags |= FT_LOAD_NO_HINTING;

      if ( image_type & ftc_image_flag_autohinted )
        load_flags |= FT_LOAD_FORCE_AUTOHINT;


      /* load a chunk of small bitmaps in a row */
      for ( ; count > 0; count--, glyph_index++ )
      {
        error = FT_Load_Glyph( face, glyph_index, load_flags );
        if (!error)
        {
          FT_Int        temp;
          FT_GlyphSlot  slot   = face->glyph;
          FT_Bitmap*    bitmap = &slot->bitmap;
          FT_Int        advance;

          /* check that our values fit in 8-bit containers !! */
#define  CHECK_SCHAR(d)  ( temp = (FT_SChar)d, temp == d )
#define  CHECK_BYTE(d)   ( temp = (FT_Byte) d, temp == d )

          advance = (slot->metrics.horiAdvance+32) >> 6;
          
          if ( CHECK_BYTE ( bitmap->rows  )     &&
               CHECK_BYTE ( bitmap->width )     &&
               CHECK_SCHAR( bitmap->pitch )     &&
               CHECK_SCHAR( slot->bitmap_left ) &&
               CHECK_SCHAR( slot->bitmap_top  ) &&
               CHECK_SCHAR( advance )           )
          {
            sbit->width   = (FT_Byte) bitmap->width;
            sbit->height  = (FT_Byte) bitmap->height;
            sbit->pitch   = (FT_SChar)bitmap->pitch;
            sbit->left    = (FT_SChar)slot->bitmap_left;
            sbit->top     = (FT_SChar)slot->bitmap_top;
            sbit->advance = (FT_SChar)advance;

            /* grab the bitmap when possible */
            if ( slot->flags & ft_glyph_own_bitmap )
            {
              slot->flags &= ~ft_glyph_own_bitmap;
              sbit->buffer = bitmap->buffer;
            }
            else
            {
              /* copy the bitmap into a new buffer - ignore error */
              ftc_bitmap_copy( memory, bitmap, sbit );
            }
          }
        }
        else
          sbit->buffer = 0;
      }

      /* ignore the errors that might have occured there      */
      /* we recognize unloaded glyphs with "sbit.buffer == 0" */
      error = 0;
    }

  Exit:
    if ( error && node )
    {
      FREE( node->elements );
      FREE( node );
    }

    *anode = node;
    return error;
  }


  /* this function is important because it is both part of */
  /* a FTC_ChunkSet_Class and a FTC_CacheNode_Class     */
  /*                                                       */
  LOCAL_FUNC_X
  FT_ULong  ftc_sbit_chunk_node_size( FTC_SBitChunk  node )
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
  FT_Error  ftc_image_set_init( FTC_ImageSet     iset,
                                FTC_Image_Desc*  type )
  {
    iset->description = *type;
    return 0;
  }


  LOCAL_FUNC_X
  FT_Bool  ftc_image_set_compare( FTC_ImageSet     iset,
                                  FTC_Image_Desc*  type )
  {
    return !memcmp( &iset->description, type, sizeof ( *type ) );
  }


  FT_CPLUSPLUS( const FTC_ChunkSet_Class )  ftc_sbit_chunk_set_class =
  {
    sizeof( FTC_ImageSetRec ),

    (FTC_ChunkSet_InitFunc)       ftc_image_set_init,
    (FTC_ChunkSet_DoneFunc)       0,
    (FTC_ChunkSet_CompareFunc)    ftc_image_set_compare,

    (FTC_ChunkSet_NewNodeFunc)    ftc_sbit_chunk_node_new,
    (FTC_ChunkSet_SizeNodeFunc)   ftc_sbit_chunk_node_size,
    (FTC_ChunkSet_DestroyNodeFunc)ftc_sbit_chunk_node_destroy
  };


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                    GLYPH IMAGE CACHE                          *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/


  FT_CPLUSPLUS( const FTC_Glyph_Cache_Class )  ftc_sbit_chunk_cache_class =
  {
    {
      sizeof( FTC_Image_CacheRec ),
      (FTC_Cache_InitFunc) FTC_Glyph_Cache_Init,
      (FTC_Cache_DoneFunc) FTC_Glyph_Cache_Done
    },
    (FTC_ChunkSet_Class*) &ftc_sbit_chunk_set_class
  };


  FT_EXPORT_FUNC( FT_Error )  FTC_Image_Cache_New( FTC_Manager       manager,
                                                   FTC_Image_Cache*  acache )
  {
    return FTC_Manager_Register_Cache(
              manager,
              (FTC_Cache_Class*)&ftc_sbit_chunk_cache_class,
              (FTC_Cache*)acache );
  }


  FT_EXPORT_DEF( FT_Error )
  FTC_Image_Cache_Lookup( FTC_Image_Cache  cache,
                          FTC_Image_Desc*  desc,
                          FT_UInt          gindex,
                          FT_Glyph*        aglyph )
  {
    FT_Error       error;
    FTC_ChunkSet   gset;
    FTC_GlyphNode  node;
    FTC_Manager    manager;

    FTC_ImageSet   img_set;


    /* check for valid `desc' delayed to FT_Lru_Lookup() */

    if ( !cache || !aglyph )
      return FT_Err_Invalid_Argument;

    *aglyph  = 0;
    gset     = cache->root.last_gset;
    img_set  = (FTC_ImageSet)gset;
    if ( !gset || memcmp( &img_set->description, desc, sizeof ( *desc ) ) )
    {
      error = FT_Lru_Lookup( cache->root.gsets_lru,
                             (FT_LruKey)desc,
                             (FT_Pointer*)&gset );
      cache->root.last_gset = gset;
      if ( error )
        goto Exit;
    }

    error = FTC_ChunkSet_Lookup_Node( gset, gindex, &node );
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

    *aglyph = ((FTC_SBitChunk)node)->ft_glyph;

  Exit:
    return error;
  }

