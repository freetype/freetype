/*******************************************************************
 *
 *  t1load.h                                                    2.0
 *
 *    Type1 Loader.                          
 *
 *  Copyright 1996-2000 by
 *  David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 *  This file is part of the FreeType project, and may only be used
 *  modified and distributed under the terms of the FreeType project
 *  license, LICENSE.TXT.  By continuing to use, modify, or distribute
 *  this file you indicate that you have read the license and
 *  understand and accept it fully.
 *
 *
 *  This is the new and improved Type 1 data loader for FreeType 2.
 *  The old loader has several problems: it is slow, complex, difficult
 *  to maintain, and contains incredible hacks to make it accept some
 *  ill-formed Type 1 fonts without hiccup-ing. Moreover, about 5%
 *  of the Type 1 fonts on my machine still aren't loaded correctly
 *  by it.
 *
 *  This version is much simpler, much faster and also easier to
 *  read and maintain by a great order of magnitude. The idea behind
 *  it is to _not_ try to read the Type 1 token stream with a state
 *  machine (i.e. a Postscript-like interpreter) but rather to perform
 *  simple pattern-matching.
 *
 *  Indeed, nearly all data definitions follow a simple pattern
 *  like :
 *
 *      ..... /Field <data> ....
 *
 *  where <data> can be a number, a boolean, a string, or an 
 *  array of numbers. There are a few exceptions, namely the
 *  encoding, font name, charstrings and subrs and they are
 *  handled with a special pattern-matching routine.
 *
 *  All other common cases are handled very simply. The matching
 *  rules are defined in the file "t1tokens.h" through the use
 *  of several macros calls PARSE_XXXX
 *
 *  This file is included twice here, the first time to generate
 *  parsing callback functions, the second to generate a table
 *  of keywords (with pointers to the associated callback).
 *
 *  The function "parse_dict" simply scans *linearly* a given
 *  dictionary (either the top-level or private one) and calls
 *  the appropriate callback when it encounters an immediate
 *  keyword.
 *
 *  This is by far the fastest way one can find to parse and read
 *  all data :-)
 *
 *  This led to tremendous code size reduction. Note that later,
 *  the glyph loader will also be _greatly_ simplified, and the
 *  automatic hinter will replace the clumsy "t1hinter"..
 *
 ******************************************************************/

#include <ftdebug.h>
#include <ftconfig.h>

#include <t1types.h>
#include <t1errors.h>
#include <t1load.h>
#include <stdio.h>

#undef  FT_COMPONENT
#define FT_COMPONENT  trace_t1load

  typedef  void  (*T1_Parse_Func)( T1_Face   face, T1_Loader*  loader );

  typedef  struct T1_KeyWord_  
  {
    const char*    name;
    T1_Parse_Func  parsing;
  
  } T1_KeyWord;

  /* some handy macros used to easily define parsing callback functions */
  /* each callback is in charge of loading a value and storing it in a  */
  /* given field of the Type 1 face..                                   */
 
#define PARSE_(x)  static void parse_##x ( T1_Face  face, T1_Loader* loader )

