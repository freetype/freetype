/* memtest.c */

#include <freetype.h>
#include <ftobjs.h>
#include <stdio.h>
#include <stdlib.h>


  EXPORT_DEF
  void  FT_Default_Drivers( FT_Library  library );


/* Our own memory allocator. To check that a single block isn't freed */
/* several time, we simply do not call "free"..                       */


#define MAX_RECORDED_BLOCKS  4096
#define CHECK_DUPLICATES

typedef  struct MyBlock
{
  void*  base;
  long   size;

} MyBlock;

static  MyBlock my_blocks[ MAX_RECORDED_BLOCKS ];
static  int     num_my_blocks = 0;

/* record a new block in the table, check for duplicates too */
static
void  record_my_block( void*  base, long  size )
{
  if (size <= 0)
  {
    fprintf( stderr, "adding a block with non-positive length - should not happen \n" );
    exit(1);
  }
  
  if ( num_my_blocks < MAX_RECORDED_BLOCKS )
  {
    MyBlock*  block;
    
#ifdef CHECK_DUPLICATES
    MyBlock*  limit;
    block = my_blocks;
    limit = block + num_my_blocks;
    for ( ; block < limit; block++ )
    {
      if ( block->base == base && block->size != 0 )
      {
        fprintf( stderr, "duplicate memory block at %08lx\n", (long)block->base );
        exit(1);
      }
    }
#endif

    block = my_blocks + num_my_blocks++;
    block->base = base;
    block->size = size;
  }
  else
  {
    fprintf( stderr, "Too many memory blocks -- test exited !!\n" );
    exit(1);
  }
}

/* forget a block, and check that it isn't part of our table already */
static
void  forget_my_block( void*  base )
{
  MyBlock*  block = my_blocks + num_my_blocks-1;
  
  /* we scan in reverse, because transient blocks are always located */
  /* at the end of the table.. (it supposedly faster then..)         */
  for ( ; block >= my_blocks; block-- )
  {
    if ( block->base == base )
    {
      if (block->size > 0)
      {
        block->size = 0;
        return;
      }
      else
      {
        fprintf( stderr, "Block at %08lx released twice \n", (long)base );
        exit(1);
      }
    }
  }
  fprintf( stderr, "Trying to release an unallocated block at %08lx\n",
                   (long)base );
  exit(1);
}


static
void*  my_alloc( FT_Memory  memory,
                 long       size )
{
  void*  p = malloc(size);
  if (p)
    record_my_block(p,size);
    
  return p;
}

static
void   my_free( FT_Memory memory, void*  block )
{
  forget_my_block(block);
  free(block);
}

static
void*  my_realloc( FT_Memory memory,
                   long      cur_size,
                   long      new_size,
                   void*     block )
{
  void*  p;

  p = my_alloc( memory, new_size );
  if (p)
  {
    my_free( memory, block );
    record_my_block( p, new_size );
  }
  return p;
}


struct FT_MemoryRec_  my_memory =
{
  0,
  my_alloc,
  my_free,
  my_realloc
};


int  main( void )
{
    FT_Library  library;
    FT_Face     face;
    int         glyphIndex;
    int         result;

    /* Create a new library with our own memory manager */
    result = FT_New_Library( &my_memory, &library );
    
    /* the new library has no drivers in it, add the default ones */
    /* (implemented in ftinit.c)..                                */
    FT_Default_Drivers(library);

    result = FT_New_Face( library, "d:/ttf/arial.ttf", 0, &face );
    result = FT_Set_Char_Size( face, 0, 16*64, 96, 96 );

    glyphIndex = FT_Get_Char_Index( face, (int)'A' );

    /* memory error occurs in FT_DoneFreeType() if FT_Load_Glyph() is called */
    result = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT );

    result = FT_Done_Face( face );

    result = FT_Done_FreeType( library ); 

    return 0;
}



