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

#include <freetype/internal/ftdebug.h>
#include <freetype/config/ftconfig.h>

#include <freetype/internal/t1types.h>
#include <t1errors.h>
#include <t1load.h>
#include <stdio.h>

#undef  FT_COMPONENT
#define FT_COMPONENT  trace_t1load

 /***************************************************************************/
 /***************************************************************************/
 /*****                                                                 *****/
 /*****                    MULTIPLE MASTERS SUPPORT                     *****/
 /*****                                                                 *****/
 /***************************************************************************/
 /***************************************************************************/

 static  T1_Error  t1_allocate_blend( T1_Face   face,
                                      T1_UInt   num_designs,
                                      T1_UInt   num_axis )
 {
   T1_Blend*  blend;
   FT_Memory  memory = face->root.memory;
   T1_Error   error  = 0;
   
   blend = face->blend;
   if (!blend)
   {
     if ( ALLOC( blend, sizeof(*blend) ) )
       goto Exit;
       
     face->blend = blend;
   }
   
   /* allocate design data if needed */
   if (num_designs > 0)
   {
     if (blend->num_designs == 0)
     {
       /* allocate the blend "private" and "font_info" dictionaries */
       if ( ALLOC_ARRAY( blend->font_infos[1], num_designs, T1_FontInfo ) ||
            ALLOC_ARRAY( blend->privates[1], num_designs, T1_Private )     )
         goto Exit;
  
       blend->font_infos[0] = &face->type1.font_info;
       blend->privates  [0] = &face->type1.private_dict;
       blend->num_designs   = num_designs;
     }
     else if (blend->num_designs != num_designs)
       goto Fail;
   }
   
   /* allocate axis data if needed */
   if (num_axis > 0)
   {
     if (blend->num_axis != 0 && blend->num_axis != num_axis)
       goto Fail;
       
     blend->num_axis = num_axis;
   }
   
   /* allocate the blend design pos table if needed */
   num_designs = blend->num_designs;
   num_axis    = blend->num_axis;
   if ( num_designs && num_axis && blend->design_pos[0] == 0)
   {
     FT_UInt  n;
     
     if ( ALLOC_ARRAY( blend->design_pos[0], num_designs*num_axis, FT_Fixed ) )
       goto Exit;
       
     for ( n = 1; n < num_designs; n++ )
       blend->design_pos[n] = blend->design_pos[0] + num_axis*n;       
   }
   
 Exit:
   return error;
 Fail:
   error = -1;
   goto Exit;
 }                                  


 static  void t1_done_blend( T1_Face  face )
 {
   FT_Memory  memory = face->root.memory;
   T1_Blend*  blend  = face->blend;
   
   if (blend)
   {
     T1_UInt  num_designs = blend->num_designs;
     T1_UInt  num_axis    = blend->num_axis;
     T1_UInt  n;
          
     /* release design pos table */
     FREE( blend->design_pos[0] );
     for ( n = 1; n < num_designs; n++ )
       blend->design_pos[n] = 0;

     /* release blend "private" and "font info" dictionaries */
     FREE( blend->privates[1] );
     FREE( blend->font_infos[1] );
     for ( n = 0; n < num_designs; n++ )
     {
       blend->privates  [n] = 0;
       blend->font_infos[n] = 0;
     }

     /* release axis names */
     for ( n = 0; n < num_axis; n++ )
       FREE( blend->axis_names[n] );
     
     /* release design map */
     for ( n = 0; n < num_axis; n++ )
     {
       T1_DesignMap*  dmap = blend->design_map + n;
       FREE( dmap->design_points );
       FREE( dmap->blend_points );
       dmap->num_points = 0;
     }
     
     FREE( face->blend );
   }
 }

 /***************************************************************************/
 /***************************************************************************/
 /*****                                                                 *****/
 /*****                      TYPE 1 SYMBOL PARSING                      *****/
 /*****                                                                 *****/
 /***************************************************************************/
 /***************************************************************************/

 /*********************************************************************
  *
  *  First of all, define the token field static variables. This is
  *  a set of T1_Field_Rec variables used later..
  *
  *********************************************************************/

#define T1_NEW_STRING( _name, _field ) \
   static const  T1_Field_Rec  t1_field_ ## _field = T1_FIELD_STRING( T1TYPE, _field );

#define T1_NEW_BOOL( _name, _field )   \
   static const  T1_Field_Rec  t1_field_ ## _field = T1_FIELD_BOOL( T1TYPE, _field );
   
#define T1_NEW_NUM( _name, _field )    \
   static const  T1_Field_Rec  t1_field_ ## _field = T1_FIELD_NUM( T1TYPE, _field );
   
#define T1_NEW_FIXED( _name, _field )  \
   static const  T1_Field_Rec  t1_field_ ## _field = T1_FIELD_FIXED( T1TYPE, _field, _power );

#define T1_NEW_NUM_TABLE( _name, _field, _max, _count ) \
   static const  T1_Field_Rec  t1_field_ ## _field = T1_FIELD_NUM_ARRAY( T1TYPE, _field, _count, _max );
   
