#ifndef __FT_SYSTEM_IO_H__
#define __FT_SYSTEM_IO_H__

#include <ft2build.h>
#include FT_INTERNAL_OBJECT_H

FT_BEGIN_HEADER

 /* handle to a FT_Stream object */
  typedef struct FT_StreamRec_*    FT_Stream;

 /* handle to the FT_Stream class */
  typedef const struct FT_Stream_ClassRec_*   FT_Stream_Class;

 /* a method used to read data from a stream into a buffer */
  typedef FT_ULong  (*FT_Stream_ReadFunc)( FT_Stream   stream,
                                           FT_Byte*    buffer,
                                           FT_ULong    size );

 /* a method used to seek to a new position within a stream */
 /* position is always relative to the start                */
  typedef FT_Error  (*FT_Stream_SeekFunc)( FT_Stream   stream,
                                           FT_ULong    pos );

 /* the stream class structure + some useful macros */
  typedef struct FT_Stream_ClassRec_
  {
    FT_ClassRec          clazz;
    FT_Stream_ReadFunc   stream_read;
    FT_Stream_SeekFunc   stream_seek;
    
  } FT_Stream_ClassRec;

#define  FT_STREAM_CLASS(x)        ((FT_Stream_Class)(x))

#define  FT_STREAM_CLASS__READ(x)  FT_STREAM_CLASS(x)->stream_read
#define  FT_STREAM_CLASS__SEEK(x)  FT_STREAM_CLASS(x)->stream_seek;

 /* the base FT_Stream object structure */
  typedef struct FT_StreamRec_
  {
    FT_ObjectRec        object;
    FT_ULong            size;
    FT_ULong            pos;
    const FT_Byte*      base;
    const FT_Byte*      cursor;
    const FT_Byte*      limit;

  } FT_StreamRec;

 /* some useful macros */
#define  FT_STREAM(x)    ((FT_Stream)(x))
#define  FT_STREAM_P(x)  ((FT_Stream*)(x))

#define  FT_STREAM__READ(x)  FT_STREAM_CLASS__READ(FT_OBJECT__CLASS(x))
#define  FT_STREAM__SEEK(x)  FT_STREAM_CLASS__SEEK(FT_OBJECT__CLASS(x))

#define  FT_STREAM_IS_BASED(x)  ( FT_STREAM(x)->base != NULL )



#if 0
#endif

 /* */

FT_END_HEADER

#endif /* __FT_SYSTEM_STREAM_H__ */
