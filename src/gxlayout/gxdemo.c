/***************************************************************************/
/*                                                                         */
/*  gxdemo.c                                                               */
/*                                                                         */
/*    Demo program for AAT/TrueTypeGX font driver implementation (body).   */
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

#define _XOPEN_SOURCE
#include <stdlib.h>

#include <ft2build.h>
#include FT_GXLAYOUT_H
#include FT_BBOX_H
#include FT_OUTLINE_H
#include FT_INTERNAL_DEBUG_H

#include <stdio.h>
#include <popt.h>
#include <glib.h>
#include <glib/gslist.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <errno.h>
#include <libgnomecanvas/libgnomecanvas.h>

#include "gxdump.h"
#include "gxfeatreg.h"
#include "gxtypes.h"
#include "gxload.h"
#include "gxaccess.h"

#define DEFAULT_UNIT 1024
#define BUFFER_LENGTH 1024

static char buffer[BUFFER_LENGTH];
static GHashTable * setting_buttons = NULL;
static gulong dump_flags 	   = GX_DUMP_mort|GX_DUMP_morx|GX_DUMP_feat|GX_DUMP_kern;
static gboolean dump_glyph_metrics = FALSE;
static GtkAdjustment * gid_spinner_adj;
static GtkWidget *glyph_canvas;

static GnomeCanvasItem *root_rect_item = NULL; 
static GnomeCanvasItem *bbox_item  = NULL; 
static GnomeCanvasItem *h_advance_item = NULL;
static GnomeCanvasItem *v_advance_item = NULL;
static GnomeCanvasItem *pixbuf_item    = NULL;
static GSList           *div_items = NULL;

static int default_gid = 19;

void create_window ( GX_Face face );
void destroy_window ( GtkObject * unused, GXL_FeaturesRequest request );

void radio_toggled( GtkToggleButton * toggle, gpointer setting );
void check_toggled( GtkToggleButton * toggle, gpointer setting);
void run_layout_engine ( GtkButton * button, gpointer request );
void reset_feature_request( GtkButton * button, gpointer request );
void check_table          ( GtkToggleButton * toggle_button, gpointer flag);
void dump_feature_request( GtkButton * button, gpointer request );
void dump_feature_registry( GtkButton * button, gpointer request );
void dump_language_id     ( GtkButton * button, gpointer face );

void horizontal_radio_toggled( GtkToggleButton * toggle, gpointer request );
void vertical_radio_toggled( GtkToggleButton * toggle, gpointer request );

void dump_file(FT_Library library, const char * file, gint verbose);
void dump_face(FT_Face face, const char* file, gint verbose);
void dump_glyph(FT_Face face, FT_UShort gid, FTL_Direction direction);

void activate_chain_trace( void );

void set_dump_glyph_metrics ( GtkWidget * check_button, gpointer data );
void render_glyph ( GtkWidget * button, gpointer request );

void set_trace_level( GtkAdjustment * adj, gpointer trace );

#define DUMP_DESC "Supported tables are mort,morx,feat,prop,trak,kern,just,lcar,opbd,bsln,fmtx,fdsc"
static const GDebugKey dump_keys[] = {
  {"mort", GX_DUMP_mort},
  {"morx", GX_DUMP_morx},
  {"feat", GX_DUMP_feat},
  {"prop", GX_DUMP_prop},
  {"trak", GX_DUMP_trak},
  {"kern", GX_DUMP_kern},
  {"just", GX_DUMP_just},
  {"lcar", GX_DUMP_lcar},
  {"opbd", GX_DUMP_opbd},
  {"bsln", GX_DUMP_bsln},
  {"fmtx", GX_DUMP_fmtx},
  {"fdsc", GX_DUMP_fdsc},
};

int
main(int argc, char ** argv)
{
  FT_Library library;
  FT_Face    face;
  poptContext optCon;
  int rc;
  const char * file;

  static char* arg_debug     = NULL;
  static int arg_batch 	     = 0;
  static int arg_memprof     = 0;
  static int arg_verbose     = 0;
  static int arg_trace_chain = 0;
  
  const int ndump_keys = sizeof(dump_keys)/sizeof(GDebugKey);
  struct poptOption optTable [] = {
    { "trace-chain",    '\0', POPT_ARG_NONE,   &arg_trace_chain, 0, "Dump chains selection", ""},
    { "default-gid",    '\0', POPT_ARG_INT,    &default_gid,     0, "Default GID", ""},
    { "batch",          'b',  POPT_ARG_NONE,   &arg_batch,       0, "batch mode",  ""},
    { "table",          't',  POPT_ARG_STRING, &arg_debug,       0, DUMP_DESC, "tableName"},
    { "memprof",        'm',  POPT_ARG_NONE,   &arg_memprof,     0, "Enter to infinite loop to run under memprof", ""},
    { "verbose",        'v',  POPT_ARG_NONE,   &arg_verbose,     0, "Print extra infomation to stdout", ""},
    POPT_AUTOHELP
    POPT_TABLEEND
  };
  FTL_EngineType engine_type;

  gtk_init(&argc, &argv);
  optCon = poptGetContext(argv[0], argc, (const char **)argv, optTable, 0);
  poptSetOtherOptionHelp(optCon, "[options] gxfont\n");      
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
	fprintf(stderr, "Bad argument %s: %s\n",
		poptBadOption(optCon, POPT_BADOPTION_NOALIAS),
		poptStrerror(rc));
	exit (1);
      }

  if (arg_trace_chain)
    activate_chain_trace();

  if (FT_Init_FreeType (&library))
    {
      fprintf(stderr, "Error in %s\n", "FT_Init_FreeType");
      exit (1);
    }

  if (arg_debug)
    dump_flags = g_parse_debug_string (arg_debug,
				       (GDebugKey *) dump_keys,
				       ndump_keys);

  file = poptGetArg(optCon);
  if (!file)
    {
      poptPrintHelp(optCon, stderr, 0);
      exit(1);
    }

  if ( arg_batch )
    {
      fprintf(stdout, "<meta>\n");
      do {
	dump_file( library, file, arg_verbose );
	file = poptGetArg(optCon);
      } while (file);
      fprintf(stdout, "</meta>\n");
      goto Exit;
    }
  
  if ( FT_New_Face (library, file, 0, &face) )
    {
      fprintf(stderr, "Error in %s: %s\n", "FT_New_Face", file);
      exit (1);
    }

