/*******************************************************************
 *
 *  ftsys.c
 *
 *    Unix-specific system operations.
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
 *  This implementation of the 'ftsys' component uses memory-mapped
 *  files, as well as the ANSI malloc/free functions..
 *
 *  IMPORTANT NOTE :
 *
 *    Porters, read carefully the comments in ftsys.h before trying
 *    to port this file to your system. It contains many essential
 *    remarks, and will ease your work greatly..
 *
 ******************************************************************/

#include "ftsys.h"
#include "ftobjs.h"
#include "ftstream.h"
#include "ftdebug.h"

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


/* The macro FT_COMPONENT is used in trace mode. It is an implicit */
/* parameter of the PTRACE and PERROR macros, used to print/log    */
/* messages during execution..                                     */
/*                                                                 */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_io




/* To ease porting, we use the macro SYS_STREAM to name the system-specific */
/* stream type. For example, it is a "FILE*" with the ANSI libc, it will be */
/* a file descriptor, i.e. an integer, when using the Unix system api, etc. */
 
/* we identify each memory-mapped file through its address in memory */
/* hence the following macro definition..                            */
 
#define  SYS_STREAM  void*


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

/* The FT_MMapFile class derives from FT_ResourceRec - description :   */
/*                                                                     */
/* <Struct> FT_AnsiFileRec                                             */
/*                                                                     */
/* <Fields>                                                            */
/*                                                                     */
/* root ::                                                             */
/*    the root resource class fields.                                  */
/*                                                                     */
/* pathname ::                                                         */
/*    the file's pathname. Needed because we open and close font       */
/*    resources dynamically in order to limit the number of            */
/*    concurrently active mappings (this saves kernel resources).      */
/*                                                                     */
/* file_size ::                                                        */
/*    the size in bytes of the resource. This field should be set to   */
/*    -1 until the resource is first opened..                          */
/*                                                                     */

#include <stdio.h>

  typedef struct FT_MMapFileRec_
  {
    FT_ResourceRec  root;
    const char*     pathname;
    FT_Long         file_size;  /* file size in bytes */
    
  } FT_MMapFileRec, *FT_MMapFile;


/* We use the macro STREAM_Name as a convenience to return a given  */
/* ANSI resource's pathname. Its "stream" argument is a FT_Resource */
/* which is typecasted to the FT_AnsiFile class                     */
#define STREAM_Name(stream)  ((FT_MMapFile)stream->resource)->pathname

/* We use the macro STREAM_File as a convenience to extract the      */
/* system-specific stream handle from a given FreeType stream object */
#define STREAM_File(stream)  ((void*)stream->stream_id.pointer)



/***************************************************************************/
/*                                                                         */
/* <Function> MMapFile_Open                                                */
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
/*   This function simply opens and maps the resource's file pathname      */
/*                                                                         */
/*   The stream object IS NOT CREATED by this function, but by its caller. */
/*                                                                         */
/***************************************************************************/

  static
  FT_Error  MMapFile_Open( FT_MMapFile  resource,
                           FT_Stream    stream )
  {
    int          file;
    struct stat  stat_buf;
    
    /* open the file */
    file = open( resource->pathname, O_RDONLY );
    if (file < 0)
    {
      PERROR(( "UnixSys.MMapFile_Open : could not open '%s'\n",
               resource->pathname ));
      return FT_Err_Cannot_Open_Stream;
    }

    if (fstat( file, &stat_buf ) < 0)
    {
      PERROR(( "UnixSys.MMapFile_Open : could not 'fstat' file '%s'\n",
               resource->pathname ));
      goto Fail_Map;
    }
      
    if ( resource->file_size < 0 )
      resource->file_size = stat_buf.st_size;
      
    stream->resource          = (FT_Resource)resource;
    stream->system            = resource->root.driver->system;
    stream->size              = resource->file_size;
    stream->stream_id.pointer = mmap( NULL,
                                      resource->file_size,
                                      PROT_READ,
                                      MAP_FILE | MAP_PRIVATE,
                                      file,
                                      0 );

    if ( (long)stream->stream_id.pointer == -1 )
    {
      PERROR(( "UnixSys.MMapFile_Open : Could not map file '%s'\n",
               resource->pathname ));
      goto Fail_Map;
    }

    close(file);
    stream->base   = (FT_Byte*)stream->stream_id.pointer;
    stream->cursor = stream->base;
    
    PTRACE1(( "UnixSys.MMapFile_Open: opened '%s' (%d bytes) succesfully\n",
              resource->pathname, resource->file_size ));

    return FT_Err_Ok;
    
  Fail_Map:
    close(file);
    stream->resource          = NULL;
    stream->size              = 0;
    stream->stream_id.pointer = NULL;
    stream->base              = NULL;
    stream->cursor            = NULL;
    
    return FT_Err_Cannot_Open_Stream;
  }


/***************************************************************************/
/*                                                                         */
/* <Function> MMapFile_Close                                               */
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
/*   This function simply unmaps the resource..                            */
/*                                                                         */
/***************************************************************************/

  static
  FT_Error  MMapFile_Close( FT_Stream  stream )
  {
    PTRACE1(( "Closing file '%s'\n", STREAM_Name(stream) ));
    
    munmap ( (void*)stream->stream_id.pointer, stream->size );
        
    stream->resource          = NULL;
    stream->stream_id.pointer = NULL;
    stream->size              = 0;
    stream->base              = NULL;
    stream->cursor            = NULL;

    return FT_Err_Ok;
  }


/***************************************************************************/
/*                                                                         */
/* <Function> MMapFile_Seek                                                */
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
/*   This function should never be called for memory-based resources..     */
/*                                                                         */
/***************************************************************************/

  static
  FT_Error  MMapFile_Seek( FT_Stream  stream,
                           FT_Long    position )
  {
    (void)stream;
    (void)position;
    return FT_Err_Invalid_Stream_Operation;
  }


