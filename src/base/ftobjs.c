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

#include <tttables.h>

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
                                 FT_Stream     *astream )
  {
    FT_Error   error;
    FT_Memory  memory;
    FT_Stream  stream;

    *astream = 0;
    memory   = library->memory;
    if ( ALLOC( stream, sizeof ( *stream ) ) )
      goto Exit;
      
    stream->memory = memory;
    
    /* now, look at the stream type */
    switch ( args->stream_type )
    {
      /***** is it a memory-based stream ? *****************************/
      case ft_stream_memory:
      {
        FT_New_Memory_Stream( library,
                              args->memory_base,
                              args->memory_size,
                              stream );
        break;
      }
      
      /***** is is a pathname stream ? ********************************/
      case ft_stream_pathname:
      {
        error = FT_New_Stream( args->pathname, stream );
        stream->pathname.pointer = args->pathname;
        break;
      }
      
      case ft_stream_copy:
      {
        if ( args->stream)
        {
          *stream        = *(args->stream);
          stream->memory = memory;
          break;
        }
      }
      default:
        error = FT_Err_Invalid_Argument;
    }
    
    if ( error )
      FREE( stream );
      
    *astream = stream;
  Exit:
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Done_Stream                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Closes and destroys a stream object.                               */
  /*                                                                       */
  EXPORT_FUNC
  void  FT_Done_Stream( FT_Stream  stream )
  {
    if ( stream->close )
      stream->close( stream );
  }


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


 /*************************************************************************
  *
  * <Function>
  *   FT_Get_Raster
  *
  * <Description>
  *   Return a pointer to the raster corresponding to a given glyph   
  *   format tag.      
  *
  * <Input>
  *   library      :: handle to source library object
  *   glyph_format :: glyph format tag
  *
  * <Output>
  *   raster_funcs :: if this field is not 0, returns the raster's interface
  *
  * <Return>
  *   a pointer to the corresponding raster object.
  *
  *************************************************************************/
  
  EXPORT_FUNC
  FT_Raster  FT_Get_Raster( FT_Library        library,
                            FT_Glyph_Format   glyph_format,
                            FT_Raster_Funcs  *raster_funcs )
  {
    FT_Int  n;
    
    for ( n = 0; n < FT_MAX_GLYPH_FORMATS; n++ )
    {
      FT_Raster_Funcs*  funcs = &library->raster_funcs[n];
      if (funcs->glyph_format == glyph_format)
      {
        if (raster_funcs)
          *raster_funcs = *funcs;
        return library->rasters[n];
      }
    }
    return 0;
  }

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Set_Raster                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Register a given raster to the library.                            */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library      :: A handle to a target library object.               */
  /*    raster_funcs :: pointer to the raster's interface                  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This function will do the following:                               */
  /*                                                                       */
  /*    - a new raster object is created through raster_func.raster_new    */
  /*      if this fails, then the function returns                         */
  /*                                                                       */
  /*    - if a raster is already registered for the glyph format           */
  /*      specified in raster_funcs, it will be destroyed                  */
  /*                                                                       */
  /*    - the new raster is registered for the glyph format                */
  /*                                                                       */
  EXPORT_FUNC
  FT_Error  FT_Set_Raster( FT_Library        library,
                           FT_Raster_Funcs*  raster_funcs )
  {
    FT_Glyph_Format  glyph_format = raster_funcs->glyph_format;
    FT_Raster_Funcs* funcs;
    FT_Raster        raster;
    FT_Error         error;
    FT_Int           n, index;
 
    if ( glyph_format == ft_glyph_format_none)
      return FT_Err_Invalid_Argument;
    
    /* create a new raster object */
    error = raster_funcs->raster_new( library->memory, &raster );
    if (error) goto Exit;
    
    raster_funcs->raster_reset( raster,
                                library->raster_pool,
                                library->raster_pool_size );
    
    index = -1;
    for (n = 0; n < FT_MAX_GLYPH_FORMATS; n++)
    {
      FT_Raster_Funcs*  funcs = library->raster_funcs + n;

      /* record the first vacant entry in "index" */
      if (index < 0 && funcs->glyph_format == ft_glyph_format_none)
        index = n;
      
      /* compare this entry's glyph format with the one we need */
      if (funcs->glyph_format == glyph_format)
      {
        /* a raster already exists for this glyph format, we will */
        /* destroy it before updating its entry in the table      */
        funcs->raster_done( library->rasters[n] );
        index = n;
        break;
      }
    }

    if (index < 0)
    {
      /* the table is full and has no vacant entries */
      error = FT_Err_Too_Many_Glyph_Formats;
      goto Fail;
    }

    funcs  = library->raster_funcs + index;
    *funcs = *raster_funcs;
    library->rasters[index] = raster;

  Exit:
    return error;

  Fail:
    raster_funcs->raster_done( raster );
    goto Exit;
  }                           

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Unset_Raster                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Removes a given raster from the library.                           */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library      :: A handle to a target library object.               */
  /*    raster_funcs :: pointer to the raster's interface                  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  EXPORT_DEF
  FT_Error  FT_Unset_Raster( FT_Library        library,
                             FT_Raster_Funcs*  raster_funcs )
  {
    FT_Glyph_Format  glyph_format = raster_funcs->glyph_format;
    FT_Error         error;
    FT_Int           n;
 
    error = FT_Err_Invalid_Argument;    
    if ( glyph_format == ft_glyph_format_none)
      goto Exit;

    for (n = 0; n < FT_MAX_GLYPH_FORMATS; n++)
    {
      FT_Raster_Funcs*  funcs = library->raster_funcs + n;

      if (funcs->glyph_format == glyph_format)
      {
        funcs->raster_done( library->rasters[n] );
        library->rasters[n]                   = 0;
        library->raster_funcs[n].glyph_format = ft_glyph_format_none;
        error = FT_Err_Ok;
        break;
      }
    }

  Exit:
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Set_Raster_Mode                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Set a raster-specific mode.                                        */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library :: A handle to a target library object.                    */
  /*    format  :: the glyph format used to select the raster              */
  /*    mode    :: the raster-specific mode descriptor                     */
  /*    args    :: the mode arguments                                      */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  EXPORT_FUNC
  FT_Error  FT_Set_Raster_Mode( FT_Library      library,
                                FT_Glyph_Format format,
                                unsigned long   mode,
                                void*           args )
  {
    FT_Raster_Funcs  funcs;
    FT_Raster        raster;
    
    raster = FT_Get_Raster( library, format, &funcs );
    if (raster && funcs.raster_set_mode )
      return funcs.raster_set_mode( raster, mode, args );
    else
      return FT_Err_Invalid_Argument;
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

    /* allocate the render pool */
    library->raster_pool_size = FT_RENDER_POOL_SIZE;
    if ( ALLOC( library->raster_pool, FT_RENDER_POOL_SIZE ) )
      goto Fail;

    /* now register the default raster for the `outline' glyph image format */
    /* for now, ignore the error...                                         */
    error = FT_Set_Raster( library, &ft_default_raster );
    

    /* That's ok now */
    *alibrary = library;

    return FT_Err_Ok;
  Fail:
    FREE( library );
    return error;
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

    /* Destroy raster objects */
    FREE( library->raster_pool );
    library->raster_pool_size = 0;

    {
      FT_Raster_Funcs*  cur   = library->raster_funcs;
      FT_Raster_Funcs*  limit = cur + FT_MAX_GLYPH_FORMATS;
      FT_Raster*        raster = library->rasters;

      for ( ; cur < limit; cur++, raster++ )
      {
        if ( cur->glyph_format != ft_glyph_format_none )
        {
          cur->raster_done( *raster );
          *raster = 0;
          cur->glyph_format = ft_glyph_format_none;
        }
      }
    }

    FREE( library );

    return FT_Err_Ok;
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
    FT_Open_Args  args;

    args.stream_type = ft_stream_pathname;    
    args.driver      = 0;
    args.pathname    = (char*)pathname;
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
    FT_Open_Args  args;

    args.stream_type = ft_stream_memory;
    args.memory_base = file_base;
    args.memory_size = file_size;
    args.driver      = 0;
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
    if ( error ) goto Exit;

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

    /* initialize transform for convenience functions */
    face->transform_matrix.xx = 0x10000L;
    face->transform_matrix.xy = 0;
    face->transform_matrix.yx = 0;
    face->transform_matrix.yy = 0x10000L;

    face->transform_delta.x = 0;
    face->transform_delta.y = 0;

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
    FT_Open_Args  open;

    open.stream_type = ft_stream_pathname;
    open.pathname    = (char*)filepathname;
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
    
    /* when the flag NO_RECURSE is set, we disable hinting and scaling */
    if (load_flags & FT_LOAD_NO_RECURSE)
      load_flags |= FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING;

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


 /***************************************************************************
  *
  * <Function>
  *    FT_Get_Sfnt_Table
  *
  * <Description>
  *    Returns a pointer to a given SFNT table within a face.
  *
  * <Input>
  *    face  :: handle to source
  *    tag   :: index if SFNT table
  *
  * <Return>
  *    type-less pointer to the table. This will be 0 in case of error, or
  *    when the corresponding table was not found *OR* loaded from the file.
  *
  * <Note>
  *    The table is owned by the face object, and disappears with it.
  *
  *    This function is only useful to access Sfnt tables that are loaded
  *    by the sfnt/truetype/opentype drivers. See FT_Sfnt_tag for a list.
  *
  *    You can load any table with a different function.. XXX
  *
  ***************************************************************************/
  
  
  EXPORT_FUNC
  void*  FT_Get_Sfnt_Table( FT_Face      face,
                            FT_Sfnt_Tag  tag )
  {
    void*                   table = 0;
    FT_Get_Sfnt_Table_Func  func;
    FT_Driver               driver;
    
    if (!face || !FT_IS_SFNT(face))
      goto Exit;
    
    driver = face->driver;
    func = (FT_Get_Sfnt_Table_Func)driver->interface.get_interface( driver, "get_sfnt" );
    if (func)
      table = func(face,tag);
    
  Exit:
    return table;
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
