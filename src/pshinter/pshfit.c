#include <ft2build.h>
#include FT_INTERNAL_OBJECTS_H
#include FT_INTERNAL_DEBUG_H
#include "pshfit.h"
#include "pshoptim.h"
 
 /* return true iff two stem hints overlap */
  static FT_Int
  psh_hint_overlap( PSH_Hint  hint1,
                    PSH_Hint  hint2 )
  {
    return ( hint1->org_pos + hint1->org_len >= hint2->org_pos &&
             hint2->org_pos + hint2->org_len >= hint1->org_pos );
  }
 
 
 /* destroy hints table */
  static void
  psh_hint_table_done( PSH_Hint_Table  table,
                       FT_Memory       memory )
  {
    FREE( table->zones );
    table->num_zones = 0;
    table->zone      = 0;
    
    FREE( table->sort );
    FREE( table->hints );
    table->num_hints   = 0;
    table->max_hints   = 0;
    table->sort_global = 0;
  }


 /* deactivate all hints in a table */
  static void
  psh_hint_table_deactivate( PSH_Hint_Table  table )
  {
    FT_UInt   count = table->max_hints;
    PSH_Hint  hint  = table->hints;
    
    for ( ; count > 0; count--, hint++ )
    {
      psh_hint_deactivate(hint);
      hint->order = -1;
    }
  }


 /* internal function used to record a new hint */
  static void
  psh_hint_table_record( PSH_Hint_Table  table,
                         FT_UInt         index )
  {
    PSH_Hint  hint = table->hints + index;

    if ( index >= table->max_hints )
    {
      FT_ERROR(( "%s.activate: invalid hint index %d\n", index ));
      return;
    }
        
    /* ignore active hints */
    if ( psh_hint_is_active(hint) )
      return;
    
    psh_hint_activate(hint);
    
    /* now scan the current active hint set in order to determine */
    /* if we're overlapping with another segment..                */
    {
      PSH_Hint*  sorted = table->sort_global;
      FT_UInt    count  = table->num_hints;
      PSH_Hint   hint2;

      hint->parent = 0;      
      for ( ; count > 0; count--, sorted++ )
      {
        hint2 = sorted[0];
        
        if ( psh_hint_overlap( hint, hint2 ) )
        {
          hint->parent = hint2;
          break;
        }
      }
    }
    
    if ( table->num_hints < table->max_hints )
      table->sort_global[ table->num_hints++ ] = hint;
    else
    {
      FT_ERROR(( "%s.activate: too many sorted hints !! BUG !!\n",
                 "ps.fitter" ));
    }
  }


  static void
  psh_hint_table_record_mask( PSH_Hint_Table  table,
                              PS_Mask         hint_mask )
  {
    FT_Int    mask = 0, val = 0;
    FT_Byte*  cursor = hint_mask->bytes;
    FT_UInt   index, limit;

    limit = hint_mask->num_bits; 
    
    if ( limit != table->max_hints )
    {
      FT_ERROR(( "%s.activate_mask: invalid bit count (%d instead of %d)\n",
                 "ps.fitter", hint_mask->num_bits, table->max_hints ));
    }
        
    for ( index = 0; index < limit; index++ )
    {
      if ( mask == 0 )
      {
        val  = *cursor++;
        mask = 0x80;
      }
      
      if ( val & mask )
        psh_hint_table_record( table, index );
        
      mask >>= 1;
    }
  }


 /* create hints table */
  static FT_Error
  psh_hint_table_init( PSH_Hint_Table  table,
                       PS_Hint_Table   hints,
                       PS_Mask_Table   hint_masks,
                       PS_Mask_Table   counter_masks,
                       FT_Memory       memory )
  {
    FT_UInt   count = hints->num_hints;
    FT_Error  error;

    FT_UNUSED(counter_masks);
    
    /* allocate our tables */
    if ( ALLOC_ARRAY( table->sort,  2*count,   PSH_Hint    ) ||
         ALLOC_ARRAY( table->hints,   count,   PSH_HintRec ) ||
         ALLOC_ARRAY( table->zones, 2*count+1, PSH_ZoneRec ) )
      goto Exit;
    
    table->max_hints   = count;
    table->sort_global = table->sort + count;
    table->num_hints   = 0;
    table->num_zones   = 0;
    table->zone        = 0;
    
    /* now, initialise the "hints" array */
    {
      PSH_Hint  write = table->hints;
      PS_Hint   read  = hints->hints;
      
      for ( ; count > 0; count--, write++, read++ )
      {
        write->org_pos = read->pos;
        write->org_len = read->len;
        write->flags   = read->flags;
      }
    }

    /* we now need to determine the initial "parent" stems, first  */
    /* activate the hints that are given by the initial hint masks */
    if ( hint_masks )
    {
      FT_UInt  count = hint_masks->num_masks;
      PS_Mask  mask  = hint_masks->masks;

      table->hint_masks = hint_masks;
      
      for ( ; count > 0; count--, mask++ )
        psh_hint_table_record_mask( table, mask );
    }
    
    /* now, do a linear parse in case some hints were left alone */
    if ( table->num_hints != table->max_hints )
    {
      FT_UInt   index, count;
      
      FT_ERROR(( "%s.init: missing/incorrect hint masks !!\n" ));
      count = table->max_hints;
      for ( index = 0; index < count; index++ )
        psh_hint_table_record( table, index );
    }    
    
  Exit:
    return error;
  }



  static void
  psh_hint_table_activate_mask( PSH_Hint_Table  table,
                                PS_Mask         hint_mask )
  {
    FT_Int    mask = 0, val = 0;
    FT_Byte*  cursor = hint_mask->bytes;
    FT_UInt   index, limit, count;

    limit = hint_mask->num_bits; 
    count = 0;

    psh_hint_table_deactivate( table );
    
    for ( index = 0; index < limit; index++ )
    {
      if ( mask == 0 )
      {
        val  = *cursor++;
        mask = 0x80;
      }
      
      if ( val & mask )
      {
        PSH_Hint  hint = &table->hints[index];
        
        if ( !psh_hint_is_active(hint) )
        {
          PSH_Hint*  sort   = table->sort;
          FT_UInt    count2;
          PSH_Hint   hint2;
          
          for ( count2 = count; count2 > 0; count2--, sort++ )
          {
            hint2 = sort[0];
            if ( psh_hint_overlap( hint, hint2 ) )
            {
              FT_ERROR(( "%s.activate_mask: found overlapping hints\n",
                         "psf.hint" ));
              break;
            }
          }
          
          if ( count2 == 0 )
          {
            psh_hint_activate( hint );
            if ( count < table->max_hints )
              table->sort[count++] = hint;
            else
            {
              FT_ERROR(( "%s.activate_mask: too many active hints\n",
                         "psf.hint" ));
            } 
          }
        }
      }
        
      mask >>= 1;
    }
    table->num_hints = count;
    
    /* now, sort the hints, they're guaranteed to not overlap */
    /* so we can compare their "org_pos" field directly..     */
    {
      FT_Int     i1, i2;
      PSH_Hint   hint1, hint2;
      PSH_Hint*  sort = table->sort;

      /* a simple bubble sort will do, since in 99% of cases, the hints */
      /* will be already sorted.. and the sort will be linear           */
      for ( i1 = 1; i1 < (FT_Int)count; i1++ )
      {
        hint1 = sort[i1];
        for ( i2 = i1-1; i2 >= 0; i2-- )
        {
          hint2 = sort[i2];
          if ( hint2->org_pos < hint1->org_pos )
            break;
            
          sort[i1] = hint2;
          sort[i2] = hint1;
        }
      }
    }
  }


