/***************************************************************************/
/*                                                                         */
/*  ftinit.c                                                               */
/*                                                                         */
/*    FreeType initialisation layer (body).                                */
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

 /**************************************************************************
  *
  *  The purpose of this file is to implement the three following
  *  functions:
  *
  *	 FT_Default_Drivers:
  *		This function is used to add the set of default drivers
  *		to a fresh new library object. The set is computed at compile
  *		time from the Makefiles inclusions in Makefile.lib. See the
  *		document "FreeType Internals" for more info.
  *
  *
  *	 FT_Init_FreeType:
  *		This function creates a system object for the current platform,
  *		builds a library out of it, then calls FT_Default_Drivers
  *
  *
  *	 FT_Done_FreeType:
  *		This function simply finalise the library and the corresponding
  *		system object.
  *
  *
  *	 Note that even if FT_Init_FreeType uses the implementation of the
  *	 system object defined at build time, client applications are still
  *	 able to provide their own "ftsystem.c"
  *
  *
  *
  *
  *
  ************************************************************************/

#include <ftobjs.h>
#include <ftdriver.h>
#include <ftconfig.h>
#include <ftdebug.h>

#undef  FT_COMPONENT
#define FT_COMPONENT  trace_init

  /*************************************************************************/
  /*                                                                       */
  /* The macros FT_SUPPORT_xxxx are defined by Makefile.lib when this file */
  /* is compiled.  They come from a make variable called FTINIT_MACROS     */
  /* which is updated by each driver Makefile.                             */
  /*                                                                       */
  /* This means that when a driver isn't part of the build, ftinit.o       */
  /* won't try to reference it.                                            */
  /*                                                                       */
  /*************************************************************************/

#define  FTINIT_DRIVER_CHAIN
#define  FT_INIT_LAST_DRIVER_CHAIN    ((FT_DriverChain*) 0)

  /* Include the SFNT driver interface if needed */

#ifdef FT_SUPPORT_SFNT
#include "sfdriver.h"
#endif

  /* Include the TrueType driver interface if needed */

#ifdef FT_SUPPORT_TRUETYPE
#include "ttdriver.h"
#endif


  /* Include the Type1 driver interface if needed */

#ifdef FT_SUPPORT_TYPE1
#include "t1driver.h"
#endif



  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Default_Drivers                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Adds the set of default drivers to a given library object.         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library :: A handle to a new library object.                       */
  /*                                                                       */
  EXPORT_FUNC
  void  FT_Default_Drivers( FT_Library  library )
  {
    FT_Error               error;
    const FT_DriverChain*  chain = FT_INIT_LAST_DRIVER_CHAIN;

    while (chain)
    {
      error = FT_Add_Driver( library, chain->interface );

      /* notify errors, but don't stop */
      if (error)
      {
        FT_ERROR(( "FT.Default_Drivers: cannot install `%s', error = %x\n",
                   chain->interface->driver_name,
                   error ));
      }

      chain = chain->next;
      error = 0;  /* clear error */
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
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  EXPORT_FUNC
  FT_Error  FT_Init_FreeType( FT_Library*  library )
  {
    FT_Error   error;
    FT_Memory  memory;

    /* First of all, allocate a new system object -this function is part */
    /* of the system-specific component, i.e. ftsystem.c                 */
    memory = FT_New_Memory();
    if (!memory)
    {
      FT_ERROR(( "FT_Init_FreeType:" ));
      FT_ERROR(( " cannot find memory manager" ));
      return FT_Err_Unimplemented_Feature;
    }
    
    /* builds a library out of it, then fill it with the set of */
    /* default drivers..										*/
    error = FT_New_Library( memory, library );
    if ( !error )
      FT_Default_Drivers(*library);
    
    return error;
  }


/* END */
