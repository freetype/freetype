/*******************************************************************
 *
 *  t1tokens.c
 *
 *  Type 1 tokenizer
 *
 *  Copyright 1996 David Turner, Robert Wilhelm and Werner Lemberg.
 *
 *  This file is part of the FreeType project, and may only be used
 *  modified and distributed under the terms of the FreeType project
 *  license, LICENSE.TXT. By continuing to use, modify or distribute
 *  this file you indicate that you have read the license and
 *  understand and accept it fully.
 *
 *
 *  The tokenizer is in charge of loading and reading a Type1 font
 *  file (either in PFB or PFA format), and extract successive tokens
 *  and keywords from its two streams (i.e. the font program, and the
 *  private dictionary).
 *
 *  Eexec decryption is performed automatically when entering the
 *  private dictionary, or when retrieving char strings..
 *
 ******************************************************************/

#include <ftstream.h>
#include <ftdebug.h>

#include <t1tokens.h>
#include <t1load.h>

#undef  READ_BUFFER_INCREMENT
#define READ_BUFFER_INCREMENT  0x400

#undef  FT_COMPONENT
#define FT_COMPONENT  trace_t1load

  /* array of Type1 keywords supported by this engine. This table places */
  /* the keyword in lexicographical order. It should always correspond   */
  /* to the enums key_XXXX !!                                            */
  /*                                                                     */
  const  char*  t1_keywords[ key_max - key_first_ ] =
  {
    "-|", "ExpertEncoding", "ND", "NP", "RD", "StandardEncoding", "array",
	"begin", "closefile", "currentdict", "currentfile", "def", "dict", "dup",
	"eexec", "end", "executeonly", "false", "for", "index",	"noaccess",
	"put", "readonly", "true", "userdict", "|", "|-"
  };



  const  char*  t1_immediates[ imm_max - imm_first_ ] =
  {
    "-|", ".notdef", "BlueFuzz", "BlueScale", "BlueShift", "BlueValues",
    "CharStrings", "Encoding", "FamilyBlues", "FamilyName", "FamilyOtherBlues",
    "FID", "FontBBox", "FontID", "FontInfo", "FontMatrix", "FontName",
    "FontType", "ForceBold", "FullName", "ItalicAngle", "LanguageGroup",
    "Metrics", "MinFeature", "ND", "NP", "Notice", "OtherBlues", "OtherSubrs",
    "PaintType", "Private", "RD", "RndStemUp", "StdHW", "StdVW", "StemSnapH",
    "StemSnapV", "StrokeWidth", "Subrs", "UnderlinePosition",
    "UnderlineThickness", "UniqueID", "Weight", "isFixedPitch", "lenIV",
    "password", "version", "|", "|-"
  };


  /* lexicographic comparison of two strings */
  static
  int  lexico_strcmp( const char*  str1,
                      int          str1_len,
                      const char*  str2 )
  {
    int  c2 = 0;

    for ( ; str1_len > 0; str1_len-- )
    {
      int c1, diff;

      c1 = *str1++;
      c2 = *str2++;

      diff = c1 - c2;
      if (diff) return diff;
    };
    return -*str2;
  }


  /* Find a given token/name, perform binary search */
  static
  int  Find_Name( char*  base, int  length,
                  const char** table, int  table_len )
  {
    /* performs a binary search */

    T1_Int  left, right;

    left  = 0;
    right = table_len-1;

    while ( right-left > 1 )
    {
      T1_Int  middle = left + (( right-left ) >> 1);
      T1_Int  cmp;

      cmp = lexico_strcmp( base, length, table[middle] );
      if (!cmp) return middle;

      if ( cmp < 0 )
        right = middle;
      else
        left  = middle;
    }

    if ( !lexico_strcmp( base, length, table[left ] ) ) return left;
    if ( !lexico_strcmp( base, length, table[right] ) ) return right;

    return -1;
  }


  /* read the small PFB section header */
  static
  T1_Error  Read_PFB_Tag( FT_Stream  stream,
                          T1_UShort* atag,
                          T1_ULong*  asize )
  {
    T1_UShort tag;
    T1_ULong  size;
    T1_Error  error;

    FT_TRACE2(( "Read_PFB_Tag : reading\n" ));

    if ( ACCESS_Frame( 6L ) ) return error;

    tag  = GET_UShort();
    size = GET_ULong();

    FORGET_Frame();

    *atag  = tag;
    *asize = (  (size        & 0xFF) << 24 ) |
             ( ((size >> 8)  & 0xFF) << 16 ) |
             ( ((size >> 16) & 0xFF) << 8 )  |
             ( ((size >> 24) & 0xFF) );

    FT_TRACE2(( "  tag  = %04x\n", tag    ));
    FT_TRACE4(( "  asze = %08x\n", size   ));
    FT_TRACE2(( "  size = %08x\n", *asize ));

    return T1_Err_Ok;
  }



  static
  T1_Error  grow( T1_Tokenizer  tokzer )
  {
    T1_Error   error;
    T1_Long    left_bytes;
    FT_Memory  memory = tokzer->memory;

    left_bytes = tokzer->max - tokzer->limit;

    if ( left_bytes > 0 )
    {
      FT_Stream stream = tokzer->stream;

      if ( left_bytes > READ_BUFFER_INCREMENT )
        left_bytes = READ_BUFFER_INCREMENT;

      FT_TRACE2(( "Growing tokenizer buffer by %d bytes\n", left_bytes ));

      (void)stream;  /* unused in non reentrant mode */

      if ( !REALLOC( tokzer->base, tokzer->limit, 
                     tokzer->limit + left_bytes )                 &&
           !FILE_Read( tokzer->base + tokzer->limit, left_bytes ) )
        tokzer->limit += left_bytes;
    }
    else
    {
      FT_ERROR(( "Unexpected end of Type1 fragment !!\n" ));
      error = T1_Err_Invalid_File_Format;
    }

    tokzer->error = error;
    return error;
  }




  LOCAL_FUNC
  void  t1_decrypt( T1_Byte*   buffer,
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


 /*************************************************************************/
 /*                                                                       */
 /*  <Function> New_Tokenizer                                             */
 /*                                                                       */
 /*  <Description>                                                        */
 /*     Creates a new tokenizer from a given input stream. This function  */
 /*     automatically recognizes "pfa" or "pfb" files. The function       */
 /*     "Read_Token" can then be used to extract successive tokens from   */
 /*     the stream..                                                      */
 /*                                                                       */
 /*  <Input>                                                              */
 /*     stream  :: input stream                                           */
 /*                                                                       */
 /*  <Output>                                                             */
 /*     tokenizer :: handle to new tokenizer object..                     */
 /*                                                                       */
 /*  <Return>                                                             */
 /*     Type1 error code. 0 means success..                               */
 /*                                                                       */
 /*  <Note>                                                               */
 /*     This function copies the stream handle within the object. Callers */
 /*     should not discard "stream". This is done by the Done_Tokenizer   */
 /*     function..                                                        */
 /*                                                                       */

 LOCAL_FUNC
 T1_Error  New_Tokenizer( FT_Stream      stream,
                          T1_Tokenizer*  tokenizer )
 {
   FT_Memory     memory = stream->memory;
   T1_Tokenizer  tokzer;
   T1_Error      error;
   T1_UShort     tag;
   T1_ULong      size;

   T1_Byte*      tok_base;
   T1_ULong      tok_limit;
   T1_ULong      tok_max;

   *tokenizer = 0;

   /* allocate object */
   if ( FILE_Seek( 0L )                     ||
        ALLOC( tokzer, sizeof(*tokzer) ) )
     return error;

   tokzer->stream = stream;
   tokzer->memory = stream->memory;

   tokzer->in_pfb     = 0;
   tokzer->in_private = 0;

   tok_base  = 0;
   tok_limit = 0;
   tok_max   = stream->size;

   error = Read_PFB_Tag( stream, &tag, &size );
   if (error) goto Fail;

   if ( tag != 0x8001 )
   {
     /* assume that it is a PFA file - an error will be produced later */
     /* if a character with value > 127 is encountered..               */

     /* rewind to start of file */
     if ( FILE_Seek(0L) ) goto Fail;

     size = stream->size;
   }
   else
     tokzer->in_pfb = 1;

   /* if it's a memory-based resource, set up pointer */
   if ( !stream->read )
   {
     tok_base  = (T1_Byte*)stream->base + stream->pos;
     tok_limit = size;
     tok_max   = size;

     /* check that the "size" field is valid */
     if ( FILE_Skip(size) ) goto Fail;
   }
   else if ( tag == 0x8001 )
   {
     /* read segment in memory */
     if ( ALLOC( tok_base, size ) )
       goto Fail;

     if ( FILE_Read( tok_base, size ) )
     {
       FREE( tok_base );
       goto Fail;
     }

     tok_limit = size;
     tok_max   = size;
   }

   tokzer->base   = tok_base;
   tokzer->limit  = tok_limit;
   tokzer->max    = tok_max;
   tokzer->cursor = 0;

   *tokenizer = tokzer;
   
   /* Now check font format, we must see a '%!PS-AdobeFont-1' */
   /* or a '%!FontType'                                       */
   {
     if ( 16 > tokzer->limit )
       grow( tokzer );
       
     if ( tokzer->limit <= 16 ||
          ( strncmp( (const char*)tokzer->base, "%!PS-AdobeFont-1", 16 ) &&
            strncmp( (const char*)tokzer->base, "%!FontType", 10 )       ) )
     {
       FT_TRACE2(( "Not a Type1 font\n" ));
       error = T1_Err_Invalid_File_Format;
       goto Fail;
     }
   }
   return T1_Err_Ok;

 Fail:
   FREE( tokzer );
   return error;
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


 /*************************************************************************/
 /*                                                                       */
 /*  <Function> Done_Tokenizer                                            */
 /*                                                                       */
 /*  <Description>                                                        */
 /*     Closes a given tokenizer. This function will also close the       */
 /*     stream embedded in the object..                                   */
 /*                                                                       */
 /*  <Input>                                                              */
 /*     tokenizer :: target tokenizer object                              */
 /*                                                                       */
 /*  <Return>                                                             */
 /*     Type1 error code. 0 means success..                               */
 /*                                                                       */

 LOCAL_FUNC
 T1_Error  Done_Tokenizer( T1_Tokenizer  tokenizer )
 {
   FT_Memory  memory = tokenizer->memory;

   /* clear read buffer if needed (disk-based resources) */
   if ( tokenizer->in_private || !tokenizer->stream->base )
     FREE( tokenizer->base );

   FREE( tokenizer );
   return T1_Err_Ok;
 }



 /*************************************************************************/
 /*                                                                       */
 /*  <Function> Open_PrivateDict                                          */
 /*                                                                       */
 /*  <Description>                                                        */
 /*     This function must be called to set the tokenizer to the private  */
 /*     section of the Type1 file. It recognizes automatically the        */
 /*     the kind of eexec encryption used (ascii or binary)..             */
 /*                                                                       */
 /*  <Input>                                                              */
 /*     tokenizer :: target tokenizer object                              */
 /*     lenIV     :: value of the "lenIV" variable..                      */
 /*                                                                       */
 /*  <Return>                                                             */
 /*     Type1 error code. 0 means success..                               */
 /*                                                                       */

 LOCAL_FUNC
 T1_Error  Open_PrivateDict( T1_Tokenizer  tokenizer )
 {
   T1_Tokenizer  tokzer = tokenizer;
   FT_Stream     stream = tokzer->stream;
   FT_Memory     memory = tokzer->memory;
   T1_Error      error = 0;

   T1_UShort     tag;
   T1_ULong      size;

   T1_Byte*      private;

   /* are we already in the private dictionary ? */
   if ( tokzer->in_private )
     return 0;

   if ( tokzer->in_pfb )
   {
     /* in the case of the PFB format, the private dictionary can be  */
     /* made of several segments. We thus first read the number of    */
     /* segments to compute the total size of the private dictionary  */
     /* then re-read them into memory..                               */
     T1_Long  start_pos    = FILE_Pos();
     T1_ULong private_size = 0;

     do
     {
       error = Read_PFB_Tag( stream, &tag, &size );
       if (error || tag != 0x8002) break;

       private_size += size;

       if ( FILE_Skip(size) )
         goto Fail;
     }
     while (1);

     /* Check that we have a private dictionary there */
     /* and allocate private dictionary buffer        */
     if ( private_size == 0 )
     {
       FT_ERROR(( "T1.Open_Private: invalid private dictionary section\n" ));
       error = T1_Err_Invalid_File_Format;
       goto Fail;
     }

     if ( ALLOC( private, private_size ) )
       goto Fail;

     /* read all sections into buffer */
     if ( FILE_Seek( start_pos ) )
       goto Fail_Private;

     private_size = 0;
     do
     {
       error = Read_PFB_Tag( stream, &tag, &size );
       if (error || tag != 0x8002) { error = 0; break; }

       if ( FILE_Read( private + private_size, size ) )
         goto Fail_Private;

       private_size += size;
     }
     while (1);

     tokzer->base   = private;
     tokzer->cursor = 0;
     tokzer->limit  = private_size;
     tokzer->max    = private_size;
   }
   else
   {
     char*  base;

     /* we're in a PFA file, read each token until we find "eexec" */
     while ( tokzer->token.kind2 != key_eexec )
     {
       error = Read_Token( tokzer );
       if (error) goto Fail;
     }

     /* now determine wether the private dictionary is encoded in binary */
     /* or hexadecimal ASCII format..                                    */

     /* we need to access the next 4 bytes (after the final \r following */
     /* the 'eexec' keyword..) if they all are hexadecimal digits, then  */
     /*we have a case of ASCII storage..                                 */
     while ( tokzer->cursor+5 > tokzer->limit )
     {
       error = grow( tokzer );
       if (error) goto Fail;
     }

     /* skip whitespace/line feed after "eexec" */
     base = (char*)tokzer->base + tokzer->cursor + 1;
     if ( ( hexa_value( base[0] ) | hexa_value( base[1] ) |
            hexa_value( base[2] ) | hexa_value( base[3] ) ) < 0 )
     {
       /* binary encoding - "simply" read the stream */

       /* if it's a memory-based resource, we need to allocate a new */
       /* storage buffer for the private dictionary, as it needs to  */
       /* be decrypted later..                                       */
       if ( stream->base )
       {
         size = stream->size - tokzer->cursor-1; /* remaining bytes */

         if ( ALLOC( private, size ) )  /* allocate private dict buffer */
           goto Fail;

         /* copy eexec-encrypted bytes */
         MEM_Copy( private, tokzer->base + tokzer->cursor+1, size );

         /* reset pointers - forget about file mapping */
         tokzer->base   = private;
         tokzer->limit  = size;
         tokzer->max    = size;
         tokzer->cursor = 0;
       }
       /* on the opposite, for disk based resources, we simply grow  */
       /* the current buffer until its completion, and decrypt the   */
       /* bytes within it. In all cases, the "base" buffer will be   */
       /* discarded on DoneTokenizer if we're in the private dict..  */
       else
       {
         /* grow the read buffer to the full file.. */
         while ( tokzer->limit < tokzer->max )
         {
           error = grow( tokenizer );
           if (error) goto Fail;
         }

         /* set up cursor to first encrypted byte */
         tokzer->cursor++;
       }
     }
     else
     {
       /* ASCII hexadecimal encoding.. This sucks.. */
       T1_Byte*  write;
       T1_Byte*  cur;
       T1_Byte*  limit;
       T1_Int    count;

       /* Allocate a buffer, read each one byte at a time .. */
       count = ( stream->size - tokzer->cursor );
       size  = count/2;

       if ( ALLOC( private, size ) )   /* allocate private dict buffer */
         goto Fail;

       write = private;
       cur   = tokzer->base + tokzer->cursor;
       limit = tokzer->base + tokzer->limit;

       /* read each bytes */
       while ( count > 0 )
       {
         /* ensure that we can read the next 2 bytes !! */
         while ( cur+2 > limit )
         {
           int  cursor = cur - tokzer->base;
           error = grow( tokzer );
           if (error) goto Fail_Private;
           cur   = tokzer->base + cursor;
           limit = tokzer->base + tokzer->limit;
         }

         /* check for new line */
         if ( cur[0] == '\r' || cur[0] == '\n' )
         {
           cur++;
           count--;
         }
         else
         {
           int  hex1 = hexa_value(cur[0]);

           /* exit if we have a non hexa-decimal digit which isn't */
           /* a new-line character..                               */
           if (hex1 < 0)
             break;

           /* otherwise, store byte */
           *write++ = ( hex1 << 4 ) | hexa_value( cur[1] );
           cur += 2;
           count -= 2;
         }
       }

       /* get rid of old buffer in the case of disk-based resources */
       if ( !stream->base )
         FREE( tokzer->base );

       /* set up pointers */
       tokzer->base   = private;
       tokzer->limit  = size;
       tokzer->max    = size;
       tokzer->cursor = 0;
     }
   }

   /* finally, decrypt the private dictionary - and skip the lenIV bytes */
   t1_decrypt( tokzer->base, tokzer->limit, 55665 );
   tokzer->cursor += 4;

 Fail:
   return error;

 Fail_Private:
   FREE( private );
   goto Fail;
 }


 /*************************************************************************/
 /*                                                                       */
 /*  <Function> Read_Token                                                */
 /*                                                                       */
 /*  <Description>                                                        */
 /*     Read a new token from the current input stream. This function     */
 /*     extracts a token from the font program until "Open_PrivateDict"   */
 /*     has been called. After this, it returns tokens from the           */
 /*     (eexec-encrypted) private dictionnary..                           */
 /*                                                                       */
 /*  <Input>                                                              */
 /*     tokenizer :: target tokenizer object                              */
 /*                                                                       */
 /*  <Return>                                                             */
 /*     Type1 error code. 0 means success..                               */
 /*                                                                       */
 /*  <Note>                                                               */
 /*     One should use the function Read_CharStrings to read the binary   */
 /*     charstrings from the private dict..                               */
 /*                                                                       */
 LOCAL_FUNC
 T1_Error  Read_Token( T1_Tokenizer  tokenizer )
 {
   T1_Tokenizer tok = tokenizer;
   T1_Long      cur, limit;
   T1_Byte*     base;
   char         c, starter, ender;
   T1_Bool      token_started;

   T1_TokenType  kind;

   tok->error      = T1_Err_Ok;
   tok->token.kind = tok_any;

   base  = tok->base;
   limit = tok->limit;
   cur   = tok->cursor;

   token_started = 0;

   for (;;)
   {
     if ( cur >= limit )
     {
       if ( grow( tok ) ) goto Exit;
       base  = tok->base;
       limit = tok->limit;
     }

     c = (char)base[cur++];

     /* check that we have an ASCII character */
     if ( (T1_Byte)c > 127 )
     {
       FT_ERROR(( "Unexpected binary data in Type1 fragment !!\n" ));
       tok->error = T1_Err_Invalid_File_Format;
       goto Exit;
     }

     switch (c)
     {
       case '\r' :
       case '\n' :
       case ' '  :
       case '\t' : /* skip initial whitespace => skip to next */
         if (token_started)
         {
           /* possibly a name, keyword, wathever */
           tok->token.kind = tok_any;
           tok->token.len  = cur-tok->token.start-1;
           goto Exit;
         }
         /* otherwise, skip everything */
         break;


       case '%' : /* this is a comment - skip everything */
         for (;;)
         {
           T1_Int  left = limit - cur;

           while (left > 0)
           {
             c = (char)base[cur++];
             if ( c == '\r' || c == '\n' )
               goto Next;
             left--;
           }
           if ( grow( tokenizer ) ) goto Exit;
           base  = tok->base;
           limit = tok->limit;
         }


       case '(' : /* a Postscript string */
         kind  = tok_string;
         ender = ')';

       L1:
         if (!token_started)
         {
           token_started    = 1;
           tok->token.start = cur-1;
         }

         {
           T1_Int  nest_level = 1;

           starter = c;
           for (;;)
           {
             T1_Int  left = limit-cur;
             while (left > 0)
             {
               c = (char)base[cur++];

               if ( c == starter )
                 nest_level++;

               else if ( c == ender )
               {
                 nest_level--;
                 if (nest_level <= 0)
                 {
                   tok->token.kind = kind;
                   tok->token.len  = cur - tok->token.start;
                   goto Exit;
                 }
               }
               left--;
             }

             if ( grow( tok ) ) goto Exit;
             base  = tok->base;
             limit = tok->limit;
           }
         }


     case '[' : /* a Postscript array */
       if (token_started)
         goto Any_Token;

       kind  = tok_array;
       ender = ']';
       goto L1;
       break;


     case '{' : /* a Postscript program */
       if (token_started)
         goto Any_Token;

       kind  = tok_program;
       ender = '}';
       goto L1;
       break;


     case '<' : /* a Postscript hex byte array ?? */
       if (token_started)
         goto Any_Token;

       kind  = tok_hexarray;
       ender = '>';
       goto L1;
       break;


     case '0':  /* any number */
     case '1':
     case '2':
     case '3':
     case '4':
     case '5':
     case '6':
     case '7':
     case '8':
     case '9':
       if (token_started)
         goto Next;

       tok->token.kind = tok_number;
       token_started    = 1;
       tok->token.start = cur-1;
     L2:
       for (;;)
       {
         T1_Int  left = limit-cur;
         while (left > 0)
         {
           c = (char)base[cur++];

           switch (c)
           {
             case '[':
             case '{':
             case '(':
             case '<':
             case '/':
               goto Any_Token;           

             case  ' ':
             case '\r':
             case '\t':
             case '\n':
               tok->token.len = cur - tok->token.start - 1;
               goto Exit;

             default:
               ;
           }
           left--;
         }
         if (grow( tok )) goto Exit;
         base  = tok->base;
         limit = tok->limit;
       }


     case '.':   /* maybe a number */
     case '-':
     case '+':
       if (token_started)
         goto Next;
         
       token_started    = 1;
       tok->token.start = cur-1;
       for (;;)
       {
         T1_Int  left = limit-cur;
         if ( left > 0 )
         {
           /* test for any following digit, interpreted as number */
           c = (char)base[cur];
           tok->token.kind = ( c >= '0' && c <= '9' ? tok_number : tok_any );
           goto L2;
         }
         if (grow( tok )) goto Exit;
         base  = tok->base;
         limit = tok->limit;
       }

     case '/':  /* maybe an immediate name */
       if (!token_started)
       {
         token_started    = 1;
         tok->token.start = cur-1;

         for (;;)
         {
           T1_Int  left = limit-cur;
           if ( left > 0 )
           {
             /* test for single '/', interpreted as garbage */
             c = (char)base[cur];
             tok->token.kind = ( c == ' '  || c == '\t' ||
                                 c == '\r' || c == '\n' ?
                                 tok_any : tok_immediate );
             goto L2;
           }
           if (grow( tok )) goto Exit;
           base  = tok->base;
           limit = tok->limit;
         }
       }
       else
       {
   Any_Token:        /* possibly a name or wathever */
         cur--;
         tok->token.len = cur - tok->token.start;
         goto Exit;
       }

     default:
       if (!token_started)
       {
         token_started    = 1;
         tok->token.start = cur-1;
       }
     }

 Next:
   ;
   }

 Exit:
   tok->cursor = cur;

   if (!tok->error)
   {
     /* now, tries to match keywords and immediate names */
     T1_Int  index;

     switch ( tok->token.kind )
     {
       case tok_immediate :  /* immediate name */
         index = Find_Name( (char*)(tok->base + tok->token.start+1),
                            tok->token.len-1,
                            t1_immediates,
                            imm_max - imm_first_ );
         tok->token.kind2 = ( index >= 0 ? imm_first_ + index : 0 );
         break;


       case tok_any : /* test for keyword */
         index = Find_Name( (char*)(tok->base + tok->token.start),
                	    tok->token.len,
                            t1_keywords,
                            key_max - key_first_ );
         if ( index >= 0 )
         {
           tok->token.kind  = tok_keyword;
           tok->token.kind2 = key_first_ + index;
         }
         else
           tok->token.kind2 = 0;
         break;

       default:
         tok->token.kind2 = 0;
     }
   }
   return tokenizer->error;
 }


#if 0
 /*************************************************************************/
 /*                                                                       */
 /*  <Function> Read_CharStrings                                          */
 /*                                                                       */
 /*  <Description>                                                        */
 /*     Read a charstrings from the current input stream. These are       */
 /*     binary bytes that encode each individual glyph outline.           */
 /*                                                                       */
 /*     The caller is responsible for skipping the "lenIV" bytes at       */
 /*     the start of the record..                                         */
 /*                                                                       */
 /*  <Input>                                                              */
 /*     tokenizer :: target tokenizer object                              */
 /*     num_chars :: number of binary bytes to read                       */
 /*                                                                       */
 /*  <Output>                                                             */
 /*     buffer    :: target array of bytes. These are eexec-decrypted..   */
 /*                                                                       */
 /*  <Return>                                                             */
 /*     Type1 error code. 0 means success..                               */
 /*                                                                       */
 /*  <Note>                                                               */
 /*     One should use the function Read_CharStrings to read the binary   */
 /*     charstrings from the private dict..                               */
 /*                                                                       */
 LOCAL_FUNC
 T1_Error  Read_CharStrings( T1_Tokenizer  tokenizer,
                             T1_Int        num_chars,
                             T1_Byte*      buffer )
 {
   for (;;)
   {
     T1_Int  left = tokenizer->limit - tokenizer->cursor;

     if ( left >= num_chars )
     {
       MEM_Copy( buffer, tokenizer->base + tokenizer->cursor, num_chars );
       t1_decrypt( buffer, num_chars, 4330 );
       tokenizer->cursor += num_chars;
       return T1_Err_Ok;
     }

     if ( grow(tokenizer) ) return tokenizer->error;
   }
 }
#endif
