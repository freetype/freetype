/***************************************************************************/
/*                                                                         */
/*  memdebug.c                                                             */
/*                                                                         */
/*    Memory debugging functions (body only).                              */
/*                                                                         */
/*  Copyright 1996-1999 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used        */
/*  modified and distributed under the terms of the FreeType project       */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#include <stdio.h>
#include <stdlib.h>


  typedef struct  TBlockRec_
  {
    char*  base;
    long   size;

  } TBlockRec;


  static TBlockRec*  mem_blocks;
  static int         num_mem_blocks;
  static int         max_mem_blocks;


  void  DM_Init_Mem()
  {
    num_mem_blocks = 0;
    max_mem_blocks = 4096;
    mem_blocks     = (TBlockRec*)malloc( max_mem_blocks *
                                         sizeof ( *mem_blocks ) );
  }


  void  DM_Done_Mem()
  {
    /* Now print the remaining blocks */
    if ( num_mem_blocks == 0 )
    {
      fprintf( stderr, "No memory leaked!\n" );
    }
    else
    {
      int  i;


      fprintf( stderr, "There were %d leaked memory blocks\n\n",
               num_mem_blocks );

      fprintf( stderr, "base     size\n" );
      fprintf( stderr, "------------------\n" );

      for ( i = 0; i < num_mem_blocks; i++ )
      {
        fprintf( stderr, "%08lx  %04lx\n", 
                 (long)mem_blocks[i].base, mem_blocks[i].size );
      }
    }
    free( mem_blocks );
  }


  void  DM_Record( char*  base,
                   long   size )
  {
    TBlockRec*  block;


#if 0
    /* First, check that the block is not located within one of the */
    /* recorded blocks                                              */
    for ( i = 0; i < num_mem_blocks; i++ )
    {
      char  *start, *end, *_limit, *_base;


      _base  = mem_blocks[i].base;
      _limit = _base + mem_blocks[i].size;

      start = base;
      end   = base + size - 1;

      if ( ( start >= base && start < limit ) ||
           ( end >= base && end < limit )     )
      {
        fprintf( stderr, "Warning: Recording an invalid block!\n" );
      }
    }
#endif

    /* Add block to list */
    if ( num_mem_blocks >= max_mem_blocks )
    {
      max_mem_blocks *= 2;
      mem_blocks = realloc( mem_blocks,
                            max_mem_blocks * sizeof ( *mem_blocks ) );
    }
    block = mem_blocks + num_mem_blocks;
    block->base = base;
    block->size = size;
    num_mem_blocks++;
  }


  void  DM_Forget( char*  base )
  {
    TBlockRec*  block = mem_blocks;
    int         i;


    for ( i = 0; i < num_mem_blocks; i++, block++ )
    {
      if ( block->base == base )
      {
        /* simply move last block to the current position */
        if ( num_mem_blocks > 1 )
          *block = mem_blocks[num_mem_blocks - 1];

        num_mem_blocks--;
        return;
      }

#if 1
      if ( base >= block->base && base < block->base + block->size )
      {
        fprintf( stderr, "Invalid block forgotten!\n" );
      }
#endif
    }
  }


/* END */