#define T1_NEW_FIXED_TABLE( _name, _field, _max, _count ) \
   static const  T1_Field_Rec  t1_field_ ## _field = T1_FIELD_FIXED_ARRAY( T1TYPE, _field, _count, _max );

#define T1_NEW_NUM_TABLE2( _name, _field, _max ) \
   static const  T1_Field_Rec  t1_field_ ## _field = T1_FIELD_NUM_ARRAY2( T1TYPE, _field, _max );
   
#define T1_NEW_FIXED_TABLE2( _name, _field, _max ) \
   static const  T1_Field_Rec  t1_field_ ## _field = T1_FIELD_FIXED_ARRAY2( T1TYPE, _field, _max );


#define T1_FONTINFO_STRING(n,f)        T1_NEW_STRING(n,f)
#define T1_FONTINFO_NUM(n,f)           T1_NEW_NUM(n,f)
#define T1_FONTINFO_BOOL(n,f)          T1_NEW_BOOL(n,f)
#define T1_PRIVATE_NUM(n,f)            T1_NEW_NUM(n,f)
#define T1_PRIVATE_FIXED(n,f)          T1_NEW_FIXED(n,f)
#define T1_PRIVATE_NUM_TABLE(n,f,m,c)  T1_NEW_NUM_TABLE(n,f,m,c)
#define T1_PRIVATE_NUM_TABLE2(n,f,m)   T1_NEW_NUM_TABLE2(n,f,m)
#define T1_TOPDICT_NUM(n,f)            T1_NEW_NUM(n,f)
#define T1_TOPDICT_NUM_FIXED2(n,f,m)   T1_NEW_FIXED_TABLE2(n,f,m)

/* including this file defines all field variables */
#include <t1tokens.h>

 /*********************************************************************
  *
  *  Second, define the keyword variables. This is a set of T1_KeyWord
  *  structures used to model the way each keyword is "loaded"..
  *
  *********************************************************************/

  typedef  void  (*T1_Parse_Func)( T1_Face   face, T1_Loader*  loader );

  typedef enum T1_KeyWord_Type_
  {
    t1_keyword_callback = 0,
    t1_keyword_field,
    t1_keyword_field_table
    
  } T1_KeyWord_Type;
  
  typedef enum T1_KeyWord_Location_
  {
    t1_keyword_type1 = 0,
    t1_keyword_font_info,
    t1_keyword_private
  
  } T1_KeyWord_Location;
  
  typedef  struct T1_KeyWord_
  {
    const char*          name;
    T1_KeyWord_Type      type;
    T1_KeyWord_Location  location;
    T1_Parse_Func        parsing;
    const T1_Field_Rec*  field;

  } T1_KeyWord;


#define T1_KEYWORD_CALLBACK( name, callback )  \
         { name, t1_keyword_callback, t1_keyword_type1, callback, 0 }

