/****************************************************************************/
/*                                                                          */
/*  The FreeType project -- a free and portable quality TrueType renderer.  */
/*                                                                          */
/*  Copyright (C) 2002-2023 by                                              */
/*  D. Turner, R.Wilhelm, and W. Lemberg                                    */
/*                                                                          */
/*  ftbench: bench some common FreeType call paths                          */
/*                                                                          */
/****************************************************************************/


#ifndef  _GNU_SOURCE
#define  _GNU_SOURCE /* we want to use extensions to `time.h' if available */
#endif

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <time.h>

#include <ft2build.h>
#include <freetype/freetype.h>

#include <freetype/ftadvanc.h>
#include <freetype/ftbbox.h>
#include <freetype/ftcache.h>
#include <freetype/ftdriver.h>
#include <freetype/ftglyph.h>
#include <freetype/ftlcdfil.h>
#include <freetype/ftmm.h>
#include <freetype/ftmodapi.h>
#include <freetype/ftoutln.h>
#include <freetype/ftstroke.h>
#include <freetype/ftsynth.h>

#define MAX_MM_AXES 16

#ifdef UNIX
#include <unistd.h>
#else
#include "mlgetopt.h"
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* Specify the timer: QPC for accurate wall time, GPT for user-mode time. */
/* Otherwise, QPCT cycles are measured accurately but with huge overhead. */
#define QPC

#ifdef QPC
  double  interval;
#endif

#endif


  typedef struct  btimer_t_ {
    double  t0;
    double  total;

  } btimer_t;


  typedef int
  (*bcall_t)( btimer_t*  timer,
              FT_Face    face,
              void*      user_data );


  typedef struct  btest_t_ {
    const char*  title;
    bcall_t      bench;
    int          cache_first;
    void*        user_data;

  } btest_t;


  typedef struct  bcharset_t_
  {
    FT_Int     size;
    FT_ULong*  code;

  } bcharset_t;


  static FT_Error
  get_face( FT_Face*  face );


  /*
   * Globals
   */

#define CACHE_SIZE  1024
#define BENCH_TIME  2.0
#define FACE_SIZE   10


  static FT_Library        lib;
  static FTC_Manager       cache_man;
  static FTC_CMapCache     cmap_cache;
  static FTC_ImageCache    image_cache;
  static FTC_SBitCache     sbit_cache;
  static FTC_ImageTypeRec  font_type;

  static FT_MM_Var*    multimaster   = NULL;
  static FT_Fixed      design_pos   [MAX_MM_AXES];
  static FT_Fixed      requested_pos[MAX_MM_AXES];
  static unsigned int  requested_cnt = 0;
  static unsigned int  used_num_axes = 0;

  enum {
    FT_BENCH_LOAD_GLYPH,
    FT_BENCH_LOAD_ADVANCES,
    FT_BENCH_RENDER,
    FT_BENCH_GET_GLYPH,
    FT_BENCH_CMAP,
    FT_BENCH_CMAP_ITER,
    FT_BENCH_NEW_FACE,
    FT_BENCH_EMBOLDEN,
    FT_BENCH_STROKE,
    FT_BENCH_GET_BBOX,
    FT_BENCH_GET_CBOX,
    FT_BENCH_NEW_FACE_AND_LOAD_GLYPH,
    N_FT_BENCH
  };


  static const char*  bench_desc[] =
  {
    "load a glyph        (FT_Load_Glyph)",
    "load advance widths (FT_Get_Advances)",
    "render a glyph      (FT_Render_Glyph)",
    "load a glyph        (FT_Get_Glyph)",
    "get glyph indices   (FT_Get_Char_Index)",
    "iterate CMap        (FT_Get_{First,Next}_Char)",
    "open a new face     (FT_New_Face)",
    "embolden            (FT_GlyphSlot_Embolden)",
    "stroke              (FT_Glyph_Stroke)",
    "get glyph bbox      (FT_Outline_Get_BBox)",
    "get glyph cbox      (FT_Glyph_Get_CBox)",

    "open face and load glyphs",
    NULL
  };


  static int    preload;
  static char*  filename;

  static int  first_index = 0;
  static int  last_index  = INT_MAX;
  static int  incr_index  = 1;

  static int  cmap_index  = -1;

