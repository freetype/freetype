/*******************************************************************
 *
 *  ttapi.c    
 *
 *    High-level interface implementation
 *
 *  Copyright 1996-1998 by
 *  David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 *  This file is part of the FreeType project, and may only be used,
 *  modified, and distributed under the terms of the FreeType project
 *  license, LICENSE.TXT.  By continuing to use, modify, or distribute 
 *  this file you indicate that you have read the license and
 *  understand and accept it fully.
 *
 *  Notes:
 *
 *    This file is used to implement most of the functions that are
 *    defined in the file "freetype.h". However, two functions are
 *    implemented elsewhere :                                
 *
 ******************************************************************/

#include <freetype.h>

#include <ftdebug.h>
#include <ftstream.h>
#include <ftcalc.h>
#include <ftlist.h>
#include <ftraster.h>

#include <ttdriver.h>
#include <ttobjs.h>
#include <ttcmap.h>

#define _TRUETYPE_
#include <truetype.h>  /* backwards compatible interface */



/* required by the tracing mode */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_ttapi


#define RENDER_POOL_SIZE  64000

  static
  const FT_DriverInterface*  tt_interface = &tt_driver_interface;
  
  static
  const TT_DriverInterface*  tt_extension = &tt_format_interface;


  /***********************************************************************/
  /*                                                                     */
  /* <Function> TT_Init_FreeType                                         */
  /*                                                                     */
  /* <Description>                                                       */
  /*    Creates a new TrueType driver/engine object.                     */
  /*                                                                     */
  /* <Output>                                                            */
  /*    engine   ::  handle to the new engine object                     */
  /*                                                                     */
  /* <Return>                                                            */
  /*    TrueType error code. 0 means success                             */
  /*                                                                     */
  /* <MT-Note>                                                           */
  /*    No.                                                              */
  /*                                                                     */
  /* <Note>                                                              */
  /*    This function is provided for stand-alone compiles of the        */
  /*    TrueType driver.                                                 */
  /*                                                                     */

  EXPORT_FUNC
  TT_Error  TT_Init_FreeType( TT_Engine*  engine )
  {
    FT_Library library;
    FT_System  system;
    TT_Error   error;

    *engine = 0;

    error = FT_New_System( &system );
    if (error) return error;
    
    error = FT_New_Library( system, &library );
    if (!error)
      /* Now create a new TrueType driver object */
      error = FT_Add_Driver( library,
                             (FT_DriverInterface*)&tt_driver_interface );
    if (error)
      FT_Done_Library(library);
    else
      *engine = (TT_Engine)library;

    return error;
  }


  /***********************************************************************/
  /*                                                                     */
  /* <Function> TT_Done_FreeType                                         */
  /*                                                                     */
  /* <Description>                                                       */
  /*    Destroys a given TrueType engine object created with             */
  /*    TT_Init_FreeType. All associated objects, (i.e. faces, outlines  */
  /*    and charmaps) will be destroyed..                                */
  /*                                                                     */
  /* <Input>                                                             */
  /*    engine   :: handle to the engine object                          */
  /*                                                                     */
  /* <Return>                                                            */
  /*    TrueType error code. 0 means success                             */
  /*                                                                     */
  /* <MT-Note>                                                           */
  /*    No.                                                              */
  /*                                                                     */
  /* <Note>                                                              */
  /*    This function is provided for stand-alone compiles of the        */
  /*    TrueType driver. The FreeType library uses the TT_Done_Engine    */
  /*    API.                                                             */
  /*                                                                     */

  EXPORT_FUNC
  TT_Error  TT_Done_FreeType( TT_Engine  engine )
  {
    FT_Library  library = (FT_Library)engine;

    FT_Done_FreeType( library );
    return FT_Err_Ok;
  }


  /***********************************************************************/
  /*                                                                     */
  /* <Function> TT_Set_Raster_Gray_Palette                               */
  /*                                                                     */
  /* <Description>                                                       */
  /*    Sets the raster's gray 5-levels palette. Entry 0 correspond to   */
  /*    the background, Entry 4 to the foreground. Intermediate entries  */
  /*    correspond to gray levels..                                      */
  /*                                                                     */
  /* <Input>                                                             */
  /*    engine   :: handle to the engine object                          */
  /*    palette  :: an array of 5 bytes used to render 8-bit pixmaps     */
  /*                                                                     */
  /* <Return>                                                            */
  /*    TrueType error code. 0 means success                             */
  /*                                                                     */
  /* <MT-Note>                                                           */
  /*    No.                                                              */
  /*                                                                     */
  /* <Note>                                                              */
  /*    This function is provided for stand-alone compiles of the        */
  /*    TrueType driver. The FreeType library accesses directly the      */
  /*    raster object to set the palette.                                */
  /*                                                                     */
  /*    This function ONLY supports 5 levels of grays.                   */
  /*                                                                     */

  EXPORT_FUNC
  TT_Error  TT_Set_Raster_Gray_Palette( TT_Engine       engine,
                                        const TT_Byte*  palette )
  {  
    FT_Library  library;
    
    if (!engine)
      return TT_Err_Invalid_Engine;
    
    library = (FT_Library)engine;
    return FT_Set_Raster_Palette( library, 5, (unsigned char*)palette );
  }


  /***********************************************************************/
  /*                                                                     */
  /* <Function> TT_Open_Face                                             */
  /*                                                                     */
  /* <Description>                                                       */
  /*    Creates a new face object from a given resource. The file can    */
  /*    be either a TrueType file (ttf) or a TrueType collection (ttc).  */
  /*    In the latter case, only the first face is opened. The number    */
  /*    of faces in a collection can be obtained in the face's           */
  /*    properties field "num_Faces". Other faces can be opened with     */
  /*    TT_Open_Collection (see below).                                  */
  /*                                                                     */
  /* <Input>                                                             */
  /*    engine    :: the parent engine object where to create the face   */
  /*                 object.                                             */
  /*                                                                     */
  /*    pathname  :: pathname for the font file.                         */
  /*                                                                     */
  /* <Output>                                                            */
  /*    face      :: a handle to the fresh face object.                  */
  /*                                                                     */
  /* <Return>                                                            */
  /*    TrueType error code. 0 means success..                           */
  /*                                                                     */
  /* <MT-Note>                                                           */
  /*    Yes.                                                             */
  /*                                                                     */
  /* <Note>                                                              */
  /*    This API is provided fro backwards compatibility. Please use     */
  /*    the functions TT_New_Face/TT_Done_Face now to create and         */
  /*    discard face objects..                                           */
  /*                                                                     */


        static
        TT_Error  open_face( FT_Library      library,
                             const TT_Text*  pathname,
                             TT_Int          face_index,
                             TT_Face        *aface )
        {
          TT_Error    error;
          FT_Resource resource;
      
          *aface = 0;
      
          error = FT_New_Resource( library, pathname, &resource );
          if (error) return error;

#if 0      
          error = FT_Add_Resource( library, resource );
          if (error) goto Fail_Install;
#endif
          error = FT_New_Face( resource, face_index, (FT_Face*)aface );

          /* Destroy glyph slot to comply with the 1.x API */
          if (!error)
            FT_Done_GlyphSlot( (*aface)->root.slot );
      
          if (error)
            FT_Done_Resource(resource);
      
          return error;
        }


  EXPORT_DEF
  TT_Error  TT_Open_Face( TT_Engine      engine,
                          const TT_Text* pathname,
                          TT_Face*       aface )
  {
    if (!engine)
      return TT_Err_Invalid_Driver_Handle;

    return  open_face( (FT_Library)engine, pathname, 0, aface );
  }
  

  /***********************************************************************/
  /*                                                                     */
  /* <Function> TT_Open_Collection                                       */
  /*                                                                     */
  /* <Description>                                                       */
  /*    Loads a given face within a collection.                          */
  /*                                                                     */
  /* <Input>                                                             */
  /*    engine    :: TrueType engine object where to load the face       */
  /*    pathname  :: the collection's pathname                           */
  /*    fontIndex :: index of face within the collection. first is 0     */
  /*                                                                     */
  /* <Output>                                                            */
  /*    face      :: handle to the new face object. Always set to NULL   */
  /*                 in case of error                                    */
  /* <Return>                                                            */
  /*    TrueType error code. 0 means success                             */
  /*                                                                     */
  /* <MT-Note>                                                           */
  /*    Yes.                                                             */
  /*                                                                     */
  /* <Note>                                                              */
  /*    This API is provided for backwards compatibility. Please use     */
  /*    the functions TT_New_Collection/TT_Done_Face now to create and   */
  /*    discard face/collection objects.                                 */
  /*                                                                     */

  EXPORT_DEF
  TT_Error  TT_Open_Collection( TT_Engine       engine,
                                const TT_Text*  pathname,
                                TT_ULong        fontIndex,
                                TT_Face*        aface )
  {
    if (!engine)
      return TT_Err_Invalid_Driver_Handle;

    return open_face( (FT_Library)engine, pathname, fontIndex, aface );
  }


  /***********************************************************************/
  /*                                                                     */
  /* <Function> TT_Close_Face                                            */
  /*                                                                     */
  /* <Description>                                                       */
  /*    Destroys a given face object opened through either TT_Open_Face  */
  /*    of TT_Open_Collection.                                           */
  /*                                                                     */
  /* <Input>                                                             */
  /*    face  :: handle to the target face object                        */
  /*                                                                     */
  /* <Return>                                                            */
  /*    TrueType error code. 0 means success                             */
  /*                                                                     */
  /* <MT-Note>                                                           */
  /*    No.                                                              */
  /*                                                                     */
  /* <Note>                                                              */
  /*    This API is provided for backwards compatibility. Please use     */
  /*    the functions TT_New_Face/TT_Done_Face now to create and         */
  /*    discard face/collection objects.                                 */
  /*                                                                     */

  EXPORT_DEF
  TT_Error  TT_Close_Face( TT_Face  face )
  {
    FT_Resource  resource;
    
    if (!face)
      return TT_Err_Invalid_Face_Handle;

    resource = face->root.resource;
    FT_Done_Face( (FT_Face)face );

    /* uninstall corresponding resource */
    FT_Done_Resource( resource );

    return TT_Err_Ok;
  }


  /***********************************************************************/
  /***********************************************************************/
  /***********************************************************************/
  /***********************************************************************/
  /***********                                                 ***********/
  /***********   End of backwards compatible APIs..            ***********/
  /***********                                                 ***********/
  /***********************************************************************/
  /***********************************************************************/
  /***********************************************************************/
  /***********************************************************************/


  /***********************************************************************/
  /*                                                                     */
  /* <Function> TT_Get_Face_Properties                                   */
  /*                                                                     */
  /* <Description>                                                       */
  /*    Return a given face's properties to the caller.                  */
  /*                                                                     */
  /* <Input>                                                             */
  /*    face  :: handle to the source face object                        */
  /*                                                                     */
  /* <Output>                                                            */
  /*    properties :: target properties structure                        */
  /*                                                                     */
  /* <Return>                                                            */
  /*    TrueType error code. 0 means success.                            */
  /*                                                                     */
  /* <MT-Note>                                                           */
  /*    Yes.                                                             */
  /*                                                                     */

  EXPORT_FUNC
  TT_Error  TT_Get_Face_Properties( TT_Face              face,
                                    TT_Face_Properties*  props )
  {
    props->num_Glyphs   = (TT_UShort)face->root.num_glyphs;
    props->max_Points   = (TT_UShort)face->root.max_points;
    props->max_Contours = (TT_UShort)face->root.max_contours;
    props->num_CharMaps = (TT_UShort)face->root.num_charmaps;
    props->num_Faces    = face->root.num_faces;
    props->num_Names    = face->num_names;
    props->header       = &face->header;
    props->horizontal   = &face->horizontal;
      
      /* The driver supports old Mac fonts where there are no OS/2  */
      /* tables present in the file. However, this is not true of   */
      /* FreeType 1.1. For the sake of backwards compatibility, we  */
      /* always return the address of the face's os2 table, even if */
      /* it is empty (in which case, the 'props.os2' field is set   */
      /* to NULL..                                                  */
      /*                                                            */
      /* Note however, that the 'os2->version' field is set to      */
      /* 0xFFFF to indicate a missing table though...               */
      /*                                                            */

    props->os2          = &face->os2;
      
    props->postscript    = &face->postscript;
    props->hdmx          = &face->hdmx;
    props->vertical      = ( face->vertical_info ? &face->vertical : 0 );

    return TT_Err_Ok;
  }


  /***********************************************************************/
  /*                                                                     */
  /* <Function> TT_Set_Face_Pointer.                                     */
  /*                                                                     */
  /* <Description>                                                       */
  /*    Each face object contains a typeless pointer, which use is left  */
  /*    to client applications (or the high-level library). This API is  */
  /*    used to set this generic pointer. It is ignored by the driver.   */
  /*                                                                     */
  /* <Input>                                                             */
  /*    face   :: target face object                                     */
  /*    data   :: generic pointer's value                                */
  /*                                                                     */
  /* <Return>                                                            */
  /*    TrueType error code. 0 means success                             */
  /*                                                                     */
  /* <MT-Note>                                                           */
  /*    No.                                                              */
  /*                                                                     */
  /* <Note>                                                              */
  /*    The generic pointer is used by the HLib when using the driver    */
  /*    within the FreeType library.                                     */
  /*                                                                     */

  EXPORT_FUNC
  TT_Error  TT_Set_Face_Pointer( TT_Face  face,
                                 void*    data )
  {
    if ( !face )
      return TT_Err_Invalid_Face_Handle;
    else
      face->root.generic.data = data;

    return TT_Err_Ok;
  }


  /***********************************************************************/
  /*                                                                     */
  /* <Function> TT_Get_Face_Pointer                                      */
  /*                                                                     */
  /* <Description>                                                       */
  /*    Each face object contains a typeless pointer, which use is left  */
  /*    to client applications (or the high-level library). This API is  */
  /*    used to retrieve this generic pointer, which is ignored by the   */
  /*    driver.                                                          */
  /*                                                                     */
  /* <Input>                                                             */
  /*    face    :: handle to source face object                          */
  /*                                                                     */
  /* <Return>                                                            */
  /*    generic pointer value. NULL if the face handle is invalid..      */
  /*                                                                     */
  /* <MT-Note>                                                           */
  /*    No.                                                              */
  /*                                                                     */

  EXPORT_FUNC
  void*  TT_Get_Face_Pointer( TT_Face  face )
  {
    return ( face ? face->root.generic.data : NULL );
  }

  /***********************************************************************/
  /*                                                                     */
  /* <Function> TT_Get_Face_Metrics                                      */
  /*                                                                     */
  /* <Description>                                                       */
  /*    Get the metrics of a given array of glyphs. Returns any number   */
  /*    of metrics arrays.                                               */
  /*                                                                     */
  /* <Input>                                                             */
  /*    face       :: handle to the source face object                   */
  /*    firstGlyph :: index of first glyph in the array                  */
  /*    lastGlyph  :: index of last glyph in the array                   */
  /*                                                                     */
  /* <Output>                                                            */
  /*    leftBearings :: target array of shorts for the glyph left side   */
  /*                    bearings. Set this field to NULL if you're not   */
  /*                    interested in these metrics.                     */
  /*                                                                     */
  /*    widths :: target array of unsigned shorts for the glyph advance  */
  /*              widths. Set this field to NULL if you're not           */
  /*              interested in these metrics.                           */
  /*                                                                     */
  /*    topBearings :: target array of shorts for the glyph top side     */
  /*                   bearings. Set this field to NULL if you're not    */
  /*                   interested in these metrics.                      */
  /*                                                                     */
  /*    heights :: target array of unsigned shorts for the glyph advance */
  /*               heights. Set this field to NULL if you're not         */
  /*               interested in these metrics.                          */
  /*                                                                     */
  /* <Return>                                                            */
  /*    TrueType error code. 0 means success.                            */
  /*                                                                     */
  /* <MT-Note>                                                           */
  /*    No.                                                              */
  /*                                                                     */

        /********************************************************/
        /* Return horizontal or vertical metrics in font units  */
        /* for a given glyph.  The metrics are the left side    */
        /* bearing (resp. top side bearing) and advance width   */
        /* (resp. advance height).                              */
        /*                                                      */
        /* This function will much probably move to another     */
        /* component in the short future, but I haven't decided */
        /* which yet...                                         */
    
          static
          void  get_metrics( TT_HoriHeader*  header,
                             TT_Int          index,
                             TT_Short*       bearing,
                             TT_UShort*      advance )
          {
            TT_LongMetrics*  longs_m;
    
            TT_UShort  k = header->number_Of_HMetrics;
    
    
            if ( index < k )
            {
              longs_m = (TT_LongMetrics*)header->long_metrics + index;
              *bearing = longs_m->bearing;
              *advance = longs_m->advance;
            }
            else
            {
              *bearing = ((TT_ShortMetrics*)header->short_metrics)[index - k];
              *advance = ((TT_LongMetrics*)header->long_metrics)[k - 1].advance;
            }
          }
    


  EXPORT_FUNC
  TT_Error  TT_Get_Face_Metrics( TT_Face     face,
                                 TT_UShort   firstGlyph,
                                 TT_UShort   lastGlyph,
                                 TT_Short*   leftBearings,
                                 TT_UShort*  widths,
                                 TT_Short*   topBearings,
                                 TT_UShort*  heights )
  {
    TT_UShort  num;

    if ( !face )
      return TT_Err_Invalid_Face_Handle;

    /* Check the glyph range */
    if ( lastGlyph >= face->root.num_glyphs || firstGlyph > lastGlyph )
      return TT_Err_Invalid_Argument;

    num = lastGlyph - firstGlyph;   /* number of elements-1 in each array */

    /* store the left side bearings and advance widths first */
    {
      TT_UShort  n;
      TT_Short   left_bearing;
      TT_UShort  advance_width;


      for ( n = 0; n <= num; n++ )
      {
        get_metrics( &face->horizontal,
                      firstGlyph + n, &left_bearing, &advance_width );

        if ( leftBearings )  leftBearings[n] = left_bearing;
        if ( widths )        widths[n]       = advance_width;
      }
    }

    /* check for vertical data if topBearings or heights is non-NULL */
    if ( !topBearings && !heights )
      return TT_Err_Ok;

    if ( !face->vertical_info )
      return TT_Err_No_Vertical_Data;

    /* store the top side bearings */
    {
      TT_UShort  n;
      TT_Short   top_bearing;
      TT_UShort  advance_height;

      for ( n = 0; n <= num; n++ )
      {
        get_metrics( (TT_HoriHeader*)&face->vertical,
                     firstGlyph + n, &top_bearing, &advance_height );

        if ( topBearings )  topBearings[n] = top_bearing;
        if ( heights )      heights[n]     = advance_height;
      }
    }

    return TT_Err_Ok;
  }


  /***********************************************************************/
  /*                                                                     */
  /* <Function> TT_New_Instance                                          */
  /*                                                                     */
  /* <Description>                                                       */
  /*    Creates a new instance object from a given face                  */
  /*                                                                     */
  /* <Input>                                                             */
  /*    face  :: handle to source/parent face object                     */
  /*                                                                     */
  /* <Output>                                                            */
  /*    instance :: handle to fresh object. Set to NULL in case of       */
  /*                error.                                               */
  /*                                                                     */
  /* <Return>                                                            */
  /*    TrueType error code. 0 means success                             */
  /*                                                                     */
  /* <MT-Note>                                                           */
  /*    Yes.                                                             */
  /*                                                                     */
  /* <Note>                                                              */
  /*    Any new instance uses a default resolution of 96x96 dpi, and     */
  /*    a default point size of 10 pts. You can change these anytime     */
  /*    with TT_Set_Instance_Resolutions/CharSize/CharSizes/PixelSizes   */
  /*    (see below).                                                     */
  /*                                                                     */

  EXPORT_FUNC
  TT_Error  TT_New_Instance( TT_Face       face,
                             TT_Instance*  instance )
  {
    return FT_New_Size( (FT_Face)face, (FT_Size*)instance );
  }


  /***********************************************************************/
  /*                                                                     */
  /* <Function> TT_Set_Instance_Resolutions                              */
  /*                                                                     */
  /* <Description>                                                       */
  /*    Sets an instance resolutions in dot-per-inches. Default values   */
  /*    for "new" instances are 96x96 dpi, but these can be changed any  */
  /*    time by calling this API.                                        */
  /*                                                                     */
  /* <Input>                                                             */
  /*    instance    :: handle to target instance object                  */
  /*    xResolution :: horizontal device resolution in dpi.              */
  /*    yResolution :: vertical device resolution in dpi.                */
  /*                                                                     */
  /* <Return>                                                            */
  /*    TrueType error code. 0 means success                             */
  /*                                                                     */
  /* <MT-Note>                                                           */
  /*    You should set the charsize or pixel size immediately after      */
  /*    call in multi-htreaded programs. This will force the instance    */
  /*    data to be resetted. Otherwise, you may encounter corruption     */
  /*    when loading two glyphs from the same instance concurrently!     */
  /*                                                                     */
  /*    Happily, 99.99% will do just that :-)                            */
  /*                                                                     */
  /*                                                                     */
  /*                                                                     */

  EXPORT_FUNC
  TT_Error  TT_Set_Instance_Resolutions( TT_Instance  ins,
                                         TT_UShort    xResolution,
                                         TT_UShort    yResolution )
  {
    return FT_Set_Resolutions( (FT_Size)ins, xResolution, yResolution );
  }

  /***********************************************************************/
  /*                                                                     */
  /* <Function> TT_Set_Instance_CharSizes                                */
  /*                                                                     */
  /* <Description>                                                       */
  /*    Same as TT_Set_Instance_CharSize, but used to specify distinct   */
  /*    horizontal and vertical coordinates.                             */
  /*                                                                     */
  /* <Input>                                                             */
  /*    instance   :: handle to target instance object                   */
  /*    charWidth  :: horizontal character size (26.6 points)            */
  /*    charHeight :: vertical character size (26.6 points)              */
  /*                                                                     */
  /* <Return>                                                            */
  /*    TrueType error code. 0 means success                             */
  /*                                                                     */
  /* <MT-Note>                                                           */
  /*    No.                                                              */
  /*                                                                     */
  /* <Note>                                                              */
  /*    There is no check for overflow; with other words, the product    */
  /*    of glyph dimensions times the device resolution must have        */
  /*    reasonable values.                                               */
  /*                                                                     */

  EXPORT_FUNC
  TT_Error  TT_Set_Instance_CharSizes( TT_Instance  ins,
                                       TT_F26Dot6   charWidth,
                                       TT_F26Dot6   charHeight )
  {
    return FT_Set_Char_Sizes( (FT_Size)ins, charWidth, charHeight );
  }


  /***********************************************************************/
  /*                                                                     */
  /* <Function> TT_Set_Instance_CharSize                                 */
  /*                                                                     */
  /* <Description>                                                       */
  /*    Sets an instance's character size. The size is in fractional     */
  /*    (26.6) point units. This function sets the horizontal and        */
  /*    vertical sizes to be equal. Use TT_Set_Instance_CharSizes        */
  /*    for distinct X and Y char sizes.                                 */
  /*                                                                     */
  /*    The default charsize of a new instance object is 10 pts.         */
  /*                                                                     */
  /* <Input>                                                             */
  /*    instance  :: handle to target instance object                    */
  /*    charSize  :: character size expressed in 26.6 fixed float        */
  /*                 points. 1 point = 1/72 dpi.                         */
  /*                                                                     */
  /* <Return>                                                            */
  /*    TrueType error code. 0 means success                             */
  /*                                                                     */
  /* <MT-Note>                                                           */
  /*    No.                                                              */
  /*                                                                     */

  EXPORT_FUNC
  TT_Error  TT_Set_Instance_CharSize( TT_Instance  instance,
                                      TT_F26Dot6   charSize )
  {
    return TT_Set_Instance_CharSizes( instance, charSize, charSize );
  }


  /***********************************************************************/
  /*                                                                     */
  /* <Function> TT_Set_Instance_PixelSizes                               */
  /*                                                                     */
  /* <Description>                                                       */
  /*    This function is used to set an instance's pixel sizes directly  */
  /*    It ignores the instance's resolutions fields, and you'll have to */
  /*    specify the corresponding pointsize in 26.6 fixed float point    */
  /*    units. The latter is a requirement of the TrueType bytecode      */
  /*    interpreter but can be ignored (or more safely, set to the       */
  /*    maximum pixel size multiplied by 64).                            */
  /*                                                                     */
  /* <Input>                                                             */
  /*    instance    :: handle to target instance object                  */
  /*    pixelWidth  :: horizontal pixel width (integer value)            */
  /*    pixelHeight :: vertical pixel height (integer value)             */
  /*    pointSize   :: corresponding point size (26.6 points)            */
  /*                                                                     */
  /* <Return>                                                            */
  /*    TrueType error code. 0 means success.                            */
  /*                                                                     */
  /* <MT-Note>                                                           */
  /*   No.                                                               */
  /*                                                                     */

  EXPORT_FUNC
  TT_Error  TT_Set_Instance_PixelSizes( TT_Instance  ins,
                                        TT_UShort    pixelWidth,
                                        TT_UShort    pixelHeight,
                                        TT_F26Dot6   pointSize )
  {
    /* The point size is now ignored by the driver */
    (void)pointSize;
    
    return FT_Set_Pixel_Sizes( (FT_Size)ins, pixelWidth, pixelHeight );
  }

  /***********************************************************************/
  /*                                                                     */
  /* <Function> TT_Get_Instance_Metrics                                  */
  /*                                                                     */
  /* <Description>                                                       */
  /*    Returns an instance's metrics into caller space.                 */
  /*                                                                     */
  /* <Input>                                                             */
  /*    instance  :: handle to source instance object                    */
  /*                                                                     */
  /* <Output>                                                            */
  /*    metrics   :: target instance metrics structure                   */
  /*                                                                     */
  /* <Return>                                                            */
  /*    TrueType error code. 0 means success                             */
  /*                                                                     */
  /* <MT-Note>                                                           */
  /*    No.                                                              */
  /*                                                                     */
  /* <Note>                                                              */
  /*    The TT_Instance_Metrics structure differs slightly from the      */
  /*    FT_Instance_Metrics one, which is why we re-implement this       */
  /*    function, rather than call a driver method for this..            */
  /*                                                                     */

  EXPORT_FUNC
  TT_Error  TT_Get_Instance_Metrics( TT_Instance           ins,
                                     TT_Instance_Metrics*  metrics )
  {
    TT_Size  size = (TT_Size)ins;

    if (!ins)
      return TT_Err_Invalid_Instance_Handle;

    metrics->x_scale      = size->root.metrics.x_scale;
    metrics->y_scale      = size->root.metrics.y_scale;
    metrics->x_resolution = size->root.metrics.x_resolution;
    metrics->y_resolution = size->root.metrics.y_resolution;
    metrics->x_ppem       = size->root.metrics.x_ppem;
    metrics->y_ppem       = size->root.metrics.y_ppem;

    metrics->pointSize    = size->root.metrics.pointSize;    
    return TT_Err_Ok;
  }


  /***********************************************************************/
  /*                                                                     */
  /* <Function> TT_Set_Instance_Pointer                                  */
  /*                                                                     */
  /* <Description>                                                       */
  /*    Each instance object contains a typeless pointer, which use is   */
  /*    left to client applications (or the high-level library). This    */
  /*    API is used to set this generic pointer. It is ignored by the    */
  /*    driver.                                                          */
  /*                                                                     */
  /* <Input>                                                             */
  /*    instance  :: handle to the target instance object                */
  /*    data      :: value of the generic pointer                        */
  /*                                                                     */
  /* <Return>                                                            */
  /*    TrueType error code. 0 means success                             */
  /*                                                                     */
  /* <MT-Note>                                                           */
  /*    No.                                                              */
  /*                                                                     */

  EXPORT_FUNC
  TT_Error  TT_Set_Instance_Pointer( TT_Instance  ins,
                                     void*        data )
  {
    if ( !ins )
      return TT_Err_Invalid_Instance_Handle;
    else
      ((FT_Size)ins)->generic.data = data;

    return TT_Err_Ok;
  }


  /***********************************************************************/
  /*                                                                     */
  /* <Function> TT_Get_Instance_Pointer                                  */
  /*                                                                     */
  /* <Description>                                                       */
  /*    Each instance object contains a typeless pointer, which use is   */
  /*    left to client applications (or the high-level library). This    */
  /*    API is used to retrieve this generic pointer. It is ignored by   */
  /*    the driver.                                                      */
  /*                                                                     */
  /* <Input>                                                             */
  /*    instance  :: handle to source instance object                    */
  /*                                                                     */
  /* <Return>                                                            */
  /*    value of generic pointer. NULL if invalid instance               */
  /*                                                                     */
  /* <MT-Note>                                                           */
  /*    No.                                                              */

  EXPORT_FUNC
  void*  TT_Get_Instance_Pointer( TT_Instance  instance )
  {
    if ( !instance )
      return NULL;
    else
      return ((FT_Size)instance)->generic.data;
  }


  /***********************************************************************/
  /*                                                                     */
  /* <Function> TT_Done_Instance                                         */
  /*                                                                     */
  /* <Description>                                                       */
  /*    Destroys a given instance object. All instances are destroyed    */
  /*    automatically when their parent face object is discarded.        */
  /*    However, this API can be used to save memory.                    */
  /*                                                                     */
  /* <Input>                                                             */
  /*    instance  :: handle to target instance object                    */
  /*                                                                     */
  /* <Return>                                                            */
  /*    TrueType error code. 0 means success                             */
  /*                                                                     */
  /* <MT-Note>                                                           */
  /*    No.                                                              */

  EXPORT_FUNC
  TT_Error  TT_Done_Instance( TT_Instance  ins )
  {
    FT_Done_Size( (FT_Size)ins );
    return TT_Err_Ok;
  }


  /***********************************************************************/
  /*                                                                     */
  /* <Function> TT_Set_Instance_Transform_Flags                          */
  /*                                                                     */
  /* <Description>                                                       */
  /*    Nothing. This function has been deprecated...                    */
  /*                                                                     */
  /* <Input>                                                             */
  /*    instance      :: handle to target instance object                */
  /*    rotated       :: 'rotation' flag..                               */
  /*    stretched     :: 'stretch' flag..                                */
  /*                                                                     */
  /* <Return>                                                            */
  /*    Always 0.                                                        */
  /*                                                                     */
  /* <Note>                                                              */
  /*    This function has been deprecated !! Do not use it, it doesn't   */
  /*    do anything now..                                                */
  /*                                                                     */

  EXPORT_FUNC
  TT_Error  TT_Set_Instance_Transform_Flags( TT_Instance  instance,
                                             TT_Bool      rotated,
                                             TT_Bool      stretched )
  {
    (void)instance;   /* the parameters are unused, the (void) prevents */
    (void)rotated;    /* warnings from pedantic compilers..             */
    (void)stretched;
    
    return TT_Err_Ok;
  }



  /***********************************************************************/
  /*                                                                     */
  /* <Function>                                                          */
  /*                                                                     */
  /* <Description>                                                       */
  /*                                                                     */
  /*                                                                     */
  /* <Input>                                                             */
  /*                                                                     */
  /*                                                                     */
  /* <Output>                                                            */
  /*                                                                     */
  /*                                                                     */
  /* <Return>                                                            */
  /*    TrueType error code. 0 means success                             */
  /*                                                                     */
  /*                                                                     */
  /* <MT-Note>                                                           */
  /*                                                                     */
  /*                                                                     */
  /*                                                                     */
  /* <Note>                                                              */
  /*                                                                     */
  /*                                                                     */
  /*                                                                     */
