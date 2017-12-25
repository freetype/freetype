/***************************************************************************/
/*                                                                         */
/*  ftpsprop.h                                                             */
/*                                                                         */
/*    Get and set properties of PostScript drivers (specification).        */
/*                                                                         */
/***************************************************************************/


#ifndef FTPSPROP_H_
#define FTPSPROP_H_


#include <ft2build.h>
#include FT_FREETYPE_H


FT_BEGIN_HEADER


  FT_Error
  ps_property_set( FT_Module    module,         /* PS_Driver */
                   const char*  property_name,
                   const void*  value,
                   FT_Bool      value_is_string );

  FT_Error
  ps_property_get( FT_Module    module,         /* PS_Driver */
                   const char*  property_name,
                   const void*  value );


FT_END_HEADER


#endif /* FTPSPROP_H_ */


/* END */
