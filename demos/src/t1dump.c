/****************************************************************************/
/*                                                                          */
/* t1dump.c                                                            1.0  */
/*                                                                          */
/* Copyright 1999 - The FreeType Project http://www.freetype.org            */
/*                                                                          */
/* T1Dump is a very simply Type 1 font dumper. It can be used to            */
/* write the following information to the standard ouput, or any            */
/* given file:                                                              */
/*                                                                          */
/*  - a description of the font file (including name, properties, etc..)    */
/*  - the decrypted private dictionary, 'as is', i.e. in binary form        */
/*  - the stream of tokens from the font file (useful to debug the          */
/*    Type1 driver or to look at the font's internal structure..)           */
/*  - the charstring commands of a given subroutine                         */
/*  - the charstring commands of a given glyph                              */
/*  - the encoding                                                          */
/*  - the glyph names                                                       */
/*                                                                          */

#include <stdio.h>
#include <stdlib.h>

#include "freetype.h"
#include <t1tokens.h>
#include <t1gload.h>
#include <t1load.h>
#include <t1parse.h>

FT_Library      library;    /* root library object */
FT_Face         face;       /* truetype face */
T1_Face         t1_face;
FT_Error        error;
FILE*           target;

  void Panic( const char* message )
  {
    fprintf( stderr, "%s\n  error code = 0x%04x\n", message, error );
    exit(1);
  }


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**********                                                       *********/
/**********                                                       *********/
/**********              DUMP FONT INFORMATION                    *********/
/**********                                                       *********/
/**********                                                       *********/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/


  static
  T1_Error  Dump_Font_Info( void )
  {
    T1_FontInfo*  info = &t1_face->font_info;
    T1_Private*   priv = &t1_face->private_dict;
    T1_Int        n;

    fprintf( target, "Font Name    : %s\n", t1_face->font_name );

    fprintf( target, "Version      : %s\n", info->version );
    fprintf( target, "Full Name    : %s\n", info->full_name );
    fprintf( target, "Family       : %s\n", info->family_name );
    fprintf( target, "Weight       : %s\n", info->weight );
    fprintf( target, "Italic angle : %ld\n", info->italic_angle );

    fprintf( target, "Fixed pitch  : %s\n",
                     info->is_fixed_pitch ? "yes" : "no" );

    fprintf( target, "Underline    : pos %d, thickness %d\n",
                     info->underline_position,
                     info->underline_thickness );

    fprintf( target, "Unique ID    : %d\n", priv->unique_id );
    fprintf( target, "lenIV        : %d\n", priv->lenIV );

    fprintf( target, "blues        : [" );
    for ( n = 0; n < priv->num_blues; n++ )
    fprintf( target, " %d", priv->blue_values[n] );
    fprintf( target, " ]\n" );

    fprintf( target, "other blues  : [" );
    for ( n = 0; n < priv->num_other_blues; n++ )
    fprintf( target, " %d", priv->other_blues[n] );
    fprintf( target, " ]\n" );

    fprintf( target, "family blues : [" );
    for ( n = 0; n < priv->num_family_blues; n++ )
    fprintf( target, " %d", priv->family_blues[n] );
    fprintf( target, " ]\n" );

    fprintf( target, "family other : [" );
    for ( n = 0; n < priv->num_family_other_blues; n++ )
    fprintf( target, " %d", priv->family_other_blues[n] );
    fprintf( target, " ]\n" );

    fprintf( target, "Blue scale   : %f\n", priv->blue_scale*1.0/65536.0 );
    fprintf( target, "Blue shift   : %d\n", priv->blue_shift );
    fprintf( target, "Blue fuzz    : %d\n", priv->blue_fuzz );

    fprintf( target, "Std width    : %d\n", priv->standard_width );
    fprintf( target, "Std height   : %d\n", priv->standard_height );
    fprintf( target, "Force bold   : %s\n", priv->force_bold ? "yes" : "no" );
    fprintf( target, "Round stem   : %s\n", priv->round_stem_up ? "yes" : "no" );

    fprintf( target, "Stem snap W  : [" );
    for ( n = 0; n < priv->num_snap_widths; n++ )
    fprintf( target, " %d", priv->stem_snap_widths[n] );
    fprintf( target, " ]\n" );

    fprintf( target, "Stem snap H  : [" );
    for ( n = 0; n < priv->num_snap_heights; n++ )
    fprintf( target, " %d", priv->stem_snap_heights[n] );
    fprintf( target, " ]\n" );

    fprintf( target, "Language     : %ld\n", priv->language_group );
    fprintf( target, "Password     : %ld\n", priv->password );
    fprintf( target, "Min feature  : [ %d %d ]\n",
             priv->min_feature[0],
             priv->min_feature[1] );

    fprintf( target, "Font BBOX    : [ %ld %ld %ld %ld ]\n",
                     t1_face->font_bbox.xMin,
                     t1_face->font_bbox.yMin,
                     t1_face->font_bbox.xMax,
                     t1_face->font_bbox.yMax );

    fprintf( target, "Font matrix  : [ %f %f %f %f ]\n",
                     1.0*t1_face->font_matrix.xx/65536000.0,
                     1.0*t1_face->font_matrix.xy/65536000.0,
                     1.0*t1_face->font_matrix.yx/65536000.0,
                     1.0*t1_face->font_matrix.yy/65536000.0 );
#if 0
    fprintf( target,
    fprintf( target,
    fprintf( target,
    fprintf( target,
    fprintf( target,
    fprintf( target,
#endif
    fprintf( target, "Num glyphs   : %d\n", t1_face->num_glyphs );
    fprintf( target, "Num subrs    : %d\n", t1_face->num_subrs );
    
    return 0;
  }



/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**********                                                       *********/
/**********                                                       *********/
/**********              DUMP PRIVATE DICT IN RAW FORM            *********/
/**********                                                       *********/
/**********                                                       *********/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/


  static
  T1_Error  parse_int( T1_Tokenizer  tokzer,
                       T1_Long*      result )
  {
    T1_Bool    sign = 0;
    T1_Long    sum  = 0;
    T1_Token*  token = &tokzer->token;
    T1_Byte*   base  = tokzer->base + token->start;
    T1_Byte*   limit = base + token->len;

    if (base >= limit)
      goto Fail;

    /* check sign */
    if ( *base == '+' )
      base++;

    else if ( *base == '-' )
    {
      sign++;
      base++;
    }

    /* parse digits */
    if ( base >= limit )
      goto Fail;

    do
    {
      sum = ( 10*sum + (*base++ - '0') );

    } while (base < limit);

    if (sign)
      sum = -sum;

    *result = sum;
    return T1_Err_Ok;

  Fail:
    *result = 0;
    return T1_Err_Syntax_Error;
  }





  static
  T1_Error  Dump_Private_Dict( const char*  filename )
  {
    struct FT_StreamRec_  stream_rec;
    FT_Stream             stream = &stream_rec;
    T1_Error              error;
    T1_Tokenizer          tokenizer;

    error = FT_New_Stream( filename, stream );
    if (error) return error;

    stream->memory = library->memory;
    
    error = New_Tokenizer( stream, &tokenizer );
    if (error) goto Exit;

    /* go directly to the Private dictionary */
    error = Open_PrivateDict( tokenizer );
    if (error)
      Panic( "Could not open private dictionary !!" );

    /* Write it to the target file */
    fwrite( tokenizer->base, tokenizer->limit, 1, target );

  Exit:
    if (stream->close)
      stream->close(stream);

    return error;
  }



/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**********                                                       *********/
/**********                                                       *********/
/**********              DUMP TYPE 1 TOKEN STREAM                 *********/
/**********                                                       *********/
/**********                                                       *********/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/


  static
  T1_Error  Dump_Type1_Tokens( const char* filename )
  {
    struct FT_StreamRec_  stream_rec;
    FT_Stream             stream = &stream_rec;

    T1_Error      error;
    T1_Tokenizer  tokenizer;

    error = FT_New_Stream( filename, stream );
    if (error) return error;

    stream->memory = library->memory;
    
    error = New_Tokenizer( stream, &tokenizer );
    if (error) goto Exit;

    /* Dump the first segment of the Type1 font */
    do
    {
      T1_Token*  token;
      T1_String  temp_string[128];
      T1_Int     len;

      error = Read_Token( tokenizer );
      if (error) { error = 0; break; }

      /* dump the token */
      token = &tokenizer->token;
      len   = token->len;
      if (len > 127) len = 127;

      strncpy( temp_string,
               (T1_String*)(tokenizer->base + token->start),
               len );
      temp_string[len] = '\0';

      fprintf( target, "%s\n", temp_string );

	  /* Exit the loop when we encounter a "currentfile" token */
	  if ( token->kind  == tok_keyword &&
	       token->kind2 == key_currentfile )
		break;

    } while (1);

    error = Open_PrivateDict( tokenizer );
    if (error)
      Panic( "** could not open private dictionary **\n" );
    else
    {
      T1_Int   num = 0;
      T1_Bool  last_num = 0;

      do
      {
        T1_Token*  token;
        T1_String  temp_string[128];
        T1_Int     len;

        error = Read_Token( tokenizer );
        if (error) { error = 0; break; }

        /* dump the token */
        token = &tokenizer->token;
        len   = token->len;
        if (len > 127) len = 127;

        strncpy( temp_string,
                 (T1_String*)(tokenizer->base + token->start),
                 len );
        temp_string[len] = '\0';

        /* detect "RD" uses */
        if ( token->kind  == tok_keyword          &&
             ( token->kind2 == key_RD ||
               token->kind2 == key_RD_alternate ) &&
             last_num                             )
        {
          fprintf( target, "%s [%d binary bytes] ", temp_string, num );
          tokenizer->cursor += num;
        }
        else
        {
          fprintf( target, "%s\n", temp_string );

          /* exit dump when we encounter a 'closefile' */
          if ( token->kind  == tok_keyword   &&
               token->kind2 == key_closefile )
            break;

          /* record numerical value if any */
          if ( token->kind == tok_number )
          {
            T1_Long  sum;

            if ( !parse_int( tokenizer, &sum ) )
            {
              num      = sum;
              last_num = 1;
            }
            else
              last_num = 0;
          }
          else
            last_num = 0;
        }

      } while (1);
    }

  Exit:
    if (stream->close)
      stream->close(stream);
    return error;
  }


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**********                                                       *********/
/**********                                                       *********/
/**********                DUMP CHARACTER ENCODING                *********/
/**********                                                       *********/
/**********                                                       *********/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/


  static
  void  Dump_Encoding( void )
  {
	T1_Encoding*  encode = &t1_face->encoding;
	int           n;

	fprintf( target, "characters count = %d\n", encode->num_chars );

	fprintf( target, "first code = %d, last code = %d\n",
			 encode->code_first, encode->code_last );

    for ( n = 0; n < encode->num_chars; n++ )
    {
	  int  code = (int)encode->char_index[n];

	  if (code || n == 0)
	    fprintf( target, "%3d %s\n", n, t1_face->glyph_names[code] );
    }
  }


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**********                                                       *********/
/**********                                                       *********/
/**********        DUMP SUBROUTINES AND GLYPH CHARSTRINGS         *********/
/**********                                                       *********/
/**********                                                       *********/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/


  static
  void  Dump_CharStrings( T1_Byte*  base,
                          T1_Int    len )
  {
    T1_Byte*   cur   = base;
    T1_Byte*   limit = base + len;
    T1_String  temp_name[128];
    T1_String* string;

    T1_Int     x = 0;

    while ( cur < limit )
    {
      switch (*cur++)
      {
        case 1:  string = "hstem";     break;

        case 3:  string = "vstem";     break;
        case 4:  string = "vmoveto";   break;
        case 5:  string = "rlineto";   break;
        case 6:  string = "hlineto";   break;
        case 7:  string = "vlineto";   break;
        case 8:  string = "rrcurveto"; break;
        case 9:  string = "closepath"; break;
        case 10: string = "callsubr";  break;
        case 11: string = "return";    break;

        case 13: string = "hsbw";      break;
        case 14: string = "endchar";   break;

        case 21: string = "rmoveto";   break;
        case 22: string = "hmoveto";   break;

        case 30: string = "vhcurveto"; break;
        case 31: string = "hvcurveto";  break;

        case 12:
          {
            if (cur > limit)
              Panic( "invalid charstrings stream\n" );

            switch (*cur++)
            {
              case 0:  string = "dotsection";      break;
              case 1:  string = "vstem3";          break;
              case 2:  string = "hstem3";          break;
              case 6:  string = "seac";            break;
              case 7:  string = "sbw";             break;
              case 12: string = "div";             break;
              case 16: string = "callothersubr";   break;
              case 17: string = "pop";             break;
              case 33: string = "setcurrentpoint"; break;

              default:
                sprintf( temp_name, "escape(12)+unknown(%d)", cur[1] );
                string = temp_name;
            }
          }
          break;

        case 255:    /* four bytes integer */
          {
            T1_Long  sum;

            if (cur+4 > limit)
              Panic( "invalid charstrings stream\n" );

            sum = ((long)cur[0] << 24) |
                  ((long)cur[1] << 16) |
                  ((long)cur[2] << 8)  |
                         cur[3];
            sprintf( temp_name, "%ld ", sum );
            string = temp_name;
            cur += 4;
          }
          break;

        default:
          if (cur[-1] >= 32)
          {
            if (cur[-1] < 247)
            {
              sprintf( temp_name, "%ld", (long)cur[-1] - 139 );
            }
            else if (cur[-1] < 251)
            {
              cur++;
              sprintf( temp_name, "%ld",
                       ((long)(cur[-2]-247) << 8) + cur[-1] + 108 );
            }
            else
            {
              cur++;
              sprintf( temp_name, "%ld",
                       -((long)(cur[-2]-251) << 8) - cur[-1] - 108 );
            }
            string = temp_name;
          }
          else
          {
            sprintf( temp_name, "unknown(%d)", cur[-1] );
            string = temp_name;
          }
      }

      /* now print the charstring command */
      {
        int  len = strlen(string)+1;

        if ( x+len > 60 )
        {
          x = 0;
          fprintf( target, "\n" );
        }
        else
          fprintf( target, " " );

        fprintf( target, "%s", string );
        x += len;
      }
    }
  }



  static
  void  Dump_Glyph( int  glyph_index )
  {
    fprintf( target, "glyph name: %s\n", t1_face->glyph_names[glyph_index] );
    Dump_CharStrings( t1_face->charstrings     [glyph_index],
                      t1_face->charstrings_len [glyph_index] );
  }

  static
  void  Dump_Subrs( int  subrs_index )
  {
    Dump_CharStrings( t1_face->subrs    [ subrs_index ],
                      t1_face->subrs_len[ subrs_index ] );
  }



/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**********                                                       *********/
/**********                                                       *********/
/**********                EXECUTE GLYPH CHARSTRINGS              *********/
/**********                                                       *********/
/**********                                                       *********/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/


         static
         T1_Error  operator_endchar( T1_Builder*  builder )
         {
           (void)builder;
           fprintf( target, "endchar\n" );
           return 0;
         }
       
       
         static
         T1_Error  operator_sbw( T1_Builder*  builder,
                                 T1_Pos       sbx,
                                 T1_Pos       sby,
                                 T1_Pos       wx,
                                 T1_Pos       wy )
         {
           (void)builder;
           fprintf( target, "set bearing [%ld,%ld] width [%ld,%ld]\n",
                            sbx, sby, wx, wy );
           return 0;
         }
       
#if 0
         static
         T1_Error  operator_seac( T1_Builder*  builder,
                                  T1_Pos       asb,
                                  T1_Pos       adx,
                                  T1_Pos       ady,
                                  T1_Int       bchar,
                                  T1_Int       achar )
         {
           (void)builder;
           fprintf( target, "accented char: %ld [%ld,%ld] b=%d, a=%d\n",
                            asb, adx, ady, bchar, achar );
           return 0;
         }
#endif
       
         static
         T1_Error  operator_closepath( T1_Builder*  builder )
         {
           (void)builder;
           fprintf( target, "closepath\n" );
           return 0;
         }
       
       
         static
         T1_Error  operator_rlineto( T1_Builder*  builder,
                                     T1_Pos       dx,
                                     T1_Pos       dy )
         {
           (void)builder;
           fprintf( target, "%ld %ld rlineto\n", dx, dy );
           return 0;
         }
       
       
         static
         T1_Error  operator_rmoveto( T1_Builder*  builder,
                                     T1_Pos       dx,
                                     T1_Pos       dy )
         {
           (void)builder;
           fprintf( target, "%ld %ld rmoveto\n", dx, dy );
           return 0;
         }
       
       
         static
         T1_Error  operator_rrcurveto( T1_Builder*  builder,
                                       T1_Pos       dx1,
                                       T1_Pos       dy1,
                                       T1_Pos       dx2,
                                       T1_Pos       dy2,
                                       T1_Pos       dx3,
                                       T1_Pos       dy3 )
         {
           (void)builder;
           fprintf( target, "%ld %ld %ld %ld %ld %ld rrcurveto\n",
                    dx1, dy1, dx2, dy2, dx3, dy3 );
           return 0;
         }
       
       
         static
         T1_Error  operator_dotsection( T1_Builder*  builder )
         {
           (void)builder;
           fprintf( target, "dotsection\n" );
           return 0;
         }
       
       
         static
         T1_Error  operator_stem( T1_Builder*  builder,
                                  T1_Pos       pos,
                                  T1_Pos       width,
                                  T1_Bool      vertical )
         {
           (void)builder;
           fprintf( target, "%ld %ld %s\n", pos, width,
                    vertical ? "vstem" : "hstem" );
           return 0;
         }
       
       
         static
         T1_Error  operator_stem3( T1_Builder*  builder,
                                   T1_Pos       pos0,
                                   T1_Pos       width0,
                                   T1_Pos       pos1,
                                   T1_Pos       width1,
                                   T1_Pos       pos2,
                                   T1_Pos       width2,
                                   T1_Bool      vertical )
         {
           (void)builder;
           fprintf( target, "%ld %ld %ld %ld %ld %ld %s\n",
                    pos0, width0, pos1, width1, pos2, width2,
                    vertical ? "vstem3" : "hstem3" );
           return 0;
         }
       
       
#if 0
         static
         T1_Error  operator_flex( T1_Builder*  builder,
                                  T1_Pos       threshold,
                                  T1_Pos       end_x,
                                  T1_Pos       end_y )
         {
           (void)builder;
           fprintf( target, "%ld %ld %ld flex\n", threshold, end_x, end_y );
           return 0;
         }
#endif  
       
         static
         T1_Error  operator_changehints( T1_Builder*  builder )
         {
           (void)builder;
           fprintf( target, "-- change hints --\n" );
           return 0;
         }
       
       
  static
  T1_Error  Execute_CharString( int  glyph_index )
  {
    static const T1_Builder_Funcs   builds =
    {
      operator_endchar,
      operator_sbw,
      operator_closepath,
      operator_rlineto,
      operator_rmoveto,
      operator_rrcurveto,
    };

    static const T1_Hinter_Funcs  hints =
    {
      operator_dotsection,
      operator_changehints,
      operator_stem,
      operator_stem3,
    };

    T1_Decoder  decoder;
    T1_Error    error;

    T1_Init_Decoder( &decoder, &hints );
    T1_Init_Builder( &decoder.builder, t1_face, 0, 0, &builds );

    error = T1_Parse_CharStrings( &decoder,
                                  t1_face->charstrings    [glyph_index],
                                  t1_face->charstrings_len[glyph_index],
                                  t1_face->num_subrs,
                                  t1_face->subrs,
                                  t1_face->subrs_len );
   return error;                               
  }








/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**********                                                       *********/
/**********                                                       *********/
/**********                  DEBUG FONT LOADING                   *********/
/**********                                                       *********/
/**********                                                       *********/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

  static
  T1_Error  Debug_Type1_Font( const char*  filename )
  {
    struct FT_StreamRec_  stream_rec;
    T1_FaceRec    t1facerec;
    T1_Tokenizer  tokenizer;
    T1_Parser     parser;
    T1_Error      error;
    FT_Stream     stream = &stream_rec;

    error = FT_New_Stream( filename, stream );
    if (error) goto Exit;

    stream->memory = library->memory;
    
    /* create an empty face record */
    memset( &t1facerec, 0, sizeof(t1facerec) );
    t1facerec.root.memory = library->memory;
    t1facerec.root.stream = stream;

    t1_face = &t1facerec;

    /* open the tokenizer, this will also check the font format */
	error = New_Tokenizer( stream, &tokenizer );
    if (error) goto Fail;

    /* Now, load the font program into the face object */
    Init_T1_Parser( &parser, t1_face, tokenizer );

    /* force token dump */
    parser.dump_tokens = 1;

    error = Parse_T1_FontProgram( &parser );

    Done_Tokenizer( tokenizer );

  Fail:
    if (stream->close)
      stream->close( stream );
  Exit:
    return error;
  }

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**********                                                       *********/
/**********                                                       *********/
/**********                     MAIN PROGRAM                      *********/
/**********                                                       *********/
/**********                                                       *********/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/


  static
  void Usage()
  {
    fprintf( stderr, "t1dump       -  a simple Type 1 font dumper\n" );
    fprintf( stderr, "(c) The FreeType project - www.freetype.org\n" );
    fprintf( stderr, "-------------------------------------------\n\n" );

    fprintf( stderr, "usage :   t1dump  [options] fontfile(.pfb|.pfa)\n\n" );

    fprintf( stderr, "    options\n" );
    fprintf( stderr, "          -o filename : dumps to a specific file\n" );
    fprintf( stderr, "          -g index    : dump glyph charstring\n" );
    fprintf( stderr, "          -s index    : dump subrs charstring\n" );
    fprintf( stderr, "          -x index    : execute glyph charstring\n" );
	fprintf( stderr, "          -e          : dump encoding\n" );
    fprintf( stderr, "          -t          : dumps the Type 1 token stream\n" );
    fprintf( stderr, "          -d          : debug font loading\n" );
    fprintf( stderr, "          -p          : dumps private dictionary 'as is'\n\n" );

    exit(1);
  }


typedef enum Request_
{
  req_dump_info,
  req_dump_private,
  req_dump_tokens,
  req_dump_encoding,
  req_dump_glyph,
  req_dump_subr,
  req_load_font,
  req_execute_glyph,
  req_debug_font

} Request;


static char*    file_name;
static int      glyph_index;
static Request  request = req_dump_info;

static FT_Driver t1_driver;

  int  main( int argc, char**  argv )
  {
    char  valid;


    /* Check number of arguments */
    if ( argc < 2 ) Usage();

    /* Check options */
    target = stdout;

    argv++;
    while (argv[0][0] == '-')
    {
      valid = 0;
      switch (argv[0][1])
      {
        case 'p':
          request = req_dump_private;
          valid   = 1;
          break;

        case 't':
          request = req_dump_tokens;
          valid   = 1;
          break;

        case 'e':
          request = req_dump_encoding;
          valid   = 1;
          break;

        case 'd':
          request = req_debug_font;
          valid   = 1;
          break;

        case 'o':
          if (argc < 2) Usage();
          target = fopen( argv[1], "w" );
          if (!target)
            Panic( "Could not open/create destination file" );
          argv++;
          argc--;
          valid = 1;
          break;

        case 'g':
        case 's':
        case 'x':
          if (argc < 2) Usage();
          if ( sscanf( argv[1], "%d", &glyph_index ) != 1 )
            Usage();

          switch (argv[0][1])
          {
            case 'g': request = req_dump_glyph;    break;
            case 's': request = req_dump_subr;     break;
            case 'x': request = req_execute_glyph; break;
          }
          argv++;
          argc--;
          valid = 1;
          break;

        default:
          ;
      }

      if (valid)
      {
        argv++;
        argc--;
        if (argc < 2) Usage();
      }
      else
        break;
    }

    /* Get file name */
    file_name = argv[0];

    /* Instead of calling FT_Init_FreeType, we set up our own system  */
    /* object and library. This is reserved for FreeType 2 wizards !! */

    /* Init library, read face object, get driver, create size */
    error = FT_Init_FreeType( &library );
    if (error) Panic( "could not initialise FreeType library" );

    t1_driver = FT_Get_Driver( library, "type1" );
    if (!t1_driver) Panic( "no Type1 driver in current FreeType lib" );

    error = FT_New_Face( library, file_name, 0, &face );
    if (error) Panic( "could not find/open/create font file" );
    
    if (face->driver != t1_driver)
      Panic( "font format is not Type 1 !" );

    switch (request)
    {
      case req_dump_private:
        error = Dump_Private_Dict(file_name);
        break;

      case req_dump_tokens:
        error = Dump_Type1_Tokens(file_name);
        break;

      case req_debug_font:
        error = Debug_Type1_Font(file_name);
        break;

      default:
        error = FT_New_Face( library, file_name, 0, &face );
        if (error) Panic( "could not load Type 1 font" );

        t1_face = (T1_Face)face;

        /* check glyph index, it is 0 by default */
        if ( glyph_index < 0 || glyph_index >= t1_face->num_glyphs )
          Panic( "invalid glyph index\n" );

        switch (request)
        {
          case req_dump_glyph:
            Dump_Glyph( glyph_index );
            break;

          case req_dump_subr:
            Dump_Subrs( glyph_index );
            break;

          case req_execute_glyph:
            Execute_CharString( glyph_index );
            break;

          case req_dump_encoding:
            Dump_Encoding();
            break;

          default:
            Dump_Font_Info();
        }
    }

    if (error) Panic( "could not dump Type 1 font" );

    FT_Done_FreeType( library );
    return 0;
  }
