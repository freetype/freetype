/*******************************************************************
 *
 *  ftsys.c
 *
 *    ANSI-specific system operations.
 *
 *  Copyright 1996-1998 by
 *  David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 *  This file is part of the FreeType project, and may only be used
 *  modified and distributed under the terms of the FreeType project
 *  license, LICENSE.TXT.  By continuing to use, modify, or distribute 
 *  this file you indicate that you have read the license and
 *  understand and accept it fully.
 *
 *
 *  This implementation of the 'ftsys' component uses the standard
 *  ANSI C library.
 *
 *  IMPORTANT NOTE :
 *
 *    Porters, read carefully the comments in ftsys.h before trying
 *    to port this file to your system. It contains many essential
 *    remarks, and will ease your work greatly..
 *
 ******************************************************************/

#include "ftsys.h"
#include "ftstream.h"
#include "ftdebug.h"


/* The macro FT_COMPONENT is used in trace mode. It is an implicit */
/* parameter of the PTRACE and PERROR macros, used to print/log    */
/* messages during execution..                                     */
/*                                                                 */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_io


#undef  CUR_SYSTEM  /* just in case */


/* To ease porting, we use the macro SYS_STREAM to name the system-specific */
/* stream type. For example, it is a "FILE*" with the ANSI libc, it will be */
/* a file descriptor, i.e. an integer, when using the Unix system api, etc. */
 
#define  SYS_STREAM  FILE*


  /* This implementation of ftsys uses the ANSI C library. Memory       */
  /* management is performed through malloc/free, i/o access through    */
  /* fopen/fread/fseek, and no synchronisation primitive is implemented */
  /* (they contain dummy code)                                          */



/**************************************************************************/
/*                                                                        */
/*                      I/O ACCESS AND MANAGEMENT                         */
/*                                                                        */
/*  We only define the "ANSI" resource class in this class. It is         */
/*  disk-based, and provides a MRU cache, in order to only keep the file  */
/*  descriptors of the 10 most recently used resource files.              */
/*                                                                        */
/*  it simply contains two lists. One contains the "cached" resources     */
/*  with a valid FILE* pointer, the second contains all other "flushed"   */
/*  resource objects.                                                     */
/*                                                                        */

/* The FT_ANSI_File class derives from FT_ResourceRec - description :  */
/*                                                                     */
/* <Struct> FT_AnsiFileRec                                             */
/*                                                                     */
/* <Fields>                                                            */
/*                                                                     */
/* root ::                                                             */
/*    the root resource class fields.                                  */
/*                                                                     */
/* pathname ::                                                         */
/*    this is a copy of the ANSI file pathname used to open streams    */
/*    for the resource. A different implementation is free to use      */
/*    unicode chars, or file i-node numbers, etc..                     */
/*                                                                     */
/* file_size ::                                                        */
/*    the size in bytes of the resource. This field should be set to   */
/*    -1 until the resource is first opened..                          */
/*                                                                     */

#include <stdio.h>

  typedef struct FT_AnsiFileRec_
  {
    FT_ResourceRec  root;
    char*           pathname;   /* the font file's pathname           */
    FT_Long         file_size;  /* file size in bytes                 */
    
  } FT_AnsiFileRec, *FT_AnsiFile;


/* We use the macro STREAM_Name as a convenience to return a given  */
/* ANSI resource's pathname. Its "stream" argument is a FT_Resource */
/* which is typecasted to the FT_AnsiFile class                     */
#define STREAM_Name(stream)  ((FT_AnsiFile)stream->resource)->pathname

/* We use the macro STREAM_File as a convenience to extract the      */
/* system-specific stream handle from a given FreeType stream object */
#define STREAM_File(stream)  ((FILE*)stream->stream_id.pointer)



