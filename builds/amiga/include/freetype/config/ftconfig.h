// TetiSoft: We must change FT_BASE_DEF and FT_EXPORT_DEF

//#define FT_BASE_DEF( x )  extern  x	// SAS/C wouldn't generate an XDEF
//#define FT_EXPORT_DEF( x )  extern  x	// SAS/C wouldn't generate an XDEF
#undef FT_BASE_DEF
#define FT_BASE_DEF( x )  x
#undef FT_EXPORT_DEF
#define FT_EXPORT_DEF( x )  x

// TetiSoft: now include original file
#include "FT:include/freetype/config/ftconfig.h"
