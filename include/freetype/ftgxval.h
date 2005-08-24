/***************************************************************************/
/*                                                                         */
/*  ftgxval.h                                                              */
/*                                                                         */
/*    FreeType API for validating TrueTypeGX/AAT tables (specification).   */
/*                                                                         */
/*  Copyright 2004 by                                                      */
/*  Masatake YAMATO, Redhat K.K,                                           */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

/***************************************************************************/
/* gxvalid is derived from both gxlayout module and otvalid module.        */
/* Development of gxlayout is support of Information-technology Promotion  */
/* Agency(IPA), Japan.                                                     */
/***************************************************************************/


#ifndef __FTGXVAL_H__
#define __FTGXVAL_H__

#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef FREETYPE_H
#error "freetype.h of FreeType 1 has been loaded!"
#error "Please fix the directory search order for header files"
#error "so that freetype.h of FreeType 2 is found first."
#endif


FT_BEGIN_HEADER


  /*************************************************************************/
  /*                                                                       */
  /* <Section>                                                             */
  /*    gx_validation                                                      */
  /*                                                                       */
  /* <Title>                                                               */
  /*    TrueTypeGX/AAT Validation                                          */
  /*                                                                       */
  /* <Abstract>                                                            */
  /*    An API to validate TrueTypeGX/AAT tables.                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This section contains the declaration of functions to validate     */
  /*    some TrueTypeGX tables (feat, mort, morx, bsln, just, kern, opbd,  */
  /*    trak, prop, lcar).                                                 */
  /*                                                                       */
  /*************************************************************************/

/***************************************************************************/
/*                                                                         */
/*                                                                         */
/* Warnings: Use FT_VALIDATE_XXX to validate a table.                      */
/*           Following definitions are for gxvalid developers.             */
/*                                                                         */
/*                                                                         */
/***************************************************************************/
#define FT_VALIDATE_feat_INDEX     0
#define FT_VALIDATE_mort_INDEX     1
#define FT_VALIDATE_morx_INDEX     2
#define FT_VALIDATE_bsln_INDEX     3
#define FT_VALIDATE_just_INDEX     4
#define FT_VALIDATE_kern_INDEX     5
#define FT_VALIDATE_opbd_INDEX     6
#define FT_VALIDATE_trak_INDEX     7
#define FT_VALIDATE_prop_INDEX     8
#define FT_VALIDATE_lcar_INDEX     9
#define FT_VALIDATE_GX_LAST_INDEX  FT_VALIDATE_lcar_INDEX
#define FT_VALIDATE_GX_LENGTH     (FT_VALIDATE_GX_LAST_INDEX + 1)

 /* Up to 0x1000 is used by otvalid.
    Ox2000 is reserved for feature ot extension. */
