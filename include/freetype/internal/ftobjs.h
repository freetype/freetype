/***************************************************************************/
/*                                                                         */
/*  ftobjs.h                                                               */
/*                                                                         */
/*  The FreeType private base classes (specification).                     */
/*                                                                         */
/*  Copyright 1996-2000 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg                       */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used        */
/*  modified and distributed under the terms of the FreeType project       */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /*  This file contains the definition of all internal FreeType classes.  */
  /*                                                                       */
  /*************************************************************************/

#ifndef FTOBJS_H
#define FTOBJS_H

#include <freetype/internal/ftmemory.h>
#include <freetype/internal/ftdriver.h>

  /*************************************************************************/
  /*                                                                       */
  /* Some generic definitions.                                             */
  /*                                                                       */
#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE  0
#endif

#ifndef NULL
#define NULL  (void*)0
#endif

#ifndef UNUSED
#define UNUSED( arg )  ( (arg)=(arg) )
#endif


  /*************************************************************************/
  /*                                                                       */
  /* The min and max functions missing in C.  As usual, be careful not to  */
  /* write things like MIN( a++, b++ ) to avoid side effects.              */
  /*                                                                       */
#ifndef MIN
#define MIN( a, b )  ( (a) < (b) ? (a) : (b) )
#endif

#ifndef MAX
#define MAX( a, b )  ( (a) > (b) ? (a) : (b) )
#endif

#ifndef ABS
#define ABS( a )     ( (a) < 0 ? -(a) : (a) )
#endif



  FT_EXPORT_DEF(FT_Error)  FT_New_Size( FT_Face   face,
                                        FT_Size*  size );

  FT_EXPORT_DEF(FT_Error)  FT_Done_Size( FT_Size  size );

  FT_EXPORT_DEF(FT_Error)  FT_New_GlyphSlot( FT_Face        face,
                                             FT_GlyphSlot*  aslot );

  FT_EXPORT_DEF(void)      FT_Done_GlyphSlot( FT_GlyphSlot  slot );




  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                         D R I V E R S                           ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_DriverRec                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The root font driver class.  A font driver is responsible for      */
  /*    managing and loading font files of a given format.                 */
  /*                                                                       */
  /*  <Fields>                                                             */
  /*     library     :: A handle to the driver's parent library.           */
  /*                                                                       */
  /*     memory      :: A handle to the driver's memory object.  This is a */
  /*                    duplicate of `library->memory'.                    */
  /*                                                                       */
  /*     interface   :: A set of driver methods that implement its         */
  /*                    behaviour.  These methods are called by the        */
  /*                    various FreeType functions like FT_New_Face(),     */
  /*                    FT_Load_Glyph(), etc.                              */
  /*                                                                       */
  /*     format      :: A typeless pointer, used to store the address of   */
  /*                    the driver's format specific interface.  This is a */
  /*                    table of other driver methods that are specific to */
  /*                    its format.  Format specific interfaces are        */
  /*                    defined in the driver's header files (e.g.,        */
  /*                    `freetype/drivers/ttlib/ttdriver.h').              */
  /*                                                                       */
  /*     version     :: The driver's version.  It can be used for          */
  /*                    versioning and dynamic driver update if needed.    */
  /*                                                                       */
  /*     description :: An ASCII string describing the driver's supported  */
  /*                    format, like `truetype', `type1', etc.             */
  /*                                                                       */
  /*     faces_list  :: The list of faces currently opened by this driver. */
  /*                                                                       */
  /*     extensions  :: a typeless pointer to the driver's extensions      */
  /*                    registry, when they are supported through the      */
  /*                    config macro FT_CONFIG_OPTION_EXTENSIONS           */
  /*                                                                       */
  typedef struct  FT_DriverRec_
  {
    FT_Library          library;
    FT_Memory           memory;

    FT_Generic          generic;

    FT_DriverInterface  interface;
    FT_FormatInterface  format;

    FT_Int              version;      /* driver version     */
    FT_String*          description;  /* format description */

    FT_ListRec          faces_list;   /* driver's faces list    */

    void*               extensions;

  } FT_DriverRec;


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                      G L Y P H   Z O N E S                      ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                       L I B R A R I E S                         ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

