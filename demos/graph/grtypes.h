/***************************************************************************
 *
 *  grtypes.h
 *
 *    basic type defintions
 *
 *  Copyright 1999 - The FreeType Development Team - www.freetype.org
 *
 *
 *
 *
 ***************************************************************************/

#ifndef GRTYPES_H
#define GRTYPES_H

  typedef unsigned char  byte;

#if 0
  typedef signed char    uchar;

  typedef unsigned long  ulong;
  typedef unsigned short ushort;
  typedef unsigned int   uint;
#endif

  typedef struct grDimension_
  {
    int  x;
    int  y;

  } grDimension;

#define gr_err_ok                    0
#define gr_err_memory               -1
#define gr_err_bad_argument         -2
#define gr_err_bad_target_depth     -3
#define gr_err_bad_source_depth     -4
#define gr_err_saturation_overflow  -5
#define gr_err_conversion_overflow  -6
#define gr_err_invalid_device       -7


#ifdef GR_MAKE_OPTION_SINGLE_OBJECT
#define  GR_LOCAL_DECL    static
#define  GR_LOCAL_FUNC    static
#else
#define  GR_LOCAL_DECL    extern
#define  GR_LOCAL_FUNC    /* void */
#endif

#endif /* GRTYPES_H */
