/***************************************************************************/
/*                                                                         */
/*  cffparse.c                                                             */
/*                                                                         */
/*    CFF token stream parser (body)                                       */
/*                                                                         */
/*  Copyright 1996-2000 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

#include <ft2build.h>
#include FT_SOURCE_FILE(cff,cffparse.h)
#include FT_INTERNAL_CFF_ERRORS_H
#include FT_INTERNAL_STREAM_H

  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_t2parse


#define T2_Err_Stack_Underflow   FT_Err_Invalid_Argument
#define T2_Err_Syntax_Error      FT_Err_Invalid_Argument


  enum
  {
    cff_kind_none = 0,
    cff_kind_num,
    cff_kind_fixed,
    cff_kind_string,
    cff_kind_bool,
    cff_kind_delta,
    cff_kind_callback,

    cff_kind_max  /* do not remove */
  };


  /* now generate handlers for the most simple fields */
  typedef FT_Error  (*CFF_Field_Reader)( CFF_Parser*  parser );

  typedef struct  CFF_Field_Handler_
  {
    int               kind;
    int               code;
    FT_UInt           offset;
    FT_Byte           size;
    CFF_Field_Reader  reader;
    FT_UInt           array_max;
    FT_UInt           count_offset;

  } CFF_Field_Handler;


  FT_LOCAL_DEF void
  CFF_Parser_Init( CFF_Parser*  parser,
                   FT_UInt     code,
                   void*       object )
  {
    MEM_Set( parser, 0, sizeof ( *parser ) );

    parser->top         = parser->stack;
    parser->object_code = code;
    parser->object      = object;
  }


  /* reads an integer */
  static FT_Long
  cff_parse_integer( FT_Byte*  start,
                     FT_Byte*  limit )
  {
    FT_Byte*  p   = start;
    FT_Int    v   = *p++;
    FT_Long   val = 0;


    if ( v == 28 )
    {
      if ( p + 2 > limit )
        goto Bad;

      val = (FT_Short)( ( (FT_Int)p[0] << 8 ) | p[1] );
      p  += 2;
    }
    else if ( v == 29 )
    {
      if ( p + 4 > limit )
        goto Bad;

      val = ( (FT_Long)p[0] << 24 ) |
            ( (FT_Long)p[1] << 16 ) |
            ( (FT_Long)p[2] <<  8 ) |
                       p[3];
      p += 4;
    }
    else if ( v < 247 )
    {
      val = v - 139;
    }
    else if ( v < 251 )
    {
      if ( p + 1 > limit )
        goto Bad;

      val = ( v - 247 ) * 256 + p[0] + 108;
      p++;
    }
    else
    {
      if ( p + 1 > limit )
        goto Bad;

      val = -( v - 251 ) * 256 - p[0] - 108;
      p++;
    }

  Exit:
    return val;

  Bad:
    val = 0;
    goto Exit;
  }


  /* read a real */
  static FT_Fixed
  cff_parse_real( FT_Byte*  start,
                  FT_Byte*  limit,
                  FT_Int    power_ten )
  {
    FT_Byte*  p    = start;
    FT_Long   num, divider, result, exp;
    FT_Int    sign = 0, exp_sign = 0;
    FT_Byte   nib;
    FT_Byte   phase;


    result  = 0;
    num     = 0;
    divider = 1;

    /* first of all, read the integer part */
    phase = 4;

    for (;;)
    {

      /* If we entered this iteration with phase == 4, we need to */
      /* read a new byte.  This also skips past the intial 0x1E.  */
      if ( phase )
      {
        p++;

        /* Make sure we don't read past the end. */
        if ( p >= limit )
          goto Bad;
      }

      /* Get the nibble. */
      nib   = ( p[0] >> phase ) & 0xF;
      phase = 4 - phase;

      if ( nib == 0xE )
        sign = 1;
      else if ( nib > 9 )
        break;
      else
        result = result * 10 + nib;
    }

    /* read decimal part, if any */
    if ( nib == 0xa )
      for (;;)
      {

        /* If we entered this iteration with phase == 4, we need */
        /* to read a new byte.                                   */
        if ( phase )
	{
          p++;

          /* Make sure we don't read past the end. */
          if ( p >= limit )
            goto Bad;
	}

        /* Get the nibble. */
        nib   = ( p[0] >> phase ) & 0xF;
        phase = 4 - phase;
        if ( nib >= 10 )
          break;

        if (divider < 10000000L)
        {
          num      = num * 10 + nib;
          divider *= 10;
        }
      }

    /* read exponent, if any */
    if ( nib == 12 )
    {
      exp_sign = 1;
      nib      = 11;
    }

    if ( nib == 11 )
    {
      exp = 0;

      for (;;)
      {
        /* If we entered this iteration with phase == 4, we need */
        /* to read a new byte.                                   */
        if ( phase )
	{
          p++;

          /* Make sure we don't read past the end. */
          if ( p >= limit )
            goto Bad;
	}

        /* Get the nibble. */
        nib   = ( p[0] >> phase ) & 0xF;
        phase = 4 - phase;
        if ( nib >= 10 )
          break;

        exp = exp * 10 + nib;
      }

      if ( exp_sign )
        exp = -exp;

      power_ten += exp;
    }

    /* raise to power of ten if needed */
    while ( power_ten > 0 )
    {
      result = result * 10;
      num    = num * 10;

      power_ten--;
    }

    while ( power_ten < 0 )
    {
      result  = result / 10;
      divider = divider * 10;

      power_ten++;
    }

    if ( num )
      result += FT_DivFix( num, divider );

    if ( sign )
      result = -result;

  Exit:
    return result;

  Bad:
    result = 0;
    goto Exit;
  }


  /* read a number, either integer or real */
  static FT_Long
  cff_parse_num( FT_Byte**  d )
  {
    return ( **d == 30 ? ( cff_parse_real   ( d[0], d[1], 0 ) >> 16 )
                       :   cff_parse_integer( d[0], d[1] ) );
  }


  /* reads a floating point number, either integer or real */
  static FT_Fixed
  cff_parse_fixed( FT_Byte**  d )
  {
    return ( **d == 30 ? cff_parse_real   ( d[0], d[1], 0 )
                       : cff_parse_integer( d[0], d[1] ) << 16 );
  }


  static FT_Error
  cff_parse_font_matrix( CFF_Parser*  parser )
  {
    CFF_Font_Dict*  dict   = (CFF_Font_Dict*)parser->object;
    FT_Matrix*      matrix = &dict->font_matrix;
    FT_Vector*      offset = &dict->font_offset;
    FT_Byte**       data   = parser->stack;
    FT_Error        error;


    error = T2_Err_Stack_Underflow;

    if ( parser->top >= parser->stack + 6 )
    {
      matrix->xx = cff_parse_fixed( data++ );
      matrix->yx = cff_parse_fixed( data++ );
      matrix->xy = cff_parse_fixed( data++ );
      matrix->yy = cff_parse_fixed( data++ );
      offset->x  = cff_parse_fixed( data++ );
      offset->y  = cff_parse_fixed( data   );
      error = T2_Err_Ok;
    }

    return error;
  }


  static FT_Error
  cff_parse_font_bbox( CFF_Parser*  parser )
  {
    CFF_Font_Dict*  dict = (CFF_Font_Dict*)parser->object;
    FT_BBox*        bbox = &dict->font_bbox;
    FT_Byte**       data = parser->stack;
    FT_Error        error;


    error = T2_Err_Stack_Underflow;

    if ( parser->top >= parser->stack + 4 )
    {
      bbox->xMin = cff_parse_num( data++ );
      bbox->yMin = cff_parse_num( data++ );
      bbox->xMax = cff_parse_num( data++ );
      bbox->yMax = cff_parse_num( data   );
      error = T2_Err_Ok;
    }

    return error;
  }


  static FT_Error
  cff_parse_private_dict( CFF_Parser*  parser )
  {
    CFF_Font_Dict*  dict = (CFF_Font_Dict*)parser->object;
    FT_Byte**       data = parser->stack;
    FT_Error        error;


    error = T2_Err_Stack_Underflow;

    if ( parser->top >= parser->stack + 2 )
    {
      dict->private_size   = cff_parse_num( data++ );
      dict->private_offset = cff_parse_num( data   );
      error = T2_Err_Ok;
    }

    return error;
  }


  static FT_Error
  cff_parse_cid_ros( CFF_Parser*  parser )
  {
    CFF_Font_Dict*  dict = (CFF_Font_Dict*)parser->object;
    FT_Byte**       data = parser->stack;
    FT_Error        error;


    error = T2_Err_Stack_Underflow;

    if ( parser->top >= parser->stack + 3 )
    {
      dict->cid_registry   = (FT_UInt)cff_parse_num ( data++ );
      dict->cid_ordering   = (FT_UInt)cff_parse_num ( data++ );
      dict->cid_supplement = (FT_ULong)cff_parse_num( data );
      error = T2_Err_Ok;
    }

    return error;
  }


