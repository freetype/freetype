// This example uses the FreeType rendering library to print 
// the MurmurHash3 hash value of the 8-bit anti-aliased
// bitmaps of all the glyphs in the font.

#include "bitmap.h" // The header file

int main(int argc, char const *argv[])
{
    FT_Library      library;
    FT_Face         face;
    FT_GlyphSlot    slot;
    FT_Bitmap*      bitmap;

    FT_Error        error;

    const char*     filename;
    FT_UInt32       size; 

    if (argc != 3)
    {
    	printf("This program prints the hashes of 8-bit grayscale\n"); 
    	printf("anti-aliased bitmaps of all the glyphs in a font\n\n");

    	printf("Usage ./<exe> <font_file> <pt_size>\n\n");
    	exit(0);
    }


    filename = argv[1];
    size =  atoi(argv[2]); 

    error = FT_Init_FreeType( &library );
    if(error){
        printf("Error while initialising library\n");
    }

    error = FT_New_Face( library, 
                         filename, 
                         0, 
                         &face );
    if(error){
        printf("Error loading new face\n");
    }

    error = FT_Set_Char_Size( face,
                              size * 64,
                              0, 
                              100, 
                              0 );
    if(error){
        printf("Error setting character size\n");
    }

    slot = face->glyph;

    for (int i = 0; i < face->num_glyphs; ++i)      // Loop
    {
	    error = FT_Load_Glyph( face,
	                            i, 
	                            FT_LOAD_DEFAULT);
	    if(error){
	        printf("Error loading glyph\n");
	    }

	    FT_Render_Glyph( slot, 
	                     FT_RENDER_MODE_NORMAL);
	    if(error){
	        printf("Error rendering the glyph\n");
	    }
	    	
		bitmap = &slot->bitmap;

		if (bitmap->width == 0|| bitmap->rows == 0)
		{
			printf("%d - Empty Glyph\n",i);
			continue;
		}

	    HASH_128 *  murmur = (HASH_128 *) malloc(sizeof(HASH_128)) ;
	    murmur = Generate_Hash_x64_128(bitmap,murmur);

	    printf("%d - %08x %08x %08x %08x\n",i,
                                            murmur->hash[0], 
	                                        murmur->hash[1],
	                                   	    murmur->hash[2], 
	                                        murmur->hash[3]); 
    }

    FT_Done_Face    ( face );
    FT_Done_FreeType( library );

    return 0;
}