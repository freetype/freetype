#ifndef __AFLATIN_H__
#define __AFLATIN_H__

#include "afhints.h"

FT_BEGIN_HEADER
 
 /* 
  * the latin-specific script class
  *
  */
  FT_LOCAL( const FT_ScriptClassRec )    af_latin_script_class;

 /*
  * the following declarations could be embedded in the file "aflatin.c"
  * they've been made semi-public to allow alternate script hinters to
  * re-use some of them
  */

 /*
  *  Latin (global) metrics management
  *
  */
  
#define  AF_LATIN_MAX_WIDTHS     16
#define  AF_LATIN_MAX_BLUES      32

  typedef struct AF_LatinAxisRec_
  {
    FT_Fixed     scale;
    FT_Pos       delta;
    
    FT_UInt      width_count;
    AF_WidthRec  widths[ AF_LATIN_MAX_WIDTHS ];
    
   /* ignored for horizontal metrics */
    FT_Bool      control_overshoot;
    FT_UInt      blue_count;
    AF_WidthRec  blue_refs  [ AF_MAX_BLUES ];
    AF_WidthRec  blue_shoots[ AF_MAX_BLUES ];
    
  } AF_LatinAxisRec, *AF_LatinAxis;
  
  typedef struct AF_LatinMetricsRec_
  {
    AF_OutlineMetricsRec  root;
    AF_LatinAxisRec       axis[ AF_DIMENSION_MAX ];
  
  } AF_LatinMetricsRec, *AF_LatinMetrics;


  FT_LOCAL( FT_Error )
  af_latin_metrics_init( AF_LatinMetrics  metrics,
                         FT_Face          face );

  FT_LOCAL( void )
  af_latin_metrics_scale( AF_LatinMetrics  metrics,
                          AF_Scaler        scaler );


 /* 
  *  Latin (glyph) hints management
  *
  */

  FT_LOCAL( 

  FT_LOCAL( void )
  af_latin_hints_compute_segments( AF_OutlineHints  hints,
                                   AF_Dimension     dim );

  FT_LOCAL( void )
  af_latin_hints_link_segments( AF_OutlineHints  hints,
                                AF_Dimension     dim );

  FT_LOCAL( void )
  af_latin_hints_compute_edges( AF_OutlineHints  hints,
                                AF_Dimension     dim );

  FT_LOCAL( void )
  af_latin_hints_init( AF_OutlineHints  hints,
                                  AF_Dimension     dim );


/* */

FT_END_HEADER

#endif /* __AFLATIN_H__ */
