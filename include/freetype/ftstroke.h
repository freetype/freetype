/***************************************************************************/
/*                                                                         */
/*  ftstroke.h                                                             */
/*                                                                         */
/*    FreeType path stroker (specification).                               */
/*                                                                         */
/*  Copyright 2002, 2003 by                                                */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __FT_STROKE_H__
#define __FT_STROKE_H__

#include <ft2build.h>
#include FT_OUTLINE_H

FT_BEGIN_HEADER

/*@*************************************************************
 *
 * @type: FT_Stroker
 *
 * @description:
 *    opaque handler to a path stroker object
 */
  typedef struct FT_StrokerRec_*    FT_Stroker;


/*@*************************************************************
 *
 * @enum: FT_Stroker_LineJoin
 *
 * @description:
 *    these values determine how two joining lines are rendered
 *    in a stroker.
 *
 * @values:
 *    FT_STROKER_LINEJOIN_ROUND ::
 *      used to render rounded line joins. circular arcs are used
 *      to join two lines smoothly
 *
 *    FT_STROKER_LINEJOIN_BEVEL ::
 *      used to render beveled line joins; i.e. the two joining lines
 *      are extended until they intersect
 *
 *    FT_STROKER_LINEJOIN_MITER ::
 *      same as beveled rendering, except that an additional line
 *      break is added if the angle between the two joining lines
 *      is too closed (this is useful to avoid unpleasant spikes
 *      in beveled rendering).
 */
  typedef enum
  {
    FT_STROKER_LINEJOIN_ROUND = 0,
    FT_STROKER_LINEJOIN_BEVEL,
    FT_STROKER_LINEJOIN_MITER

  } FT_Stroker_LineJoin;


/*@*************************************************************
 *
 * @enum: FT_Stroker_LineCap
 *
 * @description:
 *    these values determine how the end of opened sub-paths are
 *    rendered in a stroke
 *
 * @values:
 *    FT_STROKER_LINECAP_BUTT ::
 *      the end of lines is rendered as a full stop on the last
 *      point itself
 *
 *    FT_STROKER_LINECAP_ROUND ::
 *      the end of lines is rendered as a half-circle around the
 *      last point
 *
 *    FT_STROKER_LINECAP_SQUARE ::
 *      the end of lines is rendered as a square around the
 *      last point
 */
  typedef enum
  {
    FT_STROKER_LINECAP_BUTT = 0,
    FT_STROKER_LINECAP_ROUND,
    FT_STROKER_LINECAP_SQUARE

  } FT_Stroker_LineCap;


/**************************************************************
 *
 * @enum: FT_StrokerBorder
 *
 * @description:
 *    theses values are used to select a given stroke border
 *    in @FT_Stroker_GetBorderCounts and @FT_Stroker_ExportBorder
 *
 * @values:
 *    FT_STROKER_BORDER_LEFT ::
 *      select the left border, relative to the drawing direction
 *
 *    FT_STROKER_BORDER_RIGHT ::
 *      select the right border, relative to the drawing direction
 *
 * @note:
 *   applications are generally interested in the "inside" and "outside"
 *   borders. However, there is no direct mapping between these and
 *   the "left" / "right" ones, since this really depends on the glyph's
 *   drawing orientation, which varies between font formats
 *
 *   you can however use @FT_Outline_GetInsideBorder and
 *   @FT_Outline_GetOutsideBorder to get these.
 */
  typedef enum
  {
    FT_STROKER_BORDER_LEFT = 0,
    FT_STROKER_BORDER_RIGHT
  
  } FT_StrokerBorder;


/**************************************************************
 *
 * @function: FT_Outline_GetInsideBorder
 *
 * @description:
 *    retrieve the @FT_StrokerBorder value corresponding to the
 *    "inside" borders of a given outline
 *
 * @input:
 *    outline :: source outline handle
 *
 * @return:
 *    border index. @FT_STROKER_BORDER_LEFT for empty or invalid outlines
 */
  FT_EXPORT( FT_StrokerBorder )
  FT_Outline_GetInsideBorder( FT_Outline*  outline );


/**************************************************************
 *
 * @function: FT_Outline_GetOutsideBorder
 *
 * @description:
 *    retrieve the @FT_StrokerBorder value corresponding to the
 *    "outside" borders of a given outline
 *
 * @input:
 *    outline :: source outline handle
 *
 * @return:
 *    border index. @FT_STROKER_BORDER_LEFT for empty or invalid outlines
 */
  FT_EXPORT( FT_StrokerBorder )
  FT_Outline_GetOutsideBorder( FT_Outline*  outline );

 
/**************************************************************
 *
 * @function: FT_Stroker_New
 *
 * @description:
 *    create a new stroker object
 *
 * @input:
 *    memory :: memory manager handle
 *
 * @output:
 *    new stroker object handle, NULL in case of error
 *
 * @return:
 *    error code. 0 means success
 */
  FT_EXPORT( FT_Error )
  FT_Stroker_New( FT_Memory    memory,
                  FT_Stroker  *astroker );


