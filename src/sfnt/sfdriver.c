#include <sfdriver.h>
#include <ttload.h>
#include <ttsbit.h>
#include <ttpost.h>
#include <ttcmap.h>
#include <sfnt.h>

  static const SFNT_Interface  sfnt_interface =
  {
    TT_Goto_Table,
    
    TT_Load_Any,
    TT_Load_Format_Tag,
    TT_Load_Directory,

    TT_Load_Header,
    TT_Load_Metrics_Header,
    TT_Load_CMap,
    TT_Load_MaxProfile,
    TT_Load_OS2,
    TT_Load_PostScript,

    TT_Load_Names,
    TT_Free_Names,

    TT_Load_Hdmx,
    TT_Free_Hdmx,

    TT_Load_Kern,
    TT_Load_Gasp,

#ifdef TT_CONFIG_OPTION_EMBEDDED_BITMAPS
    /* see `ttsbit.h' */
    TT_Load_SBit_Strikes,
    TT_Load_SBit_Image,
    TT_Free_SBit_Strikes,
#else
    0,
    0,
    0,
#endif  
    
    /* see `ttpost.h' */
#ifdef TT_CONFIG_OPTION_POSTSCRIPT_NAMES 
    TT_Get_PS_Name,
    TT_Free_Post_Names,    
#else
    0,
    0,
#endif 

    /* see `ttcmap.h' */
    TT_CharMap_Load,
    TT_CharMap_Free,
  };
  

  EXPORT_FUNC
  const FT_DriverInterface  sfnt_driver_interface =
  {
    sizeof(FT_DriverRec),
    0,
    0,
    0,

    "sfnt",     /* driver name                         */
    1,          /* driver version                      */
    2,          /* driver requires FreeType 2 or above */

    (void*)&sfnt_interface
  };

