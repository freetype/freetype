/* memtest.c */

#include <freetype/freetype.h>
#include <freetype/ftmodule.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

  FT_Error      error;

  FT_Library    library;
  FT_Face       face;

  unsigned int  num_glyphs;
  int           ptsize;

  int  Fail;
  int  Num;




/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/* Our own memory allocator. To check that a single block isn't freed */
/* several time, we simply do not call "free"..                       */

#define MAX_RECORDED_BLOCKS  1638400
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
        fprintf( stderr, "Block at %p released twice \n", base );
        exit(1);
      }
    }
  }
  fprintf( stderr, "Trying to release an unallocated block at %p\n",
                   base );
  exit(1);
}


static
void*  my_alloc( FT_Memory  memory,
                 long       size )
{
  void*  p = malloc(size);
  if (p)
    record_my_block(p,size);

  memory=memory;
  return p;
}

static
void   my_free( FT_Memory memory, void*  block )
{
  memory=memory;
  forget_my_block(block);
  /* free(block);  WE DO NOT REALLY FREE THE BLOCK */
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
    long  size;
    
    size = cur_size;
    if (new_size < size)
      size = new_size;
      
    memcpy( p, block, size );
    my_free( memory, block );
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

static void  dump_mem( void )
{
  MyBlock*  block = my_blocks + num_my_blocks-1;
  int       bad   = 0;

  printf( "total allocated blocks = %d\n", num_my_blocks );

  /* we scan in reverse, because transient blocks are always located */
  /* at the end of the table.. (it supposedly faster then..)         */
  for ( ; block >= my_blocks; block-- )
  {
    if (block->size > 0)
    {
      fprintf( stderr, "%p (%6ld bytes) leaked !!\n", block->base, (long)block->size );
      bad = 1;
    }
  }
  if (!bad)
    fprintf( stderr, "no leaked memory block\n\n" );
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


  static void  Usage( char*  name )
  {
    printf( "memtest: simple memory tester -- part of the FreeType project\n" );
    printf( "-------------------------------------------------------------\n" );
    printf( "\n" );
    printf( "Usage: %s ppem fontname[.ttf|.ttc] [fontname2..]\n", name );
    printf( "\n" );

    exit( 1 );
  }


  static void  Panic( const char*  message )
  {
    fprintf( stderr, "%s\n  error code = 0x%04x\n", message, error );
    exit(1);
  }


int  main( int argc, char** argv )
{
    int           i, file_index;
    unsigned int  id;
    char          filename[128 + 4];
    char          alt_filename[128 + 4];
    char*         execname;
    char*         fname;

    execname = argv[0];

    if ( argc < 3 )
      Usage( execname );

    if ( sscanf( argv[1], "%d", &ptsize ) != 1 )
      Usage( execname );

    /* Create a new library with our own memory manager */
    error = FT_New_Library( &my_memory, &library );
    if (error) Panic( "Could not create library object" );

    /* the new library has no drivers in it, add the default ones */
    /* (implemented in ftinit.c)..                                */
    FT_Add_Default_Modules(library);


    /* Now check all files */
    for ( file_index = 2; file_index < argc; file_index++ )
    {
      fname = argv[file_index];
      i     = strlen( fname );
      while ( i > 0 && fname[i] != '\\' && fname[i] != '/' )
      {
        if ( fname[i] == '.' )
          i = 0;
        i--;
      }

      filename[128] = '\0';
      alt_filename[128] = '\0';

      strncpy( filename, fname, 128 );
      strncpy( alt_filename, fname, 128 );

#ifndef macintosh
      if ( i >= 0 )
      {
        strncpy( filename + strlen( filename ), ".ttf", 4 );
        strncpy( alt_filename + strlen( alt_filename ), ".ttc", 4 );
      }
#endif
      i     = strlen( filename );
      fname = filename;

      while ( i >= 0 )
#ifndef macintosh
        if ( filename[i] == '/' || filename[i] == '\\' )
#else
        if ( filename[i] == ':' )
#endif
        {
          fname = filename + i + 1;
          i = -1;
        }
        else
          i--;

      printf( "%s: ", fname );

      /* Load face */
      error = FT_New_Face( library, filename, 0, &face );
      if (error)
      {
        if (error == FT_Err_Invalid_File_Format)
          printf( "unknown format\n" );
        else
          printf( "could not find/open file (error: %d)\n", error );
        continue;
      }
      if (error) Panic( "Could not open file" );

      num_glyphs = face->num_glyphs;

      error = FT_Set_Char_Size( face, ptsize << 6, ptsize << 6, 72, 72 );
      if (error) Panic( "Could not set character size" );

      Fail = 0;
      {
        for ( id = 0; id < num_glyphs; id++ )
        {
          error = FT_Load_Glyph( face, id, FT_LOAD_RENDER | FT_LOAD_ANTI_ALIAS );
          if (error)
          {
            if ( Fail < 10 )
              printf( "glyph %4u: 0x%04x\n" , id, error );
            Fail++;
          }
        }
      }

      if ( Fail == 0 )
        printf( "OK.\n" );
      else
        if ( Fail == 1 )
          printf( "1 fail.\n" );
        else
          printf( "%d fails.\n", Fail );

      FT_Done_Face( face );
    }

    FT_Done_FreeType(library);

    dump_mem();

    exit( 0 );      /* for safety reasons */
    return 0;       /* never reached */
}



