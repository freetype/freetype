/***************************************************************************/
/*                                                                         */
/*  ttpost.c                                                               */
/*                                                                         */
/*    Postscript names table processing (body).                            */
/*                                                                         */
/*  Copyright 1996-1999 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used        */
/*  modified and distributed under the terms of the FreeType project       */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* The post table is not completely loaded by the core engine.  This     */
  /* file loads the missing PS glyph names and implements an API to access */
  /* them.                                                                 */
  /*                                                                       */
  /*************************************************************************/

#include <ftstream.h>

#include <ttpost.h>
#include <tterrors.h>
#include <ttload.h>
#include <tttags.h>

/* When this configuration macro is defined, we rely on the "psnames" */
/* module to grab the glyph names..                                   */
#ifdef FT_CONFIG_OPTION_POSTSCRIPT_NAMES
#include <psnames.h>
#define  MAC_NAME(x)  ((TT_String*)psnames->macintosh_name(x))

#else

/* Otherwise, we ignore the "psnames" module, then provide our own */
/* table of Mac names.. Thus, it is possible to build a version of */
/* FreeType without the Type 1 driver & PSNames module             */

#define  MAC_NAME(x)  TT_Post_Default_Names[x]

  /* the 258 default Mac PS glyph names */

  FT_String*  TT_Post_Default_Names[258] =
  {
    /*   0 */
    ".notdef", ".null", "CR", "space", "exclam",
    "quotedbl", "numbersign", "dollar", "percent", "ampersand",
    /*  10 */
    "quotesingle", "parenleft", "parenright", "asterisk", "plus",
    "comma", "hyphen", "period", "slash", "zero",
    /*  20 */
    "one", "two", "three", "four", "five",
    "six", "seven", "eight", "nine", "colon",
    /*  30 */
    "semicolon", "less", "equal", "greater", "question",
    "at", "A", "B", "C", "D",
    /*  40 */
    "E", "F", "G", "H", "I",
    "J", "K", "L", "M", "N",
    /*  50 */
    "O", "P", "Q", "R", "S",
    "T", "U", "V", "W", "X",
    /*  60 */
    "Y", "Z", "bracketleft", "backslash", "bracketright",
    "asciicircum", "underscore", "grave", "a", "b",
    /*  70 */
    "c", "d", "e", "f", "g",
    "h", "i", "j", "k", "l",
    /*  80 */
    "m", "n", "o", "p", "q",
    "r", "s", "t", "u", "v",
    /*  90 */
    "w", "x", "y", "z", "braceleft",
    "bar", "braceright", "asciitilde", "Adieresis", "Aring",
    /* 100 */
    "Ccedilla", "Eacute", "Ntilde", "Odieresis", "Udieresis",
    "aacute", "agrave", "acircumflex", "adieresis", "atilde",
    /* 110 */
    "aring", "ccedilla", "eacute", "egrave", "ecircumflex",
    "edieresis", "iacute", "igrave", "icircumflex", "idieresis",
    /* 120 */
    "ntilde", "oacute", "ograve", "ocircumflex", "odieresis",
    "otilde", "uacute", "ugrave", "ucircumflex", "udieresis",
    /* 130 */
    "dagger", "degree", "cent", "sterling", "section",
    "bullet", "paragraph", "germandbls", "registered", "copyright",
    /* 140 */
    "trademark", "acute", "dieresis", "notequal", "AE",
    "Oslash", "infinity", "plusminus", "lessequal", "greaterequal",
    /* 150 */
    "yen", "mu", "partialdiff", "summation", "product",
    "pi", "integral", "ordfeminine", "ordmasculine", "Omega",
    /* 160 */
    "ae", "oslash", "questiondown", "exclamdown", "logicalnot",
    "radical", "florin", "approxequal", "Delta", "guillemotleft",
    /* 170 */
    "guillemotright", "ellipsis", "nbspace", "Agrave", "Atilde",
    "Otilde", "OE", "oe", "endash", "emdash",
    /* 180 */
    "quotedblleft", "quotedblright", "quoteleft", "quoteright", "divide",
    "lozenge", "ydieresis", "Ydieresis", "fraction", "currency",
    /* 190 */
    "guilsinglleft", "guilsinglright", "fi", "fl", "daggerdbl",
    "periodcentered", "quotesinglbase", "quotedblbase", "perthousand", "Acircumflex",
    /* 200 */
    "Ecircumflex", "Aacute", "Edieresis", "Egrave", "Iacute",
    "Icircumflex", "Idieresis", "Igrave", "Oacute", "Ocircumflex",
    /* 210 */
    "apple", "Ograve", "Uacute", "Ucircumflex", "Ugrave",
    "dotlessi", "circumflex", "tilde", "macron", "breve",
    /* 220 */
    "dotaccent", "ring", "cedilla", "hungarumlaut", "ogonek",
    "caron", "Lslash", "lslash", "Scaron", "scaron",
    /* 230 */
    "Zcaron", "zcaron", "brokenbar", "Eth", "eth",
    "Yacute", "yacute", "Thorn", "thorn", "minus",
    /* 240 */
    "multiply", "onesuperior", "twosuperior", "threesuperior", "onehalf",
    "onequarter", "threequarters", "franc", "Gbreve", "gbreve",
    /* 250 */
    "Idot", "Scedilla", "scedilla", "Cacute", "cacute",
    "Ccaron", "ccaron", "dmacron",
  };