#define CFF_FIELD_NUM( code, name ) \
          CFF_FIELD( code, name, cff_kind_num )
#define CFF_FIELD_FIXED( code, name ) \
          CFF_FIELD( code, name, cff_kind_fixed )
#define CFF_FIELD_STRING( code, name ) \
          CFF_FIELD( code, name, cff_kind_string )
#define CFF_FIELD_BOOL( code, name ) \
          CFF_FIELD( code, name, cff_kind_bool )
#define CFF_FIELD_DELTA( code, name, max ) \
          CFF_FIELD( code, name, cff_kind_delta )

#define CFF_FIELD_CALLBACK( code, name ) \
          {                              \
            cff_kind_callback,           \
            code | CFFCODE,              \
            0, 0,                        \
            cff_parse_ ## name,          \
            0, 0                         \
          },

#undef  CFF_FIELD
#define CFF_FIELD( code, name, kind ) \
          {                          \
            kind,                    \
            code | CFFCODE,          \
            FT_FIELD_OFFSET( name ), \
            FT_FIELD_SIZE( name ),   \
            0, 0, 0                  \
          },

#undef  CFF_FIELD_DELTA
#define CFF_FIELD_DELTA( code, name, max ) \
        {                                  \
          cff_kind_delta,                  \
          code | CFFCODE,                  \
          FT_FIELD_OFFSET( name ),         \
          FT_FIELD_SIZE_DELTA( name ),     \
          0,                               \
          max,                             \
          FT_FIELD_OFFSET( num_ ## name )  \
        },

#define CFFCODE_TOPDICT  0x1000
#define CFFCODE_PRIVATE  0x2000

  static const CFF_Field_Handler  cff_field_handlers[] =
  {

#include  FT_SOURCE_FILE(cff,cfftoken.h)

    { 0, 0, 0, 0, 0, 0, 0 }
  };


  FT_LOCAL_DEF FT_Error
  CFF_Parser_Run( CFF_Parser*  parser,
                  FT_Byte*    start,
                  FT_Byte*    limit )
  {
    FT_Byte*  p     = start;
    FT_Error  error = T2_Err_Ok;


    parser->top    = parser->stack;
    parser->start  = start;
    parser->limit  = limit;
    parser->cursor = start;

    while ( p < limit )
    {
      FT_Byte  v = *p;


      if ( v >= 27 && v != 31 )
      {
        /* it's a number; we will push its position on the stack */
        if ( parser->top - parser->stack >= CFF_MAX_STACK_DEPTH )
          goto Stack_Overflow;

        *parser->top ++ = p;

        /* now, skip it */
        if ( v == 30 )
        {
          /* skip real number */
          p++;
          for (;;)
          {
            if ( p >= limit )
              goto Syntax_Error;
            v = p[0] >> 4;
            if ( v == 15 )
              break;
            v = p[0] & 0xF;
            if ( v == 15 )
              break;
            p++;
          }
        }
        else if ( v == 28 )
          p += 2;
        else if ( v == 29 )
          p += 4;
        else if ( v > 246 )
          p += 1;
      }
      else
      {
        /* This is not a number, hence it's an operator.  Compute its code */
        /* and look for it in our current list.                            */

        FT_UInt                  code;
        FT_UInt                  num_args = (FT_UInt)
                                              ( parser->top - parser->stack );
        const CFF_Field_Handler*  field;


        /* first of all, a trivial check */
        if ( num_args < 1 )
          goto Stack_Underflow;

        *parser->top = p;
        code = v;
        if ( v == 12 )
        {
          /* two byte operator */
          p++;
          code = 0x100 | p[0];
        }
        code = code | parser->object_code;

        for ( field = cff_field_handlers; field->kind; field++ )
        {
          if ( field->code == (FT_Int)code )
          {
            /* we found our field's handler; read it */
            FT_Long   val;
            FT_Byte*  q = (FT_Byte*)parser->object + field->offset;

            switch ( field->kind )
            {
            case cff_kind_bool:
            case cff_kind_string:
            case cff_kind_num:
              val = cff_parse_num( parser->stack );
              goto Store_Number;

            case cff_kind_fixed:
              val = cff_parse_fixed( parser->stack );

            Store_Number:
              switch ( field->size )
              {
              case 1:
                *(FT_Byte*)q = (FT_Byte)val;
                break;

              case 2:
                *(FT_Short*)q = (FT_Short)val;
                break;

              case 4:
                *(FT_Int32*)q = (FT_Int)val;
                break;

              default:  /* for 64-bit systems where long is 8 bytes */
                *(FT_Long*)q = val;
              }
              break;

            case cff_kind_delta:
              {
                FT_Byte*   qcount = (FT_Byte*)parser->object +
                                      field->count_offset;

                FT_Byte**  data = parser->stack;


                if ( num_args > field->array_max )
                  num_args = field->array_max;

                /* store count */
                *qcount = (FT_Byte)num_args;

                val = 0;
                while ( num_args > 0 )
                {
                  val += cff_parse_num( data++ );
                  switch ( field->size )
                  {
                  case 1:
                    *(FT_Byte*)q = (FT_Byte)val;
                    break;

                  case 2:
                    *(FT_Short*)q = (FT_Short)val;
                    break;

                  case 4:
                    *(FT_Int32*)q = (FT_Int)val;
                    break;

                  default:  /* for 64-bit systems */
                    *(FT_Long*)q = val;
                  }

                  q += field->size;
                  num_args--;
                }
              }
              break;

            default:  /* callback */
              error = field->reader( parser );
              if ( error )
                goto Exit;
            }
            goto Found;
          }
        }

        /* this is an unknown operator, or it is unsupported; */
        /* we will ignore it for now.                         */

      Found:
        /* clear stack */
        parser->top = parser->stack;
      }
      p++;
    }

  Exit:
    return error;

  Stack_Overflow:
    error = T2_Err_Invalid_Argument;
    goto Exit;

  Stack_Underflow:
    error = T2_Err_Invalid_Argument;
    goto Exit;

  Syntax_Error:
    error = T2_Err_Invalid_Argument;
    goto Exit;
  }


/* END */