#if 0
  if ( FT_HAS_VERTICAL(face) )
    fprintf(stdout, "Face has vertical infomation\n");
  else
    fprintf(stdout, "Face does not have vertical infomation\n");
#endif /* 0 */  

  if (( FTL_Query_EngineType( face, &engine_type ) )
      || ( engine_type != FTL_TRUETYPEGX_ENGINE ))
    {
      fprintf(stderr, "No GX table is existed: %s\n", file);
      exit ( 1 );
    }
  setting_buttons = g_hash_table_new(NULL, NULL);
  create_window( (GX_Face)face );
  
  if ( FT_Done_Face ( face ) )
    fprintf(stderr, "Error in %s: %s\n", "FT_Done_Face", file);
  
 Exit:
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

GtkWidget * create_gx_window( GX_Face face );
GtkWidget * create_feat_area( GXL_FeaturesRequest request );
GtkWidget * create_dump_area( GX_Face face, 
			      GXL_FeaturesRequest request );
GtkWidget * create_trace_area(void);

void
create_window (GX_Face face)
{
  GtkWidget * window;

  window = create_gx_window ( face );
  gtk_widget_show(window);
  gtk_window_resize( GTK_WINDOW(window), 1, 400);
  gtk_window_set_title (GTK_WINDOW( window ), ((FT_Face)face)->family_name);
  gtk_main();
}

GtkWidget *
create_gx_window( GX_Face face )
{

  GtkWidget * window;
  GtkWidget * feat;
  GtkWidget * dump;
  GtkWidget * trace;
  GtkWidget * vbox;
  GtkWidget * hbox;
  GtkWidget * button;
  GtkWidget * notebook;
  GtkWidget * label;
  GXL_FeaturesRequest request;
  GtkWidget *gid_spinner;
  
  FTL_New_FeaturesRequest ( (FT_Face)face, 
			    (FTL_FeaturesRequest*)&request );
  
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  g_signal_connect (G_OBJECT (window), "destroy",
                    G_CALLBACK ( destroy_window ), request);
  gtk_container_set_border_width (GTK_CONTAINER (window), 4);
  notebook = gtk_notebook_new ();
  gtk_widget_show( notebook );
  gtk_container_add ( GTK_CONTAINER( window ),
		      notebook );  

  /* Features */
  vbox 	= gtk_vbox_new ( FALSE, 8 );
  label = gtk_label_new("Features");
  gtk_notebook_append_page ( GTK_NOTEBOOK(notebook),
			     vbox,
			     label );

  gtk_widget_show( vbox );

  feat = create_feat_area ( request );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       feat,
		       TRUE,
		       TRUE,
		       4 );
  hbox = gtk_hbox_new ( TRUE, 4 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       TRUE,
		       4 );
  gtk_widget_show( hbox );
  
  button = gtk_button_new_with_label ("Reset");
  gtk_widget_show ( button );
  g_signal_connect ( G_OBJECT( button ), "clicked",
		     G_CALLBACK ( reset_feature_request ), request );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       button,
		       TRUE,
		       TRUE,
		       4 );

  button = gtk_button_new_with_label ("Run");
  gtk_widget_show ( button );
  g_signal_connect ( G_OBJECT( button ), "clicked",
		     G_CALLBACK ( run_layout_engine ), request );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       button,
		       TRUE,
		       TRUE,
		       4 );

  /* Glyph */
  vbox 	= gtk_vbox_new ( FALSE, 8 );
  label = gtk_label_new("Glyph");
  gtk_notebook_append_page ( GTK_NOTEBOOK(notebook),
			     vbox,
			     label );
  gtk_widget_show ( vbox );
  hbox  = gtk_hbox_new ( TRUE, 4 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( hbox );
  gid_spinner_adj = (GtkAdjustment *) gtk_adjustment_new ((gdouble)default_gid, 0.0, (gdouble)0xFFFF,
						      1.0, 5.0, 5.0);
  gid_spinner = gtk_spin_button_new (gid_spinner_adj, 1.0, 0);
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       gid_spinner,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show(gid_spinner);

  button = gtk_button_new_with_label("Render");
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       button,
		       FALSE,
		       TRUE,
		       0 );
  g_signal_connect( button,
		    "clicked",
		    G_CALLBACK( render_glyph ),
		    request );

  gtk_widget_push_visual (gdk_rgb_get_visual ());
  gtk_widget_push_colormap (gdk_rgb_get_cmap ());
  glyph_canvas = gnome_canvas_new_aa ();
  gtk_widget_pop_colormap ();
  gtk_widget_pop_visual ();
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       glyph_canvas,
		       TRUE,
		       TRUE,
		       4 );
  gnome_canvas_set_pixels_per_unit(GNOME_CANVAS(glyph_canvas),0.2);
  gnome_canvas_set_scroll_region(GNOME_CANVAS(glyph_canvas),0.0,0.0,
				 (double)DEFAULT_UNIT,
				 (double)-DEFAULT_UNIT );
  gtk_widget_show( glyph_canvas );

  gtk_widget_show ( button );

  /* Styles */
  vbox 	= gtk_vbox_new ( FALSE, 8 );
  label = gtk_label_new("Styles");
  gtk_notebook_append_page ( GTK_NOTEBOOK(notebook),
			     vbox,
			     label );
  
  gtk_widget_show ( vbox );

  /* Variations */
  vbox 	= gtk_vbox_new ( FALSE, 8 );
  label = gtk_label_new("Variations");
  gtk_notebook_append_page ( GTK_NOTEBOOK(notebook),
			     vbox,
			     label );
  /* gtk_widget_show ( vbox ); */

  /* Dump */
  dump = create_dump_area( face, request );
  label = gtk_label_new("Dump");
  gtk_notebook_append_page ( GTK_NOTEBOOK(notebook),
			     dump,
			     label );
  gtk_widget_show ( dump );

  /* Trace */
  trace = create_trace_area();
  label = gtk_label_new("Trace");
  gtk_notebook_append_page ( GTK_NOTEBOOK(notebook),
			     trace,
			     label );
  gtk_widget_show ( trace );

  return window;
}

