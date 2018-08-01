/****************************************************************************
 *
 * tfm.h
 *
 *   Auxiliary functions and data structures related to TFM metric files
 *   (specification).
 *
 * Copyright 1996-2018 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef TFM_H_
#define TFM_H_


#include <ft2build.h>
#include FT_INTERNAL_OBJECTS_H
#include FT_INTERNAL_HASH_H
#include FT_INTERNAL_TRUETYPE_TYPES_H



FT_BEGIN_HEADER


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /***                                                                   ***/
  /***                                                                   ***/
  /***                TFM FONT INFORMATION STRUCTURES                    ***/
  /***                                                                   ***/
  /***                                                                   ***/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

  typedef struct  TFM_FontInfoRec_
  {
    /* Font Info */
    FT_Long        cs; /* Check Sum */
    /* Metrics */
    FT_ULong       ds, fs; /* Design Size */
    FT_UInt        design_size;
    FT_UInt        slant;
    FT_UInt        begin_char, end_char;
    FT_Long        *width, *height, *depth;
    /* Font bounding box */
    FT_UInt        font_bbx_w, font_bbx_h;
    FT_UInt        font_bbx_xoff, font_bbx_yoff;

  } TFM_FontInfoRec, *TFM_FontInfo;

  #define RDS2PT(rds)    (tfm->design_size * ((FT_Long)(rds)/(FT_Long)(1<<20)))

  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                            TFM PARSER                         *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  typedef struct TFM_ParserRec_*  TFM_Parser;

  /**************************************************************************
   *
   * @struct:
   *   TFM_ParserRec
   *
   * @description:
   *   An TFM_Parser is a parser for the TFM files.
   *
   * @fields:
   *   memory ::
   *     The object used for memory operations (alloc and
   *     realloc).
   *
   *   stream ::
   *     This is an FT_Stream object.
   *
   *   FontInfo ::
   *     The result will be stored here.
   */
  typedef struct  TFM_ParserRec_
  {
    FT_Memory     memory;
    FT_Stream     stream;

    TFM_FontInfo  FontInfo;

    void*         user_data;  /* To be checked for compatibility */

  } TFM_ParserRec;


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                        TFM Module Interface                   *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  typedef struct  TFM_ServiceRec_
  {
    FT_Error
    (*init)( TFM_Parser  parser,
             FT_Memory   memory,
             FT_Stream   stream );

    FT_Error
    (*parse_metrics)( TFM_Parser  parser );

    void
    (*done)( TFM_Parser  parser );

  } TFM_ServiceRec, *TFM_Service;

  /* backward compatible type definition */
  typedef TFM_ServiceRec   TFM_Interface;


FT_END_HEADER

#endif /* PSAUX_H_ */


/* END */
