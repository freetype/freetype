/***************************************************************************/
/*                                                                         */
/*  t42drivr.c                                                             */
/*                                                                         */
/*    FreeType font driver for Type 42 fonts (body only).                  */
/*                                                                         */
/*  Copyright 2002 by Roberto Alameda.                                     */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#include <ft2build.h>
#define FT_ERR_PREFIX  T42_Err_
#define FT_ERR_BASE    FT_Mod_Err_Type42

#include FT_CONFIG_STANDARD_LIBRARY_H

#include FT_INTERNAL_DEBUG_H
#include FT_CONFIG_CONFIG_H

#include FT_ERRORS_H
#include FT_FREETYPE_H
#include FT_TYPE1_TABLES_H
#include FT_LIST_H

#include FT_INTERNAL_DRIVER_H
#include FT_INTERNAL_OBJECTS_H
#include FT_INTERNAL_TYPE1_TYPES_H
#include FT_INTERNAL_TYPE42_TYPES_H
#include FT_INTERNAL_POSTSCRIPT_AUX_H
#include FT_INTERNAL_STREAM_H


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_t42


  /********************* Data Definitions ******************/

  typedef struct  T42_DriverRec_
  {
    FT_DriverRec     root;
    FT_Driver_Class  ttclazz;
    void*            extension_component;

  } T42_DriverRec, *T42_Driver;


  typedef struct  T42_SizeRec_
  {
    FT_SizeRec  root;
    FT_Size     ttsize;

  } T42_SizeRec, *T42_Size;


  typedef struct  T42_GlyphSlotRec_
  {
    FT_GlyphSlotRec  root;
    FT_GlyphSlot     ttslot;

  } T42_GlyphSlotRec, *T42_GlyphSlot;


  /*********** Parser definitions *************/

  typedef struct  T42_ParserRec_
  {
    PS_ParserRec  root;
    FT_Stream     stream;

    FT_Byte*      base_dict;
    FT_Int        base_len;

    FT_Byte       in_memory;

  } T42_ParserRec, *T42_Parser;


  typedef struct  T42_Loader_
  {
    T42_ParserRec  parser;          /* parser used to read the stream */

    FT_Int         num_chars;       /* number of characters in encoding */
    PS_TableRec    encoding_table;  /* PS_Table used to store the       */
                                    /* encoding character names         */

    FT_Int         num_glyphs;
    PS_TableRec    glyph_names;
    PS_TableRec    charstrings;

  } T42_LoaderRec, *T42_Loader;


  /*********************** Prototypes *********************/

  static void
  parse_font_name( T42_Face    face,
                   T42_Loader  loader );
  static void
  parse_font_bbox( T42_Face    face,
                   T42_Loader  loader );
  static void
  parse_font_matrix( T42_Face    face,
                     T42_Loader  loader );
  static void
  parse_encoding( T42_Face    face,
                  T42_Loader  loader );
  static void
  parse_charstrings( T42_Face    face,
                     T42_Loader  loader );
  static void
  parse_sfnts( T42_Face    face,
               T42_Loader  loader );


  static const
  T1_FieldRec  t42_keywords[] = {

#undef  FT_STRUCTURE
#define FT_STRUCTURE  T1_FontInfo
#undef  T1CODE
#define T1CODE        T1_FIELD_LOCATION_FONT_INFO

    T1_FIELD_STRING   ( "version",            version )
    T1_FIELD_STRING   ( "Notice",             notice )
    T1_FIELD_STRING   ( "FullName",           full_name )
    T1_FIELD_STRING   ( "FamilyName",         family_name )
    T1_FIELD_STRING   ( "Weight",             weight )
    T1_FIELD_NUM      ( "ItalicAngle",        italic_angle )
    T1_FIELD_TYPE_BOOL( "isFixedPitch",       is_fixed_pitch )
    T1_FIELD_NUM      ( "UnderlinePosition",  underline_position )
    T1_FIELD_NUM      ( "UnderlineThickness", underline_thickness )

#undef  FT_STRUCTURE
#define FT_STRUCTURE  T42_FontRec
#undef  T1CODE
#define T1CODE        T1_FIELD_LOCATION_FONT_DICT

    T1_FIELD_NUM( "PaintType",   paint_type )
    T1_FIELD_NUM( "FontType",    font_type )
    T1_FIELD_NUM( "StrokeWidth", stroke_width )

    T1_FIELD_CALLBACK( "FontName",    parse_font_name )
    T1_FIELD_CALLBACK( "FontBBox",    parse_font_bbox )
    T1_FIELD_CALLBACK( "FontMatrix",  parse_font_matrix )
    T1_FIELD_CALLBACK( "Encoding",    parse_encoding )
    T1_FIELD_CALLBACK( "CharStrings", parse_charstrings )
    T1_FIELD_CALLBACK( "sfnts",       parse_sfnts )

    { 0, T1_FIELD_LOCATION_CID_INFO, T1_FIELD_TYPE_NONE, 0, 0, 0, 0, 0 }
  };


#define T1_Add_Table( p, i, o, l )  (p)->funcs.add( (p), i, o, l )
#define T1_Done_Table( p )          \
          do                        \
          {                         \
            if ( (p)->funcs.done )  \
              (p)->funcs.done( p ); \
          } while ( 0 )
#define T1_Release_Table( p )          \
          do                           \
          {                            \
            if ( (p)->funcs.release )  \
              (p)->funcs.release( p ); \
          } while ( 0 )

#define T1_Skip_Spaces( p )  (p)->root.funcs.skip_spaces( &(p)->root )
#define T1_Skip_Alpha( p )   (p)->root.funcs.skip_alpha ( &(p)->root )

#define T1_ToInt( p )       (p)->root.funcs.to_int( &(p)->root )
#define T1_ToFixed( p, t )  (p)->root.funcs.to_fixed( &(p)->root, t )

#define T1_ToCoordArray( p, m, c )                           \
          (p)->root.funcs.to_coord_array( &(p)->root, m, c )
#define T1_ToFixedArray( p, m, f, t )                           \
          (p)->root.funcs.to_fixed_array( &(p)->root, m, f, t )
#define T1_ToToken( p, t )                          \
          (p)->root.funcs.to_token( &(p)->root, t )
#define T1_ToTokenArray( p, t, m, c )                           \
          (p)->root.funcs.to_token_array( &(p)->root, t, m, c )

#define T1_Load_Field( p, f, o, m, pf )                         \
          (p)->root.funcs.load_field( &(p)->root, f, o, m, pf )