GtkWidget *
create_feat_area( GXL_FeaturesRequest request )
{
  GtkWidget * features_vbox;
  GtkWidget * feature_frame;
  GtkWidget * settings_vbox;
  GtkWidget * setting_toggle;
  GtkWidget * setting_radio;
  GtkWidget * scrolled;
  GtkWidget * sep;

  gint i_feat, j_string, k_setting;
  gint nFeatures = GXL_FeaturesRequest_Get_Feature_Count ( request );
  GXL_Feature feature;
  FT_SfntName feature_name;

  gint nSettings;
  GXL_Setting setting;
  FT_SfntName setting_name;
  
  char * c_string;

  static GSList * group = NULL;

  scrolled = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (scrolled),
				 GTK_POLICY_NEVER,
				 GTK_POLICY_AUTOMATIC );
  features_vbox = gtk_vbox_new( FALSE, 4 );
  gtk_scrolled_window_add_with_viewport( GTK_SCROLLED_WINDOW( scrolled ),
					 features_vbox );

  feature_frame = gtk_frame_new ( "Direction" );
  gtk_box_pack_start( GTK_BOX( features_vbox ),
			  feature_frame,
			  FALSE,
			  FALSE,
			  2 );
  settings_vbox = gtk_vbox_new( FALSE, 4 );
  gtk_container_add( GTK_CONTAINER( feature_frame ),
		     settings_vbox );

  setting_radio = gtk_radio_button_new_with_label( group, "Horizontal" );  
  gtk_box_pack_start( GTK_BOX(settings_vbox),
		      setting_radio,
		      FALSE,
		      FALSE,
		      0 );
  g_signal_connect  ( setting_radio,
		      "toggled",
		      G_CALLBACK(horizontal_radio_toggled),
		      request );
  group = gtk_radio_button_get_group( GTK_RADIO_BUTTON (setting_radio) );
  
  setting_radio = gtk_radio_button_new_with_label( group, "Vertical" );  
  gtk_box_pack_start( GTK_BOX(settings_vbox),
		      setting_radio,
		      FALSE,
		      FALSE,
		      0 );
  g_signal_connect  ( setting_radio,
		      "toggled",
		      G_CALLBACK(vertical_radio_toggled),
		      request );

  sep = gtk_hseparator_new();
  gtk_box_pack_start( GTK_BOX(features_vbox),
		      sep,
		      FALSE,
		      FALSE,
		      0 );

  for ( i_feat = 0; i_feat < nFeatures; i_feat++ )
    {
      group = NULL;
      feature = GXL_FeaturesRequest_Get_Feature ( request, i_feat );
      if ( GXL_Feature_Get_Name ( feature, 0, 0, 0, &feature_name ) )
	{
	  fprintf(stderr, "Cannot find name\n");
	  exit (1);
	}
      
      c_string = g_new(char, feature_name.string_len + 1 );
      c_string[feature_name.string_len] = '\0';
      for (j_string = 0; j_string < feature_name.string_len; j_string++)
	c_string[j_string] = feature_name.string[j_string];
      feature_frame = gtk_frame_new ( c_string );
      g_free(c_string);
      gtk_box_pack_start( GTK_BOX( features_vbox ),
			  feature_frame,
			  FALSE,
			  FALSE,
			  2 );
      settings_vbox = gtk_vbox_new( FALSE, 4 );
      gtk_container_add( GTK_CONTAINER( feature_frame ),
			 settings_vbox );
      nSettings = GXL_Feature_Get_Setting_Count( feature );
      for ( k_setting = 0; k_setting < nSettings; k_setting++ )
	{
	  setting = GXL_Feature_Get_Setting( feature, k_setting );
	  if ( GXL_Setting_Get_Name ( setting, 0, 0, 0, &setting_name ) )
	    {
	      fprintf (stderr, "Cannot find setting name\n");
	      exit (1);
	    }
	  c_string = g_new(char, setting_name.string_len + 1 );
	  c_string[setting_name.string_len] = '\0';
	  for (j_string = 0; j_string < setting_name.string_len; j_string++)
	    c_string[j_string] = setting_name.string[j_string];
	  if ( GXL_Feature_Is_Setting_Exclusive (feature) )
	    {
	      setting_radio = gtk_radio_button_new_with_label(group, c_string);
	      group 	    = gtk_radio_button_get_group( GTK_RADIO_BUTTON (setting_radio) );
	      g_free(c_string);
	      gtk_toggle_button_set_active(  GTK_TOGGLE_BUTTON(setting_radio),
					     GXL_Setting_Get_State (setting) );
	      g_signal_connect  ( setting_radio,
				  "toggled",
				  G_CALLBACK(radio_toggled),
				  setting );
	      gtk_box_pack_start( GTK_BOX(settings_vbox),
				  setting_radio,
				  FALSE,
				  FALSE,
				  0 );
	      gtk_container_set_border_width (GTK_CONTAINER (setting_radio), 2);
	      g_hash_table_insert ( setting_buttons, setting_radio, setting );
	    }
	  else
	    {
	      setting_toggle       = gtk_check_button_new_with_label(c_string);
	      g_free(c_string);
	      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(setting_toggle),
					     GXL_Setting_Get_State (setting) );
	      g_signal_connect  ( setting_toggle,
				  "toggled",
				  G_CALLBACK(check_toggled),
				  setting );
	      gtk_box_pack_start( GTK_BOX(settings_vbox),
				  setting_toggle,
				  FALSE,
				  FALSE,
				  0 );
	      gtk_container_set_border_width (GTK_CONTAINER (setting_toggle), 2);
	      g_hash_table_insert ( setting_buttons, setting_toggle, setting );
	    }
	}
      group = NULL;
    }
  gtk_widget_show_all(scrolled);
  return scrolled;
}

