#include "bitmap.h"

int main (int argc, char const *argv[])
{
	FT_Library       library;
  FT_Face          face;
  FT_GlyphSlot     slot;

  FT_Bitmap*       bitmap;

  FT_Error         error;

  const char*      font_file;
  int       		   size;
  int     		     render_mode; // argument 

  int 			       load_flag;	 // FT_LOAD_XXX
  int              render_flag; // FT_RENDER_MODE_XXX
  int 			       target_flag; // FT_LOAD_TARGET_XXX
	char* 			     render_type; // for file_name 	

	char 			       name[100]; // hashes file name
	int              i; // for loop

  if (argc != 4)
  {
    printf("\nTo generate 32-bit RGBA PNG(s) of all glyphs in a font\n");
    printf("Images will be saved in a file named \n\
    		  $(font)_$(pt_size)_$(render_mode)_$(glyph_index).png \n\n");

    printf("By default, hashes of 256-level gray bitmaps will be generated\n\n");

    printf("Usage   ./<exe> <font_file> <pt_size> <render_mode>\n\n");

    printf("Values for render_mode    0 - monochrome\n");
    printf("                          1 - anti-aliased\n");
    printf("                          2 - lcd horizontal-RGB\n");
    printf("                          3 - lcd horizontal-BGR\n");
    printf("                          4 - lcd vertical-RGB\n");
    printf("                          5 - lcd vertical-BGR\n");

    return 1;
  }

  font_file 	     =     argv[1];
  size 		         =     atof(argv[2]); 
  render_mode      =     atoi(argv[3]); 

  error = FT_Init_FreeType( &library );
  if(error){
    printf("Error while initialising library\n");
  }

  if (render_mode > 1 )
  {
    error = FT_Library_SetLcdFilter( library,
                                      FT_LCD_FILTER_DEFAULT );
    if(error){
      printf("Error while setting LCD filter\n");
    }
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
                            DPI,
                            0 );
  if(error){
    printf("Error setting character size\n");
  }

  switch ( render_mode ) {
  	case 0: render_flag 	= FT_RENDER_MODE_MONO;
  			    load_flag 		= FT_LOAD_MONOCHROME;
  			    target_flag 	= FT_LOAD_TARGET_MONO;
  			    render_type   = "mono";
  			    break;

  	case 1:	render_flag 	= FT_RENDER_MODE_NORMAL;
  			    load_flag 		= FT_LOAD_DEFAULT;
  			    target_flag 	= FT_LOAD_TARGET_NORMAL;
  			    render_type   = "gray";
  			    break;

  	case 2:	render_flag 	= FT_RENDER_MODE_LCD;
  			    load_flag 		= FT_LOAD_DEFAULT;
  			    target_flag 	= FT_LOAD_TARGET_LCD;
  			    render_type   = "lcd_rgb";
  			    break;

  	case 3:	render_flag 	= FT_RENDER_MODE_LCD;
  			    load_flag 		= FT_LOAD_DEFAULT;
  			    target_flag 	= FT_LOAD_TARGET_LCD;
  			    render_type   = "lcd_bgr";
  			    break;

  	case 4:	render_flag 	= FT_RENDER_MODE_LCD_V;
  			    load_flag 		= FT_LOAD_DEFAULT;
  			    target_flag 	= FT_LOAD_TARGET_LCD_V;
  			    render_type   = "lcd_ver_rgb";
  			    break;

  	case 5:	render_flag 	= FT_RENDER_MODE_LCD_V;
  			    load_flag 		= FT_LOAD_DEFAULT;
  			    target_flag 	= FT_LOAD_TARGET_LCD_V;
  			    render_type   = "lcd_ver_bgr";
  			    break;

  	default:render_flag 	= FT_RENDER_MODE_NORMAL;
  			    load_flag 		= FT_LOAD_DEFAULT;
  			    target_flag 	= FT_LOAD_TARGET_NORMAL;
  			    render_type   = "gray";
  }

	slot = face->glyph;

	sprintf(name,"./images/%s_%d_%s", font_file,
										                size,
										                render_type);

	for (i = 0; i <face->num_glyphs; ++i)
	{
		error = FT_Load_Glyph( face,
                           i, 
                           load_flag | target_flag);
  	if(error){
      printf("Error loading glyph\n");
  	}

  	FT_Render_Glyph( slot, 
                   	 render_flag);
  	if(error){
      printf("Error rendering the glyph\n");
  	}

  	bitmap = &slot->bitmap;

  	if (bitmap->width == 0 || bitmap->rows == 0)
    {
      continue;
    }

    FT_Bitmap       target;
    FT_Bitmap_Init( &target );

    int alignment = 4;
    error = FT_Bitmap_Convert(  library, 
                                bitmap, 
                                &target,
                                alignment);
    if(error){
      printf("Error converting the bitmap\n");
    }
    
    if (render_mode == 0)
    {
    	Make_PNG(&target,name,i,render_mode);

    }else{

    	Make_PNG(bitmap,name,i,render_mode);
    } 
	}

	FT_Done_Face    ( face );
  FT_Done_FreeType( library );

  return 0;
    
}
