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
   *   External Debugginng APIs
   *
   * @abstract:
   *   Pubic APIs to use while debugging using `FT_LOGGING' macro
   *
   * @description:
   *   This section contains the  declaration  the public  APIs which can be 
   *   used to debug an application using `FT_LOGGING'.
   *
   */


  /**************************************************************************
   *
   * @function:
   *   FT_Trace_Set_Level
   *
   * @description:
   *   To change the levels of tracing components at run time.
   *
   * @input:
   * 
   *   tracing_level ::
   *     New levels of of tracing components. 
   *
   * @example:
   *   This function can be used to change the tracing levels of  FreeType's
   *   component as follows:
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
   *   values of the tracing components of  FreeType, this function could
   *   be  used to reset the level of  tracing  components to the default
   *   value.
   *
   */
  FT_EXPORT( void )
  FT_Trace_Set_Default_Level( void );


  /**************************************************************************
   *
   * @functype:
   *   FT_Custom_Log_Handler
   *
   * @description:
   *   A function used  to  handle the logging of tracing and debug messages
   *   on a file system. 
   *
   * @input:
   *   ft_component ::
   *     The name of `FT_COMPONENT' from  which  the  current debug or error
   *     message is produced. 
   *
   *   fmt ::
   *     Actual debug or tracing message.
   * 
   *   args::
   *     Arguments of debug or tracing messages.
   *     
   */
  typedef void 
  (*FT_Custom_Log_Handler)( const char* ft_component, const char* fmt, 
                            va_list args ); 


  /**************************************************************************
   *
   * @function:
   *   FT_Set_Log_Handler
   *
   * @description:
   *   A function to set a custom log handler
   * 
   * @input:
   * 
   *   handler ::
   *     New logging function 
   *
   */
  FT_EXPORT( void )
  FT_Set_Log_Handler( FT_Custom_Log_Handler handler ); 


  /**************************************************************************
   *
   * @function:
   *   FT_Set_Default_Log_Handler
   *
   * @description:
   *   If previously, `FT_Set_Log_Handler'  functions is used  to set new
   *   custom logging function, this API could be  used to reset the back 
   *   the log handler to FreeType's inbuilt log handler.
   *
   */

  FT_EXPORT( void )
  FT_Set_Default_Log_Handler( void );
  

FT_END_HEADER

#endif /* FTLOGGING_H_ */
