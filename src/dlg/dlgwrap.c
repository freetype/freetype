#include <freetype/freetype.h>


#ifdef FT_LOGGING
      #include "dlg.c"
#else
    /* ANSI C doesn't like empty source files */
    typedef int  _dlg_dummy;
#endif