#define FT_VALIDATE_GX_START 0x4000
#define FT_VALIDATE_GX_BITFIELD(tag)  \
  (FT_VALIDATE_GX_START << FT_VALIDATE_##tag##_INDEX)


 /**********************************************************************
  *
  * @enum:
  *    FT_VALIDATE_GXXXX
  *
  * @description:
  *    A list of bit-field constants used with @FT_TrueTypeGX_Validate to
  *    indicate which TrueTypeGX/AAT Type tables should be validated.
  *
  * @values:
  *    FT_VALIDATE_feat ::
  *      Validate feat table.
  *
  * @values:
  *    FT_VALIDATE_mort ::
  *      Validate mort table.
  *
  * @values:
  *    FT_VALIDATE_morx ::
  *      Validate morx table.
  *
  * @values:
  *    FT_VALIDATE_bsln ::
  *      Validate bsln table.
  *
  * @values:
  *    FT_VALIDATE_just ::
  *      Validate just table.
  *
  * @values:
  *    FT_VALIDATE_kern ::
  *      Validate kern table.
  *
  * @values:
  *    FT_VALIDATE_opbd ::
  *      Validate opbd table.
  *
  * @values:
  *    FT_VALIDATE_trak ::
  *      Validate trak table.
  *
  * @values:
  *    FT_VALIDATE_prop ::
  *      Validate prop table.
  *
  * @values:
  *    FT_VALIDATE_lcar ::
  *      Validate lcar table.
  *
  * @values:
  *    FT_VALIDATE_GX ::
  *      Validate all TrueTypeGX tables (feat, mort, morx, bsln, just, kern,
  *      opbd, trak, prop and lcar).
  *
  */

#define FT_VALIDATE_feat  FT_VALIDATE_GX_BITFIELD(feat)
#define FT_VALIDATE_mort  FT_VALIDATE_GX_BITFIELD(mort)
#define FT_VALIDATE_morx  FT_VALIDATE_GX_BITFIELD(morx)
#define FT_VALIDATE_bsln  FT_VALIDATE_GX_BITFIELD(bsln)
#define FT_VALIDATE_just  FT_VALIDATE_GX_BITFIELD(just)
#define FT_VALIDATE_kern  FT_VALIDATE_GX_BITFIELD(kern)
#define FT_VALIDATE_opbd  FT_VALIDATE_GX_BITFIELD(opbd)
#define FT_VALIDATE_trak  FT_VALIDATE_GX_BITFIELD(trak)
#define FT_VALIDATE_prop  FT_VALIDATE_GX_BITFIELD(prop)
#define FT_VALIDATE_lcar  FT_VALIDATE_GX_BITFIELD(lcar)

#define FT_VALIDATE_GX  ( FT_VALIDATE_feat |     \
                          FT_VALIDATE_mort |     \
                          FT_VALIDATE_morx |     \
                          FT_VALIDATE_bsln |     \
                          FT_VALIDATE_just |     \
                          FT_VALIDATE_kern |     \
                          FT_VALIDATE_opbd |     \
                          FT_VALIDATE_trak |     \
                          FT_VALIDATE_prop |     \
                          FT_VALIDATE_lcar )


  /* */

 /**********************************************************************
  *
  * @function:
  *    FT_TrueTypeGX_Validate
  *
  * @description:
  *    Validate various TrueTypeGX tables to assure that all offsets and
  *    indices are valid.  The idea is that a higher-level library which
  *    actually does the text layout can access those tables without
  *    error checking (which can be quite time consuming).
  *
  * @input:
  *    face ::
  *       A handle to the input face.
  *
  *    validation_flags ::
  *       A bit field which specifies the tables to be validated.  See
  *       @FT_VALIDATE_GXXXX for possible values.
  *
  *    table_length ::
  *       The length of tables. Generally FT_VALIDATE_GX_LENGTH should
  *       be passed.
  *
  * @output
  *    tables ::
  *       The array where each validated sfnt tables are stored to.
  *       The array itself must be allocated by a client.
  *
  * @return:
  *   FreeType error code.  0 means success.
  *
  * @note:
  *   This function only works with TrueTypeGX fonts, returning an error
  *   otherwise.
  *
  *   After use, the application should deallocate the buffers pointed by each
  *   tables' element.  A NULL value indicates that the table either
  *   doesn't exist in the font, the application hasn't asked for validation, or
  *   the validator doesn't have ability to validate the sfnt table.
  */
  FT_EXPORT( FT_Error )
  FT_TrueTypeGX_Validate( FT_Face    face,
                          FT_UInt    validation_flags,
                          FT_Bytes   tables[FT_VALIDATE_GX_LENGTH],
                          FT_UInt    table_length );



  /* */

 /**********************************************************************
  *
  * @enum:
  *    FT_VALIDATE_CKERNXXX
  *
  * @description:
  *    A list of bit-field constants used with @FT_ClassicKern_Validate
  *    to indicate (a) classic kern dialect(s).
  *
  * @values:
  *    FT_VALIDATE_MS ::
  *      Validate the kern table as it has classic Microsoft kern dialect.
  *      If @FT_ClassicKern_Validate detects the table has the other
  *      dialect, it regards the table invalid.
  *
  * @values:
  *    FT_VALIDATE_APPLE ::
  *      Validate the kern table as it has classic Apple kern dialect.
  *      If @FT_ClassicKern_Validate detects the table has the other
  *      dialect, it regards the table invalid.
  *
  * @values:
  *    FT_VALIDATE_CKERN ::
  *      Validate the kern table as it has classic Apple kern dialect or
  *      Microsoft kern dialect.
  */
#define FT_VALIDATE_MS     (FT_VALIDATE_GX_START << 0)
#define FT_VALIDATE_APPLE  (FT_VALIDATE_GX_START << 1)

#define FT_VALIDATE_CKERN  ( FT_VALIDATE_MS | FT_VALIDATE_APPLE )


  /* */

 /**********************************************************************
  *
  * @function:
  *    FT_ClassicKern_Validate
  *
  * @description:
  *    Validate classic(16bit format) kern table to assure that the offsets
  *    and indices are valid.  The idea is that a higher-level library
  *    which actually does the text layout can access those tables without
  *    error checking (which can be quite time consuming).
  *
  *    Kern table validator in @FT_TrueTypeGX_Validate deals both
  *    new 32 bit format and classic 16 bit format. In other hand
  *    this function supports only the classic 16 bit format.
  *
  * @input:
  *    face ::
  *       A handle to the input face.
  *
  *    validation_flags ::
  *       A bit field which specifies the dialect to be validated.  See
  *       @FT_VALIDATE_CKERNXXX for possible values.
  *
  * @output
  *    ckern_table ::
  *       A pointer to the kern table.
  *
  * @return:
  *   FreeType error code.  0 means success.
  *
  * @note:
  *   After use, the application should deallocate the buffers pointed by
  *   ckern_table.  A NULL value indicates that the table either
  *   doesn't exist in the font.
  */
  FT_EXPORT( FT_Error )
  FT_ClassicKern_Validate( FT_Face   face,
                           FT_UInt   validation_flags,
                           FT_Bytes *ckern_table );


 /* */


FT_END_HEADER

#endif /* __FTGXVAL_H__ */


/* END */
