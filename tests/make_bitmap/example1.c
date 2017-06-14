// This example uses the FreeType rendering library to produce 
// LCD rendered bitmap (RGB) for the glyphs of a font
// Also gives the MurmurHash3 hash value of the bitmap rendered.

#include "bitmap.h"

int main(int argc, char const *argv[])
{
    FT_Library      library;
    FT_Face         face;
    FT_GlyphSlot    slot;
    FT_Bitmap*		bitmap;
    FT_Error        error;

    const char*     font_file; 		// Arguments
    FT_UInt32       size;
    FT_UInt         glyph_index;
    const char*     character;

    if (argc != 4)
    {
        printf("\nTo render a RGB bitmap of a glyph to be displayed on an \n");
        printf("LCD surface and to generate the 128bit MurmurHash3 hash \n\n");

        printf("Filter used is FT_LCD_FILTER_DEFAULT\n\n");

   		printf("Usage	./<exe> <font_file> <pt_size> <character>\n\n");

        exit(0);
    }

    font_file = 	argv[1];
    size =  		atoi(argv[2]);  
    character =  	argv[3];

    error = FT_Init_FreeType( &library );
    if(error){
        printf("Error while initialising library\n");
    }

    error = FT_Library_SetLcdFilter( library,
                                     FT_LCD_FILTER_DEFAULT );
    if(error){
        printf("Error while setting LCD filter\n");
    }

    error = FT_New_Face( library, 
                         font_file, 
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

    glyph_index = FT_Get_Char_Index( face, *character );

    slot = face->glyph;

    error = FT_Load_Glyph( face,
                           glyph_index, 
                           FT_LOAD_NO_BITMAP | FT_LOAD_TARGET_LCD );
    if(error){
        printf("Error loading glyph\n");
    }

    FT_Render_Glyph( slot, 
                     FT_RENDER_MODE_LCD);
    if(error){
        printf("Error rendering the glyph\n");
    }

    bitmap = &slot->bitmap;

    Write_Bitmap_Header(bitmap);
    Write_Bitmap_Data_LCD_RGB(bitmap);
    
    HASH_128 *  murmur = (HASH_128 *) malloc(sizeof(HASH_128)) ;
    murmur = Generate_Hash_x64_128(bitmap,murmur);	// using the x64_128 function of the hash
    
    printf("%08x %08x %08x %08x\n", murmur->hash[0], // Print hash
                                    murmur->hash[1],
                                    murmur->hash[2], 
                                    murmur->hash[3]); 
    
    FT_Done_Face    ( face );
    FT_Done_FreeType( library );

    return 0;
}