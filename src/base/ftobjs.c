/***************************************************************************/
/*                                                                         */
/*  ftobjs.c                                                               */
/*                                                                         */
/*  The FreeType private base classes (base).                              */
/*                                                                         */
/*  Copyright 1996-2000 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg                       */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used        */
/*  modified and distributed under the terms of the FreeType project       */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

#include <ftobjs.h>
#include <ftlist.h>
#include <ftdebug.h>
#include <ftstream.h>



  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                           M E M O R Y                           ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
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
  /*    zero-filled; this is a strong convention in many FreeType parts.   */
  /*                                                                       */
  /* <Input>                                                               */
  /*    memory :: A handle to a given `memory object' where allocation     */
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
  FT_Error  FT_Alloc( FT_Memory  memory,
                      FT_Long    size,
                      void**     P )
  {
    FT_Assert( P != 0 );

    if ( size > 0 )
    {
      *P = memory->alloc( memory, size );
      if ( !*P )
      {
        FT_ERROR(( "FT.Alloc:" ));
        FT_ERROR(( " Out of memory? (%ld requested)\n",
                   size ));

        return FT_Err_Out_Of_Memory;
      }
      MEM_Set( *P, 0, size );
    }
    else
      *P = NULL;

    FT_TRACE2(( "FT_Alloc:" ));
    FT_TRACE2(( " size = %ld, block = 0x%08lx, ref = 0x%08lx\n",
                size, (long)*P, (long)P ));

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
  /*    memory  :: A handle to a given `memory object' where allocation    */
  /*               occurs.                                                 */
  /*                                                                       */
  /*    current :: The current block size in bytes.                        */
  /*    size    :: The new block size in bytes.                            */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    P       :: A pointer to the fresh new block.  It should be set to  */
  /*               NULL if `size' is 0, or in case of error.               */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    All callers of FT_Realloc() _must_ provide the current block size  */
  /*    as well as the new one.                                            */
  /*                                                                       */
  /*    If the memory object's flag FT_SYSTEM_FLAG_NO_REALLOC is set, this */
  /*    function will try to emulate a reallocation using FT_Alloc() and   */
  /*    FT_Free().  Otherwise, it will call the system-specific `realloc'  */
  /*    implementation.                                                    */
  /*                                                                       */
  /*    (Some embedded systems do not have a working realloc function).    */
  /*                                                                       */
  BASE_FUNC
  FT_Error  FT_Realloc( FT_Memory  memory,
                        FT_Long    current,
                        FT_Long    size,
                        void**     P )
  {
    void*  Q;


    FT_Assert( P != 0 );

    /* if the original pointer is NULL, call FT_Alloc() */
    if ( !*P )
      return FT_Alloc( memory, size, P );

    /* if the new block if zero-sized, clear the current one */
    if ( size <= 0 )
    {
      FT_Free( memory, P );
      return FT_Err_Ok;
    }

    Q = memory->realloc( memory, current, size, *P );
    if ( !Q )
      goto Fail;

    *P = Q;
    return FT_Err_Ok;

  Fail:
    FT_ERROR(( "FT.Realloc:" ));
    FT_ERROR(( " Failed (current %ld, requested %ld)\n",
               current, size ));
    return FT_Err_Out_Of_Memory;
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
  /*    memory :: A handle to a given `memory object' where allocation     */
  /*              occurred.                                                */
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
  void  FT_Free( FT_Memory  memory,
                 void**     P )
  {
    FT_TRACE2(( "FT_Free:" ));
    FT_TRACE2(( " Freeing block 0x%08lx, ref 0x%08lx\n",
                (long)P, (P ? (long)*P : -1) ));

    FT_Assert( P != 0 );

    if ( *P )
    {
      memory->free( memory, *P );
      *P = 0;
    }
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    ft_new_input_stream                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Creates a new input stream object from an FT_Open_Args structure.  */
  /*                                                                       */
  static
  FT_Error  ft_new_input_stream( FT_Library     library,
                                 FT_Open_Args*  args,
                                 FT_Stream*     astream )
  {
    FT_Error   error;
    FT_Memory  memory;
    FT_Stream  stream;

    
    memory = library->memory;
    if ( ALLOC( stream, sizeof ( *stream ) ) )
      return error;
      
    stream->memory = memory;
    
    /* is it a memory stream? */
    if ( args->memory_base && args->memory_size )
      FT_New_Memory_Stream( library,
                            args->memory_base,
                            args->memory_size,
                            stream );

    /* do we have an 8-bit pathname? */
    else if ( args->pathname )
    {
      error = FT_New_Stream( args->pathname, stream );
      stream->pathname.pointer = args->pathname;
    }

    /* do we have a custom stream? */
    else if ( args->stream )
    {
      /* copy the contents of the argument stream */
      /* into the new stream object               */
      *stream = *(args->stream);
      stream->memory = memory;
    }
    else
      error = FT_Err_Invalid_Argument;
      
    if ( error )
      FREE( stream );
      
    *astream = stream;
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    ft_done_stream                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Closes and destroys a stream object.                               */
  /*                                                                       */
  static
  void  ft_done_stream( FT_Stream*  astream )
  {
    FT_Stream  stream = *astream;
    FT_Memory  memory = stream->memory;


    if ( stream->close )
      stream->close( stream );

    FREE( stream );
    *astream = 0;
  }



  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****               O B J E C T   M A N A G E M E N T                 ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

  /* destructor for sizes list */
  static
  void  destroy_size( FT_Memory  memory,
                      FT_Size    size,
                      FT_Driver  driver )
  {
    /* finalize format-specific stuff */
    driver->interface.done_size( size );
    FREE( size );
  }


  /* destructor for faces list */
  static
  void  destroy_face( FT_Memory  memory,
                      FT_Face    face,
                      FT_Driver  driver )
  {
    /* Discard glyph slots for this face                                */
    /* XXX: Beware!  FT_Done_GlyphSlot() changes the field `face->slot' */
    while ( face->glyph )
      FT_Done_GlyphSlot( face->glyph );

    /* Discard all sizes for this face */
    FT_List_Finalize( &face->sizes_list,
                     (FT_List_Destructor)destroy_size,
                     memory,
                     driver );
    face->size = 0;

    /* finalize format-specific stuff */
    driver->interface.done_face( face );

    /* Now discard client data */
    if ( face->generic.finalizer )
      face->generic.finalizer( face );

    /* close the stream for this face */
    ft_done_stream( &face->stream );    

    /* get rid of it */
    FREE( face );
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Destroy_Driver                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Destroys a given driver object.  This also destroys all child      */
  /*    faces.                                                             */
  /*                                                                       */
  /* <InOut>                                                               */
  /*     driver :: A handle to the target driver object.                   */
  /*                                                                       */
  /* <Note>                                                                */
  /*     The driver _must_ be LOCKED!                                      */
  /*                                                                       */
  static
  void  Destroy_Driver( FT_Driver  driver )
  {
    FT_Memory  memory = driver->memory;


    /* now, finalize all faces in the driver list */
    FT_List_Finalize( &driver->faces_list,
                      (FT_List_Destructor)destroy_face,
                      memory,
                      driver );

    /* finalize the driver object */
    if ( driver->interface.done_driver )
      driver->interface.done_driver( driver );

    /* finalize client-data */
    if ( driver->generic.finalizer )
      driver->generic.finalizer( driver );

    /* discard it */
    FREE( driver );
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Get_Glyph_Format                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Gets the glyph format for a given format tag.                      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library    :: A handle to the library object.                      */
  /*    format_tag :: A tag identifying the glyph format.                  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    A pointer to a glyph format.  0 if `format_tag' isn't defined.     */
  /*                                                                       */
  BASE_FUNC
  FT_Glyph_Format*  FT_Get_Glyph_Format( FT_Library    library,
                                         FT_Glyph_Tag  format_tag )
  {
    FT_Glyph_Format*  cur   = library->glyph_formats;
    FT_Glyph_Format*  limit = cur + FT_MAX_GLYPH_FORMATS;


    for ( ; cur < limit; cur ++ )
    {
      if ( cur->format_tag == format_tag )
        return cur;
    }

    return 0;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Set_Raster                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This function changes the raster module used to convert from a     */
  /*    given memory object.  It is thus possible to use libraries with    */
  /*    distinct memory allocators within the same program.                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library   :: A handle to the library object.                       */
  /*    interface :: A pointer to the interface of the new raster module.  */
  /*                                                                       */
  /* <Output>                                                              */
  /*    raster    :: A handle to the raster object.                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  EXPORT_FUNC
  FT_Error  FT_Set_Raster( FT_Library            library,
                           FT_Raster_Interface*  interface,
                           FT_Raster             raster )
  {
    FT_Memory         memory = library->memory;
    FT_Error          error  = FT_Err_Ok;
    FT_Glyph_Format*  format;


    /* allocate the render pool if necessary */
    if ( !library->raster_pool &&
         ALLOC( library->raster_pool, FT_RENDER_POOL_SIZE ) )
      goto Exit;

    /* find the glyph formatter for the raster's format */
    format = FT_Get_Glyph_Format( library, interface->format_tag );
    if ( !format )
    {
      error = FT_Err_Invalid_Argument;
      goto Exit;
    }

    /* free previous raster object if necessary */
    if ( format->raster_allocated )
    {
      FREE( format->raster );
      format->raster_allocated = 0;
    }

    /* allocate new raster object is necessary */
    if ( !raster )
    {
      if ( ALLOC( raster, interface->size ) )
        goto Exit;

      format->raster_allocated = 1;
    }
    format->raster           = raster;
    format->raster_interface = interface;

    /* initialize the raster object */
    error = interface->init( raster,
                             (char*)library->raster_pool,
                             FT_RENDER_POOL_SIZE );
    if ( error )
    {
      if ( format->raster_allocated )
      {
        FREE( format->raster );
        format->raster_allocated = 0;
      }
    }

  Exit:
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Set_Debug_Hook                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Sets a debug hook function for debugging the interpreter of a      */
  /*    font format.                                                       */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library    :: A handle to the library object.                      */
  /*    hook_index :: The index of the debug hook.  You should use the     */
  /*                  values defined in ftobjs.h, e.g.                     */
  /*                  FT_DEBUG_HOOK_TRUETYPE                               */
  /*    debug_hook :: The function used to debug the interpreter.          */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Currently, four debug hook slots are available, but only two (for  */
  /*    the TrueType and the Type 1 interpreter) are defined.              */
  /*                                                                       */
  EXPORT_FUNC
  void  FT_Set_Debug_Hook( FT_Library         library,
                           FT_UInt            hook_index,
                           FT_DebugHook_Func  debug_hook )
  {
    if ( hook_index < ( sizeof ( library->debug_hooks ) / sizeof ( void* ) ) )
      library->debug_hooks[hook_index] = debug_hook;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Add_Glyph_Format                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Adds a glyph format to the library.                                */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    library :: A handle to the library object.                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    format  :: A pointer to the new glyph format.                      */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  BASE_FUNC
  FT_Error  FT_Add_Glyph_Format( FT_Library        library,
                                 FT_Glyph_Format*  format )
  {
    FT_Glyph_Format*  new = 0;


    {
      FT_Glyph_Format*  cur   = library->glyph_formats;
      FT_Glyph_Format*  limit = cur + FT_MAX_GLYPH_FORMATS;


      for ( ; cur < limit; cur++ )
      {
        /* return an error if the format is already registered */
        if ( cur->format_tag == format->format_tag )
          return FT_Err_Invalid_Glyph_Format;

        if ( cur->format_tag == 0 && new == 0 )
          new = cur;
      }
    }

    /* if there is no place to hold the new format, return an error */
    if ( !new )
      return FT_Err_Too_Many_Glyph_Formats;

    *new = *format;

    /* now, create a raster object if we need to */
    return FT_Set_Raster( library,
                          format->raster_interface,
                          format->raster );
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Remove_Glyph_Format                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Removes a glyph format from the library.                           */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    library    :: A handle to the library object.                      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    format_tag :: A tag identifying the format to be removed.          */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  BASE_FUNC
  FT_Error  FT_Remove_Glyph_Format( FT_Library    library,
                                    FT_Glyph_Tag  format_tag )
  {
    FT_Memory         memory;
    FT_Glyph_Format*  cur   = library->glyph_formats;
    FT_Glyph_Format*  limit = cur + FT_MAX_GLYPH_FORMATS;


    memory = library->memory;

    for ( ; cur < limit; cur++ )
    {
      if ( cur->format_tag == format_tag )
      {
        if ( cur->raster_allocated )
        {
          FREE( cur->raster );
          cur->raster_allocated = 0;
        }
        cur->format_tag = 0;
        return FT_Err_Ok;
      }
    }

    return FT_Err_Invalid_Argument;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_New_Library                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This function is used to create a new FreeType library instance    */
  /*    from a given memory object.  It is thus possible to use libraries  */
  /*    with distinct memory allocators within the same program.           */
  /*                                                                       */
  /* <Input>                                                               */
  /*    memory   :: A handle to the original memory object.                */
  /*                                                                       */
  /* <Output>                                                              */
  /*    alibrary :: A pointer to handle of a new library object.           */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  EXPORT_FUNC
  FT_Error  FT_New_Library( FT_Memory    memory,
                            FT_Library*  alibrary )
  {
    FT_Library library = 0;
    FT_Error   error;


    /* first of all, allocate the library object */
    if ( ALLOC( library, sizeof ( *library ) ) )
      return error;

    library->memory = memory;

    /* now register the default raster for the `outline' glyph image format */
    {
      FT_Glyph_Format  outline_format =
      {
        ft_glyph_format_outline,
        &ft_default_raster,
        0,
        0
      };


      error = FT_Add_Glyph_Format( library, &outline_format );
    }

    /* That's ok now */
    *alibrary = library;

    return FT_Err_Ok;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Done_Library                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Discards a given library object.  This closes all drivers and      */
  /*    discards all resource objects.                                     */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library :: A handle to the target library                          */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  EXPORT_FUNC
  FT_Error  FT_Done_Library( FT_Library  library )
  {
    FT_Memory  memory;
    FT_Int     n;


    if ( !library )
      return FT_Err_Invalid_Library_Handle;

    memory = library->memory;

    /* Discard client-data */
    if ( library->generic.finalizer )
      library->generic.finalizer( library );

    /* Close all drivers in the library */
    for ( n = 0; n < library->num_drivers; n++ )
    {
      FT_Driver  driver = library->drivers[n];


      if ( driver )
      {
        Destroy_Driver( driver );
        library->drivers[n] = 0;
      }
    }

    /* Destroy raster object */
    FREE( library->raster_pool );

    {
      FT_Glyph_Format*  cur   = library->glyph_formats;
      FT_Glyph_Format*  limit = cur + FT_MAX_GLYPH_FORMATS;


      for ( ; cur < limit; cur++ )
      {
        if ( cur->raster_allocated )
        {
          FREE( cur->raster );
          cur->raster_allocated = 0;
        }
      }
    }

    FREE( library );

    return FT_Err_Ok;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Set_Raster_Mode                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Sets a raster-specific mode.                                       */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    library :: A handle to a target library object.                    */
  /*                                                                       */
  /* <Input>                                                               */
  /*    format  :: The glyph format used to select the raster.             */
  /*    mode    :: The raster-specific mode descriptor.                    */
  /*    args    :: The mode arguments.                                     */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  EXPORT_FUNC
  FT_Error  FT_Set_Raster_Mode( FT_Library    library,
                                FT_Glyph_Tag  format_tag,
                                const char*   mode,
                                const char*   args )
  {
    FT_Memory         memory;
    FT_Error          error;
    FT_Glyph_Format*  format = 0;


    {
      FT_Glyph_Format*  cur   = library->glyph_formats;
      FT_Glyph_Format*  limit = cur + FT_MAX_GLYPH_FORMATS;


      for ( ; cur < limit; cur++ )
      {
        if ( cur->format_tag == format_tag )
        {
          format = cur;
          break;
        }
      }
    }

    if ( !format )
      return FT_Err_Invalid_Argument;

    memory = library->memory;

    error = FT_Err_Ok;
    if ( format->raster )
      error = format->raster_interface->set_mode( format->raster,
                                                  mode, args );

    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Add_Driver                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Registers a new driver in a given library object.  This function   */
  /*    takes only a pointer to a driver interface; it uses it to create   */
  /*    the new driver, then sets up some important fields.                */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    library          :: A handle to the target library object.         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    driver_interface :: A pointer to a driver interface table.         */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This function doesn't check whether the driver is already          */
  /*    installed!                                                         */
  /*                                                                       */
  EXPORT_FUNC
  FT_Error  FT_Add_Driver( FT_Library                 library,
                           const FT_DriverInterface*  driver_interface )
  {
    FT_Error   error;
    FT_Driver  driver;
    FT_Memory  memory;


    if ( !library || !driver_interface )
      return FT_Err_Invalid_Library_Handle;

    memory = library->memory;
    error  = FT_Err_Ok;

    if ( library->num_drivers >= FT_MAX_DRIVERS )
      error = FT_Err_Too_Many_Drivers;
    else
    {
      if ( ALLOC( driver, driver_interface->driver_object_size ) )
        goto Exit;

      driver->library   = library;
      driver->memory    = memory;
      driver->interface = *driver_interface;

      if ( driver_interface->init_driver )
      {
        error = driver_interface->init_driver( driver );
        if ( error )
          goto Fail;
      }

      library->drivers[library->num_drivers++] = driver;
      goto Exit;

    Fail:
      FREE( driver );
    }

  Exit:
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Remove_Driver                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Unregisters a given driver.  This closes the driver, which in turn */
  /*    destroys all faces, sizes, slots, etc. associated with it.         */
  /*                                                                       */
  /*    This function also DESTROYS the driver object.                     */
  /*                                                                       */
  /* <Input>                                                               */
  /*    driver :: A handle to target driver object.                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  EXPORT_FUNC
  FT_Error  FT_Remove_Driver( FT_Driver  driver )
  {
    FT_Library  library;
    FT_Memory   memory;
    FT_Driver   *cur, *last;
    FT_Error    error;


    if ( !driver )
      return FT_Err_Invalid_Driver_Handle;

    library = driver->library;
    memory  = driver->memory;

    if ( !library || !memory )
      return FT_Err_Invalid_Driver_Handle;

    /* look-up driver entry in library table */
    error = FT_Err_Invalid_Driver_Handle;
    cur   = library->drivers;
    last  = cur + library->num_drivers - 1;

    for ( ; cur <= last; cur++ )
    {
      if ( *cur == driver )
      {
        /* destroy the driver object */
        Destroy_Driver( driver );

        /* now move the last driver in the table to the vacant slot */
        if ( cur < last )
        {
          *cur  = *last;
          *last = 0;
        }
        library->num_drivers--;

        /* exit loop */
        error = FT_Err_Ok;
        break;
      }
    }

    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Get_Driver                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Returns the handle of the driver responsible for a given format    */
  /*    (or service) according to its `name'.                              */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library     :: A handle to the library object.                     */
  /*    driver_name :: The name of the driver to look up.                  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    A handle to the driver object, 0 otherwise.                        */
  /*                                                                       */
  EXPORT_FUNC
  FT_Driver  FT_Get_Driver( FT_Library  library,
                            char*       driver_name )
  {
    FT_Driver  *cur, *limit;


    if ( !library || !driver_name )
      return 0;

    cur   = library->drivers;
    limit = cur + library->num_drivers;
    for ( ; cur < limit; cur++ )
    {
      if ( strcmp( (*cur)->interface.driver_name, driver_name ) == 0 )
        return *cur;
    }
    return 0;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    open_face                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This function does some work for FT_Open_Face().                   */
  /*                                                                       */
  static
  FT_Error  open_face( FT_Driver  driver,
                       FT_Stream  stream,
                       FT_Long    face_index,
                       FT_Face*   aface )
  {
    FT_Memory            memory;
    FT_DriverInterface*  interface;
    FT_Face              face = 0;
    FT_Error             error;


    interface = &driver->interface;
    memory    = driver->memory;

    /* allocate the face object, and perform basic initialization */
    if ( ALLOC( face, interface->face_object_size ) )
      goto Fail;

    face->driver = driver;
    face->memory = memory;
    face->stream = stream;

    error = interface->init_face( stream, face_index, face );
    if ( error )
      goto Fail;

    *aface = face;

  Fail:
    if ( error )
    {
      interface->done_face( face );
      FREE( face );
      *aface = 0;
    }

    return error;
  }


  static
  const FT_Open_Args  ft_default_open_args = 
    { 0, 0, 0, 0, 0 };


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_New_Face                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Creates a new face object from a given resource and typeface index */
  /*    using a pathname to the font file.                                 */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    library    :: A handle to the library resource.                    */
  /*                                                                       */
  /* <Input>                                                               */
  /*    pathname   :: A path to the font file.                             */
  /*    face_index :: The index of the face within the resource.  The      */
  /*                  first face has index 0.                              */
  /* <Output>                                                              */
  /*    face       :: A handle to a new face object.                       */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Unlike FreeType 1.x, this function automatically creates a glyph   */
  /*    slot for the face object which can be accessed directly through    */
  /*    `face->glyph'.                                                     */
  /*                                                                       */
  /*    Note that additional slots can be added to each face with the      */
  /*    FT_New_GlyphSlot() API function.  Slots are linked in a single     */
  /*    list through their `next' field.                                   */
  /*                                                                       */
  /*    FT_New_Face() can be used to determine and/or check the font       */
  /*    format of a given font resource.  If the `face_index' field is     */
  /*    negative, the function will _not_ return any face handle in        */
  /*    `*face'.  Its return value should be 0 if the resource is          */
  /*    recognized, or non-zero if not.                                    */
  /*                                                                       */
  EXPORT_FUNC
  FT_Error  FT_New_Face( FT_Library   library,
                         const char*  pathname,
                         FT_Long      face_index,
                         FT_Face*     aface )
  {
    FT_Open_Args  args = ft_default_open_args;

    
    args.pathname = (char*)pathname;
    return FT_Open_Face( library, &args, face_index, aface );
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_New_Memory_Face                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Creates a new face object from a given resource and typeface index */
  /*    using a font file already loaded into memory.                      */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    library    :: A handle to the library resource.                    */
  /*                                                                       */
  /* <Input>                                                               */
  /*    file_base  :: A pointer to the beginning of the font data.         */
  /*    file_size  :: The size of the memory chunk used by the font data.  */
  /*    face_index :: The index of the face within the resource.  The      */
  /*                  first face has index 0.                              */
  /* <Output>                                                              */
  /*    face       :: A handle to a new face object.                       */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Unlike FreeType 1.x, this function automatically creates a glyph   */
  /*    slot for the face object which can be accessed directly through    */
  /*    `face->glyph'.                                                     */
  /*                                                                       */
  /*    Note that additional slots can be added to each face with the      */
  /*    FT_New_GlyphSlot() API function.  Slots are linked in a single     */
  /*    list through their `next' field.                                   */
  /*                                                                       */
  /*    FT_New_Memory_Face() can be used to determine and/or check the     */
  /*    font format of a given font resource.  If the `face_index' field   */
  /*    is negative, the function will _not_ return any face handle in     */
  /*    `*face'.  Its return value should be 0 if the resource is          */
  /*    recognized, or non-zero if not.                                    */
  /*                                                                       */
  EXPORT_FUNC
  FT_Error  FT_New_Memory_Face( FT_Library   library,
                                void*        file_base,
                                FT_Long      file_size,
                                FT_Long      face_index,
                                FT_Face*     face )
  {
    FT_Open_Args  args = ft_default_open_args;


    args.memory_base = file_base;
    args.memory_size = file_size;
    return FT_Open_Face( library, &args, face_index, face );
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Open_Face                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Opens a face object from a given resource and typeface index using */
  /*    an `FT_Open_Args' structure.  If the face object doesn't exist, it */
  /*    will be created.                                                   */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    library    :: A handle to the library resource.                    */
  /*                                                                       */
  /* <Input>                                                               */
  /*    args       :: A pointer to an `FT_Open_Args' structure which must  */
  /*                  be filled by the caller.                             */
  /*    face_index :: The index of the face within the resource.  The      */
  /*                  first face has index 0.                              */
  /* <Output>                                                              */
  /*    face       :: A handle to a new face object.                       */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Unlike FreeType 1.x, this function automatically creates a glyph   */
  /*    slot for the face object which can be accessed directly through    */
  /*    `face->glyph'.                                                     */
  /*                                                                       */
  /*    Note that additional slots can be added to each face with the      */
  /*    FT_New_GlyphSlot() API function.  Slots are linked in a single     */
  /*    list through their `next' field.                                   */
  /*                                                                       */
  /*    FT_Open_Face() can be used to determine and/or check the font      */
  /*    format of a given font resource.  If the `face_index' field is     */
  /*    negative, the function will _not_ return any face handle in        */
  /*    `*face'.  Its return value should be 0 if the resource is          */
  /*    recognized, or non-zero if not.                                    */
  /*                                                                       */
  EXPORT_FUNC
  FT_Error  FT_Open_Face( FT_Library     library,
                          FT_Open_Args*  args,
                          FT_Long        face_index,
                          FT_Face*       aface )
  {
    FT_Error     error;
    FT_Driver    driver;
    FT_Memory    memory;
    FT_Stream    stream;
    FT_Face      face = 0;
    FT_ListNode  node = 0;


    *aface = 0;

    if ( !library )
      return FT_Err_Invalid_Handle;

    /* create input stream */
    error = ft_new_input_stream( library, args, &stream );
    if ( error )
      goto Exit;

    memory = library->memory;

    /* if the font driver is specified in the `args' structure, use */
    /* it.  Otherwise, we'll scan the list of registered drivers.   */
    if ( args->driver )
    {
      driver = args->driver;
      /* not all drivers directly support face objects, so check... */
      if ( driver->interface.face_object_size )
      {
        error = open_face( driver, stream, face_index, &face );
        if ( !error )
          goto Success;
      }
      else
        error = FT_Err_Invalid_Handle;

      goto Fail;
    }
    else
    {
      /* check each font driver for an appropriate format */
      FT_Driver*  cur   = library->drivers;
      FT_Driver*  limit = cur + library->num_drivers;


      for ( ; cur < limit; cur++ )
      {
        driver = *cur;
        /* not all drivers directly support face objects, so check... */
        if ( driver->interface.face_object_size )
        {
          error = open_face( driver, stream, face_index, &face );
          if ( !error )
            goto Success;

          if ( error != FT_Err_Unknown_File_Format )
            goto Fail;
        }
      }

      /* no driver is able to handle this format */
      error = FT_Err_Unknown_File_Format;
      goto Fail;
    }

  Success:
    FT_TRACE4(( "FT_New_Face: New face object, adding to list\n" ));

    /* add the face object to its driver's list */
    if ( ALLOC( node, sizeof ( *node ) ) )
      goto Fail;

    node->data = face;
    /* don't assume driver is the same as face->driver, so use 
       face->driver instead. (JvR 3/5/2000) */
    FT_List_Add( &face->driver->faces_list, node );

    /* now allocate a glyph slot object for the face */
    {
      FT_GlyphSlot  slot;


      FT_TRACE4(( "FT_Open_Face: Creating glyph slot\n" ));
      error = FT_New_GlyphSlot( face, &slot );
      if ( error )
        goto Fail;
    }

    /* finally allocate a size object for the face */
    {
      FT_Size  size;


      FT_TRACE4(( "FT_Open_Face: Creating size object\n" ));
      error = FT_New_Size( face, &size );
      if ( error )
        goto Fail;
    }

    *aface = face;
    goto Exit;

  Fail:
    FT_Done_Face( face );

  Exit:
    FT_TRACE4(( "FT_Open_Face: Return %d\n", error ));
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Attach_File                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    `Attaches' a given font file to an existing face.  This is usually */
  /*    to read additional information for a single face object.  For      */
  /*    example, it is used to read the AFM files that come with Type 1    */
  /*    fonts in order to add kerning data and other metrics.              */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    face         :: The target face object.                            */
  /*                                                                       */
  /* <Input>                                                               */
  /*    filepathname :: An 8-bit pathname naming the `metrics' file.       */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    If your font file is in memory, or if you want to provide your     */
  /*    own input stream object, use FT_Attach_Stream().                   */
  /*                                                                       */
  /*    The meaning of the `attach' action (i.e., what really happens when */
  /*    the new file is read) is not fixed by FreeType itself.  It really  */
  /*    depends on the font format (and thus the font driver).             */
  /*                                                                       */
  /*    Client applications are expected to know what they are doing       */
  /*    when invoking this function. Most  drivers simply do not implement */
  /*    file attachments.                                                  */
  /*                                                                       */
  EXPORT_FUNC
  FT_Error  FT_Attach_File( FT_Face      face,
                            const char*  filepathname )
  {
    FT_Open_Args  open = ft_default_open_args;

    open.pathname = (char*)filepathname;
    return FT_Attach_Stream( face, &open );
  }

                            
  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Attach_Stream                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This function is similar to FT_Attach_File with the exception      */
  /*    that it reads the attachment from an arbitrary stream.             */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face :: target face object                                         */
  /*                                                                       */
  /*    args :: a pointer to an FT_Open_Args structure used to describe    */
  /*            the input stream to FreeType                               */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  /*    The meaning of the "attach" (i.e. what really happens when the     */
  /*    new file is read) is not fixed by FreeType itself. It really       */
  /*    depends on the font format (and thus the font driver).             */
  /*                                                                       */
  /*    Client applications are expected to know what they're doing        */
  /*    when invoking this function. Most drivers simply do not implement  */
  /*    file attachments..                                                 */
  /*                                                                       */
  EXPORT_DEF
  FT_Error  FT_Attach_Stream( FT_Face       face,
                              FT_Open_Args* parameters )
  {
    FT_Stream  stream;
    FT_Error   error;
    FT_Driver  driver;
    
    FTDriver_getInterface  get_interface;
    
    if ( !face || !face->driver )
      return FT_Err_Invalid_Handle;
      
    driver = face->driver;
    error = ft_new_input_stream( driver->library, parameters, &stream );
    if (error) goto Exit;
    
    /* we implement FT_Attach_Stream in each driver through the */
    /* "attach_file" interface..                                */
    error = FT_Err_Unimplemented_Feature;

    get_interface = driver->interface.get_interface;
    if (get_interface)    
    {
      FT_Attach_Reader  reader;
      
      reader = (FT_Attach_Reader)(get_interface( driver, "attach_file" ));
      if (reader)
        error = reader( face, stream );
    }
    
    /* close the attached stream */
    ft_done_stream( &stream );
    
  Exit:
    return error;
  }



  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Done_Face                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Discards a given face object, as well as all of its child slots    */
  /*    and sizes.                                                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face :: A handle to a target face object.                          */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  EXPORT_FUNC
  FT_Error  FT_Done_Face( FT_Face  face )
  {
    FT_Error             error;
    FT_Driver            driver;
    FT_Memory            memory;
    FT_DriverInterface*  interface;
    FT_ListNode          node;


    if (!face || !face->driver )
      return FT_Err_Invalid_Face_Handle;

    driver    = face->driver;
    interface = &driver->interface;
    memory    = driver->memory;

    /* find face in driver's list */
    node = FT_List_Find( &driver->faces_list, face );
    if ( node )
    {
      /* remove face object from the driver's list */
      FT_List_Remove( &driver->faces_list, node );
      FREE( node );

      /* now destroy the object proper */
      destroy_face( memory, face, driver );
      error = FT_Err_Ok;
    }
    else
      error = FT_Err_Invalid_Face_Handle;

    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_New_Size                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Creates a new size object from a given face object.                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face :: A handle to a parent face object.                          */
  /*                                                                       */
  /* <Output>                                                              */
  /*    size :: A handle to a new size object.                             */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  EXPORT_FUNC
  FT_Error  FT_New_Size( FT_Face   face,
                         FT_Size*  asize )
  {
    FT_Error             error;
    FT_Memory            memory;
    FT_Driver            driver;
    FT_DriverInterface*  interface;

    FT_Size              size = 0;
    FT_ListNode          node = 0;


    if ( !face || !face->driver )
      return FT_Err_Invalid_Face_Handle;

    *asize    = 0;
    driver    = face->driver;
    interface = &driver->interface;
    memory    = face->memory;

    /* Allocate new size object and perform basic initialisation */
    if ( ALLOC( size, interface->size_object_size ) ||
         ALLOC( node, sizeof ( FT_ListNodeRec )   ) )
      goto Exit;

    size->face = face;

    error = interface->init_size( size );

    /* in case of success, add to the face's list */
    if ( !error )
    {
      *asize     = size;
      node->data = size;
      FT_List_Add( &face->sizes_list, node );

      /* record as current size for the face */
      face->size = size;
    }

  Exit:
    if ( error )
    {
      FREE( node );
      FREE( size );
    }

    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Done_Size                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Discards a given size object.                                      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    size :: A handle to a target size object                           */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  EXPORT_FUNC
  FT_Error  FT_Done_Size( FT_Size  size )
  {
    FT_Error     error;
    FT_Driver    driver;
    FT_Memory    memory;
    FT_Face      face;
    FT_ListNode  node;


    if ( !size || !size->face )
      return FT_Err_Invalid_Size_Handle;

    driver = size->face->driver;
    if ( !driver )
      return FT_Err_Invalid_Driver_Handle;

    memory = driver->memory;

    error = FT_Err_Ok;
    face  = size->face;
    node  = FT_List_Find( &face->sizes_list, size );
    if ( node )
    {
      FT_List_Remove( &face->sizes_list, node );
      FREE( node );

      if (face->size == size)
      {
        face->size = 0;
        if (face->sizes_list.head)
          face->size = (FT_Size)(face->sizes_list.head->data);
      }

      destroy_size( memory, size, driver );
    }
    else
      error = FT_Err_Invalid_Size_Handle;

    return FT_Err_Ok;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Set_Char_Size                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Sets the character dimensions of a given size object.  The         */
  /*    `char_size' value is used for the width and height, expressed in   */
  /*    26.6 fractional points.  1 point = 1/72 inch.                      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    size      :: A handle to a target size object.                     */
  /*    char_size :: The character size, in 26.6 fractional points.        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  /* <Note>                                                                */
  /*    When dealing with fixed-size faces (i.e., non-scalable formats),   */
  /*    use the function FT_Set_Pixel_Sizes().                             */
  /*                                                                       */
  EXPORT_FUNC
  FT_Error  FT_Set_Char_Size( FT_Face     face,
                              FT_F26Dot6  char_width,
                              FT_F26Dot6  char_height,
                              FT_UInt     horz_resolution,
                              FT_UInt     vert_resolution )
  {
    FT_Error             error;
    FT_Driver            driver;
    FT_Memory            memory;
    FT_DriverInterface*  interface;
    FT_Size_Metrics*     metrics = &face->size->metrics;
    FT_Long              dim_x, dim_y;

    if ( !face || !face->size || !face->driver)
      return FT_Err_Invalid_Face_Handle;

    if (!char_width)
      char_width = char_height;

    else if (!char_height)
      char_height = char_width;

    if (!horz_resolution)
      horz_resolution = 72;

    if (!vert_resolution)
      vert_resolution = 72;

    driver    = face->driver;
    interface = &driver->interface;
    memory    = driver->memory;

    /* default processing - this can be overriden by the driver */
    if ( char_width  < 1*64 ) char_width  = 1*64;
    if ( char_height < 1*64 ) char_height = 1*64;

    /* Compute pixel sizes in 26.6 units */
    dim_x = (((char_width  * horz_resolution) / 72) + 32) & -64;
    dim_y = (((char_height * vert_resolution) / 72) + 32) & -64;

    metrics->x_ppem    = (FT_UShort)(dim_x >> 6);
    metrics->y_ppem    = (FT_UShort)(dim_y >> 6);

    metrics->x_scale = 0x10000;
    metrics->y_scale = 0x10000;
    if ( face->face_flags & FT_FACE_FLAG_SCALABLE)
    {
      metrics->x_scale = FT_DivFix( dim_x, face->units_per_EM );
      metrics->y_scale = FT_DivFix( dim_y, face->units_per_EM );
    }

    error = interface->set_char_sizes( face->size,
                                       char_width,
                                       char_height,
                                       horz_resolution,
                                       vert_resolution );
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Set_Pixel_Sizes                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Sets the character dimensions of a given size object.  The width   */
  /*    and height are expressed in integer pixels.                        */
  /*                                                                       */
  /* <Input>                                                               */
  /*    size         :: A handle to a target size object.                  */
  /*    pixel_width  :: The character width, in integer pixels.            */
  /*    pixel_height :: The character height, in integer pixels.           */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  EXPORT_FUNC
  FT_Error  FT_Set_Pixel_Sizes( FT_Face  face,
                                FT_UInt  pixel_width,
                                FT_UInt  pixel_height )
  {
    FT_Error             error;
    FT_Driver            driver;
    FT_Memory            memory;
    FT_DriverInterface*  interface;
    FT_Size_Metrics*     metrics = &face->size->metrics;

    if ( !face || !face->size || !face->driver )
      return FT_Err_Invalid_Face_Handle;

    driver    = face->driver;
    interface = &driver->interface;
    memory    = driver->memory;

    /* default processing - this can be overriden by the driver */
    if ( pixel_width  < 1 ) pixel_width  = 1;
    if ( pixel_height < 1 ) pixel_height = 1;

    metrics->x_ppem = pixel_width;
    metrics->y_ppem = pixel_height;

    if ( face->face_flags & FT_FACE_FLAG_SCALABLE )
    {
      metrics->x_scale = FT_DivFix( metrics->x_ppem << 6,
                                    face->units_per_EM );

      metrics->y_scale = FT_DivFix( metrics->y_ppem << 6,
                                    face->units_per_EM );
    }

    error = interface->set_pixel_sizes( face->size, 
                                        pixel_width,
                                        pixel_height );
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_New_GlyphSlot                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    It is sometimes useful to have more than one glyph slot for a      */
  /*    given face object.  This function is used to create additional     */
  /*    slots.  All of them are automatically discarded when the face is   */
  /*    destroyed.                                                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face  :: A handle to a parent face object.                         */
  /*                                                                       */
  /* <Output>                                                              */
  /*    aslot :: A handle to a new glyph slot object.                      */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  EXPORT_FUNC
  FT_Error  FT_New_GlyphSlot( FT_Face        face,
                              FT_GlyphSlot*  aslot )
  {
    FT_Error             error;
    FT_Driver            driver;
    FT_DriverInterface*  interface;
    FT_Memory            memory;
    FT_GlyphSlot         slot;


    *aslot = 0;

    if ( !face || !face->driver )
      return FT_Err_Invalid_Face_Handle;

    driver    = face->driver;
    interface = &driver->interface;
    memory    = driver->memory;

    FT_TRACE4(( "FT_New_GlyphSlot: Creating new slot object\n" ));
    if ( ALLOC( slot, interface->slot_object_size ) )
      goto Exit;

    slot->face = face;

    slot->max_subglyphs = 0;
    slot->num_subglyphs = 0;
    slot->subglyphs     = 0;

    error = interface->init_glyph_slot( slot );
    if ( !error )
    {
      /* in case of success, add slot to the face's list */
      slot->next  = face->glyph;
      face->glyph = slot;
      *aslot      = slot;
    }

    if ( error )
      FREE( slot );

  Exit:
    FT_TRACE4(( "FT_New_GlyphSlot: Return %d\n", error ));
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Done_GlyphSlot                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Destroys a given glyph slot.  Remember however that all slots are  */
  /*    automatically destroyed with its parent.  Using this function is   */
  /*    not always mandatory.                                              */
  /*                                                                       */
  /* <Input>                                                               */
  /*    slot :: A handle to a target glyph slot.                           */
  /*                                                                       */
  EXPORT_FUNC
  void  FT_Done_GlyphSlot( FT_GlyphSlot  slot )
  {
    if (slot)
    {
      FT_Driver      driver = slot->face->driver;
      FT_Memory      memory = driver->memory;
      FT_GlyphSlot*  parent;
      FT_GlyphSlot   cur;


      /* Remove slot from its parent face's list */
      parent = &slot->face->glyph;
      cur    = *parent;
      while ( cur )
      {
        if ( cur == slot )
        {
          *parent = cur->next;
          break;
        }
        cur = cur->next;
      }

      driver->interface.done_glyph_slot( slot );
      FREE( slot );
    }
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Load_Glyph                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A function used to load a single glyph within a given glyph slot,  */
  /*    for a given size.                                                  */
  /*                                                                       */
  /* <Input>                                                               */
  /*    slot        :: A handle to a target slot object where the glyph    */
  /*                   will be loaded.                                     */
  /*                                                                       */
  /*    size        :: A handle to the source face size at which the glyph */
  /*                   must be scaled/loaded.                              */
  /*                                                                       */
  /*    glyph_index :: The index of the glyph in the font file.            */
  /*                                                                       */
  /*    load_flags  :: A flag indicating what to load for this glyph.  The */
  /*                   FT_LOAD_XXX constants can be used to control the    */
  /*                   glyph loading process (e.g., whether the outline    */
  /*                   should be scaled, whether to load bitmaps or not,   */
  /*                   whether to hint the outline, etc).                  */
  /* <Output>                                                              */
  /*    result      :: A set of bit flags indicating the type of data that */
  /*                   was loaded in the glyph slot (outline or bitmap,    */
  /*                   etc).                                               */
  /*                                                                       */
  /*                   You can set this field to 0 if you don't want this  */
  /*                   information.                                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  EXPORT_FUNC
  FT_Error  FT_Load_Glyph( FT_Face  face,
                           FT_UInt  glyph_index,
                           FT_Int   load_flags )
  {
    FT_Error   error;
    FT_Driver  driver;

    if ( !face || !face->size || !face->glyph )
      return FT_Err_Invalid_Face_Handle;

    driver = face->driver;

    error = driver->interface.load_glyph( face->glyph,
                                          face->size,
                                          glyph_index,
                                          load_flags );

    return error;
  }


  EXPORT_FUNC
  FT_Error  FT_Load_Char( FT_Face   face,
                          FT_ULong  char_code,
                          FT_Int    load_flags )
  {
    FT_Error   error;
    FT_Driver  driver;
    FT_UInt    glyph_index;

    if (!face || !face->size || !face->glyph || !face->charmap )
      return FT_Err_Invalid_Face_Handle;

    driver      = face->driver;
    glyph_index = FT_Get_Char_Index( face, char_code );

    if (glyph_index == 0)
      error = FT_Err_Invalid_Character_Code;
    else
      error = driver->interface.load_glyph( face->glyph,
                                            face->size,
                                            glyph_index,
                                            load_flags );
    return error;
  }

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Get_Kerning                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Returns the kerning vector between two glyphs of a same face.      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face        :: A handle to a source face object.                   */
  /*                                                                       */
  /*    left_glyph  :: The index of the left glyph in the kern pair.       */
  /*                                                                       */
  /*    right_glyph :: The index of the right glyph in the kern pair.      */
  /*                                                                       */
  /* <Output>                                                              */
  /*    kerning     :: The kerning vector.  This is in font units for      */
  /*                   scalable formats, and in pixels for fixed-sizes     */
  /*                   formats.                                            */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Only horizontal layouts (left-to-right & right-to-left) are        */
  /*    supported by this method.  Other layouts, or more sophisticated    */
  /*    kernings, are out of the scope of this API function -- they can be */
  /*    implemented through format-specific interfaces.                    */
  /*                                                                       */
  EXPORT_FUNC
  FT_Error  FT_Get_Kerning( FT_Face     face,
                            FT_UInt     left_glyph,
                            FT_UInt     right_glyph,
                            FT_Vector*  kerning )
  {
    FT_Error   error;
    FT_Driver  driver;
    FT_Memory  memory;


    if ( !face )
    {
      error = FT_Err_Invalid_Face_Handle;
      goto Exit;
    }

    driver = face->driver;
    memory = driver->memory;

    if ( driver->interface.get_kerning )
    {
      error = driver->interface.get_kerning( face, left_glyph,
                                             right_glyph, kerning );
    }
    else
    {
      kerning->x = 0;
      kerning->y = 0;
      error      = FT_Err_Ok;
    }

  Exit:
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Get_Char_Index                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Returns the glyph index of a given character code.  This function  */
  /*    uses a charmap object to do the translation.                       */
  /*                                                                       */
  /* <Input>                                                               */
  /*    charmap  :: A handle to a filter charmap object.                   */
  /*    charcode :: The character code.                                    */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The glyph index.  0 means `undefined character code'.              */
  /*                                                                       */
  EXPORT_DEF
  FT_UInt  FT_Get_Char_Index( FT_Face  face,
                              FT_ULong charcode )
  {
    FT_UInt    result;
    FT_Driver  driver;

    result = 0;
    if ( face && face->charmap )
    {
      driver = face->driver;
      result = driver->interface.get_char_index( face->charmap, charcode );
    }
    return result;
  }

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Outline_Decompose                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Walks over an outline's structure to decompose it into individual  */
  /*    segments and Bezier arcs.  This function is also able to emit      */
  /*    `move to' and `close to' operations to indicate the start and end  */
  /*    of new contours in the outline.                                    */
  /*                                                                       */
  /* <Input>                                                               */
  /*    outline   :: A pointer to the source target.                       */
  /*                                                                       */
  /*    interface :: A table of `emitters', i.e,. function pointers called */
  /*                 during decomposition to indicate path operations.     */
  /*                                                                       */
  /*    user      :: A typeless pointer which is passed to each emitter    */
  /*                 during the decomposition.  It can be used to store    */
  /*                 the state during the decomposition.                   */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means sucess.                                       */
  /*                                                                       */
  EXPORT_FUNC
  int  FT_Outline_Decompose( FT_Outline*        outline,
                             FT_Outline_Funcs*  interface,
                             void*              user )
  {
    typedef enum _phases
    {
      phase_point,
      phase_conic,
      phase_cubic,
      phase_cubic2

    } TPhase;

    FT_Vector  v_first;
    FT_Vector  v_last;
    FT_Vector  v_control;
    FT_Vector  v_control2;
    FT_Vector  v_start;

    FT_Vector* point;
    char*      tags;

    int    n;         /* index of contour in outline     */
    int    first;     /* index of first point in contour */
    int    index;     /* current point's index           */

    int    error;

    char   tag;       /* current point's state           */
    TPhase phase;


    first = 0;

    for ( n = 0; n < outline->n_contours; n++ )
    {
      int  last;  /* index of last point in contour */


      last = outline->contours[n];

      v_first = outline->points[first];
      v_last  = outline->points[last];

      v_start = v_control = v_first;

      tag   = FT_CURVE_TAG( outline->tags[first] );
      index = first;

      /* A contour cannot start with a cubic control point! */

      if ( tag == FT_Curve_Tag_Cubic )
        return FT_Err_Invalid_Outline;


      /* check first point to determine origin */

      if ( tag == FT_Curve_Tag_Conic )
      {
        /* first point is conic control.  Yes, this happens. */
        if ( FT_CURVE_TAG( outline->tags[last] ) == FT_Curve_Tag_On )
        {
          /* start at last point if it is on the curve */
          v_start = v_last;
        }
        else
        {
          /* if both first and last points are conic,         */
          /* start at their middle and record its position    */
          /* for closure                                      */
          v_start.x = ( v_start.x + v_last.x ) / 2;
          v_start.y = ( v_start.y + v_last.y ) / 2;

          v_last = v_start;
        }
        phase = phase_conic;
      }
      else
        phase = phase_point;


      /* Begin a new contour with MOVE_TO */

      error = interface->move_to( &v_start, user );
      if ( error )
        return error;

      point = outline->points + first;
      tags  = outline->tags  + first;

      /* now process each contour point individually */

      while ( index < last )
      {
        index++;
        point++;
        tags++;

        tag = FT_CURVE_TAG( tags[0] );

        switch ( phase )
        {
        case phase_point:     /* the previous point was on the curve */

          switch ( tag )
          {
            /* two succesive on points -> emit segment */
          case FT_Curve_Tag_On:
            error = interface->line_to( point, user );
            break;

            /* on point + conic control -> remember control point */
          case FT_Curve_Tag_Conic:
            v_control = point[0];
            phase     = phase_conic;
            break;

            /* on point + cubic control -> remember first control */
          default:
            v_control = point[0];
            phase     = phase_cubic;
            break;
          }
          break;

        case phase_conic:   /* the previous point was a conic control */

          switch ( tag )
          {
            /* conic control + on point -> emit conic arc */
          case  FT_Curve_Tag_On:
            error = interface->conic_to( &v_control, point, user );
            phase = phase_point;
            break;

            /* two successive conics -> emit conic arc `in between' */
          case FT_Curve_Tag_Conic:
            {
              FT_Vector  v_middle;


              v_middle.x = (v_control.x + point->x)/2;
              v_middle.y = (v_control.y + point->y)/2;

              error = interface->conic_to( &v_control,
                                           &v_middle, user );
              v_control = point[0];
            }
             break;

          default:
            error = FT_Err_Invalid_Outline;
          }
          break;

        case phase_cubic:  /* the previous point was a cubic control */

          /* this point _must_ be a cubic control too */
          if ( tag != FT_Curve_Tag_Cubic )
            return FT_Err_Invalid_Outline;

          v_control2 = point[0];
          phase      = phase_cubic2;
          break;


        case phase_cubic2:  /* the two previous points were cubics */

          /* this point _must_ be an on point */
          if ( tag != FT_Curve_Tag_On )
            error = FT_Err_Invalid_Outline;
          else
            error = interface->cubic_to( &v_control, &v_control2,
                                         point, user );
          phase = phase_point;
          break;
        }

        /* lazy error testing */
        if ( error )
          return error;
      }

      /* end of contour, close curve cleanly */
      error = 0;

      tag = FT_CURVE_TAG( outline->tags[first] );

      switch ( phase )
      {
      case phase_point:
        if ( tag == FT_Curve_Tag_On )
          error = interface->line_to( &v_first, user );
        break;

      case phase_conic:
        error = interface->conic_to( &v_control, &v_start, user );
        break;

      case phase_cubic2:
        if ( tag == FT_Curve_Tag_On )
          error = interface->cubic_to( &v_control, &v_control2,
                                       &v_first,   user );
        else
          error = FT_Err_Invalid_Outline;
        break;

      default:
        error = FT_Err_Invalid_Outline;
        break;
      }

      if ( error )
        return error;

      first = last + 1;
    }

    return 0;
  }


  static
  const FT_Outline  null_outline = { 0, 0, 0, 0, 0, 0 };


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Outline_New                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Creates a new outline of a given size.                             */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library     :: A handle to the library object from where the       */
  /*                   outline is allocated.  Note however that the new    */
  /*                   outline will NOT necessarily be FREED when          */
  /*                   destroying the library, by FT_Done_FreeType().      */
  /*                                                                       */
  /*    numPoints   :: The maximum number of points within the outline.    */
  /*                                                                       */
  /*    numContours :: The maximum number of contours within the outline.  */
  /*                                                                       */
  /* <Output>                                                              */
  /*    outline     :: A handle to the new outline.  NULL in case of       */
  /*                   error.                                              */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <MT-Note>                                                             */
  /*    No.                                                                */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The reason why this function takes a `library' parameter is simply */
  /*    to use the library's memory allocator.  You can copy the source    */
  /*    code of this function, replacing allocations with `malloc()' if    */
  /*    you want to control where the objects go.                          */
  /*                                                                       */
  BASE_FUNC
  FT_Error  FT_Outline_New( FT_Library   library,
                            FT_UInt      numPoints,
                            FT_Int       numContours,
                            FT_Outline*  outline )
  {
    FT_Error   error;
    FT_Memory  memory;


    if ( !outline )
      return FT_Err_Invalid_Argument;

    *outline = null_outline;
    memory   = library->memory;

    if ( ALLOC_ARRAY( outline->points,   numPoints * 2L, FT_Pos    ) ||
         ALLOC_ARRAY( outline->tags,    numPoints,      FT_Byte   ) ||
         ALLOC_ARRAY( outline->contours, numContours,    FT_UShort ) )
      goto Fail;

    outline->n_points       = (FT_UShort)numPoints;
    outline->n_contours     = (FT_Short)numContours;
    outline->flags |= ft_outline_owner;

    return FT_Err_Ok;

  Fail:
    outline->flags |= ft_outline_owner;
    FT_Outline_Done( library, outline );

    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Outline_Done                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Destroys an outline created with FT_Outline_New().                 */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library :: A handle of the library object used to allocate the     */
  /*               outline.                                                */
  /*                                                                       */
  /*    outline :: A pointer to the outline object to be discarded.        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <MT-Note>                                                             */
  /*    No.                                                                */
  /*                                                                       */
  /* <Note>                                                                */
  /*    If the outline's `owner' field is not set, only the outline        */
  /*    descriptor will be released.                                       */
  /*                                                                       */
  /*    The reason why this function takes an `outline' parameter is       */
  /*    simply to use FT_Alloc()/FT_Free().  You can copy the source code  */
  /*    of this function, replacing allocations with `malloc()' in your    */
  /*    application if you want something simpler.                         */
  /*                                                                       */
  BASE_FUNC
  FT_Error  FT_Outline_Done( FT_Library   library,
                             FT_Outline*  outline )
  {
    FT_Memory  memory = library->memory;


    if ( outline )
    {
      if ( outline->flags & ft_outline_owner )
      {
        FREE( outline->points   );
        FREE( outline->tags    );
        FREE( outline->contours );
      }
      *outline = null_outline;

      return FT_Err_Ok;
    }
    else
      return FT_Err_Invalid_Argument;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Outline_Get_CBox                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Returns an outline's `control box'.  The control box encloses all  */
  /*    the outline's points, including Bezier control points.  Though it  */
  /*    coincides with the exact bounding box for most glyphs, it can be   */
  /*    slightly larger in some situations (like when rotating an outline  */
  /*    which contains Bezier outside arcs).                               */
  /*                                                                       */
  /*    Computing the control box is very fast, while getting the bounding */
  /*    box can take much more time as it needs to walk over all segments  */
  /*    and arcs in the outline.  To get the latter, you can use the       */
  /*    `ftbbox' component which is dedicated to this single task.         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    outline :: A pointer to the source outline descriptor.             */
  /*                                                                       */
  /* <Output>                                                              */
  /*    cbox    :: The outline's control box.                              */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <MT-Note>                                                             */
  /*    Yes.                                                               */
  /*                                                                       */
  BASE_FUNC
  FT_Error  FT_Outline_Get_CBox( FT_Outline*  outline,
                                 FT_BBox*     cbox )
  {
    FT_Pos  xMin, yMin, xMax, yMax;
    
    if ( outline && cbox )
    {
      if ( outline->n_points == 0 )
      {
        xMin = 0;
        yMin = 0;
        xMax = 0;
        yMax = 0;
      }
      else
      {
        FT_Vector*  vec   = outline->points;
        FT_Vector*  limit = vec + outline->n_points;

        xMin = xMax = vec->x;
        yMin = yMax = vec->y;
        vec++;

        for ( ; vec < limit; vec++ )
        {
          FT_Pos  x, y;

          x = vec->x;
          if ( x < xMin ) xMin = x;
          if ( x > xMax ) xMax = x;

          y = vec->y;
          if ( y < yMin ) yMin = y;
          if ( y > yMax ) yMax = y;
        }
      }
      cbox->xMin = xMin;
      cbox->xMax = xMax;
      cbox->yMin = yMin;
      cbox->yMax = yMax;
      return FT_Err_Ok;
    }
    else
      return FT_Err_Invalid_Argument;
  }

  

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Outline_Translate                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Applies a simple translation to the points of an outline.          */
  /*                                                                       */
  /* <Input>                                                               */
  /*    outline :: A pointer to the target outline descriptor.             */
  /*    xOffset :: The horizontal offset.                                  */
  /*    yOffset :: The vertical offset.                                    */
  /*                                                                       */
  /* <MT-Note>                                                             */
  /*    Yes.                                                               */
  /*                                                                       */
  BASE_FUNC
  void  FT_Outline_Translate( FT_Outline*  outline,
                              FT_Pos       xOffset,
                              FT_Pos       yOffset )
  {
    FT_UShort   n;
    FT_Vector*  vec = outline->points;

    for ( n = 0; n < outline->n_points; n++ )
    {
      vec->x += xOffset;
      vec->y += yOffset;
      vec++;
    }
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Done_GlyphZone                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Deallocates a glyph zone.                                          */
  /*                                                                       */
  /* <Input>                                                               */
  /*    zone  :: pointer to the target glyph zone.                         */
  /*                                                                       */
  BASE_FUNC 
  void  FT_Done_GlyphZone( FT_GlyphZone*  zone )
  {
    FT_Memory  memory = zone->memory;
    
    FREE( zone->contours );
    FREE( zone->tags );
    FREE( zone->cur );
    FREE( zone->org );

    zone->max_points   = zone->n_points   = 0;
    zone->max_contours = zone->n_contours = 0;
  }
  
  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_New_GlyphZone                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Allocates a new glyph zone.                                        */
  /*                                                                       */
  /* <Input>                                                               */
  /*    memory      :: A handle to the current memory object.              */
  /*                                                                       */
  /*    maxPoints   :: The capacity of glyph zone in points.               */
  /*                                                                       */
  /*    maxContours :: The capacity of glyph zone in contours.             */
  /*                                                                       */
  /* <Output>                                                              */
  /*    zone        :: A pointer to the target glyph zone record.          */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  BASE_FUNC 
  FT_Error  FT_New_GlyphZone( FT_Memory      memory,
                              FT_UShort      maxPoints,
                              FT_Short       maxContours,
                              FT_GlyphZone*  zone )
  {
    FT_Error      error;

    if (maxPoints > 0)
      maxPoints += 2;

    MEM_Set( zone, 0, sizeof(*zone) );
    zone->memory = memory;
    
    if ( ALLOC_ARRAY( zone->org,      maxPoints*2, FT_F26Dot6 ) ||
         ALLOC_ARRAY( zone->cur,      maxPoints*2, FT_F26Dot6 ) ||
         ALLOC_ARRAY( zone->tags,    maxPoints,   FT_Byte    ) ||
         ALLOC_ARRAY( zone->contours, maxContours, FT_UShort  ) )
    {
      FT_Done_GlyphZone(zone);
    }
    return error;
  }

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Update_GlyphZone                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Checks the size of a zone and reallocates it if necessary.         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    newPoints   :: The new capacity for points.  We add two slots for  */
  /*                   phantom points.                                     */
  /*                                                                       */
  /*    newContours :: The new capacity for contours.                      */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    zone        :: The address of the target zone.                     */
  /*                                                                       */
  /*    maxPoints   :: The address of the zone's current capacity for      */
  /*                   points.                                             */
  /*                                                                       */
  /*    maxContours :: The address of the zone's current capacity for      */
  /*                   contours.                                           */
  /*                                                                       */
  BASE_FUNC
  FT_Error  FT_Update_GlyphZone( FT_GlyphZone*  zone,
                                 FT_UShort      newPoints,
                                 FT_Short       newContours )
  {
    FT_Error      error  = FT_Err_Ok;
    FT_Memory     memory = zone->memory;
    
    newPoints += 2;
    if ( zone->max_points < newPoints )
    {
      /* reallocate the points arrays */
      if ( REALLOC_ARRAY( zone->org,   zone->max_points*2, newPoints*2, FT_F26Dot6 ) ||
           REALLOC_ARRAY( zone->cur,   zone->max_points*2, newPoints*2, FT_F26Dot6 ) ||
           REALLOC_ARRAY( zone->tags, zone->max_points*2, newPoints,   FT_Byte    ) )
        goto Exit;
        
      zone->max_points = newPoints;
    }
    
    if ( zone->max_contours < newContours )
    {
      /* reallocate the contours array */
      if ( REALLOC_ARRAY( zone->contours, zone->max_contours, newContours, FT_UShort ) )
        goto Exit;
        
      zone->max_contours = newContours;
    }
  Exit:
    return error;
  }



  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Done_FreeType                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Destroys a given FreeType library object and all of its childs,    */
  /*    including resources, drivers, faces, sizes, etc.                   */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library :: A handle to the target library object.                  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  EXPORT_FUNC
  FT_Error  FT_Done_FreeType( FT_Library  library )
  {
    /* Discard the library object */
    FT_Done_Library( library );

    return FT_Err_Ok;
  }

/* END */
