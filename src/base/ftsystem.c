/**************************************************************************
 *
 *  ftsystem.h                                                        1.0
 *
 *    ANSI-specific FreeType low-level system interface
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


#define STREAM_FILE(stream)  ((FILE*)stream->descriptor.pointer)

  static
  void  ft_close_stream( FT_Stream  stream )
  {
    fclose( STREAM_FILE(stream) );
  }

  static
  unsigned long  ft_io_stream( FT_Stream      stream,
                               unsigned long  offset,
                               char*          buffer,
                               unsigned long  count )
  {
    FILE*  file;
    
    file = STREAM_FILE(stream);

    fseek( file, offset, SEEK_SET );    
    return (unsigned long)fread( buffer, 1, count, file );
  }


  extern
  int  FT_New_Stream( const char*  filepathname,
                      FT_Stream    stream )
  {
    FILE*  file;
    
    file = fopen( filepathname, "rb" );
    if (!file)
      return FT_Err_Cannot_Open_Resource;
      
    fseek( file, 0, SEEK_END );
    stream->size = ftell(file);
    fseek( file, 0, SEEK_SET );
    
    stream->descriptor.pointer = file;
    stream->pos                = 0;
    
    stream->read  = ft_io_stream;
    stream->close = ft_close_stream;

    return 0;
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