#define FOREACH( i )  for ( i = first_index ;                          \
                            ( first_index <= i && i <= last_index ) || \
                            ( first_index >= i && i >= last_index ) ;  \
                            i += incr_index )

  static FT_Render_Mode  render_mode = FT_RENDER_MODE_NORMAL;
  static FT_Int32        load_flags  = FT_LOAD_DEFAULT;

  static unsigned int  tt_interpreter_versions[2];
  static int           num_tt_interpreter_versions;
  static unsigned int  dflt_tt_interpreter_version;

  static unsigned int  ps_hinting_engines[2];
  static int           num_ps_hinting_engines;
  static unsigned int  dflt_ps_hinting_engine;

  static char  ps_hinting_engine_names[2][10] = { "freetype",
                                                  "adobe" };


  /*
   * Face requester for cache testing
   */

  static FT_Error
  face_requester( FTC_FaceID  face_id,
                  FT_Library  library,
                  FT_Pointer  request_data,
                  FT_Face*    aface )
  {
    FT_UNUSED( face_id );
    FT_UNUSED( library );
    FT_UNUSED( request_data );

    return get_face( aface );
  }


  /*
   * timer in milliseconds
   */

  static double
  get_time( void )
  {
    /* NOTE: When building with the Mingw64 toolchain, `_POSIX_TIMERS` is
     * defined, but function `clock_gettime` is not.  Ensure that the
     * `_WIN32` specific timer code appears first here.
     */
#if defined  _WIN32

#ifdef QPC
    LARGE_INTEGER  ticks;


    QueryPerformanceCounter( &ticks );

    return  interval * ticks.QuadPart;

#elif defined GPT
    FILETIME  start, end, kern, user;


    GetProcessTimes( GetCurrentProcess(), &start, &end, &kern, &user );

    return  0.1 * user.dwLowDateTime + 429496729.6 * user.dwHighDateTime;

#else
    ULONG64  cycles;


    QueryProcessCycleTime( GetCurrentProcess(), &cycles );

    return  1e-3 * cycles; /* at 1GHz */

#endif

#elif defined _POSIX_TIMERS && _POSIX_TIMERS > 0
    struct timespec  tv;


#ifdef _POSIX_CPUTIME
    clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &tv );
#else
    clock_gettime( CLOCK_REALTIME, &tv );
#endif /* _POSIX_CPUTIME */

    return 1E6 * (double)tv.tv_sec + 1E-3 * (double)tv.tv_nsec;

#else
    /* clock() accuracy has improved since glibc 2.18 */
    return 1E6 * (double)clock() / (double)CLOCKS_PER_SEC;
#endif /* _POSIX_TIMERS */
  }

