#include "pshoptim.h"


#ifdef DEBUG_VIEW
  void
  ps_simple_scale( PSH_Hint_Table  table,
                   FT_Fixed        scale,
                   FT_Fixed        delta,
                   FT_Bool         vertical )
  {
    PSH_Hint  hint;
    FT_UInt   count;
  
    for ( count = 0; count < table->num_hints; count++ )
    {
      hint = table->sort[count];
      if ( psh_hint_is_active(hint) )
      {
        hint->cur_pos = FT_MulFix( hint->org_pos, scale ) + delta;
        hint->cur_len = FT_MulFix( hint->org_len, scale );
        
        if (ps_debug_hint_func)
          ps_debug_hint_func( hint, vertical );
      }
    }
  }                
#endif

  FT_LOCAL_DEF  FT_Error
  psh_hint_table_optimize( PSH_Hint_Table  table,
                           PSH_Globals     globals,
                           FT_Outline*     outline,
                           FT_Bool         vertical )
  {
    PSH_Dimension  dim   = &globals->dimension[vertical];
    FT_Fixed       scale = dim->scale_mult;
    FT_Fixed       delta = dim->scale_delta;

#ifdef DEBUG_VIEW
    if ( ps_debug_no_vert_hints && vertical )
    {
      ps_simple_scale( table, scale, delta, vertical );
      return 0;
    }
      
    if ( ps_debug_no_horz_hints && !vertical )
    {
      ps_simple_scale( table, scale, delta, vertical );
      return 0;
    }
#endif

    /* XXXX: for now, we only scale the hints to test all other aspects */
    /*       of the Postscript Hinter..                                 */
    {  
      PSH_Hint  hint;
      FT_UInt   count;
    
      for ( count = 0; count < table->num_hints; count++ )
      {
        hint = table->sort[count];
        if ( psh_hint_is_active(hint) )
        {
# if 1
          FT_Pos   pos = FT_MulFix( hint->org_pos, scale ) + delta;
          FT_Pos   len = FT_MulFix( hint->org_len, scale );
          
          FT_Pos   fit_center;
          FT_Pos   fit_len;
          
          PSH_Blue_AlignementRec  align;

          /* compute fitted width/height */
          fit_len = psh_dimension_snap_width( dim, hint->org_len );
          if ( fit_len < 64 )
            fit_len = 64;
          else
            fit_len = (fit_len + 16 ) & -64;
            
          hint->cur_len = fit_len;
            
          /* check blue zones for horizontal stems */
          align.align = 0;
          if (!vertical)
          {
            psh_blues_snap_stem( &globals->blues,
                                  hint->org_pos + hint->org_len,
                                  hint->org_pos,
                                  &align );
          }

          switch (align.align)
          {
            case PSH_BLUE_ALIGN_TOP:
              {
                /* the top of the stem is aligned against a blue zone */
                hint->cur_pos = align.align_top - fit_len;
                break;
              }
              
            case PSH_BLUE_ALIGN_BOT:
              {
                /* the bottom of the stem is aligned against a blue zone */
                hint->cur_pos = align.align_bot;
                break;
              }
              
            case PSH_BLUE_ALIGN_TOP | PSH_BLUE_ALIGN_BOT:
              {
                /* both edges of the stem are aligned against blue zones */
                hint->cur_pos = align.align_bot;
                hint->cur_len = align.align_top - align.align_bot;
              }
              break;
              
            default:
              /* normal processing */
              if ( (fit_len/64) & 1 )
              {
                /* odd number of pixels */
                fit_center = ((pos + (len >> 1)) & -64) + 32;
              }
              else
              {
                /* even number of pixels */
                fit_center = (pos + (len >> 1) + 32) & -64;
              }
              
              hint->cur_pos = fit_center - (fit_len >> 1);
          }
# else
          hint->cur_pos = (FT_MulFix( hint->org_pos, scale ) + delta + 32) & -64;
          hint->cur_len =  FT_MulFix( hint->org_len, scale );
# endif          

#ifdef DEBUG_VIEW
        if (ps_debug_hint_func)
          ps_debug_hint_func( hint, vertical );
#endif        
        }
      }
    }
    
    return 0;
  }
