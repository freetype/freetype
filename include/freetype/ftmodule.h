/***************************************************************************/
/*                                                                         */
/*  ftmodule.h                                                             */
/*                                                                         */
/*  FreeType modules public interface.                                     */
/*                                                                         */
/*  Copyright 1996-2000 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used        */
/*  modified and distributed under the terms of the FreeType project       */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

#ifndef FTMODULE_H
#define FTMODULE_H

#include <freetype/freetype.h>

  /* module bit flags */
  typedef enum FT_Module_Flags_
  {
    ft_module_font_driver         = 1, /* this module is a font driver */
    ft_module_renderer            = 2, /* this module is a renderer    */

    ft_module_driver_scalable     = 4, /* this driver supports scalable fonts          */
    ft_module_driver_no_outlines  = 8  /* this driver does not support vector outlines */

  } FT_Module_Flags;



  typedef void  (*FT_Module_Interface)( void );


  typedef FT_Error  (*FT_Module_Constructor)( FT_Module  module );

  typedef void      (*FT_Module_Destructor)( FT_Module   module );

  typedef FT_Module_Interface (*FT_Module_Requester)( FT_Module    module,
                                                      const char*  name );


 /*************************************************************************
  *
  *  <Struct>
  *     FT_Module_Class
  *
  *  <Description>
  *     The module class descriptor.
  *
  *  <Fields>
  *     module_flags      :: bit flags describing the module
  *     module_size       :: size of one module object/instance in bytes
  *     module_name       :: name of the module
  *     module_version    :: version, as a 16.16 fixed number (major.minor)
  *     module_requires   :: the version of FreeType this module requires
  *                          (starts at 2.0, a.k.a. 0x20000)
  *
  *     module_init       :: a function used to initialise (not create) a
  *                          new module object
  *
  *     module_done       :: a function used to finalise (not destroy) a
  *                          given module object
  *
  *     get_interface     :: queries a given module for a specific interface
  *                          by name..
  *
  *************************************************************************/
  
  typedef struct  FT_Module_Class_
  {
    FT_ULong               module_flags;
    FT_Int                 module_size;
    const FT_String*       module_name;
    FT_Fixed               module_version;
    FT_Fixed               module_requires;

    const void*            module_interface;

    FT_Module_Constructor  module_init;
    FT_Module_Destructor   module_done;
    FT_Module_Requester    get_interface;
    
  } FT_Module_Class;



 /*************************************************************************
  *
  *  <Function>
  *     FT_Add_Module
  *
  *  <Description>
  *     Add a new module to a given library instance.
  *
  *  <Input>
  *     library  :: handle to library object
  *     clazz    :: pointer to class descriptor for the module
  *
  *  <Return>
  *     Error code. 0 means success
  *
  *  <Note>
  *     An error will be returned if a module already exists by that
  *     name, or if the module requires a version of freetype that is
  *     too great
  *
  *************************************************************************/
  
  FT_EXPORT_DEF(FT_Error)  FT_Add_Module( FT_Library              library,
                                          const FT_Module_Class*  clazz );


 /*************************************************************************
  *
  *  <Function>
  *     FT_Get_Module
  *
  *  <Description>
  *     Find a module by its name.
  *
  *  <Input>
  *     library     :: handle to library object
  *     module_name :: the module's ASCII name.
  *
  *  <Return>
  *     Module handle, 0 if none was found.
  *
  *  <Note>
  *     You'd better be familiar with FreeType internals to know which
  *     module to look for :-)
  *
  *************************************************************************/
  
  FT_EXPORT_DEF(FT_Module) FT_Get_Module( FT_Library   library,
                                          const char*  module_name );


 /*************************************************************************
  *
  *  <Function>
  *     FT_Get_Module_Interface
  *
  *  <Description>
  *     Find a module and returns it's specific interface as a void*
  *
  *  <Input>
  *     library     :: handle to library object
  *     module_name :: the module's ASCII name.
  *
  *  <Return>
  *     Module specific interface, if any
  *
  *  <Note>
  *     You'd better be familiar with FreeType internals to know which
  *     module to look for, and what it's interface is :-)
  *
  *************************************************************************/
  
  FT_EXPORT_DEF(const void*)  FT_Get_Module_Interface( FT_Library   library,
                                                       const char*  mod_name );

 /*************************************************************************
  *
  *  <Function>
  *     FT_Remove_Module
  *
  *  <Description>
  *     Removes a given module from a library instance.
  *
  *  <Input>
  *     library  :: handle to library object
  *     module   :: handle to module object
  *
  *  <Return>
  *     Error code (module not listed)
  *
  *  <Note>
  *     The module object is destroyed by the function in case of success
  *
  *************************************************************************/
  
  FT_EXPORT_DEF(FT_Error)  FT_Remove_Module( FT_Library  library,
                                             FT_Module   module );


#endif /* FTMODULE_H */


/* END */