#define FT_DEBUG_HOOK_TRUETYPE   0
#define FT_DEBUG_HOOK_TYPE1      1

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_LibraryRec                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The FreeType library class.  This is the root of all FreeType      */
  /*    data.  Use FT_New_Library() to create a library object, and        */
  /*    FT_Done_Library() to discard it and all child objects.             */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    memory         :: The library's memory object.  Manages memory     */
  /*                      allocation                                       */
  /*                                                                       */
  /*    generic        :: Client data variable.  Used to extend the        */
  /*                      Library class by higher levels and clients.      */
  /*                                                                       */
  /*    num_drivers    :: The number of drivers currently registered       */
  /*                      within this library.  This is set to 0 for new   */
  /*                      libraries.  New drivers are added through the    */
  /*                      FT_Add_Driver() API function.                    */
  /*                                                                       */
  /*    drivers        :: A table used to store handles to the currently   */
  /*                      registered font drivers.  Note that each driver  */
  /*                      contains a list of its opened faces.             */
  /*                                                                       */
  /*    raster_pool    :: The raster object's render pool.  This can       */
  /*                      ideally be changed dynamically at run-time.      */
  /*                                                                       */
  typedef  void  (*FT_DebugHook_Func)( void* arg );

  typedef struct  FT_LibraryRec_
  {
    FT_Memory           memory;         /* library's memory object          */

    FT_Generic          generic;

    FT_Int              num_drivers;
    FT_Driver           drivers[ FT_MAX_DRIVERS ];  /* driver objects  */

    FT_Raster_Funcs     raster_funcs[ FT_MAX_GLYPH_FORMATS ];
    FT_Raster           rasters     [ FT_MAX_GLYPH_FORMATS ];

    void*               raster_pool;      /* scan-line conversion render pool */
    long                raster_pool_size; /* size of render pool in bytes     */

    FT_DebugHook_Func   debug_hooks[4];

  } FT_LibraryRec;


  FT_EXPORT_DEF(FT_Error)  FT_New_Library( FT_Memory    memory,
                                           FT_Library*  library );


  FT_EXPORT_DEF(FT_Error)  FT_Done_Library( FT_Library  library );



  FT_EXPORT_DEF(void)  FT_Set_Debug_Hook( FT_Library         library,
                                          FT_UInt            hook_index,
                                          FT_DebugHook_Func  debug_hook );


  FT_EXPORT_DEF(FT_Error)  FT_Add_Driver( FT_Library                 library,
                                          const FT_DriverInterface*  driver_interface );


  FT_EXPORT_DEF(FT_Error)  FT_Remove_Driver( FT_Driver  driver );


  FT_EXPORT_DEF(FT_Driver) FT_Get_Driver( FT_Library  library,
                                          char*       driver_name );

#ifndef FT_CONFIG_OPTION_NO_DEFAULT_SYSTEM

  FT_EXPORT_DEF(FT_Error)   FT_New_Stream( const char*  filepathname,
                                           FT_Stream    astream );

  FT_EXPORT_DEF(void)       FT_Done_Stream( FT_Stream  stream );

  FT_EXPORT_DEF(FT_Memory)  FT_New_Memory( void );

#endif

/* Define default raster's interface. The default raster is located in `src/base/ftraster.c' */
/*                                                                                           */
/* Client applications can register new rasters through the FT_Set_Raster API..              */
/*                                                                                           */
#ifndef FT_NO_DEFAULT_RASTER
  FT_EXPORT_VAR(FT_Raster_Funcs)  ft_default_raster;
#endif


#endif /* FTOBJS_H */


/* END */
