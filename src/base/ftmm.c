#include <freetype/ftmm.h>
#include <freetype/internal/ftobjs.h>

  FT_EXPORT_FUNC(FT_Error)   FT_Get_Multi_Master( FT_Face           face,
                                                  FT_Multi_Master*  master )
  {
    FT_Error  error;
    
    error = FT_Err_Invalid_Argument;
    if (face && FT_HAS_MULTIPLE_MASTERS(face))
    {
      FT_Driver       driver = face->driver;
      FT_Get_MM_Func  func;

      func   = (FT_Get_MM_Func)driver->interface.get_interface(
                                                   driver, "get_mm" );
      if (func)
        error = func(face,master);
    }
      
    return error;
  }                                                  


  FT_EXPORT_FUNC(FT_Error)   FT_Set_MM_Design_Coordinates( FT_Face  face,
                                                           FT_UInt  num_coords,
                                                           FT_Long* coords )
  {
    FT_Error  error;
    
    error = FT_Err_Invalid_Argument;
    if (face && FT_HAS_MULTIPLE_MASTERS(face))
    {
      FT_Driver              driver = face->driver;
      FT_Set_MM_Design_Func  func;

      func = (FT_Set_MM_Design_Func)driver->interface.get_interface(
                                             driver, "set_mm_design" );
      if (func)
        error = func(face,num_coords,coords);
    }
      
    return error;
  }                                                           

  FT_EXPORT_FUNC(FT_Error)   FT_Set_MM_Blend_Coordinates( FT_Face   face,
                                                          FT_UInt   num_coords,
                                                          FT_Fixed* coords )
  {                                                          
    FT_Error  error;
    
    error = FT_Err_Invalid_Argument;
    if (face && FT_HAS_MULTIPLE_MASTERS(face))
    {
      FT_Driver              driver = face->driver;
      FT_Set_MM_Blend_Func  func;

      func = (FT_Set_MM_Blend_Func)driver->interface.get_interface(
                                             driver, "set_mm_blend" );
      if (func)
        error = func(face,num_coords,coords);
    }
      
    return error;
  }

