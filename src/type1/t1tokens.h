/*******************************************************************
 *
 *  t1tokens.h
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
 *  The tokenizer is in charge of loading and reading a Type1 font
 *  file (either in PFB or PFA format), and extract successive tokens
 *  and keywords from its two streams (i.e. the font program, and the
 *  private dictionary).
 *
 *  Eexec decryption is performed automatically when entering the
 *  private dictionary, or when retrieving char strings..
 *
 ******************************************************************/

#ifndef T1TOKENS_H
#define T1TOKENS_H

#include <t1objs.h>

/* enum value of first keyword */
#define key_first_     100

/* enum value of first immediate name */
#define imm_first_     200

  typedef  enum T1_TokenType_
  {
    tok_error = 0,

    tok_eof,             /* end of file              */

    /* simple token types */

    tok_keyword,         /* keyword                      */
    tok_number,          /* number (integer or real)     */
    tok_string,          /* postscript string            */
    tok_program,         /* postscript program           */
    tok_immediate,       /* any immediate name           */
    tok_array,           /* matrix, array, etc..         */
    tok_hexarray,        /* array of hexadecimal nibbles */
    tok_any,             /* anything else                */

    /* Postscript keywords - placed in lexicographical order */

    key_RD_alternate = key_first_,      /* "-|" = alternate form of RD */
	key_ExpertEncoding,
    key_ND,
    key_NP,
    key_RD,
	key_StandardEncoding,
    key_array,
    key_begin,
    key_closefile,
    key_currentdict,
    key_currentfile,
    key_def,
    key_dict,
    key_dup,
    key_eexec,
    key_end,
    key_execonly,
    key_false,
    key_for,
    key_index,
    key_noaccess,
    key_put,
    key_readonly,
    key_true,
    key_userdict,
    key_NP_alternate,                   /* "|" = alternate form of NP  */
    key_ND_alternate,                   /* "|-" = alternate form of ND */

    key_max,   /* always keep this value there */

    /* Postscript immediate names - other names will be ignored, except */
    /* in charstrings..                                                 */

    imm_RD_alternate = imm_first_,      /* "-|" = alternate form of RD */
    imm_notdef,                         /* "/.notdef" immediate        */
    imm_BlueFuzz,
    imm_BlueScale,
    imm_BlueShift,
    imm_BlueValues,
    imm_CharStrings,
    imm_Encoding,
    imm_FamilyBlues,
    imm_FamilyName,
    imm_FamilyOtherBlues,
    imm_FID,
    imm_FontBBox,
    imm_FontID,
    imm_FontInfo,
    imm_FontMatrix,
    imm_FontName,
    imm_FontType,
    imm_ForceBold,
    imm_FullName,
    imm_ItalicAngle,
    imm_LanguageGroup,
    imm_Metrics,
    imm_MinFeature,
    imm_ND,
    imm_NP,
    imm_Notice,
    imm_OtherBlues,
    imm_OtherSubrs,
    imm_PaintType,
    imm_Private,
    imm_RD,
    imm_RndStemUp,
    imm_StdHW,
    imm_StdVW,
    imm_StemSnapH,
    imm_StemSnapV,
    imm_StrokeWidth,
    imm_Subrs,
    imm_UnderlinePosition,
    imm_UnderlineThickness,
    imm_UniqueID,
    imm_Weight,

    imm_isFixedPitch,
    imm_lenIV,
    imm_password,
    imm_version,

    imm_NP_alternate,                   /* "|"  = alternate form of NP  */
    imm_ND_alternate,                   /* "|-" = alternate form of ND  */

    imm_max   /* always keep this value here */

  } T1_TokenType;


  /* these arrays are visible for debugging purposes.. */
  extern const  char*  t1_keywords[];
  extern const  char*  t1_immediates[];


 /*************************************************************************/
 /*                                                                       */
 /*  <Struct> T1_Token                                                    */
 /*                                                                       */
 /*  <Description>                                                        */
 /*     A structure used to describe a token in the current input         */
 /*     stream. Note that the Type1 driver doesn't try to interpret       */
 /*     tokens until it really needs to..                                 */
 /*                                                                       */
 /*  <Fields>                                                             */
 /*     kind  :: token type. Describes the token to the loader            */
 /*     kind2 :: detailed token type.                                     */
 /*                                                                       */
 /*     start ::  index of first character of token in input stream       */
 /*                                                                       */
 /*     len   ::  length of token in characters.                          */
 /*                                                                       */
  typedef struct T1_Token_
  {
    T1_TokenType   kind;     /* simple type                    */
    T1_TokenType   kind2;    /* detailed type                  */
    T1_Int         start;    /* index of first token character */
    T1_Int         len;      /* length of token in chars       */

  } T1_Token;




  typedef  struct  T1_TokenParser_
  {
    FT_Memory   memory;
    FT_Stream   stream;

    T1_Bool     in_pfb;      /* true if PFB file, PFA otherwise */
    T1_Bool     in_private;  /* true if in private dictionary   */

    T1_Byte*    base;        /* base address of current read buffer */
    T1_Long     cursor;      /* current position in read buffer     */
    T1_Long     limit;       /* limit of current read buffer        */
    T1_Long     max;         /* maximum size of read buffer         */

    T1_Error    error;       /* last error                          */
    T1_Token    token;       /* last token read                     */

  } T1_TokenParser;



 /*************************************************************************/
 /*                                                                       */
 /*  <Type> T1_Tokenizer                                                  */
 /*                                                                       */
 /*  <Description>                                                        */
 /*     A handle to an object used to extract tokens from the input.      */
 /*     The object is able to perform PFA/PFB recognition, eexec          */
 /*     decryption of the private dictionary, as well as eexec decryption */
 /*     of the charstrings..                                              */
 /*                                                                       */
  typedef  T1_TokenParser*    T1_Tokenizer;


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
 LOCAL_DEF
 T1_Error  New_Tokenizer( FT_Stream      stream,
                          T1_Tokenizer*  tokenizer );



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
 LOCAL_DEF
 T1_Error  Done_Tokenizer( T1_Tokenizer  tokenizer );



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
 LOCAL_DEF
 T1_Error  Open_PrivateDict( T1_Tokenizer  tokenizer );



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
 LOCAL_DEF
 T1_Error  Read_Token( T1_Tokenizer  tokenizer );


#if 0
 /*************************************************************************/
 /*                                                                       */
 /*  <Function> Read_CharStrings                                          */
 /*                                                                       */
 /*  <Description>                                                        */
 /*     Read a charstrings from the current input stream. These are       */
 /*     binary bytes that encode each individual glyph outline.           */
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
 LOCAL_DEF
 T1_Error  Read_CharStrings( T1_Tokenizer  tokenizer,
                             T1_Int        num_chars,
                             T1_Byte*      buffer );
#endif

 /*************************************************************************/
 /*                                                                       */
 /*  <Function> t1_decrypt                                                */
 /*                                                                       */
 /*  <Description>                                                        */
 /*     Performs the Type 1 charstring decryption process..               */
 /*                                                                       */
 /*  <Input>                                                              */
 /*     buffer  :: base address of data to decrypt                        */
 /*     length  :: number of bytes to decrypt from base address           */
 /*     seed    :: ecnryption seed (4330 for charstrings).                */
 /*                                                                       */
  LOCAL_DEF
  void  t1_decrypt( T1_Byte*   buffer,
                    T1_Int     length,
                    T1_UShort  seed );

#endif /* T1TOKENS_H */
