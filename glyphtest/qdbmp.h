#ifndef _BMP_H_
#define _BMP_H_


/**************************************************************

	QDBMP - Quick n' Dirty BMP

	v1.0.0 - 2007-04-07
	http://qdbmp.sourceforge.net


	The library supports the following BMP variants:
	1. Uncompressed 32 BPP (alpha values are ignored)
	2. Uncompressed 24 BPP
	3. Uncompressed 8 BPP (indexed color)

	QDBMP is free and open source software, distributed
	under the MIT licence.

	Copyright (c) 2007 Chai Braudo (braudo@users.sourceforge.net)

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.

**************************************************************/

#include <stdio.h>



/* Type definitions */
#ifndef UINT
	#define UINT	unsigned long int
#endif

#ifndef USHORT
	#define USHORT	unsigned short
#endif

#ifndef UCHAR
	#define UCHAR	unsigned char
#endif


/* Version */
#define QDBMP_VERSION_MAJOR		1
#define QDBMP_VERSION_MINOR		0
#define QDBMP_VERSION_PATCH		1


/* Error codes */
typedef enum
{
	BMP_OK = 0,				/* No error */
	BMP_ERROR,				/* General error */
	BMP_OUT_OF_MEMORY,		/* Could not allocate enough memory to complete the operation */
	BMP_IO_ERROR,			/* General input/output error */
	BMP_FILE_NOT_FOUND,		/* File not found */
	BMP_FILE_NOT_SUPPORTED,	/* File is not a supported BMP variant */
	BMP_FILE_INVALID,		/* File is not a BMP image or is an invalid BMP */
	BMP_INVALID_ARGUMENT,	/* An argument is invalid or out of range */
	BMP_TYPE_MISMATCH,		/* The requested action is not compatible with the BMP's type */
	BMP_ERROR_NUM
} BMP_STATUS;


/* Bitmap image */
typedef struct _BMP BMP;




/*********************************** Public methods **********************************/


/* Construction/destruction */
BMP*			BMP_Create					( UINT width, UINT height, USHORT depth );
void			BMP_Free					( BMP* bmp );


/* I/O */
BMP*			BMP_ReadFile				( const char* filename );
void			BMP_WriteFile				( BMP* bmp, const char* filename );


/* Meta info */
UINT			BMP_GetWidth				( BMP* bmp );
UINT			BMP_GetHeight				( BMP* bmp );
USHORT			BMP_GetDepth				( BMP* bmp );


/* Pixel access */
void			BMP_GetPixelRGB				( BMP* bmp, UINT x, UINT y, UCHAR* r, UCHAR* g, UCHAR* b );
void			BMP_SetPixelRGB				( BMP* bmp, UINT x, UINT y, UCHAR r, UCHAR g, UCHAR b );
void			BMP_GetPixelIndex			( BMP* bmp, UINT x, UINT y, UCHAR* val );
void			BMP_SetPixelIndex			( BMP* bmp, UINT x, UINT y, UCHAR val );


/* Palette handling */
void			BMP_GetPaletteColor			( BMP* bmp, UCHAR index, UCHAR* r, UCHAR* g, UCHAR* b );
void			BMP_SetPaletteColor			( BMP* bmp, UCHAR index, UCHAR r, UCHAR g, UCHAR b );


/* Error handling */
BMP_STATUS		BMP_GetError				();
const char*		BMP_GetErrorDescription		();


/* Useful macro that may be used after each BMP operation to check for an error */
#define BMP_CHECK_ERROR( output_file, return_value ) \
	if ( BMP_GetError() != BMP_OK )													\
	{																				\
		fprintf( ( output_file ), "BMP error: %s\n", BMP_GetErrorDescription() );	\
		return( return_value );														\
	}																				\

#endif