/*******************************************************************
 *
 *  Function    :  TT_New_Glyph
 *
 *  Description :  Creates a new glyph object related to a given
 *                 face.
 *
 *  Input  :  face       the face handle
 *            glyph      address of target glyph handle
 *
 *  Output :  Error code.
 *
 *  MT-Safe : YES!
 *
 ******************************************************************/

  EXPORT_FUNC
  TT_Error  TT_New_Glyph( TT_Face    face,
                          TT_Glyph*  aglyph )
  {
    return FT_New_GlyphSlot( (FT_Face)face, (FT_GlyphSlot*)aglyph );
  }


/*******************************************************************
 *
 *  Function    :  TT_Done_Glyph
 *
 *  Description :  Destroys a given glyph object.
 *
 *  Input  :  glyph  the glyph handle
 *
 *  Output :  Error code.
 *
 *  MT-Safe : YES!
 *
 ******************************************************************/

  EXPORT_FUNC
  TT_Error  TT_Done_Glyph( TT_Glyph  glyph )
  {
    FT_Done_GlyphSlot( (FT_GlyphSlot)glyph );
    return TT_Err_Ok;
  }


/*******************************************************************
 *
 *  Function    :  TT_Load_Glyph
 *
 *  Description :  Loads a glyph.
 *
 *  Input  :  instance      the instance handle
 *            glyph         the glyph handle
 *            glyphIndex    the glyph index
 *            loadFlags     flags controlling how to load the glyph
 *                          (none, scaled, hinted, both)
 *
 *  Output :  Error code.
 *
 *  MT-Safe : YES!
 *
 ******************************************************************/

  EXPORT_FUNC
  TT_Error  TT_Load_Glyph( TT_Instance  instance,
                           TT_Glyph     glyph,
                           TT_UShort    glyphIndex,
                           TT_UShort    loadFlags   )
  {
    TT_Int  result, flags;

    flags = 0;

    /* Convert load flags */
    if ( (loadFlags & TTLOAD_SCALE_GLYPH) == 0 )
      flags = FT_LOAD_NO_SCALE;

    else if ( (loadFlags & TTLOAD_HINT_GLYPH) == 0 )
      flags = FT_LOAD_NO_HINTING;

    else
      flags = FT_LOAD_DEFAULT;

    flags |= FT_LOAD_NO_BITMAP |   /* prevent bitmap loading for */
             FT_LOAD_LINEAR;       /* compatibility purposes..   */

    return FT_Load_Glyph( (FT_GlyphSlot)glyph,
                          (FT_Size)instance,
                          glyphIndex,
                          flags,
                          &result );
  }


