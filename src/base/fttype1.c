#include <ft2build.h>
#include FT_INTERNAL_TYPE1_TYPES_H
#include FT_INTERNAL_OBJECTS_H

 /* case a FT_Face to a T1_Face when relevant                           */
 /* this implementation sucks, but a lot of things should change in the */
 /* future anyway..                                                     */
 /*                                                                     */
  static T1_Face
  t1_face_check_cast( FT_Face   face )
  {
    FT_Module  driver;
    T1_Face    result = NULL;

    if ( face && face->driver != NULL )
    {
      driver = (FT_Module) face->driver;

      if ( driver->clazz && driver->clazz->module_name &&
           ft_strcmp( driver->clazz->module_name, "type1" ) == 0 )
      {
        /* correct typecast ! */
        result = (T1_Face) face;
      }
    }
    return result;
  }



 /* documentation is in t1tables.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Get_PS_Font_Info( FT_Face          face,
                       PS_FontInfoRec*  afont_info )
  {
    FT_Error  error   = FT_Err_Invalid_Argument;
    T1_Face   t1_face = t1_face_check_cast( face );

    if ( t1_face != NULL )
    {
      *afont_info = t1_face->type1.font_info;
      error = FT_Err_Ok;
    }
    return error;
  }


 /* XXX: bad hack, but I didn't want to change several drivers here */

 /* documentation is in t1tables.h */

  FT_EXPORT_DEF( FT_Int )
  FT_Has_PS_Glyph_Names( FT_Face   face )
  {
    FT_Int       result = 0;
    const char*  driver_name;

    if ( face && face->driver && face->driver->root.clazz )
    {
      /* for now, only the type1 and cff drivers provide reliable */
      /* glyph names...                                           */

      /* we could probably hack the TrueType driver to recognize  */
      /* certain cases where the glyph names  are most certainly  */
      /* correct (e.g. using a 20 or 22 format 'post' table), but */
      /* this will probably happen later... :-)                   */

      driver_name = face->driver->root.clazz->module_name;
      result      = ( ft_strcmp( driver_name, "type1" ) ||
                      ft_strcmp( driver_name, "cff"   ) );
    }
    return result;
  }
