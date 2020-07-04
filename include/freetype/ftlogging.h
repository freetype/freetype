#include FT_CONFIG_CONFIG_H

FT_BEGIN_HEADER

 /***************************************************************************
  * 
  * If FT_LOGGING is enabled, user can change the trace level at run time 
  * using the function `FT_Trace_Set_Level()` by passing the desired trace
  * level as an argument.
  * User can also set the default trace level which is supplied by 
  * environment variable `FT2_DEBUG`
  * See ftdebug.c for definitions
  * 
  */ 

  FT_EXPORT( void )
  FT_Trace_Set_Level( const char* level );

  FT_EXPORT( void )
  FT_Trace_Set_Default_Level( void );


  /* An external callback function to be used to define an output handler */
  typedef void 
  (*ft_custom_log_handler)( const char* fmt, va_list args ); 


  /**************************************************************************
   * 
   * If FT_LOGGING is enabled user can provide their own function to handle 
   * the log messages using the function `FT_Set_Log_Handler()` by passing 
   * the function name which they want to use.
   * User could also revert back to use FreeType's inbuilt function to 
   * handle logs using function `FT_Set_Default_Log_Handler()` 
   * Defined in src/base/ftdebug.c
   *
   */ 
  FT_EXPORT( void )
  FT_Set_Log_Handler( ft_custom_log_handler handler ); 

  FT_EXPORT( void )
  FT_Set_Default_Log_Handler();
  

FT_END_HEADER