/*******************************************************************
 *
 *  Function    :  TT_Get_Glyph_Outline
 *
 *  Description :  Returns the glyph's outline data.
 *
 *  Input  :  glyph     the glyph handle
 *            outline   address where the glyph outline will be returned
 *
 *  Output :  Error code.
 *
 *  MT-Safe : YES!  Reads only semi-permanent data.
 *
 ******************************************************************/

  EXPORT_FUNC
  TT_Error  TT_Get_Glyph_Outline( TT_Glyph     glyph,
                                  TT_Outline*  outline )
  {
    FT_GlyphSlot  slot = (FT_GlyphSlot)glyph;

    if (!glyph)
      return TT_Err_Invalid_Glyph_Handle;

    /* the structures TT_Outline and FT_Outline are equivalent */
    *((FT_Outline*)outline) = slot->outline;
    return TT_Err_Ok;
  }


/*******************************************************************
 *
 *  Function    :  TT_Get_Glyph_Metrics
 *
 *  Description :  Extracts the glyph's horizontal metrics information.
 *
 *  Input  :  glyph       glyph object handle
 *            metrics     address where metrics will be returned
 *
 *  Output :  Error code.
 *
 *  MT-Safe : NO!  Glyph containers can't be shared.
 *
 ******************************************************************/

  EXPORT_FUNC
  TT_Error  TT_Get_Glyph_Metrics( TT_Glyph           glyph,
                                  TT_Glyph_Metrics*  metrics )
  {
    FT_GlyphSlot  slot = (FT_GlyphSlot)glyph;

    if (!glyph)
      return TT_Err_Invalid_Glyph_Handle;

    /* TT_Glyph_Metrics and FT_Glyph_Metrics are slightly different */
    metrics->bbox.xMin = slot->metrics.horiBearingX;

    metrics->bbox.xMax = slot->metrics.horiBearingX +
                         slot->metrics.width;

    metrics->bbox.yMax = slot->metrics.horiBearingY;

    metrics->bbox.yMin = slot->metrics.horiBearingY -
                         slot->metrics.height;

    metrics->bearingX  = slot->metrics.horiBearingX;
    metrics->bearingY  = slot->metrics.horiBearingY;
    metrics->advance   = slot->metrics.horiAdvance;
    
    return TT_Err_Ok;
  }


