/***************************************************************************/
/*                                                                         */
/*  ftsystem.h                                                             */
/*                                                                         */
/*    FreeType low-level system interface definition (specification).      */
/*                                                                         */
/*  Copyright 1996-2000 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __FTSYSTEM_H__
#define __FTSYSTEM_H__


#include <ft2build.h>


FT_BEGIN_HEADER

  /**************************************************************************
   *
   *  <Section> system_interface
   *
   *  <Title> System Interface
   *
   *  <Abstract>
   *     How FreeType manages memory and i/o
   *
   *  <Description>
   *     This section contains various definitions related to memory
   *     management and i/o access. You'll need to understand this
   *     information if you want to use a custom memory manager or
   *     you own input i/o streams
   *
   */

  /*************************************************************************/
  /*                                                                       */
  /*                  M E M O R Y   M A N A G E M E N T                    */
  /*                                                                       */
  /*************************************************************************/

 /**********************************************************************
  *
  * @type: FT_Memory
  *
  * @description:
  *   a handle to a given memory manager object, defined with a
  *   @FT_MemoryRec structure.
  */
  typedef struct FT_MemoryRec_*  FT_Memory;


 /**********************************************************************
  *
  * @functype: FT_Alloc_Func
  *
  * @description:
  *   a function used to allocate "size" bytes from "memory"
  *
  * @input:
  *   memory :: handle to source memory manager
  *   size   :: size in bytes to allocate
  *
  * @return:
  *   address of new memory block. 0 in case of failure
  */
  typedef void*  (*FT_Alloc_Func)( FT_Memory  memory,
                                   long       size );


 /**********************************************************************
  *
  * @functype: FT_Free_Func
  *
  * @description:
  *   a function used to release a given block of memory
  *
  * @input:
  *   memory :: handle to source memory manager
  *   block  :: address of target memory block
  */
  typedef void   (*FT_Free_Func)( FT_Memory  memory,
                                  void*      block );

 /**********************************************************************
  *
  * @functype: FT_Realloc_Func
  *
  * @description:
  *   a function used to re-allocate a given block of memory
  *
  * @input:
  *   memory   :: handle to source memory manager
  *   cur_size :: the block's current size in bytes
  *   new_size :: the block's requested new size
  *   block    :: the block's current address
  *
  * @return:
  *   new block address. 0 in case of memory shortage.
  *
  * @note:
  *   note that in case of error, the old block must still be available
  */
  typedef void*  (*FT_Realloc_Func)( FT_Memory  memory,
                                     long       cur_size,
                                     long       new_size,
                                     void*      block );


 /**********************************************************************
  *
  * @struct: FT_MemoryRec
  *
  * @description:
  *   a structure used to describe a given memory manager to FreeType 2
  *
  * @fields:
  *    user    ::
  *    alloc   :: 
  *    free    ::
  *    realloc ::
  *
  */
  struct FT_MemoryRec_
  {
    void*            user;
    FT_Alloc_Func    alloc;
    FT_Free_Func     free;
    FT_Realloc_Func  realloc;
  };


  /*************************************************************************/
  /*                                                                       */
  /*                       I / O   M A N A G E M E N T                     */
  /*                                                                       */
  /*************************************************************************/


 /************************************************************************
  *
  * @type: FT_Stream
  *
  * @description:
  *   a handle to an input stream.
  */
  typedef struct FT_StreamRec_*  FT_Stream;



 /************************************************************************
  *
  * @struct: FT_StreamDesc
  *
  * @description:
  *   a union type used to store either a long or a pointer. This is
  *   used to store a file descriptor or a FILE* in an input stream
  *
  */
  typedef union  FT_StreamDesc_
  {
    long   value;
    void*  pointer;

  } FT_StreamDesc;


 /************************************************************************
  *
  * @functype: FT_Stream_IO
  *
  * @description:
  *   a function used to seek and read data from a given input stream
  *
  * @input:
  *   stream :: handle to source stream
  *   offset :: offset of read in stream (always from start)
  *   buffer :: address of read buffer
  *   count  :: number of bytes to read from the stream
  *
  * @return:
  *   number of bytes effectively read by the stream
  *
  * @note:
  *   this function might be called to perform seek / skip with
  *   a "count" of 0
  */
  typedef unsigned long  (*FT_Stream_IO)( FT_Stream       stream,
                                          unsigned long   offset,
                                          unsigned char*  buffer,
                                          unsigned long   count );

 /************************************************************************
  *
  * @functype: FT_Stream_Close
  *
  * @description:
  *   a function used to close a given input stream
  *
  * @input:
  *   stream :: handle to target stream
  */
  typedef void  (*FT_Stream_Close)( FT_Stream  stream );


 /************************************************************************
  *
  * @struct: FT_StreamRec
  *
  * @description:
  *   a structure used to describe an input stream
  *
  * @input:
  *   base       :: for memory-based stream, this is the address of the first
  *                 stream byte in memory. this field should always be set to
  *                 NULL for disk-based streams.
  *
  *   size       :: the stream size in bytes
  *   pos        :: the current position within the stream
  *
  *   descriptor :: this field is a union that can hold an integer or a pointer
  *                 it is used by stream implementations to store file
  *                 descriptors or FILE* pointers..
  *
  *   pathname   :: this field is completely ignored by FreeType, however,
  *                 it's often useful during debugging to use it to store
  *                 the stream's filename, where available
  *
  *   read       :: the stream's input function
  *   close      :: the stream close function
  *
  *   memory     :: memory manager to use to preload frames. this is set
  *                 internally by FreeType and shouldn't be touched by
  *                 stream implementations
  *
  *   cursor     :: this field is set and used internally by FreeType
  *                 when parsing frames.
  *
  *   limit      :: this field is set and used internally by FreeType
  *                 when parsing frames.
  */
  struct  FT_StreamRec_
  {
    unsigned char*   base;
    unsigned long    size;
    unsigned long    pos;

    FT_StreamDesc    descriptor;
    FT_StreamDesc    pathname;
    FT_Stream_IO     read;
    FT_Stream_Close  close;

    FT_Memory        memory;
    unsigned char*   cursor;
    unsigned char*   limit;
  };


  /* */


FT_END_HEADER

#endif /* __FTSYSTEM_H__ */


/* END */