/***************************************************************************/
/*                                                                         */
/* <Function> AnsiFile_Open                                                */
/*                                                                         */
/* <Description>                                                           */
/*    This function is used to open a system-stream for a given resource.  */
/*                                                                         */
/*    Note that it must update the target FreeType stream object with the  */
/*    system-stream handle and the resource's size.                        */
/*                                                                         */
/*    Also, the 'stream->base' field must be set to NULL for disk-based    */
/*    resource, and to the address in memory of the resource's first byte  */
/*    for a memory-based one.                                              */
/*                                                                         */
/* <Input>                                                                 */
/*   resource :: the source resource                                       */
/*   stream   :: the target stream object                                  */
/*                                                                         */
/* <Return>                                                                */
/*   Error code                                                            */
/*                                                                         */
/* <Note>                                                                  */
/*   This function simply calls fopen in the resource's file pathname      */
/*                                                                         */
/*   The stream object IS NOT CREATED by this function, but by its caller. */
/*                                                                         */
/***************************************************************************/

  static
  FT_Error  AnsiFile_Open( FT_AnsiFile  resource,
                           FT_Stream    stream )
  {
    FILE*  file;
    
    /* open the file */
    file = fopen( resource->pathname, "rb" );
    if (!file)
    {
      PERROR(( "AnsiFile_Open: could not open file '%s'\n",
               resource->pathname ));
      return FT_Err_Cannot_Open_Stream;
    }

    /* compute file size if necessary */
    if ( resource->file_size < 0 )
    {
      fseek( file, 0, SEEK_END );
      resource->file_size = ftell(file);
      fseek( file, 0, SEEK_SET );
    }

    stream->resource          = (FT_Resource)resource;
    stream->stream_id.pointer = file;
    stream->size              = resource->file_size;
    
    /* it's a disk-based resource, we don't need to use the "base" and   */
    /* "cursor" fields of the stream objects                             */
    stream->base              = NULL;
    stream->cursor            = NULL;

    PTRACE1(( "AnsiFile_Open: opened '%s' (%d bytes) succesfully\n",
              resource->pathname, resource->file_size ));

    return FT_Err_Ok;
  }


/***************************************************************************/
/*                                                                         */
/* <Function> AnsiFile_Close                                               */
/*                                                                         */
/* <Description> Closes a given stream                                     */
/*                                                                         */
/* <Input>                                                                 */
/*   stream   :: the target stream object                                  */
/*                                                                         */
/* <Return>                                                                */
/*   Error code                                                            */
/*                                                                         */
/* <Note>                                                                  */
/*   This function simply calls fclose on the stream's ANSI FILE object    */
/*                                                                         */
/***************************************************************************/

  static
  FT_Error  AnsiFile_Close( FT_Stream  stream )
  {
    PTRACE1(( "Closing file '%s'\n", STREAM_Name(stream) ));
    
    fclose( STREAM_File(stream) );
    
    stream->resource          = NULL;
    stream->stream_id.pointer = NULL;
    stream->size              = 0;

    return FT_Err_Ok;
  }


/***************************************************************************/
/*                                                                         */
/* <Function> AnsiFile_Seek                                                */
/*                                                                         */
/* <Description> Seek a stream to a given position                         */
/*                                                                         */
/* <Input>                                                                 */
/*   stream   :: the target stream object                                  */
/*   position :: offset in bytes from start of resource/stream             */
/*                                                                         */
/* <Return>                                                                */
/*   Error code                                                            */
/*                                                                         */
/* <Note>                                                                  */
/*   This function simply calls fseek on the stream.                       */
/*                                                                         */
/*   The "seek" method is never called by the stream manager in the case   */
/*   of a memory-based resource (i.e. when 'stream->base' isn't NULL).     */
/*                                                                         */
/***************************************************************************/

  static
  FT_Error  AnsiFile_Seek( FT_Stream  stream,
                           FT_Long    position )
  {
    if ( fseek( STREAM_File(stream), position, SEEK_SET ) )
    {
      PERROR(( "AnsiFile_Seek : FAILED !! pos. %ld of '%s'\n",
               position, STREAM_Name(stream) ));
             
      return FT_Err_Invalid_Stream_Seek;
    }
      
    PTRACE2(( "AnsiFile_Seek : pos. %ld of '%s'\n",
              position, STREAM_Name(stream) ));
             
    return FT_Err_Ok;
  }