/*******************************************************************
 *
 *  Function    :  TT_Get_Big_Glyph_Metrics
 *
 *  Description :  Extracts the glyph's big metrics information.
 *
 *  Input  :  glyph       glyph object handle
 *            metrics     address where big metrics will be returned
 *
 *  Output :  Error code.
 *
 *  MT-Safe : NO!  Glyph containers can't be shared.
 *
 ******************************************************************/

  EXPORT_FUNC
  TT_Error  TT_Get_Glyph_Big_Metrics( TT_Glyph               glyph,
                                      TT_Big_Glyph_Metrics*  metrics )
  {
    FT_GlyphSlot       slot = (FT_GlyphSlot)glyph;
    FT_Glyph_Metrics*  met;
    FT_Glyph_Metrics*  met2;

    if (!glyph)
      return TT_Err_Invalid_Glyph_Handle;

    met  = &slot->metrics;
    met2 = &slot->metrics2;

    metrics->bbox.xMin = met->horiBearingX;
    metrics->bbox.xMax = met->horiBearingX + met->width;
    metrics->bbox.yMin = met->horiBearingY - met->height;
    metrics->bbox.yMax = met->horiBearingY;

    metrics->horiBearingX = met->horiBearingX;
    metrics->horiBearingY = met->horiBearingY;
    metrics->horiAdvance  = met->horiAdvance;

    metrics->vertBearingX = met->vertBearingX;
    metrics->vertBearingY = met->vertBearingY;
    metrics->vertAdvance  = met->vertAdvance;

    metrics->linearHoriAdvance  = met2->horiAdvance;
    metrics->linearHoriBearingX = met2->horiBearingX;

    metrics->linearVertAdvance  = met2->vertAdvance;
    metrics->linearVertBearingY = met2->vertBearingY;
   
    return TT_Err_Ok;
  }


