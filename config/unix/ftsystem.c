/**************************************************************************
 *
 *  ftsystem.h                                                        1.0
 *
 *    Unix-specific FreeType low-level system interface
 *
 *    This file contains the definition of interface used by FreeType
 *    to access low-level, i.e. memory management, i/o access as well
 *    as thread synchronisation.              
 *
 *
 *  Copyright 1996-1999 by                                                   
 *  David Turner, Robert Wilhelm, and Werner Lemberg                         
 *                                                                           
 *  This file is part of the FreeType project, and may only be used          
 *  modified and distributed under the terms of the FreeType project         
 *  license, LICENSE.TXT.  By continuing to use, modify, or distribute       
 *  this file you indicate that you have read the license and                 
 *  understand and accept it fully.                                          
 *                                                                           
 **************************************************************************/

#include <ftsystem.h>
#include <fterrors.h>
#include <ftconfig.h>
#include <ftdebug.h>

/* Memory-mapping includes and definitions..                            */
/*                                                                      */
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <sys/mman.h>
#ifndef MAP_FILE
#define MAP_FILE  0x00
#endif

/*
 * The prototype for munmap() is not provided on SunOS.  This needs to
 * have a check added later to see if the GNU C library is being used.
 * If so, then this prototype is not needed.
 */
#if defined(__sun__) && !defined(SVR4) && !defined(__SVR4)
  extern int  munmap( caddr_t  addr, int  len );
#endif

#include <sys/stat.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

  /*********************************************************************/
  /*                                                                   */
  /*                       MEMORY MANAGEMENT INTERFACE                 */
  /*                                                                   */

/************************************************************************
 *
 * <FuncType>
 *    FT_Alloc_Func
 *
 * <Description>
 *    The memory allocator function type
 *
 * <Input>
 *    system    :: pointer to the system object
 *    size      :: requested size in bytes
 *
 * <Output>
 *    block     :: address of newly allocated block
 *
 * <Return>  
 *    Error code. 0 means success.
 *
 * <Note>
 *    If your allocation routine ALWAYS zeroes the new block, you
 *    should set the flag FT_SYSTEM_FLAG_ALLOC_ZEROES in your system
 *    object 'flags' field.
 *
 *    If you have set the flag FT_SYSTEM_FLAG_REPORT_CURRENT_ALLOC in
 *    your system's "system_flags" field, this function should update
 *    the "current_alloc" field of the system object.
 *
 ************************************************************************/

  static
  void*  ft_alloc( FT_Memory  memory,
                   long       size )
  {
    (void)memory;
    return malloc(size);
  }


/************************************************************************
 *
 * <FuncType>
 *    FT_Realloc_Func
 *
 * <Description>
 *    The memory reallocator function type
 *
 * <Input>
 *    system   :: pointer to the system object
 *    new_size :: new requested size in bytes
 *
 * <InOut>
 *    block    :: address of block in memory
 *
 * <Return>
 *    Error code. 0 means success.
 *
 * <Note>
 *    This function is _never_ called when the system flag 
 *    FT_SYSTEM_FLAG_NO_REALLOC is set. Instead, the engine will emulate
 *    realloc through "alloc" and "free".
 *
 *    Note that this is possible due to the fact that FreeType's
 *    "FT_Realloc" always requests the _current_ size of the reallocated
 *    block as a parameter, thus avoiding memory leaks.
 *
 *    If you have set the flag FT_SYSTEM_FLAG_REPORT_CURRENT_ALLOC in
 *    your system's "system_flags" field, this function should update
 *    the "current_alloc" field of the system object.
 *
 ************************************************************************/

  static
  void*  ft_realloc( FT_Memory  memory,
                     long       cur_size,
                     long       new_size,
                     void*      block )
  {
    (void)memory;
    (void)cur_size;

    return realloc( block, new_size );
  }


/************************************************************************
 *
 * <FuncType>
 *    FT_Free_Func
 *
 * <Description>
 *    The memory release function type
 *
 * <Input>
 *    system  :: pointer to the system object
 *    block   :: address of block in memory
 *
 * <Note>
 *    If you have set the flag FT_SYSTEM_FLAG_REPORT_CURRENT_ALLOC in
 *    your system's "system_flags" field, this function should update
 *    the "current_alloc" field of the system object.
 *
 ************************************************************************/

  static
  void  ft_free( FT_Memory  memory,
                 void*      block )
  {
    (void)memory;
    free( block );
  }

  /*********************************************************************/
  /*                                                                   */
  /*                     RESOURCE MANAGEMENT INTERFACE                 */
  /*                                                                   */

/* We use the macro STREAM_File as a convenience to extract the      */
/* system-specific stream handle from a given FreeType stream object */
#define STREAM_File(stream)  ((void*)stream->descriptor.pointer)


#undef  FT_COMPONENT
#define FT_COMPONENT  trace_io

  static
  void  ft_close_stream( FT_Stream  stream )
  {
    munmap ( stream->descriptor.pointer, stream->size );
        
    stream->descriptor.pointer = NULL;
    stream->size               = 0;
    stream->base               = 0;
  }


  extern
  int  FT_New_Stream( const char*  filepathname,
                      FT_Stream    stream )
  {
    int          file;
    struct stat  stat_buf;

    /* open the file */
    file = open( filepathname, O_RDONLY );
    if (file < 0)
    {
      FT_ERROR(( "FT.Unix.Open:" ));
      FT_ERROR(( " could not open '%s'\n", filepathname ));
      return FT_Err_Cannot_Open_Stream;
    }

    if (fstat( file, &stat_buf ) < 0)
    {
      FT_ERROR(( "FT.Unix.Open:" ));
      FT_ERROR(( " could not 'fstat' file '%s'\n", filepathname ));
      goto Fail_Map;
    }
      
    stream->size     = stat_buf.st_size;
    stream->pos      = 0;
    stream->base     = mmap( NULL,
                             stream->size,
                             PROT_READ,
                             MAP_FILE | MAP_PRIVATE,
                             file,
                             0 );

    if ( (long)stream->base == -1 )
    {
      FT_ERROR(( "FT.Unix.Open:" ));
      FT_ERROR(( " Could not map file '%s'\n", filepathname ));
      goto Fail_Map;
    }

    close(file);

    stream->descriptor.pointer = stream->base;
    stream->pathname.pointer   = (char*)filepathname;
    
    stream->close = ft_close_stream;
    stream->read  = 0;
    
    FT_TRACE1(( "FT.Unix.Open:" ));
    FT_TRACE1(( " opened '%s' (%d bytes) succesfully\n",
                filepathname, stream->size ));

    return FT_Err_Ok;
    
  Fail_Map:
    close(file);
    stream->base      = NULL;
    stream->size      = 0;
    stream->pos       = 0;
    
    return FT_Err_Cannot_Open_Stream;
  }


  extern
  FT_Memory  FT_New_Memory( void )
  {
    FT_Memory  memory;
    
    memory = (FT_Memory)malloc( sizeof(*memory) );
    if (memory)
    {
      memory->user    = 0;
      memory->alloc   = ft_alloc;
      memory->realloc = ft_realloc;
      memory->free    = ft_free;
    }
    return memory;
  }

