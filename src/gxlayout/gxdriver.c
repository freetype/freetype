/***************************************************************************/
/*                                                                         */
/*  gxdriver.c                                                             */
/*                                                                         */
/*    AAT/TrueTypeGX font driver implementation (body).                    */
/*                                                                         */
/*  Copyright 2003 by                                                      */
/*  Masatake YAMATO and Redhat K.K.                                        */
/*                                                                         */
/*  This file may only be used,                                            */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

/***************************************************************************/
/* Development of the code in this file is support of                      */
/* Information-technology Promotion Agency, Japan.                         */
/***************************************************************************/

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_INTERNAL_DEBUG_H
#include "gxdriver.h"
#include "gxobjs.h"

  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_gxdriver

  /*************************************************************************
   * gx_driver_class is not initialized statically but dynamically.
   * -----------------------------------------------------------------------
   * Almost all fields should be initialized in gxobjs.c::gx_driver_init.
   *************************************************************************/

  FT_CALLBACK_TABLE_DEF
  const FT_Driver_ClassRec  gx_driver_class =
  {
    /* FT_Module_Class */
    {
      /* module_flags, copyied from ttdriver.c */
      FT_MODULE_FONT_DRIVER        |
      FT_MODULE_DRIVER_SCALABLE    |
#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER
      FT_MODULE_DRIVER_HAS_HINTER,
#else
      0,
#endif
      /* module_size, copyied from ttdriver.c */
      sizeof (GX_DriverRec),
      /* module_name */
      "gx",
      /* module_version */
      0x10000L,
      /* module_requires */
      0x20000L,
      
      /* module_interface */
      NULL,

      /* module_init */
      (FT_Module_Constructor) gx_driver_init,
      
      /* module_done */
      (FT_Module_Destructor)  NULL,
      
      /* get_interface */
      (FT_Module_Requester)   NULL,
    },

    /* now the specific driver fields */
    0,
    0,
    0,
    
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    
    NULL,
    NULL,
    
    NULL,

    NULL,
    NULL,
    NULL,
  };

/* END */