/*******************************************************************
 *
 *  Function    :  TT_Get_Glyph_Bitmap
 *
 *  Description :  Produces a bitmap from a glyph outline.
 *
 *  Input  :  glyph      the glyph container's handle
 *            map        target pixmap description block
 *            xOffset    x offset in fractional pixels (26.6 format)
 *            yOffset    y offset in fractional pixels (26.6 format)
 *
 *  Output :  Error code.
 *
 *  Note : Only use integer pixel offsets if you want to preserve
 *         the fine hints applied to the outline.  This means that
 *         xOffset and yOffset must be multiples of 64!
 *
 *  MT-Safe : NO!  Glyph containers can't be shared.
 *
 ******************************************************************/

  EXPORT_FUNC
  TT_Error  TT_Get_Glyph_Bitmap( TT_Glyph        glyph,
                                 TT_Raster_Map*  map,
                                 TT_F26Dot6      xOffset,
                                 TT_F26Dot6      yOffset )
  {
    FT_Library     library;
    TT_Error       error;
    FT_GlyphSlot   slot = (FT_GlyphSlot)glyph;
    FT_Outline     outline;
    FT_Raster_Map  bitmap;

    if ( !glyph )
      return TT_Err_Invalid_Glyph_Handle;

    library = slot->face->resource->library;
    outline = slot->outline;

    /* XXX : For now, use only dropout mode 2    */
    /* outline.dropout_mode = _glyph->scan_type; */
    outline.dropout_mode = 2;

    bitmap.width    = map->width;
    bitmap.rows     = map->rows;
    bitmap.cols     = map->cols;
    bitmap.flow     = map->flow;
    bitmap.pix_bits = 1;
    bitmap.buffer   = map->bitmap;

    FT_Translate_Outline( &outline, xOffset, yOffset );

    error = FT_Get_Outline_Bitmap( library, &outline, &bitmap );

    FT_Translate_Outline( &outline, -xOffset, -yOffset );

    return error;
  }


/*******************************************************************
 *
 *  Function    :  TT_Get_Glyph_Pixmap
 *
 *  Description :  Produces a grayscaled pixmap from a glyph
 *                 outline.
 *
 *  Input  :  glyph      the glyph container's handle
 *            map        target pixmap description block
 *            xOffset    x offset in fractional pixels (26.6 format)
 *            yOffset    y offset in fractional pixels (26.6 format)
 *
 *  Output :  Error code.
 *
 *  Note : Only use integer pixel offsets to preserve the fine
 *         hinting of the glyph and the 'correct' anti-aliasing
 *         (where vertical and horizontal stems aren't grayed).
 *         This means that xOffset and yOffset must be multiples
 *         of 64!
 *
 *         You can experiment with offsets of +32 to get 'blurred'
 *         versions of the glyphs (a nice effect at large sizes that
 *         some graphic designers may appreciate :)
 *
 *  MT-Safe : NO!  Glyph containers can't be shared.
 *
 ******************************************************************/

  EXPORT_FUNC
  TT_Error  TT_Get_Glyph_Pixmap( TT_Glyph        glyph,
                                 TT_Raster_Map*  map,
                                 TT_F26Dot6      xOffset,
                                 TT_F26Dot6      yOffset )
  {
    FT_Library     library;
    TT_Error       error;
    FT_GlyphSlot   slot = (FT_GlyphSlot)glyph;
    FT_Outline     outline;
    FT_Raster_Map  bitmap;

    if ( !glyph )
      return TT_Err_Invalid_Glyph_Handle;

    library = slot->face->resource->library;
    outline = slot->outline;

    /* XXX : For now, use only dropout mode 2    */
    /* outline.dropout_mode = _glyph->scan_type; */
    outline.dropout_mode = 2;

    bitmap.width    = map->width;
    bitmap.rows     = map->rows;
    bitmap.cols     = map->cols;
    bitmap.flow     = map->flow;
    bitmap.pix_bits = 8;
    bitmap.buffer   = map->bitmap;

    FT_Translate_Outline( &outline, xOffset, yOffset );

    error = FT_Get_Outline_Bitmap( library, &outline, &bitmap );

    FT_Translate_Outline( &outline, -xOffset, -yOffset );

    return error;
  }



  /***********************************************************************/
  /*                                                                     */
  /* <Function> TT_Get_Outline_Bitmap                                    */
  /*                                                                     */
  /* <Description>                                                       */
  /*     Renders an outline within a bitmap. The outline's image is      */
  /*     simply or-ed to the target bitmap.                              */
  /*                                                                     */
  /* <Input>                                                             */
  /*     engine  ::   handle to the TrueType driver object               */
  /*     outline ::   pointer to the source outline descriptor           */
  /*     raster  ::   pointer to the target bitmap descriptor            */
  /*                                                                     */
  /* <Return>                                                            */
  /*     TrueType error code. 0 means success.                           */
  /*                                                                     */
  /* <MT-Note>                                                           */
  /*     Yes.                                                            */
  /*                                                                     */

  EXPORT_FUNC
  TT_Error  TT_Get_Outline_Bitmap( TT_Engine       engine,
                                   TT_Outline*     outline,
                                   TT_Raster_Map*  map )
  {
    FT_Library     library = (FT_Library)engine;
    FT_Raster_Map  bitmap;

    if ( !engine )
      return TT_Err_Invalid_Engine;

    if ( !outline || !map )
      return TT_Err_Invalid_Argument;

    bitmap.width    = map->width;
    bitmap.rows     = map->rows;
    bitmap.cols     = map->cols;
    bitmap.flow     = map->flow;
    bitmap.pix_bits = 1;
    bitmap.buffer   = map->bitmap;

    return FT_Get_Outline_Bitmap( library, (FT_Outline*)outline, &bitmap );
  }


  /***********************************************************************/
  /*                                                                     */
  /* <Function> TT_Get_Outline_Pixmap                                    */
  /*                                                                     */
  /* <Description>                                                       */
  /*     Renders an outline within a pixmap. The outline's image is      */
  /*     simply or-ed to the target pixmap.                              */
  /*                                                                     */
  /* <Input>                                                             */
  /*     engine  ::   handle to the TrueType driver object               */
  /*     outline ::   pointer to the source outline descriptor           */
  /*     raster  ::   pointer to the target pixmap descriptor            */
  /*                                                                     */
  /* <Return>                                                            */
  /*     TrueType error code. 0 means success.                           */
  /*                                                                     */
  /* <MT-Note>                                                           */
  /*     Yes.                                                            */
  /*                                                                     */

  EXPORT_FUNC
  TT_Error  TT_Get_Outline_Pixmap( TT_Engine       engine,
                                   TT_Outline*     outline,
                                   TT_Raster_Map*  map )
  {
    FT_Library     library = (FT_Library)engine;
    FT_Raster_Map  bitmap;

    if ( !engine )
      return TT_Err_Invalid_Engine;

    if ( !outline || !map )
      return TT_Err_Invalid_Argument;

    bitmap.width    = map->width;
    bitmap.rows     = map->rows;
    bitmap.cols     = map->cols;
    bitmap.flow     = map->flow;
    bitmap.pix_bits = 8;
    bitmap.buffer   = map->bitmap;

    return FT_Get_Outline_Bitmap( library, (FT_Outline*)outline, &bitmap );
  }




  /***********************************************************************/
  /*                                                                     */
  /* <Function> TT_New_Outline                                           */
  /*                                                                     */
  /* <Description>                                                       */
  /*    Creates a new outline of a given size.                           */
  /*                                                                     */
  /* <Input>                                                             */
  /*    numPoints ::  maximum number of points within the outline        */
  /*                                                                     */
  /*    numContours :: maximum number of contours within the outline     */
  /*                                                                     */
  /* <Output>                                                            */
  /*    outline  :: target outline descriptor.                           */
  /*                                                                     */
  /* <Return>                                                            */
  /*    TrueType error code. 0 means success                             */
  /*                                                                     */
  /* <MT-Note>                                                           */
  /*    Yes.                                                             */
  /*                                                                     */
  /* <Note>                                                              */
  /*    This function uses 'malloc' to allocate the outline's array,     */
  /*    _unlike_ all other functions in the TrueType API. This means     */
  /*    that the outline won't be destroyed when the TrueType engine     */
  /*    is finalized..                                                   */
  /*                                                                     */
  /*    It is provided for backwards compatibility ONLY. Use the new     */
  /*    FT_New_Outline function defined in "ftoutln.h" instead if you're */
  /*    working with the FreeType 2.0 API.                               */
  /*                                                                     */

