#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_INTERNAL_OBJECTS_H
#include "pshglob.h"

/* "simple" ps hinter globals management, inspired from the new auto-hinter */

 /*************************************************************************/
 /*************************************************************************/
 /*****                                                               *****/
 /*****                       STANDARD WIDTHS                         *****/
 /*****                                                               *****/
 /*************************************************************************/
 /*************************************************************************/
 

 /* scale the widths/heights table */
  static void
  psh_globals_scale_widths( PSH_Globals   globals,
                            FT_UInt       direction )
  {
    PSH_Dimension  dim   = &globals->dimension[direction];
    PSH_Widths     std   = &dim->std;
    FT_UInt        count = std->count;
    PSH_Width      width = std->widths;
    FT_Fixed       scale = dim->scale_mult;

    for ( ; count > 0; count--, width++ )
    {
      width->cur = FT_MulFix( width->org, scale );
      width->fit = FT_RoundFix( width->cur );
    }
  }



 /* org_width is is font units, result in device pixels, 26.6 format */
  FT_LOCAL_DEF FT_Pos
  psh_dimension_snap_width( PSH_Dimension  dimension,
                            FT_Int         org_width )
  {
    FT_UInt  n;
    FT_Pos   width     = FT_MulFix( org_width, dimension->scale_mult );
    FT_Pos   best      = 64 + 32 + 2;
    FT_Pos   reference = width;

    for ( n = 0; n < dimension->std.count; n++ )
    {
      FT_Pos  w;
      FT_Pos  dist;

      w = dimension->std.widths[n].cur;
      dist = width - w;
      if ( dist < 0 )
        dist = -dist;
      if ( dist < best )
      {
        best      = dist;
        reference = w;
      }
    }

    if ( width >= reference )
    {
      width -= 0x21;
      if ( width < reference )
        width = reference;
    }
    else
    {
      width += 0x21;
      if ( width > reference )
        width = reference;
    }

    return width;
  }


 /*************************************************************************/
 /*************************************************************************/
 /*****                                                               *****/
 /*****                       BLUE ZONES                              *****/
 /*****                                                               *****/
 /*************************************************************************/
 /*************************************************************************/

  static void
  psh_blues_set_zones_0( PSH_Blues       target,
                         FT_UInt         read_count,
                         FT_Short*       read,
                         PSH_Blue_Table  top_table,
                         PSH_Blue_Table  bot_table )
  {
    FT_UInt    count_top = top_table->count;
    FT_UInt    count_bot = bot_table->count;
    
    for ( ; read_count > 0; read_count -= 2 )
    {
      FT_Int         reference, delta;
      FT_UInt        count;
      PSH_Blue_Zone  zones, zone;
      
      /* read blue zone entry, and select target top/bottom zone */
      reference = read[0];
      delta     = read[1] - reference;
      
      if ( delta >= 0 )
      {
        zones = top_table->zones;
        count = count_top;
      }
      else
      {
        zones = bot_table->zones;
        count = count_bot;
      }
      
      /* insert into sorted table */
      zone = zones;  
      for ( ; count > 0; count--, zone++ )
      {
        if ( reference < zone->org_ref )
          break;
          
        if ( reference == zone->org_ref )
        {
          FT_Int  delta0 = zone->org_delta;

          /* we have two zones on the same reference position */
          /* only keep the largest one..                      */          
          if ( delta < 0 )
          {
            if ( delta < delta0 )
              zone->org_delta = delta;
          }
          else
          {
            if ( delta > delta0 )
              zone->org_delta = delta;
          }
          goto Skip;
        }
      }
      
      for ( ; count > 0; count-- )
        zone[count] = zone[count-1];
        
      zone->org_ref   = reference;
      zone->org_delta = delta;
      
      if ( delta >= 0 )
        count_top ++;
      else
        count_bot ++;
      
    Skip:
      read += 2;
    }    

    top_table->count = count_top;
    bot_table->count = count_bot;
  }                         


 /* re-read blue zones from the original fonts, and store them into out    */
 /* private structure. This function re-orders, sanitizes and fuzz-expands */
 /* the zones as well..                                                    */
  static void
  psh_blues_set_zones( PSH_Blues         target,
                       FT_UInt           count,
                       FT_Short*         blues,
                       FT_UInt           count_others,
                       FT_Short*         other_blues,
                       FT_Int            fuzz,
                       FT_Int            family )
  {
    PSH_Blue_Table  top_table, bot_table;
    FT_Int          count_top, count_bot;
    
    if ( family )
    {
      top_table  = &target->family_top;
      bot_table  = &target->family_bottom;
    }
    else
    {
      top_table  = &target->normal_top;
      bot_table  = &target->normal_bottom;
    }
    
    /* read the input blue zones, and build two sorted tables */
    /* (one for the top zones, the other for the bottom zones */
    top_table->count = 0;
    bot_table->count = 0;
    
    /* first, the blues */
    psh_blues_set_zones_0( target, count, blues, top_table, bot_table );
    psh_blues_set_zones_0( target, count_others, other_blues, top_table, bot_table );
    
    count_top = top_table->count;
    count_bot = bot_table->count;
    
    /* sanitize top table */
    if ( count_top > 0 )
    {
      PSH_Blue_Zone  zone = top_table->zones;
      
      for ( count = count_top-1; count > 0; count--, zone++ )
      {
        FT_Int  delta;
      
        delta = zone[1].org_ref - zone[0].org_ref;
        if ( zone->org_delta > delta )
          zone->org_delta = delta;
          
        zone->org_bottom = zone->org_ref;
        zone->org_top    = zone->org_delta + zone->org_ref;
      }
    }
    
    /* sanitize bottom table */
    if ( count_bot > 0 )
    {
      PSH_Blue_Zone  zone = bot_table->zones;

      for ( count = count_bot-1; count > 0; count--, zone++ )
      {
        FT_Int  delta;
        
        delta = zone[0].org_ref - zone[1].org_ref;
        if ( zone->org_delta < delta )
          zone->org_delta = delta;
          
        zone->org_top    = zone->org_ref;
        zone->org_bottom = zone->org_delta + zone->org_ref;
      }
    }

    /* expand top and bottom tables with blue fuzz */
    {
      FT_Int         dim, top, bot, delta;
      PSH_Blue_Zone  zone;

      zone  = top_table->zones;
      count = count_top;
      
      for ( dim = 1; dim >= 0; dim-- )
      {
        if ( count > 0 )
        {
          /* expand the bottom of the lowest zone normally */
          zone->org_bottom -= fuzz;
          
          /* expand the top and bottom of intermediate zones     */
          /* checking that the interval is smaller than the fuzz */
          top = zone->org_top;
          
          for ( count--; count > 0; count--, zone++ )
          {
            bot   = zone[1].org_bottom;
            delta = bot - top;
            if ( delta < 2*fuzz )
            {
              zone[0].org_top = zone[1].org_bottom = top + delta/2;
            }
            else
            {
              zone[0].org_top    = top + fuzz;
              zone[1].org_bottom = bot - fuzz;
            }
            
            zone++;
            top = zone->org_top;
          }
          
          /* expand the top of the highest zone normally */
          zone->org_top = top + fuzz;
        }
        zone  = bot_table->zones;
        count = count_bot;
      }
    }
  }



 /* reset the blues table when the device transform changes */
  static void
  psh_blues_scale_zones( PSH_Blues  blues,
                         FT_Fixed   scale,
                         FT_Pos     delta )
  {
    FT_UInt         count;
    FT_UInt         num;
    PSH_Blue_Table  table = 0;
    
    for ( num = 0; num < 4; num++ )
    {
      PSH_Blue_Zone  zone;
      
      switch (num)
      {
        case 0:  table = &blues->normal_top; break;
        case 1:  table = &blues->normal_bottom; break;
        case 2:  table = &blues->family_top; break;
        default: table = &blues->family_bottom;
      }
      
      zone  = table->zones;
      count = table->count;
      for ( ; count > 0; count--, zone++ )
      {
        zone->cur_top    = FT_MulFix( zone->org_top, scale    ) + delta;
        zone->cur_bottom = FT_MulFix( zone->org_bottom, scale ) + delta;
        zone->cur_ref    = FT_MulFix( zone->org_ref, scale ) + delta;
        zone->cur_delta  = FT_MulFix( zone->org_delta, scale );
        
        /* round scaled reference position */
        zone->cur_ref = ( zone->cur_ref + 32 ) & -64;
      }
    }
    
    /* XXX: we should process the family / normal tables here !! */
  }


  FT_LOCAL_DEF void
  psh_blues_snap_stem( PSH_Blues            blues,
                       FT_Int               stem_top,
                       FT_Int               stem_bot,
                       PSH_Blue_Alignement  alignment )
  {
    PSH_Blue_Table  table;
    FT_UInt         count;
    PSH_Blue_Zone   zone;
    
    alignment->align = 0;
    
    /* lookup stem top in top zones table */
    table = &blues->normal_top;
    count = table->count;
    zone  = table->zones;
    for ( ; count > 0; count-- )
    {
      if ( stem_top < zone->org_bottom )
        break;
        
      if ( stem_top <= zone->org_top )
      {
        alignment->align    |= PSH_BLUE_ALIGN_TOP;
        alignment->align_top = zone->cur_ref;
        break;
      }
    }

    /* lookup stem bottom in bottom zones table */
    table = &blues->normal_bottom;
    count = table->count;
    zone  = table->zones;
    for ( ; count > 0; count-- )
    {
      if ( stem_bot < zone->org_bottom )
        break;

      if ( stem_bot <= zone->org_top )
      {
        alignment->align    |= PSH_BLUE_ALIGN_BOT;
        alignment->align_bot = zone->cur_ref;
      }
    }
  }


 /*************************************************************************/
 /*************************************************************************/
 /*****                                                               *****/
 /*****                        GLOBAL HINTS                           *****/
 /*****                                                               *****/
 /*************************************************************************/
 /*************************************************************************/

  static void
  psh_globals_destroy( PSH_Globals  globals )
  {
    if (globals)
    {
      FT_Memory  memory;
      
      memory = globals->memory;
      globals->dimension[0].std.count = 0;
      globals->dimension[1].std.count = 0;   
      
      globals->blues.normal_top.count    = 0;
      globals->blues.normal_bottom.count = 0;
      globals->blues.family_top.count    = 0;
      globals->blues.family_bottom.count = 0;
      
      FREE( globals );
    }
  }

  
  static FT_Error
  psh_globals_new( FT_Memory     memory,
                   T1_Private*   priv,
                   PSH_Globals  *aglobals )
  {
    PSH_Globals  globals;
    FT_Error     error;
    
    if ( !ALLOC( globals, sizeof(*globals) ) )
    {
      FT_UInt    count;
      FT_Short*  read;
      
      globals->memory = memory;

      /* copy standard widths */      
      {
        PSH_Dimension  dim   = &globals->dimension[1];
        PSH_Width      write = dim->std.widths;
        
        write->org = priv->standard_width[1];
        write++;
        
        read = priv->snap_widths;
        for ( count = priv->num_snap_widths; count > 0; count-- )
        {
          write->org = *read;
          write++;
          read++;
        }
        
        dim->std.count = write - dim->std.widths;
      }

      /* copy standard heights */
      {
        PSH_Dimension  dim = &globals->dimension[0];
        PSH_Width      write = dim->std.widths;
        
        write->org = priv->standard_height[1];
        write++;
        
        read = priv->snap_heights;
        for ( count = priv->num_snap_heights; count > 0; count-- )
        {
          write->org = *read;
          write++;
          read++;
        }

        dim->std.count = write - dim->std.widths;
      }
        
      /* copy blue zones */        
      psh_blues_set_zones( &globals->blues, priv->num_blue_values,
                           priv->blue_values, priv->num_other_blues,
                           priv->other_blues, priv->blue_fuzz, 0 );
                           
      psh_blues_set_zones( &globals->blues, priv->num_family_blues,
                           priv->family_blues, priv->num_family_other_blues,
                           priv->family_other_blues, priv->blue_fuzz, 1 );

      globals->dimension[0].scale_mult  = 0;
      globals->dimension[0].scale_delta = 0;
      globals->dimension[1].scale_mult  = 0;
      globals->dimension[1].scale_delta = 0;
    }
    
    *aglobals = globals;
    return error;
  }



  static FT_Error
  psh_globals_set_scale( PSH_Globals    globals,
                         FT_Fixed       x_scale,
                         FT_Fixed       y_scale,
                         FT_Fixed       x_delta,
                         FT_Fixed       y_delta )
  {
    PSH_Dimension  dim    = &globals->dimension[0];
    
    dim = &globals->dimension[0];
    if ( x_scale != dim->scale_mult  ||
         x_delta != dim->scale_delta )
    {
      dim->scale_mult  = x_scale;
      dim->scale_delta = x_delta;
      
      psh_globals_scale_widths( globals, 0 );
    }

    dim = &globals->dimension[1];
    if ( y_scale != dim->scale_mult  ||
         y_delta != dim->scale_delta )
    {
      dim->scale_mult  = y_scale;
      dim->scale_delta = y_delta;

      psh_globals_scale_widths( globals, 1 );
      psh_blues_scale_zones( &globals->blues, y_scale, y_delta );
    }

    return 0;
  }


  FT_LOCAL_DEF void
  psh_globals_funcs_init( PSH_Globals_FuncsRec*  funcs )
  {
    funcs->create    = psh_globals_new;
    funcs->set_scale = psh_globals_set_scale;
    funcs->destroy   = psh_globals_destroy;
  }