/***************************************************************************/
/*                                                                         */
/* <Function> AnsiFile_Skip                                                */
/*                                                                         */
/* <Description> Skip a given number of bytes in an ANSI stream.           */
/*               Useful to skip pad bytes, for example.                    */
/*                                                                         */
/* <Input>                                                                 */
/*   stream   :: the target stream object                                  */
/*   count    :: number of bytes to skip in the stream                     */
/*                                                                         */
/* <Return>                                                                */
/*   Error code                                                            */
/*                                                                         */
/* <Note>                                                                  */
/*   This function simply calls fseek on the stream.                       */
/*                                                                         */
/*   The "skip" method is never called by the stream manager in the case   */
/*   of a memory-based resource (i.e. when 'stream->base' isn't NULL).     */
/*                                                                         */
/***************************************************************************/

  static
  FT_Error  AnsiFile_Skip( FT_Stream  stream,
                           FT_Long    count )
  {
    if ( fseek( STREAM_File(stream), count, SEEK_CUR ) )
    {
      PERROR(( "AnsiFile_Skip : FAILED !! %ld bytes in '%s'\n",
               count, STREAM_Name(stream) ));
             
      return FT_Err_Invalid_Stream_Seek;
    }
      
    PTRACE2(( "AnsiFile_Skip : %ld bytes in '%s'\n",count, 
              STREAM_Name(stream) ));
             
    return FT_Err_Ok;
  }                        

/***************************************************************************/
/*                                                                         */
/* <Function> AnsiFile_Pos                                                 */
/*                                                                         */
/* <Description> Returns the current offset within an ANSI stream's        */
/*               resource.                                                 */
/*                                                                         */
/* <Input>                                                                 */
/*   stream   :: the target stream object                                  */
/*                                                                         */
/* <Output>                                                                */
/*   position :: current offset. -1 in case of error                       */
/*                                                                         */
/* <Return>                                                                */
/*   Error code.                                                           */
/*                                                                         */
/* <Note>                                                                  */
/*   This function simply calls ftell on the stream.                       */
/*                                                                         */
/*   The  "pos" method is never called by the stream manager in the case   */
/*   of a memory-based resource (i.e. when 'stream->base' isn't NULL).     */
/*                                                                         */
/***************************************************************************/

  static
  FT_Error  AnsiFile_Pos( FT_Stream  stream,
                          FT_Long*   position )
  {
    *position = ftell( STREAM_File(stream) );
    if ( *position == -1 )
    {
      PTRACE2(( "AnsiFile_Pos : FAILED !! in '%s'\n", STREAM_Name(stream) ));
      return FT_Err_Invalid_Stream_Seek;
    }
    
    PTRACE2(( "AnsiFile_Pos : for '%s'\n", STREAM_Name(stream) ));
    return FT_Err_Ok;
  }

/***************************************************************************/
/*                                                                         */
/* <Function> AnsiFile_Read                                                */
/*                                                                         */
/* <Description> Read a given number of bytes from an ANSI stream into     */
/*               memory.                                                   */
/*                                                                         */
/* <Input>                                                                 */
/*   stream   :: the target stream object                                  */
/*   buffer   :: the target read buffer where data is copied               */
/*   size     :: number of bytes to read from the stream                   */
/*                                                                         */
/* <Output>                                                                */
/*   read_bytes :: the number of bytes effectively read from the stream    */
/*                 used in case of error (i.e. FT_Err_Invalid_Stream_Read) */
/*                 by some parts of the library..                          */
/* <Return>                                                                */
/*   Error code                                                            */
/*                                                                         */
/* <Note>                                                                  */
/*   This function simply calls fread on the stream.                       */
/*                                                                         */
/*   It MUST return the error FT_Err_Invalid_Stream_Read in case of        */
/*   an over-read (i.e. reading more bytes from the stream that what       */
/*   is left int), as the stream component checks for this specific        */
/*   value..                                                                      */
/*                                                                         */
/*   The "read" method is never called by the stream manager in the case   */
/*   of a memory-based resource (i.e. when 'stream->base' isn't NULL).     */
/*                                                                         */
/***************************************************************************/

  static
  FT_Error  AnsiFile_Read( FT_Stream  stream,
                           FT_Char*   buffer,
                           FT_Long    size,
                           FT_Long*   read_bytes )
  {
    *read_bytes = fread( buffer, 1, size, STREAM_File(stream) );
    if ( *read_bytes != size )
    {
      /* Note : we can have an over-read here when called by the */
      /*        function FT_Access_Compressed_Frame. This means  */
      /*        that the following message should be a trace,    */
      /*        rather than an error for disk-based resources..  */
      /*                                                         */
      /*        the function must set the value of 'read_bytes'  */
      /*        even if it returns an error code..               */
      PTRACE2(( "AnsiFile_Read : FAILED !! read %ld bytes from '%s'\n",
               size, STREAM_Name(stream) ));

      return FT_Err_Invalid_Stream_Read;
    }

    PTRACE2(( "AnsiFile_Read : read %ld bytes to buffer 0x%08lx from '%s'\n",
              size, (long)buffer, STREAM_Name(stream) ));
    return FT_Err_Ok;
  }