#include <stdlib.h>  /* for malloc and free */

  static  TT_Outline  null_api_outline = { 0, 0, NULL, NULL, NULL, 
                                           0, 0, 0, 0 };

  EXPORT_FUNC
  TT_Error  TT_New_Outline( TT_UShort    numPoints,
                            TT_Short     numContours,
                            TT_Outline*  outline )
  {
    if ( !outline )
      return TT_Err_Invalid_Argument;

    *outline = null_api_outline;

    if ( numPoints > 0 && numContours > 0 )
    {
      outline->points   = (TT_Vector*)malloc( numPoints * sizeof(TT_Vector) );
      outline->flags    = (TT_Byte*)  malloc( numPoints * sizeof(TT_Char)   );
      outline->contours = (TT_UShort*)malloc( numPoints * sizeof(TT_UShort) );
    
      if ( !outline->points || !outline->flags || !outline->contours )
        goto Fail;

      outline->n_points   = numPoints;
      outline->n_contours = numContours;
      outline->owner      = TRUE;
    }
    return TT_Err_Ok;

  Fail:
    outline->owner = TRUE;
    TT_Done_Outline( outline );
    return TT_Err_Out_Of_Memory;
  }


  /***********************************************************************/
  /*                                                                     */
  /* <Function> TT_Done_Outline                                          */
  /*                                                                     */
  /* <Description>                                                       */
  /*    Destroys an outline created with FT_New_Outline                  */
  /*                                                                     */
  /* <Input>                                                             */
  /*    outline ::                                                       */
  /*       pointer to the outline destructor to discard. This function   */
  /*       doesn't destroy the TT_Outline sturcture, only the data it    */
  /*       contains/own.                                                 */
  /*                                                                     */
  /* <Return>                                                            */
  /*    TrueType error code. 0 means success                             */
  /*                                                                     */
  /* <MT-Note>                                                           */
  /*    No.                                                              */
  /*                                                                     */
  /* <Note>                                                              */
  /*    This function uses 'free' to discard the outline's arrays.       */
  /*    You should only discard outline allocated with TT_New_Outline.   */
  /*                                                                     */
  /*    It is provided for backwards compatibility ONLY. Use the new     */
  /*    FT_Done_Outline function defined in "ftoutln.h" instead if you're*/
  /*    working with the FreeType 2.0 API.                               */
  /*                                                                     */

  EXPORT_FUNC
  TT_Error  TT_Done_Outline( TT_Outline*  outline )
  {
    if ( outline )
    {
      if ( outline->owner )
      {
        free( outline->points   );
        free( outline->flags    );
        free( outline->contours );
      }
      *outline = null_api_outline;
      return TT_Err_Ok;
    }
    else
      return TT_Err_Invalid_Argument;
  }



  /***********************************************************************/
  /*                                                                     */
  /* <Function> TT_Copy_Outline                                          */
  /*                                                                     */
  /* <Description>                                                       */
  /*     Copy an outline into another one. Both objects must have        */
  /*     the same sizes ( num. points & num. contours ) when this        */
  /*     function is called..                                            */
  /*                                                                     */
  /* <Input>                                                             */
  /*     source  :: handle to source outline                             */
  /*     target  :: handle to target outline                             */
  /*                                                                     */
  /* <Return>                                                            */
  /*     TrueType error code. 0 means success                            */
  /*                                                                     */

  EXPORT_FUNC
  TT_Error  TT_Copy_Outline( TT_Outline*  source,
                             TT_Outline*  target )
  {
    return FT_Copy_Outline( (FT_Outline*)source,
                            (FT_Outline*)target );
  }


/*******************************************************************
 *
 *  Function    :  TT_Transform_Outline
 *
 *  Description :  Applies a simple transformation to an outline.
 *
 *  Input  :  outline     the glyph's outline.  Can be extracted
 *                        from a glyph container through
 *                        TT_Get_Glyph_Outline().
 *
 *            matrix      simple matrix with 16.16 fixed floats
 *
 *  Output :  Error code (always TT_Err_Ok).
 *
 *  MT-Safe : YES!
 *
 ******************************************************************/

  EXPORT_FUNC
  void  TT_Transform_Outline( TT_Outline*  outline,
                              TT_Matrix*   matrix )
  {
    FT_Transform_Outline( (FT_Outline*)outline, (FT_Matrix*)matrix );
  }  

/*******************************************************************
 *
 *  Function    :  TT_Transform_Vector
 *
 *  Description :  Apply a simple transform to a vector
 *
 *  Input  :  x, y        the vector.
 *
 *            matrix      simple matrix with 16.16 fixed floats
 *
 *  Output :  None.
 *
 *  MT-Safe : YES!
 *
 ******************************************************************/

  EXPORT_FUNC
  void  TT_Transform_Vector( TT_F26Dot6*  x,
                             TT_F26Dot6*  y,
                             TT_Matrix*   matrix )
  {
    FT_Transform_Vector( x, y, (FT_Matrix*) matrix );
  }


/*******************************************************************
 *
 *  Function    :  TT_Translate_Outline
 *
 *  Description :  Applies a simple translation.
 *
 *  Input  :  outline   no comment :)
 *            xOffset
 *            yOffset
 *
 *  Output :  Error code.
 *
 *  MT-Safe : YES!
 *
 ******************************************************************/

  EXPORT_FUNC
  void      TT_Translate_Outline( TT_Outline*  outline,
                                  TT_F26Dot6   xOffset,
                                  TT_F26Dot6   yOffset )
  {
    FT_Translate_Outline( (FT_Outline*)outline, xOffset, yOffset );
  }


