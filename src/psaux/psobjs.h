#ifndef PSOBJS_H
#define PSOBJS_H

#include <freetype/internal/psaux.h>


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                             T1_TABLE                          *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/


  LOCAL_DEF
  FT_Error  T1_New_Table( T1_Table*  table,
                          FT_Int     count,
                          FT_Memory  memory );
 
  LOCAL_DEF
  FT_Error  T1_Add_Table( T1_Table*  table,
                          FT_Int     index,
                          void*      object,
                          FT_Int     length );

  LOCAL_DEF
  void  T1_Done_Table( T1_Table*  table );

  LOCAL_DEF
  void  T1_Release_Table( T1_Table*  table );



  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                            T1 PARSER                          *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/


  LOCAL_DEF
  void  T1_Skip_Spaces( T1_Parser*  parser );

  LOCAL_DEF
  void  T1_Skip_Alpha( T1_Parser*  parser );

  LOCAL_DEF
  void  T1_ToToken( T1_Parser*  parser,
                    T1_Token*   token );

  LOCAL_DEF
  void  T1_ToTokenArray( T1_Parser*  parser,
                         T1_Token*   tokens,
                         FT_UInt     max_tokens,
                         FT_Int*     pnum_tokens );
 
  LOCAL_DEF
  FT_Error  T1_Load_Field( T1_Parser*       parser,
                           const T1_Field*  field,
                           void**           objects,
                           FT_UInt          max_objects,
                           FT_ULong*        pflags );
 
  LOCAL_DEF
  FT_Error  T1_Load_Field_Table( T1_Parser*       parser,
                                 const T1_Field*  field,
                                 void**           objects,
                                 FT_UInt          max_objects,
                                 FT_ULong*        pflags );
 
  LOCAL_DEF
  FT_Long  T1_ToInt  ( T1_Parser*  parser );


  LOCAL_DEF
  FT_Fixed  T1_ToFixed( T1_Parser*  parser,
                        FT_Int      power_ten );


  LOCAL_DEF
  FT_Int  T1_ToCoordArray( T1_Parser*  parser,
                           FT_Int      max_coords,
                           FT_Short*   coords );

  LOCAL_DEF
  FT_Int  T1_ToFixedArray( T1_Parser*  parser,
                           FT_Int      max_values,
                           FT_Fixed*   values,
                           FT_Int      power_ten );


  LOCAL_DEF
  void      T1_Init_Parser( T1_Parser*  parser,
                            FT_Byte*    base,
                            FT_Byte*    limit,
                            FT_Memory   memory );

  LOCAL_DEF
  void      T1_Done_Parser( T1_Parser*  parser )



  LOCAL_DEF
  void  T1_Decrypt( FT_Byte*   buffer,
                    FT_Int     length,
                    FT_UShort  seed );


#endif /* PSOBJS_H */

