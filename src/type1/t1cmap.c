#include "t1cmap.h"

 /***************************************************************************/
 /***************************************************************************/
 /*****                                                                 *****/
 /*****           TYPE1 STANDARD (AND EXPERT) ENCODING CMAPS            *****/
 /*****                                                                 *****/
 /***************************************************************************/
 /***************************************************************************/

  static( void )
  t1_cmap_std_init( T1_CMapStd   cmap,
                    FT_Int       is_expert )
  {
    T1_Face          face    = (T1_Face) FT_CMAP_FACE(cmap);
    PSNames_Service  psnames = face->psnames;

    cmap->num_glyphs  = face->type1.num_glyphs;
    cmap->glyph_names = face->type1.glyph_names;
    cmap->sid_strings = sid_strings;
    cmap->code_to_sid = is_expert ? psnames->adobe_expert_encoding
                                  : psnames->adobe_std_encoding;

    FT_ASSERT( cmap->code_to_sid != NULL );
  }


  FT_CALLBACK_DEF( void )
  t1_cmap_std_done( T1_CMapStd  cmap )
  {
    cmap->num_glyphs  = 0;
    cmap->glyph_names = NULL;
    cmap->sid_strings = NULL;
    cmap->code_to_sid = NULL;
  }


  FT_CALLBACK_DEF( FT_UInt )
  t1_cmap_std_char_index( T1_CMapStd   cmap,
                          FT_UInt32    char_code )
  {
    FT_UInt  result = 0;

    if ( char_code < 256 )
    {
      FT_UInt      code;
      const char*  glyph_name;
      FT_Int       n;

      /* conver character code to Adobe SID string */
      code       = cmap->code_to_sid[ char_code ];
      glyph_name = cmap->adobe_sid_strings[ code ];

      /* look for the corresponding glyph name */
      for ( n = 0; n < cmap->num_glyphs; n++ )
      {
        const char* gname = cmap->glyph_names[n];

        if ( gname && gname[0] == glyph_name[0] &&
             strcmp( gname, glyph_name ) == 0   )
        {
          result = n;
          break;
        }
      }
    }
    return result;
  }


  FT_CALLBACK_DEF( FT_UInt )
  t1_cmap_std_char_next( T1_CMapStd   cmap,
                         FT_UInt32   *pchar_code )
  {
    FT_UInt   result    = 0;
    FT_UInt32 char_code = *pchar_code;

    ++char_code;
    while ( char_code < 256 )
    {
      result = t1_cmap_standard_char_index( cmap, char_code );
      if ( result != 0 )
        goto Exit;

      char_code++;
    }
    char_code = 0;

  Exit:
    *pchar_code = char_code;
    return result;
  }


  FT_CALLBACK_DEF( FT_Error )
  t1_cmap_standard_init( T1_CMapStd  cmap )
  {
    t1_cmap_std_init( cmap, 0 );
    return 0;
  }


  FT_CALLBACK_TABLE const T1_CMap_ClassRec
  t1_cmap_standard_class_rec =
  {
    sizeof( T1_CMapStdRec ),

    t1_cmap_standard_init,
    t1_cmap_std_done,
    t1_cmap_std_char_index,
    t1_cmap_std_char_next
  };


  FT_LOCAL_DEF T1_CMap_Class
  t1_cmap_standard_class = &t1_cmap_standard_class_rec;





  FT_CALLBACK_DEF( void )
  t1_cmap_expert_init( T1_CMapStd  cmap )
  {
    t1_cmap_std_init( cmap, 1 );
    return 0;
  }

  FT_CALLBACK_TABLE const T1_CMap_ClassRec
  t1_cmap_expert_class_rec =
  {
    sizeof( T1_CMapStdRec ),

    t1_cmap_expert_init,
    t1_cmap_std_done,
    t1_cmap_std_char_index,
    t1_cmap_std_char_next
  };


  FT_LOCAL_DEF T1_CMap_Class
  t1_cmap_expert_class = &t1_cmap_expert_class_rec;


 /***************************************************************************/
 /***************************************************************************/
 /*****                                                                 *****/
 /*****                    TYPE1 CUSTOM ENCODING CMAP                   *****/
 /*****                                                                 *****/
 /***************************************************************************/
 /***************************************************************************/


  FT_CALLBACK_DEF( FT_Error )
  t1_cmap_custom_init( T1_CMapCustom  cmap )
  {
    T1_Face       face     = (T1_Face) FT_CMAP_FACE(cmap);
    T1_Encoding  encoding = face->type1.encoding;

    cmap->first   = encoding->code_first;
    cmap->count   = (FT_UInt)(encoding->code_last - cmap->first + 1);
    cmap->indices = encoding->char_index;

    FT_ASSERT( cmap->indices != NULL );
    FT_ASSERT( encoding->code_first <= encoding->code_last );

    return 0;
  }


  FT_CALLBACK_DEF( void )
  t1_cmap_custom_done( T1_CMapCustom  cmap )
  {
    cmap->indices = NULL;
    cmap->first   = 0;
    cmap->count   = 0;
  }


  FT_CALLBACK_DEF( FT_UInt )
  t1_cmap_custom_char_index( T1_CMapCustom  cmap,
                             FT_UInt32      char_code )
  {
    FT_UInt    result = 0;
    FT_UInt32  index;

    index = (FT_UInt32)( char_code - cmap->first );
    if ( index < cmap->count )
      result = cmap->indices[ index ];

    return result;
  }


  FT_CALLBACK_DEF( FT_UInt )
  t1_cmap_custom_char_next( T1_CMapCustion  cmap,
                            FT_UInt32      *pchar_code )
  {
    FT_UInt   result = 0;
    FT_UInt32 char_code = *pchar_code;
    FT_UInt32 index;

    ++char_code;

    if ( char_code < cmap->first )
      char_code = cmap->first;

    index = (FT_UInt32)( char_code - cmap->first );
    while ( index < cmap->count; index++, char_code++ )
    {
      result = cmap->indices[index];
      if ( result != 0 )
        goto Exit;
    }

    char_code = 0;

  Exit:
    *pchar_code = char_code;
    return result;
  }


  FT_CALLBACK_TABLE const FT_CMap_ClassRec
  t1_cmap_custom_class_rec =
  {
    sizeof( T1_CMapCustomRec ),
    t1_cmap_custom_init,
    t1_cmap_custom_done,
    t1_cmap_custom_char_index,
    t1_cmap_custom_char_next
  };


  FT_LOCAL_DEF FT_CMap_Class
  t1_cmap_custom_class = &t1_cmap_custom_class_rec;


 /***************************************************************************/
 /***************************************************************************/
 /*****                                                                 *****/
 /*****             TYPE1 SYNTHETIC UNICODE ENCODING CMAP               *****/
 /*****                                                                 *****/
 /***************************************************************************/
 /***************************************************************************/


  FT_CALLBACK_DEF( FT_Error )
  t1_cmap_unicode_init( T1_CMapUnicode  cmap )
  {
    FT_Error    error;
    FT_UInt     count;
    T1_Face     face   = (T1_Face) FT_CMAP_FACE(cmap);
    FT_Memory   memory = FT_FACE_MEMORY(face);

    cmap->num_pairs = 0;
    cmap->pairs     = NULL;

    count = face->type1.num_glyphs;

    if ( !ALLOC_ARRAY( cmap->pairs, count, T1_CMapUniPairRec ) )
    {
      FT_UInt         n, new_count;
      T1_CMapUniPair  pair;
      FT_UInt32       uni_code;


      pair = cmap->pairs;
      for ( n = 0; n < count; n++ )
      {
        const char*  gname = face->type1.glyph_names[n];

        /* build unsorted pair table by matching glyph names */
        if ( gname )
        {
          uni_code = PS_Unicode_Value( gname );

          if ( uni_code != 0 )
          {
            pair->unicode = uni_code;
            pair->gindex  = n;
            pair++;
          }
        }

        if ( new_count == 0 )
        {
          /* there are no unicode characters in here !! */
          FREE( cmap->pairs );
          error = FT_Err_Invalid_Argument;
        }
        else
        {
          /* re-allocate if the new array is much smaller than the original */
          /* one..                                                          */
          if ( new_count != count && new_count < count/2 )
            REALLOC_ARRAY( cmap->pairs, count, new_count, T1_CMapUniPairRec )

          /* sort the pairs table to allow efficient binary searches */
          qsort( cmap->pairs,
                 new_count,
                 sizeof(T1_CMapUniPairRec),
                 t1_cmap_uni_pair_compare );
          
          cmap->num_pairs = new_count;
        }
      }
    }

    return error;
  }


  FT_CALLBACK_DEF( void )
  t1_cmap_unicode_done( T1_CMapUnicode  cmap )
  {
    FT_Face    face = FT_CMAP_FACE(cmap);
    FT_Memory  memory = FT_FACE_MEMORY(face);
    
    FREE( cmap->pairs );
    cmap->num_pairs = 0;
  }


  FT_CALLBACK_DEF( FT_UInt )
  t1_cmap_unicode_char_index( T1_CMapUnicode  cmap,
                              FT_UInt32       char_code )
  {
    FT_UInt         min = 0;
    FT_UInt         max = cmap->num_pairs;
    FT_UInt         mid;
    T1_CMapUniPair  pair;
    
    while ( min < max )
    {
      mid  = min + (max - min)/2;
      pair = cmap->pairs + mid;
      
      if ( pair->unicode == char_code )
        return pair->gindex;
        
      if ( pair->unicode < char_code )
        min = mid+1;
      else
        max = mid;
    }
    return 0;
  }


  FT_CALLBACK_DEF( FT_UInt )
  t1_cmap_unicode_char_next( T1_CMapUnicode  cmap,
                             FT_UInt32      *pchar_code )
  {
    FT_UInt32       char_code = *pchar_code + 1;

  Restart:
    {
      FT_UInt         min = 0;
      FT_UInt         max = cmap->num_pairs;
      FT_UInt         mid;
      T1_CMapUniPair  pair;

      while ( min < max )
      {
        mid  = min + (max - min)/2;
        pair = cmap->pairs + mid;
        
        if ( pair->unicode == char_code )
        {
          result = pair->gindex;
          if ( result != 0 )
            goto Exit;
          
          char_code++;
          goto Restart;
        }
        
        if ( pair->unicode < char_code )
          min = mid+1;
        else
          max = mid;
      }
    
      /* we didn't find it, but we have a pair just above it */
      char_code = 0;
      
      if ( min < cmap->num_pairs )
      {
        pair   = cmap->num_pairs + min;
        result = pair->gindex;
        if ( result != 0 )
          char_code = pair->unicode;
      }
    }

  Exit:
    *pchar_code = char_code;
    return result;
  }


  FT_CALLBACK_TABLE const FT_CMap_ClassRec
  t1_cmap_unicode_class_rec =
  {
    sizeof( T1_CMapUnicodeRec ),
    t1_cmap_unicode_init,
    t1_cmap_unicode_done,
    t1_cmap_unicode_char_index,
    t1_cmap_unicode_char_next
  };


  