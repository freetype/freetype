/***************************************************************************/
/*                                                                         */
/*  ttload.h                                                               */
/*                                                                         */
/*    Load the basic TrueType tables, i.e., tables that can be either in   */
/*    TTF or OTF font (specification).                                     */
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


#ifndef TTLOAD_H
#define TTLOAD_H


#include <ftstream.h>
#include <tttypes.h>
/*
#include <ttobjs.h>
*/

#ifdef __cplusplus
  extern "C" {
#endif


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_LookUp_Table                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Looks for a TrueType table by name.                                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face :: A face object handle.                                      */
  /*    tag  :: The  searched tag.                                         */
  /*                                                                       */
  /* <Return>                                                              */
  /*    pointer to table directory entry. 0 if not found..                 */
  /*                                                                       */
  EXPORT_DEF
  TT_Table*  TT_LookUp_Table( TT_Face   face,
                              TT_ULong  tag );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Goto_Table                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Looks for a TrueType table by name, then seek a stream to it.      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face   :: a face object handle.                                    */
  /*    tag    :: the  searched tag.                                       */
  /*    stream :: the stream to seek when the table is found               */
  /*                                                                       */
  /* <Return>                                                              */
  /*    pointer to table directory entry. 0 if not found..                 */
  /*                                                                       */
  EXPORT_DEF
  TT_Error   TT_Goto_Table( TT_Face    face,
                            TT_ULong   tag,
                            FT_Stream  stream,
                            TT_ULong  *length );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_Directory                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads the table directory into a face object.                      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face      :: A handle to the target face object.                   */
  /*    stream    :: The input stream.                                     */
  /*    faceIndex :: The index of the TrueType font, if we're opening a    */
  /*                 collection.                                           */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The stream cursor must be at the font file's origin                */
  /*                                                                       */
  LOCAL_DEF
  TT_Error  TT_Load_Directory( TT_Face    face,
                               FT_Stream  stream,
                               TT_Long    faceIndex );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_Any                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads any font table into client memory.  Used by the              */
  /*    TT_Get_Font_Data() API function.                                   */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face   :: The face object to look for.                             */
  /*                                                                       */
  /*    tag    :: The tag of table to load.  Use the value 0 if you  want  */
  /*              to access the whole font file, else set this parameter   */
  /*              to a valid TrueType table tag that you can forge with    */
  /*              the MAKE_TT_TAG macro.                                   */
  /*                                                                       */
  /*    offset :: The starting offset in the table (or the file if         */
  /*              tag == 0).                                               */
  /*                                                                       */
  /*    length :: The address of the decision variable:                    */
  /*                                                                       */
  /*                If length == NULL:                                     */
  /*                  Loads the whole table.  Returns an error if          */
  /*                  `offset' == 0!                                       */
  /*                                                                       */
  /*                If *length == 0:                                       */
  /*                  Exits immediately; returning the length of the given */
  /*                  table or of the font file, depending on the value of */
  /*                  `tag'.                                               */
  /*                                                                       */
  /*                If *length != 0:                                       */
  /*                  Loads the next `length' bytes of table or font,      */
  /*                  starting at offset `offset' (in table or font too).  */
  /*                                                                       */
  /* <Output>                                                              */
  /*    buffer :: The address of target buffer.                            */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_DEF
  TT_Error  TT_Load_Any( TT_Face   face,
                         TT_ULong  tag,
                         TT_Long   offset,
                         void*     buffer,
                         TT_Long*  length );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_Header                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads the TrueType font header.                                    */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face   :: A handle to the target face object.                      */
  /*    stream :: The input stream.                                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_DEF
  TT_Error  TT_Load_Header( TT_Face    face,
                            FT_Stream  stream );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_Metrics_Header                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads the horizontal or vertical header in a face object.          */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face     :: A handle to the target face object.                    */
  /*    stream   :: The input stream.                                      */
  /*    vertical :: A boolean flag.  If set, load vertical metrics.        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_DEF
  TT_Error  TT_Load_Metrics_Header( TT_Face    face,
                                    FT_Stream  stream,
                                    TT_Bool    vertical );


  LOCAL_DEF
  TT_Error  TT_Load_CMap( TT_Face    face,
                          FT_Stream  stream );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_MaxProfile                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads the maximum profile into a face object.                      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face   :: A handle to the target face object.                      */
  /*    stream :: The input stream.                                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_DEF
  TT_Error  TT_Load_MaxProfile( TT_Face    face,
                                FT_Stream  stream );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_Names                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads the name records.                                            */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face   :: A handle to the target face object.                      */
  /*    stream :: The input stream.                                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_DEF
  TT_Error  TT_Load_Names( TT_Face    face,
                           FT_Stream  stream );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_OS2                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads the OS2 table.                                               */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face   :: A handle to the target face object.                      */
  /*    stream :: A handle to the input stream.                            */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_DEF
  TT_Error  TT_Load_OS2( TT_Face    face,
                         FT_Stream  stream );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_Postscript                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads the Postscript table.                                        */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face   :: A handle to the target face object.                      */
  /*    stream :: A handle to the input stream.                            */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_DEF
  TT_Error  TT_Load_PostScript( TT_Face    face,
                                FT_Stream  stream );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_Hdmx                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads the horizontal device metrics table.                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face   :: A handle to the target face object.                      */
  /*    stream :: A handle to the input stream.                            */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_DEF
  TT_Error  TT_Load_Hdmx( TT_Face    face,
                          FT_Stream  stream );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Free_Names                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Frees the name records.                                            */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face :: A handle to the target face object.                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_DEF
  void  TT_Free_Names( TT_Face  face );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Free_Hdmx                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Frees the horizontal device metrics table.                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face :: A handle to the target face object.                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_DEF
  void  TT_Free_Hdmx ( TT_Face  face );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_Kern                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads the first kerning table with format 0 in the font.  Only     */
  /*    accepts the first horizontal kerning table.  Developers should use */
  /*    the `ftxkern' extension to access other kerning tables in the font */
  /*    file, if they really want to.                                      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face   :: A handle to the target face object.                      */
  /*    stream :: The input stream.                                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_DEF
  TT_Error  TT_Load_Kern( TT_Face    face,
                          FT_Stream  stream );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_Gasp                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads the `GASP' table into a face object.                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face   :: A handle to the target face object.                      */
  /*    stream :: The input stream.                                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_DEF
  TT_Error  TT_Load_Gasp( TT_Face    face,
                          FT_Stream  stream );


#endif /* TTLOAD_H */


/* END */
