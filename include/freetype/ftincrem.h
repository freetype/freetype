#ifndef __FT_INCREMENTAL_H__
#define __FT_INCREMENTAL_H__

#include <ft2build.h>
#include FT_FREETYPE_H

FT_BEGIN_HEADER

 /*************************************************************************
  *
  * @type: FT_Incremental
  *
  * @description:
  *   an opaque type describing a user-provided object used to implement
  *   "incremental" glyph loading within FreeType. This is used to support
  *   embedded fonts in certain environments (e.g. Postscript interpreters),
  *   where the glyph data isn't in the font file, or must be over-ridden by
  *   different values.
  *
  * @note:
  *   it's up to client applications to create and implement @FT_Incremental
  *   objects, as long as they provide implementations for the methods
  *   @FT_Incremental_GetGlyphDataFunc, @FT_Incremental_FreeGlyphDataFunc
  *   and @FT_Incremental_GetGlyphMetricsFunc
  *
  *   see the description of @FT_Incremental_ServiceRec to understand how to
  *   use incremental objects with FreeType.
  */
  typedef struct FT_IncrementalRec_*    FT_Incremental;


 /*************************************************************************
  *
  * @struct: FT_Incremental_Metrics
  *
  * @description:
  *   a small structure used to model the basic glyph metrics returned
  *   by the @FT_Incremental_GetGlyphMetricsFunc method
  *
  * @fields:
  *   bearing_x :: left-most bearing, in font units.
  *   bearing_y :: top-most bearing, in font units.
  *   advance   :: glyph advance, in font units
  *
  * @note:
  *   these correspond to horizontal or vertical metrics depending on the
  *   value of the 'vertical' parameter of the method
  *   @FT_Incremental_GetGlyphMetricsFunc
  */
  typedef struct FT_Incremental_MetricsRec_
  {
    FT_Long  bearing_x;
    FT_Long  bearing_y;
    FT_Long  advance;

  } FT_Incremental_MetricsRec, *FT_Incremental_Metrics;


 /**************************************************************************
  *
  * @type: FT_Incremental_GetGlyphDataFunc
  *
  * @description:
  *   a function called by FreeType to access a given glyph's data bytes
  *   during @FT_Load_Glyph or @FT_Load_Char, when incremental loading is
  *   enable.
  *
  *   note that the format of the glyph's data bytes depends on the font
  *   file format. For TrueType, it must correspond to the raw bytes within
  *   the 'glyf' table. For Postscript formats, it must correspond to the
  *   *unencrypted* charstrings bytes, without any 'lenIV' header. It is
  *   undefined for any other format.
  *
  * @input:
  *   incremental :: handle to an opaque @FT_Incremental handle provided
  *                  by the client application
  *
  *   glyph_index :: index of relevant glyph
  *
  * @output:
  *   adata  :: a structure describing the returned glyph data bytes
  *             (which will be accessed as a read-only byte block)
  *
  * @return:
  *   FreeType error code. 0 means success
  *
  * @note:
  *   if this function returns succesfully, the method
  *   @FT_Incremental_FreeGlyphDataFunc will be called later to "release"
  *   the se bytes.
  *
  *   nested @FT_Incremental_GetGlyphDataFunc can happen, in the case of
  *   compound glyphs !!
  */
  typedef FT_Error (*FT_Incremental_GetGlyphDataFunc)
                      ( FT_Incremental  incremental,
                        FT_UInt         glyph_index,
                        FT_DataRec     *adata );

 /**************************************************************************
  *
  * @type: FT_Incremental_FreeGlyphDataFunc
  *
  * @description:
  *   a function used to "release" the glyph data bytes returned by a
  *   successful call to @FT_Incremental_GetGlyphDataFunc.
  *
  * @input:
  *   incremental :: handle to an opaque @FT_Incremental handle provided
  *                  by the client application
  *
  *   data        :: glyph_index :: index of relevant glyph
  */
  typedef void     (*FT_Incremental_FreeGlyphDataFunc)
                      ( FT_Incremental  incremental,
                        FT_DataRec*     data );


 /**************************************************************************
  *
  * @type: FT_Incremental_GetGlyphMetricsFunc
  *
  * @description:
  *   a function used to retrieve the basic metrics of a given glyph index
  *   before accessing its data. This is necessary because, in certain formats
  *   like TrueType, the metrics are stored in a different place than the
  *   glyph images proper.
  *
  * @input:
  *   incremental :: handle to an opaque @FT_Incremental handle provided
  *                  by the client application
  *
  */
  typedef FT_Error (*FT_Incremental_GetGlyphMetricsFunc)
                      ( FT_Incremental              incremental,
                        FT_UInt                     glyph_index,
                        FT_Bool                     vertical,
                        FT_Incremental_MetricsRec  *ametrics,
                        FT_Bool                    *afound );


 /**************************************************************************
  *
  * @struct: FT_Incremental_ServiceRec
  *
  * @description:
  *   a structure to be used with @FT_Open_Face to indicate that the user
  *   wants to support "incremental" glyph loading. You should use it with
  *   @FT_PARAM_TAG_INCREMENTAL as in the following example:
  *
  *  {
  *    FT_Incremental_ServiceRec  incr_service;
  *    FT_Parameter               parameter;
  *    FT_Open_Args               open_args;
  *
  *    // set up incremental descriptor
  *    incr_service.incremental       = my_object;
  *    incr_service.get_glyph_metrics = my_get_glyph_metrics;
  *    incr_service.get_glyph_data    = my_get_glyph_data;
  *    incr_service.free_glyph_data   = my_free_glyph_data;
  *
  *    // set up optional parameter
  *    parameter.tag  = FT_PARAM_TAG_INCREMENTAL;
  *    parameter.data = &incr_service;
  *
  *    // set up FT_Open_Args structure
  *    open_args.flags      = ft_open_flag_pathname;
  *    open_args.pathname   = my_font_pathname;
  *    open_args.num_params = 1;
  *    open_args.params     = &parameter;   // we use one optional argument
  *
  *    // open the 
  *    error = FT_Open_Face( library, &open_args, index, &face );
  *    ....
  *  }
  */
  typedef struct FT_IncrementalParamsRec_
  {
    FT_Incremental                      incremental;
    FT_Incremental_GetGlyphMetricsFunc  get_glyph_metrics;
    FT_Incremental_GetGlyphDataFunc     get_glyph_data;
    FT_Incremental_FreeGlyphDataFunc    free_glyph_data;
  
  } FT_IncrementalParamsRec, *FT_IncrementalParams;


 /**************************************************************************
  *
  * @constant: FT_PARAM_TAG_INCREMENTAL
  *
  * @description:
  *   a constant used as the tag of @FT_Parameter structures to indicate
  *   an incremental loading object to be used by FreeType
  *
  *   see the node for @FT_IncrementalParamsRec
  */
#define  FT_PARAM_TAG_INCREMENTAL   FT_MAKE_TAG('i','n','c','r')

 /* */  

FT_END_HEADER

#endif /* __FT_INCREMENTAL_H__ */
