/****************************************************************************
 *
 * ftdebug.c
 *
 *   Debugging and logging component (body).
 *
 * Copyright (C) 1996-2020 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


  /**************************************************************************
   *
   * This component contains various macros and functions used to ease the
   * debugging of the FreeType engine.  Its main purpose is in assertion
   * checking, tracing, and error detection.
   *
   * There are now three debugging modes:
   *
   * - trace mode
   *
   *   Error and trace messages are sent to the log file (which can be the
   *   standard error output).
   *
   * - error mode
   *
   *   Only error messages are generated.
   *
   * - release mode:
   *
   *   No error message is sent or generated.  The code is free from any
   *   debugging parts.
   *
   */


#include <freetype/freetype.h>
#include <freetype/internal/ftdebug.h>

#ifdef FT_LOGGING

  /**************************************************************************
   * 
   * Variable used when FT_LOGGING is enabled to control logging:
   * 
   * 1. ft_default_trace_level: stores the value of trace levels which are 
   *    provided to FreeType using FT2_DEBUG environment variable.
   * 
   * 2. ft_custom_trace_level: stores the value of custom trace level which 
   *    is provided by user at run-time.
   * 
   * 3. ft_component: a string that holds the name of FT_COMPONENT
   * 
   * 4. ft_fileptr: store the FILE*
   * 
   * 5. ft_component_flag: a flag when is true, prints FT_COMPONENT along 
   *    with log message if `-v` is defined in FT2_DEBUG
   * 6. ft_timestamp_flag: a flag when is true, prints time in millisec along
   *    with log message if `-t` is define in FT2_DEBUG
   * 7. ft_have_newline_char: It is used to differentiate between a log 
   *    message with '\n' char and log message without '\n' char  
   * 
   * Static Variables are defined here to remove [ -Wunused-variable ] 
   * warning
   *     
   */ 
  static const char* ft_default_trace_level = NULL;
  static const char* ft_custom_trace_level = NULL;
  static const char* ft_component = NULL;
  static FILE* ft_fileptr = NULL;
  static bool ft_component_flag = false;
  static bool ft_timestamp_flag = false;
  static bool ft_have_newline_char = true;

  ft_custom_log_handler custom_output_handler = NULL;
  dlg_handler ft_default_log_handler = NULL;

  /* different types of dlg features to be used according to the flags     */
  /* passed in FT2_DEBUG environment variable                              */

  static unsigned int features_component = dlg_output_threadsafe 
                                          | dlg_output_tags;
  static unsigned int features_timestamp = dlg_output_threadsafe 
                                          | dlg_output_time
                                          | dlg_output_time_msecs; 
  static unsigned int features_both = dlg_output_threadsafe 
                                      | dlg_output_time_msecs 
                                      |dlg_output_time
                                      | dlg_output_tags ;
  static unsigned int features_none = dlg_output_threadsafe;



#endif


#ifdef FT_DEBUG_LEVEL_ERROR

  /* documentation is in ftdebug.h */

  FT_BASE_DEF( void )
  FT_Message( const char*  fmt,
              ... )
  {
    va_list  ap;


    va_start( ap, fmt );
    vfprintf( stderr, fmt, ap );
    va_end( ap );
  }


  /* documentation is in ftdebug.h */

  FT_BASE_DEF( void )
  FT_Panic( const char*  fmt,
            ... )
  {
    va_list  ap;


    va_start( ap, fmt );
    vfprintf( stderr, fmt, ap );
    va_end( ap );

    exit( EXIT_FAILURE );
  }


  /* documentation is in ftdebug.h */

  FT_BASE_DEF( int )
  FT_Throw( FT_Error     error,
            int          line,
            const char*  file )
  {
#if 0
    /* activating the code in this block makes FreeType very chatty */
    fprintf( stderr,
             "%s:%d: error 0x%02x: %s\n",
             file,
             line,
             error,
             FT_Error_String( error ) );
#else
    FT_UNUSED( error );
    FT_UNUSED( line );
    FT_UNUSED( file );
#endif

    return 0;
  }

#endif /* FT_DEBUG_LEVEL_ERROR */



#ifdef FT_DEBUG_LEVEL_TRACE

  /* array of trace levels, initialized to 0; */
  /* this gets adjusted at run-time           */
  static int  ft_trace_levels_enabled[trace_count];

  /* array of trace levels, always initialized to 0 */
  static int  ft_trace_levels_disabled[trace_count];

  /* a pointer to either `ft_trace_levels_enabled' */
  /* or `ft_trace_levels_disabled'                 */
  int*  ft_trace_levels;

  /* define array of trace toggle names */
