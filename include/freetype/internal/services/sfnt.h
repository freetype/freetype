#ifndef __FT_SERVICE_SFNT_H__
#define __FT_SERVICE_SFNT_H__

#include FT_INTERNAL_SERVICE_H
#include FT_TRUETYPE_TABLES_H

FT_BEGIN_HEADER

 /*
  *  SFNT table loading service
  *
  */

#define FT_SERVICE_ID_SFNT_TABLE  "sfnt-table"

 /* used to implement FT_Load_Sfnt_Table()
  */
  typedef FT_Error
  (*FT_SFNT_TableLoadFunc)( FT_Face    face,
                            FT_ULong   tag,
                            FT_Long    offset,
                            FT_Byte*   buffer,
                            FT_ULong*  length );

 /* used to implement FT_Get_Sfnt_Table()
  */
  typedef void*
  (*FT_SFNT_TableGetFunc)( FT_Face      face,
                           FT_Sfnt_Tag  tag );

 
  FT_DEFINE_SERVICE( SFNT_Table )
  {
    FT_SFNT_TableLoadFunc    load_table;
    FT_SFNT_TableGetFunc     get_table;
  };

 /* */
 
FT_END_HEADER

#endif /* __FT_SERVICE_SFNT_H__ */
