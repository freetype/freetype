/***************************************************************************/
/*                                                                         */
/*  otload.h                                                               */
/*                                                                         */
/*    OpenType layout loader functions (specification).                    */
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


#ifndef OTLOAD_H
#define OTLOAD_H

#include <otlayout.h>
#include <ftstream.h>


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    OTL_Load_Script_List                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads an OpenType Script List from a font resource.                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    list   :: The target script list.                                  */
  /*    stream :: The input stream.                                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_DEF
  TT_Error  OTL_Load_Script_List( OTL_Script_List*  list,
                                  FT_Stream         stream );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    OTL_Load_Feature_List                                              */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads an OpenType Feature List from a font resource.               */
  /*                                                                       */
  /* <Input>                                                               */
  /*    list   :: The target feature list.                                 */
  /*    stream :: The input stream.                                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_DEF
  TT_Error  OTL_Load_Feature_List( OTL_Feature_List*  list,
                                   FT_Stream          stream );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    OTL_Load_Lookup_List                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads an OpenType Lookup List from a font resource.                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    list   :: The target lookup list.                                  */
  /*    stream :: The input stream.                                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This function does NOT load the lookup sub-tables.  Instead, it    */
  /*    stores the file offsets of the particular table in each lookup     */
  /*    element.  It is up to the caller to load these sub-tables.  This   */
  /*    can be done more easily with OTL_Iterate_Lookup_List().            */
  /*                                                                       */
  LOCAL_DEF
  TT_Error  OTL_Load_Lookup_List( OTL_Lookup_List*  list,
                                  FT_Stream         stream );


  typedef int  OTL_Lookup_Iterator( OTL_Lookup*  lookup,
                                    void*        closure );


  typedef void  OTL_Lookup_Destructor( OTL_Lookup*  lookup,
                                       FT_System    system );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    OTL_Iterate_Lookup_List                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Scans an OpenType Lookup List.  This can be used to load the       */
  /*    Lookup sub-tables in a GSUB or GPOS loader.                        */
  /*                                                                       */
  /* <Input>                                                               */
  /*    list     :: The source list.                                       */
  /*    iterator :: The iterator -- a function which is called on each     */
  /*                element of the list.                                   */
  /*    closure  :: User-specified data which is passed to each iterator   */
  /*                with the lookup element pointer.                       */
  /*                                                                       */
  /* <Return>                                                              */
  /*    If one iterator call returns a non-zero `result', the list parsing */
  /*    is aborted and the value is returned to the caller.  Otherwise,    */
  /*    the function returns 0 when the list has been parsed completely.   */
  /*                                                                       */
  LOCAL_DEF
  TT_Error  OTL_Iterate_Lookup_List( OTL_Lookup_List*     list,
                                     OTL_Lookup_Iterator  iterator,
                                     void*                closure );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    OTL_Load_Coverage                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads an OpenType Coverage table from a font resource.             */
  /*                                                                       */
  /* <Input>                                                               */
  /*    coverage :: The target coverage.                                   */
  /*    stream   :: The input stream.                                      */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_FUNC
  TT_Error  OTL_Load_Coverage( OTL_Coverage*  coverage,
                               FT_Stream      stream );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    OTL_Load_Class_Def                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads an OpenType Class Definition table from a resource.          */
  /*                                                                       */
  /* <Input>                                                               */
  /*    class_def :: The target class definition.                          */
  /*    stream    :: The input stream.                                     */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_DEF
  TT_Error  OTL_Load_Class_Def( OTL_Class_Def*  class_def,
                                FT_Stream       stream );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    OTL_Load_Device                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads an OpenType Device table from a font resource.               */
  /*                                                                       */
  /* <Input>                                                               */
  /*    device :: The target device table.                                 */
  /*    stream :: The input stream.                                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_DEF
  TT_Error  OTL_Load_Device( OTL_Device*  device,
                             FT_Stream    stream );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    OTL_Free_Script_List                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Releases a given OpenType Script list.                             */
  /*                                                                       */
  /* <Input>                                                               */
  /*    list   :: The target script list.                                  */
  /*    system :: The current system object.                               */
  /*                                                                       */
  LOCAL_DEF
  void  OTL_Free_Script_List( OTL_Script_List*  list,
                              FT_System         system );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    OTL_Free_Features_List                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Releases a given OpenType Features list.                           */
  /*                                                                       */
  /* <Input>                                                               */
  /*    list   :: The target feature list.                                 */
  /*    system :: The current system object.                               */
  /*                                                                       */
  LOCAL_DEF
  void  OTL_Free_Features_List( OTL_Feature_List*  list,
                                FT_System          system );



  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    OTL_Free_Lookup_List                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Releases a given OpenType Lookup list.  Uses a destructor called   */
  /*    to destroy the Lookup sub-tables.                                  */
  /*                                                                       */
  /* <Input>                                                               */
  /*    list       :: The target lookup list.                              */
  /*    system     :: The current system object.                           */
  /*    destructor :: A destructor function called on each lookup element. */
  /*                  Can be used to destroy sub-tables.  Ignored if NULL. */
  /*                                                                       */
  LOCAL_DEF
  void  OTL_Free_Lookup_List( OTL_Lookup_List*       list,
                              FT_System              system,
                              OTL_Lookup_Destructor  destroy );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    OTL_Free_Coverage                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Releases a given OpenType Coverage table.                          */
  /*                                                                       */
  /* <Input>                                                               */
  /*    coverage :: The target coverage.                                   */
  /*    system   :: The current system object.                             */
  /*                                                                       */
  LOCAL_DEF
  void  OTL_Free_Coverage( OTL_Coverage*  coverage,
                           FT_System      system );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    OTL_Free_Class_Def                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Releases a given OpenType Class Definition table.                  */
  /*                                                                       */
  /* <Input>                                                               */
  /*    class_def :: The target class definition.                          */
  /*    system    :: The current system object.                            */
  /*                                                                       */
  LOCAL_DEF
  void  OTL_Free_Class_Def( OTL_Class_Def*  class_def,
                            FT_System       system );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    OTL_Free_Device                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Releases a given OpenType Layout Device table.                     */
  /*                                                                       */
  /* <Input>                                                               */
  /*    device  :: The target device table.                                */
  /*    system  :: The current system object.                              */
  /*                                                                       */
  LOCAL_DEF
  void  OTL_Free_Device( OTL_Device*  device,
                         FT_System    system );


#endif /* OTLOAD_H */


/* END */
