#define FT_MAKE_OPTION_SINGLE_OBJECT

#include <ttload.c>
#include <ttcmap.c>

#ifdef TT_CONFIG_OPTION_EMBEDDED_BITMAPS
#include <ttsbit.c>
#endif

#ifdef TT_CONFIG_OPTION_POSTSCRIPT_NAMES
#include <ttpost.c>
#endif

#include <sfdriver.c>


