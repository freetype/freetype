
#ifndef FTSDFBERRS_H_
#define FTSDFBERRS_H_

#include <freetype/ftmoderr.h>

#undef FTERRORS_H_

#undef  FT_ERR_PREFIX
#define FT_ERR_PREFIX  Sdfb_Err_
#define FT_ERR_BASE    FT_Mod_Err_Sdfb

#include <freetype/fterrors.h>

#endif /* FTSDFBERRS_H_ */


/* END */
