/*******************************************************************
 *
 *  ftextend.h                                                   2.0
 *
 *    Extensions Interface.
 *
 *  Copyright 1996-1999 by
 *  David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 *  This file is part of the FreeType project, and may only be used
 *  modified and distributed under the terms of the FreeType project
 *  license, LICENSE.TXT.  By continuing to use, modify, or distribute
 *  this file you indicate that you have read the license and
 *  understand and accept it fully.
 *
 ******************************************************************/

#ifndef FTEXTEND_H
#define FTEXTEND_H

#include "ftconfig.h"
#include "ftobjs.h"


#ifdef __cplusplus
  extern "C" {
#endif

  /* The extensions don't need to be integrated at compile time into */
  /* the engine, only at link time.                                  */


 /*****************************************************************
  *
  * <FuncType>
  *   FT_Extension_Initializer
  *
  * <Description>
  *   Each new face object can have several extensions associated
  *   to it at creation time. This function is used to initialize
  *   a given extension data for a given face.
  *
  * <Input>
  *   ext   :: a typeless pointer to the extension data.
  *   face  :: handle to the source face object the extension is
  *            associated with
  *
  * <Return>
  *   Error condition. 0 means success
  *
  * <Note>
  *   In case of error, the initializer should not destroy the
  *   extension data, as the finalizer will get called later
  *   by the function's caller..
  *
  *****************************************************************/

  typedef FT_Error (*FT_Extension_Initializer)( void*    ext, 
                                                FT_Face  face );

 /*****************************************************************
  *
  * <FuncType>
  *   FT_Extension_Finalizer
  *
  * <Description>
  *   Each new face object can have several extensions associated
  *   to it at creation time. This function is used to finalize
  *   a given extension data for a given face. This occurs before
  *   the face object itself is finalized.
  *
  * <Input>
  *   ext   :: a typeless pointer to the extension data.
  *   face  :: handle to the source face object the extension is
  *            associated with
  *
  *****************************************************************/

  typedef void  (*FT_Extension_Finalizer)( void*  ext, FT_Face  face );


 /*****************************************************************
  *
  * <Struct>
  *   FT_Extension_Class
  *
  * <Description>
  *   A simple structure used to describe a given extension to
  *   the FreeType base layer. An FT_Extension_Class is used as
  *   a parameter for FT_Register_Extension.
  *
  * <Fields>
  *   id        :: the extension's id. This is a normal C string
  *                that is used to uniquely reference the extension's
  *                interface.
  *
  *   size      :: the size in bytes of the extension data that must
  *                be associated with each face object.
  *
  *   init      :: a pointer to the extension data's initializer
  *   finalize  :: a pointer to the extension data's finalizer
  *
  *   inteface  :: this pointer can be anything, but should usually
  *                point to a table of function pointers which implement
  *                the extension's interface.
  *
  *   offset    :: this field is set and used within the base layer
  *                and should be set to 0 when registering an
  *                extension through FT_Register_Extension. It contains
  *                an offset within the face's extension block for the
  *                current extension's data.
  *
  *****************************************************************/

  typedef struct FT_Extension_Class_
  {
    const char*               id;
    FT_ULong                  size;
    FT_Extension_Initializer  init;
    FT_Extension_Finalizer    finalize;
    void*                     interface;
    
    FT_ULong                  offset;
    
  } FT_Extension_Class;
  

  EXPORT_DEF
  FT_Error  FT_Register_Extension( FT_Driver            driver,
                                   FT_Extension_Class*  clazz );


#ifdef FT_CONFIG_OPTION_EXTEND_ENGINE
  /* Initialize the extension component */
  LOCAL_DEF
  FT_Error  FT_Init_Extensions( FT_Library  library );

  /* Finalize the extension component */
  LOCAL_DEF
  FT_Error  FT_Done_Extensions( FT_Library  library );

  /* Create an extension within a face object.  Called by the */
  /* face object constructor.                                 */
  LOCAL_DEF
  FT_Error  FT_Create_Extensions( FT_Face  face );

  /* Destroy all extensions within a face object.  Called by the */
  /* face object destructor.                                     */
  LOCAL_DEF
  FT_Error  FT_Destroy_Extensions( FT_Face  face );
#endif

  /* Returns an extension's data & inteface according to its ID */
  EXPORT_DEF
  void*     FT_Get_Extension( FT_Face      face,
                              const char*  extension_id,
                              void*       *extension_interface );
#ifdef __cplusplus
  }
#endif


#endif /* FTEXTEND_H */


/* END */
