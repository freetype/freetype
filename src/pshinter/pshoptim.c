#include "pshoptim.h"

  FT_LOCAL_DEF  FT_Error
  psh_hint_table_optimize( PSH_Hint_Table  table,
                           PSH_Globals     globals,
                           FT_Outline*     outline,
                           FT_Bool         vertical )
  {
    PSH_Dimension  dim   = &globals->dimension[vertical];
    FT_Fixed       scale = dim->scale_mult;
    FT_Fixed       delta = dim->scale_delta;

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
          hint->cur_pos = FT_MulFix( hint->org_pos, scale ) + delta;
          hint->cur_len = FT_MulFix( hint->org_len, scale );
        }
      }
    }
    return 0;
  }
