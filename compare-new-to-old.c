#include "bitmap.h"

int main(int argc, char const * argv[])
{
  FT_Library library;
  FT_Face face;
  FT_GlyphSlot slot;
  FT_Bitmap * bitmap;
  FT_UInt32 size;

  const char * filename;

  int i, j;

  char mmhash[40];
  unsigned int lines;
  int line_len = 40;
  char ** hashes;
  char fname[20];

  FILE * fp = fopen("old-hashes.txt", "r");

  filename = argv[1];
  size = atoi(argv[2]);

  if (FT_Init_FreeType(&library))
  {
    printf("Error: library init.\n");
  }

  if (FT_New_Face(library, filename, 0, &face)) 
  {
    printf("Error: loading face.\n");
  }

  lines = face->num_glyphs;
  hashes = (char **)malloc(sizeof(char*)*lines);

  for (i = 0; i < lines; i++)
  {
    hashes[i] = malloc(line_len);
    if (hashes[i]==NULL)
    {
      fprintf(stderr,"Out of memory (3).\n");
      exit(4);
    }
    if (fgets(hashes[i],line_len-1,fp)==NULL)
      break;

    /* Get rid of CR or LF at end of line */
    for (j=strlen(hashes[i])-1;j>=0 && (hashes[i][j]=='\n' || hashes[i][j]=='\r');j--)
      ;
    hashes[i][j+1]='\0';
  }
  fclose(fp);

  if (FT_Set_Char_Size(face, size * 64, 0, 96, 0))
  {
    printf("Error: setting char size.\n");
  }

  slot = face->glyph;

  for (i = 0; i < face->num_glyphs; ++i)
  {
    if (FT_Load_Glyph(face, i, FT_LOAD_DEFAULT))
    {
      printf("Error: loading glyph.\n");
    }

    if (FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL))
    {
      printf("Error: rendering glyph.\n");
    }

    bitmap = &slot->bitmap;

    HASH_128 * murmur = (HASH_128 *)malloc(sizeof(HASH_128));
    murmur = Generate_Hash_x64_128(bitmap,murmur);

    sprintf(mmhash, "%08x%08x%08x%08x", murmur->hash[0], murmur->hash[1], murmur->hash[2], murmur->hash[3]);

    printf("%s\n", hashes[i]);
    printf("%s\n", mmhash);
    if (strcmp(mmhash, hashes[i]) == 0)
    {
      if (bitmap->width != 0 && bitmap->rows !=0)
      {
          printf("Glyph %d differs.\n", i);
          sprintf(fname, "%d-new.bmp", i);
          Write_Bitmap_Header(bitmap, fname);
          Write_Bitmap_Data_GRAY(bitmap, fname);
      }  
    }

  }

  FT_Done_Face(face);
  FT_Done_FreeType(library);

  return 0;

}