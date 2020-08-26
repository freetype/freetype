/****************************************************************************
 *
 * ftlogging.h
 *
 *   Additional debugging APIs.
 *
 * Copyright (C) 2008-2020 by
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
   *   Pubic APIs to use while debugging using `FT_LOGGING' macro
   *
   * @description:
   *   This section contains the declaration of the public  APIs which can be
   *   used to debug an application using `FT_LOGGING'.
   *
   */


  /**************************************************************************
   *
   * @function:
   *   FT_Trace_Set_Level
   *
   * @description:
   *   To change the levels of tracing components of FreeType, at run time.
   *
   * @input:
   *
   *   tracing_level ::
   *     New tracing values of FreeType's components.
   *
   * @example:
   *   This function can be used to change the levels of tracing components
   *   of FreeType as follows:
   *
   *   ```
   *     new_levels  = "any:7 memory:0";
   *     FT_Trace_Set_Level( new_levels );
   *   ```
   */
  FT_EXPORT( void )
  FT_Trace_Set_Level( const char* tracing_level );


  /**************************************************************************
   *
   * @function:
   *   FT_Trace_Set_Default_Level
   *
   * @description:
   *   If previously, `FT_Trace_Set_Level'  functions is used  to set new
   *   tracing values of FreeType components, this function could be  used to
   *   reset the tracing values of FreeType's components to the default value.
   *
   */
  FT_EXPORT( void )
  FT_Trace_Set_Default_Level( void );

  /* */

FT_END_HEADER

#endif /* FTLOGGING_H_ */