#define T1_KEYWORD_TYPE1( name, f ) \
         { name, t1_keyword_field, t1_keyword_type1, 0, &t1_field_ ## f }
         
#define T1_KEYWORD_FONTINFO( name, f ) \
         { name, t1_keyword_field, t1_keyword_font_info, 0, &t1_field_ ## f }

#define T1_KEYWORD_PRIVATE( name, f ) \
         { name, t1_keyword_field, t1_keyword_private, 0, &t1_field_ ## f }

#define T1_KEYWORD_FONTINFO_TABLE( name, f ) \
         { name, t1_keyword_field_table, t1_keyword_font_info, 0, &t1_field_ ## f }

#define T1_KEYWORD_PRIVATE_TABLE( name, f ) \
         { name, t1_keyword_field_table, t1_keyword_private, 0, &t1_field_ ## f }

#undef  T1_FONTINFO_STRING
#undef  T1_FONTINFO_NUM
#undef  T1_FONTINFO_BOOL
#undef  T1_PRIVATE_NUM
#undef  T1_PRIVATE_FIXED
#undef  T1_PRIVATE_NUM_TABLE
#undef  T1_PRIVATE_NUM_TABLE2
#undef  T1_TOPDICT_NUM
#undef  T1_TOPDICT_NUM_FIXED2

#define T1_FONTINFO_STRING(n,f)        T1_KEYWORD_FONTINFO(n,f),
#define T1_FONTINFO_NUM(n,f)           T1_KEYWORD_FONTINFO(n,f),
#define T1_FONTINFO_BOOL(n,f)          T1_KEYWORD_FONTINFO(n,f),
#define T1_PRIVATE_NUM(n,f)            T1_KEYWORD_PRIVATE(n,f),
#define T1_PRIVATE_FIXED(n,f)          T1_KEYWORD_PRIVATE(n,f),
#define T1_PRIVATE_NUM_TABLE(n,f,m,c)  T1_KEYWORD_PRIVATE_TABLE(n,f),
#define T1_PRIVATE_NUM_TABLE2(n,f,m)   T1_KEYWORD_PRIVATE_TABLE(n,f),
#define T1_TOPDICT_NUM(n,f)            T1_KEYWORD_TYPE1(n,f),
#define T1_TOPDICT_NUM_FIXED2(n,f,m)   T1_KEYWORD_TYPE1(n,f),


  static  T1_Error   t1_load_keyword( T1_Face     face,
                                      T1_Loader*  loader,
                                      T1_KeyWord* keyword )
  {
    T1_Error  error;
    void*     dummy_object;
    void**    objects;
    T1_UInt   max_objects;
    T1_Blend* blend = face->blend;
    
    /* if the keyword has a dedicated callback, call it */
    if (keyword->type == t1_keyword_callback)
    {
      keyword->parsing( face, loader );
      error = loader->parser.error;
      goto Exit;
    }
    
    /* now, the keyword is either a simple field, or a table of fields */
    /* we are now going to take care of it..                           */
    switch (keyword->location)
    {
      case t1_keyword_font_info:
        {
          dummy_object = &face->type1.font_info;
          objects      = &dummy_object;
          max_objects  = 0;
          if (blend)
          {
            objects     = (void**)blend->font_infos;
            max_objects = blend->num_designs;
          }
        }
        break;
        
      case t1_keyword_private:
        {
          dummy_object = &face->type1.private_dict;
          objects      = &dummy_object;
          max_objects  = 0;
          if (blend)
          {
            objects     = (void**)blend->privates;
            max_objects = blend->num_designs;
          }
        }
        break;
      
      default:
        dummy_object = &face->type1;
        objects      = &dummy_object;
        max_objects  = 0;
    }
    
    if (keyword->type == t1_keyword_field_table)
      error = T1_Load_Field_Table( &loader->parser, keyword->field, objects, max_objects, 0 );
    else
      error = T1_Load_Field( &loader->parser, keyword->field, objects, max_objects, 0 );
    
  Exit:
    return error;
  }                                


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
  void  parse_font_matrix( T1_Face  face, T1_Loader*  loader )
  {
    T1_Parser*  parser = &loader->parser;
    FT_Matrix*  matrix = &face->type1.font_matrix;
    T1_Fixed    temp[4];

    (void)T1_ToFixedArray( parser, 4, temp, 3 );
    matrix->xx = temp[0];
    matrix->yx = temp[1];
    matrix->xy = temp[2];
    matrix->yy = temp[3];
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




  static
  const T1_KeyWord  t1_keywords[] =
  {
#include <t1tokens.h>  
    
    /* now add the special functions... */
    T1_KEYWORD_CALLBACK( "FontName", parse_font_name ),
    T1_KEYWORD_CALLBACK( "FontBBox", parse_font_bbox ),
    T1_KEYWORD_CALLBACK( "FontMatrix", parse_font_matrix ),
    T1_KEYWORD_CALLBACK( "Encoding", parse_encoding  ),
    T1_KEYWORD_CALLBACK( "Subrs",    parse_subrs     ),
    T1_KEYWORD_CALLBACK( "CharStrings", parse_charstrings ),
    T1_KEYWORD_CALLBACK( 0, 0 )
  };


  static
  T1_Error  parse_dict( T1_Face     face,
                        T1_Loader*  loader,
                        T1_Byte*    base,
                        T1_Long     size )
  {
    T1_Parser*  parser   = &loader->parser;

    parser->cursor = base;
    parser->limit  = base + size;
    parser->error  = 0;

    {
      T1_Byte*  cur     = base;
      T1_Byte*  limit   = cur + size;

      for ( ;cur < limit; cur++ )
      {
        /* look for "FontDirectory", which causes problems on some fonts */
        if ( *cur == 'F' && cur+25 < limit &&
             strncmp( (char*)cur, "FontDirectory", 13 ) == 0 )
        {
          T1_Byte*  cur2;
          
          /* skip the "FontDirectory" keyword */
          cur += 13;
          cur2 = cur;
          
          /* lookup the 'known' keyword */
          while (cur < limit && *cur != 'k' && strncmp( (char*)cur, "known", 5 ) )
            cur++;
          
          if (cur < limit)
          {
            T1_Token_Rec  token;
            
            /* skip the "known" keyword and the token following it */
            cur += 5;
            loader->parser.cursor = cur;
            T1_ToToken( &loader->parser, &token );
            
            /* if the last token was an array, skip it !! */
            if (token.type == t1_token_array)
              cur2 = parser->cursor;
          }
          cur = cur2;
        }
        /* look for immediates */
        else if (*cur == '/' && cur+2 < limit)
        {
          T1_Byte* cur2;
          T1_Int   len;

          cur  ++;
          cur2 = cur;
          while (cur2 < limit && is_alpha(*cur2)) cur2++;
          len  = cur2-cur;

          if (len > 0 && len < 20)
          {
            if (!loader->fontdata)
            {
              if ( strncmp( (char*)cur, "FontInfo", 8 ) == 0 )
                loader->fontdata = 1;
            }
            else
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
                    parser->error = t1_load_keyword( face, loader, keyword );
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
    loader->fontdata            = 0;
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