/* The following table is the "virtual method table" for the 'ANSI  */
/* resource class', which methods are defined above. Its address is */
/* set in the 'interface' field of all resource objects created by  */
/* the function FT_Create_AnsiFile (see below)                      */

  static
  FTRes_InterfaceRec  FT_AnsiFile_Interface =
  {
    (FTRes_Open_Func)        AnsiFile_Open,
    (FTRes_Close_Func)       AnsiFile_Close,
    (FTRes_Seek_Func)        AnsiFile_Seek,
    (FTRes_Skip_Func)        AnsiFile_Skip,
    (FTRes_Pos_Func)         AnsiFile_Pos,
    (FTRes_Read_Func)        AnsiFile_Read,
  };


/***************************************************************************/
/*                                                                         */
/* <Function> FT_Create_Resource                                           */
/*                                                                         */
/* <Description> Creates a new resource object, of class "AnsiFile".       */
/*               This function is never called directly by the font        */
/*               drivers. Only by the higher-level part of FreeType        */
/*               (called the HLib), or client applications                 */
/*                                                                         */
/* <Input>                                                                 */
/*   pathname :: the file's pathname, in ASCII                             */
/*                                                                         */
/* <Return>                                                                */
/*   Handle/pointer to the new resource object. NULL in case of error      */
/*                                                                         */
/* <Note>                                                                  */
/*   This functions does not open a stream. It simply copies the           */
/*   pathname within a fresh new resource object.                          */
/*                                                                         */
/***************************************************************************/

  EXPORT_FUNC
  FT_Error     FT_Create_Resource( FT_Library    library,
                                   const char*   pathname,
                                   FT_Resource*  aresource )
  {
    FT_Int       len;
    FT_AnsiFile  resource;
    FT_Error     error;
    FT_System    system = library->system;
    
    if ( !pathname )
      goto Fail_Null;
      
    len = strlen(pathname);
    if (len == 0)
      goto Fail_Null;

    resource = NULL;
    
    if ( ALLOC( resource, sizeof(*resource) ) ||
         ALLOC( resource->pathname, len+1   ) )
      goto Fail_Memory;

    resource->root.library   = library;
    resource->root.interface = &FT_AnsiFile_Interface;
    resource->root.flags     = FT_RESOURCE_TYPE_DISK_BASED;
    resource->file_size      = -1;
    strcpy( resource->pathname, pathname );

    PTRACE1(( "Create_AnsiFile : Ansi resource created for '%s'\n",
              pathname ));
    
    *aresource = (FT_Resource)resource;
    return FT_Err_Ok;

  Fail_Null:    
    PERROR(( "Create_AnsiFile : null pathname !!\n" ));
    return FT_Err_Invalid_Argument;
    
  Fail_Memory:
    if (resource)
      FREE( resource->pathname );
    FREE( resource );
    PERROR(( "Create_AnsiFile : not enough memory to create resource !\n" ));
    return error;
  }

  