#define TIMER_START( timer )  ( timer )->t0 = get_time()
#define TIMER_STOP( timer )   ( timer )->total += get_time() - ( timer )->t0
#define TIMER_GET( timer )    ( timer )->total
#define TIMER_RESET( timer )  ( timer )->total = 0


  /*
   * Bench code
   */

  static void
  benchmark( FT_Face   face,
             btest_t*  test,
             int       max_iter,
             double    max_time )
  {
    int       n, done;
    btimer_t  timer, elapsed;


    if ( test->cache_first )
    {
      TIMER_RESET( &timer );
      test->bench( &timer, face, test->user_data );
    }

    printf( "  %-25s ", test->title );
    fflush( stdout );

    TIMER_RESET( &timer );
    TIMER_RESET( &elapsed );

    for ( n = 0, done = 0; !max_iter || n < max_iter; n++ )
    {
      TIMER_START( &elapsed );

      done += test->bench( &timer, face, test->user_data );

      TIMER_STOP( &elapsed );

      if ( TIMER_GET( &elapsed ) > 1E6 * max_time )
        break;
    }

    if ( done )
      printf( "%10.3f us/op %10d done\n",
              TIMER_GET( &timer ) / (double)done, done );
    else
      printf( "no error-free calls\n" );
  }


  /*
   * Various tests
   */

  static int
  test_load( btimer_t*  timer,
             FT_Face    face,
             void*      user_data )
  {
    int  i, done = 0;

    FT_UNUSED( user_data );


    TIMER_START( timer );

    FOREACH( i )
    {
      if ( !FT_Load_Glyph( face, (FT_UInt)i, load_flags ) )
        done++;
    }

    TIMER_STOP( timer );

    return done;
  }


  static int
  test_load_advances( btimer_t*  timer,
                      FT_Face    face,
                      void*      user_data )
  {
    int        done = 0;
    FT_Fixed*  advances;
    FT_ULong   flags = *((FT_ULong*)user_data);
    FT_Int     start, count;


    if ( incr_index > 0 )
    {
      start = first_index;
      count = last_index - first_index + 1;
    }
    else
    {
      start = last_index;
      count = first_index - last_index + 1;
    }

    advances = (FT_Fixed *)calloc( sizeof ( FT_Fixed ), (size_t)count );

    TIMER_START( timer );

    FT_Get_Advances( face,
                     (FT_UInt)start, (FT_UInt)count,
                     (FT_Int32)flags, advances );
    done += (int)count;

    TIMER_STOP( timer );

    free( advances );

    return done;
  }


  static int
  test_render( btimer_t*  timer,
               FT_Face    face,
               void*      user_data )
  {
    int  i, done = 0;

    FT_UNUSED( user_data );


    FOREACH( i )
    {
      if ( FT_Load_Glyph( face, (FT_UInt)i, load_flags ) )
        continue;

      TIMER_START( timer );
      if ( !FT_Render_Glyph( face->glyph, render_mode ) )
        done++;
      TIMER_STOP( timer );
    }

    return done;
  }


  static int
  test_embolden( btimer_t*  timer,
                 FT_Face    face,
                 void*      user_data )
  {
    int  i, done = 0;

    FT_UNUSED( user_data );


    FOREACH( i )
    {
      if ( FT_Load_Glyph( face, (FT_UInt)i, load_flags ) )
        continue;

      TIMER_START( timer );
      FT_GlyphSlot_Embolden( face->glyph );
      done++;
      TIMER_STOP( timer );
    }

    return done;
  }


  static int
  test_stroke( btimer_t*  timer,
               FT_Face    face,
               void*      user_data )
  {
    FT_Glyph    glyph;
    FT_Stroker  stroker;

    int  i, done = 0;

    FT_UNUSED( user_data );


    FT_Stroker_New( lib, &stroker );
    FT_Stroker_Set( stroker, face->size->metrics.y_ppem,
                    FT_STROKER_LINECAP_ROUND,
                    FT_STROKER_LINEJOIN_ROUND,
                    0 );

    FOREACH( i )
    {
      if ( FT_Load_Glyph( face, (FT_UInt)i, load_flags )  ||
           face->glyph->format != FT_GLYPH_FORMAT_OUTLINE ||
           FT_Get_Glyph( face->glyph, &glyph )            )
        continue;

      TIMER_START( timer );
      FT_Glyph_Stroke( &glyph, stroker, 1 );
      TIMER_STOP( timer );

      FT_Done_Glyph( glyph );
      done++;
    }

    FT_Stroker_Done( stroker );

    return done;
  }


  static int
  test_get_glyph( btimer_t*  timer,
                  FT_Face    face,
                  void*      user_data )
  {
    FT_Glyph  glyph;

    int  i, done = 0;

    FT_UNUSED( user_data );


    FOREACH( i )
    {
      if ( FT_Load_Glyph( face, (FT_UInt)i, load_flags ) )
        continue;

      TIMER_START( timer );
      if ( !FT_Get_Glyph( face->glyph, &glyph ) )
      {
        FT_Done_Glyph( glyph );
        done++;
      }
      TIMER_STOP( timer );
    }

    return done;
  }


  static int
  test_get_cbox( btimer_t*  timer,
                 FT_Face    face,
                 void*      user_data )
  {
    FT_Glyph  glyph;
    FT_BBox   bbox;

    int  i, done = 0;

    FT_UNUSED( user_data );


    FOREACH( i )
    {
      if ( FT_Load_Glyph( face, (FT_UInt)i, load_flags ) )
        continue;

      if ( FT_Get_Glyph( face->glyph, &glyph ) )
        continue;

      TIMER_START( timer );
      FT_Glyph_Get_CBox( glyph, FT_GLYPH_BBOX_PIXELS, &bbox );
      TIMER_STOP( timer );

      FT_Done_Glyph( glyph );
      done++;
    }

    return done;
  }


  static int
  test_get_bbox( btimer_t*  timer,
                 FT_Face    face,
                 void*      user_data )
  {
    FT_BBox    bbox;

    int  i, done = 0;

    FT_UNUSED( user_data );


    FOREACH( i )
    {
      if ( FT_Load_Glyph( face, (FT_UInt)i, load_flags ) )
        continue;

      TIMER_START( timer );
      FT_Outline_Get_BBox( &face->glyph->outline, &bbox );
      TIMER_STOP( timer );

      done++;
    }

    return done;
  }


  static int
  test_get_char_index( btimer_t*  timer,
                       FT_Face    face,
                       void*      user_data )
  {
    bcharset_t*  charset = (bcharset_t*)user_data;
    int          i, done = 0;


    TIMER_START( timer );

    for ( i = 0; i < charset->size; i++ )
    {
      if ( FT_Get_Char_Index(face, charset->code[i]) )
        done++;
    }

    TIMER_STOP( timer );

    return done;
  }


  static int
  test_cmap_cache( btimer_t*  timer,
                   FT_Face    face,
                   void*      user_data )
  {
    bcharset_t*  charset = (bcharset_t*)user_data;
    int          i, done = 0;

    FT_UNUSED( face );


    TIMER_START( timer );

    for ( i = 0; i < charset->size; i++ )
    {
      if ( FTC_CMapCache_Lookup( cmap_cache,
                                 font_type.face_id,
                                 cmap_index,
                                 charset->code[i] ) )
        done++;
    }

    TIMER_STOP( timer );

    return done;
  }


  static int
  test_image_cache( btimer_t*  timer,
                    FT_Face    face,
                    void*      user_data )
  {
    FT_Glyph  glyph;

    int  i, done = 0;

    FT_UNUSED( face );
    FT_UNUSED( user_data );


    TIMER_START( timer );

    FOREACH( i )
    {
      if ( !FTC_ImageCache_Lookup( image_cache,
                                   &font_type,
                                   (FT_UInt)i,
                                   &glyph,
                                   NULL ) )
        done++;
    }

    TIMER_STOP( timer );

    return done;
  }


  static int
  test_sbit_cache( btimer_t*  timer,
                   FT_Face    face,
                   void*      user_data )
  {
    FTC_SBit  glyph;

    int  i, done = 0;

    FT_UNUSED( face );
    FT_UNUSED( user_data );


    TIMER_START( timer );

    FOREACH( i )
    {
      if ( !FTC_SBitCache_Lookup( sbit_cache,
                                  &font_type,
                                  (FT_UInt)i,
                                  &glyph,
                                  NULL ) )
        done++;
    }

    TIMER_STOP( timer );

    return done;
  }


  static int
  test_cmap_iter( btimer_t*  timer,
                  FT_Face    face,
                  void*      user_data )
  {
    FT_UInt   idx;
    FT_ULong  charcode;
    int       done;

    FT_UNUSED( user_data );


    TIMER_START( timer );

    charcode = FT_Get_First_Char( face, &idx );
    done = ( idx != 0 );

    while ( idx != 0 )
      charcode = FT_Get_Next_Char( face, charcode, &idx );

    TIMER_STOP( timer );

    return done;
  }


  static int
  test_new_face( btimer_t*  timer,
                 FT_Face    face,
                 void*      user_data )
  {
    FT_Face  bench_face;

    FT_UNUSED( face );
    FT_UNUSED( user_data );


    TIMER_START( timer );

    if ( !get_face( &bench_face ) )
      FT_Done_Face( bench_face );

    TIMER_STOP( timer );

    return 1;
  }


  static int
  test_new_face_and_load_glyph( btimer_t*  timer,
                                FT_Face    face,
                                void*      user_data )
  {
    FT_Face  bench_face;

    int  i, done = 0;

    FT_UNUSED( face );
    FT_UNUSED( user_data );


    TIMER_START( timer );

    if ( !get_face( &bench_face ) )
    {
      FOREACH( i )
      {
        if ( !FT_Load_Glyph( bench_face, (FT_UInt)i, load_flags ) )
          done++;
      }

      FT_Done_Face( bench_face );
    }

    TIMER_STOP( timer );

    return done;
  }


  /*
   * main
   */

  static void
  get_charset( FT_Face      face,
               bcharset_t*  charset )
  {
    FT_ULong  charcode;
    int       i = 0;


    charset->code = (FT_ULong*)calloc( (size_t)face->num_glyphs,
                                       sizeof ( FT_ULong ) );
    if ( !charset->code )
      return;

    if ( face->charmap )
    {
      FT_UInt  idx;


      charcode = FT_Get_First_Char( face, &idx );

      /* certain fonts contain a broken charmap that will map character */
      /* codes to out-of-bounds glyph indices.  Take care of that here. */
      /*                                                                */
      while ( idx && i < face->num_glyphs )
      {
        FT_Int  gindex = (FT_Int)idx;


        if ( ( first_index <= gindex && gindex <= last_index ) ||
             ( first_index >= gindex && gindex >= last_index ) )
          charset->code[i++] = charcode;
        charcode = FT_Get_Next_Char( face, charcode, &idx );
      }
    }
    else
    {
      int  j;


      /* no charmap, do an identity mapping */
      FOREACH( j )
        charset->code[i++] = (FT_ULong)j;
    }

    charset->size = i;
  }


  static void
  header( FT_Face  face )
  {
    const FT_String*  target =
                 render_mode == FT_RENDER_MODE_NORMAL ? "normal" :
                 render_mode == FT_RENDER_MODE_LIGHT  ? "light"  :
                 render_mode == FT_RENDER_MODE_MONO   ? "mono"   :
                 render_mode == FT_RENDER_MODE_LCD    ? "lcd"    :
                 render_mode == FT_RENDER_MODE_LCD_V  ? "lcd-v"  :
                 render_mode == FT_RENDER_MODE_SDF    ? "sdf"    : "";
    const FT_String*  module_name = FT_FACE_DRIVER_NAME( face );
    const FT_String*  hinting_engine = "";
    FT_UInt           prop;


    if ( !FT_IS_SCALABLE( face ) )
      hinting_engine = "bitmap";

    else if ( load_flags & FT_LOAD_NO_SCALE )
      hinting_engine = "unscaled";

    else if ( load_flags & FT_LOAD_NO_HINTING )
      hinting_engine = "unhinted";

    else if ( render_mode == FT_RENDER_MODE_LIGHT )
      hinting_engine = "auto";

    else if ( load_flags == FT_LOAD_FORCE_AUTOHINT )
      hinting_engine = "auto";

    else if ( !FT_Property_Get( lib, module_name,
                                     "interpreter-version", &prop ) )
    {
      switch ( prop )
      {
      case TT_INTERPRETER_VERSION_35:
        hinting_engine = "v35";
        break;
      case TT_INTERPRETER_VERSION_40:
        hinting_engine = "v40";
        break;
      }
    }

    else if ( !FT_Property_Get( lib, module_name,
                                     "hinting-engine", &prop ) )
    {
      switch ( prop )
      {
      case FT_HINTING_FREETYPE:
        hinting_engine = "FT";
        break;
      case FT_HINTING_ADOBE:
        hinting_engine = "Adobe";
        break;
      }
    }

    printf( "\n"
            "family: %s\n"
            " style: %s\n"
            "driver: %s %s\n"
            "target: %s\n"
            " flags: 0x%X\n"
            "  cmap: %d\n"
            "glyphs: %ld\n",
            face->family_name,
            face->style_name,
            module_name, hinting_engine,
            target,
            load_flags,
            FT_Get_Charmap_Index( face->charmap ),
            face->num_glyphs );
  }


  static FT_Error
  get_face( FT_Face*  face )
  {
    static unsigned char*  memory_file = NULL;
    static size_t          memory_size;
    int                    face_index = 0;
    FT_Error               error;


    if ( preload )
    {
      if ( !memory_file )
      {
        FILE*  file = fopen( filename, "rb" );


        if ( file == NULL )
        {
          fprintf( stderr, "couldn't find or open `%s'\n", filename );

          return 1;
        }

        fseek( file, 0, SEEK_END );
        memory_size = (size_t)ftell( file );
        fseek( file, 0, SEEK_SET );

        memory_file = (FT_Byte*)malloc( memory_size );
        if ( memory_file == NULL )
        {
          fprintf( stderr,
                   "couldn't allocate memory to pre-load font file\n" );

          return 1;
        }

        if ( !fread( memory_file, memory_size, 1, file ) )
        {
          fprintf( stderr, "read error\n" );
          free( memory_file );
          memory_file = NULL;

          return 1;
        }
      }

      error = FT_New_Memory_Face( lib,
                                  memory_file,
                                  (FT_Long)memory_size,
                                  face_index,
                                  face );
    }
    else
      error = FT_New_Face( lib, filename, face_index, face );

    if ( error )
      fprintf( stderr, "couldn't load font resource\n");

    /* Set up MM_Var. */
    if ( requested_cnt != 0 )
    {
      unsigned int  n;


      error = FT_Get_MM_Var( *face, &multimaster );
      if ( error )
      {
        fprintf( stderr, "couldn't load MultiMaster info\n" );
        return error;
      }

      used_num_axes = multimaster->num_axis;

      for ( n = 0; n < used_num_axes; n++ )
      {
        FT_Var_Axis*  a = multimaster->axis + n;


        design_pos[n] = n < requested_cnt ? requested_pos[n] : a->def;

        if ( design_pos[n] < a->minimum )
          design_pos[n] = a->minimum;
        else if ( design_pos[n] > a->maximum )
          design_pos[n] = a->maximum;

        if ( !FT_IS_SFNT( *face ) )
          design_pos[n] = FT_RoundFix( design_pos[n] );
      }

      error = FT_Set_Var_Design_Coordinates( *face,
                                             used_num_axes,
                                             design_pos );
      if ( error )
      {
        fprintf( stderr, "couldn't set design coordinates\n" );
        return error;
      }

      FT_Done_MM_Var( lib, multimaster );
    }

    return error;
  }


  static void
  usage( void )
  {
    int   i;
    char  interpreter_versions[32];
    char  hinting_engines[32];


    /* we expect that at least one interpreter version is available */
    if ( num_tt_interpreter_versions == 1 )
      snprintf( interpreter_versions, sizeof ( interpreter_versions ),
                "%u",
                tt_interpreter_versions[0]);
    else
      snprintf( interpreter_versions, sizeof ( interpreter_versions ),
                "%u and %u",
                tt_interpreter_versions[0],
                tt_interpreter_versions[1] );

    /* we expect that at least one hinting engine is available */
    if ( num_ps_hinting_engines == 1 )
      snprintf( hinting_engines, sizeof ( hinting_engines ),
                "`%s'",
                ps_hinting_engine_names[ps_hinting_engines[0]] );
    else
      snprintf( hinting_engines, sizeof ( hinting_engines ),
                "`%s' and `%s'",
                ps_hinting_engine_names[ps_hinting_engines[0]],
                ps_hinting_engine_names[ps_hinting_engines[1]] );


    fprintf( stderr,
      "\n"
      "ftbench: run FreeType benchmarks\n"
      "--------------------------------\n"
      "\n"
      "Usage: ftbench [options] fontname\n"
      "\n"
      "  -C        Compare with cached version (if available).\n"
      "  -c N      Use at most N iterations for each test\n"
      "            (0 means time limited).\n"
      "  -e E      Set specific charmap index E.\n"
      "  -f L      Use hex number L as load flags (see `FT_LOAD_XXX').\n"
      "  -H NAME   Use PS hinting engine NAME.\n"
      "            Available versions are %s; default is `%s'.\n"
      "  -I VER    Use TT interpreter version VER.\n"
      "            Available versions are %s; default is version %u.\n"
      "  -i I-J    Forward or reverse range of glyph indices to use\n"
      "            (default is from 0 to the number of glyphs minus one).\n"
      "  -l N      Set LCD filter to N\n"
      "              0: none, 1: default, 2: light, 16: legacy\n"
      "  -m M      Set maximum cache size to M KiByte (default is %d).\n",
             hinting_engines,
             ps_hinting_engine_names[dflt_ps_hinting_engine],
             interpreter_versions,
             dflt_tt_interpreter_version,
             CACHE_SIZE );
    fprintf( stderr,
      "  -p        Preload font file in memory.\n"
      "  -r N      Set render mode to N\n"
      "              0: normal, 1: light, 2: mono, 3: LCD, 4: LCD vertical\n"
      "            (default is 0).\n"
      "  -s S      Use S ppem as face size (default is %dppem).\n"
      "            If set to zero, don't call FT_Set_Pixel_Sizes.\n"
      "            Use value 0 with option `-f 1' or something similar to\n"
      "            load the glyphs unscaled, otherwise errors will show up.\n",
             FACE_SIZE );
    fprintf( stderr,
      "  -t T      Use at most T seconds per bench (default is %.0f).\n"
      "\n"
      "  -b tests  Perform chosen tests (default is all):\n",
             BENCH_TIME );

    for ( i = 0; i < N_FT_BENCH; i++ )
    {
      if ( !bench_desc[i] )
        break;

      fprintf( stderr,
      "              %c  %s\n", 'a' + i, bench_desc[i] );
    }

    fprintf( stderr,
      "\n"
      "  -v        Show version.\n"
      "\n" );

    exit( 1 );
  }