void dump_face_cb( GtkButton * button, gpointer face );

GtkWidget * 
create_dump_area( GX_Face face, 
		   GXL_FeaturesRequest request )

{
  GtkWidget * vbox = gtk_vbox_new ( FALSE, 0 );
  GtkWidget * button;
  GtkWidget * frame;
  GtkWidget * dump_face_vbox;
  GtkWidget * dump_tables_vbox;
  GtkWidget * check_button;

  /* Language ID */
  button = gtk_button_new_with_label("Dump Language ID");
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       button,
		       FALSE,
		       TRUE,
		       0 );
  g_signal_connect ( G_OBJECT( button ), "clicked",
		     G_CALLBACK ( dump_language_id ), face );
  gtk_widget_show ( button );
    
  /* Feature Request */
  button = gtk_button_new_with_label("Dump Feature Request");
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       button,
		       FALSE,
		       TRUE,
		       0 );
  g_signal_connect ( G_OBJECT( button ), "clicked",
		     G_CALLBACK ( dump_feature_request ), request );
  gtk_widget_show ( button );

  
  /* Feature Registry */
  button = gtk_button_new_with_label("Dump Feature Registry");
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       button,
		       FALSE,
		       TRUE,
		       0 );
  g_signal_connect ( G_OBJECT( button ), "clicked",
		     G_CALLBACK ( dump_feature_registry ), NULL );
  gtk_widget_show ( button );

  /* Dump Glyph Metrics */
  button = gtk_check_button_new_with_label ("Enable to Dump Glyph Metrics");
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       button,
		       FALSE,
		       TRUE,
		       0 );
  g_signal_connect ( G_OBJECT( button ), "toggled",
		     G_CALLBACK ( set_dump_glyph_metrics ), &dump_glyph_metrics );
  gtk_widget_show ( button );
  
  /* Dump tables */
  frame = gtk_frame_new ( "Dump Tables" );
  gtk_box_pack_start( GTK_BOX( vbox ), frame, FALSE, FALSE, 2 );
  dump_face_vbox = gtk_vbox_new ( FALSE, 0 );
  gtk_container_add( GTK_CONTAINER( frame ), dump_face_vbox );
  dump_tables_vbox = gtk_vbox_new ( FALSE, 0 );
  gtk_container_add( GTK_CONTAINER( dump_face_vbox ), dump_tables_vbox );