/***************************************************************************/
/*                                                                         */
/* <Function> FT_Destroy_Resource                                          */
/*                                                                         */
/* <Description> Destroys an ANSI resource object.                         */
/*               This function is never called directly by the font        */
/*               drivers. Only by the higher-level part of FreeType        */
/*               (called the HLib), or client applications                 */
/*                                                                         */
/* <Input>                                                                 */
/*   resource :: the Ansi resource object                                  */
/*                                                                         */
/* <Note>                                                                  */
/*   This functions does not check that runs or streams are opened for     */
/*   the resource (for now, we assume developer intelligence. We'll most   */
/*   probably lower our standard later to ease debugging ;-)               */
/*                                                                         */
/***************************************************************************/
 
  EXPORT_FUNC
  FT_Error  FT_Destroy_Resource( FT_Resource  resource )
  {
    FT_System    system = resource->library->system;
    FT_AnsiFile  ansi   = (FT_AnsiFile)resource;
    
    if ( !ansi || ansi->root.interface != &FT_AnsiFile_Interface )
    {
      PERROR(( 
        "Destroy_AnsiFile : Trying to destroy an invalid resource !!\n" ));
      return FT_Err_Invalid_Resource_Handle;
    }
    
    PTRACE1(( "Destroy_AnsiFile : destroying resource for '%s'\n",
              ansi->pathname ));

    FREE( ansi->pathname );
    FREE( ansi );

    return FT_Err_Ok;
  }

  
  
/**************************************************************************/
/*                                                                        */
/*                         MEMORY MANAGEMENT                              */
/*                                                                        */
/*                                                                        */
/*   This part copies the old FreeType 1.0 and 1.1 memory management      */
/*   scheme that was defined in the file "ttmemory.h". One can see that   */
/*                                                                        */
/*   - a set of macros is defined for the memory operations used          */
/*     by the engine ( MEM_Copy, MEM_Move, MEM_Set ). This comes from     */
/*     the fact that many compilers are able to inline these ops directly */
/*     within the compiled code, rather than generating a call to the     */
/*     C library. However, this obliges us to include the <string.h>      */
/*     header file.                                                       */
/*                                                                        */
/*     If you provide your own memory operations routine, you can get     */
/*     rid of the #include <string.h> below.                              */
/*                                                                        */
/*                                                                        */
/*   - the FT_Alloc function has several essential properties that        */
/*     MUST be retained by each port :                                    */
/*                                                                        */
/*      - it returns an error code, NOT the allocated block's base        */
/*        address                                                         */
/*                                                                        */
/*      - it takes the address of a target pointer, where the block's     */
/*        base address will be set. if the size is zero, its value        */
/*        will be NULL and the function returns successfully              */
/*                                                                        */
/*      - in case of error, the pointer's value is set to NULL and        */
/*        an error code is returned..                                     */
/*                                                                        */
/*      - the new allocated block MUST be zero-filled. This is a strong   */
/*        convetion the rest of the engine relies on                      */
/*                                                                        */
/*                                                                        */
/*                                                                        */
/*   - the FT_Free function has also its essentials :                     */
/*                                                                        */
/*      - it takes the address of a pointer which value is the block's    */
/*        base address. This is UNLIKE a standard "free" which takes the  */
/*        the block's base directly.                                      */
/*                                                                        */
/*      - it accepts succesfully the address of a pointer which value     */
/*        is NULL, in which case it simply returns.                       */
/*                                                                        */
/*      - the pointer is always set to NULL by the function               */
/*                                                                        */
/*                                                                        */
/*   - the MEM_Alloc, ALLOC and ALLOC_ARRAY macros are used by the        */
/*     library, and should NOT be modified by porters !!                  */
/*                                                                        */

/* The macro FT_COMPONENT is used in trace mode. It is an implicit */
/* parameter of the PTRACE and PERROR macros, used to print/log    */
/* messages during execution..                                     */
/*                                                                 */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_memory


#include <stdlib.h>