/***************************************************************************/
/*                                                                         */
/* <Function> MMapFile_Skip                                                */
/*                                                                         */
/* <Description> Skip a given number of bytes in an MMap stream.           */
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
/*   This function should never be called for memory-based resources..     */
/*                                                                         */
/***************************************************************************/

  static
  FT_Error  MMapFile_Skip( FT_Stream  stream,
                           FT_Long    count )
  {
    (void)stream;
    (void)count;
    return FT_Err_Invalid_Stream_Operation;
  }                        

/***************************************************************************/
/*                                                                         */
/* <Function> MMapFile_Pos                                                 */
/*                                                                         */
/* <Description> Returns the current offset within an MMap stream's        */
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
/*   This function should never be called for memory-based resources..     */
/*                                                                         */
/***************************************************************************/

  static
  FT_Error  MMapFile_Pos( FT_Stream  stream,
                          FT_Long*   position )
  {
    (void)stream;
    (void)position;
    return FT_Err_Invalid_Stream_Operation;
  }

/***************************************************************************/
/*                                                                         */
/* <Function> MMapFile_Read                                                */
/*                                                                         */
/* <Description> Read a given number of bytes from an MMap stream into     */
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
/*   This function should never be called for memory-based resources..     */
/*                                                                         */
/***************************************************************************/

  static
  FT_Error  MMapFile_Read( FT_Stream  stream,
                           FT_Byte*   buffer,
                           FT_Long    size,
                           FT_Long*   read_bytes )
  {
    (void)stream;
    (void)buffer;
    (void)size;
    (void)read_bytes;
    return FT_Err_Invalid_Stream_Operation;                      
  }

/* The following table is the "virtual method table" for the 'MMap  */
/* resource class', which methods are defined above. Its address is */
/* set in the 'interface' field of all resource objects created by  */
/* the function FT_Create_MMapFile (see below)                      */

  static
  FTRes_InterfaceRec  FT_MMapFile_Interface =
  {
    (FTRes_Open_Func)  MMapFile_Open,
    (FTRes_Close_Func) MMapFile_Close,
    (FTRes_Seek_Func)  MMapFile_Seek,
    (FTRes_Skip_Func)  MMapFile_Skip,
    (FTRes_Pos_Func)   MMapFile_Pos,
    (FTRes_Read_Func)  MMapFile_Read,
  };


 /************************************************************************/
 /*                                                                      */
 /* <Function>  FT_Create_Resource                                       */
 /*                                                                      */
 /* <Description>                                                        */
 /*    Create a new resource object for a given library. Note that this  */
 /*    function takes an ASCII 'pathname' as an argument.                */
 /*                                                                      */
 /* <Input>                                                              */
 /*     library  :: handle to target library object                      */
 /*     pathanme :: ASCII pathname of the font file                      */
 /*                                                                      */
 /* <Output>                                                             */
 /*     resource :: handle to new resource object                        */
 /*                                                                      */
 /* <Return>                                                             */
 /*     Error code. 0 means success                                      */
 /*                                                                      */
 /* <Note>                                                               */
 /*     When porting the library to exotic environments, where an        */
 /*     ASCII pathname isn't used to name files, developers should       */
 /*     invoke directly their own resource creation function which       */
 /*     must be placed in their port of the "ftsys" component.           */
 /*                                                                      */
 /*     See the porting guide for more information..                     */
 /*                                                                      */
  EXPORT_FUNC
  FT_Error  FT_Create_Resource( FT_Library    library,
                                const char*   pathname,
                                FT_Resource*  aresource )
  {
    FT_Int       len;
    FT_System    system;
    FT_MMapFile  resource;
    FT_Error     error;

    *aresource = NULL;
    
    if (!library)
    {
      PERROR(( "Unix.New_Resource : null library handle\n" ));
      return FT_Err_Invalid_Library_Handle;
    }

    system = library->system;

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
    resource->root.interface = &FT_MMapFile_Interface;
    resource->root.flags     = FT_RESOURCE_TYPE_MEMORY_BASED;
    resource->file_size      = -1;
    strcpy( (char*)resource->pathname, pathname );

    PTRACE1(( "Create_MMapFile : MMap resource created for '%s'\n",
              pathname ));
    
    *aresource = (FT_Resource)resource;
    return FT_Err_Ok;

  Fail_Null:    
    PERROR(( "Create_MMapFile : null pathname !!\n" ));
    return FT_Err_Invalid_Argument;
    
  Fail_Memory:
    if (resource)
      FREE( resource->pathname );
    FREE( resource );
    PERROR(( "Create_MMapFile : error when creating resource !\n" ));
    return error;
  }

  
/***************************************************************************/
/*                                                                         */
/* <Function> FT_Destroy_Resource                                          */
/*                                                                         */
/* <Description> Destroys an MMap resource object.                         */
/*               This function is never called directly by the font        */
/*               drivers. Only by the higher-level part of FreeType        */
/*               (called the HLib), or client applications                 */
/*                                                                         */
/* <Input>                                                                 */
/*   resource :: the MMap resource object                                  */
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
    FT_MMapFile  res    = (FT_MMapFile)resource;
    
    if ( !res || res->root.interface != &FT_MMapFile_Interface )
    {
      PERROR(( 
        "Destroy_MMapFile : Trying to destroy an invalid resource !!\n" ));
      return FT_Err_Invalid_Resource_Handle;
    }
    
    PTRACE1(( "Destroy_MMapFile : destroying resource for '%s'\n",
              res->pathname ));

    FREE( res->pathname );
    FREE( res );

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
    *system = (FT_System)malloc( sizeof(FT_SystemRec) );
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

