#ifndef FTSTREAM_H
#define FTSTREAM_H

#include <ftobjs.h>

/* format of an 8-bit frame_op value = [ xxxxx | e | s ] */
/* where s is set to 1 when the value is signed..        */
/* where e is set to 1 when the value is little-endian   */
/* xxxxx is a command                                    */

#define FT_FRAME_OP_SHIFT   2
#define FT_FRAME_OP_SIGNED  1
#define FT_FRAME_OP_LITTLE  2
#define FT_FRAME_OP_COMMAND(x) (x >> FT_FRAME_OP_SHIFT)

#define FT_MAKE_FRAME_OP( command, little, sign )  \
          ((command << FT_FRAME_OP_SHIFT) | (little << 1) | sign)

#define FT_FRAME_OP_END   0
#define FT_FRAME_OP_START 1  /* start a new frame */
#define FT_FRAME_OP_BYTE  2  /* read 1-byte value */
#define FT_FRAME_OP_SHORT 3  /* read 2-byte value */
#define FT_FRAME_OP_LONG  4  /* read 4-byte value */
#define FT_FRAME_OP_OFF3  5  /* read 3-byte value */

typedef enum FT_Frame_Op_
{
  ft_frame_end       = 0,
  ft_frame_start     = FT_MAKE_FRAME_OP( FT_FRAME_OP_START, 0, 0 ),
  
  ft_frame_byte      = FT_MAKE_FRAME_OP( FT_FRAME_OP_BYTE,  0, 0 ),
  ft_frame_schar     = FT_MAKE_FRAME_OP( FT_FRAME_OP_BYTE,  0, 1 ),
  
  ft_frame_ushort_be = FT_MAKE_FRAME_OP( FT_FRAME_OP_SHORT, 0, 0 ),
  ft_frame_short_be  = FT_MAKE_FRAME_OP( FT_FRAME_OP_SHORT, 0, 1 ),
  ft_frame_ushort_le = FT_MAKE_FRAME_OP( FT_FRAME_OP_SHORT, 1, 0 ),
  ft_frame_short_le  = FT_MAKE_FRAME_OP( FT_FRAME_OP_SHORT, 1, 1 ),
  
  ft_frame_ulong_be  = FT_MAKE_FRAME_OP( FT_FRAME_OP_LONG, 0, 0 ),
  ft_frame_ulong_le  = FT_MAKE_FRAME_OP( FT_FRAME_OP_LONG, 0, 1 ),
  ft_frame_long_be   = FT_MAKE_FRAME_OP( FT_FRAME_OP_LONG, 1, 0 ),
  ft_frame_long_le   = FT_MAKE_FRAME_OP( FT_FRAME_OP_LONG, 1, 1 ),

  ft_frame_uoff3_be  = FT_MAKE_FRAME_OP( FT_FRAME_OP_OFF3, 0, 0 ),
  ft_frame_uoff3_le  = FT_MAKE_FRAME_OP( FT_FRAME_OP_OFF3, 0, 1 ),
  ft_frame_off3_be   = FT_MAKE_FRAME_OP( FT_FRAME_OP_OFF3, 1, 0 ),
  ft_frame_off3_le   = FT_MAKE_FRAME_OP( FT_FRAME_OP_OFF3, 1, 1 )

} FT_Frame_Op;


typedef struct FT_Frame_Field_
{
  FT_Frame_Op   value;
  char          size;
  FT_UShort     offset;
  
} FT_Frame_Field;

/* make-up a FT_Frame_Field out of a structure type and a field name */
#define FT_FIELD_REF(s,f)  (((s*)0)->f)

#define FT_FRAME_FIELD( frame_op, struct_type, field )                 \
          {                                                            \
            frame_op,                                                  \
            sizeof(FT_FIELD_REF(struct_type,field)),                   \
            (FT_UShort)(char*)&FT_FIELD_REF(struct_type,field) }

#define FT_MAKE_EMPTY_FIELD( frame_op )  { frame_op, 0, 0 }

#define FT_FRAME_START(s)    { ft_frame_start, 0, s }
#define FT_FRAME_END         { ft_frame_end, 0, 0 }

#define FT_FRAME_LONG(s,f)   FT_FRAME_FIELD( ft_frame_long_be, s, f )
#define FT_FRAME_ULONG(s,f)  FT_FRAME_FIELD( ft_frame_ulong_be, s, f )
#define FT_FRAME_SHORT(s,f)  FT_FRAME_FIELD( ft_frame_short_be, s, f )
#define FT_FRAME_USHORT(s,f) FT_FRAME_FIELD( ft_frame_ushort_be, s, f )
#define FT_FRAME_BYTE(s,f)   FT_FRAME_FIELD( ft_frame_byte, s, f )
#define FT_FRAME_CHAR(s,f)   FT_FRAME_FIELD( ft_frame_schar, s, f )

#define FT_FRAME_LONG_LE(s,f)   FT_FRAME_FIELD( ft_frame_long_le, s, f )
#define FT_FRAME_ULONG_LE(s,f)  FT_FRAME_FIELD( ft_frame_ulong_le, s, f )
#define FT_FRAME_SHORT_LE(s,f)  FT_FRAME_FIELD( ft_frame_short_le, s, f )
#define FT_FRAME_USHORT_LE(s,f) FT_FRAME_FIELD( ft_frame_ushort_le, s, f )

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


#define NEXT_ShortLE(buffer)  ( buffer += 2,                              \
                                ( (short)((signed char)buffer[-1] << 8) | \
				          (unsigned char)buffer[-2]     ) )

#define NEXT_UShortLE(buffer)  ((unsigned short)NEXT_ShortLE(buffer))

#define NEXT_OffsetLE(buffer)  ( buffer += 3,                                \
                                 ( ((long)(signed   char)buffer[-1] << 16) | \
                                   ((long)(unsigned char)buffer[-2] <<  8) | \
                                    (long)(unsigned char)buffer[-3]        ) )

#define NEXT_UOffsetLE(buffer) ((unsigned long)NEXT_OffsetLE(buffer))


#define NEXT_LongLE(buffer)  ( buffer += 4,                                \
                               ( ((long)(signed   char)buffer[-1] << 24) | \
                                 ((long)(unsigned char)buffer[-2] << 16) | \
                                 ((long)(unsigned char)buffer[-3] <<  8) | \
                                  (long)(unsigned char)buffer[-4]        ) )

#define NEXT_ULongLE(buffer)   ((unsigned long)NEXT_LongLE(buffer))

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

  BASE_DEF
  FT_Error FT_Read_Fields( FT_Stream             stream,
                           const FT_Frame_Field* fields,
                           void*                 structure );


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

#define READ_Fields( fields, object )  \
          ((error = FT_Read_Fields( stream, fields, object )) != FT_Err_Ok)

#endif /* FTSTREAM_H */
