/*******************************************************************
 *
 *  t1parse.c                                                   2.0
 *
 *    Type1 parser.
 *
 *  Copyright 1996-1998 by
 *  David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 *  This file is part of the FreeType project, and may only be used
 *  modified and distributed under the terms of the FreeType project
 *  license, LICENSE.TXT.  By continuing to use, modify, or distribute
 *  this file you indicate that you have read the license and
 *  understand and accept it fully.
 *
 *  The Type 1 parser is in charge of the following:
 *
 *   - provide an implementation of a growing sequence of
 *     objects called a T1_Table (used to build various tables
 *     needed by the loader).
 *
 *   - opening .pfb and .pfa files to extract their top-level
 *     and private dictionaries
 *
 *   - read numbers, arrays & strings from any dictionary
 *
 *  See "t1load.c" to see how data is loaded from the font file
 *
 ******************************************************************/
 
#include <ftdebug.h>
#include <ftcalc.h>
#include <ftobjs.h>
#include <ftstream.h>
#include <t1errors.h>
#include <t1parse.h>

#undef FT_COMPONENT
#define FT_COMPONENT  trace_t1load

/*************************************************************************/
/*                                                                       */
/* <Function> T1_New_Table                                               */
/*                                                                       */
/* <Description>                                                         */
/*    Initialise a T1_Table.                                             */
/*                                                                       */
/* <Input>                                                               */
/*    table  :: address of target table                                  */
/*    count  :: table size = maximum number of elements                  */
/*    memory :: memory object to use for all subsequent reallocations    */
/*                                                                       */
/* <Return>                                                              */
/*    Error code. 0 means success                                        */
/*                                                                       */

  LOCAL_FUNC
  T1_Error  T1_New_Table( T1_Table*  table,
                          T1_Int     count,
                          FT_Memory  memory )
  {
	 T1_Error  error;

	 table->memory = memory;
	 if ( ALLOC_ARRAY( table->elements, count, T1_Byte*  ) ||
          ALLOC_ARRAY( table->lengths, count, T1_Byte* ) )
       goto Exit;

	table->max_elems = count;
    table->init      = 0xdeadbeef;
	table->num_elems = 0;
	table->block     = 0;
	table->capacity  = 0;
	table->cursor    = 0;

  Exit:
    if (error) FREE(table->elements);
      
	return error;
  }



/*************************************************************************/
/*                                                                       */
/* <Function> T1_Add_Table                                               */
/*                                                                       */
/* <Description>                                                         */
/*    Adds an object to a T1_Table, possibly growing its memory block    */
/*                                                                       */
/* <Input>                                                               */
/*    table  :: target table                                             */
/*    index  :: index of object in table                                 */
/*    object :: address of object to copy in memory                      */
/*    length :: length in bytes of source object                         */
/*                                                                       */
/* <Return>                                                              */
/*    Error code. 0 means success. An error is returned when a           */
/*    realloc failed..                                                   */
/*                                                                       */


      static void  shift_elements( T1_Table*  table, T1_Byte*  old_base )
      {
        T1_Long    delta  = table->block - old_base;
        T1_Byte**  offset = table->elements;
        T1_Byte**  limit  = offset + table->max_elems;
    
        if (delta)
          for ( ; offset < limit; offset++ )
          {
            if (offset[0])
              offset[0] += delta;
          }
      }
  
      static
      T1_Error  reallocate_t1_table( T1_Table*  table,
                                     T1_Int     new_size )
      {
        FT_Memory  memory   = table->memory;
        T1_Byte*   old_base = table->block;
        T1_Error   error;
  
        /* realloc the base block */
        if ( REALLOC( table->block, table->capacity, new_size ) )
          return error;
  
        table->capacity = new_size;
  
        /* shift all offsets when needed */
        if (old_base)
          shift_elements( table, old_base );
  
        return T1_Err_Ok;
      }



  LOCAL_FUNC
  T1_Error  T1_Add_Table( T1_Table*  table,
                          T1_Int     index,
                          void*      object,
                          T1_Int     length )
  {
	if (index < 0 || index > table->max_elems)
    {
	  FT_ERROR(( "T1.Add_Table: invalid index\n" ));
	  return T1_Err_Syntax_Error;
    }

    /* grow the base block if needed */
    if ( table->cursor + length > table->capacity )
    {
      T1_Error  error;
      T1_Int    new_size = table->capacity;

      while ( new_size < table->cursor+length )
        new_size += 1024;

      error = reallocate_t1_table( table, new_size );
      if (error) return error;
    }

    /* add the object to the base block and adjust offset */
    table->elements[ index ] = table->block + table->cursor;
    table->lengths [ index ] = length;
    MEM_Copy( table->block + table->cursor, object, length );

    table->cursor += length;
    return T1_Err_Ok;
  }


