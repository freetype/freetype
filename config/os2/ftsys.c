/***************************************************************************/
/*                                                                         */
/*  ftsys.c                                                                */
/*                                                                         */
/*    OS/2-specific system operations (body).                              */
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


  /*************************************************************************/
  /*                                                                       */
  /* This implementation of the `ftsys' component uses malloc()/free() for */
  /* memory management, and the OS/2 DosXXXXX() API functionss for file    */
  /* access.                                                               */
  /*                                                                       */
  /* IMPORTANT NOTE:                                                       */
  /*                                                                       */
  /*    Porters, read carefully the comments in ftsys.h before trying to   */
  /*    port this file to your system.  It contains many essential         */
  /*    remarks, and will ease your work greatly.                          */
  /*                                                                       */
  /*************************************************************************/


#include "ftsys.h"
#include "ftstream.h"
#include "ftdebug.h"

#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif


  /*************************************************************************/
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the PTRACE() and PERROR() macros, used to print/log      */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_io


#undef  CUR_SYSTEM  /* just in case */


  /*************************************************************************/
  /*                                                                       */
  /* To ease porting, we use the macro SYS_STREAM to name the              */
  /* system-specific stream type.  For example, it is a `FILE*' with the   */
  /* ANSI libc, it will be a file descriptor, i.e. an integer, when using  */
  /* the Unix system API, etc.                                             */
  /*                                                                       */
#define SYS_STREAM  HFILE


  /*************************************************************************/
  /*                                                                       */
  /*                      I/O ACCESS AND MANAGEMENT                        */
  /*                                                                       */
  /* We only define the `ANSI' resource class in this class.  It is        */
  /* disk-based and provides a MRU cache in order to only keep the file    */
  /* descriptors of the 10 most recently used resource files.              */
  /*                                                                       */
  /* It simply contains two lists.  One contains the `cached' resources    */
  /* with a valid FILE* pointer, the second contains all other `flushed'   */
  /* resource objects.                                                     */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Os2FileRec                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The FT_Os2File class derives from FT_ResourceRec.                  */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    root      :: The root resource class fields.                       */
  /*                                                                       */
  /*    pathname  :: This is a copy of the ANSI file pathname used to open */
  /*                 streams for the resource.  A different implementation */
  /*                 is free to use Unicode chars, or file i-node numbers, */
  /*                 etc.                                                  */
  /*                                                                       */
  /*    file_size :: The size in bytes of the resource.  This field should */
  /*                 be set to -1 until the resource is first opened.      */
  /*                                                                       */
  typedef struct  FT_Os2FileRec_
  {
    FT_ResourceRec  root;
    char*           pathname;   /* the font file's pathname           */
    FT_Long         file_size;  /* file size in bytes                 */

  } FT_Os2FileRec, *FT_Os2File;


  /*************************************************************************/
  /*                                                                       */
  /* We use the macro STREAM_Name() as a convenience to return a given     */
  /* ANSI resource's pathname.  Its `stream' argument is a FT_Resource     */
  /* which is typecasted to the FT_Os2File class.                          */
  /*                                                                       */
#define STREAM_Name( stream )  ((FT_Os2File)stream->resource)->pathname


  /*************************************************************************/
  /*                                                                       */
  /* We use the macro STREAM_File() as a convenience to extract the        */
  /* system-specific stream handle from a given FreeType stream object.    */
  /*                                                                       */
