#include <stdio.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "qdbmp.h"

int main() 
{
	// ft
	FT_Library library;
	FT_Face face;
	FT_Bitmap bmp;
	FT_UInt x, y, glyph_index;
	FT_Byte byte;
	BMP* qdbmp;
	char filename[20];

	if (FT_Init_FreeType(&library)) 
	{
		printf("Error: Library initialization\n");
	}

	if (FT_New_Face(library, "/usr/share/fonts/truetype/liberation/LiberationSerif-Regular.ttf", 0, &face)) 
	{
		printf("Error: Loading face\n");
	}

	// if (FT_Set_Char_Size(face, 0, 16*64, 96, 0))
	// {
	// 	printf("Error: Setting character size\n");
	// }

	if (FT_Set_Pixel_Sizes(face, 0, 48))
	{
		printf("Error: Setting character size\n");
	}

	for (glyph_index = 65; glyph_index < 91; glyph_index++)
	{
		if (FT_Load_Char(face, glyph_index, FT_LOAD_RENDER)) 
		{
	  		printf("Error: Loading characte.r\n");
		}

		bmp = face->glyph->bitmap;

		// printf("Width: %u\n", bmp.width);
		// printf("Height: %u\n", bmp.rows);
		// printf("Num grays: %u\n", bmp.num_grays);
		// printf("Pitch: %d\n", bmp.pitch);

		qdbmp = BMP_Create(bmp.width,bmp.rows,24);
		BMP_CHECK_ERROR(stdout, 1);

		for (y = 0; y < bmp.rows; y++) 
		{
			for (x = 0; x < bmp.width; x++)
			{
				byte = bmp.buffer[bmp.pitch * y + x];
				BMP_SetPixelRGB(qdbmp, x, y, byte, byte, byte);
				BMP_CHECK_ERROR(stdout, 1);
			}
		}

		sprintf(filename, "%d-2", glyph_index);

		BMP_WriteFile(qdbmp, filename);
		BMP_CHECK_ERROR(stdout, 1);

	}

	FT_Done_Face(face);
	FT_Done_FreeType(library);
	return 0;
}