/*************************************************************************/
/*                                                                       */
/* <Function> T1_Done_Table                                              */
/*                                                                       */
/* <Description>                                                         */
/*    Finalise a T1_Table. (realloc it to its current cursor).           */
/*                                                                       */
/* <Input>                                                               */
/*    table :: target table                                              */
/*                                                                       */
/* <Note>                                                                */
/*    This function does NOT release the heap's memory block. It is up   */
/*    to the caller to clean it, or reference it in its own structures.  */
/*                                                                       */
#if 0
  LOCAL_FUNC
  void  T1_Done_Table( T1_Table*  table )
  {
    FT_Memory  memory = table->memory;
    T1_Error   error;
    T1_Byte*   old_base;

    /* should never fail, as rec.cursor <= rec.size */
    old_base = table->block;
    if (!old_base)
      return;
    
    (void)REALLOC( table->block, table->capacity, table->cursor );
    table->capacity = table->cursor;
    
    if (old_base != table->block)
      shift_elements( table, old_base );
  }
#endif

  LOCAL_FUNC
  void  T1_Release_Table( T1_Table*  table )
  {
    FT_Memory  memory = table->memory;
    
    if (table->init == (FT_Long)0xdeadbeef)
    {
      FREE( table->block );
      FREE( table->elements );
      FREE( table->lengths );
      table->init = 0;
    }
  }

  static
  T1_Long  t1_toint( T1_Byte* *cursor,
                     T1_Byte*  limit )
  {
    T1_Long  result = 0;
    T1_Byte* cur    = *cursor;
    T1_Byte  c, d;
    
    for (; cur < limit; cur++)
    {
      c = *cur;
      d = (T1_Byte)(c - '0');
      if (d < 10) break;
      
      if ( c=='-' )
      {
        cur++;
        break;
      }
    }
    
    if (cur < limit)
    {
      do
      {
        d = (T1_Byte)(cur[0] - '0');
        if (d >= 10)
          break;
          
        result = result*10 + d;
        cur++;
        
      } while (cur < limit);
      
      if (c == '-')
        result = -result;
    }
    
    *cursor = cur;
    return result;
  }


  static
  T1_Long  t1_tofixed( T1_Byte* *cursor,
                       T1_Byte*  limit,
                       T1_Long   power_ten )
  {
    T1_Byte* cur    = *cursor;
    T1_Long  num, divider, result;
    T1_Int   sign   = 0;
    T1_Byte  d;
    
    if (cur >= limit) return 0;
    
    /* first of all, read the integer part */
    result  = t1_toint( &cur, limit ) << 16;
    num     = 0;
    divider = 1;
    
    if (result < 0)
    {
      sign   = 1;
      result = -result;
    }
    if (cur >= limit) goto Exit;
    
    /* read decimal part, if any */
    if (*cur == '.' && cur+1 < limit)
    {
      cur++;
      
      for (;;)
      {
        d = (T1_Byte)(*cur - '0');
        if (d >= 10) break;

        if (divider < 10000000L)
        {
          num      = num*10 + d;
          divider *= 10;
        }
        cur++;
        if (cur >= limit) break;
      }
    }
    
    /* read exponent, if any */
    if ( cur+1 < limit && (*cur == 'e' || *cur == 'E'))
    {
      cur++;
      power_ten += t1_toint( &cur, limit );
    }
    
  Exit:
    /* raise to power of ten if needed */
    while (power_ten > 0)
    {
      result = result*10;
      num    = num*10;
      power_ten--;
    }
    
    while (power_ten < 0)
    {
      result  = result/10;
      divider = divider*10;
      power_ten++;
    }

    if (num)
      result += FT_DivFix( num, divider );

    if (sign)
      result = -result;
      
    *cursor = cur;
    return result;
  }


  static
  T1_Int  t1_tocoordarray( T1_Byte*  *cursor,
                           T1_Byte*   limit,
                           T1_Int     max_coords,
                           T1_Short*  coords )
  {
    T1_Byte*  cur   = *cursor;
    T1_Int    count = 0;
    T1_Byte   c, ender;
    
    if (cur >= limit) goto Exit;
    
    /* check for the beginning of an array. If not, only one number will be read */
    c     = *cur;
    ender = 0;
    
    if (c == '[')
      ender = ']';
      
    if (c == '{')
      ender = '}';
      
    if (ender)
      cur++;

    /* now, read the coordinates */
    for ( ; cur < limit; )
    {
      /* skip whitespace in front of data */
      for (;;)
      {
        c = *cur;
        if ( c != ' ' && c != '\t' ) break;
        
        cur++;
        if (cur >= limit) goto Exit;
      }
      
      if (count >= max_coords || c == ender)
        break;
      
      coords[count] = (T1_Short)(t1_tofixed(&cur,limit,0) >> 16);
      count++;
      
      if (!ender)
        break;
    }
    
  Exit:
    *cursor = cur;
    return count;
  }



  static
  T1_Int  t1_tofixedarray( T1_Byte*  *cursor,
                           T1_Byte*   limit, 
                           T1_Int     max_values,
                           T1_Fixed*  values,
                           T1_Int     power_ten )
  {
    T1_Byte*  cur   = *cursor;
    T1_Int    count = 0;
    T1_Byte   c, ender;
    
    if (cur >= limit) goto Exit;
    
    /* check for the beginning of an array. If not, only one number will be read */
    c     = *cur;
    ender = 0;
    
    if (c == '[')
      ender = ']';
      
    if (c == '{')
      ender = '}';
      
    if (ender)
      cur++;

    /* now, read the values */
    for ( ; cur < limit; )
    {
      /* skip whitespace in front of data */
      for (;;)
      {
        c = *cur;
        if ( c != ' ' && c != '\t' ) break;
        
        cur++;
        if (cur >= limit) goto Exit;
      }

      if (count >= max_values || c == ender)
        break;
      
      values[count] = t1_tofixed(&cur,limit,power_ten);
      count++;
      
      if (!ender)
        break;
    }
    
  Exit:
    *cursor = cur;
    return count;
  }


  static
  T1_String*  t1_tostring( T1_Byte* *cursor, T1_Byte* limit, FT_Memory memory )
  {
    T1_Byte*    cur = *cursor;
    T1_Int      len = 0;
    T1_Int      count;
    T1_String*  result;
    FT_Error    error;

    /* XXX : some stupid fonts have a "Notice" or "Copyright" string     */
    /*       that simply doesn't begin with an opening parenthesis, even */
    /*       though they have a closing one !!! E.g. "amuncial.pfb"      */
    /*                                                                   */
    /*       We must deal with these ill-fated cases there. Note that    */
    /*       these fonts didn't work with the old Type 1 driver as the   */
    /*       notice/copyright was not recognized as a valid string token */
    /*       and made the old token parser commit errors..               */

    while ( cur < limit && (*cur == ' ' || *cur == '\t')) cur++;
    if (cur+1 >= limit) return 0;
    
    if (*cur == '(') cur++;  /* skip the opening parenthesis, if there is one */
    
    *cursor = cur;
    count   = 0;
    
    /* then, count its length */
    for ( ; cur < limit; cur++ )
    {
      if (*cur == '(')
        count++;
        
      else if (*cur == ')')
      {
        count--;
        if (count < 0)
          break;
      }
    }

    len = cur - *cursor;    
    if (cur >= limit || ALLOC(result,len+1)) return 0;
    
    /* now copy the string */
    MEM_Copy( result, *cursor, len );
    result[len] = '\0';
    *cursor = cur;
    return result;    
  }

  static
  int  t1_tobool( T1_Byte* *cursor, T1_Byte* limit )
  {
    T1_Byte*  cur    = *cursor;
    T1_Bool   result = 0;
    
    /* return 1 if we find a "true", 0 otherwise */
    if ( cur+3 < limit &&
         cur[0] == 't' &&
         cur[1] == 'r' &&
         cur[2] == 'u' &&
         cur[3] == 'e' )
    {
      result = 1;
      cur   += 5;
    }
    else if ( cur+4 < limit &&
              cur[0] == 'f' &&
              cur[1] == 'a' &&
              cur[2] == 'l' &&
              cur[3] == 's' &&
              cur[4] == 'e' )
    {
      result = 0;
      cur   += 6;
    }
    *cursor = cur;
    return result;
  }


  LOCAL_FUNC
  T1_Long  T1_ToInt  ( T1_Parser*  parser )
  {
    return t1_toint( &parser->cursor, parser->limit );
  }


  LOCAL_FUNC
  T1_Long  T1_ToFixed( T1_Parser*  parser, T1_Int power_ten )
  {
    return t1_tofixed( &parser->cursor, parser->limit, power_ten );
  }


  LOCAL_FUNC
  T1_Int  T1_ToCoordArray( T1_Parser* parser,
                           T1_Int     max_coords,
                           T1_Short*  coords )
  {
    return t1_tocoordarray( &parser->cursor, parser->limit, max_coords, coords );
  }


  LOCAL_FUNC
  T1_Int  T1_ToFixedArray( T1_Parser* parser,
                           T1_Int     max_values,
                           T1_Fixed*  values,
                           T1_Int     power_ten )
  {
    return t1_tofixedarray( &parser->cursor, parser->limit, max_values, values, power_ten );
  }


  LOCAL_FUNC
  T1_String*  T1_ToString( T1_Parser* parser )
  {
    return t1_tostring( &parser->cursor, parser->limit, parser->memory );
  }


  LOCAL_FUNC
  T1_Bool   T1_ToBool( T1_Parser* parser )
  {
    return t1_tobool( &parser->cursor, parser->limit );
  }

  static
  FT_Error  read_pfb_tag( FT_Stream  stream, T1_UShort *tag, T1_Long*  size )
  {
    FT_Error  error;
    
    if (READ_UShort(*tag)) goto Exit;
    if (*tag == 0x8001 || *tag == 0x8002)
    {
      FT_Long  asize;
      
      if (READ_ULong(asize)) goto Exit;
      
      /* swap between big and little endianness */
      *size  = ((asize & 0xFF000000) >> 24) |
               ((asize & 0x00FF0000) >> 8 ) |
               ((asize & 0x0000FF00) << 8 ) |
               ((asize & 0x000000FF) << 24);
    }
    
  Exit:
    return error;
  }



  LOCAL_FUNC
  T1_Error  T1_New_Parser( T1_Parser*  parser,
                           FT_Stream   stream,
                           FT_Memory   memory )
  {
    FT_Error  error;
    T1_UShort tag;
    T1_Long   size;
  
    parser->stream       = stream;
    parser->memory       = memory;
    parser->base_len     = 0;
    parser->base_dict    = 0;
    parser->private_len  = 0;
    parser->private_dict = 0;
    parser->in_pfb       = 0;
    parser->in_memory    = 0;
    parser->single_block = 0;
    
    parser->cursor       = 0;
    parser->limit        = 0;
    
    /******************************************************************/
    /*                                                                */
    /* Here's a short summary of what is going on :                   */
    /*                                                                */
    /*   When creating a new Type 1 parser, we try to locate and      */
    /*   load the base dictionary when this is possible (i.e. for     */
    /*   .pfb files). Otherwise, we load the whole font in memory.    */
    /*                                                                */
    /*   When "loading" the base dictionary, we only setup pointers   */
    /*   in the case of a memory-based stream. Otherwise, we allocate */
    /*   and load the base dict in it.                                */
    /*                                                                */
    /*   parser->in_pfb is set when we are in a binary (".pfb") font  */
    /*   parser->in_memory is set when we have a memory stream.       */
    /*                                                                */
    
    /* try to compute the size of the base dictionary    */
    /* look for a Postscript binary file tag, i.e 0x8001 */
    if ( FILE_Seek(0L) )
      goto Exit;
      
    error = read_pfb_tag( stream, &tag, &size );
    if (error) goto Exit;

    if (tag != 0x8001)
    {
      /* assume that this is a PFA file for now, an error will */
      /* be produced later when more things are checked        */
      (void)FILE_Seek(0L);
      size = stream->size;
    }
    else
      parser->in_pfb = 1;

    /* now, try to load the "size" bytes of the "base" dictionary we */
    /* found previously                                              */
 
    /* if it's a memory-based resource, set up pointers */
    if ( !stream->read )
    {
      parser->base_dict = (T1_Byte*)stream->base + stream->pos;
      parser->base_len  = size;
      parser->in_memory = 1;
 
      /* check that the "size" field is valid */
      if ( FILE_Skip(size) ) goto Exit;
    }
    else
    {
      /* read segment in memory */
      if ( ALLOC( parser->base_dict, size )     ||
           FILE_Read( parser->base_dict, size ) )
        goto Exit;
      parser->base_len = size;
    }
 
    /* Now check font format, we must see a '%!PS-AdobeFont-1' */
    /* or a '%!FontType'                                       */
    {
      if ( size <= 16 ||
           ( strncmp( (const char*)parser->base_dict, "%!PS-AdobeFont-1", 16 ) &&
             strncmp( (const char*)parser->base_dict, "%!FontType", 10 )       ) )
      {
        FT_TRACE2(( "Not a Type1 font\n" ));
        error = T1_Err_Invalid_File_Format;
      }
      else
      {
        parser->cursor = parser->base_dict;
        parser->limit  = parser->cursor + parser->base_len;
      }
    }

  Exit:
    if (error && !parser->in_memory)
      FREE( parser->base_dict );

    return error;
  }


  LOCAL_FUNC
  void  T1_Done_Parser( T1_Parser*  parser )
  {
    FT_Memory   memory = parser->memory;

    /* always free the private dictionary */
    FREE( parser->private_dict );

    /* free the base dictionary only when we have a disk stream */
    if (!parser->in_memory)
      FREE( parser->base_dict );
  }


 /* return the value of an hexadecimal digit */
 static
 int  hexa_value( char c )
 {
   unsigned int  d;

   d = (unsigned int)(c-'0');
   if ( d <= 9 ) return (int)d;

   d = (unsigned int)(c-'a');
   if ( d <= 5 ) return (int)(d+10);

   d = (unsigned int)(c-'A');
   if ( d <= 5 ) return (int)(d+10);

   return -1;
 }


  LOCAL_FUNC
  void  T1_Decrypt( T1_Byte*   buffer,
                    T1_Int     length,
                    T1_UShort  seed )
  {
    while ( length > 0 )
    {
      T1_Byte  plain;

      plain     = (*buffer ^ (seed >> 8));
      seed      = (*buffer+seed)*52845+22719;
      *buffer++ = plain;
      length--;
    }
  }


  LOCAL_FUNC
  T1_Error  T1_Get_Private_Dict( T1_Parser*  parser )
  {
    FT_Stream  stream = parser->stream;
    FT_Memory  memory = parser->memory;
    FT_Error   error  = 0;
    T1_Long    size;
    
    if (parser->in_pfb)
    {
      /* in the case of the PFB format, the private dictionary can be  */
      /* made of several segments. We thus first read the number of    */
      /* segments to compute the total size of the private dictionary  */
      /* then re-read them into memory..                               */
      T1_Long    start_pos    = FILE_Pos();
      T1_UShort  tag;
      T1_Long    size;

      parser->private_len = 0;      
      for (;;)
      {
        error = read_pfb_tag(stream, &tag, &size);
        if (error) goto Fail;
        
        if (tag != 0x8002)
          break;
          
        parser->private_len += size;

        if ( FILE_Skip(size) )
          goto Fail;
      }

      /* Check that we have a private dictionary there */
      /* and allocate private dictionary buffer        */
      if ( parser->private_len == 0 )
      {
        FT_ERROR(( "T1.Open_Private: invalid private dictionary section\n" ));
        error = T1_Err_Invalid_File_Format;
        goto Fail;
      }

      if ( FILE_Seek( start_pos )                             ||
           ALLOC( parser->private_dict, parser->private_len ) )
        goto Fail;

      parser->private_len = 0;
      for (;;)
      {
        error = read_pfb_tag( stream, &tag, &size );
        if (error || tag != 0x8002) { error = 0; break; }

        if ( FILE_Read( parser->private_dict + parser->private_len, size ) )
          goto Fail;

        parser->private_len += size;
      }
    }
    else
    {
      /* we have already "loaded" the whole PFA font file in memory */
      /* if this is a memory resource, allocate a new block to hold */
      /* the private dict. Otherwise, simply overwrite into the     */
      /* base dict block in the heap..                              */

      /* first of all, look at the "eexec" keyword */
      FT_Byte*  cur   = parser->base_dict;
      FT_Byte*  limit = cur + parser->base_len;      
      FT_Byte   c;
      
      for (;;)
      {
        c = cur[0];
        if (c == 'e' && cur+9 < limit)  /* 9 = 5 letters for 'eexec' + newline + 4 chars */
        {
          if ( cur[1] == 'e' && cur[2] == 'x' &&
               cur[3] == 'e' && cur[4] == 'c' )
          {
            cur += 6; /* we skip the newling after the "eexec" */
            
            /* XXX: Some fonts use DOS-linefeeds, i.e. \r\n, we need to skip */
            /*      the extra \n when we find it..                           */
            if (cur[0] == '\n')
              cur++;
              
            break;
          }
        }
        cur++;
        if (cur >= limit)
        {
          FT_ERROR(("T1.Open_Private: could not find 'eexec' keyword\n"));
          error = FT_Err_Invalid_File_Format;
          goto Exit;
        }
      }
      
      /* now determine wether where to write the _encrypted_ binary private    */
      /* dictionary. We overwrite the base dictionary for disk-based resources */
      /* and allocate a new block otherwise                                    */
      
      size = parser->base_len - (cur-parser->base_dict); 
      
      if ( parser->in_memory )
      {
        /* note that we allocate one more byte to put a terminating '0' */
        if (ALLOC( parser->private_dict, size+1 )) goto Fail;
        parser->private_len = size;
      }
      else
      {
        parser->single_block = 1;        
        parser->private_dict = parser->base_dict;
        parser->private_len  = size;
        parser->base_dict    = 0;
        parser->base_len     = 0;
      }
      
      /* now determine wether the private dictionary is encoded in binary */
      /* or hexadecimal ASCII format..                                    */
      /* and decode it accordingly                                        */

      /* we need to access the next 4 bytes (after the final \r following */
      /* the 'eexec' keyword..) if they all are hexadecimal digits, then  */
      /*we have a case of ASCII storage..                                 */

      if ( ( hexa_value( cur[0] ) | hexa_value( cur[1] ) |
             hexa_value( cur[2] ) | hexa_value( cur[3] ) ) < 0 )
      {
       /* binary encoding - "simply" copy the private dict */
       MEM_Copy( parser->private_dict, cur, size );
      }
      else
      {      
        /* ASCII hexadecimal encoding.. This blows goats !!.. */
       
        T1_Byte*  write;
        T1_Int    count;

        write = parser->private_dict;
        count = 0;
       
        for ( ;cur < limit; cur++)
        {
          int  hex1;
         
          /* check for newline */
          if (cur[0] == '\r' || cur[0] == '\n')
            continue;
           
          /* exit if we have a non-hexadecimal digit that isn't a newline */
          hex1 = hexa_value(cur[0]);
          if (hex1 < 0 || cur+1 >= limit)
            break;
         
          /* otherwise, store byte */
          *write++ = (hex1 << 4) | hexa_value(cur[1]);
          count++;
          cur++;
        }
       
        /* put a safeguard */
        parser->private_len = write - parser->private_dict;
        *write++ = 0;
      }
    }
   
    /* we now decrypt the encoded binary private dictionary */
    T1_Decrypt( parser->private_dict, parser->private_len, 55665 );
    parser->cursor = parser->private_dict;
    parser->limit  = parser->cursor + parser->private_len;

  Fail:
  Exit:
    return error;
  }
  
