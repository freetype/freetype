/***************************************************************************/
/*                                                                         */
/*  otdemo.c                                                               */
/*                                                                         */
/*    Demo program for FreeType/OpenType font driver implementation (body).*/
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

#include <ft2build.h>
#include FT_LAYOUT_H
#include FT_OTLAYOUT_H
#include <stdio.h>
#include <stdlib.h>
#include <popt.h>

/* KANA specific implementation */

/* Script */
#define OT_SCRIPT_KANA         FT_MAKE_TAG('k', 'a', 'n', 'a')

/* Language */
#define OT_LANGUAGE_JAPANESE   FT_MAKE_TAG('J', 'A', 'N', ' ')

/* Feature */
#define OT_FEATURE_VERT        FT_MAKE_TAG('v', 'e', 'r', 't')
#define OT_FEATURE_VRT2        FT_MAKE_TAG('v', 'r', 't', '2')

/* Property bit */
enum {
	OT_VERT = 1 << 0,
	OT_VRT2 = 1 << 1
};

/* Property */
enum {
	OT_VERT_P = (~OT_VERT) & (~OT_VRT2)
};

void doit(FT_Face face);

int
main(int argc, char ** argv)
{
  FT_Library library;
  FT_Face    face;
  poptContext optCon;
  int rc;
  const char * file;
  static int arg_memprof = 0;

  struct poptOption optTable [] = {
    { "memprof", 'm',  POPT_ARG_NONE, &arg_memprof, 0, "Enter to infinite loop to run under memprof", ""},
    POPT_AUTOHELP
    POPT_TABLEEND
  };

  FTL_EngineType engine_type;

  optCon = poptGetContext(argv[0], argc, (const char **)argv, optTable, 0);
  poptSetOtherOptionHelp(optCon, "[options] otfont\n");      
  rc = poptReadDefaultConfig (optCon, 0);  
  if (rc < 0)
    {
      fprintf(stderr, "Fail to read .popt config file: %s\n",
	      poptStrerror(rc));
      exit (1);
    }
  while ((rc = poptGetNextOpt(optCon)) > 0) 
    if (rc != -1)
      {
	fprintf(stderr, "bad argument %s: %s\n",
		poptBadOption(optCon, POPT_BADOPTION_NOALIAS),
		poptStrerror(rc));
	exit (1);
      }

  if (FT_Init_FreeType (&library))
    {
      fprintf(stderr, "Error in %s\n", "FT_Init_FreeType");
      exit (1);
    }

  file = poptGetArg(optCon);
  if (!file)
    {
      poptPrintHelp(optCon, stderr, 0);
      exit(1);
    }

  if ( FT_New_Face (library, file, 0, &face) )
    {
      fprintf(stderr, "Error in %s: %s\n", "FT_New_Face", file);
      exit (1);
    }


  if ( !( face->face_flags & FT_FACE_FLAG_GLYPH_SUBSTITUTION) )
    {
      fprintf(stderr, "Abort: No substitution table for the face\n");
      exit(1);
    }
  
  if ( FTL_Query_EngineType( face, &engine_type )
       ||  ( engine_type != FTL_OPENTYPE_ENGINE ) )
    {
      fprintf(stderr, "No OT table is existed: %s\n", file);
      exit ( 1 );
    }

  doit( face );
  
  if ( FT_Done_Face ( face ) )
    fprintf(stderr, "Error in %s: %s\n", "FT_Done_Face", file);
  if ( FT_Done_FreeType  (library) )
    {
      fprintf(stderr, "Error in %s\n", "FT_Done_FreeType");
      exit(1);
    }
  if ( arg_memprof || getenv("_MEMPROF_SOCKET") )
    {
      fprintf(stderr, "Enter infinite loop for memprof\n");
      while (1);
    }
  return 0;
} 

void
doit(FT_Face face)
{
  FTL_FeaturesRequest request;
  FT_UInt script_index, feature_index;
  FT_Memory memory = face->stream->memory;
  FTL_Glyphs_Array in, out;
  FT_Int i;
  if ( FTL_New_FeaturesRequest( face, &request ) )
    {
      fprintf(stderr, "error: %s\n", "FTL_New_FeaturesRequest");
      return ;
    }
  if ( !OTL_FeaturesRequest_Find_Script ( (OTL_FeaturesRequest)request,
					  OT_TABLE_GSUB,
					  OT_SCRIPT_KANA,
					  &script_index ) )
    {
      fprintf(stderr, "cannot find script: %s\n", 
	      "OTL_FeaturesRequest_Find_Script");      
      return ;
    }

  if ( !OTL_FeaturesRequest_Find_Feature( (OTL_FeaturesRequest)request,
					  OT_TABLE_GSUB,
					  OT_FEATURE_VERT,
					  script_index,
					  0xffff, /* default language system */
					  &feature_index ) )
    {
      fprintf(stderr, "cannot find feature: %s\n", 
	      "OTL_FeaturesRequest_Find_Feature");
      return ;
    }
  OTL_FeaturesRequest_Add_Feature ( (OTL_FeaturesRequest)request,
				    OT_TABLE_GSUB,
				    feature_index,
				    OT_VERT );
  
  if ( FTL_New_Glyphs_Array ( memory, &in )
       || FTL_New_Glyphs_Array ( memory, &out ) )
    {
      fprintf(stderr, "mem error: %s\n", "FTL_New_Glyphs_Array");
      return ;
    }
  if ( FTL_Set_Glyphs_Array_Length ( in, 14751 ) )
    {
      fprintf(stderr, "mem error: %s\n", 
	      "FTL_Set_Glyphs_Array_Length");
      return ;
    }

  for ( i = 0; i < 14751; i++ )
    {
      in->glyphs[i].gid     = i;
      in->glyphs[i].ot_prop = OT_VERT_P;
    }
  FTL_Activate_FeaturesRequest( request );
  FTL_Substitute_Glyphs ( face, in, out );

  for ( i = 0; i < 14751; i++ )
    {
      if ( out->glyphs[i].gid != i )
	fprintf( stdout, "%d => %u\n", i, out->glyphs[i].gid );
    }
  FTL_Done_Glyphs_Array ( out );
  FTL_Done_Glyphs_Array ( in );
  FTL_Done_FeaturesRequest ( request );
}

/* END */
