/*
 *  This is a cheap replacement for getopt() because that routine is not
 *  available on some platforms and behaves differently on other platforms.
 *  This code was written from scratch without looking at any other
 *  implementation.
 *
 *  This code is hereby expressly placed in the public domain.
 *  mleisher@crl.nmsu.edu (Mark Leisher)
 *  10 October 1997
 */

#ifndef lint
#ifdef __GNUC__
  static char rcsid[] __attribute__ ((unused)) = "$Id$";
#else
  static char rcsid[] = "$Id$";
#endif
#endif

#include "common.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

  /*
   *  Externals visible to programs.
   */

  int    opterr = 1;
  int    optind = 1;
  char*  optarg;

  /*
   *  Internal variables that are used to detect when the global values
   *  need to be reset.
   */

  static int  cmdac;
#ifdef __STDC__
  static const char*   cmdname;
  static char* const*  cmdav;
#else
  static char*   cmdname;
  static char**  cmdav;
#endif

  int
#ifdef __STDC__
  getopt( int  ac, char* const*  av, const char*  pat )
#else
  getopt( ac, av, pat )
    int     ac;
    char**  av;
    char*   pat;
#endif
  {
    int  opt;
#ifdef __STDC__
    const char*  p;
    const char*  pp;
#else
    char*  p;
    char*  pp;
#endif

    /*
     *  If there is no pattern, indicate the parsing is done.
     */
    if ( pat == 0 || *pat == 0 )
      return -1;

    /*
     *  Always reset the option argument to NULL.
     */
    optarg = 0;

    /*
     *  If the number of arguments or argument list do not match the last
     *  values seen, reset the internal pointers and the globals.
     */
    if ( ac != cmdac || av != cmdav )
    {
      optind = 1;
      cmdac = ac;
      cmdav = av;

      /*
       *  Determine the command name in case it is needed for warning
       *  messages.
       */
      for ( cmdname = 0, p = av[0]; *p; p++ )
      {
        if ( *p == '/' || *p == '\\' )
          cmdname = p;
      }
      /*
       *  Skip the path separator if the name was assigned.
       */
      if ( cmdname )
        cmdname++;
      else
        cmdname = av[0];
    }

    /*
     *  If the next index is greater than or equal to the number of
     *  arguments, then the command line is done.
     */
    if ( optind >= ac )
      return -1;

    /*
     *  Test the next argument for one of three cases:
     *    1. The next argument does not have an initial '-'.
     *    2. The next argument is '-'.
     *    3. The next argument is '--'.
     *
     *  In either of these cases, command line processing is done.
     */
    if ( av[optind][0] != '-'            ||
         strcmp( av[optind], "-" ) == 0  ||
         strcmp( av[optind], "--" ) == 0 )
      return -1;

    /*
     *  Point at the next command line argument and increment the
     *  command line index.
     */
    p = av[optind++];

    /*
     *  Look for the first character of the command line option.
     */
    for ( opt = *(p + 1), pp = pat; *pp && *pp != opt; pp++ )
      ;

    /*
     *  If nothing in the pattern was recognized, then issue a warning
     *  and return a '?'.
     */
    if ( *pp == 0 )
    {
      if ( opterr )
        fprintf( stderr, "%s: illegal option -- %c\n", cmdname, opt );
      return '?';
    }

    /*
     *  If the option expects an argument, get it.
     */
    if ( *(pp + 1) == ':' && (optarg = av[optind]) == 0 )
    {
      /*
       *  If the option argument is NULL, issue a warning and return a '?'.
       */
      if ( opterr )
        fprintf( stderr, "%s: option requires an argument -- %c\n",
                         cmdname, opt );
      opt = '?';
    }
    else if ( optarg )
    /*
     *  Increment the option index past the argument.
     */
      optind++;

    /*
     *  Return the option character.
     */
    return opt;
  }


/****************************************************************************/
/*                                                                          */
/*  The FreeType project -- a free and portable quality TrueType renderer.  */
/*                                                                          */
/*  Copyright 1996-1998 by                                                  */
/*  D. Turner, R.Wilhelm, and W. Lemberg                                    */
/*                                                                          */
/* ft_basename():                                                           */
/*                                                                          */
/* a stupid but useful function...                                          */
/*                                                                          */
/* rewritten by DavidT to get rid of GPLed programs in the FreeType demos.  */
/*                                                                          */
/****************************************************************************/

  char*
#ifdef __STDC__
  ft_basename ( const char*  name )
#else
  ft_basename ( name )
    char* name;
#endif
  {
#ifdef __STDC__
    const char*  base;
    const char*  current;
#else
    char*        base;
    char*        current;
#endif
    char         c;

    base    = name;
    current = name;

    c = *current;
  
    while ( c )
    {
#ifndef macintosh
      if ( c == '/' || c == '\\' )
#else
      if ( c == ':' )
#endif
        base = current + 1;

      current++;
      c = *current;
    }

    return (char*)base;
  }


#ifdef __STDC__
  void Panic( const char*  fmt, ... )
#else
  void Panic( fmt )
    const char* fmt;
#endif
  {
    va_list  ap;


    va_start( ap, fmt );
    vprintf( fmt, ap );
    va_end( ap );

    exit( 1 );
  }


/* End */
