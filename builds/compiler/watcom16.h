#include "16bit.h"

/*
 * Watcom C compiler for 16bit cannot parse a command line option
 * to set preprocessor macro with a value including a dot.
 * it is misunderstood as a filename.
 */
#define FT_CONFIG_MODULES_H     "ftmodule.h"

#undef TT_CONFIG_OPTION_GX_VAR_SUPPORT