/*******************************************************************
 *
 *  Function    :  TT_Get_Outline_BBox
 *
 *  Description :  Returns an outline's bounding box.
 *
 *  Input  :  outline   no comment :)
 *            bbox      address where the bounding box is returned
 *
 *  Output :  Error code.
 *
 *  MT-Safe : YES!
 *
 ******************************************************************/

  EXPORT_FUNC
  TT_Error  TT_Get_Outline_BBox( TT_Outline*  outline,
                                 TT_BBox*     bbox )
  {
    return FT_Get_Outline_CBox( (FT_Outline*)outline, (FT_BBox*)bbox );
  }

  /***********************************************************************/
  /*                                                                     */
  /* <Function>                                                          */
  /*                                                                     */
  /* <Description>                                                       */
  /*                                                                     */
  /*                                                                     */
  /* <Input>                                                             */
  /*                                                                     */
  /*                                                                     */
  /* <Output>                                                            */
  /*                                                                     */
  /*                                                                     */
  /* <Return>                                                            */
  /*                                                                     */
  /*                                                                     */
  /* <MT-Note>                                                           */
  /*                                                                     */
  /*                                                                     */
  /*                                                                     */
  /* <Note>                                                              */
  /*                                                                     */
  /*                                                                     */
  /*                                                                     */
  /* Compute A*B/C with 64 bits intermediate precision. */

  EXPORT_FUNC
  TT_Long   TT_MulDiv( TT_Long A, TT_Long B, TT_Long C )
  {
    return FT_MulDiv( A, B, C );
  }

  
  /***********************************************************************/
  /*                                                                     */
  /* <Function>                                                          */
  /*                                                                     */
  /* <Description>                                                       */
  /*                                                                     */
  /*                                                                     */
  /* <Input>                                                             */
  /*                                                                     */
  /*                                                                     */
  /* <Output>                                                            */
  /*                                                                     */
  /*                                                                     */
  /* <Return>                                                            */
  /*                                                                     */
  /*                                                                     */
  /* <MT-Note>                                                           */
  /*                                                                     */
  /*                                                                     */
  /*                                                                     */
  /* <Note>                                                              */
  /*                                                                     */
  /*                                                                     */
  /*                                                                     */
  /* Compute A*B/0x10000 with 64 bits intermediate precision. */
  /* Useful to multiply by a 16.16 fixed float value.         */

  EXPORT_FUNC
  TT_Long   TT_MulFix( TT_Long A, TT_Long B )
  {
    return FT_MulFix( A, B );
  }

  /* ----------------- character mappings support ------------- */

/*******************************************************************
 *
 *  Function    :  TT_Get_CharMap_Count
 *
 *  Description :  Returns the number of charmaps in a given face.
 *
 *  Input  :  face   face object handle
 *
 *  Output :  Number of tables. -1 in case of error (bad handle).
 *
 *  Note   :  DON'T USE THIS FUNCTION! IT HAS BEEN DEPRECATED!
 *
 *            It is retained for backwards compatibility only and will
 *            fail on 16bit systems.
 *
 *  MT-Safe : YES !
 *
 ******************************************************************/

  EXPORT_FUNC
  int  TT_Get_CharMap_Count( TT_Face  face )
  {
    FT_Face  _face = (FT_Face)face;

    return ( _face ? _face->num_charmaps : -1 );
  }


/*******************************************************************
 *
 *  Function    :  TT_Get_CharMap_ID
 *
 *  Description :  Returns the ID of a given charmap.
 *
 *  Input  :  face             face object handle
 *            charmapIndex     index of charmap in directory
 *            platformID       address of returned platform ID
 *            encodingID       address of returned encoding ID
 *
 *  Output :  error code
 *
 *  MT-Safe : YES !
 *
 ******************************************************************/

  EXPORT_FUNC
  TT_Error  TT_Get_CharMap_ID( TT_Face     face,
                               TT_UShort   charmapIndex,
                               TT_UShort*  platformID,
                               TT_UShort*  encodingID )
  {
    TT_CharMap  cmap;

    if ( !face )
      return TT_Err_Invalid_Face_Handle;

    if ( charmapIndex >= face->num_charmaps )
      return TT_Err_Bad_Argument;

    cmap = (TT_CharMap)face->charmaps + charmapIndex;

    *platformID = cmap->root.platform_id;
    *encodingID = cmap->root.encoding_id;

    return TT_Err_Ok;
  }


/*******************************************************************
 *
 *  Function    :  TT_Get_CharMap
 *
 *  Description :  Looks up a charmap.
 *
 *  Input  :  face          face object handle
 *            charmapIndex  index of charmap in directory
 *            charMap       address of returned charmap handle
 *
 *  Output :  Error code.
 *
 *  MT-Safe : YES!
 *
 ******************************************************************/

  EXPORT_FUNC
  TT_Error  TT_Get_CharMap( TT_Face      face,
                            TT_UShort    charmapIndex,
                            TT_CharMap*  charMap )
  {
    if ( !face )
      return TT_Err_Invalid_Face_Handle;

    if ( charmapIndex >= face->num_charmaps )
      return TT_Err_Bad_Argument;

    *charMap = (TT_CharMap)face->charmaps + charmapIndex;
    return TT_Err_Ok;
  }


/*******************************************************************
 *
 *  Function    :  TT_Char_Index
 *
 *  Description :  Returns the glyph index corresponding to
 *                 a given character code defined for the 'charmap'.
 *
 *  Input  :  charMap    charmap handle
 *            charcode   character code
 *
 *  Output :  glyph index.
 *
 *  Notes  :  Character code 0 is the unknown glyph, which should never
 *            be displayed.
 *
 *  MT-Safe : YES!
 *
 ******************************************************************/

  EXPORT_FUNC
  TT_UShort  TT_Char_Index( TT_CharMap  charMap,
                            TT_UShort   charCode )
  {
    return tt_interface->get_char_index( (FT_CharMap)charMap, charCode );
  }


/*******************************************************************
 *
 *  Function    :  TT_Get_Name_Count
 *
 *  Description :  Returns the number of strings found in the
 *                 name table.
 *
 *  Input  :  face   face handle
 *
 *  Output :  number of strings.
 *
 *  Notes  :  Returns -1 on error (invalid handle).
 *
 *            DON'T USE THIS FUNCTION! IT HAS BEEN DEPRECATED!
 *
 *            It is retained for backwards compatibility only and will
 *            fail on 16bit systems.
 *
 *  MT-Safe : YES!
 *
 ******************************************************************/

  EXPORT_FUNC
  int  TT_Get_Name_Count( TT_Face  face )
  {
    if ( !face )
      return -1;

    return face->num_names;
  }


/*******************************************************************
 *
 *  Function    :  TT_Get_Name_ID
 *
 *  Description :  Returns the IDs of the string number 'nameIndex'
 *                 in the name table of a given face.
 *
 *  Input  :  face        face handle
 *            nameIndex   index of string. First is 0
 *            platformID  addresses of returned IDs
 *            encodingID
 *            languageID
 *            nameID
 *
 *  Output :  Error code.
 *
 *  Notes  :  Some files have a corrupt or unusual name table, with some
 *            entries having a platformID > 3.  These can usually
 *            be ignored by a client application.
 *
 *  MT-Safe : YES!
 *
 ******************************************************************/

  EXPORT_FUNC
  TT_Error  TT_Get_Name_ID( TT_Face     face,
                            TT_UShort   nameIndex,
                            TT_UShort*  platformID,
                            TT_UShort*  encodingID,
                            TT_UShort*  languageID,
                            TT_UShort*  nameID )
  {
    TT_NameRec*  name;

    if (!face)
      return TT_Err_Invalid_Face_Handle;
      
    if ( nameIndex >= face->num_names )
      return TT_Err_Bad_Argument;

    name = face->name_table.names + nameIndex;
        
    *platformID = name->platformID;
    *encodingID = name->encodingID;
    *languageID = name->languageID;
    *nameID     = name->nameID;

    return TT_Err_Ok;
  }


/*******************************************************************
 *
 *  Function    :  TT_Get_Name_String
 *
 *  Description :  Returns the address and length of a given
 *                 string found in the name table.
 *
 *  Input  :  face        face handle
 *            nameIndex   string index
 *            stringPtr   address of returned pointer to string
 *            length      address of returned string length
 *
 *  Output :  Error code.
 *
 *  Notes  :  If the string's platformID is invalid,
 *            stringPtr is NULL, and length is 0.
 *
 *  MT-Safe : YES!
 *
 ******************************************************************/

  EXPORT_FUNC
  TT_Error  TT_Get_Name_String( TT_Face      face,
                                TT_UShort    nameIndex,
                                TT_String**  stringPtr,
                                TT_UShort*   length )
  {
    TT_NameRec* name;
    
    if (!face)
      return TT_Err_Invalid_Face_Handle;
      
    if ( nameIndex >= face->num_names )
      return TT_Err_Bad_Argument;

    name = face->name_table.names + nameIndex;

    *stringPtr = (TT_String*)name->string;
    *length    = name->stringLength;        
    
    return TT_Err_Ok;
  }