#define FT_TRACE_DEF( x )  #x ,

  static const char*  ft_trace_toggles[trace_count + 1] =
  {
#include <freetype/internal/fttrace.h>
    NULL
  };

#undef FT_TRACE_DEF


  /* documentation is in ftdebug.h */

  FT_BASE_DEF( FT_Int )
  FT_Trace_Get_Count( void )
  {
    return trace_count;
  }


  /* documentation is in ftdebug.h */

  FT_BASE_DEF( const char * )
  FT_Trace_Get_Name( FT_Int  idx )
  {
    int  max = FT_Trace_Get_Count();


    if ( idx < max )
      return ft_trace_toggles[idx];
    else
      return NULL;
  }


  /* documentation is in ftdebug.h */

  FT_BASE_DEF( void )
  FT_Trace_Disable( void )
  {
    ft_trace_levels = ft_trace_levels_disabled;
  }


  /* documentation is in ftdebug.h */

  FT_BASE_DEF( void )
  FT_Trace_Enable( void )
  {
    ft_trace_levels = ft_trace_levels_enabled;
  }


  /**************************************************************************
   *
   * Initialize the tracing sub-system.  This is done by retrieving the
   * value of the `FT2_DEBUG' environment variable.  It must be a list of
   * toggles, separated by spaces, `;', or `,'.  Example:
   *
   *   export FT2_DEBUG="any:3 memory:7 stream:5"
   *
   * This requests that all levels be set to 3, except the trace level for
   * the memory and stream components which are set to 7 and 5,
   * respectively.
   *
   * See the file `include/freetype/internal/fttrace.h' for details of
   * the available toggle names.
   *
   * The level must be between 0 and 7; 0 means quiet (except for serious
   * runtime errors), and 7 means _very_ verbose.
   */
  FT_BASE_DEF( void )
  ft_debug_init( void )
  {
    const char*  ft2_debug = NULL;

#ifdef FT_LOGGING

if( ft_custom_trace_level != NULL )
    ft2_debug = ft_custom_trace_level;
else  
    ft2_debug = ft_default_trace_level;

#else
    ft2_debug = ft_getenv( "FT2_DEBUG" );
#endif /* FT_LOGGIGN */


    if ( ft2_debug )
    {
      const char*  p = ft2_debug;
      const char*  q;


      for ( ; *p; p++ )
      {
       
        /* skip leading whitespace and separators */
        if ( *p == ' ' || *p == '\t' || *p == ',' || *p == ';' || *p == '=' )
          continue;

#ifdef FT_LOGGING
        /* check extra arguments for logging */
        if( *p == '-' )
        {
          const char* r = ++p;
          if( *r == 'v' )
          {
            ft_component_flag = true;
            const char* s = ++r;
            if( *s == 't' )
            {
              ft_timestamp_flag = true;
              p++;
            }
            p++;
          }
          else if( *r == 't' )
          {
            ft_timestamp_flag = true;
            const char* s = ++r;
            if( *s == 'v' )
            {
              ft_component_flag = true;
              p++;
            }
            p++;
          }
        }
#endif 
        /* read toggle name, followed by ':' */
        q = p;
        while ( *p && *p != ':' )
          p++;

        if ( !*p )
          break;

        if ( *p == ':' && p > q )
        {
          FT_Int  n, i, len = (FT_Int)( p - q );
          FT_Int  level = -1, found = -1;


          for ( n = 0; n < trace_count; n++ )
          {
            const char*  toggle = ft_trace_toggles[n];


            for ( i = 0; i < len; i++ )
            {
              if ( toggle[i] != q[i] )
                break;
            }

            if ( i == len && toggle[i] == 0 )
            {
              found = n;
              break;
            }
          }

          /* read level */
          p++;
          if ( *p )
          {
            level = *p - '0';
            if ( level < 0 || level > 7 )
              level = -1;
          }

          if ( found >= 0 && level >= 0 )
          {
            if ( found == trace_any )
            {
              /* special case for `any' */
              for ( n = 0; n < trace_count; n++ )
                ft_trace_levels_enabled[n] = level;
            }
            else
              ft_trace_levels_enabled[found] = level;
          }
        }
      }
    }

    ft_trace_levels = ft_trace_levels_enabled;
  }


