#include "bitmap.h"

int main(int argc, char const * argv[])
{
  FT_Library library;
  FT_Face face;
  FT_GlyphSlot slot;
  FT_Bitmap * bitmap;
  FT_UInt32 size;

  const char * filename;

  int i;

  FILE * fp = fopen("old-hashes.txt", "w");

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
    murmur = Generate_Hash_x64_128(bitmap, murmur);

    fprintf(fp, "%08x%08x%08x%08x\n", murmur->hash[0], murmur->hash[1], murmur->hash[2], murmur->hash[3]);
  }

  fclose(fp);

  FT_Done_Face(face);
  FT_Done_FreeType(library);

  return 0;

}