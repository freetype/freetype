#ifndef __PSRECORD_H_
#define __PSRECORD_H__

#include FT_INTERNAL_POSTSCRIPT_HINTS_H

FT_BEGIN_HEADER

 /**********************************************************************/
 /**********************************************************************/
 /*****                                                            *****/
 /*****                 GLYPH HINTS RECORDER INTERNALS             *****/
 /*****                                                            *****/
 /**********************************************************************/
 /**********************************************************************/

 /* handle to hint record */
  typedef struct PS_HintRec_*   PS_Hint;

 /* hint flags */
  typedef enum
  {
    PS_HINT_FLAG_ACTIVE = 1,
    PS_HINT_FLAG_GHOST  = 2,
    PS_HINT_FLAG_BOTTOM = 4

  } PS_Hint_Flags;

 /* hint descriptor */
  typedef struct PS_HintRec_
  {
    FT_Int    org_pos;
    FT_Int    org_len;
    FT_UInt   flags;

#if 0
    FT_Pos    cur_pos;
    FT_Pos    cur_len;

    PS_Hint   parent;

    FT_Fixed  interp_scale;
    FT_Fixed  interp_delta;
#endif

  } PS_HintRec;


#define  ps_hint_is_active(x)  ((x)->flags & PS_HINT_FLAG_ACTIVE)
#define  ps_hint_is_ghost(x)   ((x)->flags & PS_HINT_FLAG_GHOST)
#define  ps_hint_is_bottom(x)  ((x)->flags & PS_HINT_FLAG_BOTTOM)


 /* hints table descriptor */
  typedef struct PS_Hint_TableRec_
  {
    FT_UInt   num_hints;
    FT_UInt   max_hints;
    PS_Hint   hints;

  } PS_Hint_TableRec, *PS_Hint_Table;


 /* hint and counter mask descriptor */
  typedef struct PS_MaskRec_
  {
    FT_UInt    num_bits;
    FT_UInt    max_bits;
    FT_Byte*   bytes;
    FT_UInt    end_point;

  } PS_MaskRec, *PS_Mask;


 /* masks and counters table descriptor */
  typedef struct PS_Mask_TableRec_
  {
    FT_UInt    num_masks;
    FT_UInt    max_masks;
    PS_Mask    masks;

  } PS_Mask_TableRec, *PS_Mask_Table;


 /* dimension-specific hints descriptor */
  typedef struct PS_DimensionRec_
  {
    PS_Hint_TableRec  hints;
    PS_Mask_TableRec  masks;
    PS_Mask_TableRec  counters;

  } PS_DimensionRec, *PS_Dimension;


 /* magic value used within PS_HintsRec */
#define PS_HINTS_MAGIC  0x68696e74   /* "hint" */


 /* glyph hints descriptor */
  typedef struct PS_HintsRec_
  {
    FT_Memory          memory;
    FT_Error           error;
    FT_UInt32          magic;
    PS_Hint_Type       hint_type;
    PS_DimensionRec    dimension[2];

  } PS_HintsRec;

 /* */

FT_END_HEADER

#endif /* __T1_FITTER_HINTS_H__ */
