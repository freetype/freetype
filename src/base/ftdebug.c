/***************************************************************************/
/*                                                                         */
/*  ftdebug.c                                                              */
/*                                                                         */
/*    Debugging and logging component (body).                              */
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


#include <ftdebug.h>

#ifdef FT_DEBUG_LEVEL_TRACE
  char  ft_trace_levels[trace_max];
#endif


#if defined( FT_DEBUG_LEVEL_ERROR ) || defined( FT_DEBUG_LEVEL_TRACE )


#include <stdarg.h>
#include <stdlib.h>
#include <string.h>


  void  FT_Message( const char*  fmt, ... )
  {
    va_list  ap;


    va_start( ap, fmt );
    vprintf( fmt, ap );
    va_end( ap );
  }


  void  FT_Panic( const char*  fmt, ... )
  {
    va_list  ap;


    va_start( ap, fmt );
    vprintf( fmt, ap );
    va_end( ap );

    exit( EXIT_FAILURE );
  }


#ifdef FT_DEBUG_LEVEL_TRACE


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_SetTraceLevel                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Sets the trace level for debugging.                                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    component :: The component which should be traced.  See ftdebug.h  */
  /*                 for a complete list.  If set to `trace_any', all      */
  /*                 components will be traced.                            */
  /*    level     :: The tracing level.                                    */
  /*                                                                       */
  EXPORT_FUNC
  void  FT_SetTraceLevel( FT_Trace  component,
                          char      level )
  {
    if ( component >= trace_max )
      return;

    /* if component is `trace_any', then change _all_ levels at once */
    if ( component == trace_any )
    {
      int  n;


      for ( n = trace_any; n < trace_max; n++ )
        ft_trace_levels[n] = level;
    }
    else        /* otherwise, only change individual component */
      ft_trace_levels[component] = level;
  }

#endif /* FT_DEBUG_LEVEL_TRACE */

#endif /* FT_DEBUG_LEVEL_TRACE || FT_DEBUG_LEVEL_ERROR */


/* END */