/**************************************************************
 *
 * @function: FT_Stroker_Set
 *
 * @description:
 *    reset a stroker object's attributes
 *
 * @input:
 *    stroker     :: target stroker handle
 *    radius      :: border radius
 *    line_cap    :: line cap style
 *    line_join   :: line join style
 *    miter_limit :: miter limit for the FT_STROKER_LINEJOIN_MITER style,
 *                   expressed as 16.16 fixed point value.
 * @note:
 *   the radius is expressed in the same units that the outline coordinates.
 */
  FT_EXPORT( void )
  FT_Stroker_Set( FT_Stroker           stroker,
                  FT_Fixed             radius,
                  FT_Stroker_LineCap   line_cap,
                  FT_Stroker_LineJoin  line_join,
                  FT_Fixed             miter_limit );


/**************************************************************
 *
 * @function: FT_Stroker_ParseOutline
 *
 * @description:
 *    a convenient function used to parse a whole outline with
 *    the stroker. The resulting outline(s) can be retrieved
 *    later by functions like @FT_Stroker_GetCounts and @FT_Stroker_Export
 *
 * @input:
 *    stroker     :: target stroker handle
 *    outline     :: source outline
 *    opened      :: boolean. if TRUE, the outline is treated as an open path,
 *                   instead of a closed one
 *
 * @return:*
 *   error code. 0 means success
 *
 * @note:
 *   if 'opened' is 0 (the default), the outline is treated as a closed path,
 *   and the stroker will generate two distinct "border" outlines
 *
 *   if 'opened' is 1, the outline is processed as an open path, and the
 *   stroker will generate a single "stroke" outline
 */
  FT_EXPORT( FT_Error )
  FT_Stroker_ParseOutline( FT_Stroker   stroker,
                           FT_Outline*  outline,
                           FT_Bool      opened );

/**************************************************************
 *
 * @function: FT_Stroker_BeginSubPath
 *
 * @description:
 *    start a new sub-path in the stroker
 *    
 * @input:
 *    stroker     :: target stroker handle
 *    to          :: pointer to start vector
 *    open        :: boolean. if TRUE, the sub-path is treated as an open
 *                   one
 *
 * @return:*
 *   error code. 0 means success
 *
 * @note:
 *   this function is useful when you need to stroke a path that is
 *   not stored as a @FT_Outline object
 */
  FT_EXPORT( FT_Error )
  FT_Stroker_BeginSubPath( FT_Stroker  stroker,
                           FT_Vector*  to,
                           FT_Bool     open );

/**************************************************************
 *
 * @function: FT_Stroker_EndSubPath
 *
 * @description:
 *    close the current sub-path in the stroker
 *    
 * @input:
 *    stroker     :: target stroker handle
 *
 * @return:
 *   error code. 0 means success
 *
 * @note:
 *   you should call this function after @FT_Stroker_BeginSubPath.
 *   if the subpath was not "opened", this function will "draw" a
 *   single line segment to the start position when needed.
 */
  FT_EXPORT( FT_Error )
  FT_Stroker_EndSubPath( FT_Stroker  stroker );


/**************************************************************
 *
 * @function: FT_Stroker_LineTo
 *
 * @description:
 *    "draw" a single line segment in the stroker's current sub-path,
 *    from the last position
 *    
 * @input:
 *    stroker     :: target stroker handle
 *    to          :: pointer to destination point
 *
 * @return:
 *   error code. 0 means success
 *
 * @note:
 *   you should call this function between @FT_Stroker_BeginSubPath and
 *   @FT_Stroker_EndSubPath
 */
  FT_EXPORT( FT_Error )
  FT_Stroker_LineTo( FT_Stroker  stroker,
                     FT_Vector*  to );

/**************************************************************
 *
 * @function: FT_Stroker_ConicTo
 *
 * @description:
 *    "draw" a single quadratic bezier in the stroker's current sub-path,
 *    from the last position
 *    
 * @input:
 *    stroker     :: target stroker handle
 *    control     :: pointer to bezier control point
 *    to          :: pointer to destination point
 *
 * @return:
 *   error code. 0 means success
 *
 * @note:
 *   you should call this function between @FT_Stroker_BeginSubPath and
 *   @FT_Stroker_EndSubPath
 */
  FT_EXPORT( FT_Error )
  FT_Stroker_ConicTo( FT_Stroker  stroker,
                      FT_Vector*  control,
                      FT_Vector*  to );

/**************************************************************
 *
 * @function: FT_Stroker_CubicTo
 *
 * @description:
 *    "draw" a single cubic bezier in the stroker's current sub-path,
 *    from the last position
 *    
 * @input:
 *    stroker     :: target stroker handle
 *    control1    :: pointer to first bezier control point
 *    control2    :: pointer to second bezier control point
 *    to          :: pointer to destination point
 *
 * @return:
 *   error code. 0 means success
 *
 * @note:
 *   you should call this function between @FT_Stroker_BeginSubPath and
 *   @FT_Stroker_EndSubPath
 */
  FT_EXPORT( FT_Error )
  FT_Stroker_CubicTo( FT_Stroker  stroker,
                      FT_Vector*  control1,
                      FT_Vector*  control2,
                      FT_Vector*  to );