/*******************************************************************
 *
 *  Function    :  TT_Get_Font_Data
 *
 *  Description :  Loads any font table in client memory.  Used by
 *                 the TT_Get_Font_Data().
 *
 *  Input  :  face     Face object to look for.
 *
 *            tag      Tag of table to load.  Use the value 0 if you
 *                     want to access the whole font file, else set
 *                     this parameter to a valid TrueType table tag
 *                     that you can forge with the MAKE_TT_TAG
 *                     macro.
 *
 *            offset   Starting offset in the table (or the file
 *                     if tag == 0).
 *
 *            buffer   Address of target buffer
 *
 *            length   Address of decision variable:
 *
 *                       if length == NULL:
 *                             Load the whole table.  Returns an
 *                             an error if 'offset' != 0.
 *
 *                       if *length == 0 :
 *                             Exit immediately, returning the
 *                             length of the given table, or of
 *                             the font file, depending on the
 *                             value of 'tag'.
 *
 *                       if *length != 0 :
 *                             Load the next 'length' bytes of
 *                             table or font, starting at offset
 *                             'offset' (in table or font too).
 *
 *  Output :  Error code.
 *
 *  MT-Safe : YES!
 *
 ******************************************************************/

  EXPORT_FUNC
  TT_Error  TT_Get_Font_Data( TT_Face   face,
                              TT_ULong  tag,
                              TT_Long   offset,
                              void*     buffer,
                              TT_Long*  length )
  {
    return tt_extension->get_font_data( face, tag, offset, buffer, length );
  }







  /*******************************************************************/
  /*                                                                 */
  /*                                                                 */
  /*                                                                 */
  /*  Postscript Names extension                                     */
  /*                                                                 */
  /*                                                                 */
  /*                                                                 */
  /*******************************************************************/

  /* Initialise the Postscript Names extension */
  EXPORT_FUNC
  TT_Error TT_Init_Post_Extension( TT_Engine  engine )
  {
    (void)engine;
    return TT_Err_Ok;
  }

  /* Load the Postscript Names table - notice that the 'post' parameter */
  /* will be ignored in 2.0.                                            */
  EXPORT_DEF
  TT_Error TT_Load_PS_Names( TT_Face   face,
                             void*     post )
  {
    (void)post;
    (void)face;

    /* the names are now loaded on demand in 2.0 */
    return TT_Err_Ok;
  }


  /* The following is directly implemented in the TrueType driver */
#if 0
  /* Gets the postscript name of a single glyph */
  EXPORT_DEF
  TT_Error TT_Get_PS_Name( TT_Face      face,
                           TT_UShort    index,
                           TT_String**  PSname );
#endif




  /*******************************************************************/
  /*                                                                 */
  /*                                                                 */
  /*                                                                 */
  /*  Embedded Bitmap (sbits) extension                              */
  /*                                                                 */
  /*                                                                 */
  /*                                                                 */
  /*******************************************************************/


  /*************************************************************/
  /*                                                           */
  /* <Function>                                                */
  /*    TT_Init_SBit_Extension                                 */
  /*                                                           */
  /* <Description>                                             */
  /*    Initialise the embedded bitmaps extension for the      */
  /*    FreeType engine.                                       */
  /*                                                           */
  /* <Input>                                                   */
  /*    engine  :: handle to current FreeType library instance */
  /*                                                           */
  /* <Return>                                                  */
  /*    Error code. 0 means success.                           */
  /*                                                           */
  EXPORT_FUNC
  TT_Error  TT_Init_SBit_Extension( TT_Engine  engine )
  {
    (void)engine;
    return TT_Err_Ok;
  }


  /*************************************************************/
  /*                                                           */
  /* <Function>                                                */
  /*    TT_Get_Face_Bitmaps                                    */
  /*                                                           */
  /* <Description>                                             */
  /*    Loads the "EBLC" table from a font file, if any.       */
  /*                                                           */
  /* <Input>                                                   */
  /*    face :: handle to the source TrueType font/face        */
  /*                                                           */
  /* <Output>                                                  */
  /*    eblc_table :: a descriptor for the EBLC table          */
  /*                                                           */
  /* <Return>                                                  */
  /*    Error code. 0 means success.                           */
  /*                                                           */
  /* <Note>                                                    */
  /*    This function returns TT_Err_Table_Missing when the    */
  /*    font contains no embedded bitmaps. All fields in       */
  /*    "eblc_table" will then be set to 0.                    */
  /*                                                           */
  EXPORT_FUNC
  TT_Error  TT_Get_Face_Bitmaps( TT_Face   face,
                                 TT_EBLC  *eblc_table )
  {
    if (!face)
      return FT_Err_Invalid_Face_Handle;

    eblc_table->num_strikes = face->num_sbit_strikes;
    eblc_table->strikes     = face->sbit_strikes;

    if ( face->num_sbit_strikes > 0 )
    {
      eblc_table->version = 0x00002000;
      return TT_Err_Ok;
    }

    eblc_table->version = 0;
    return TT_Err_Table_Missing;
  }


  /*************************************************************/
  /*                                                           */
  /* <Function>                                                */
  /*    TT_New_SBit_Image                                      */
  /*                                                           */
  /* <Description>                                             */
  /*    Allocates a new embedded bitmap container              */
  /*                                                           */
  /* <Output>                                                  */
  /*    image :: sbit image                                    */
  /*                                                           */
  /* <Return>                                                  */
  /*    Error code. 0 means success.                           */
  /*                                                           */
  EXPORT_DEF
  TT_Error  TT_New_SBit_Image( TT_SBit_Image*  *image )
  {
    *image = (TT_SBit_Image*)malloc( sizeof(**image) );
    if ( !*image )
      return TT_Err_Out_Of_Memory;

    MEM_Set( *image, sizeof(**image), 0 );
    return TT_Err_Ok;
  }


  /*************************************************************/
  /*                                                           */
  /* <Function>                                                */
  /*    TT_Done_SBit_Image                                     */
  /*                                                           */
  /* <Description>                                             */
  /*    Releases an embedded bitmap container                  */
  /*                                                           */
  /* <Input>                                                   */
  /*    image :: sbit image                                    */
  /*                                                           */
  EXPORT_DEF
  void      TT_Done_SBit_Image( TT_SBit_Image*  image )
  {
    free( image->map.bitmap );
    free( image );
  }


  /*************************************************************/
  /*                                                           */
  /* <Function>                                                */
  /*    TT_Load_Glyph_Bitmap                                   */
  /*                                                           */
  /* <Description>                                             */
  /*    Loads a given glyph embedded bitmap                    */
  /*                                                           */
  /* <Input>                                                   */
  /*    face        :: handle to the source TrueType font/face */
  /*    instance    :: current size/transform instance         */
  /*    glyph_index :: index of source glyph                   */
  /*    bitmap      :: target embedded bitmap descriptor       */
  /*                                                           */
  /* <Return>                                                  */
  /*    Error code. 0 means success.                           */
  /*                                                           */
  /* <Note>                                                    */
  /*    This function returns an error if there is no          */
  /*    embedded bitmap for the glyph at the given             */
  /*    instance.                                              */
  /*                                                           */
  EXPORT_FUNC
  TT_Error  TT_Load_Glyph_Bitmap( TT_Face         face,
                                  TT_Instance     instance,
                                  TT_UShort       glyph_index,
                                  TT_SBit_Image*  bitmap )
  {
    FT_GlyphSlot  slot;
    TT_Error      error;

    error = FT_New_GlyphSlot( (FT_Face)face, &slot );
    if (error) goto Exit;

    error = FT_Load_Glyph( slot, (FT_Size)instance, glyph_index,
                           FT_LOAD_NO_OUTLINE, 0 );
    if (!error)
    {
      /* copy bitmap */
      bitmap->map.width  = slot->bitmap.width;
      bitmap->map.rows   = slot->bitmap.rows;
      bitmap->map.cols   = slot->bitmap.cols;
      bitmap->map.flow   = slot->bitmap.flow;
      bitmap->bit_depth  = slot->bitmap.pix_bits;
      bitmap->map.bitmap = slot->bitmap.buffer;

      /* copy metrics */
      bitmap->metrics.bbox.xMin = slot->metrics.horiBearingX >> 6;
      bitmap->metrics.bbox.xMax = bitmap->metrics.bbox.xMin +
                                  (slot->metrics.width >> 6);
      bitmap->metrics.bbox.yMax = slot->metrics.horiBearingY >> 6;
      bitmap->metrics.bbox.yMin = bitmap->metrics.bbox.yMax -
                                  (slot->metrics.height >> 6);

      bitmap->metrics.horiBearingX = bitmap->metrics.bbox.xMin;
      bitmap->metrics.horiBearingY = bitmap->metrics.bbox.yMax;
      bitmap->metrics.horiAdvance  = slot->metrics.horiAdvance >> 6;

      bitmap->metrics.vertBearingX = slot->metrics.vertBearingX >> 6;
      bitmap->metrics.vertBearingY = slot->metrics.vertBearingY >> 6;
      bitmap->metrics.vertAdvance  = slot->metrics.vertAdvance  >> 6;

      slot->bitmap.buffer = 0;
    }

    FT_Done_GlyphSlot( slot );
  Exit:
    return error;
  }

/* END */