#define MAKE_CHECK_BUTTON(tag)						\
  check_button = gtk_check_button_new_with_label(#tag);			\
  gtk_container_add( GTK_CONTAINER( dump_tables_vbox ), check_button );	\
  gtk_widget_show(check_button);					\
  gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(check_button),	\
				( dump_flags & GX_DUMP_##tag )? TRUE: FALSE ); \
  g_signal_connect ( G_OBJECT( check_button ),				\
		     "toggled",						\
		     G_CALLBACK(check_table),				\
		     GINT_TO_POINTER(GX_DUMP_##tag) )

  MAKE_CHECK_BUTTON(mort);
  MAKE_CHECK_BUTTON(morx);
  MAKE_CHECK_BUTTON(feat);
  MAKE_CHECK_BUTTON(prop);
  MAKE_CHECK_BUTTON(trak);
  MAKE_CHECK_BUTTON(kern);
  MAKE_CHECK_BUTTON(just);
  MAKE_CHECK_BUTTON(lcar);
  MAKE_CHECK_BUTTON(opbd);
  MAKE_CHECK_BUTTON(bsln);
  MAKE_CHECK_BUTTON(fmtx);
  MAKE_CHECK_BUTTON(fdsc);

  button = gtk_button_new_with_label("Dump Font Tables");
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK ( dump_face_cb ), face);
  gtk_container_add( GTK_CONTAINER( dump_face_vbox ), button );
  gtk_widget_show ( button );
  
  gtk_widget_show ( dump_tables_vbox );
  gtk_widget_show ( dump_face_vbox );
  gtk_widget_show ( frame );

  return vbox;
}

GtkWidget*
create_trace_area(void)
{
  int count = FT_Trace_Get_Count();
  int i;
  GtkWidget * vbox, *hbox;
  GtkWidget * label;
  GtkWidget * scrolled;
  const char * label_string;
  GtkObject * adj;
  GtkWidget * spinner;
  int level;
  
  scrolled = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (scrolled),
				 GTK_POLICY_NEVER,
				 GTK_POLICY_AUTOMATIC );
  
  vbox = gtk_vbox_new ( TRUE, 1 );
  for ( i = 0; i < count; i++ )
    {
      hbox = gtk_hbox_new( TRUE, 2 );
      label_string = FT_Trace_Get_Name( i );
      label 	   = gtk_label_new( label_string );
      gtk_container_add(GTK_CONTAINER(hbox), label);
      gtk_widget_show(label);

#ifdef FT_DEBUG_LEVEL_TRACE  
      level = (gdouble)ft_trace_levels[i];
#else 
      level = 0;
#endif  /* FT_DEBUG_LEVEL_TRACE */

      adj = gtk_adjustment_new(level,
			       0.0, (gdouble)7, 1.0, 1.0, 1.0);
      spinner = gtk_spin_button_new( GTK_ADJUSTMENT(adj), 1.0, 0 );
      gtk_spin_button_set_range(GTK_SPIN_BUTTON(spinner), 0.0, 7.0);
      gtk_box_pack_end( GTK_BOX(hbox), spinner, FALSE, TRUE, 0 );
      gtk_widget_show(spinner);
      g_signal_connect( G_OBJECT(adj),
			"value_changed",
			G_CALLBACK(set_trace_level),
			GINT_TO_POINTER(i));
      gtk_container_add( GTK_CONTAINER(vbox), hbox);
      gtk_widget_show(hbox);
    }
  gtk_widget_show(vbox);
  gtk_scrolled_window_add_with_viewport( GTK_SCROLLED_WINDOW( scrolled ),
					 vbox );
  return scrolled;
}

void
dump_face_cb( GtkButton * button, gpointer face )
{
  dump_face( face, ((FT_Face)face)->family_name, 1);
}

void
check_table          ( GtkToggleButton * toggle_button, gpointer flag)
{
  if ( gtk_toggle_button_get_active( toggle_button ) )
    dump_flags |= GPOINTER_TO_INT(flag);
  else
    dump_flags &= (~(GPOINTER_TO_INT(flag)));
}

void
destroy_window ( GtkObject * unused, GXL_FeaturesRequest request )
{
  FTL_Done_FeaturesRequest ( (FTL_FeaturesRequest)request );
  gtk_main_quit();
}

void
radio_toggled( GtkToggleButton * toggle, gpointer setting )
{
  gboolean state = gtk_toggle_button_get_active(toggle);
  GXL_Setting_Set_State( setting, state );
}

void
check_toggled( GtkToggleButton * toggle, gpointer setting )
{
  gboolean state = gtk_toggle_button_get_active(toggle);
  GXL_Setting_Set_State( setting, state );
}

void
run_layout_engine ( GtkButton * button, gpointer request )
{
  FTL_Glyphs_Array in, out;
  FT_Face face 	   = ((FTL_FeaturesRequest)request)->font->face;
  FT_Memory memory = face->driver->root.memory;

  char * tmp, *next;
  long value, length = 0, i;
  
  printf( "Input:       ");
  tmp = fgets(buffer, BUFFER_LENGTH, stdin);
  if ( !tmp )
    {
      fprintf(stderr, "Fail to read string\n");
      return;
    }
  
  FTL_New_Glyphs_Array ( memory, &in );
  FTL_New_Glyphs_Array ( memory, &out );

  tmp = buffer;
  while ( 1 )
    {
      value = strtol(tmp, &next, 10);
      if (( value == 0 ) && ( tmp == next ))
	break;
      else
	{
	  length++;
	  tmp = next;
	}
    }
  FTL_Set_Glyphs_Array_Length ( in, length );

  tmp = buffer;
  for ( i = 0; i < length; i++ )
    in->glyphs[i].gid = (FT_UShort)strtol(tmp, &tmp, 10);
  
  FTL_Activate_FeaturesRequest( request );
  FTL_Substitute_Glyphs ( face, in, out );
  
  fprintf(stdout, "Substituted: ");
  for ( i = 0; i < length; i++ )
    fprintf(stdout, "%u%s ", out->glyphs[i].gid, 
	    (out->glyphs[i].gid == 0xFFFF)? "<empty>": "");
  fprintf(stdout, "\n");
  
  if ( dump_glyph_metrics )
    {
      fprintf(stdout, "\nGlyph Metrics\n");
      fprintf(stdout, "---------------\n");
      for ( i = 0; i < length; i++ )
	dump_glyph(face, 
		   out->glyphs[i].gid, 
		   FTL_Get_FeaturesRequest_Direction((FTL_FeaturesRequest)request));
    }

  FTL_Done_Glyphs_Array ( in );
  FTL_Done_Glyphs_Array ( out );
}

void
reflect_request ( gpointer key, gpointer value, gpointer user_data );   
void
reset_feature_request( GtkButton * button, gpointer request )
{
  FTL_Reset_FeaturesRequest( request );
  g_hash_table_foreach(setting_buttons, reflect_request, NULL);
}
void
reflect_request ( gpointer key, gpointer value, gpointer user_data )
{
  GtkWidget * button  = GTK_WIDGET(key);
  GXL_Setting setting = value;
  gtk_toggle_button_set_active(  GTK_TOGGLE_BUTTON(button),
				 GXL_Setting_Get_State (setting) );
}

void
dump_feature_request( GtkButton * button, gpointer request )
{
  gxl_features_request_dump(request, stdout);
}

void
dump_feature_registry( GtkButton * button, gpointer request )
{
  fprintf(stdout, "Contents of registry data base\n");
  fprintf(stdout, "------------------------------\n");
  gx_feature_registory_dump(stdout);
}

void
horizontal_radio_toggled( GtkToggleButton * toggle,
			  gpointer request )
{
  if ( gtk_toggle_button_get_active( toggle ) )
    FTL_Set_FeaturesRequest_Direction ( request,
					FTL_HORIZONTAL );
					
}

void
vertical_radio_toggled( GtkToggleButton * toggle,
			gpointer request )
{
  if ( gtk_toggle_button_get_active( toggle ) )
    FTL_Set_FeaturesRequest_Direction ( request,
					FTL_VERTICAL );
  
}

void
dump_face( FT_Face face, const char * file, gint verbose)
{
  FT_Bool gx_p;
  FTL_EngineType engine_type;

  gx_p = !((FTL_Query_EngineType (face, &engine_type))
	   || (engine_type != FTL_TRUETYPEGX_ENGINE));
  if ( verbose )
    {
      if ( gx_p )
	fprintf(stderr, "ok\n");
      else if (((TT_Face)face)->extra.data)
	fprintf(stderr, "failed(no gx, cff)\n");
      else
	fprintf(stderr, "failed(no gx)\n");
    }

  if ( gx_p )
    gx_face_dump(face, dump_flags, file);
    
  fflush ( stdout );
}

void
dump_file( FT_Library library, const char * file, gint verbose)
{
  FT_Face face;

  if ( FT_New_Face (library, file, 0, &face) )
    {
      fprintf(stderr, "Error in %s: %s\n", "FT_New_Face", file);
      return;
    }

  if ( verbose )
    fprintf(stderr, "loading %s...", file);
  /* dump_language_id( NULL, face ); */
  dump_face( face, file, verbose );
  if ( FT_Done_Face ( face ) )
    fprintf(stderr, "Error in %s: %s\n", "FT_Done_Face", file);
}

void
dump_language_id     ( GtkButton * unused, gpointer face )
{
  /* See ttnameid.h */
  FT_CharMap * charmaps 	 = ((FT_Face)face)->charmaps;
  FT_Int i, num_charmaps = ((FT_Face)face)->num_charmaps;
  FT_ULong langid;

  fprintf(stdout, "Laguage ID for the charmaps in the face(see ttnameid.h)\n");
  fprintf(stdout, "---------------------------------------\n");
  
  for ( i = 0; i < num_charmaps; i++ )
    {
      langid = FT_Get_CMap_Language_ID ( charmaps[i] );
      switch ( i )
	{
	case 0:
	  fprintf(stdout, "Laguage ID for the 1st charmap: %lu\n", langid );
	  break;
	case 1:
	  fprintf(stdout, "Laguage ID for the 2nd charmap: %lu\n", langid );
	  break;
	case 2:
	  fprintf(stdout, "Laguage ID for the 3rd charmap: %lu\n", langid );
	  break;
	default:
	  fprintf(stdout, "Laguage ID for the %dth charmap: %lu\n", i, langid );
	}
    }
}

void
activate_chain_trace( void )
{
  char * tmp = getenv("FT2_DEBUG");
  if ( !tmp )
    tmp = g_strdup("FT2_DEBUG=gxchain:6");
  else
    tmp = g_strconcat (tmp, " gxchain:6", NULL);
  putenv(tmp);
}

void
dump_glyph(FT_Face face, FT_UShort gid, FTL_Direction dir)
{
  FT_Int32 vmask;
  FT_BBox bbox;
  FT_UShort i, ligcount, div, new_div;
  
  fprintf(stdout, "gid: %u\n", gid);
  
  if ( gid == 0xFFFF )
    {
      fprintf(stdout, "\t<null glyph>\n");
      return ;
    }

  ligcount = FTL_Get_LigatureCaret_Count ( face, gid );	
  vmask = (dir == FTL_VERTICAL)? FT_LOAD_VERTICAL_LAYOUT: 0;
  FT_Load_Glyph (face, gid, 
		 vmask | FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP);
  
  FT_Outline_Get_CBox (&face->glyph->outline, &bbox);

  if ( ligcount == 0 )
    {
      fprintf(stdout, "\twidth: %g\n\theight: %g\n", 
	      (double)bbox.xMax - (double)bbox.xMin,
	      (double)bbox.yMax - (double)bbox.yMin );
      fprintf(stdout, "\tbbox: [xmin: %g ymin: %g xmax: %g ymax: %g]\n",
	      (double)bbox.xMin, (double)bbox.yMin, (double)bbox.xMax, (double)bbox.yMax);
    }
  else
    {
      div = bbox.xMin;
      for ( i = 0; i < ligcount; i++ )
	{
	  new_div = FTL_Get_LigatureCaret_Division( face, gid, i );
	  fprintf(stdout, "\twidth[%d]: %u\n\theight[%d]: %g\n", 
		  i, new_div - div,
		  i, (double)bbox.yMax - (double)bbox.yMin );
	  fprintf(stdout, "\tbbox[%d]: [xmin: %g ymin: %g xmax: %g ymax: %g]\n",
		  i, (double)div, (double)bbox.yMin, (double)new_div, (double)bbox.yMax);
	  div = new_div;
	  fprintf(stdout, "\t----------------\n");
	}
      fprintf(stdout, "\twidth[%d]: %g\n\theight[%d]: %g\n", 
	      i, (double)bbox.xMax - (double)div,
	      i, (double)bbox.yMax - (double)bbox.yMin );
      fprintf(stdout, "\tbbox[%d]: [xmin: %g ymin: %g xmax: %g ymax: %g]\n",
	      i, (double)div, (double)bbox.yMin, (double)bbox.xMax, (double)bbox.yMax);
    }
}

void
set_dump_glyph_metrics ( GtkWidget * check_button, gpointer data )
{
  *(gboolean*)data = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(check_button) );
}

