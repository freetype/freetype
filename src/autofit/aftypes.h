#ifndef __AFTYPES_H__
#define __AFTYPES_H__

FT_BEGIN_HEADER

 /**************************************************************************/
 /**************************************************************************/
 /*****                                                                *****/
 /*****                D E B U G G I N G                               *****/
 /*****                                                                *****/
 /**************************************************************************/
 /**************************************************************************/

#define xxAF_DEBUG

#ifdef AF_DEBUG

#  include <stdio.h>
#  define AF_LOG( x )  printf ## x

#else

#  define AF_LOG( x )  do ; while ( 0 ) /* nothing */

#endif /* AF_DEBUG */

 /**************************************************************************/
 /**************************************************************************/
 /*****                                                                *****/
 /*****                A N G L E   T Y P E S                           *****/
 /*****                                                                *****/
 /**************************************************************************/
 /**************************************************************************/
 
 /*
  *  Angle type. The auto-fitter doesn't need a very high angular accuracy,
  *  and this allows us to speed up some computations considerably with a
  *  light Cordic algorithm (see afangle.c)
  *
  */

  typedef FT_Int    AF_Angle;

#define  AF_ANGLE_PI     128
#define  AF_ANGLE_2PI    (AF_ANGLE_PI*2)
#define  AF_ANGLE_PI2    (AF_ANGLE_PI/2)
#define  AF_ANGLE_PI4    (AF_ANGLE_PI/4)

 /*
  *  compute the angle of a given 2-D vector
  *
  */
  FT_LOCAL( AF_Angle )
  af_angle( FT_Pos  dx,
            FT_Pos  dy );


 /*
  *  computes "angle2 - angle1", the result is always within
  *  the range [ -AF_ANGLE_PI .. AF_ANGLE_PI-1 ]
  *
  */
  FT_LOCAL( AF_Angle )
  af_angle_diff( AF_Angle  angle1,
                 AF_Angle  angle2 );


 /**************************************************************************/
 /**************************************************************************/
 /*****                                                                *****/
 /*****                O U T L I N E S                                 *****/
 /*****                                                                *****/
 /**************************************************************************/
 /**************************************************************************/

  typedef struct AF_OutlineHintsRec_*  AF_OutlineHints;

  typedef struct AF_GlobalHintsRec_*   AF_GlobalHints;

  typedef struct AF_OutlineRec_
  {
    FT_Memory        memory;
    FT_Face          face;
    FT_OutlineRec    outline;
    FT_UInt          outline_resolution;
    
    FT_Int           advance;
    FT_UInt          metrics_resolution;
    
    AF_OutlineHints  hints;

  } AF_OutlineRec;

 /**************************************************************************/
 /**************************************************************************/
 /*****                                                                *****/
 /*****                G L O B A L   M E T R I C S                     *****/
 /*****                                                                *****/
 /**************************************************************************/
 /**************************************************************************/

  /*
   * the following define global metrics in a _single_ dimension
   *
   * the "blue_refs" and "blue_shoots" arrays are ignored in
   * the horizontal dimension
   */

  typedef struct AF_WidthRec_
  {
    FT_Pos  org;  /* original position/width in font units              */
    FT_Pos  cur;  /* current/scaled position/width in device sub-pixels */
    FT_Pos  fit;  /* current/fitted position/width in device sub-pixels */
  
  } AF_WidthRec, *AF_Width;
  

