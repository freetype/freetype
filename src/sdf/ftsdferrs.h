
#ifndef FTSDFERRS_H_
#define FTSDFERRS_H_

#include <freetype/ftmoderr.h>

#undef FTERRORS_H_

#undef  FT_ERR_PREFIX
#define FT_ERR_PREFIX  Sdf_Err_
#define FT_ERR_BASE    FT_Mod_Err_Sdf

#include <freetype/fterrors.h>

#endif /* FTSDFERRS_H_ */

/* END */
