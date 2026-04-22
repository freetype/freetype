/****************************************************************************
 *
 * ftstdlib.h
 *
 *   Plan 9-specific library and header configuration file
 *
 * Copyright (C) 2002-2026 by
 * David Turner, Robert Wilhelm, Werner Lemberg and Yaroslav Kolomiiets.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */
#ifndef FTSTDLIB_H_
#define FTSTDLIB_H_

#include <u.h>
#include <libc.h>
#include <ape/limits.h>
#include <stdio.h>

#define size_t   	ulong
#define ptrdiff_t	int

#define ft_ptrdiff_t  ptrdiff_t

#define FT_CHAR_BIT    CHAR_BIT
#define FT_USHORT_MAX  USHRT_MAX
#define FT_INT_MAX     INT_MAX
#define FT_INT_MIN     INT_MIN
#define FT_UINT_MAX    UINT_MAX
#define FT_LONG_MIN    LONG_MIN
#define FT_LONG_MAX    LONG_MAX
#define FT_ULONG_MAX   ULONG_MAX
#define FT_LLONG_MAX   LLONG_MAX
#define FT_LLONG_MIN   LLONG_MIN
#define FT_ULLONG_MAX  ULLONG_MAX

#define ft_memchr   memchr
#define ft_memcmp   memcmp
#define ft_memcpy   memcpy
#define ft_memmove  memmove
#define ft_memset   memset
#define ft_strcat   strcat
#define ft_strcmp   strcmp
#define ft_strcpy   strcpy
#define ft_strlen   strlen
#define ft_strncmp  strncmp
#define ft_strncpy  strncpy
#define ft_strrchr  strrchr
#define ft_strstr   strstr

#define FT_FILE      FILE
#define ft_fclose    fclose
#define ft_fopen     fopen
#define ft_fread     fread
#define ft_fseek     fseek
#define ft_ftell     ftell
#define ft_snprintf  snprintf

#define ft_qsort  qsort

#define ft_scalloc   calloc
#define ft_sfree     free
#define ft_smalloc   malloc
#define ft_srealloc  realloc

#define ft_strtol  strtol
#define ft_getenv  getenv

#define ft_jmp_buf     jmp_buf

#define ft_longjmp     longjmp
#define ft_setjmp( b ) setjmp( *(ft_jmp_buf*) &(b) )

#endif /* FTSTDLIB_H_ */