#define  AF_MAX_WIDTHS     16
#define  AF_MAX_BLUES      32

  typedef struct  AF_GlobalMetricsRec_
  {
    FT_Int       num_widths;
    AF_WidthRec  widths[ AF_MAX_WIDTHS ];

    FT_Fixed     scale;  /* used to scale from org to cur with:   */
    FT_Pos       delta;  /*   x_cur = x_org * scale + delta       */

    /* ignored for horizontal metrics */
    AF_WidthRec  blue_refs  [ AF_MAX_BLUES ];
    AF_WidthRec  blue_shoots[ AF_MAX_BLUES ];

    FT_Bool      control_overshoot;

  } AF_GlobalMetricsRec, *AF_GlobalMetrics;


 /**************************************************************************/
 /**************************************************************************/
 /*****                                                                *****/
 /*****                S C A L E R S                                   *****/
 /*****                                                                *****/
 /**************************************************************************/
 /**************************************************************************/

 /*
  *  A scaler models the target pixel device that will receive the
  *  auto-hinted glyph image
  *
  */
  
  typedef enum
  {
    AF_SCALER_FLAG_NO_HORIZONTAL = 1,  /* disable horizontal hinting */
    AF_SCALER_FLAG_NO_VERTICAL   = 2,  /* disable vertical hinting   */
    AF_SCALER_FLAG_NO_ADVANCE    = 4   /* disable advance hinting    */

  } AF_ScalerFlags;


  typedef struct AF_ScalerRec_
  {
    FT_Face         face;         /* source font face                        */
    FT_Fixed        x_scale;      /* from font units to 1/64th device pixels */
    FT_Fixed        y_scale;      /* from font units to 1/64th device pixels */
    FT_Pos          x_delta;      /* in 1/64th device pixels                 */
    FT_Pos          y_delta;      /* in 1/64th device pixels                 */
    FT_Render_Mode  render_mode;  /* monochrome, anti-aliased, LCD, etc..    */
    FT_UInt32       flags;        /* additionnal control flags, see above    */
  
  } AF_ScalerRec, *AF_Scaler;



 /**************************************************************************/
 /**************************************************************************/
 /*****                                                                *****/
 /*****                S C R I P T S                                   *****/
 /*****                                                                *****/
 /**************************************************************************/
 /**************************************************************************/

 /*
  *  the list of know scripts. Each different script correspond to the
  *  following information:
  *
  *   - a set of Unicode ranges to test wether the face supports the
  *     script
  *
  *   - a specific global analyzer that will compute global metrics
  *     specific to the script.
  *
  *   - a specific hinting routine
  *
  *  all scripts should share the same analysis routine though
  */
  typedef enum
  {
    AF_SCRIPT_LATIN = 0,
    /* add new scripts here */
    
    AF_SCRIPT_MAX   /* do not remove */
  
  } AF_Script;


  typedef struct AF_ScriptClassRec_ const*  AF_ScriptClass;

 /*
  *  root class for script-specific metrics
  */
  typedef struct AF_ScriptMetricsRec_
  {
    AF_ScriptClass        script_class;
    AF_GlobalMetricsRec   horz_metrics;
    AF_GlobalMetricsRec   vert_metrics;

  } AF_ScriptMetricsRec, *AF_ScriptMetrics;


 /* this function parses a FT_Face to compute global metrics for
  * a specific script
  */ 
  typedef FT_Error  (*AF_Script_InitMetricsFunc)( AF_ScriptMetrics  metrics,
                                                  FT_Face           face );

  typedef void      (*AF_Script_ScaleMetricsFunc)( AF_ScriptMetrics  metrics,
                                                   AF_Scaler         scaler );

  typedef void      (*AF_Script_DoneMetricsFunc)( AF_ScriptMetrics  metrics );


  typedef FT_Error  (*AF_Script_InitHintsFunc)( AF_OutlineHints   hints,
                                                AF_Scaler         scaler,
                                                AF_ScriptMetrics  metrics );

  typedef void      (*AF_Script_ApplyHintsFunc)( AF_OutlineHints  hints );
                                                 

  typedef struct AF_Script_UniRangeRec_
  {
    FT_UInt32    first;
    FT_UInt32    last;
  
  } AF_Script_UniRangeRec, *AF_Script_UniRange;
 

  typedef struct AF_ScriptClassRec_
  {
    AF_Script                   script;
    AF_Scipt_UniRange           script_uni_ranges;  /* last must be { 0, 0 } */

    FT_UInt                     script_metrics_size;
    AF_Script_InitMetricsFunc   script_metrics_init;
    AF_Script_ScaleMetricsFunc  script_metrics_scale;
    AF_Script_DoneMetricsFunc   script_metrics_done;

  } AF_ScriptClassRec;



 /**************************************************************************/
 /**************************************************************************/
 /*****                                                                *****/
 /*****                F A C E   G L O B A L S                         *****/
 /*****                                                                *****/
 /**************************************************************************/
 /**************************************************************************/

 /*
  *  models the global hints data for a given face, decomposed into
  *  script-specific items..
  *
  */
  typedef struct AF_FaceGlobalsRec_
  {
    FT_Face               face;
    FT_UInt               glyph_count;    /* same as face->num_glyphs     */
    FT_Byte*              glyph_scripts;  /* maps each gindex to a script */
    
    FT_ScriptMetrics      metrics[ AF_SCRIPT_MAX ];

  } AF_FaceGlobalsRec, *AF_FaceGlobals;
 
/* */

FT_END_HEADER

#endif /* __AFTYPES_H__ */
