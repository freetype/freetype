/****************************************************************************
 *
 * ftlogging.h
 *
 *   Additional debugging APIs.
 *
 * Copyright (C) 2020 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef FTLOGGING_H_
#define FTLOGGING_H_


#include <ft2build.h>
#include FT_CONFIG_CONFIG_H


FT_BEGIN_HEADER


  /**************************************************************************
   *
   * @section:
   *   debugging_apis
   *
   * @title:
   *   External Debugging APIs
   *
   * @abstract:
   *   Public APIs to control the `FT_LOGGING` macro.
   *
   * @description:
   *   This section contains the declarations of public functions that
   *   enables fine control of what the `FT_LOGGING` macro outputs.
   *
   */


  /**************************************************************************
   *
   * @function:
   *   FT_Trace_Set_Level
   *
   * @description:
   *   Change the levels of tracing components of FreeType at run time.
   *
   * @input:
   *   tracing_level ::
   *     New tracing value.
   *
   * @example:
   *   The following call makes FreeType trace everything but the 'memory'
   *   component.
   *
   *   ```
   *   FT_Trace_Set_Level( "any:7 memory:0 );
   *   ```
   *
   * @note:
   *   This function is only available if compilation option `@FT_LOGGING`
   *   is set.
   */
  FT_EXPORT( void )
  FT_Trace_Set_Level( const char*  tracing_level );


  /**************************************************************************
   *
   * @function:
   *   FT_Trace_Set_Default_Level
   *
   * @description:
   *   Reset tracing value of FreeType's components to the default value
   *   (i.e., to the value of the `FT2_DEBUG` environment value or to NULL
   *   if `FT2_DEBUG` is not set).
   *
   *
   * @note:
   *   This function is only available if compilation option `@FT_LOGGING`
   *   is set.
   */
  FT_EXPORT( void )
  FT_Trace_Set_Default_Level( void );

  /* */


FT_END_HEADER

#endif /* FTLOGGING_H_ */


/* END */