/**************************************************************************/
/*                                                                        */
/* <Function> FT_Alloc                                                    */
/*                                                                        */
/* <Description>                                                          */
/*    Allocates a new bloc of memory. The returned area is always         */
/*    zero-filled, this is a strong convention in many FreeType parts     */
/*                                                                        */
/* <Input>                                                                */
/*    system    :: handle to a given 'system object' where allocation     */
/*                 occurs..                                               */
/*                                                                        */
/*    size      :: size in bytes of the block to allocate                 */
/*                                                                        */
/* <Output>                                                               */
/*    P         :: pointer to the fresh new block. It should be set       */
/*                 to NULL if 'size' is 0, of in case of error..          */
/*                                                                        */
/* <Return>                                                               */
/*    FreeType error code. 0 means success.                               */
/*                                                                        */
/**************************************************************************/

  BASE_FUNC
  FT_Error  FT_Alloc( FT_System  system,
                      long       size, 
                      void*     *P ) 
  {
  
    if (!P)
    {
      PERROR(( "FT_Alloc : invalid pointer address !!\n" ));
      return FT_Err_Invalid_Argument;
    }
    
    if ( size > 0 )
    {
      *P = malloc( size );
      if ( !*P )
      {
        PERROR(( "FT_Alloc : out of memory (%ld bytes requested) !!\n",
                 size ));

        return FT_Err_Out_Of_Memory;
      }

      system->total_alloc += size;

      /* ALWAYS ZERO-FILL THE BLOCK !!!!! */   
      MEM_Set( *P, 0, size );
    }
    else
      *P = NULL;

    PTRACE2(( "FT_Alloc : size = %ld, pointer = 0x%08lx, block = 0x%08lx\n",
              size, (long)P, (long)*P ));

    return FT_Err_Ok;
  }


/**************************************************************************/
/*                                                                        */
/* <Function> FT_Realloc                                                  */
/*                                                                        */
/* <Description>                                                          */
/*    Reallocates a block of memory pointed to by '*P' to 'Size'          */
/*    bytes from the hea^, possibly changing '*P'.                        */
/*                                                                        */
/* <Input>                                                                */
/*    system    :: handle to a given 'system object' where allocation     */
/*                 occurs..                                               */
/*                                                                        */
/*    size      :: size in bytes of the block to allocate                 */
/*                                                                        */
/* <InOut>                                                                */
/*    P         :: pointer to the fresh new block. It should be set       */
/*                 to NULL if 'size' is 0, of in case of error..          */
/*                                                                        */
/* <Return>                                                               */
/*    FreeType error code. 0 means success.                               */
/*                                                                        */

  BASE_FUNC
  int  FT_Realloc( FT_System  system,
                   long       size, 
                   void*     *P )
  {
    void*  Q;
    
    if (!P)
    {
      PERROR(( "FT_Realloc : invalid pointer address !!\n" ));
      return FT_Err_Invalid_Argument;
    }

    /* if the original pointer is NULL, call FT_Alloc */    
    if (!*P)
      return FT_Alloc( system, size, P );

    /* if the new block if zero-sized, clear the current one */    
    if (size <= 0)
      return FT_Free( system, P );
    
    Q = (void*)realloc( *P, size );
    if (!Q)
    {
      PERROR(( "FT_Realloc : reallocation failed\n" ));
      return FT_Err_Out_Of_Memory;
    }

    *P = Q;    
    return FT_Err_Ok;
  }


/**************************************************************************/
/*                                                                        */
/* <Function> FT_Free                                                     */
/*                                                                        */
/* <Description>                                                          */
/*    Releases a given block of memory allocated through FT_Alloc         */
/*                                                                        */
/* <Input>                                                                */
/*    system    :: handle to a given 'system object' where allocation     */
/*                 occured..                                              */
/*                                                                        */
/*    P         :: This is the _address_ of a _pointer_ which points to   */
/*                 the allocated block. It is always set to NULL on exit  */
/*                                                                        */
/* <Return>                                                               */
/*    FreeType error code. 0 means success.                               */
/*                                                                        */
/* <Note>                                                                 */
/*    If P or *P are NULL, this function should return successfuly. This  */
/*    is a strong convention within all of FreeType and its drivers..     */
/*                                                                        */

  BASE_FUNC
  FT_Error  FT_Free( FT_System  system,
                     void*     *P )
  {
    (void)system;  /* unused parameter. Gets rid of warnings */

    PTRACE2(( "FT_Free : freeing pointer 0x%08lx (block 0x%08lx)\n",
              (long)P, (P ? (long)*P : -1) ));

    if ( !P || !*P )
      return FT_Err_Ok;

    free( *P );
    *P = NULL;

    return FT_Err_Ok;
  }

