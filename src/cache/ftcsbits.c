#include <freetype/cache/ftcsbits.h>
#include <freetype/fterrors.h>

#define  FTC_SBITSET_ELEMENT_COUNT  16


  typedef struct FTC_SBitSetRec_
  {
    FTC_ChunkSetRec   root;
    FTC_Image_Desc    desc;

  } FTC_SBitSetRec, *FTC_SBitSet;


  typedef struct FTC_SBit_CacheRec_
  {
    FTC_Chunk_CacheRec    root;

  } FTC_SBit_CacheRec;



  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                     SBIT CACHE NODES                          *****/
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

    FREE( node->elements );
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
                                     FTC_ChunkNode *anode )
  {
    FT_Error         error;
    FT_Memory        memory  = cset->memory;
    FTC_SBitSet      sbitset = (FTC_SBitSet)cset;
    FTC_ChunkNode    node    = 0;
    FT_Face          face;
    FT_Size          size;


    /* allocate node */
    if ( ALLOC( node, sizeof ( *node ) ) )
      goto Exit;

    /* init its inner fields */
    error = FTC_ChunkNode_Init( node, cset, index, 1 );
    if (error)
      goto Exit;

    /* we will now load all glyph images for this chunk */
    error = FTC_Manager_Lookup_Size( cset->manager,
                                     &sbitset->desc.font,
                                     &face, &size );
    if ( !error )
    {
      FT_UInt  glyph_index = index * cset->element_count;
      FT_UInt  load_flags  = FT_LOAD_DEFAULT;
      FT_UInt  image_type  = sbitset->desc.image_type;
      FT_UInt  count       = node->num_elements;
      FTC_SBit sbit        = (FTC_SBit)node->elements;

      /* determine load flags, depending on the font description's */
      /* image type..                                              */
      {
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
          FT_ERROR(( "FTC_SBit_Cache: cannot load scalable glyphs in a"
                     " sbit cache, please check your arguments !!\n" ));
          error = FT_Err_Invalid_Argument;
          goto Exit;
        }

        /* always render glyphs to bitmaps */
        load_flags |= FT_LOAD_RENDER;

        if ( image_type & ftc_image_flag_unhinted )
          load_flags |= FT_LOAD_NO_HINTING;

        if ( image_type & ftc_image_flag_autohinted )
          load_flags |= FT_LOAD_FORCE_AUTOHINT;
      }


      /* load a chunk of small bitmaps in a row */
      for ( ; count > 0; count--, glyph_index++, sbit++ )
      {
        /* by default, indicates a "missing" glyph */
        sbit->buffer = 0;

        error = FT_Load_Glyph( face, glyph_index, load_flags );
        if (!error)
        {
          FT_Int        temp;
          FT_GlyphSlot  slot   = face->glyph;
          FT_Bitmap*    bitmap = &slot->bitmap;
          FT_Int        xadvance, yadvance;

          /* check that our values fit in 8-bit containers !!       */
          /* if this is not the case, our bitmap is too large       */
          /* and we will leave it as "missing" with sbit.buffer = 0 */

#define  CHECK_CHAR(d)   ( temp = (FT_Char)d, temp == d )
#define  CHECK_BYTE(d)   ( temp = (FT_Byte)d, temp == d )

          /* FIXME: add support for vertical layouts maybe.. */

          /* horizontal advance in pixels              */
          xadvance = (slot->metrics.horiAdvance+32) >> 6;
          yadvance = (slot->metrics.vertAdvance+32) >> 6;

          if ( CHECK_BYTE ( bitmap->rows  )     &&
               CHECK_BYTE ( bitmap->width )     &&
               CHECK_CHAR ( bitmap->pitch )     &&
               CHECK_CHAR ( slot->bitmap_left ) &&
               CHECK_CHAR ( slot->bitmap_top  ) &&
               CHECK_CHAR ( xadvance )          &&
               CHECK_CHAR ( yadvance )          )
          {
            sbit->width    = (FT_Byte) bitmap->width;
            sbit->height   = (FT_Byte) bitmap->rows;
            sbit->pitch    = (FT_Char) bitmap->pitch;
            sbit->left     = (FT_Char) slot->bitmap_left;
            sbit->top      = (FT_Char) slot->bitmap_top;
            sbit->xadvance = (FT_Char) xadvance;
            sbit->yadvance = (FT_Char) yadvance;
            sbit->format   = (FT_Byte) bitmap->pixel_mode;

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
  /* a FTC_ChunkSet_Class and a FTC_CacheNode_Class        */
  /*                                                       */
  LOCAL_FUNC_X
  FT_ULong  ftc_sbit_chunk_node_size( FTC_ChunkNode  node )
  {
    FT_ULong      size;
    FTC_ChunkSet  cset  = node->cset;
    FT_UInt       count = node->num_elements;
    FT_Int        pitch;
    FTC_SBit      sbit  = (FTC_SBit)node->elements;

    size  = sizeof (*node);                            /* the node itself */
    size += cset->element_count * sizeof(FTC_SBitRec); /* the sbit recors */

    for ( ; count > 0; count--, sbit++ )
    {
      if (sbit->buffer)
      {
        pitch = sbit->pitch;
        if (pitch < 0)
          pitch = -pitch;

        /* add the size of a given glyph image */
        size += pitch * sbit->height;
      }
    }

    return size;
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                     SBIT CHUNK SETS                           *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/


  LOCAL_FUNC_X
  FT_Error  ftc_sbit_chunk_set_sizes( FTC_ChunkSet     cset,
                                      FTC_Image_Desc*  desc )
  {
    FT_Error  error;
    FT_Face   face;

    cset->element_count = FTC_SBITSET_ELEMENT_COUNT;
    cset->element_size  = sizeof(FTC_SBitRec);

    /* lookup the FT_Face to obtain the number of glyphs */
    error = FTC_Manager_Lookup_Face( cset->manager,
                                     desc->font.face_id, &face );
    if (!error)
      cset->element_max = face->num_glyphs;

    return error;
  }



  LOCAL_FUNC_X
  FT_Error  ftc_sbit_chunk_set_init( FTC_SBitSet      sset,
                                     FTC_Image_Desc*  type )
  {
    sset->desc = *type;
    return 0;
  }


  LOCAL_FUNC_X
  FT_Bool  ftc_sbit_chunk_set_compare( FTC_SBitSet      sset,
                                       FTC_Image_Desc*  type )
  {
    return !memcmp( &sset->desc, type, sizeof ( *type ) );
  }


  FT_CPLUSPLUS( const FTC_ChunkSet_Class )  ftc_sbit_chunk_set_class =
  {
    sizeof( FTC_SBitSetRec ),

    (FTC_ChunkSet_InitFunc)        ftc_sbit_chunk_set_init,
    (FTC_ChunkSet_DoneFunc)        0,
    (FTC_ChunkSet_CompareFunc)     ftc_sbit_chunk_set_compare,
    (FTC_ChunkSet_SizesFunc)       ftc_sbit_chunk_set_sizes,

    (FTC_ChunkSet_NewNodeFunc)     ftc_sbit_chunk_node_new,
    (FTC_ChunkSet_SizeNodeFunc)    ftc_sbit_chunk_node_size,
    (FTC_ChunkSet_DestroyNodeFunc) ftc_sbit_chunk_node_destroy
  };


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                     SBITS CACHE                               *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/


  FT_CPLUSPLUS( const FTC_Chunk_Cache_Class )  ftc_sbit_cache_class =
  {
    {
      sizeof( FTC_SBit_CacheRec ),
      (FTC_Cache_InitFunc) FTC_Chunk_Cache_Init,
      (FTC_Cache_DoneFunc) FTC_Chunk_Cache_Done
    },
    (FTC_ChunkSet_Class*) &ftc_sbit_chunk_set_class
  };


  FT_EXPORT_FUNC( FT_Error )
  FTC_SBit_Cache_New( FTC_Manager      manager,
                      FTC_SBit_Cache*  acache )
  {
    return FTC_Manager_Register_Cache(
              manager,
              (FTC_Cache_Class*)&ftc_sbit_cache_class,
              (FTC_Cache*)acache );
  }


  FT_EXPORT_DEF( FT_Error )
  FTC_SBit_Cache_Lookup( FTC_SBit_Cache   cache,
                         FTC_Image_Desc*  desc,
                         FT_UInt          gindex,
                         FTC_SBit        *asbit )
  {
    FT_Error       error;
    FTC_ChunkSet   cset;
    FTC_ChunkNode  node;
    FT_UInt        cindex;
    FTC_Manager    manager;

    FTC_SBitSet    sset;
    FTC_SBit       sbit;


    /* check for valid `desc' delayed to FT_Lru_Lookup() */

    if ( !cache || !asbit )
      return FT_Err_Invalid_Argument;

    *asbit   = 0;
    cset     = cache->root.last_cset;
    sset     = (FTC_SBitSet)cset;
    if ( !cset || memcmp( &sset->desc, desc, sizeof ( *desc ) ) )
    {
      error = FT_Lru_Lookup( cache->root.csets_lru,
                             (FT_LruKey)desc,
                             (FT_Pointer*)&cset );
      cache->root.last_cset = cset;
      if ( error )
        goto Exit;
    }

    error = FTC_ChunkSet_Lookup_Node( cset, gindex, &node, &cindex );
    if ( error )
      goto Exit;

    /* now compress the manager's cache pool if needed */
    manager = cache->root.root.manager;
    if ( manager->num_bytes > manager->max_bytes )
    {
      FTC_ChunkNode_Ref   ( node );
      FTC_Manager_Compress( manager );
      FTC_ChunkNode_Unref ( node );
    }

    sbit   = ((FTC_SBit)((FTC_ChunkNode)node)->elements) + cindex;
    *asbit = sbit;

  Exit:
    return error;
  }