void
render_glyph ( GtkWidget * button, gpointer request )
{
  FT_Error error;
  GnomeCanvasGroup *root = gnome_canvas_root(GNOME_CANVAS(glyph_canvas));
  GnomeCanvasItem *div_item;
  GdkPixbuf *pixbuf;
  
  FT_Face face 	    = ((FTL_FeaturesRequest)request)->font->face;
  FTL_Direction dir = FTL_Get_FeaturesRequest_Direction((FTL_FeaturesRequest)request);
  FT_UShort gid     = (FT_UShort)gtk_adjustment_get_value(gid_spinner_adj);

  FT_Int32 vmask;
  FT_BBox bbox;
  FT_UShort i, ligcount, div;
  double affine[6] = {1.0, 0.0, 0.0, -1.0, 0.0, 0.0};
  double affine_with_bearing[6] = {1.0, 0.0, 0.0, 1.0, 0.0, 0.0};
  double factor 	 = (double)DEFAULT_UNIT/(double)face->units_per_EM;

  if ( dump_glyph_metrics )
    dump_glyph ( face, gid, dir );

  if ( pixbuf_item )
    {
      gtk_object_destroy( GTK_OBJECT(pixbuf_item) );
      pixbuf_item = NULL;
    }
  if ( root_rect_item )
    gtk_object_destroy( GTK_OBJECT(root_rect_item) );
  if ( bbox_item )
    gtk_object_destroy( GTK_OBJECT(bbox_item) );
  if ( h_advance_item )
    gtk_object_destroy( GTK_OBJECT(h_advance_item) );
  if ( v_advance_item )
    gtk_object_destroy( GTK_OBJECT(v_advance_item) );
  
  if ( div_items )
    {
      GSList * tmp;
      for ( tmp = div_items; tmp; tmp = tmp->next )
	gtk_object_destroy( tmp->data );
      g_slist_free ( div_items );
      div_items = NULL;
    }

  error = FT_Set_Pixel_Sizes(face, 0, DEFAULT_UNIT );
  if ( error )
    {
      fprintf(stderr, "Fail in FT_Set_Pixel_Sizes\n");
      return ;
    }
  vmask = (dir == FTL_VERTICAL)? FT_LOAD_VERTICAL_LAYOUT: 0;
  error = FT_Load_Glyph (face, gid, 
		 vmask | FT_LOAD_NO_BITMAP);
  if ( error )
    {
      fprintf(stderr, "Fail in FT_Load_Glyph[0]\n");
      return ;
    }
  error = FT_Render_Glyph( face->glyph, FT_RENDER_MODE_NORMAL );
  if ( error )
    {
      fprintf(stderr, "Fail in FT_Render_Glyph\n");
      return ;
    }
#if 0
  fprintf(stdout, "gid: %d row: %d, width: %d, pitch: %d\n",
	  gid,
	  face->glyph->bitmap.rows,   face->glyph->bitmap.width,
	  face->glyph->bitmap.pitch);
#endif /* 0 */  

  if ( face->glyph->bitmap.width == 0 )
    goto IGNORE_PIXBUF;

  pixbuf = gdk_pixbuf_new( GDK_COLORSPACE_RGB,
			   TRUE,
			   8,
			   face->glyph->bitmap.width,
			   face->glyph->bitmap.rows );

  {
    int x, y;
    unsigned char src;
    unsigned char*  src_buffer;
    guchar *dist_buffer = gdk_pixbuf_get_pixels (pixbuf);
    guchar * dist;
    int rowstride = gdk_pixbuf_get_rowstride (pixbuf);

    for ( y = 0; y < face->glyph->bitmap.rows; y++ )
      {
	src_buffer  = &(face->glyph->bitmap.buffer[y*face->glyph->bitmap.pitch]);
	for (x = 0; x < face->glyph->bitmap.width; x++ )
	{
	  src = src_buffer[x];
	  dist = dist_buffer + y * rowstride + x * 4;
	  dist[0]   = dist[1] = dist[2] = 255 - src;

	  if (255 - src)
	    dist[3] = 0;
	  else
	  dist[3] = 255;
	}
      }
  }
  pixbuf_item = gnome_canvas_item_new(root,
				      GNOME_TYPE_CANVAS_PIXBUF,
				      "pixbuf", pixbuf,
				      NULL);
  g_object_unref(pixbuf);

 IGNORE_PIXBUF:
  error = FT_Load_Glyph (face, gid, 
			 vmask | FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP);
  if ( error )
    {
      fprintf(stderr, "Fail in FT_Load_Glyph[1]\n");
      return ;
    }
  
  FT_Outline_Get_CBox (&face->glyph->outline, &bbox);
  if ( error )
    {
      fprintf(stderr, "Fail in FT_Outline_Get_CBox\n");
      return ;
    }

  root_rect_item = gnome_canvas_item_new(root,
					 GNOME_TYPE_CANVAS_RECT,
					 "x1", (double)0,
					 "y1", (double)0,
					 "x2", (double)DEFAULT_UNIT,
					 "y2", (double)DEFAULT_UNIT,
					 "outline_color", "black",
					 NULL);
  gnome_canvas_item_affine_relative( root_rect_item, affine );

  affine[0] *= factor;
  affine[3] *= factor;

  h_advance_item = gnome_canvas_item_new( root,
					  GNOME_TYPE_CANVAS_RECT,
					  "x1", (double)0,
					  "y1", (double)0,
					  "x2", (double)face->glyph->metrics.horiAdvance,
					  "y2", (double)0,
					  "outline_color", "red",
					  NULL);
  gnome_canvas_item_affine_relative( h_advance_item, affine );

  {
    /* Next line is workaround against libart(?)'s bug */
    double vertAdvance = ((face->glyph->metrics.vertAdvance)/2)*2.0;
    v_advance_item = gnome_canvas_item_new( root,
					    GNOME_TYPE_CANVAS_RECT,
					    "x1", (double)0,
					    "y1", (double)0,
					    "x2", (double)0,
					    "y2", (double)vertAdvance,
					    "outline_color", "red",
					    NULL);
  }
  gnome_canvas_item_affine_relative( v_advance_item, affine );

  bbox_item = gnome_canvas_item_new(root,
				    GNOME_TYPE_CANVAS_RECT,
				    "x1", (double)bbox.xMin,
				    "y1", (double)bbox.yMin,
				    "x2", (double)bbox.xMax,
				    "y2", (double)bbox.yMax,
				    "outline_color", "blue",
				    NULL);
  gnome_canvas_item_affine_relative( bbox_item, affine );

  ligcount = FTL_Get_LigatureCaret_Count ( face, gid );
  for ( i = 0; i < ligcount; i++ )
    {
      /* Here, we will ignore the direction. */
      div = FTL_Get_LigatureCaret_Division( face, gid, i );
      div_item = gnome_canvas_item_new(gnome_canvas_root(GNOME_CANVAS(glyph_canvas)),
				       GNOME_TYPE_CANVAS_RECT,
				       "x1", (double)div,
				       "y1", (double)0,
				       "x2", (double)div,
				       "y2", (double)DEFAULT_UNIT,
				       "outline_color", "red",
				       NULL);
      gnome_canvas_item_affine_relative( div_item, affine );
      div_items = g_slist_append ( div_items, div_item );
    }

  if ( pixbuf_item )
    {
      affine_with_bearing[4] = (double)bbox.xMin * factor;
      affine_with_bearing[5] = -(double)bbox.yMax * factor;
      gnome_canvas_item_affine_relative( pixbuf_item, affine_with_bearing );
    }
}

void
set_trace_level( GtkAdjustment * adj, gpointer trace )
{
#ifdef FT_DEBUG_LEVEL_TRACE  
  gint index 	       	 = GPOINTER_TO_INT(trace);
  gint level 		 = (gint)gtk_adjustment_get_value(adj);
  ft_trace_levels[index] = level;
#endif  /* FT_DEBUG_LEVEL_TRACE */
}


/* END */
