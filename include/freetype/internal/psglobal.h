#ifndef __PSGLOBALS_H__
#define __PSGLOBALS_H__

 /**********************************************************************/
 /**********************************************************************/
 /*****                                                            *****/
 /*****                  PUBLIC STRUCTURES & API                   *****/
 /*****                                                            *****/
 /**********************************************************************/
 /**********************************************************************/

 /****************************************************************
  *
  * @constant: PS_GLOBALS_MAX_BLUE_ZONES
  *
  * @description:
  *   the maximum number of blue zones in a font global hints
  *   structure. See @PS_Globals_BluesRec
  */                                             
#define  PS_GLOBALS_MAX_BLUE_ZONES  16

 /****************************************************************
  *
  * @constant: PS_GLOBALS_MAX_STD_WIDTHS
  *
  * @description:
  *   the maximum number of standard and snap widths in either the
  *   horizontal or vertical direction. See @PS_Globals_WidthsRec
  */                                             
#define  PS_GLOBALS_MAX_STD_WIDTHS  16

 /****************************************************************
  *
  * @type: PS_Globals
  *
  * @description:
  *   a handle to a @PS_GlobalsRec structure used to
  *   describe the global hints of a given font
  */                                             
  typedef struct PS_GlobalsRec_*    PS_Globals;
                                             
 /****************************************************************
  *
  * @struct: PS_Globals_BluesRec
  *
  * @description:
  *   a structure used to model the global blue zones of a given
  *   font
  *
  * @fields:
  *   count        :: number of blue zones
  *   zones        :: an array of (count*2) coordinates describing the zones
  *
  *   count_family :: number of family blue zones
  *   zones_family :: an array of (count_family*2) coordinates describing
  *                   the family blue zones
  *
  *   scale  :: the blue scale to be used (fixed float)
  *   shift  :: the blue shift to be used
  *   fuzz   :: the blue fuzz to be used
  *
  * @note:
  *   each blue zone is modeled by a (reference,overshoot) coordinate pair
  *   in the table. zones can be placed in any order..
  */                                             
  typedef struct PS_Globals_BluesRec
  {                    
    FT_UInt    count;
    FT_Int16   zones[ 2*PS_GLOBALS_MAX_BLUE_ZONES ];
    
    FT_UInt    count_family;
    FT_Int16   zones_family[ 2*PS_GLOBALS_MAX_BLUE_ZONES ];

    FT_Fixed   scale;
    FT_Int16   shift;
    FT_Int16   fuzz;
  
  } PS_Globals_BluesRec, *PS_Globals_Blues;


 /****************************************************************
  *
  * @type: PS_Global_Widths;
  *
  * @description:
  *   a handle to a @PS_Globals_WidthsRec structure used to model
  *   the global standard and snap widths in a given direction
  */                                             
  typedef struct PS_Globals_WidthsRec_*  PS_Globals_Widths;
  

 /****************************************************************
  *
  * @struct: PS_Globals_WidthsRec
  *
  * @description:
  *   a structure used to model the global standard and snap widths
  *   in a given font
  *
  * @fields:
  *   count  :: number of widths
  *   widths :: an array of 'count' widths in font units.
  *
  * @note:
  *   'widths[0]' must be the standard width or height, while
  *   remaining elements of the array are snap widths or heights
  */                                             
  typedef struct PS_Globals_WidthsRec_
  {                     
    FT_UInt    count;
    FT_Int16   widths[ PS_GLOBALS_MAX_STD_WIDTHS ];
  
  } PS_Globals_WidthsRec;

  
 /****************************************************************
  *
  * @struct: PS_Globals_GlobalsRec
  *
  * @description:
  *   a structure used to model the global hints for a given font
  *
  * @fields:
  *   horizontal :: horizontal widths
  *   vertical   :: vertical heights
  *   blues      :: blue zones
  */                                             
  typedef struct PS_GlobalsRec_
  {
    PS_Globals_WidthsRec  horizontal;
    PS_Globals_WidthsRec  vertical;
    PS_Globals_BluesRec   blues;
  
  } PS_GlobalsRec;
 

 /* */

#endif /* __PS_GLOBALS_H__ */
