#ifndef PSMODULE_H
#define PSMODULE_H

#include <freetype/ftmodule.h>

#ifdef __cplusplus
  extern "C" {
#endif

  FT_EXPORT_VAR( const FT_Module_Class )  psaux_driver_class;

#ifdef __cplusplus
  }
#endif

#endif /* PSMODULE_H */
