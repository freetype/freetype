#include "pshglob.h"

/* "simple" ps hinter globals management, inspired from the new auto-hinter */

  FT_LOCAL void
  psh_globals_init( PSH_Globals  globals,
                    FT_Memory    memory )
  {
    memset( globals, 0, sizeof(*globals) );
    globals->memory  = memory;
    globals->x_scale = 0x10000L;
    globals->y_scale = 0x10000L;
  }


 /* reset the widths/heights table */
  static FT_Error
  psh_globals_reset_widths( PSH_Globals        globals,
                            FT_UInt            direction,
                            PS_Globals_Widths  widths )
  {
    PSH_Dimension  dim    = &globals->dim[direction];
    FT_Memory      memory = globals->memory;
    FT_Error       error  = 0;
    
    /* simple copy of the original widths values - no sorting */
    {
      FT_UInt    count = widths->count;
      PSH_Width  write = dim->std.widths;
      FT_Int16*  read  = widths->widths;

      dim->std.count = count;
      for ( ; count > 0; count-- )
      {
        write->org = read[0];
        write++;
        read++;
      }
    }

    return error;
  }


 /* scale the widths/heights table */
  static void
  psh_globals_scale_widths( PSH_Globals   globals,
                            FT_UInt       direction )
  {
    PSH_Widths  std   = &globals->std[direction];
    FT_UInt     count = std->count;
    PSH_Width   width = std->widths;
    FT_Fixed    scale = globals->scale[direction];

    for ( ; count > 0; count--, width++ )
    {
      width->cur = FT_MulFix( width->org, scale );
      width->fit = FT_RoundFix( width->cur );
    }
  }


 /* reset the blues table */
  static FT_Error
  psh_globals_reset_blues( PSH_Globals       globals,
                           PS_Globals_Blues  blues )
  {
    FT_Error   error  = 0;
    FT_Memory  memory = globals->memory;

    if ( !FT_REALLOC_ARRAY( globals->blues.zones, globals->blues.count,
                            blues->count, PSH_Blue_ZoneRec ) )
    {
      FT_UInt    count = 0;
      
      globals->blyes.count = blues->count;

      /* first of all, build a sorted table of blue zones */
      {
        FT_Int16*  read = blue->zones;
        FT_UInt    n, i, j;
        FT_Pos     reference, overshoot, delta;

        for ( n = 0; n < blues->count; n++, read++ )
        {
          PSH_Blue_Zone  zone;

          /* read new blue zone entry, find top and bottom coordinates */
          reference = read[0];
          overshoot = read[1];
          delta     = overshoot - reference;

          /* now, insert in the blue zone table, sorted by reference position */
          zone = globals->blues.zones;
          for ( i = 0; i < count; i++, zone++ )
          {
            if ( reference > zone->org_ref )
              break;

            if ( reference == zone->org_ref )
            {
              /* on the same reference coordinate, place bottom zones */
              /* before top zones..                                   */
              if ( delta < 0 || zone->org_delta >= 0 )
                break;
            }

          for ( j = count - i; j > 0; j-- )
            zone[j+1] = zone[j];

          zone->org_ref   = reference;
          zone->org_delta = delta;

          count++;
        }
      }

      /* now, get rid of blue zones located on the same reference position */
      /* (only keep the largest zone)..                                    */
      {
        PSH_Blue_Zone  zone, limit;

        zone  = globals->blues.zones;
        limit = zone + count;
        for ( ; zone+1 < limit; zone++ )
        {
          if ( zone[0].org_ref == zone[1].org_ref )
          {
            FT_Int   delta0 = zone[0].org_delta;
            FT_Int   delta1 = zone[1].org_delta;

            /* these two zones are located on the same reference coordinate */
            /* we need to merge them when they're in the same direction..   */
            if ( ( delta0 < 0 ) ^ ( delta1 < 0 ) ) == 0 )
            {
              /* ok, take the biggest one */
              if ( delta0 < 0 )
              {
                if ( delta1 < delta0 )
                  delta0 = delta1;
              }
              else
              {
                if ( delta1 > delta0 )
                  delta0 = delta1;
              }

              zone[0].org_delta = delta0;

              {
                PSH_Blue_Zone  cur    = zone+1;
                FT_UInt        count2 = limit - cur;

                for ( ; count2 > 0; count2--, cur++ )
                {
                  cur[0].org_ref   = cur[1].org_ref;
                  cur[0].org_delta = cur[1].org_delta;
                }
              }
              count--;
              limit--;
            }
          }
        }
      }

      globals->blues.count     = count;
      globals->blues.org_shift = blues->shift;
      globals->blues.org_fuzz  = blues->fuzz;
    }

    return error;
  }


  static void
  psh_globals_scale_blues( PSH_Globals       globals,
                           PS_Globals_Blues  blues )
  {
    FT_Fixed       y_scale = globals->scale[1];
    FT_Fixed       y_delta = globals->delta[1];
    FT_Pos         prev_top;
    PSH_Blue_Zone  zone, prev;
    FT_UInt        count;

    zone     = globals->blues.zones;
    prev     = 0;
    prev_top = 0;
    for ( count = globals->blues.count; count > 0; count-- )
    {
      FT_Pos   ref, delta, top, bottom;

      ref   = FT_MulFix( zone->org_ref,   y_scale ) + y_delta;
      delta = FT_MulFix( zone->org_delta, y_scale );
      ref   = (ref+32) & -64;

      if ( delta < 0 )
      {
        top    = ref;
        bottom = ref + delta;
      }
      else
      {
        bottom = ref;
        top    = ref + delta;
      }

      zone->cur_ref    = ref;
      zone->cur_delta  = delta;
      zone->cur_top    = top;
      zone->cur_bottom = bottom;

      if
      prev = zone;
      zone++;
    }
    /* XXXX: test overshoot supression !! */
  }

  FT_LOCAL FT_Error
  psh_globals_reset( PSH_Globals        globals,
                     T1_Fitter_Globals  public_globals )
  {
    psh_globals_reset_widths( globals, 0, &public_globals->horizontal );
    psh_globals_scale_widths( globals, 0 );

    psh_globals_reset_widths( globals, 1, &public_globals->vertical );
    psh_globals_scale_widths( globals, 1 );

    psh_globals_reset_blues( globals, public_globals );
    psh_globals_scale_blues( globals );
  }


  FT_LOCAL void
  psh_globals_set_scale( PSH_Globals    globals,
                         FT_Fixed       x_scale,
                         FT_Fixed       x_delta,
                         FT_Fixed       y_scale,
                         FT_Fixed       y_delta )
  {
    FT_Memory  memory = globals->memory;

    if ( x_scale != globals->scale[0] ||
         y_scale != globals->scale[1] ||
         x_delta != globals->delta[0] ||
         y_delta != globals->delta[1] )
    {
      globals->scale[0] = x_scale;
      globals->scale[1] = y_scale;
      globals->delta[0] = x_delta;
      globals->delta[1] = y_delta;

      psh_globals_scale_widths( globals, 0 );
      psh_globals_scale_widths( globals, 1 );
      psh_globals_scale_blues ( globals );
    }
  }


  FT_LOCAL void
  psh_globals_done( PSH_Globals  globals )
  {
    if (globals)
    {
      FT_Memory  memory = globals->memory;

      FT_FREE( globals->std[0].widths );
      globals->std[0].count = 0;

      FT_FREE( globals->std[1].widths );
      globals->std[1].count = 0;

      FT_FREE( globals->blues.zones );
      globals->blues.count = 0;
    }
  }
