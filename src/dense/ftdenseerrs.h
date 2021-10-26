
#ifndef FTDENSEERRS_H_
#define FTDENSEERRS_H_

#include <freetype/ftmoderr.h>

#undef FTERRORS_H_

#undef FT_ERR_PREFIX
#define FT_ERR_PREFIX Dense_Err_
#define FT_ERR_BASE   FT_Mod_Err_Dense

#include <freetype/fterrors.h>

#endif /* FTDENSEERRS_H_ */

/* END */