#define TEST( x ) ( !test_string || strchr( test_string, (x) ) )


  static void
  parse_design_coords( char  *s )
  {
    for ( requested_cnt = 0;
          requested_cnt < MAX_MM_AXES && *s;
          requested_cnt++ )
    {
      requested_pos[requested_cnt] = (FT_Fixed)( strtod( s, &s ) * 65536.0 );

      while ( *s==' ' )
        ++s;
    }
  }


  int
  main( int     argc,
        char**  argv )
  {
    FT_Face   face;
    FT_Error  error;

    unsigned long  max_bytes      = CACHE_SIZE * 1024;
    char*          test_string    = NULL;
    unsigned int   size           = FACE_SIZE;
    int            max_iter       = 0;
    double         max_time       = BENCH_TIME;
    int            j;

    unsigned int  versions[2] = { TT_INTERPRETER_VERSION_35,
                                  TT_INTERPRETER_VERSION_40 };
    unsigned int  engines[2]  = { FT_HINTING_FREETYPE,
                                  FT_HINTING_ADOBE };
    int           version;
    char         *engine;

#if defined _WIN32 && defined QPC
    LARGE_INTEGER  freq;

    QueryPerformanceFrequency( &freq );
    interval = 1e6 / freq.QuadPart;
#endif


    if ( FT_Init_FreeType( &lib ) )
    {
      fprintf( stderr, "could not initialize font library\n" );

      return 1;
    }


    /* collect all available versions, then set again the default */
    FT_Property_Get( lib,
                     "truetype",
                     "interpreter-version", &dflt_tt_interpreter_version );
    for ( j = 0; j < 2; j++ )
    {
      error = FT_Property_Set( lib,
                               "truetype",
                               "interpreter-version", &versions[j] );
      if ( !error )
        tt_interpreter_versions[num_tt_interpreter_versions++] = versions[j];
    }
    FT_Property_Set( lib,
                     "truetype",
                     "interpreter-version", &dflt_tt_interpreter_version );

    FT_Property_Get( lib,
                     "cff",
                     "hinting-engine", &dflt_ps_hinting_engine );
    for ( j = 0; j < 2; j++ )
    {
      error = FT_Property_Set( lib,
                               "cff",
                               "hinting-engine", &engines[j] );
      if ( !error )
        ps_hinting_engines[num_ps_hinting_engines++] = engines[j];
    }
    FT_Property_Set( lib,
                     "cff",
                     "hinting-engine", &dflt_ps_hinting_engine );
    FT_Property_Set( lib,
                     "type1",
                     "hinting-engine", &dflt_ps_hinting_engine );
    FT_Property_Set( lib,
                     "t1cid",
                     "hinting-engine", &dflt_ps_hinting_engine );


    version = (int)dflt_tt_interpreter_version;
    engine  = ps_hinting_engine_names[dflt_ps_hinting_engine];

    while ( 1 )
    {
      int  opt;


      opt = getopt( argc, argv, "a:b:Cc:e:f:H:I:i:l:m:pr:s:t:v" );

      if ( opt == -1 )
        break;

      switch ( opt )
      {
      case 'a':
        parse_design_coords( optarg );
        break;

      case 'b':
        test_string = optarg;
        break;

      case 'C':
        FTC_Manager_New( lib,
                         0, 0, max_bytes,
                         face_requester,
                         NULL,
                         &cache_man );
        break;

      case 'c':
        max_iter = atoi( optarg );
        if ( max_iter < 0 )
          max_iter = -max_iter;
        break;

      case 'e':
        cmap_index = atoi( optarg );
        break;

      case 'f':
        load_flags = strtol( optarg, NULL, 16 );
        break;

      case 'H':
        engine = optarg;

        for ( j = 0; j < num_ps_hinting_engines; j++ )
        {
          if ( !strcmp( engine, ps_hinting_engine_names[j] ) )
          {
            FT_Property_Set( lib,
                             "cff",
                             "hinting-engine", &j );
            FT_Property_Set( lib,
                             "type1",
                             "hinting-engine", &j );
            FT_Property_Set( lib,
                             "t1cid",
                             "hinting-engine", &j );
            break;
          }
        }

        if ( j == num_ps_hinting_engines )
          fprintf( stderr,
                   "warning: couldn't set hinting engine\n" );
        break;

      case 'I':
        version = atoi( optarg );

        for ( j = 0; j < num_tt_interpreter_versions; j++ )
        {
          if ( version == (int)tt_interpreter_versions[j] )
          {
            FT_Property_Set( lib,
                             "truetype",
                             "interpreter-version", &version );
            break;
          }
        }

        if ( j == num_tt_interpreter_versions )
          fprintf( stderr,
                   "warning: couldn't set TT interpreter version\n" );
        break;

      case 'i':
        {
          int  fi, li;

          if ( sscanf( optarg, "%i%*[,:-]%i", &fi, &li ) == 2 )
          {
            first_index = fi < 0 ? 0 : fi;
            last_index  = li < 0 ? 0 : li;
          }
        }
        break;

      case 'l':
        {
          int  filter = atoi( optarg );


          switch ( filter )
          {
          case FT_LCD_FILTER_NONE:
          case FT_LCD_FILTER_DEFAULT:
          case FT_LCD_FILTER_LIGHT:
          case FT_LCD_FILTER_LEGACY1:
          case FT_LCD_FILTER_LEGACY:
            FT_Library_SetLcdFilter( lib, (FT_LcdFilter)filter );
          }
        }
        break;

      case 'm':
        {
          int  mb = atoi( optarg );


          if ( mb > 0 )
            max_bytes = (unsigned int)mb * 1024;
        }
        break;

      case 'p':
        preload = 1;
        break;

      case 'r':
        {
          int  rm = atoi( optarg );


          if ( rm < 0 || rm >= FT_RENDER_MODE_MAX )
            render_mode = FT_RENDER_MODE_NORMAL;
          else
            render_mode = (FT_Render_Mode)rm;
        }
        break;

      case 's':
        {
          int  sz = atoi( optarg );


          /* value 0 is special */
          if ( sz < 0 )
            size = 1;
          else
            size = (unsigned int)sz;
        }
        break;

      case 't':
        max_time = atof( optarg );
        if ( max_time < 0 )
          max_time = -max_time;
        break;

      case 'v':
        {
          FT_Int  major, minor, patch;


          FT_Library_Version( lib, &major, &minor, &patch );

          printf( "ftbench (FreeType) %d.%d", major, minor );
          if ( patch )
            printf( ".%d", patch );
          printf( "\n" );
          exit( 0 );
        }
        /* break; */

      default:
        usage();
        break;
      }
    }

    argc -= optind;
    argv += optind;

    if ( argc != 1 )
      usage();

    filename = *argv;

    if ( get_face( &face ) )
      goto Exit;

    j = printf( "\n"
                "ftbench results for font `%s'\n",
                filename ) - 2;
    while ( j-- )
      putchar( '-' );
    putchar( '\n' );

    if ( cmap_index >= face->num_charmaps )
      cmap_index = -1;
    if ( cmap_index >= 0 )
      face->charmap = face->charmaps[cmap_index];

    /* sync target and mode */
    load_flags |= FT_LOAD_TARGET_( render_mode );
    render_mode = (FT_Render_Mode)( ( load_flags & 0xF0000 ) >> 16 );

    header( face );

    if ( !face->num_glyphs )
      goto Exit;

    if ( first_index >= face->num_glyphs )
      first_index = face->num_glyphs - 1;
    if ( last_index >= face->num_glyphs )
      last_index = face->num_glyphs - 1;
    incr_index = last_index > first_index ? 1 : -1;

    if ( size )
    {
      if ( FT_IS_SCALABLE( face ) )
      {
        if ( FT_Set_Pixel_Sizes( face, size, size ) )
        {
          fprintf( stderr, "failed to set pixel size to %u\n", size );

          return 1;
        }
      }
      else
      {
        size = (unsigned int)face->available_sizes[0].size >> 6;
        fprintf( stderr,
                 "using size of first bitmap strike (%upx)\n", size );
        FT_Select_Size( face, 0 );
      }
    }

    if ( cache_man )
    {
      font_type.face_id = (FTC_FaceID)1;
      font_type.width   = size;
      font_type.height  = size;
      font_type.flags   = load_flags;
    }

    printf( "\n"
            "font preloading into memory: %s\n"
            "maximum cache size: %lu KiByte\n",
            preload ? "yes" : face->stream->base ? "mapped" : "no",
            max_bytes / 1024 );

    printf( "\n"
            "testing glyph indices from %d to %d at %u ppem\n"
            "number of seconds for each test: %s%g\n",
            first_index, last_index, size,
            max_iter ? "at most " : "", max_time );
    if ( max_iter )
      printf( "number of iterations for each test: at most %d\n",
              max_iter );

    printf( "\n"
            "executing tests:\n" );

    for ( j = 0; j < N_FT_BENCH; j++ )
    {
      btest_t   test;
      FT_ULong  flags;


      if ( !TEST( 'a' + j ) )
        continue;

      test.title       = NULL;
      test.bench       = NULL;
      test.cache_first = 0;
      test.user_data   = NULL;

      switch ( j )
      {
      case FT_BENCH_LOAD_GLYPH:
        test.title = "Load";
        test.bench = test_load;
        benchmark( face, &test, max_iter, max_time );

        if ( cache_man )
        {
          test.cache_first = 1;

          if ( !FTC_ImageCache_New( cache_man, &image_cache ) )
          {
            test.title = "Load (image cached)";
            test.bench = test_image_cache;
            benchmark( face, &test, max_iter, max_time );
          }

          if ( !FTC_SBitCache_New( cache_man, &sbit_cache ) )
          {
            test.title = "Load (sbit cached)";
            test.bench = test_sbit_cache;
            if ( size )
              benchmark( face, &test, max_iter, max_time );
            else
              printf( "  %-25s disabled (size = 0)\n", test.title );
          }
        }
        break;

      case FT_BENCH_LOAD_ADVANCES:
        test.user_data = &flags;

        test.title = "Load_Advances (Normal)";
        test.bench = test_load_advances;
        flags      = FT_LOAD_DEFAULT;
        benchmark( face, &test, max_iter, max_time );

        test.title  = "Load_Advances (Fast)";
        test.bench  = test_load_advances;
        flags       = FT_LOAD_TARGET_LIGHT;
        benchmark( face, &test, max_iter, max_time );

        test.title  = "Load_Advances (Unscaled)";
        test.bench  = test_load_advances;
        flags       = FT_LOAD_NO_SCALE;
        benchmark( face, &test, max_iter, max_time );
        break;

      case FT_BENCH_RENDER:
        test.title = "Render";
        test.bench = test_render;
        if ( size )
          benchmark( face, &test, max_iter, max_time );
        else
          printf( "  %-25s disabled (size = 0)\n", test.title );
        break;

      case FT_BENCH_GET_GLYPH:
        test.title = "Get_Glyph";
        test.bench = test_get_glyph;
        benchmark( face, &test, max_iter, max_time );
        break;

      case FT_BENCH_GET_CBOX:
        test.title = "Get_CBox";
        test.bench = test_get_cbox;
        benchmark( face, &test, max_iter, max_time );
        break;

      case FT_BENCH_GET_BBOX:
        test.title = "Get_BBox";
        test.bench = test_get_bbox;
        {
          FT_Matrix  rot30 = { 0xDDB4, -0x8000, 0x8000, 0xDDB4 };


          /* rotate outlines by 30 degrees so that CBox and BBox differ */
          FT_Set_Transform( face, &rot30, NULL );
          benchmark( face, &test, max_iter, max_time );
          FT_Set_Transform( face, NULL, NULL );
        }
        break;

      case FT_BENCH_CMAP:
        {
          bcharset_t  charset;


          get_charset( face, &charset );
          if ( charset.code )
          {
            test.user_data = (void*)&charset;


            test.title = "Get_Char_Index";
            test.bench = test_get_char_index;

            benchmark( face, &test, max_iter, max_time );

            if ( cache_man                                    &&
                 !FTC_CMapCache_New( cache_man, &cmap_cache ) )
            {
              test.cache_first = 1;

              test.title = "Get_Char_Index (cached)";
              test.bench = test_cmap_cache;
              benchmark( face, &test, max_iter, max_time );
            }

            free( charset.code );
          }
        }
        break;

      case FT_BENCH_CMAP_ITER:
        test.title = "Iterate CMap";
        test.bench = test_cmap_iter;
        benchmark( face, &test, max_iter, max_time );
        break;

      case FT_BENCH_NEW_FACE:
        test.title = "New_Face";
        test.bench = test_new_face;
        benchmark( face, &test, max_iter, max_time );
        break;

      case FT_BENCH_EMBOLDEN:
        test.title = "Embolden";
        test.bench = test_embolden;
        if ( size )
          benchmark( face, &test, max_iter, max_time );
        else
          printf( "  %-25s disabled (size = 0)\n", test.title );
        break;

      case FT_BENCH_STROKE:
        test.title = "Stroke";
        test.bench = test_stroke;
        if ( size )
          benchmark( face, &test, max_iter, max_time );
        else
          printf( "  %-25s disabled (size = 0)\n", test.title );
        break;

      case FT_BENCH_NEW_FACE_AND_LOAD_GLYPH:
        test.title = "New_Face & load glyph(s)";
        test.bench = test_new_face_and_load_glyph;
        benchmark( face, &test, max_iter, max_time );
        break;
      }
    }

    if ( cache_man )
      FTC_Manager_Done( cache_man );

  Exit:
    /* releases any remaining FT_Face object too */
    FT_Done_FreeType( lib );

    return 0;
  }


/* End */
