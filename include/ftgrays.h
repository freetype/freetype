#ifndef FTGRAYS_H
#define FTGRAYS_H

#include <ftimage.h>

  /*************************************************************************/
  /*                                                                       */
  /* To make ftgrays.h independent from configuration files we check       */
  /* whether EXPORT_DEF has been defined already.                          */
  /*                                                                       */
  /* On some systems and compilers (Win32 mostly), an extra keyword is     */
  /* necessary to compile the library as a DLL.                            */
  /*                                                                       */
#ifndef EXPORT_DEF
#define EXPORT_DEF  extern
#endif

  EXPORT_DEF  FT_Raster_Funcs  ft_grays_raster;

#endif
