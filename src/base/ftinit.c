/***************************************************************************/
/*                                                                         */
/*  ftinit.c                                                               */
/*                                                                         */
/*    FreeType initialisation layer (body).                                */
/*                                                                         */
/*  Copyright 1996-2000 by                                                 */
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
  /*  The purpose of this file is to implement the three following         */
  /*  functions:                                                           */
  /*                                                                       */
  /*  FT_Default_Drivers:                                                  */
  /*     This function is used to add the set of default drivers to a      */
  /*     fresh new library object.  The set is computed at compile time    */
  /*     from the Makefiles inclusions in Makefile.lib.  See the document  */
  /*     `FreeType Internals' for more info.                               */
  /*                                                                       */
  /*  FT_Init_FreeType:                                                    */
  /*     This function creates a system object for the current platform,   */
  /*     builds a library out of it, then calls FT_Default_Drivers().      */
  /*                                                                       */
  /*  FT_Done_FreeType:                                                    */
  /*     This function simply finalizes the library and the corresponding  */
  /*     system object.                                                    */
  /*                                                                       */
  /*  Note that even if FT_Init_FreeType() uses the implementation of the  */
  /*  system object defined at build time, client applications are still   */
  /*  able to provide their own `ftsystem.c'.                              */
  /*                                                                       */
  /*************************************************************************/


#include <ftobjs.h>
#include <ftconfig.h>
#include <ftdebug.h>
#include <ftdriver.h>

#undef  FT_COMPONENT
#define FT_COMPONENT  trace_init

#undef  FT_DRIVER
#define FT_DRIVER( x )  extern FT_DriverInterface  x;

#include <ftmodule.h>

#undef  FT_DRIVER
#define FT_DRIVER( x )  &x,

static
const FT_DriverInterface*  ft_default_drivers[] =
  {
#include <ftmodule.h>
    0
  };


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Default_Drivers                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Adds the set of default drivers to a given library object.         */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    library :: A handle to a new library object.                       */
  /*                                                                       */
  EXPORT_FUNC
  void  FT_Default_Drivers( FT_Library  library )
  {
    FT_Error                   error;
    const FT_DriverInterface* *cur;


    cur = ft_default_drivers;
    while ( *cur )
    {
      error = FT_Add_Driver( library, *cur );
      /* notify errors, but don't stop */
      if ( error )
        FT_ERROR(( "FT.Default_Drivers: Cannot install `%s', error = %x\n",
                   (*cur)->driver_name, error ));
      cur++;
    }
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Init_FreeType                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Initializes a new FreeType library object.  The set of drivers     */
  /*    that are registered by this function is determined at build time.  */
  /*                                                                       */
  /* <Output>                                                              */
  /*    library :: A handle to a new library object.                       */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeTyoe error code.  0 means success.                             */
  /*                                                                       */
  EXPORT_FUNC
  FT_Error  FT_Init_FreeType( FT_Library*  library )
  {
    FT_Error   error;
    FT_Memory  memory;


    /* First of all, allocate a new system object - -this function is part */
    /* of the system-specific component, i.e. `ftsystem.c'.                */

    memory = FT_New_Memory();
    if ( !memory )
    {
      FT_ERROR(( "FT_Init_FreeType:" ));
      FT_ERROR(( " cannot find memory manager" ));
      return FT_Err_Unimplemented_Feature;
    }

    /* builds a library out of it, then fill it with the set of */
    /* default drivers.                                         */

    error = FT_New_Library( memory, library );
    if ( !error )
      FT_Default_Drivers( *library );

    return error;
  }


/* END */
