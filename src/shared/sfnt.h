/***************************************************************************/
/*                                                                         */
/*  sfnt.h                                                                 */
/*                                                                         */
/*    High-level `sfnt' driver interface (specification).                  */
/*                                                                         */
/*  Copyright 1996-2000 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef SFNT_H
#define SFNT_H

#include <freetype.h>
#include <tttypes.h>


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    TT_Load_Format_Tag                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads the first 4 bytes of the font file. This is a tag that       */
  /*    identifies the font format used.                                   */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face      :: A handle to the target face object.                   */
  /*    stream    :: The input stream.                                     */
  /*    faceIndex :: The index of the TrueType font, if we're opening a    */
  /*                 collection.                                           */
  /* <Output>                                                              */
  /*    format_tag :: a 4-byte tag                                         */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The stream cursor must be at the font file's origin                */
  /*    This function recognizes fonts embedded in a "TrueType collection" */
  /*                                                                       */
  typedef
  TT_Error  (*TT_Load_Format_Tag_Func)( TT_Face    face,
                                        FT_Stream  stream,
                                        TT_Long    faceIndex,
                                        TT_ULong  *format_tag );

  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    TT_Load_Directory_Func                                             */
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
  /*    The stream cursor must be on the first byte after the 4-byte       */
  /*    font format tag. This is the case just after a call to             */
  /*    TT_Load_Format_Tag                                                 */
  /*                                                                       */
  typedef
  TT_Error  (*TT_Load_Directory_Func)( TT_Face    face,
                                       FT_Stream  stream,
                                       TT_Long    faceIndex );


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    TT_Load_Any_Func                                                   */
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
  typedef
  TT_Error  (*TT_Load_Any_Func)( TT_Face   face,
                                 TT_ULong  tag,
                                 TT_Long   offset,
                                 void*     buffer,
                                 TT_Long*  length );


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    TT_Load_SBit_Image_Func                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads a given glyph sbit image from the font resource.  This also  */
  /*    returns its metrics.                                               */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face        :: The target face object.                             */
  /*                                                                       */
  /*    x_ppem      :: The horizontal resolution in points per EM.         */
  /*                                                                       */
  /*    y_ppem      :: The vertical resolution in points per EM.           */
  /*                                                                       */
  /*    glyph_index :: The current glyph index.                            */
  /*                                                                       */
  /*    stream      :: The input stream.                                   */
  /*                                                                       */
  /* <Output>                                                              */
  /*    map         :: The target pixmap.                                  */
  /*    metrics     :: A big sbit metrics structure for the glyph image.   */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.  Returns an error if no     */
  /*    glyph sbit exists for the index.                                   */
  /*                                                                       */
  /*  <Note>                                                               */
  /*    The `map.buffer' field is always freed before the glyph is loaded. */
  /*                                                                       */
  typedef  
  TT_Error  (*TT_Load_SBit_Image_Func)( TT_Face           face,
                                        TT_Int            x_ppem,
                                        TT_Int            y_ppem,
                                        TT_UInt           glyph_index,
                                        FT_Stream         stream,
                                        FT_Bitmap*        map,
                                        TT_SBit_Metrics*  metrics );

  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    TT_Get_PS_Name_Func                                                */
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
  typedef
  TT_Error (*TT_Get_PS_Name_Func)( TT_Face      face,
                                   TT_UInt      index,
                                   TT_String**  PSname );


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    TT_Load_Metrics                                                    */
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
  typedef
  TT_Error  (*TT_Load_Metrics_Func)( TT_Face    face,
                                     FT_Stream  stream,
                                     TT_Bool    vertical );



  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    TT_CharMap_Load_Func                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads a given TrueType character map into memory.                  */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face   :: A handle to the parent face object.                      */
  /*    stream :: A handle to the current stream object.                   */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    cmap   :: A pointer to a cmap object.                              */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The function assumes that the stream is already in use (i.e.,      */
  /*    opened).  In case of error, all partially allocated tables are     */
  /*    released.                                                          */
  /*                                                                       */
  typedef
  TT_Error  (*TT_CharMap_Load_Func)( TT_Face        face,
                                     TT_CMapTable*  cmap,
                                     FT_Stream      input );


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    TT_CharMap_Free_Func                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Destroys a character mapping table.                                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face :: A handle to the parent face object.                        */
  /*    cmap :: A handle to a cmap object.                                 */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  typedef
  TT_Error  (*TT_CharMap_Free_Func)( TT_Face        face,
                                     TT_CMapTable*  cmap );


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    TT_Load_Table                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads a given TrueType table.                                      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face   :: A handle to the target face object.                      */
  /*    stream :: The input stream.                                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The function will use `face->goto_table' to seek the stream to     */
  /*    the start of the table                                             */
  /*                                                                       */
  typedef
  TT_Error  (*TT_Load_Table_Func)( TT_Face    face,
                                   FT_Stream  stream );


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    TT_Load_Table                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads a given TrueType table.                                      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face   :: A handle to the target face object.                      */
  /*    stream :: The input stream.                                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The function will use `face->goto_table' to seek the stream to     */
  /*    the start of the table                                             */
  /*                                                                       */
  typedef
  void  (*TT_Free_Table_Func)( TT_Face  face );



  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    SFNT_Interface                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    this structure holds pointers to the functions used to load and    */
  /*    free the basic tables that are required in a `sfnt' font file.     */
  /*                                                                       */
  /* <Fields>                                                              */
  /*                                                                       */
  /*                                                                       */
  /*                                                                       */
  /*                                                                       */
  /*                                                                       */
  /*                                                                       */
  /*                                                                       */
  /*                                                                       */
  typedef struct  SFNT_Interface_
  {
    TT_Goto_Table_Func      goto_table;
  
    TT_Load_Any_Func        load_any;
    TT_Load_Format_Tag_Func load_format_tag;
    TT_Load_Directory_Func  load_directory;

    TT_Load_Table_Func      load_header;
    TT_Load_Metrics_Func    load_metrics;
    TT_Load_Table_Func      load_charmaps;
    TT_Load_Table_Func      load_max_profile;
    TT_Load_Table_Func      load_os2;
    TT_Load_Table_Func      load_psnames;

    TT_Load_Table_Func      load_names;
    TT_Free_Table_Func      free_names;

    /* optional tables */
    TT_Load_Table_Func      load_hdmx;
    TT_Free_Table_Func      free_hdmx;

    TT_Load_Table_Func      load_kerning;
    TT_Load_Table_Func      load_gasp;


    /* see `ttsbit.h' */
    TT_Load_Table_Func      load_sbits;
    TT_Load_SBit_Image_Func load_sbit_image;
    TT_Free_Table_Func      free_sbits;
    
    /* see `ttpost.h' */
    TT_Get_PS_Name_Func     get_psname;
    TT_Free_Table_Func      free_psnames;

    /* see `ttcmap.h' */
    TT_CharMap_Load_Func    load_charmap;
    TT_CharMap_Free_Func    free_charmap;
    
  } SFNT_Interface;

#endif /* SFNT_H */


/* END */