#else  /* !FT_DEBUG_LEVEL_TRACE */


  FT_BASE_DEF( void )
  ft_debug_init( void )
  {
    /* nothing */
  }


  FT_BASE_DEF( FT_Int )
  FT_Trace_Get_Count( void )
  {
    return 0;
  }


  FT_BASE_DEF( const char * )
  FT_Trace_Get_Name( FT_Int  idx )
  {
    FT_UNUSED( idx );

    return NULL;
  }


  FT_BASE_DEF( void )
  FT_Trace_Disable( void )
  {
    /* nothing */
  }


  /* documentation is in ftdebug.h */

  FT_BASE_DEF( void )
  FT_Trace_Enable( void )
  {
    /* nothing */
  }


#endif /* !FT_DEBUG_LEVEL_TRACE */

#ifdef FT_LOGGING

  /**************************************************************************
   * 
   * If FT_LOGGING is enabled, FreeType needs to initialize all logging 
   * variables to write logs. 
   * Therefore it uses `ft_logging_init()` function to initialize a 
   * loggging variables and `ft_logging_deinit()` to un-initialize the 
   * logging variables.
   * 
   */
  
  FT_BASE_DEF( void )
  ft_logging_init( void )
  {
    ft_default_log_handler = ft_log_handler;
    ft_default_trace_level = ft_getenv( "FT2_DEBUG" );
    ft_fileptr = fopen( "freetype2.log", "w" );
    
    ft_debug_init();
    /* We need to set the default FreeType specific dlg's output handler */
    dlg_set_handler( ft_default_log_handler, NULL );

  }

  FT_BASE_DEF( void )
  ft_logging_deinit( void )
  {
    fclose( ft_fileptr );
  }

  /*************************************************************************
   * 
   * TODO:
   * 1. Add support for priniting FT_COMPONENT
   *    
   */
  FT_BASE_DEF( void ) 
  ft_log_handler( const struct dlg_origin* origin, 
                              const char* string, void* data )
  {
     static unsigned int features ;
     if( ft_timestamp_flag && ft_component_flag && ft_have_newline_char )
      features = features_both;
     else if( ft_component_flag && ft_have_newline_char)
      features = features_component;
     else if( ft_timestamp_flag && ft_have_newline_char )
      features = features_timestamp;
     else
      features = features_none;

      
	   dlg_generic_output_stream( ft_fileptr, features, origin, string, 
                                dlg_default_output_styles );

     if( strchr( string, '\n' ) )
       ft_have_newline_char = true;   
     else
       ft_have_newline_char = false;  
  }

 /**************************************************************************
  * 
  * Functions to set trace levels at run-time
  * 1. FT_Trace_Set_Level() - Use this function to change the trace level 
  *    at run-time. e.g. FT_Trace_Set_Level( "any:6 io:2" );
  * 
  * 2. FT_Trace_Set_Default_Level() - Use this function to set the default 
  *    value of trace levels which is provided by environment variable 
  *    FT2_DEBUG
  * 
  */
 
  FT_EXPORT_DEF( void )
  FT_Trace_Set_Level( const char* level )
  {  
    ft_custom_trace_level = level;
    ft_debug_init();
  }

  FT_EXPORT_DEF( void )
  FT_Trace_Set_Default_Level( void )
  {
    ft_custom_trace_level = NULL ;
    ft_debug_init();
  }

/****************************************************************************
 * 
 * Functions to handle custom log handler:
 * 
 */
  
  FT_EXPORT_DEF( void )
  FT_Set_Log_Handler( ft_custom_log_handler handler )
  {
    custom_output_handler = handler;
  }
  
  FT_EXPORT_DEF( void )
  FT_Set_Default_Log_Handler()
  {
    custom_output_handler = NULL;
  }

  FT_BASE_DEF( void )
  FT_Callback( const char* fmt, ... )
  {
    va_list ap;
    va_start( ap, fmt );
    custom_output_handler( ft_component , fmt, ap ); 
    va_end( ap );
  }

  FT_BASE_DEF( void )
  ft_add_tag( const char* tag )
  {
    ft_component = tag;
    dlg_add_tag( tag, NULL );
  }

  FT_BASE_DEF( void )
  ft_remove_tag( const char* tag )
  {
    dlg_remove_tag( tag, NULL );
  }
  
#else /* ! FT_LOGGING */

  FT_BASE_DEF( void )
  ft_add_tag( const char* tag )
  {
    (void) tag;
    /* nothing */
  }

  FT_BASE_DEF( void )
  ft_remove_tag( const char* tag )
  {
    (void) tag;
    /* nothing */
  }
 
#endif /* FT_LOGGING */

/* END */