#define PARSE_STRING(s,x)   PARSE_(x)                               \
        {                                                           \
          FACE.##x = T1_ToString(&loader->parser);                  \
          FT_TRACE2(( "type1.parse_##x##: \"%s\"\n", FACE.##x ));   \
        }

#define PARSE_NUM(s,x,t)  PARSE_(x)                                 \
        {                                                           \
          FACE.##x = (t)T1_ToInt(&loader->parser);                  \
          FT_TRACE2(( "type1.parse_##x##: \"%d\"\n", FACE.##x ));   \
        }
        
#define PARSE_INT(s,x)   PARSE_(x)                                  \
        {                                                           \
          FACE.##x = T1_ToInt(&loader->parser);                     \
          FT_TRACE2(( "type1.parse_##x##: \"%d\"\n", FACE.##x ));   \
        }

#define PARSE_BOOL(s,x)   PARSE_(x)                                 \
        {                                                           \
          FACE.##x = T1_ToBool(&loader->parser);                    \
          FT_TRACE2(( "type1.parse_##x##: \"%s\"\n",                \
                       FACE.##x ? "true" : "false" ));              \
        }
 
#define PARSE_FIXED(s,x)   PARSE_(x)                                        \
        {                                                                   \
          FACE.##x = T1_ToFixed(&loader->parser,3);                         \
          FT_TRACE2(( "type1.parse_##x##: \"%f\"\n", FACE.##x/65536.0 ));   \
        }
 
#define PARSE_COORDS(s,c,m,x)   PARSE_(x)                                         \
        {                                                                         \
          FACE.##c = T1_ToCoordArray(&loader->parser, m, (T1_Short*)FACE.##x );   \
          FT_TRACE2(( "type1.parse_##x##\n" ));                                   \
        }
           
#define PARSE_FIXEDS(s,c,m,x)   PARSE_(x)                                           \
        {                                                                           \
          FACE.##c = T1_ToFixedArray(&loader->parser, m, (T1_Fixed*)FACE.##x, 3 );  \
          FT_TRACE2(( "type1.parse_##x##\n" ));                                     \
        }


#define PARSE_COORDS2(s,m,x)   PARSE_(x)                                     \
        {                                                                    \
          (void)T1_ToCoordArray( &loader->parser, m, (T1_Short*)&FACE.##x );  \
          FT_TRACE2(( "type1.parse_##x##\n" ));                              \
        }

#define PARSE_FIXEDS2(s,m,x)   PARSE_(x)                                           \
        {                                                                           \
          (void)T1_ToFixedArray(&loader->parser, m, (T1_Fixed*)&FACE.##x, 3 );  \
          FT_TRACE2(( "type1.parse_##x##\n" ));                                     \
        }


/* define all parsing callbacks */
#include <t1tokens.h>


  static
  int  is_space( char c )
  {
    return ( c == ' ' || c == '\t' || c == '\r' || c == '\n' );
  }

  static
  int  is_alpha( char c )
  {
    return ( (c >= 'A' && c <= 'Z') ||
             (c >= 'a' && c <= 'z') ||
             (c >= '0' && c <= '9') ||
             (c == '.')             ||
             (c == '_') );
  }

  static
  void  skip_whitespace( T1_Parser* parser )
  {
    T1_Byte*  cur = parser->cursor;
    
    while ( cur < parser->limit && is_space(*cur) )
      cur++;
      
    parser->cursor = cur;
  }
  
  static
  void skip_blackspace( T1_Parser* parser )
  {
    T1_Byte*  cur = parser->cursor;
    
    while ( cur < parser->limit && !is_space(*cur) )
      cur++;
      
    parser->cursor = cur;
  }
  
  static
  int   read_binary_data( T1_Parser*  parser, T1_Int *size, T1_Byte* *base )
  {
    T1_Byte*  cur;
    T1_Byte*  limit = parser->limit;
    
    /* the binary data has the following format */
    /*                                          */
    /* "size" [white*] RD white ....... ND      */
    /*                                          */
    
    skip_whitespace(parser);
    cur = parser->cursor;
    
    if ( cur < limit && (T1_Byte)(*cur-'0') < 10 )
    {
      *size = T1_ToInt(parser);
      
      skip_whitespace(parser);
      skip_blackspace(parser);  /* "RD" or "-|" or something else */
      
      /* there is only one whitespace char after the */
      /* "RD" or "-|" token                          */
      *base = parser->cursor + 1;
      
      parser->cursor += *size+1;
      return 1;
    }

    FT_ERROR(( "type1.read_binary_data: invalid size field\n" ));
    parser->error = FT_Err_Invalid_File_Format;
    return 0;
  }


  /* we will now define the routines used to handle */
  /* the /Encoding, /Subrs and /CharStrings         */
  /* dictionaries..                                 */

  static
  void  parse_font_name( T1_Face  face, T1_Loader*  loader )
  {
    T1_Parser*  parser = &loader->parser;
    FT_Error    error;
    FT_Memory   memory = parser->memory;
    T1_Int      len;
    T1_Byte*    cur;
    T1_Byte*    cur2;
    T1_Byte*    limit;
    
    skip_whitespace(parser);
    cur   = parser->cursor;
    limit = parser->limit;
    if ( cur >= limit-1 || *cur != '/' ) return;
    
    cur++;
    cur2 = cur;
    while (cur2 < limit && is_alpha(*cur2)) cur2++;
    len = cur2-cur;
    if (len > 0)
    {
      if ( ALLOC( face->type1.font_name, len+1 ) )
      {
        parser->error = error;
        return;
      }
      
      MEM_Copy( face->type1.font_name, cur, len );
      face->type1.font_name[len] = '\0';
    }
    parser->cursor = cur2;
  }

  static
  void  parse_font_bbox( T1_Face  face, T1_Loader*  loader )
  {
    T1_Parser*  parser = &loader->parser;
    T1_Short    temp[4];
    T1_BBox*    bbox = &face->type1.font_bbox;
    
    (void)T1_ToCoordArray( parser, 4, temp );
    bbox->xMin = temp[0];
    bbox->yMin = temp[1];
    bbox->xMax = temp[2];
    bbox->yMax = temp[3];
  }

  static
  void  parse_encoding( T1_Face  face, T1_Loader*  loader )
  {
    T1_Parser*  parser = &loader->parser;
    T1_Byte*    cur   = parser->cursor;
    T1_Byte*    limit = parser->limit;
    
    /* skip whitespace */
    while (is_space(*cur))
    {
      cur++;
      if (cur >= limit)
      {
        FT_ERROR(( "type1.parse_encoding: out of bounds !!\n" ));
        parser->error = FT_Err_Invalid_File_Format;
        return;
      }
    }
    
    /* if we have a number, then the encoding is an array, */
    /* and we must load it now                             */
    if ((T1_Byte)(*cur - '0') < 10)
    {
      T1_Encoding*  encode     = &face->type1.encoding;
      T1_Int        count, n;
      T1_Table*     char_table = &loader->encoding_table;
      FT_Memory     memory     = parser->memory;
      FT_Error      error;
      
      /* read the number of entries in the encoding, should be 256 */
      count = T1_ToInt( parser );
      if (parser->error) return;

      /* we use a T1_Table to store our charnames */
      encode->num_chars = count;
      if ( ALLOC_ARRAY( encode->char_index, count, T1_Short   ) ||
           ALLOC_ARRAY( encode->char_name,  count, T1_String* ) ||
           (error = T1_New_Table( char_table, count, memory )) != 0     )
      {
        parser->error = error;
        return;
      }
      
      /* now, we will need to read a record of the form         */
      /* ... charcode /charname ... for each entry in our table */
      /*                                                        */
      /* we simply look for a number followed by an immediate   */
      /* name. Note that this ignores correctly the sequence    */
      /* that is often seen in type1 fonts :                    */
      /*                                                        */
      /*   0 1 255 { 1 index exch /.notdef put } for dup        */
      /*                                                        */
      /* used to clean the encoding array before anything else  */
      /*                                                        */
      /* we stop when we encounter a "def"                      */
      /*                                                        */

      cur   = parser->cursor;
      limit = parser->limit;
      n     = 0;

      for ( ; cur < limit; )
      {
        T1_Byte  c;
        
        c = *cur;
        
        /* we stop when we encounter a 'def' */
        if ( c == 'd' && cur+3 < limit )
        {
          if ( cur[1] == 'e' &&
               cur[2] == 'f' &&
               is_space(cur[-1]) &&
               is_space(cur[3]) )
          {
              FT_TRACE6(( "encoding end\n" ));
              break;
          }
        }
        
        /* otherwise, we must find a number before anything else */
        if ( (T1_Byte)(c-'0') < 10 )
        {
          T1_Int  charcode;
          
          parser->cursor = cur;
          charcode = T1_ToInt(parser);
          cur = parser->cursor;
          
          /* skip whitespace */
          while (cur < limit && is_space(*cur)) cur++;
          
          if (cur < limit && *cur == '/')
          {
            /* bingo, we have an immediate name - it must be a */
            /* character name                                  */
            FT_Byte*  cur2 = cur+1;
            T1_Int    len;
            
            while (cur2 < limit && is_alpha(*cur2)) cur2++;
            len = cur2-cur-1;
            parser->error = T1_Add_Table( char_table, charcode, cur+1, len+1 );
            char_table->elements[charcode][len] = '\0';
            if (parser->error) return;
            
            cur = cur2;
          }
        }
        else
          cur++;
      }
      
      face->type1.encoding_type = t1_encoding_array;
      parser->cursor            = cur;
    }
    /* Otherwise, we should have either "StandardEncoding" or */
    /* "ExpertEncoding"                                       */
    else
    {
      if ( cur+17 < limit &&
           strncmp( (const char*)cur, "StandardEncoding", 16 ) == 0 )
        face->type1.encoding_type = t1_encoding_standard;
        
      else if ( cur+15 < limit &&
                strncmp( (const char*)cur, "ExpertEncoding", 14 ) == 0 )
        face->type1.encoding_type = t1_encoding_expert;

      else
      {
        FT_ERROR(( "type1.parse_encoding: invalid token !!\n" ));
        parser->error = FT_Err_Invalid_File_Format;
      }      
    }
  }


  static
  void  parse_subrs( T1_Face  face, T1_Loader*  loader )
  {
    T1_Parser*  parser = &loader->parser;
    T1_Table*   table  = &loader->subrs;
    FT_Memory   memory = parser->memory;
    FT_Error    error;
    T1_Int      n;
    
    loader->num_subrs = T1_ToInt( parser );    
    if (parser->error) return;
    
    /* initialise subrs array */
    error = T1_New_Table( table, loader->num_subrs, memory );
    if (error) goto Fail;

    /* the format is simple :                                */
    /*                                                       */
    /*   "index" + binary data                               */
    /*                                                       */
    
    for ( n = 0; n < loader->num_subrs; n++ )
    {
      T1_Int    index, size;
      T1_Byte*  base;
      
      index = T1_ToInt(parser);
      if (!read_binary_data(parser,&size,&base)) return;
      
      T1_Decrypt( base, size, 4330 );      
      size -= face->type1.private_dict.lenIV;
      base += face->type1.private_dict.lenIV;

      error = T1_Add_Table( table, index, base, size );
      if (error) goto Fail;
    }
    return;

  Fail:
    parser->error = error;
  }




  static
  void  parse_charstrings( T1_Face  face, T1_Loader*  loader )
  {
    T1_Parser*  parser     = &loader->parser;
    T1_Table*   code_table = &loader->charstrings;
    T1_Table*   name_table = &loader->glyph_names;
    FT_Memory   memory     = parser->memory;
    FT_Error    error;

    T1_Byte*    cur;
    T1_Byte*    limit = parser->limit;
    T1_Int      n;
    
    loader->num_glyphs = T1_ToInt( parser );    
    if (parser->error) return;
    
    /* initialise tables */
    error = T1_New_Table( code_table, loader->num_glyphs, memory ) ||
            T1_New_Table( name_table, loader->num_glyphs, memory );
    if (error) goto Fail;
    
    n = 0;
    for ( ;; )
    {
      T1_Int    size;
      T1_Byte*  base;
      
      /* the format is simple :                   */
      /*   "/glyphname" + binary data             */
      /*                                          */
      /* note that we stop when we find a "def"   */
      /*                                          */
      skip_whitespace(parser);
      cur = parser->cursor;
      if (cur >= limit) break;

      /* we stop when we find a "def" or "end" keyword */
      if (*cur    == 'd' && 
           cur+3 < limit &&
           cur[1] == 'e' &&
           cur[2] == 'f' )
        break;

      if (*cur == 'e'   &&
          cur+3 < limit &&
          cur[1] == 'n' &&
          cur[2] == 'd' )
        break;

      if (*cur != '/')
        skip_blackspace(parser);
      else
      {
        T1_Byte*  cur2 = cur+1;
        T1_Int    len;
        
        while (cur2 < limit && is_alpha(*cur2)) cur2++;
        len = cur2-cur-1;
        
        error = T1_Add_Table( name_table, n, cur+1, len+1 );
        if (error) goto Fail;
       
        /* add a trailing zero to the name table */
        name_table->elements[n][len] = '\0';
       
        parser->cursor = cur2; 
        if (!read_binary_data(parser,&size,&base)) return;

        T1_Decrypt( base, size, 4330 );      
        size -= face->type1.private_dict.lenIV;
        base += face->type1.private_dict.lenIV;

        error = T1_Add_Table( code_table, n, base, size );
        if (error) goto Fail;
        
        n++;
        if (n >= loader->num_glyphs)
          break;
      }
    }
    loader->num_glyphs = n;
    return;
    
  Fail:
    parser->error = error;
  }


#undef PARSE_STRING
#undef PARSE_INT
#undef PARSE_NUM
#undef PARSE_BOOL
#undef PARSE_FIXED
#undef PARSE_COORDS
#undef PARSE_FIXEDS
#undef PARSE_COORDS2
#undef PARSE_FIXEDS2

#undef PARSE_
#define PARSE_(s,x)   { s, parse_##x },

#define PARSE_STRING(s,x)      PARSE_(s,x)
#define PARSE_INT(s,x)         PARSE_(s,x)
#define PARSE_NUM(s,x,t)       PARSE_(s,x)
#define PARSE_BOOL(s,x)        PARSE_(s,x)
#define PARSE_FIXED(s,x)       PARSE_(s,x)
#define PARSE_COORDS(s,c,m,x)  PARSE_(s,x)
#define PARSE_FIXEDS(s,c,m,x)  PARSE_(s,x)
#define PARSE_COORDS2(s,m,x)   PARSE_(s,x)
#define PARSE_FIXEDS2(s,m,x)   PARSE_(s,x)

  static
  const T1_KeyWord  t1_keywords[] =
  {
#include <t1tokens.h>
    /* now add the special functions... */
    { "FontName",    parse_font_name   },
    { "FontBBox",    parse_font_bbox   },
    { "Encoding",    parse_encoding    },
    { "Subrs",       parse_subrs       },
    { "CharStrings", parse_charstrings },
    { 0, 0 }
  };


  static
  T1_Error  parse_dict( T1_Face     face,
                        T1_Loader*  loader,
                        T1_Byte*    base,
                        T1_Long     size )
  {
    T1_Parser*  parser = &loader->parser;
    
    parser->cursor = base;
    parser->limit  = base + size;
    parser->error  = 0;
    
    {
      T1_Byte*  cur     = base;
      T1_Byte*  limit   = cur + size;
      
      for ( ;cur < limit; cur++ )
      {
        /* look for immediates */
        if (*cur == '/' && cur+2 < limit)
        {
          T1_Byte* cur2;
          T1_Int   len;
          
          cur  ++;
          cur2 = cur;
          while (cur2 < limit && is_alpha(*cur2)) cur2++;
          len  = cur2-cur;
          
          if (len > 0 && len < 20)
          {
            /* now, compare the immediate name to the keyword table */
            T1_KeyWord*  keyword = (T1_KeyWord*)t1_keywords;
            
            for (;;)
            {
              T1_Byte*  name;
              
              name = (T1_Byte*)keyword->name;
              if (!name) break;

              if ( cur[0] == name[0] &&
                   len == (T1_Int)strlen((const char*)name) )
              {
                T1_Int  n;
                for ( n = 1; n < len; n++ )
                  if (cur[n] != name[n])
                    break;
                    
                if (n >= len)
                {
                  /* we found it - run the parsing callback !! */
                  parser->cursor = cur2;
                  skip_whitespace( parser );
                  keyword->parsing( face, loader );
                  if (parser->error)
                    return parser->error;

                  cur = parser->cursor;
                  break;
                }
              }
              keyword++;
            }
          }
        }
      }
    }
    return parser->error;
  }

  static
  void t1_init_loader( T1_Loader* loader, T1_Face  face )
  {
    UNUSED(face);
    
    MEM_Set( loader, 0, sizeof(*loader) );
    loader->num_glyphs = 0;
    loader->num_chars  = 0;

    /* initialize the tables - simply set their 'init' field to 0 */    
    loader->encoding_table.init = 0;
    loader->charstrings.init    = 0;
    loader->glyph_names.init    = 0;
    loader->subrs.init          = 0;
  }
  
  static
  void t1_done_loader( T1_Loader* loader )
  {
    T1_Parser*  parser = &loader->parser;
    
    /* finalize tables */
    T1_Release_Table( &loader->encoding_table );
    T1_Release_Table( &loader->charstrings );
    T1_Release_Table( &loader->glyph_names );
    T1_Release_Table( &loader->subrs );
    
    /* finalize parser */
    T1_Done_Parser( parser );
  }

  LOCAL_FUNC
  T1_Error  T1_Open_Face( T1_Face  face )
  {
    T1_Loader  loader;
    T1_Parser* parser;
    T1_Font*   type1 = &face->type1;
    FT_Error   error;

    t1_init_loader( &loader, face );

    /* default lenIV */
    type1->private_dict.lenIV = 4;
    
    parser = &loader.parser;
    error = T1_New_Parser( parser, face->root.stream, face->root.memory );
    if (error) goto Exit;

    error = parse_dict( face, &loader, parser->base_dict, parser->base_len );
    if (error) goto Exit;
    
    error = T1_Get_Private_Dict( parser );
    if (error) goto Exit;
    
    error = parse_dict( face, &loader, parser->private_dict, parser->private_len );
    if (error) goto Exit;    

    /* now, propagate the subrs, charstrings and glyphnames tables */
    /* to the Type1 data                                           */
    type1->num_glyphs = loader.num_glyphs;
    
    if ( !loader.subrs.init )
    {
      FT_ERROR(( "T1.Open_Face: no subrs array in face !!\n" ));
      error = FT_Err_Invalid_File_Format;
    }
    
    if ( !loader.charstrings.init )
    {
      FT_ERROR(( "T1.Open_Face: no charstrings array in face !!\n" ));
      error = FT_Err_Invalid_File_Format;
    }
    
    loader.subrs.init  = 0;
    type1->num_subrs   = loader.num_subrs;
    type1->subrs_block = loader.subrs.block;
    type1->subrs       = loader.subrs.elements;
    type1->subrs_len   = loader.subrs.lengths;
    
    loader.charstrings.init  = 0;
    type1->charstrings_block = loader.charstrings.block;
    type1->charstrings       = loader.charstrings.elements;
    type1->charstrings_len   = loader.charstrings.lengths;
    
    /* we copy the glyph names "block" and "elements" fields */
    /* but the "lengths" field must be released later..      */
    type1->glyph_names_block    = loader.glyph_names.block;
    type1->glyph_names          = (T1_String**)loader.glyph_names.elements;
    loader.glyph_names.block    = 0;
    loader.glyph_names.elements = 0;

    /* we must now build type1.encoding when we have a custom */
    /* array..                                                */
    if ( type1->encoding_type == t1_encoding_array )
    {
      T1_Int    charcode, index, min_char, max_char;
      T1_Byte*  char_name;
      T1_Byte*  glyph_name;

      /* OK, we do the following : for each element in the encoding */
      /* table, lookup the index of the glyph having the same name  */
      /* the index is then stored in type1.encoding.char_index, and */
      /* a the name to type1.encoding.char_name                     */
      
      min_char = +32000;
      max_char = -32000;
      
      charcode = 0;
      for ( ; charcode < loader.encoding_table.num_elems; charcode++ )
      {
        type1->encoding.char_index[charcode] = 0;
        type1->encoding.char_name [charcode] = ".notdef";
        
        char_name = loader.encoding_table.elements[charcode];
        if (char_name)
          for ( index = 0; index < type1->num_glyphs; index++ )
          {
            glyph_name = (T1_Byte*)type1->glyph_names[index];
            if ( strcmp( (const char*)char_name,
                         (const char*)glyph_name ) == 0 )
            {
              type1->encoding.char_index[charcode] = index;
              type1->encoding.char_name [charcode] = (char*)glyph_name;
              
              if (charcode < min_char) min_char = charcode;
              if (charcode > max_char) max_char = charcode;
              break;
            }
          }
      }
      type1->encoding.code_first = min_char;
      type1->encoding.code_last  = max_char;
      type1->encoding.num_chars  = loader.num_chars;
   }
    
  Exit:
    t1_done_loader( &loader );
    return error;
  } 
