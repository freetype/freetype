/***************************************************************************/
/*                                                                         */
/*  ftdebug.h                                                              */
/*                                                                         */
/*    Debugging and logging component (specification).                     */
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
  /* This component contains various macros and functions used to ease the */
  /* debugging of the FreeType engine.  Its main purpose is in assertion   */
  /* checking, tracing, and error detection.                               */
  /*                                                                       */
  /* There are now three debugging modes:                                  */
  /*                                                                       */
  /* - trace mode                                                          */
  /*                                                                       */
  /*   Error and trace messages are sent to the log file (which can be the */
  /*   standard error output).                                             */
  /*                                                                       */
  /* - error mode                                                          */
  /*                                                                       */
  /*   Only error messages are generated.                                  */
  /*                                                                       */
  /* - release mode:                                                       */
  /*                                                                       */
  /*   No error message is sent or generated.  The code is free from any   */
  /*   debugging parts.                                                    */
  /*                                                                       */
  /*************************************************************************/


#ifndef FTDEBUG_H
#define FTDEBUG_H

#include <ftconfig.h>   /* for FT_DEBUG_LEVEL_TRACE, FT_DEBUG_LEVEL_ERROR */


#ifdef __cplusplus
  extern "C" {
#endif


#ifdef FT_DEBUG_LEVEL_TRACE


  typedef enum  FT_Trace_
  {
    /* the first level must always be `trace_any' */
    trace_any = 0,

    /* first, define an enum for each common component */
    trace_io,        /* in ftsys */
    trace_memory,    /* in ftsys */
    trace_sync,      /* in ftsys */
    trace_stream,    /* stream manager - see ftstream.c */
    trace_calc,      /* computations   - see ftcalc.c   */
    trace_raster,    /* raster         - see ftraster.c */
    trace_list,      /* list manager   - see ftlist.c   */
    trace_objs,      /* base objects   - see ftobjs.c   */

    /* then define an enum for each TrueType driver component */
    trace_ttobjs,
    trace_ttload,
    trace_ttgload,
    trace_ttinterp,
    trace_ttcmap,
    trace_ttextend,
    trace_ttdriver,

#if 0
    /* define an enum for each TrueDoc driver component */
    trace_tdobjs,
    trace_tdload,
    trace_tdgload,
    trace_tdhint,
    trace_tddriver,
#endif

    /* define an enum for each Type1 driver component */
    trace_t1objs,
    trace_t1load,
    trace_t1gload,
    trace_t1hint,
    trace_t1driver,

    /* other trace levels */
    trace_init,

    /* the last level must always be `trace_max' */
    trace_max

  } FT_Trace;

  /* declared in ftdebug.c */
  extern char  ft_trace_levels[trace_max];


  /*************************************************************************/
  /*                                                                       */
  /* IMPORTANT!                                                            */
  /*                                                                       */
  /* Each component must define the macro FT_COMPONENT to a valid          */
  /* Trace_Component value before using any TRACE macro.                   */
  /*                                                                       */
  /*************************************************************************/


#define FT_TRACE( level, varformat )                        \
          do                                                \
          {                                                 \
            if ( ft_trace_levels[FT_COMPONENT] >= level )   \
              FT_Message##varformat;                        \
          } while ( 0 )


  EXPORT_DEF
  void  FT_SetTraceLevel( FT_Trace  component,
                          char      level );


#elif defined( FT_DEBUG_LEVEL_ERROR )


#define FT_TRACE( level, varformat )    while ( 0 ) { }     /* nothing */


#else  /* release mode */


#define FT_Assert( condition )          while ( 0 ) { }     /* nothing */

#define FT_TRACE( level, varformat )    while ( 0 ) { }     /* nothing */
#define FT_ERROR( varformat )           while ( 0 ) { }     /* nothing */


#endif /* FT_DEBUG_LEVEL_TRACE, FT_DEBUG_LEVEL_ERROR */


  /*************************************************************************/
  /*                                                                       */
  /* Define macros and functions that are common to the debug and trace    */
  /* modes.                                                                */
  /*                                                                       */
  /* You need vprintf() to be able to compile ftdebug.c.                   */
  /*                                                                       */
  /*************************************************************************/

#if defined( FT_DEBUG_LEVEL_TRACE ) || defined( FT_DEBUG_LEVEL_ERROR )


#include "stdio.h"  /* for vprintf() */

#define FT_Assert( condition )                                      \
          do                                                        \
          {                                                         \
            if ( !( condition ) )                                   \
              FT_Panic( "assertion failed on line %d of file %s\n", \
                        __LINE__, __FILE__ );                       \
          } while ( 0 )

  /* print a message */
  extern void  FT_Message( const char*  fmt, ... );

  /* print a message and exit */
  extern void  FT_Panic  ( const char*  fmt, ... );

#define FT_ERROR( varformat )  FT_Message##varformat


#endif /* FT_DEBUG_LEVEL_TRACE || FT_DEBUG_LEVEL_ERROR */


/* you need two opening resp. closing parentheses!
   Example: FT_TRACE0(( "Value is %i", foo ))      */

#define FT_TRACE0( varformat )  FT_TRACE( 0, varformat )
#define FT_TRACE1( varformat )  FT_TRACE( 1, varformat )
#define FT_TRACE2( varformat )  FT_TRACE( 2, varformat )
#define FT_TRACE3( varformat )  FT_TRACE( 3, varformat )
#define FT_TRACE4( varformat )  FT_TRACE( 4, varformat )
#define FT_TRACE5( varformat )  FT_TRACE( 5, varformat )
#define FT_TRACE6( varformat )  FT_TRACE( 6, varformat )
#define FT_TRACE7( varformat )  FT_TRACE( 7, varformat )


#ifdef __cplusplus
  }
#endif


#endif /* FTDEBUG_H */


/* END */