#endif


  static
  TT_Error  Load_Format_20( TT_Face    face,
                            FT_Stream  stream )
  {
    FT_Memory  memory = stream->memory;
    TT_Error   error;

    TT_Int     num_glyphs;
    TT_Int     num_names;

    TT_UShort* glyph_indices = 0;
    TT_Char**  name_strings  = 0;


    if ( READ_UShort( num_glyphs ) )
      goto Exit;

    /* UNDOCUMENTED!  The number of glyphs in this table can be smaller */
    /* than the value in the maxp table (cf. cyberbit.ttf).             */

    /* There already exist fonts which have more than 32768 glyph names */
    /* in this table, so the test for this threshold has been dropped.  */

    if ( num_glyphs > face->root.num_glyphs )
    {
      error = TT_Err_Invalid_File_Format;
      goto Exit;
    }

    /* load the indices */
    {
      TT_Int  n;

      if ( ALLOC_ARRAY ( glyph_indices, num_glyphs, TT_UShort ) ||
           ACCESS_Frame( num_glyphs * 2L )                      )
        goto Fail;

      for ( n = 0; n < num_glyphs; n++ )
        glyph_indices[n] = GET_UShort();

      FORGET_Frame();
    }

    /* compute number of names stored in table */
    {
      TT_Int  n;

      num_names = 0;

      for ( n = 0; n < num_glyphs; n++ )
      {
        TT_Int  index;

        index = glyph_indices[n];
        if ( index >= 258 )
        {
          index -= 257;
          if ( index > num_names )
            num_names = index;
        }
      }
    }

    /* now load the name strings */
    {
      TT_Int  n;

      if ( ALLOC_ARRAY( name_strings, num_names, TT_Char* ) )
        goto Fail;

      for ( n = 0; n < num_names; n++ )
      {
        TT_UInt  len;

        if ( READ_Byte  ( len )                             ||
             ALLOC_ARRAY( name_strings[n], len+1, TT_Char ) ||
             FILE_Read  ( name_strings[n], len )            )
          goto Fail1;

        name_strings[n][len] = '\0';
      }
    }

    /* all right, set table fields and exit successfuly */
    {
      TT_Post_20*  table = &face->postscript_names.names.format_20;

      table->num_glyphs    = num_glyphs;
      table->num_names     = num_names;
      table->glyph_indices = glyph_indices;
      table->glyph_names   = name_strings;
    }
    return TT_Err_Ok;


  Fail1:
    {
      TT_Int  n;

      for ( n = 0; n < num_names; n++ )
        FREE( name_strings[n] );
    }

  Fail:
    FREE( name_strings );
    FREE( glyph_indices );

  Exit:
    return error;
  }


  static
  TT_Error  Load_Format_25( TT_Face    face,
                            FT_Stream  stream )
  {
    FT_Memory  memory = stream->memory;
    TT_Error   error;

    TT_Int     num_glyphs;
    TT_Char*   offset_table = 0;


    /* UNDOCUMENTED!  This value appears only in the Apple TT specs. */
    if ( READ_UShort( num_glyphs ) )
      goto Exit;

    /* check the number of glyphs */
    if ( num_glyphs > face->root.num_glyphs || num_glyphs > 258 )
    {
      error = TT_Err_Invalid_File_Format;
      goto Exit;
    }

    if ( ALLOC    ( offset_table, num_glyphs ) ||
         FILE_Read( offset_table, num_glyphs ) )
      goto Fail;

    /* now check the offset table */
    {
      TT_Int  n;


      for ( n = 0; n < num_glyphs; n++ )
      {
        TT_Long  index = (TT_Long)n + offset_table[n];


        if ( index < 0 || index > num_glyphs )
        {
          error = TT_Err_Invalid_File_Format;
          goto Fail;
        }
      }
    }

    /* OK, set table fields and exit successfuly */
    {
      TT_Post_25*  table = &face->postscript_names.names.format_25;


      table->num_glyphs = num_glyphs;
      table->offsets    = offset_table;
    }

    return TT_Err_Ok;

  Fail:
    FREE( offset_table );

  Exit:
    return error;
  }


  static
  TT_Error  Load_Post_Names( TT_Face  face )
  {
    FT_Stream   stream;
    TT_Error    error;

    /* get a stream for the face's resource */
    stream = face->root.stream;

    /* seek to the beginning of the PS names table */
    error = face->goto_table( face, TTAG_post, stream, 0 );
    if (error) goto Exit;

    /* now read postscript table */
    switch ( face->postscript.FormatType )
    {
    case 0x00020000:
      error = Load_Format_20( face, stream );
      break;

    case 0x00028000:
      error = Load_Format_25( face, stream );
      break;

    default:
      error = TT_Err_Invalid_File_Format;
    }

    face->postscript_names.loaded = 1;

  Exit:
    return error;
  }


  LOCAL_FUNC
  void  TT_Free_Post_Names( TT_Face  face )
  {
    FT_Memory       memory = face->root.memory;
    TT_Post_Names*  names  = &face->postscript_names;


    if ( names->loaded )
    {
      switch ( face->postscript.FormatType )
      {
      case 0x00020000:
        {
          TT_Post_20*  table = &names->names.format_20;
          TT_UInt      n;


          FREE( table->glyph_indices );
          table->num_glyphs = 0;

          for ( n = 0; n < table->num_names; n++ )
            FREE( table->glyph_names[n] );

          FREE( table->glyph_names );
          table->num_names = 0;
        }
        break;

      case 0x00028000:
        {
          TT_Post_25*  table = &names->names.format_25;


          FREE( table->offsets );
          table->num_glyphs = 0;
        }
        break;
      }
    }
    names->loaded = 0;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Get_PS_Name                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Gets the PostScript glyph name of a glyph.                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    index  :: The glyph index.                                         */
  /*                                                                       */
  /*    PSname :: The address of a string pointer.  Will be NULL in case   */
  /*              of error, otherwise it is a pointer to the glyph name.   */
  /*                                                                       */
  /*              You must not modify the returned string!                 */
  /*                                                                       */
  /* <Output>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  EXPORT_FUNC
  TT_Error  TT_Get_PS_Name( TT_Face      face,
                            TT_UInt      index,
                            TT_String**  PSname )
  {
    TT_Error            error;
    TT_Post_Names*      names;
#ifdef FT_CONFIG_OPTION_POSTSCRIPT_NAMES
    PSNames_Interface*  psnames;
#endif
    if ( !face )
      return TT_Err_Invalid_Face_Handle;

    if ( index >= (TT_UInt)face->root.num_glyphs )
      return TT_Err_Invalid_Glyph_Index;

#ifdef FT_CONFIG_OPTION_POSTSCRIPT_NAMES
    psnames = (PSNames_Interface*)face->psnames;
    if (!psnames)
      return TT_Err_Unimplemented_Feature;
#endif

    names   = &face->postscript_names;

    /* `.notdef' by default */
    *PSname = MAC_NAME(0);

    switch ( face->postscript.FormatType )
    {
    case 0x00010000:
      if ( index < 258 )                    /* paranoid checking */
        *PSname = MAC_NAME(index);
      break;

    case 0x00020000:
      {
        TT_Post_20*  table = &names->names.format_20;


        if ( !names->loaded )
        {
          error = Load_Post_Names( face );
          if ( error )
            break;
        }

        if ( index < table->num_glyphs )
        {
          TT_UShort  name_index = table->glyph_indices[index];


          if ( name_index < 258 )
            *PSname = MAC_NAME(name_index);
          else
            *PSname = (TT_String*)table->glyph_names[name_index - 258];
        }
      }
      break;

    case 0x00028000:
      {
        TT_Post_25*  table = &names->names.format_25;


        if ( !names->loaded )
        {
          error = Load_Post_Names( face );
          if ( error )
            break;
        }

        if ( index < table->num_glyphs )    /* paranoid checking */
        {
          index  += table->offsets[index];
          *PSname = MAC_NAME(index);
        }
      }
      break;

    case 0x00030000:
      break;                                /* nothing to do */
    }

    return TT_Err_Ok;
  }


/* END */
