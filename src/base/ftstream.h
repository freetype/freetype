#ifndef FTSTREAM_H
#define FTSTREAM_H

#include <ftobjs.h>


  /*************************************************************************/
  /*                                                                       */
  /* integer extraction macros - the `buffer' parameter must ALWAYS be of  */
  /* type `char*' or equivalent (1-byte elements).                         */
  /*                                                                       */
#define NEXT_Char(buffer)    ((signed char)*buffer++)
#define NEXT_Byte(buffer)    ((unsigned char)*buffer++)

#define NEXT_Short(buffer)   ( buffer += 2,                                \
                               ( (short)((signed   char)buffer[-2] << 8) | \
                                         (unsigned char)buffer[-1]       ) )

#define NEXT_UShort(buffer)  ((unsigned short)NEXT_Short(buffer))

#define NEXT_Offset(buffer)  ( buffer += 3,                                \
                               ( ((long)(signed   char)buffer[-3] << 16) | \
                                 ((long)(unsigned char)buffer[-2] <<  8) | \
                                  (long)(unsigned char)buffer[-1]        ) )

#define NEXT_UOffset(buffer) ((unsigned long)NEXT_Offset(buffer))

#define NEXT_Long(buffer)    ( buffer += 4,                                \
                               ( ((long)(signed   char)buffer[-4] << 24) | \
                                 ((long)(unsigned char)buffer[-3] << 16) | \
                                 ((long)(unsigned char)buffer[-2] <<  8) | \
                                  (long)(unsigned char)buffer[-1]        ) )

#define NEXT_ULong(buffer)   ((unsigned long)NEXT_Long(buffer))


  /*************************************************************************/
  /*                                                                       */
  /* Each GET_xxxx() macro uses an implicit `stream' variable.             */
  /*                                                                       */
#define FT_GET_MACRO( func, type )        ( (type)func(stream) )

#define GET_Char()     FT_GET_MACRO( FT_Get_Char, FT_Char )
#define GET_Byte()     FT_GET_MACRO( FT_Get_Char, FT_Byte )
#define GET_Short()    FT_GET_MACRO( FT_Get_Short, FT_Short )
#define GET_UShort()   FT_GET_MACRO( FT_Get_Short, FT_UShort )
#define GET_Offset()   FT_GET_MACRO( FT_Get_Offset, FT_Long )
#define GET_UOffset()  FT_GET_MACRO( FT_Get_Offset, FT_ULong )
#define GET_Long()     FT_GET_MACRO( FT_Get_Long, FT_Long )
#define GET_ULong()    FT_GET_MACRO( FT_Get_Long, FT_ULong )
#define GET_Tag4()     FT_GET_MACRO( FT_Get_Long, FT_ULong )


#define FT_READ_MACRO( func, type, var )        \
          ( var = (type)func( stream, &error ), \
            error != FT_Err_Ok )

#define READ_Byte( var )     FT_READ_MACRO( FT_Read_Char, FT_Byte, var )
#define READ_Char( var )     FT_READ_MACRO( FT_Read_Char, FT_Char, var )
#define READ_Short( var )    FT_READ_MACRO( FT_Read_Short, FT_Short, var )
#define READ_UShort( var )   FT_READ_MACRO( FT_Read_Short, FT_UShort, var )
#define READ_Offset( var )   FT_READ_MACRO( FT_Read_Offset, FT_Long, var )
#define READ_UOffset( var )  FT_READ_MACRO( FT_Read_Offset, FT_ULong, var )
#define READ_Long( var )     FT_READ_MACRO( FT_Read_Long, FT_Long, var )
#define READ_ULong( var )    FT_READ_MACRO( FT_Read_Long, FT_ULong, var )



  BASE_DEF
  void  FT_New_Memory_Stream( FT_Library     library,
                              void*          base,
     			      unsigned long  size,
                              FT_Stream      stream );
 
  BASE_DEF
  FT_Error  FT_Seek_Stream( FT_Stream  stream,
                            FT_ULong   pos );

  BASE_DEF
  FT_Error  FT_Skip_Stream( FT_Stream  stream,
                            FT_Long    distance );
                            
  BASE_DEF
  FT_Long   FT_Stream_Pos( FT_Stream  stream );


  BASE_DEF
  FT_Error  FT_Read_Stream( FT_Stream  stream,
                            void*      buffer,
                            FT_ULong   count );
                            
  BASE_DEF
  FT_Error  FT_Read_Stream_At( FT_Stream  stream,
                               FT_ULong   pos,
                               void*      buffer,
                               FT_ULong   count );
                               
  BASE_DEF
  FT_Error  FT_Access_Frame( FT_Stream  stream,
                             FT_ULong   count );
                             
  BASE_DEF
  void      FT_Forget_Frame( FT_Stream  stream );



  BASE_DEF
  FT_Char   FT_Get_Char( FT_Stream  stream );
  
  BASE_DEF
  FT_Short  FT_Get_Short( FT_Stream  stream );
  
  BASE_DEF
  FT_Long   FT_Get_Offset( FT_Stream  stream );
  
  BASE_DEF
  FT_Long   FT_Get_Long( FT_Stream  stream );



  BASE_DEF
  FT_Char  FT_Read_Char( FT_Stream  stream,
                         FT_Error*  error ); 

  BASE_DEF
  FT_Short  FT_Read_Short( FT_Stream  stream,
                           FT_Error*  error ); 

  BASE_DEF
  FT_Long  FT_Read_Offset( FT_Stream  stream,
                           FT_Error*  error ); 

  BASE_DEF
  FT_Long  FT_Read_Long( FT_Stream  stream,
                         FT_Error*  error ); 


#define USE_Stream( resource, stream )  \
          FT_SET_ERROR( FT_Open_Stream( resource, stream ) )

#define DONE_Stream( stream )  \
          FT_Done_Stream( stream )


#define ACCESS_Frame( size )  \
          FT_SET_ERROR( FT_Access_Frame( stream, size ) )

#define ACCESS_Compressed_Frame( size )  \
          FT_SET_ERROR( FT_Access_Compressed_Frame( stream, size ) )


#define FORGET_Frame() \
          FT_Forget_Frame( stream )


#define FILE_Seek( position )  \
          FT_SET_ERROR( FT_Seek_Stream( stream, position ) )

#define FILE_Skip( distance )  \
          FT_SET_ERROR( FT_Skip_Stream( stream, distance ) )

#define FILE_Pos() \
          FT_Stream_Pos( stream )

#define FILE_Read( buffer, count )                         \
          FT_SET_ERROR( FT_Read_Stream( stream,            \
                                        (FT_Char*)buffer,  \
                                        count ) )

#define FILE_Read_At( position, buffer, count )              \
          FT_SET_ERROR( FT_Read_Stream_At( stream,           \
                                           position,         \
                                           (FT_Char*)buffer, \
                                           count ) )



#endif /* FTIO_H */