/**************************************************************
 *
 * @function: FT_Stroker_GetBorderCounts
 *
 * @description:
 *    call this function once you finished parsing your paths
 *    with the stroker. It will return the number of points and
 *    contours necessary to export one of the "border" or "stroke"
 *    outlines generated by the stroker.
 *    
 * @input:
 *    stroker  :: target stroker handle
 *    border   :: border index
 *
 * @output:
 *    anum_points   :: number of points
 *    anum_contours :: number of contours
 *
 * @return:
 *   error code. 0 means success
 *
 * @note:
 *   when an outline, or a sub-path, is "closed", the stroker generates
 *   two independent 'border' outlines, named 'left' and 'right'
 *   
 *   when the outline, or a sub-path, is "opened", the stroker merges
 *   the 'border' outlines with caps. The 'left' border receives all
 *   points, while the 'right' border becomes empty.
 *
 *   use the function @FT_Stroker_GetCounts instead if you want to
 *   retrieve the counts associated to both borders.
 */
  FT_EXPORT( FT_Error )
  FT_Stroker_GetBorderCounts( FT_Stroker        stroker,
                              FT_StrokerBorder  border,
                              FT_UInt          *anum_points,
                              FT_UInt          *anum_contours );

/**************************************************************
 *
 * @function: FT_Stroker_ExportBorder
 *
 * @description:
 *    call this function after @FT_Stroker_GetBorderCounts to
 *    export the corresponding border to your own @FT_Outline
 *    structure.
 *    
 *    note that this function will append the border points and
 *    contours to your outline, but will not try to resize its
 *    arrays.
 *
 * @input:
 *    stroker  :: target stroker handle
 *    border   :: border index
 *    outline  :: target outline handle
 *
 * @return:
 *   error code. 0 means success
 *
 * @note:
 *   always call this function after @FT_Stroker_GetBorderCounts to
 *   get sure that there is enough room in your @FT_Outline object to
 *   receive all new data.
 *
 *   when an outline, or a sub-path, is "closed", the stroker generates
 *   two independent 'border' outlines, named 'left' and 'right'
 *   
 *   when the outline, or a sub-path, is "opened", the stroker merges
 *   the 'border' outlines with caps. The 'left' border receives all
 *   points, while the 'right' border becomes empty.
 *
 *   use the function @FT_Stroker_Export instead if you want to
 *   retrieve all borders at once
 */
  FT_EXPORT( void )
  FT_Stroker_ExportBorder( FT_Stroker        stroker,
                           FT_StrokerBorder  border,
                           FT_Outline*       outline );

/**************************************************************
 *
 * @function: FT_Stroker_GetCounts
 *
 * @description:
 *    call this function once you finished parsing your paths
 *    with the stroker. It will return the number of points and
 *    contours necessary to export all points/borders from the stroked
 *    outline/path.
 *    
 * @input:
 *    stroker  :: target stroker handle
 *
 * @output:
 *    anum_points   :: number of points
 *    anum_contours :: number of contours
 *
 * @return:
 *   error code. 0 means success
 *
 * @note:
 */
  FT_EXPORT( FT_Error )
  FT_Stroker_GetCounts( FT_Stroker  stroker,
                        FT_UInt    *anum_points,
                        FT_UInt    *anum_contours );

/**************************************************************
 *
 * @function: FT_Stroker_ExportBorder
 *
 * @description:
 *    call this function after @FT_Stroker_GetBorderCounts to
 *    export the corresponding border to your own @FT_Outline
 *    structure.
 *    
 *    note that this function will append the border points and
 *    contours to your outline, but will not try to resize its
 *    arrays.
 *
 * @input:
 *    stroker  :: target stroker handle
 *    border   :: border index
 *    outline  :: target outline handle
 *
 * @return:
 *   error code. 0 means success
 *
 * @note:
 *   always call this function after @FT_Stroker_GetBorderCounts to
 *   get sure that there is enough room in your @FT_Outline object to
 *   receive all new data.
 *
 *   when an outline, or a sub-path, is "closed", the stroker generates
 *   two independent 'border' outlines, named 'left' and 'right'
 *   
 *   when the outline, or a sub-path, is "opened", the stroker merges
 *   the 'border' outlines with caps. The 'left' border receives all
 *   points, while the 'right' border becomes empty.
 *
 *   use the function @FT_Stroker_Export instead if you want to
 *   retrieve all borders at once
 */
  FT_EXPORT( void )
  FT_Stroker_Export( FT_Stroker   stroker,
                     FT_Outline*  outline );

/**************************************************************
 *
 * @function: FT_Stroker_Done
 *
 * @description:
 *    destroy a stroker object
 *
 * @input:
 *    stroker :: stroker handle. can be NULL
 */
  FT_EXPORT( void )
  FT_Stroker_Done( FT_Stroker  stroker );

 /* */

FT_END_HEADER

#endif /* __FT_STROKE_H__ */


/* END */