#define  PSH_ZONE_MIN  -3200000
#define  PSH_ZONE_MAX  +3200000


#define xxDEBUG_ZONES

#ifdef DEBUG_ZONES

#include <stdio.h>

  static void
  print_zone( PSH_Zone  zone )
  {
    printf( "zone [scale,delta,min,max] = [%.3f,%.3f,%d,%d]\n",
             zone->scale/65536.0,
             zone->delta/64.0,
             zone->min,
             zone->max );
  }

#else
#  define  print_zone(x)   do { } while (0)
#endif

 /* setup interpolation zones once the hints have been grid-fitted */
 /* by the optimizer..                                             */
  static void
  psh_hint_table_setup_zones( PSH_Hint_Table  table,
                              FT_Fixed        scale,
                              FT_Fixed        delta )
  {
    FT_UInt   count;
    PSH_Zone  zone;
    PSH_Hint *sort, hint, hint2;
    
    zone  = table->zones;
    
    /* special case, no hints defined */
    if ( table->num_hints == 0 )
    {
      zone->scale = scale;
      zone->delta = delta;
      zone->min   = PSH_ZONE_MIN;
      zone->max   = PSH_ZONE_MAX;
      table->num_zones = 1;
      return;
    }
    
    /* the first zone is before the first hint */
    /* x' = (x-x0)*s + x0' = x*s + ( x0' - x0*s ) */
    sort  = table->sort;
    hint  = sort[0];
    
    zone->scale = scale;
    zone->delta = hint->cur_pos - FT_MulFix( hint->org_pos, scale );
    zone->min   = PSH_ZONE_MIN;
    zone->max   = hint->org_pos;
    
    print_zone( zone );
    
    zone++;
    
    for ( count = table->num_hints; count > 0; count-- )
    {
      FT_Fixed  scale2;

      if ( hint->org_len > 0 )
      {
        /* setup a zone for inner-stem interpolation */
        /* (x' - x0') = (x - x0)*(x1'-x0')/(x1-x0)   */
        /* x' = x*s2 + x0' - x0*s2                   */
        
        scale2      = FT_DivFix( hint->cur_len, hint->org_len );
        zone->scale = scale2;
        zone->min   = hint->org_pos;
        zone->max   = hint->org_pos + hint->org_len;
        zone->delta = hint->cur_pos - FT_MulFix( zone->min, scale2 );

        print_zone( zone );
    
        zone++;
      }
      
      if ( count == 1 )
        break;
        
      sort++;
      hint2 = sort[0];
      
      /* setup zone for inter-stem interpolation */
      /* (x'-x1') = (x-x1)*(x2'-x1')/(x2-x1)     */
      /* x' = x*s3 + x1' - x1*s3                 */
      scale2 = FT_DivFix( hint2->cur_pos - (hint->cur_pos + hint->cur_len),
                          hint2->org_pos - (hint->org_pos + hint->org_len) );
      zone->scale = scale2;
      zone->min   = hint->org_pos + hint->org_len;
      zone->max   = hint2->org_pos;
      zone->delta = hint->cur_pos + hint->cur_len - FT_MulFix( zone->min, scale2 );

      print_zone( zone );
    
      zone++;
      
      hint  = hint2;
    }

    /* the last zone */
    zone->scale = scale;
    zone->min   = hint->org_pos + hint->org_len;
    zone->max   = PSH_ZONE_MAX;
    zone->delta = hint->cur_pos + hint->cur_len - FT_MulFix( zone->min, scale );

    print_zone( zone );
    
    zone++;
    
    table->num_zones = zone - table->zones;
    table->zone      = table->zones;
  }


 /* tune a single coordinate with the current interpolation zones */  
  static FT_Pos
  psh_hint_table_tune_coord( PSH_Hint_Table  table,
                             FT_Int          coord )
  {
    PSH_Zone   zone;
    
    zone = table->zone;
      
    if ( coord < zone->min )
    {
      do
      {
        if ( zone == table->zones )
          break;
          
        zone--;
      }
      while ( coord < zone->min );
      table->zone = zone;
    }
    else if ( coord > zone->max )
    {
      do
      {
        if ( zone == table->zones + table->num_zones - 1 )
          break;
          
        zone++;
      }
      while ( coord > zone->max );
      table->zone = zone;
    }
        
    return FT_MulFix( coord, zone->scale ) + zone->delta;
  }


 /* tune a given outline with current interpolation zones */
 /* the function only works in a single dimension..       */
  static void
  psh_hint_table_tune_outline( PSH_Hint_Table  table,
                               FT_Outline*     outline,
                               PSH_Globals     globals,
                               FT_Bool         vertical )

  {
    FT_UInt        count, first, last;
    PS_Mask_Table  hint_masks = table->hint_masks;
    PS_Mask        mask;
    PSH_Dimension  dim        = &globals->dimension[vertical];
    FT_Fixed       scale      = dim->scale_mult;
    FT_Fixed       delta      = dim->scale_delta;
    
    if ( hint_masks && hint_masks->num_masks > 0 )
    {
      first = 0;
      mask  = hint_masks->masks;
      count = hint_masks->num_masks;
      for ( ; count > 0; count--, mask++ )
      {
        last = mask->end_point;
        
        if ( last > first )
        {
          FT_Vector*   vec;
          FT_Int       count2;
          
          psh_hint_table_activate_mask( table, mask );
          psh_hint_table_optimize( table, globals, outline, vertical );
          psh_hint_table_setup_zones( table, scale, delta );
          last = mask->end_point;
          
          vec    = outline->points + first;
          count2 = last - first;
          for ( ; count2 > 0; count2--, vec++ )
          {
            FT_Pos  x, *px;
            
            px  = vertical ? &vec->y : &vec->x;
            x   = *px;
            
            *px = psh_hint_table_tune_coord( table, (FT_Int)x );
          }
        }
          
        first = last;
      }
    }
    else    /* no hints in this glyph, simply scale the outline */
    {
      FT_Vector*  vec;
      
      vec   = outline->points;
      count = outline->n_points;
      
      if ( vertical )
      {
        for ( ; count > 0; count--, vec++ )
          vec->y = FT_MulFix( vec->y, scale ) + delta;
      }
      else
      {
        for ( ; count > 0; count--, vec++ )
          vec->x = FT_MulFix( vec->x, scale ) + delta;
      }
    }
  }
  
  
  


  
  
  
  
  
  FT_LOCAL_DEF FT_Error
  ps_hints_apply( PS_Hints     ps_hints,
                  FT_Outline*  outline,
                  PSH_Globals  globals )
  {
    PSH_Hint_TableRec  hints;
    FT_Error           error;
    FT_Int             dimension;
    
    for ( dimension = 1; dimension >= 0; dimension-- )
    {
      PS_Dimension  dim = &ps_hints->dimension[dimension];
      
      /* initialise hints table */
      memset( &hints, 0, sizeof(hints) );
      error = psh_hint_table_init( &hints,
                                   &dim->hints,
                                   &dim->masks,
                                   &dim->counters,
                                   ps_hints->memory );
      if (error) goto Exit;
      
      psh_hint_table_tune_outline( &hints,
                                   outline,
                                   globals,
                                   dimension );
                                   
      psh_hint_table_done( &hints, ps_hints->memory );
    }
    
  Exit:
    return error;                                   
  }                   