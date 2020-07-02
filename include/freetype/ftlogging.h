#include FT_CONFIG_CONFIG_H

FT_BEGIN_HEADER

 /***************************************************************************
  * 
  * If FT_LOGGING is enabled, user can change the trace level at run time 
  * using the function `FT_Trace_Set_Level()` by passing the desired trace
  * level as an argument.
  * User can also set the default trace level which is supplied by 
  * environment variable `FT2_DEBUG`
  * 
  */ 

  typedef void
  (*ft_ouput_handler)( const char* string ); 

  FT_EXPORT( void )
  FT_Trace_Set_Level( const char* level );

  FT_EXPORT( void )
  FT_Trace_Set_Default_Level( void );

  FT_EXPORT( void )
  FT_Trace_Set_Output( ft_ouput_handler handler ); 

  FT_EXPORT( void )
  FT_Trace_Set_Default_Output();

FT_END_HEADER