#define STREAM_File(stream)  ((HFILE)stream->stream_id.pointer)


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Os2File_Open                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This function is used to open a system-stream for a given          */
  /*    resource.                                                          */
  /*                                                                       */
  /*    Note that it must update the target FreeType stream object with    */
  /*    the system-stream handle and the resource's size.                  */
  /*                                                                       */
  /*    Also, the `stream->base' field must be set to NULL for disk-based  */
  /*    resources, and to the address in memory of the resource's first    */
  /*    byte for memory-based ones.                                        */
  /*                                                                       */
  /* <Input>                                                               */
  /*    resource :: The source resource.                                   */
  /*    stream   :: The target stream object.                              */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The stream object IS NOT CREATED by this function, but by its      */
  /*    caller.                                                            */
  /*                                                                       */
  static
  FT_Error  Os2File_Open( FT_Os2File  resource,
                          FT_Stream   stream )
  {
    HFILE  file;
    ULONG  ulAction;


    /* open the file */
#ifdef __EMX__
    if ( DosOpen( (FT_Byte*)resource->pathname,
                  &file,
                  &ulAction, 0, 0, OPEN_ACTION_OPEN_IF_EXISTS,
                  OPEN_SHARE_DENYNONE | OPEN_ACCESS_READONLY,
                  NULL ) )
#else
    if ( DosOpen( resource->pathname,
                  &file,
                  &ulAction, 0, 0, OPEN_ACTION_OPEN_IF_EXISTS,
                  OPEN_SHARE_DENYNONE | OPEN_ACCESS_READONLY,
                  NULL ) )
#endif  /* __EMX__ */
    {
      PERROR(( "Os2File_Open: Could not open file `%s'\n",
               resource->pathname ));
      return FT_Err_Cannot_Open_Stream;
    }

    /* compute file size if necessary */
    if ( resource->file_size < 0 )
    {
      DosSetFilePtr( file, 0, FILE_END, (ULONG*)&resource->file_size );
      DosSetFilePtr( file, 0, FILE_BEGIN, &ulAction );
    }

    stream->resource          = (FT_Resource)resource;
    stream->stream_id.pointer = (void*)file;
    stream->size              = resource->file_size;

    /* it's a disk-based resource, we don't need to use the "base" and   */
    /* "cursor" fields of the stream objects                             */
    stream->base              = NULL;
    stream->cursor            = NULL;

    PTRACE1(( "Os2File_Open: Opened `%s' (%d bytes) successfully\n",
              resource->pathname, resource->file_size ));

    return FT_Err_Ok;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Os2File_Close                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Closes a given stream.                                             */
  /*                                                                       */
  /* <Input>                                                               */
  /*    stream :: The target stream object.                                */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  static
  FT_Error  Os2File_Close( FT_Stream  stream )
  {
    PTRACE1(( "OS2File_Close: Closing file `%s'\n", STREAM_Name( stream ) ));

    DosClose( STREAM_File( stream ) );

    stream->resource          = NULL;
    stream->stream_id.pointer = NULL;
    stream->size              = 0;

    return FT_Err_Ok;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Os2File_Seek                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Seeks a stream to a given position.                                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    stream   :: The target stream object.                              */
  /*    position :: The offset in bytes from the start of the              */
  /*                resource/stream.                                       */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The `seek' method is never called by the stream manager in case    */
  /*    of a memory-based resource (i.e., when `stream->base' isn't NULL). */
  /*                                                                       */
  static
  FT_Error  Os2File_Seek( FT_Stream  stream,
                          FT_Long    position )
  {
    ULONG  ibActual;


    if ( DosSetFilePtr( STREAM_File( stream ), position,
                        FILE_BEGIN, &ibActual ) )
    {
      PERROR(( "Os2File_Seek: FAILED! Pos. %ld of `%s'\n",
               position, STREAM_Name( stream ) ));

      return FT_Err_Invalid_Stream_Seek;
    }

    PTRACE2(( "Os2File_Seek: Pos. %ld of `%s'\n",
              position, STREAM_Name( stream ) ));

    return FT_Err_Ok;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Os2File_Skip                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Skips a given number of bytes in an OS/2 stream.  Useful to skip   */
  /*    pad bytes, for example.                                            */
  /*                                                                       */
  /* <Input>                                                               */
  /*    stream :: The target stream object.                                */
  /*    count  :: The number of bytes to skip in the stream.               */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The `skip' method is never called by the stream manager in case    */
  /*    of a memory-based resource (i.e., when `stream->base' isn't NULL). */
  /*                                                                       */
  static
  FT_Error  Os2File_Skip( FT_Stream  stream,
                          FT_Long    count )
  {
    ULONG  ibActual;


    DosSetFilePtr( STREAM_File( stream ), 0, FILE_CURRENT, &ibActual );
    return Os2File_Seek( stream, ibActual + count );
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Os2File_Pos                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Returns the current offset within an OS/2 stream's resource.       */
  /*                                                                       */
  /* <Input>                                                               */
  /*    stream   :: The target stream object.                              */
  /*                                                                       */
  /* <Output>                                                              */
  /*    position :: The current offset.  -1 in case of error.              */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The `pos' method is never called by the stream manager in case     */
  /*    of a memory-based resource (i.e., when `stream->base' isn't NULL). */
  /*                                                                       */
  static
  FT_Error  Os2File_Pos( FT_Stream  stream,
                         FT_Long*   position )
  {
    ULONG  ibActual;


    if ( DosSetFilePtr( STREAM_File( stream ), 0, FILE_CURRENT, &ibActual ) )
    {
      PTRACE2(( "Os2File_Pos: FAILED! in `%s'\n", STREAM_Name( stream ) ));
      return FT_Err_Invalid_Stream_Seek;
    }

    *position = ibActual;

    PTRACE2(( "Os2File_Pos: For `%s'\n", STREAM_Name( stream ) ));
    return FT_Err_Ok;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Os2File_Read                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Reads a given number of bytes from an OS/2 stream into memory.     */
  /*                                                                       */
  /* <Input>                                                               */
  /*    stream     :: The target stream object.                            */
  /*    buffer     :: The target read buffer where data is copied.         */
  /*    size       :: The number of bytes to read from the stream.         */
  /*                                                                       */
  /* <Output>                                                              */
  /*    read_bytes :: The number of bytes effectively read from the        */
  /*                  stream.  Used in case of error                       */
  /*                  (i.e. FT_Err_Invalid_Stream_Read) by some parts of   */
  /*                  the library.                                         */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*   It MUST return the error FT_Err_Invalid_Stream_Read in case of      */
  /*   an over-read (i.e., reading more bytes from the stream that what    */
  /*   is left), as the stream component checks for this specific value.   */
  /*                                                                       */
  /*   The `read' method is never called by the stream manager in case     */
  /*   of a memory-based resource (i.e., when `stream->base' isn't NULL).  */
  /*                                                                       */
  static
  FT_Error  Os2File_Read( FT_Stream  stream,
                          char*      buffer,
                          FT_Long    size,
                          FT_Long*   read_bytes )
  {
    ULONG  cbActual;


    DosRead( STREAM_File( stream ), buffer, size, &cbActual );

    *read_bytes = cbActual;

    if ( cbActual != (ULONG)size )
    {
      /* Note : we can have an over-read here when called by the */
      /*        function FT_Access_Compressed_Frame. This means  */
      /*        that the following message should be a trace,    */
      /*        rather than an error for disk-based resources..  */
      /*                                                         */
      /*        the function must set the value of 'read_bytes'  */
      /*        even if it returns an error code..               */
      PTRACE2(( "Os2File_Read: FAILED!  Read %ld bytes from '%s'\n",
               size, STREAM_Name( stream ) ));

      return FT_Err_Invalid_Stream_Read;
    }

    PTRACE2(( "Os2File_Read: Read %ld bytes to buffer 0x%08lx from `%s'\n",
              size, (long)buffer, STREAM_Name( stream ) ));
    return FT_Err_Ok;
  }


  /*************************************************************************/
  /*                                                                       */
  /* The following table is the `virtual method table' for the `OS/2       */
  /* resource class', which methods are defined above.  Its address is set */
  /* in the `interface' field of all resource objects created by the       */
  /* function FT_Create_Os2File() (see below).                             */
  /*                                                                       */
  static
  FTRes_InterfaceRec  FT_Os2File_Interface =
  {
    (FTRes_Open_Func)        Os2File_Open,
    (FTRes_Close_Func)       Os2File_Close,
    (FTRes_Seek_Func)        Os2File_Seek,
    (FTRes_Skip_Func)        Os2File_Skip,
    (FTRes_Pos_Func)         Os2File_Pos,
    (FTRes_Read_Func)        Os2File_Read,
  };


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Create_Resource                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Creates a new resource object.  This function is called by the     */
  /*    FT_New_Resource() function of the base layer.                      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library   :: The input library object.                             */
  /*    pathname  :: The file's pathname as an ASCII string.               */
  /*                                                                       */
  /* <Output>                                                              */
  /*    aresource :: A handle to new resource object.                      */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This functions does not open a stream.  It simply copies the       */
  /*    pathname within a fresh new resource object.                       */
  /*                                                                       */
  EXPORT_FUNC
  FT_Error  FT_Create_Resource( FT_Library    library,
                                const char*   pathname,
                                FT_Resource*  aresource )
  {
    FT_Int      len;
    FT_Os2File  resource;
    FT_Error    error;
    FT_System   system;


    if ( !library )
      return FT_Err_Invalid_Library_Handle;

    system = library->system;

    if ( !pathname )
      goto Fail_Null;

    len = strlen( pathname );
    if ( len == 0 )
      goto Fail_Null;

    resource = NULL;

    if ( ALLOC( resource, sizeof ( *resource ) ) ||
         ALLOC( resource->pathname, len + 1 )    )
      goto Fail_Memory;

    resource->root.library   = library;
    resource->root.interface = &FT_Os2File_Interface;
    resource->root.flags     = FT_RESOURCE_TYPE_DISK_BASED;
    resource->file_size      = -1;
    strcpy( resource->pathname, pathname );

    PTRACE1(( "Create_Os2File: OS/2 resource created for '%s'\n",
              pathname ));

    *aresource = (FT_Resource)resource;
    return FT_Err_Ok;

  Fail_Null:
    PERROR(( "Create_Os2File: Null pathname!\n" ));
    return FT_Err_Invalid_Argument;

  Fail_Memory:
    if ( resource )
      FREE( resource->pathname );
    FREE( resource );
    PERROR(( "Create_Os2File: Not enough memory to create resource!\n" ));
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Destroy_Resource                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Discards a given resource object explicitly.                       */
  /*                                                                       */
  /* <Input>                                                               */
  /*    resource :: The OS/2 resource object.                              */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This function does not check whether runs or streams are opened    */
  /*    for the resource (for now, we assume developer intelligence.       */
  /*    We'll most probably lower our standard later to ease debugging :-) */
  /*                                                                       */
  EXPORT_FUNC
  FT_Error  FT_Destroy_Resource( FT_Resource  resource )
  {
    FT_System   system = resource->library->system;
    FT_Os2File  ansi   = (FT_Os2File)resource;

    if ( !ansi || ansi->root.interface != &FT_Os2File_Interface )
    {
      PERROR((
        "Destroy_Os2File: Trying to destroy an invalid resource!\n" ));
      return FT_Err_Invalid_Resource_Handle;
    }

    PTRACE1(( "Destroy_Os2File: Destroying resource for `%s'\n",
              ansi->pathname ));

    FREE( ansi->pathname );
    FREE( ansi );

    return FT_Err_Ok;
  }


  /*************************************************************************/
  /*                                                                       */
  /*                          MEMORY MANAGEMENT                            */
  /*                                                                       */
  /*                                                                       */
  /*  This part copies the old FreeType 1.0 and 1.1 memory management      */
  /*  scheme that was defined in the file `ttmemory.h'.  One can see that  */
  /*                                                                       */
  /*  - a set of macros is defined for the memory operations used by the   */
  /*    engine (MEM_Copy(), MEM_Move(), MEM_Set()).  This comes from the   */
  /*    fact that many compilers are able to inline these operations       */
  /*    directly within the compiled code, rather than generating a call   */
  /*    to the C library.  However, this obliges us to include the         */
  /*    `<string.h>' header file.                                          */
  /*                                                                       */
  /*    If you provide your own memory operations, you can get rid of the  */
  /*    `#include <string.h>' below.                                       */
  /*                                                                       */
  /*                                                                       */
  /*  - the FT_Alloc() function has several essential properties that MUST */
  /*    be retained by each port:                                          */
  /*                                                                       */
  /*    - It returns an error code, NOT the allocated block's base         */
  /*      address.                                                         */
  /*                                                                       */
  /*    - It takes the address of a target pointer, where the block's base */
  /*      address will be set.  If the size is zero, its value will be     */
  /*      NULL, and the function returns successfully.                     */
  /*                                                                       */
  /*    - In case of error, the pointer's value is set to NULL and an      */
  /*      error code is returned.                                          */
  /*                                                                       */
  /*    - The new allocated block MUST be zero-filled.  This is a strong   */
  /*      convention the rest of the engine relies on.                     */
  /*                                                                       */
  /*                                                                       */
  /*  - the FT_Free() function has also its essentials:                    */
  /*                                                                       */
  /*    - It takes the address of a pointer which value is the block's     */
  /*      base address.  This is UNLIKE a standard `free()' which takes    */
  /*      the block's base directly.                                       */
  /*                                                                       */
  /*    - It accepts successfully the address of a pointer which value is  */
  /*      NULL, in which case it simply returns.                           */
  /*                                                                       */
  /*    - The pointer is always set to NULL by the function.               */
  /*                                                                       */
  /*                                                                       */
  /*  - The MEM_Alloc(), ALLOC(), and ALLOC_ARRAY() macros are used by the */
  /*    library and should NOT be modified by porters!                     */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the PTRACE() and PERROR() macros, used to print/log      */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_memory


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Alloc                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Allocates a new block of memory.  The returned area is always      */
  /*    zero-filled, this is a strong convention in many FreeType parts.   */
  /*                                                                       */
  /* <Input>                                                               */
  /*    system :: A handle to a given `system object' where allocation     */
  /*              occurs.                                                  */
  /*                                                                       */
  /*    size   :: The size in bytes of the block to allocate.              */
  /*                                                                       */
  /* <Output>                                                              */
  /*    P      :: A pointer to the fresh new block.  It should be set to   */
  /*              NULL if `size' is 0, or in case of error.                */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  BASE_FUNC
  FT_Error  FT_Alloc( FT_System  system,
                      FT_Long    size,
                      void**     P )
  {
    if ( !P )
    {
      PERROR(( "FT_Alloc: Invalid pointer address!\n" ));
      return FT_Err_Invalid_Argument;
    }

    if ( size > 0 )
    {
      *P = malloc( size );
      if ( !*P )
      {
        PERROR(( "FT_Alloc: Out of memory (%ld bytes requested)!\n",
                 size ));

        return FT_Err_Out_Of_Memory;
      }

      system->total_alloc += size;

      /* ALWAYS ZERO-FILL THE BLOCK! */
      MEM_Set( *P, 0, size );
    }
    else
      *P = NULL;

    PTRACE2(( "FT_Alloc: Size = %ld, pointer = 0x%08lx, block = 0x%08lx\n",
              size, (long)P, (long)*P ));

    return FT_Err_Ok;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Realloc                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Reallocates a block of memory pointed to by `*P' to `Size' bytes   */
  /*    from the heap, possibly changing `*P'.                             */
  /*                                                                       */
  /* <Input>                                                               */
  /*    system :: A handle to a given `system object' where allocation     */
  /*              occurs.                                                  */
  /*                                                                       */
  /*    size   :: The size in bytes of the block to allocate.              */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    P      :: A pointer to the fresh new block.  It should be set to   */
  /*              NULL if `size' is 0, or in case of error.                */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  BASE_FUNC
  FT_Error  FT_Realloc( FT_System  system,
                        FT_Long    size,
                        void**     P )
  {
    void*  Q;


    if ( !P )
    {
      PERROR(( "FT_Realloc: Invalid pointer address!\n" ));
      return FT_Err_Invalid_Argument;
    }

    /* if the original pointer is NULL, call FT_Alloc() */
    if ( !*P )
      return FT_Alloc( system, size, P );

    /* if the new block if zero-sized, clear the current one */
    if ( size <= 0 )
      return FT_Free( system, P );

    Q = (void*)realloc( *P, size );
    if ( !Q )
    {
      PERROR(( "FT_Realloc: Reallocation failed!\n" ));
      return FT_Err_Out_Of_Memory;
    }

    *P = Q;
    return FT_Err_Ok;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Free                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Releases a given block of memory allocated through FT_Alloc().     */
  /*                                                                       */
  /* <Input>                                                               */
  /*    system :: A handle to a given `system object' where allocation     */
  /*              occured.                                                 */
  /*                                                                       */
  /*    P      :: This is the _address_ of a _pointer_ which points to the */
  /*              allocated block.  It is always set to NULL on exit.      */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    If P or *P are NULL, this function should return successfully.     */
  /*    This is a strong convention within all of FreeType and its         */
  /*    drivers.                                                           */
  /*                                                                       */
  BASE_FUNC
  FT_Error  FT_Free( FT_System  system,
                     void*     *P )
  {
    UNUSED( system );

    PTRACE2(( "FT_Free: Freeing pointer 0x%08lx (block 0x%08lx)\n",
              (long)P, (P ? (long)*P : -1) ));

    if ( !P || !*P )
      return FT_Err_Ok;

    free( *P );
    *P = NULL;

    return FT_Err_Ok;
  }


  /*************************************************************************/
  /*                                                                       */
  /*                       SYNCHRONIZATION MANAGEMENT                      */
  /*                                                                       */
  /*                                                                       */
  /*   This section deals with mutexes.  The library can be compiled to    */
  /*   two distinct thread support levels (namely single threaded and      */
  /*   re-entrant modes).                                                  */
  /*                                                                       */
  /*   It protects its variables through the MUTEX_Lock() and              */
  /*   MUTEX_Release() macros which are void in single threaded mode.      */
  /*                                                                       */
  /*   It defines a typeless mutex reference type, `FT_Mutex', that you're */
  /*   free to redefine for your system's needs.                           */
  /*                                                                       */
  /*   The default implementation of ftsys.c contains only dummy functions */
  /*   which always return successfully.  You NEED to specialize them in   */
  /*   order to port ftsys.c to any multi-threaded environment.            */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the PTRACE() and PERROR() macros, used to print/log      */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_sync


#ifdef FT_CONFIG_THREADS

  BASE_FUNC
  FT_Error  FT_Mutex_Create( FT_System  system,
                             TMutex*    mutex )
  {
    UNUSED( system );

    mutex = (void*)-1;
    system->num_mutexes++;

    /* Insert your own mutex creation code here */

    return FT_Err_Ok;
  }


  BASE_FUNC
  void  FT_Mutex_Delete( FT_System  system,
                         TMutex*    mutex )
  {
    UNUSED( system );

    mutex = (void*)0;
    system->num_mutexes--;

    /* Insert your own mutex destruction code here */
  }


  BASE_FUNC
  void  FT_Mutex_Lock( FT_System  system,
                       TMutex*    mutex )
  {
    /* NOTE: It is legal to call this function with a NULL argument */
    /*       in which case an immediate return is appropriate.      */

    UNUSED( system );

    if ( !mutex )
      return;

    /* Insert your own mutex locking code here */
  }


  BASE_FUNC
  void  FT_Mutex_Release( FT_System  system,
                          TMutex*    mutex )
  {
    /* NOTE: It is legal to call this function with a NULL argument */
    /*       in which case an immediate return is appropriate       */

    UNUSED( system );

    if ( !mutex )
      return;

    /* Insert your own mutex release code here */
  }

#endif /* FT_CONFIG_THREADS */


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_New_System                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This function is used to create and initialize new system objects. */
  /*    These are mainly used to let client applications and font servers  */
  /*    specify their own memory allocators and synchronization            */
  /*    procedures.                                                        */
  /*                                                                       */
  /* <Output>                                                              */
  /*    system :: A handle to a given `system object'.                     */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  EXPORT_FUNC
  FT_Error  FT_New_System( FT_System*  system )
  {
    *system = (FT_System)malloc( sizeof ( **system ) );

    if ( !*system )
      return FT_Err_Out_Of_Memory;

    /* initialize memory management */

    (*system)->system_flags = FT_SYSTEM_FLAG_TOTAL_ALLOC |
                              FT_SYSTEM_FLAG_MUTEXES;
    (*system)->total_alloc = 0;
    (*system)->num_mutexes = 0;

    /* initialize i/o management (nothing) */

    /* initialize synchronisation (nothing) */

    /* initialize streams (nothing) */

    return FT_Err_Ok;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Done_System                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Destroys a given FreeType system object.                           */
  /*                                                                       */
  /* <Input>                                                               */
  /*    system :: A handle to a given `system object'.                     */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  EXPORT_FUNC
  FT_Error  FT_Done_System( FT_System  system )
  {
    /* finalize synchronization (nothing) */

    /* finalize i/o management (nothing)  */

    /* finalize memory management         */

    free( system );

    return FT_Err_Ok;
  }


/* END */
