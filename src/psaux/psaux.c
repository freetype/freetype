#define FT_MAKE_OPTION_SINGLE_OBJECT

#ifdef FT_FLAT_COMPILE

#include "psobjs.c"
#include "psmodule.c"
#include "t1decode.c"

#else

#include <psaux/psobjs.c>
#include <psaux/psmodule.c>
#include <psaux/t1decode.c>

#endif