/**************************************************************************/
/*                                                                        */
/*                       SYNCHRONIZATION MANAGEMENT                       */
/*                                                                        */
/*                                                                        */
/*   This section deals with mutexes. The library can be compiled to      */
/*   three distinct thread-support levels ( namely single-threaded,       */
/*   thread-safe and re-entrant modes ).                                  */
/*                                                                        */
/*   It protects its variables through the MUTEX_Lock and MUTEX_Release   */
/*   macros which are void in single-threaded mode.                       */
/*                                                                        */
/*                                                                        */
/*   It defines a type-less mutex reference type, "TMutex", that you're   */
/*   free to redefine for your system's needs..                           */
/*                                                                        */
/*   The default implementation of ftsys.c contains only dummy functions  */
/*   which always return succesfully. you NEED to specialize them in      */
/*   order to port ftsys.c in any multi-threaded environment...           */
/*                                                                        */

/* The macro FT_COMPONENT is used in trace mode. It is an implicit */
/* parameter of the PTRACE and PERROR macros, used to print/log    */
/* messages during execution..                                     */
/*                                                                 */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_sync

#ifdef FT_CONFIG_THREADS


  BASE_FUNC
  FT_Error  FT_Mutex_Create ( FT_System  system,
                              TMutex*    mutex )
  {
    (void)system;  /* unused parameter. Gets rid of warnings */
    
    mutex = (void*)-1;
    system->num_mutexes++;
    return FT_Err_Ok;
    /* Replace this line with your own mutex creation code */
  }


  BASE_FUNC
  void  FT_Mutex_Delete ( FT_System  system,
                          TMutex*    mutex )
  {
    (void)system; /* unused parameter. Gets rid of warnings */
    
    mutex = (void*)0;
    system->num_mutexes--;
    /* Replace this line with your own mutex destruction code */
  }

  BASE_FUNC
  void  FT_Mutex_Lock   ( FT_System  system,
                          TMutex*    mutex )
  {
    /* NOTE: It is legal to call this function with a NULL argument */
    /*       in which case an immediate return is appropriate.      */
    (void)system; /* unused parameter. Gets rid of warnings */
    
    if ( !mutex )
      return;

    ; /* Insert your own mutex locking code here */
  }


  void  FT_Mutex_Release( FT_System  system,
                          TMutex*  mutex )
  {
    /* NOTE: It is legal to call this function with a NULL argument */
    /*       in which case an immediate return is appropriate       */
    (void)system; /* unused parameter. Gets rid of warnings */
    
    if ( !mutex )
      return;
    ; /* Insert your own mutex release code here */
  }

#endif /* FT_CONFIG_THREADS */




  EXPORT_FUNC
  FT_Error  FT_New_System( FT_System*  system )
  {
    *system = (FT_System)malloc( sizeof(**system) );
    if ( !*system )
      return FT_Err_Out_Of_Memory;

    /* the ANSI function 'free' is unable to  return the number   */
    /* of released bytes. Hence, the 'current_alloc' field of the */
    /* system object cannot be used                               */
    
    (*system)->system_flags = FT_SYSTEM_FLAG_TOTAL_ALLOC |
                              FT_SYSTEM_FLAG_MUTEXES;
    (*system)->total_alloc = 0;
    (*system)->num_mutexes = 0;

    /* initialise i/o management (nothing) */
    
    /* initialise synchronisation (nothing) */
    
    /* initialise streams */

    return FT_Err_Ok;
  }



  EXPORT_FUNC
  FT_Error  FT_Done_System( FT_System  system )
  {
    /* finalise syncrhonisation (nothing) */
    
    /* finalise i/o management (nothing)  */
    
    /* finalise memory management         */
    free( system );

    return FT_Err_Ok;
  }




