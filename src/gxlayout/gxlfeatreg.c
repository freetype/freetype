/***************************************************************************/
/*                                                                         */
/*  gxlfeatreg.c                                                           */
/*                                                                         */
/*    High level interface for the font feature registry(body)             */
/*                                                                         */
/*  Copyright 2003 by                                                      */
/*  Masatake YAMATO and Redhat K.K.                                        */
/*                                                                         */
/*  This file may only be used,                                            */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

/***************************************************************************/
/* Development of the code in this file is support of                      */
/* Information-technology Promotion Agency, Japan.                         */
/***************************************************************************/

#include "gxlfeatreg.h"
#include "gxltypes.h"
#include "gxaccess.h"

#undef  FT_COMPONENT
#define FT_COMPONENT  trace_gxlayout

FT_LOCAL_DEF( FT_Error )
gxl_feature_registry_fill_feature(GX_Feature_Registry featreg,
				  FT_Memory memory,
				  GXL_Feature feature)
{
  FT_Error error = GX_Err_Ok;
  FT_Int i;
  
  feature->value 	  = gx_feature_registry_get_value( featreg );
  feature->request        = NULL; /* for debug */
  feature->name.string  = gx_feature_registry_get_name( featreg );
  if ( !feature->name.string )
    {
      FT_ERROR(("Broken feature is used to lookup feature registry\n"));
      error = GX_Err_Invalid_Argument;
      goto Exit;
    }
  FT_TRACE3(("Name from feature registry: %s\n", feature->name.string));
  feature->name.index   = 0; /* for debug */
  feature->nSettings    = gx_feature_registry_count_setting ( featreg );
  if ( FT_NEW_ARRAY( feature->setting, feature->nSettings ) )
    goto Exit;

  feature->exclusive.exclusive = gx_feature_registry_is_setting_exclusive ( featreg );
  
  for ( i = 0; i < feature->nSettings; i++ )
    {
      if ( feature->exclusive.exclusive )
	feature->setting[i].value 	   = i;
      else
	{
	  feature->setting[i].value 	   = 2*i;
	  if ( i != 0)
	    feature->setting[i].value = gx_feat_setting_off ( feature->setting[i].value );
	}      
      feature->setting[i].name.string  = gx_feature_registry_get_setting_name( featreg, i );
      if ( ! feature->setting[i].name.string )
	{
	  FT_ERROR(("Broken setting is used to lookup feature registry: feature=%s\n",
		    feature->name.string));
	  error = GX_Err_Invalid_Argument;
	  goto Failure;
	}
      feature->setting[i].name.index   = 0; /* for debug */
      feature->setting[i].feature = feature;
    }
  if ( feature->exclusive.exclusive )
    feature->exclusive.setting = &feature->setting[0];
  else
    feature->exclusive.setting = NULL;
 Exit:      
  return error;
 Failure:
  FT_FREE( feature->setting );
  return error;
}

/* END */
