#ifndef __PS_HINTER_GLOBALS_H__
#define __PS_HINTER_GLOBALS_H__

FT_BEGIN_HEADER

 /**********************************************************************/
 /**********************************************************************/
 /*****                                                            *****/
 /*****                    GLOBAL HINTS INTERNALS                  *****/
 /*****                                                            *****/
 /**********************************************************************/
 /**********************************************************************/

 /* blue zone descriptor */
  typedef struct PSH_Blue_ZoneRec_
  {
    FT_Int    org_ref;
    FT_Int    org_delta;
    FT_Pos    cur_ref;
    FT_Pos    cur_delta;
    FT_Pos    cur_bottom;
    FT_Pos    cur_top;

  } PSH_Blue_ZoneRec, *PSH_Blue_Zone;


 /* blue zones table */
  typedef struct PSH_BluesRec_
  {
    FT_UInt           count;
    PSH_Blue_ZoneRec  zones[ PS_GLOBALS_MAX_BLUE_ZONES ];
    
    FT_UInt           count_family;
    PSH_Blue_ZoneRec  zones_family[ PS_GLOBALS_MAX_BLUE_ZONES ];
    
    FT_Fixed          scale;
    FT_Int            org_shift;
    FT_Int            org_fuzz;
    FT_Pos            cur_shift;
    FT_Pos            cur_fuzz;

  } PSH_BluesRec, *PSH_Blues;


 /* standard and snap width */
  typedef struct PSH_WidthRec_
  {
    FT_Int  org;
    FT_Pos  cur;
    FT_Pos  fit;

  } PSH_WidthRec;


 /* standard and snap widths table */
  typedef struct PSH_WidthsRec_
  {
    FT_UInt       count;
    PSH_WidthRec  widths[ PS_GLOBALS_MAX_STD_WIDTHS ];

  } PSH_WidthsRec, *PSH_Widths;


  typedef struct PSH_DimensionRec_
  {
    PSH_WidthsRec  std;
    FT_Fixed       scale_mult;
    FT_Fixed       scale_delta;
  
  } PSH_DimensionRec, *PSH_Dimension;

 /* font globals */
  typedef struct PSH_GlobalsRec_
  {
    FT_Memory         memory;
    PSH_DimensionRec  dimension[2];
    PSH_BluesRec      blues;

  } PSH_GlobalsRec, *PSH_Globals;


 /* initialise font globals */
  FT_LOCAL void
  psh_globals_init( PSH_Globals  globals,
                    FT_Memory    memory );

 /* reset font globals to new values */
  FT_LOCAL FT_Error
  psh_globals_reset( PSH_Globals   globals_hinter,
                     PS_Globals    globals );

 /* change current scale transform */
  FT_LOCAL void
  psh_globals_set_scale( PSH_Globals    globals_hinter,
                         FT_Fixed       x_scale,
                         FT_Fixed       x_delta,
                         FT_Fixed       y_scale,
                         FT_Fixed       y_delta );

 /* finalize font globals */
  FT_LOCAL void
  psh_globals_done( PSH_Globals  globals );

 /* */

FT_END_HEADER

#endif __T1_FITTER_GLOBALS_H__