#define T1_Load_Field_Table( p, f, o, m, pf )                         \
          (p)->root.funcs.load_field_table( &(p)->root, f, o, m, pf )


  /********************* Parsing Functions ******************/

  static FT_Error
  T42_New_Parser( T42_Parser     parser,
                  FT_Stream      stream,
                  FT_Memory      memory,
                  PSAux_Service  psaux )
  {
    FT_Error  error = T42_Err_Ok;
    FT_Long   size;


    psaux->ps_parser_funcs->init( &parser->root, 0, 0, memory );

    parser->stream    = stream;
    parser->base_len  = 0;
    parser->base_dict = 0;
    parser->in_memory = 0;

    /*******************************************************************/
    /*                                                                 */
    /* Here a short summary of what is going on:                       */
    /*                                                                 */
    /*   When creating a new Type 42 parser, we try to locate and load */
    /*   the base dictionary, loading the whole font into memory.      */
    /*                                                                 */
    /*   When `loading' the base dictionary, we only setup pointers in */
    /*   the case of a memory-based stream.  Otherwise, we allocate    */
    /*   and load the base dictionary in it.                           */
    /*                                                                 */
    /*   parser->in_memory is set if we have a memory stream.          */
    /*                                                                 */

    if ( FT_STREAM_SEEK( 0L ) )
      goto Exit;

    size = stream->size;

    /* now, try to load `size' bytes of the `base' dictionary we */
    /* found previously                                          */

    /* if it is a memory-based resource, set up pointers */
    if ( !stream->read )
    {
      parser->base_dict = (FT_Byte*)stream->base + stream->pos;
      parser->base_len  = size;
      parser->in_memory = 1;

      /* check that the `size' field is valid */
      if ( FT_STREAM_SKIP( size ) )
        goto Exit;
    }
    else
    {
      /* read segment in memory */
      if (FT_ALLOC( parser->base_dict, size )       ||
          FT_STREAM_READ( parser->base_dict, size ) )
        goto Exit;
      parser->base_len = size;
    }

    /* Now check font format; we must see `%!PS-TrueTypeFont' */
    if (size <= 17                                    ||
        ( ft_strncmp( (const char*)parser->base_dict,
                      "%!PS-TrueTypeFont", 17) )      )
      error = T42_Err_Unknown_File_Format;
    else
    {
      parser->root.base   = parser->base_dict;
      parser->root.cursor = parser->base_dict;
      parser->root.limit  = parser->root.cursor + parser->base_len;
    }

  Exit:
    if ( error && !parser->in_memory )
      FT_FREE( parser->base_dict );

    return error;
  }


  static void
  T42_Finalize_Parser( T42_Parser  parser )
  {
    FT_Memory  memory = parser->root.memory;


    /* free the base dictionary only when we have a disk stream */
    if ( !parser->in_memory )
      FT_FREE( parser->base_dict );

    parser->root.funcs.done( &parser->root );
  }


  static int
  is_alpha( FT_Byte  c )
  {
    /* Note: we must accept "+" as a valid character, as it is used in */
    /*       embedded type1 fonts in PDF documents.                    */
    /*                                                                 */
    return ( ft_isalnum( c ) ||
             c == '.'        ||
             c == '_'        ||
             c == '-'        ||
             c == '+'        );
  }


  static int
  is_space( FT_Byte  c )
  {
    return ( c == ' ' || c == '\t' || c == '\r' || c == '\n' );
  }


  static void
  parse_font_name( T42_Face    face,
                   T42_Loader  loader )
  {
    T42_Parser  parser = &loader->parser;
    FT_Error    error;
    FT_Memory   memory = parser->root.memory;
    FT_Int      len;
    FT_Byte*    cur;
    FT_Byte*    cur2;
    FT_Byte*    limit;


    T1_Skip_Spaces( parser );

    cur   = parser->root.cursor;
    limit = parser->root.limit;

    if ( cur >= limit - 1              ||
         ( *cur != '/' && *cur != '(') )
      return;

    cur++;
    cur2 = cur;
    while ( cur2 < limit && is_alpha( *cur2 ) )
      cur2++;

    len = (FT_Int)( cur2 - cur );
    if ( len > 0 )
    {
      if ( FT_ALLOC( face->type42.font_name, len + 1 ) )
      {
        parser->root.error = error;
        return;
      }

      FT_MEM_COPY( face->type42.font_name, cur, len );
      face->type42.font_name[len] = '\0';
    }
    parser->root.cursor = cur2;
  }


  static void
  parse_font_bbox( T42_Face   face,
                  T42_Loader  loader )
  {
    T42_Parser  parser = &loader->parser;
    FT_BBox*    bbox   = &face->type42.font_bbox;

    bbox->xMin = T1_ToInt( parser );
    bbox->yMin = T1_ToInt( parser );
    bbox->xMax = T1_ToInt( parser );
    bbox->yMax = T1_ToInt( parser );
  }


  static void
  parse_font_matrix( T42_Face    face,
                     T42_Loader  loader )
  {
    T42_Parser  parser = &loader->parser;
    FT_Matrix*  matrix = &face->type42.font_matrix;
    FT_Vector*  offset = &face->type42.font_offset;
    FT_Face     root   = (FT_Face)&face->root;
    FT_Fixed    temp[6];
    FT_Fixed    temp_scale;


    (void)T1_ToFixedArray( parser, 6, temp, 3 );

    temp_scale = ABS( temp[3] );

    /* Set Units per EM based on FontMatrix values.  We set the value to */
    /* 1000 / temp_scale, because temp_scale was already multiplied by   */
    /* 1000 (in t1_tofixed, from psobjs.c).                              */

    root->units_per_EM = (FT_UShort)( FT_DivFix( 1000 * 0x10000L,
                                                 temp_scale ) >> 16 );

    /* we need to scale the values by 1.0/temp_scale */
    if ( temp_scale != 0x10000L ) {
      temp[0] = FT_DivFix( temp[0], temp_scale );
      temp[1] = FT_DivFix( temp[1], temp_scale );
      temp[2] = FT_DivFix( temp[2], temp_scale );
      temp[4] = FT_DivFix( temp[4], temp_scale );
      temp[5] = FT_DivFix( temp[5], temp_scale );
      temp[3] = 0x10000L;
    }

    matrix->xx = temp[0];
    matrix->yx = temp[1];
    matrix->xy = temp[2];
    matrix->yy = temp[3];

    /* note that the offsets must be expressed in integer font units */
    offset->x  = temp[4] >> 16;
    offset->y  = temp[5] >> 16;
  }


  static void
  parse_encoding( T42_Face    face,
                  T42_Loader  loader )
  {
    T42_Parser     parser = &loader->parser;
    FT_Byte*       cur    = parser->root.cursor;
    FT_Byte*       limit  = parser->root.limit;

    PSAux_Service  psaux  = (PSAux_Service)face->psaux;


    /* skip whitespace */
    while ( is_space( *cur ) )
    {
      cur++;
      if ( cur >= limit )
      {
        FT_ERROR(( "parse_encoding: out of bounds!\n" ));
        parser->root.error = T42_Err_Invalid_File_Format;
        return;
      }
    }

    /* if we have a number, then the encoding is an array, */
    /* and we must load it now                             */
    if ( (FT_Byte)( *cur - '0' ) < 10 )
    {
      T1_Encoding  encode     = &face->type42.encoding;
      FT_Int       count, n;
      PS_Table     char_table = &loader->encoding_table;
      FT_Memory    memory     = parser->root.memory;
      FT_Error     error;


      /* read the number of entries in the encoding, should be 256 */
      count = T1_ToInt( parser );
      if ( parser->root.error )
        return;

      /* we use a T1_Table to store our charnames */
      loader->num_chars = encode->num_chars = count;
      if ( FT_NEW_ARRAY( encode->char_index, count ) ||
           FT_NEW_ARRAY( encode->char_name,  count ) ||
           FT_SET_ERROR( psaux->ps_table_funcs->init(
                           char_table, count, memory ) ) )
      {
        parser->root.error = error;
        return;
      }

      /* We need to `zero' out encoding_table.elements */
      for ( n = 0; n < count; n++ )
      {
        char*  notdef = (char *)".notdef";


        T1_Add_Table( char_table, n, notdef, 8 );
      }

      /* Now, we will need to read a record of the form         */
      /* ... charcode /charname ... for each entry in our table */
      /*                                                        */
      /* We simply look for a number followed by an immediate   */
      /* name.  Note that this ignores correctly the sequence   */
      /* that is often seen in type1 fonts:                     */
      /*                                                        */
      /*   0 1 255 { 1 index exch /.notdef put } for dup        */
      /*                                                        */
      /* used to clean the encoding array before anything else. */
      /*                                                        */
      /* We stop when we encounter a `def'.                     */

      cur   = parser->root.cursor;
      limit = parser->root.limit;
      n     = 0;

      for ( ; cur < limit; )
      {
        FT_Byte  c;


        c = *cur;

        /* we stop when we encounter a `def' */
        if ( c == 'd' && cur + 3 < limit )
        {
          if ( cur[1] == 'e'       &&
               cur[2] == 'f'       &&
               is_space( cur[-1] ) &&
               is_space( cur[3] )  )
          {
            FT_TRACE6(( "encoding end\n" ));
            break;
          }
        }

        /* otherwise, we must find a number before anything else */
        if ( (FT_Byte)( c - '0' ) < 10 )
        {
          FT_Int  charcode;


          parser->root.cursor = cur;
          charcode = T1_ToInt( parser );
          cur      = parser->root.cursor;

          /* skip whitespace */
          while ( cur < limit && is_space( *cur ) )
            cur++;

          if ( cur < limit && *cur == '/' )
          {
            /* bingo, we have an immediate name -- it must be a */
            /* character name                                   */
            FT_Byte*  cur2 = cur + 1;
            FT_Int    len;


            while ( cur2 < limit && is_alpha( *cur2 ) )
              cur2++;

            len = (FT_Int)( cur2 - cur - 1 );

            parser->root.error = T1_Add_Table( char_table, charcode,
                                               cur + 1, len + 1 );
            char_table->elements[charcode][len] = '\0';
            if ( parser->root.error )
              return;

            cur = cur2;
          }
        }
        else
          cur++;
      }

      face->type42.encoding_type = T1_ENCODING_TYPE_ARRAY;
      parser->root.cursor        = cur;
    }
    /* Otherwise, we should have either `StandardEncoding', */
    /* `ExpertEncoding', or `ISOLatin1Encoding'             */
    else
    {
      if ( cur + 17 < limit                                            &&
           ft_strncmp( (const char*)cur, "StandardEncoding", 16 ) == 0 )
        face->type42.encoding_type = T1_ENCODING_TYPE_STANDARD;

      else if ( cur + 15 < limit                                          &&
                ft_strncmp( (const char*)cur, "ExpertEncoding", 14 ) == 0 )
        face->type42.encoding_type = T1_ENCODING_TYPE_EXPERT;

      else if ( cur + 18 < limit                                             &&
                ft_strncmp( (const char*)cur, "ISOLatin1Encoding", 17 ) == 0 )
        face->type42.encoding_type = T1_ENCODING_TYPE_ISOLATIN1;

      else {
        FT_ERROR(( "parse_encoding: invalid token!\n" ));
        parser->root.error = T42_Err_Invalid_File_Format;
      }
    }
  }


  static FT_UInt
  hexval( FT_Byte  v )
  {
    FT_UInt  d;
    
    d = (FT_UInt)( v - 'A' );
    if ( d < 6 )
    {
      d += 10;
      goto Exit;
    }
      
    d = (FT_UInt)( v - 'a' );
    if ( d < 6 )
    {
      d += 10;
      goto Exit;
    }
      
    d = (FT_UInt)( v - '0' );
    if ( d < 10 )
      goto Exit;
      
    d = 0;
 
  Exit:         
    return d;
  }


  static void
  parse_sfnts( T42_Face    face,
               T42_Loader  loader )
  {
    T42_Parser  parser = &loader->parser;
    FT_Memory   memory = parser->root.memory;
    FT_Byte*    cur    = parser->root.cursor;
    FT_Byte*    limit  = parser->root.limit;
    FT_Error    error;
    FT_Int      num_tables = 0, status;
    FT_ULong    count, ttf_size = 0, string_size = 0;
    FT_Bool     in_string  = 0;
    FT_Byte     v = 0;


    /* The format is `/sfnts [ <...> <...> ... ] def' */

    while ( is_space( *cur ) )
      cur++;

    if (*cur++ == '[')
    {
      status = 0;
      count = 0;
    }
    else
    {
      FT_ERROR(( "parse_sfnts: can't find begin of sfnts vector!\n" ));
      error = T42_Err_Invalid_File_Format;
      goto Fail;
    }

    while ( cur < limit - 2 )
    {
      while ( is_space( *cur ) )
        cur++;

      switch ( *cur )
      {
      case ']':
        parser->root.cursor = cur++;
        return;

      case '<':
        in_string   = 1;
        string_size = 0;
        cur++;
        continue;

      case '>':
        if ( !in_string )
        {
          FT_ERROR(( "parse_sfnts: found unpaired `>'!\n" ));
          error = T42_Err_Invalid_File_Format;
          goto Fail;
        }

        /* A string can have, as a last byte,         */
        /* a zero byte for padding.  If so, ignore it */
        if ( ( v == 0 ) && ( string_size % 2 == 1 ) )
          count--;
        in_string = 0;
        cur++;
        continue;

      case '%':
        if ( !in_string )
        {
          /* Comment found; skip till end of line */
          while ( *cur != '\n' )
            cur++;
          continue;
        }
        else
        {
          FT_ERROR(( "parse_sfnts: found `%' in string!\n" ));
          error = T42_Err_Invalid_File_Format;
          goto Fail;
        }

      default:
        if ( !ft_xdigit( *cur ) || !ft_xdigit( *(cur + 1) ) )
        {
          FT_ERROR(( "parse_sfnts: found non-hex characters in string" ));
          error = T42_Err_Invalid_File_Format;
          goto Fail;
        }

        v = (FT_Byte)( 16 * hexval( *cur++ ) + hexval( *cur++ ) );
        string_size++;
      }

      switch ( status )
      {
      case 0: /* The '[' was read, so load offset table, 12 bytes */
        if ( count < 12 )
        {
          face->ttf_data[count++] = v;
          continue;
        }
        else
        {
          num_tables = 16 * face->ttf_data[4] + face->ttf_data[5];
          status     = 1;
          ttf_size   = 12 + 16 * num_tables;

          if ( FT_REALLOC( face->ttf_data, 12, ttf_size ) )
            goto Fail;
        }
        /* No break, fall-through */

      case 1: /* The offset table is read; read now the table directory */
        if ( count < ttf_size )
        {
          face->ttf_data[count++] = v;
          continue;
        }
        else
        {
          int      i;
          FT_ULong len;


          for ( i = 0; i < num_tables; i++ )
          {
            len  = face->ttf_data[12 + 16*i + 12 + 0] << 24;
            len += face->ttf_data[12 + 16*i + 12 + 1] << 16;
            len += face->ttf_data[12 + 16*i + 12 + 2] << 8;
            len += face->ttf_data[12 + 16*i + 12 + 3];

            /* Pad to a 4-byte boundary length */
            ttf_size += ( len + 3 ) & ~3;
          }

          status         = 2;
          face->ttf_size = ttf_size;

          if ( FT_REALLOC( face->ttf_data, 12 + 16 * num_tables,
                           ttf_size + 1 ) )
            goto Fail;
        }
        /* No break, fall-through */

      case 2: /* We are reading normal tables; just swallow them */
        face->ttf_data[count++] = v;

      }
    }

    /* If control reaches this point, the format was not valid */
    error = T42_Err_Invalid_File_Format;

  Fail:
    parser->root.error = error;
  }


  static void
  parse_charstrings( T42_Face    face,
                     T42_Loader  loader )
  {
    T42_Parser     parser     = &loader->parser;
    PS_Table       code_table = &loader->charstrings;
    PS_Table       name_table = &loader->glyph_names;
    FT_Memory      memory     = parser->root.memory;
    FT_Error       error;

    PSAux_Service  psaux      = (PSAux_Service)face->psaux;

    FT_Byte*       cur;
    FT_Byte*       limit      = parser->root.limit;
    FT_Int         n;


    loader->num_glyphs = T1_ToInt( parser );
    if ( parser->root.error )
      return;

    /* initialize tables */

    error = psaux->ps_table_funcs->init( code_table,
                                         loader->num_glyphs,
                                         memory );
    if ( error )
      goto Fail;

    error = psaux->ps_table_funcs->init( name_table,
                                         loader->num_glyphs,
                                         memory );
    if ( error )
      goto Fail;

    n = 0;

    for (;;)
    {
      /* the format is simple:                    */
      /*   `/glyphname' + index + def             */
      /*                                          */
      /* note that we stop when we find an `end'  */
      /*                                          */
      T1_Skip_Spaces( parser );

      cur = parser->root.cursor;
      if ( cur >= limit )
        break;

      /* we stop when we find an `end' keyword */
      if ( *cur   == 'e'   &&
           cur + 3 < limit &&
           cur[1] == 'n'   &&
           cur[2] == 'd'   )
        break;

      if ( *cur != '/' )
        T1_Skip_Alpha( parser );
      else
      {
        FT_Byte*  cur2 = cur + 1;
        FT_Int    len;


        while ( cur2 < limit && is_alpha( *cur2 ) )
          cur2++;
        len = (FT_Int)( cur2 - cur - 1 );

        error = T1_Add_Table( name_table, n, cur + 1, len + 1 );
        if ( error )
          goto Fail;

        /* add a trailing zero to the name table */
        name_table->elements[n][len] = '\0';

        parser->root.cursor = cur2;
        T1_Skip_Spaces( parser );

        cur2 = cur = parser->root.cursor;
        if ( cur >= limit )
          break;

        while ( cur2 < limit && is_alpha( *cur2 ) )
          cur2++;
        len = (FT_Int)( cur2 - cur );

        error = T1_Add_Table( code_table, n, cur, len + 1 );
        if ( error )
          goto Fail;

        code_table->elements[n][len] = '\0';

        n++;
        if ( n >= loader->num_glyphs )
          break;
      }
    }

    /* Index 0 must be a .notdef element */
    if ( ft_strcmp( (char *)name_table->elements[0], ".notdef" ) )
    {
      FT_ERROR(( "parse_charstrings: Index 0 is not `.notdef'!\n" ));
      error = T42_Err_Invalid_File_Format;
      goto Fail;
    }

    loader->num_glyphs = n;
    return;

  Fail:
    parser->root.error = error;
  }


  static FT_Error
  t42_load_keyword( T42_Face    face,
                    T42_Loader  loader,
                    T1_Field    field )
  {
    FT_Error  error;
    void*     dummy_object;
    void**    objects;
    FT_UInt   max_objects = 0;


    /* if the keyword has a dedicated callback, call it */
    if ( field->type == T1_FIELD_TYPE_CALLBACK ) {
      field->reader( (FT_Face)face, loader );
      error = loader->parser.root.error;
      goto Exit;
    }

    /* now, the keyword is either a simple field, or a table of fields; */
    /* we are now going to take care of it                              */
    switch ( field->location )
    {
    case T1_FIELD_LOCATION_FONT_INFO:
      dummy_object = &face->type42.font_info;
      objects      = &dummy_object;
      break;

    default:
      dummy_object = &face->type42;
      objects      = &dummy_object;
    }

    if ( field->type == T1_FIELD_TYPE_INTEGER_ARRAY ||
         field->type == T1_FIELD_TYPE_FIXED_ARRAY   )
      error = T1_Load_Field_Table( &loader->parser, field,
                                   objects, max_objects, 0 );
    else
      error = T1_Load_Field( &loader->parser, field,
                             objects, max_objects, 0 );

   Exit:
    return error;
  }


  static FT_Error
  parse_dict( T42_Face    face,
              T42_Loader  loader,
              FT_Byte*    base,
              FT_Long     size )
  {
    T42_Parser  parser = &loader->parser;
    FT_Byte*    cur    = base;
    FT_Byte*    limit  = cur + size;
    FT_UInt     n_keywords = sizeof ( t42_keywords ) / 
                             sizeof ( t42_keywords[0] );


    parser->root.cursor = base;
    parser->root.limit  = base + size;
    parser->root.error  = 0;

    for ( ; cur < limit; cur++ )
    {
      /* look for `FontDirectory', which causes problems on some fonts */
      if ( *cur == 'F' && cur + 25 < limit                    &&
           ft_strncmp( (char*)cur, "FontDirectory", 13 ) == 0 )
      {
        FT_Byte*  cur2;


        /* skip the `FontDirectory' keyword */
        cur += 13;
        cur2 = cur;

        /* lookup the `known' keyword */
        while ( cur < limit && *cur != 'k'           &&
                ft_strncmp( (char*)cur, "known", 5 ) )
          cur++;

        if ( cur < limit )
        {
          T1_TokenRec  token;


          /* skip the `known' keyword and the token following it */
          cur += 5;
          loader->parser.root.cursor = cur;
          T1_ToToken( &loader->parser, &token );

          /* if the last token was an array, skip it! */
          if ( token.type == T1_TOKEN_TYPE_ARRAY )
            cur2 = parser->root.cursor;
        }
        cur = cur2;
      }
      /* look for immediates */
      else if ( *cur == '/' && cur + 2 < limit )
      {
        FT_Byte*  cur2;
        FT_UInt    i, len;


        cur++;
        cur2 = cur;
        while ( cur2 < limit && is_alpha( *cur2 ) )
          cur2++;

        len  = (FT_UInt)( cur2 - cur );
        if ( len > 0 && len < 22 ) /* XXX What shall it this 22? */
        {
          /* now, compare the immediate name to the keyword table */

          /* Loop through all known keywords */
          for ( i = 0; i < n_keywords; i++ )
          {
            T1_Field  keyword = (T1_Field)&t42_keywords[i];
            FT_Byte   *name   = (FT_Byte*)keyword->ident;


            if ( !name )
              continue;

            if ( ( len == ft_strlen( (const char *)name ) ) &&
                 ( ft_memcmp( cur, name, len ) == 0 )       )
            {
              /* we found it -- run the parsing callback! */
              parser->root.cursor = cur2;
              T1_Skip_Spaces( parser );
              parser->root.error = t42_load_keyword(face,
                                                    loader,
                                                    keyword );
              if ( parser->root.error )
                return parser->root.error;
              cur = parser->root.cursor;
              break;
            }
          }
        }
      }
    }
    return parser->root.error;
  }


  static void
  t42_init_loader( T42_Loader  loader,
                   T42_Face    face )
  {
    FT_UNUSED( face );

    FT_MEM_SET( loader, 0, sizeof ( *loader ) );
    loader->num_glyphs = 0;
    loader->num_chars  = 0;

    /* initialize the tables -- simply set their `init' field to 0 */
    loader->encoding_table.init = 0;
    loader->charstrings.init    = 0;
    loader->glyph_names.init    = 0;
  }


  static void
  t42_done_loader( T42_Loader  loader )
  {
    T42_Parser  parser = &loader->parser;


    /* finalize tables */
    T1_Release_Table( &loader->encoding_table );
    T1_Release_Table( &loader->charstrings );
    T1_Release_Table( &loader->glyph_names );

    /* finalize parser */
    T42_Finalize_Parser( parser );
  }


  static FT_Error
  T42_Open_Face( T42_Face  face )
  {
    T42_LoaderRec  loader;
    T42_Parser     parser;
    T42_Font       type42 = &face->type42;
    FT_Memory      memory = face->root.memory;
    FT_Error       error;

    PSAux_Service  psaux  = (PSAux_Service)face->psaux;


    t42_init_loader( &loader, face );

    parser = &loader.parser;

    if ( FT_ALLOC( face->ttf_data, 12 ) )
      goto Exit;

    error = T42_New_Parser( parser,
                            face->root.stream,
                            memory,
                            psaux);
    if ( error )
      goto Exit;

    error = parse_dict( face, &loader, parser->base_dict, parser->base_len );

    if ( type42->font_type != 42 )
    {
      error = T42_Err_Unknown_File_Format;
      goto Exit;
    }

    /* now, propagate the charstrings and glyphnames tables */
    /* to the Type42 data                                   */
    type42->num_glyphs = loader.num_glyphs;

    if ( !loader.charstrings.init ) {
      FT_ERROR(( "T42_Open_Face: no charstrings array in face!\n" ));
      error = T42_Err_Invalid_File_Format;
    }

    loader.charstrings.init   = 0;
    type42->charstrings_block = loader.charstrings.block;
    type42->charstrings       = loader.charstrings.elements;
    type42->charstrings_len   = loader.charstrings.lengths;

    /* we copy the glyph names `block' and `elements' fields; */
    /* the `lengths' field must be released later             */
    type42->glyph_names_block   = loader.glyph_names.block;
    type42->glyph_names         = (FT_String**)loader.glyph_names.elements;
    loader.glyph_names.block    = 0;
    loader.glyph_names.elements = 0;

    /* we must now build type42.encoding when we have a custom array */
    if ( type42->encoding_type == T1_ENCODING_TYPE_ARRAY )
    {
      FT_Int    charcode, idx, min_char, max_char;
      FT_Byte*  char_name;
      FT_Byte*  glyph_name;


      /* OK, we do the following: for each element in the encoding   */
      /* table, look up the index of the glyph having the same name  */
      /* as defined in the CharStrings array.                        */
      /* The index is then stored in type42.encoding.char_index, and */
      /* the name in type42.encoding.char_name                       */

      min_char = +32000;
      max_char = -32000;

      charcode = 0;
      for ( ; charcode < loader.encoding_table.max_elems; charcode++ )
      {
        type42->encoding.char_index[charcode] = 0;
        type42->encoding.char_name [charcode] = (char *)".notdef";

        char_name = loader.encoding_table.elements[charcode];
        if ( char_name )
          for ( idx = 0; idx < type42->num_glyphs; idx++ )
          {
            glyph_name = (FT_Byte*)type42->glyph_names[idx];
            if ( ft_strcmp( (const char*)char_name,
                            (const char*)glyph_name ) == 0 )
            {
              type42->encoding.char_index[charcode] = (FT_UShort)idx;
              type42->encoding.char_name [charcode] = (char*)glyph_name;

              /* Change min/max encoded char only if glyph name is */
              /* not /.notdef                                      */
              if ( ft_strcmp( (const char*)".notdef",
                              (const char*)glyph_name ) != 0 )
              {
                if ( charcode < min_char ) min_char = charcode;
                if ( charcode > max_char ) max_char = charcode;
              }
              break;
            }
          }
      }
      type42->encoding.code_first = min_char;
      type42->encoding.code_last  = max_char;
      type42->encoding.num_chars  = loader.num_chars;
    }

  Exit:
    t42_done_loader( &loader );
    return error;
  }


  /***************** Driver Functions *************/


  /*************************************************************************/
  /*                                                                       */
  /* <Description>                                                         */
  /*    The face object constructor.                                       */
  /*                                                                       */
  /* <Input>                                                               */
  /*    stream     ::  input stream where to load font data.               */
  /*                                                                       */
  /*    face_index :: The index of the font face in the resource.          */
  /*                                                                       */
  /*    num_params :: Number of additional generic parameters.  Ignored.   */
  /*                                                                       */
  /*    params     :: Additional generic parameters.  Ignored.             */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    face       :: The face record to build.                            */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  static FT_Error
  T42_Face_Init( FT_Stream      stream,
                 T42_Face       face,
                 FT_Int         face_index,
                 FT_Int         num_params,
                 FT_Parameter*  params)
  {
    FT_Error          error;
    PSNames_Service   psnames;
    PSAux_Service     psaux;
    FT_Face           root    = (FT_Face)&face->root;
    FT_CharMap        charmap = face->charmaprecs;

    FT_UNUSED( num_params );
    FT_UNUSED( params );
    FT_UNUSED( face_index );
    FT_UNUSED( stream );


    face->ttf_face       = NULL;
    face->root.num_faces = 1;

    /* XXX */
    psnames = (PSNames_Service)face->psnames;
    if ( !psnames )
    {
      psnames = (PSNames_Service)
                FT_Get_Module_Interface( FT_FACE_LIBRARY( face ),
                                         "psnames" );
      face->psnames = psnames;
    }

    psaux = (PSAux_Service)face->psaux;
    if ( !psaux )
    {
      psaux = (PSAux_Service)
              FT_Get_Module_Interface( FT_FACE_LIBRARY( face ),
                                       "psaux" );
      face->psaux = psaux;
    }

    /* open the tokenizer, this will also check the font format */
    error = T42_Open_Face( face );
    if ( error )
      goto Exit;

    /* if we just wanted to check the format, leave successfully now */
    if ( face_index < 0 )
      goto Exit;

    /* check the face index */
    if ( face_index != 0 )
    {
      FT_ERROR(( "T42_Face_Init: invalid face index\n" ));
      error = T42_Err_Invalid_Argument;
      goto Exit;
    }

    /* Now, load the font program into the face object */

    /* Init the face object fields */
    /* Now set up root face fields */

    root->num_glyphs   = face->type42.num_glyphs;
    root->num_charmaps = 1;
    root->face_index  = face_index;

    root->face_flags  = FT_FACE_FLAG_SCALABLE;
    root->face_flags |= FT_FACE_FLAG_HORIZONTAL;
    root->face_flags |= FT_FACE_FLAG_GLYPH_NAMES;

    if ( face->type42.font_info.is_fixed_pitch )
      root->face_flags |= FT_FACE_FLAG_FIXED_WIDTH;

    /* XXX: TODO -- add kerning with .afm support */

    /* get style name -- be careful, some broken fonts only */
    /* have a `/FontName' dictionary entry!                 */
    root->family_name = face->type42.font_info.family_name;
    if ( root->family_name )
    {
      char*  full   = face->type42.font_info.full_name;
      char*  family = root->family_name;


      if ( full )
      {
        while ( *family && *full == *family )
        {
          family++;
          full++;
        }

        root->style_name = ( *full == ' ' ? full + 1
                                          : (char *)"Regular" );
      }
      else
        root->style_name = (char *)"Regular";
    }
    else
    {
      /* do we have a `/FontName'? */
      if ( face->type42.font_name )
      {
        root->family_name = face->type42.font_name;
        root->style_name  = (char *)"Regular";
      }
    }

    /* no embedded bitmap support */
    root->num_fixed_sizes = 0;
    root->available_sizes = 0;

    /* Load the TTF font embedded in the T42 font */
    error = FT_New_Memory_Face( FT_FACE_LIBRARY( face ),
                                face->ttf_data,
                                face->ttf_size,
                                0,
                                &face->ttf_face );
    if ( error )
      goto Exit;

    /* Ignore info in FontInfo dictionary and use the info from the  */
    /* loaded TTF font.  The PostScript interpreter also ignores it. */
    root->bbox         = face->ttf_face->bbox;
    root->units_per_EM = face->ttf_face->units_per_EM;

    root->ascender  = face->ttf_face->ascender;
    root->descender = face->ttf_face->descender;
    root->height    = face->ttf_face->height;

    root->max_advance_width = face->ttf_face->max_advance_width;
    root->max_advance_height = face->ttf_face->max_advance_height;

    root->underline_position  = face->type42.font_info.underline_position;
    root->underline_thickness = face->type42.font_info.underline_thickness;

    root->internal->max_points   = 0;
    root->internal->max_contours = 0;

    /* compute style flags */
    root->style_flags = 0;
    if ( face->type42.font_info.italic_angle )
      root->style_flags |= FT_STYLE_FLAG_ITALIC;

    if ( face->ttf_face->style_flags & FT_STYLE_FLAG_BOLD )
      root->style_flags |= FT_STYLE_FLAG_BOLD;

    if ( face->ttf_face->face_flags & FT_FACE_FLAG_VERTICAL )
      root->face_flags |= FT_FACE_FLAG_VERTICAL;

    /* XXX: Add support for new cmaps code in FT 2.1.0 */

    /* charmap support -- synthetize unicode charmap if possible */

    /* synthesize a Unicode charmap if there is support in the `PSNames' */
    /* module                                                            */
    if ( psnames && psnames->unicode_value )
    {
      error = psnames->build_unicodes( root->memory,
                                       face->type42.num_glyphs,
                                       (const char**)face->type42.glyph_names,
                                       &face->unicode_map );
      if ( !error )
      {
        root->charmap        = charmap;
        charmap->face        = (FT_Face)face;
        charmap->encoding    = ft_encoding_unicode;
        charmap->platform_id = 3;
        charmap->encoding_id = 1;
        charmap++;
      }

      /* XXX: Is the following code correct?  It is used in t1objs.c */

      /* simply clear the error in case of failure (which really) */
      /* means that out of memory or no unicode glyph names       */
      error = T42_Err_Ok;
    }

    /* now, support either the standard, expert, or custom encoding */
    charmap->face        = (FT_Face)face;
    charmap->platform_id = 7;  /* a new platform id for Adobe fonts? */

    switch ( face->type42.encoding_type )
    {
    case T1_ENCODING_TYPE_STANDARD:
      charmap->encoding    = ft_encoding_adobe_standard;
      charmap->encoding_id = 0;
      break;

    case T1_ENCODING_TYPE_EXPERT:
      charmap->encoding    = ft_encoding_adobe_expert;
      charmap->encoding_id = 1;
      break;

    case T1_ENCODING_TYPE_ARRAY:
      charmap->encoding    = ft_encoding_adobe_custom;
      charmap->encoding_id = 2;
      break;

    case T1_ENCODING_TYPE_ISOLATIN1:
      charmap->encoding    = ft_encoding_latin_1;
      charmap->encoding_id = 3;
      break;

    default:
      FT_ERROR(( "T42_Face_Init: invalid encoding\n" ));
      error = T42_Err_Invalid_File_Format;
      goto Exit;
    }

    root->charmaps     = face->charmaps;
    root->num_charmaps = charmap - face->charmaprecs + 1;
    face->charmaps[0]  = &face->charmaprecs[0];
    face->charmaps[1]  = &face->charmaprecs[1];

  Exit:
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*   T42_Face_Done                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The face object destructor.                                        */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face :: A typeless pointer to the face object to destroy.          */
  /*                                                                       */
  static void
  T42_Face_Done( T42_Face  face )
  {
    T42_Font     type42;
    PS_FontInfo  info;
    FT_Memory    memory;


    if ( face )
    {
      type42 = &face->type42;
      info   = &type42->font_info;
      memory = face->root.memory;

      /* delete internal ttf face prior to freeing face->ttf_data */
      if ( face->ttf_face )
        FT_Done_Face( face->ttf_face );

      /* release font info strings */
      FT_FREE( info->version );
      FT_FREE( info->notice );
      FT_FREE( info->full_name );
      FT_FREE( info->family_name );
      FT_FREE( info->weight );

      /* release top dictionary */
      FT_FREE( type42->charstrings_len );
      FT_FREE( type42->charstrings );
      FT_FREE( type42->glyph_names );

      FT_FREE( type42->charstrings_block );
      FT_FREE( type42->glyph_names_block );

      FT_FREE( type42->encoding.char_index );
      FT_FREE( type42->encoding.char_name );
      FT_FREE( type42->font_name );

      FT_FREE( face->ttf_data );

#if 0
      /* release afm data if present */
      if ( face->afm_data )
        T1_Done_AFM( memory, (T1_AFM*)face->afm_data );
#endif

      /* release unicode map, if any */
      FT_FREE( face->unicode_map.maps );
      face->unicode_map.num_maps = 0;

      face->root.family_name = 0;
      face->root.style_name  = 0;
    }
  }

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    T42_Driver_Init                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Initializes a given Type 42 driver object.                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    driver :: A handle to the target driver object.                    */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  static FT_Error
  T42_Driver_Init( T42_Driver  driver )
  {
    FT_Module  ttmodule;


    ttmodule = FT_Get_Module( driver->root.root.library, "truetype" );
    driver->ttclazz = (FT_Driver_Class)ttmodule->clazz;

    return T42_Err_Ok;
  }


  static void
  T42_Driver_Done( T42_Driver  driver )
  {
    FT_UNUSED( driver );
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Get_Char_Index                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Uses a charmap to return a given character code's glyph index.     */
  /*                                                                       */
  /* <Input>                                                               */
  /*    charmap  :: A handle to the source charmap object.                 */
  /*                                                                       */
  /*    charcode :: The character code.                                    */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Glyph index.  0 means `undefined character code'.                  */
  /*                                                                       */
  static FT_UInt
  Get_Char_Index( FT_CharMap  charmap,
                  FT_Long     charcode )
  {
    T42_Face         face;
    FT_UInt          result = 0;
    PSNames_Service  psnames;


    face    = (T42_Face)charmap->face;
    psnames = (PSNames_Service)face->psnames;
    if (!psnames )
      goto Exit;

    switch ( charmap->encoding )
    {
      /*******************************************************************/
      /*                                                                 */
      /* Unicode encoding support                                        */
      /*                                                                 */
    case ft_encoding_unicode:
      /* if this charmap is used, we ignore the encoding of the font and */
      /* use the `PSNames' module to synthetize the Unicode charmap      */
      result = psnames->lookup_unicode( &face->unicode_map,
                                        (FT_ULong)charcode );

      /* the function returns 0xFFFF if the Unicode charcode has */
      /* no corresponding glyph                                  */
      if ( result == 0xFFFFU )
        result = 0;

      /* The result returned is the index (position)in the CharStrings */
      /* array.  This must be used now to get the value associated to  */
      /* that glyph_name, which is the real index within the truetype  */
      /* structure.                                                    */
      result = ft_atoi( (const char*)face->type42.charstrings[result] );
      goto Exit;

      /*******************************************************************/
      /*                                                                 */
      /* ISOLatin1 encoding support                                      */
      /*                                                                 */
    case ft_encoding_latin_1:
      /* ISOLatin1 is the first page of Unicode */
      if ( charcode < 256 && psnames->unicode_value )
      {
        result = psnames->lookup_unicode( &face->unicode_map,
                                          (FT_ULong)charcode );

        /* the function returns 0xFFFF if the Unicode charcode has */
        /* no corresponding glyph                                  */
        if ( result == 0xFFFFU )
          result = 0;
      }
      goto Exit;

      /*******************************************************************/
      /*                                                                 */
      /* Custom Type 1 encoding                                          */
      /*                                                                 */
    case ft_encoding_adobe_custom:
      {
        T1_Encoding  encoding = &face->type42.encoding;


        if ( charcode >= encoding->code_first &&
             charcode <= encoding->code_last  )
        {
          FT_UInt idx = encoding->char_index[charcode];


          result = ft_atoi( (const char *)face->type42.charstrings[idx] );
        }
        goto Exit;
      }

      /*******************************************************************/
      /*                                                                 */
      /* Adobe Standard & Expert encoding support                        */
      /*                                                                 */
    default:
      if ( charcode < 256 )
      {
        FT_UInt      code;
        FT_Int       n;
        const char*  glyph_name;


        code = psnames->adobe_std_encoding[charcode];
        if ( charmap->encoding == ft_encoding_adobe_expert )
          code = psnames->adobe_expert_encoding[charcode];

        glyph_name = psnames->adobe_std_strings( code );
        if ( !glyph_name )
          break;

        for ( n = 0; n < face->type42.num_glyphs; n++ )
        {
          const char*  gname = face->type42.glyph_names[n];

          if ( gname && ( ft_strcmp( gname, glyph_name ) == 0 ) )
          {
            result = ft_atoi( (const char *)face->type42.charstrings[n] );
            break;
          }
        }
      }
    }

  Exit:
    return result;
  }


  static FT_Error
  T42_Size_Init( T42_Size  size )
  {
    FT_Face   face    = size->root.face;
    T42_Face  t42face = (T42_Face)face;
    FT_Size   ttsize;
    FT_Error  error   = T42_Err_Ok;


    if ( face->size == NULL )
    {
      /* First size for this face */
      size->ttsize = t42face->ttf_face->size;
    }
    else
    {
      error = FT_New_Size( t42face->ttf_face, &ttsize );
      size->ttsize = ttsize;
    }

    return error;
  }


  static void
  T42_Size_Done( T42_Size  size )
  {
    FT_Face      face    = size->root.face;
    T42_Face     t42face = (T42_Face)face;
    FT_ListNode  node;


    node = FT_List_Find( &t42face->ttf_face->sizes_list, size->ttsize );
    if ( node )
      FT_Done_Size( size->ttsize );
  }


  static FT_Error
  T42_GlyphSlot_Init( T42_GlyphSlot  slot )
  {
    FT_Face       face    = slot->root.face;
    T42_Face      t42face = (T42_Face)face;
    FT_GlyphSlot  ttslot;
    FT_Error      error   = T42_Err_Ok;


    if ( face->glyph == NULL )
    {
      /* First glyph slot for this face */
      slot->ttslot = t42face->ttf_face->glyph;
    }
    else
    {
      error = FT_New_GlyphSlot( t42face->ttf_face, &ttslot );
      slot->ttslot = ttslot;
    }

    return error;
  }


  static void
  T42_GlyphSlot_Done( T42_GlyphSlot slot )
  {
    FT_Face       face    = slot->root.face;
    T42_Face      t42face = (T42_Face)face;
    FT_GlyphSlot  cur     = t42face->ttf_face->glyph;


    while ( cur )
    {
      if ( cur == slot->ttslot )
      {
        FT_Done_GlyphSlot( slot->ttslot );
        break;
      }

      cur = cur->next;
    }
  }


  static FT_Error
  T42_Char_Size( T42_Size    size,
                 FT_F26Dot6  char_width,
                 FT_F26Dot6  char_height,
                 FT_UInt     horz_resolution,
                 FT_UInt     vert_resolution )
  {
    FT_Face   face    = size->root.face;
    T42_Face  t42face = (T42_Face)face;


    return FT_Set_Char_Size( t42face->ttf_face,
                             char_width,
                             char_height,
                             horz_resolution,
                             vert_resolution );
  }


  static FT_Error
  T42_Pixel_Size( T42_Size  size,
                  FT_UInt   pixel_width,
                  FT_UInt   pixel_height )
  {
    FT_Face   face    = size->root.face;
    T42_Face  t42face = (T42_Face)face;


    return FT_Set_Pixel_Sizes( t42face->ttf_face,
                               pixel_width,
                               pixel_height );
  }


  static void
  ft_glyphslot_clear( FT_GlyphSlot  slot )
  {
    /* free bitmap if needed */
    if ( slot->flags & FT_GLYPH_OWN_BITMAP )
    {
      FT_Memory  memory = FT_FACE_MEMORY( slot->face );


      FT_FREE( slot->bitmap.buffer );
      slot->flags &= ~FT_GLYPH_OWN_BITMAP;
    }

    /* clear all public fields in the glyph slot */
    FT_MEM_SET( &slot->metrics, 0, sizeof ( slot->metrics ) );
    FT_MEM_SET( &slot->outline, 0, sizeof ( slot->outline ) );
    FT_MEM_SET( &slot->bitmap,  0, sizeof ( slot->bitmap )  );

    slot->bitmap_left   = 0;
    slot->bitmap_top    = 0;
    slot->num_subglyphs = 0;
    slot->subglyphs     = 0;
    slot->control_data  = 0;
    slot->control_len   = 0;
    slot->other         = 0;
    slot->format        = ft_glyph_format_none;

    slot->linearHoriAdvance = 0;
    slot->linearVertAdvance = 0;
  }


  static FT_Error
  T42_Load_Glyph( FT_GlyphSlot  glyph,
                  FT_Size       size,
                  FT_Int        glyph_index,
                  FT_Int        load_flags )
  {
    FT_Error         error;
    T42_GlyphSlot    t42slot = (T42_GlyphSlot)glyph;
    T42_Size         t42size = (T42_Size)size;
    FT_Driver_Class  ttclazz = ((T42_Driver)glyph->face->driver)->ttclazz;


    ft_glyphslot_clear( t42slot->ttslot );
    error = ttclazz->load_glyph( t42slot->ttslot,
                                 t42size->ttsize,
                                 glyph_index,
                                 load_flags | FT_LOAD_NO_BITMAP );

    if ( !error )
    {
      glyph->metrics = t42slot->ttslot->metrics;

      glyph->linearHoriAdvance = t42slot->ttslot->linearHoriAdvance;
      glyph->linearVertAdvance = t42slot->ttslot->linearVertAdvance;

      glyph->format  = t42slot->ttslot->format;
      glyph->outline = t42slot->ttslot->outline;

      glyph->bitmap      = t42slot->ttslot->bitmap;
      glyph->bitmap_left = t42slot->ttslot->bitmap_left;
      glyph->bitmap_top  = t42slot->ttslot->bitmap_top;
    }

    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Get_Next_Char                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Uses a charmap to return the next encoded char.                    */
  /*                                                                       */
  /* <Input>                                                               */
  /*    charmap  :: A handle to the source charmap object.                 */
  /*                                                                       */
  /*    charcode :: The character code.                                    */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Next char code.  0 means `no more char codes'.                     */
  /*                                                                       */
  static FT_Long
  Get_Next_Char( FT_CharMap  charmap,
                 FT_Long     charcode )
  {
    T42_Face         face;
    PSNames_Service  psnames;


    face    = (T42_Face)charmap->face;
    psnames = (PSNames_Service)face->psnames;

    if ( psnames )
      switch ( charmap->encoding )
      {
        /*******************************************************************/
        /*                                                                 */
        /* Unicode encoding support                                        */
        /*                                                                 */
      case ft_encoding_unicode:
        /* use the `PSNames' module to synthetize the Unicode charmap */
        return psnames->next_unicode( &face->unicode_map,
                                      (FT_ULong)charcode );

        /*******************************************************************/
        /*                                                                 */
        /* ISOLatin1 encoding support                                      */
        /*                                                                 */
      case ft_encoding_latin_1:
        {
          FT_ULong code;


          /* use the `PSNames' module to synthetize the Unicode charmap */
          code = psnames->next_unicode( &face->unicode_map,
                                        (FT_ULong)charcode );
          if ( code < 256 )
            return code;
          break;
        }

        /*******************************************************************/
        /*                                                                 */
        /* Custom Type 1 encoding                                          */
        /*                                                                 */
      case ft_encoding_adobe_custom:
        {
          T1_Encoding  encoding = &face->type42.encoding;


          charcode++;
          if ( charcode < encoding->code_first )
            charcode = encoding->code_first;
          while ( charcode <= encoding->code_last  ) {
            if ( encoding->char_index[charcode] )
              return charcode;
            charcode++;
          }
        }


        /*******************************************************************/
        /*                                                                 */
        /* Adobe Standard & Expert encoding support                        */
        /*                                                                 */
      default:
        while ( ++charcode < 256 )
        {
          FT_UInt      code;
          FT_Int       n;
          const char*  glyph_name;


          code = psnames->adobe_std_encoding[charcode];
          if ( charmap->encoding == ft_encoding_adobe_expert )
            code = psnames->adobe_expert_encoding[charcode];

          glyph_name = psnames->adobe_std_strings( code );
          if ( !glyph_name )
            continue;

          for ( n = 0; n < face->type42.num_glyphs; n++ )
          {
            const char*  gname = face->type42.glyph_names[n];


            if ( gname && gname[0] == glyph_name[0]  &&
                 ft_strcmp( gname, glyph_name ) == 0 )
              return charcode;
          }
        }
      }

    return 0;
  }


  static FT_Error
  t42_get_glyph_name( T42_Face    face,
                      FT_UInt     glyph_index,
                      FT_Pointer  buffer,
                      FT_UInt     buffer_max )
  {
    FT_String*  gname;


    gname = face->type42.glyph_names[glyph_index];

    if ( buffer_max > 0 )
    {
      FT_UInt  len = (FT_UInt)( ft_strlen( gname ) );

  
      if ( len >= buffer_max )
        len = buffer_max - 1;
      FT_MEM_COPY( buffer, gname, len );
      ((FT_Byte*)buffer)[len] = 0;
    }

    return T42_Err_Ok;
  }


  static const char*
  t42_get_ps_name( T42_Face  face )
  {
    return (const char*)face->type42.font_name;
  }


  static FT_UInt
  t42_get_name_index( T42_Face    face,
                      FT_String*  glyph_name )
  {
    FT_Int      i;
    FT_String*  gname;


    for ( i = 0; i < face->type42.num_glyphs; i++ )
    {
      gname = face->type42.glyph_names[i];

      if ( !ft_strcmp( glyph_name, gname ) )
        return ft_atoi( (const char *)face->type42.charstrings[i] );
    }

    return 0;
  }


  static FT_Module_Interface
  Get_Interface( FT_Driver         driver,
                 const FT_String*  t42_interface )
  {
    FT_UNUSED( driver );

    /* Any additional interface are defined here */
    if (ft_strcmp( (const char*)t42_interface, "glyph_name" ) == 0 )
      return (FT_Module_Interface)t42_get_glyph_name;

    if ( ft_strcmp( (const char*)t42_interface, "name_index" ) == 0 )
      return (FT_Module_Interface)t42_get_name_index;

    if ( ft_strcmp( (const char*)t42_interface, "postscript_name" ) == 0 )
      return (FT_Module_Interface)t42_get_ps_name;

    return 0;
  }


  const FT_Driver_ClassRec  t42_driver_class =
  {
    {
      ft_module_font_driver      |
      ft_module_driver_scalable  |
#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER
      ft_module_driver_has_hinter,
#else
      0,
#endif

      sizeof ( T42_DriverRec ),

      "type42",
      0x10000L,
      0x20000L,

      0,    /* format interface */

      (FT_Module_Constructor)T42_Driver_Init,
      (FT_Module_Destructor) T42_Driver_Done,
      (FT_Module_Requester)  Get_Interface,
    },

    sizeof ( T42_FaceRec ),
    sizeof ( T42_SizeRec ),
    sizeof ( T42_GlyphSlotRec ),

    (FT_Face_InitFunc)        T42_Face_Init,
    (FT_Face_DoneFunc)        T42_Face_Done,
    (FT_Size_InitFunc)        T42_Size_Init,
    (FT_Size_DoneFunc)        T42_Size_Done,
    (FT_Slot_InitFunc)        T42_GlyphSlot_Init,
    (FT_Slot_DoneFunc)        T42_GlyphSlot_Done,

    (FT_Size_ResetPointsFunc) T42_Char_Size,
    (FT_Size_ResetPixelsFunc) T42_Pixel_Size,
    (FT_Slot_LoadFunc)        T42_Load_Glyph,
    (FT_CharMap_CharIndexFunc)Get_Char_Index,

    (FT_Face_GetKerningFunc)  0,
    (FT_Face_AttachFunc)      0,

    (FT_Face_GetAdvancesFunc) 0,

    (FT_CharMap_CharNextFunc) Get_Next_Char
  };


/* END */
