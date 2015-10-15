// we use `unique_ptr' and `decltype', defined since C++11
#if __cplusplus < 201103L
#  error "a C++11 compiler is needed"
#endif

#include <assert.h>
#include <stdint.h>

#include <memory>
#include <vector>


using namespace std;


#include <ft2build.h>

#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_CACHE_H
#include FT_CACHE_CHARMAP_H
#include FT_CACHE_IMAGE_H
#include FT_CACHE_SMALL_BITMAPS_H
#include FT_SYNTHESIS_H
#include FT_ADVANCES_H
#include FT_OUTLINE_H
#include FT_BBOX_H
#include FT_MODULE_H
#include FT_CFF_DRIVER_H
#include FT_TRUETYPE_DRIVER_H
#include FT_MULTIPLE_MASTERS_H


  static FT_Library  library;
  static int         InitResult;

  struct FT_Global {
    FT_Global() {
      InitResult = FT_Init_FreeType( &library );
    }
    ~FT_Global() {
      FT_Done_FreeType( library );
    }
  };

  FT_Global  global_ft;


  static void
  setIntermediateAxis( FT_Face  face )
  {
    // only handle Multiple Masters and GX variation fonts
    if ( !( face->face_flags & FT_FACE_FLAG_MULTIPLE_MASTERS ) )
      return;

    // get variation data for current instance
    FT_MM_Var*  variations_ptr = nullptr;
    if ( FT_Get_MM_Var( face, &variations_ptr ) )
      return;

    unique_ptr<FT_MM_Var,
               decltype ( free )*>  variations( variations_ptr, free );
    vector<FT_Fixed>                coords( variations->num_axis );

    // select an arbitrary instance
    for ( unsigned int  i = 0; i < variations->num_axis; i++ )
      coords[i] = ( variations->axis[i].minimum +
                    variations->axis[i].def     ) / 2;

    if ( FT_Set_Var_Design_Coordinates( face,
                                        coords.size(),
                                        coords.data() ) )
      return;
  }


  extern "C" int
  LLVMFuzzerTestOneInput( const uint8_t*  data,
                          size_t          size_ )
  {
    assert( !InitResult );

    if ( size_ < 1 )
      return 0;

    long  size = (long)size_;

    FT_Face         face;
    FT_Int32        load_flags  = FT_LOAD_DEFAULT;
#if 0
    FT_Render_Mode  render_mode = FT_RENDER_MODE_NORMAL;
#endif

    // We use a conservative approach here, at the cost of calling
    // `FT_New_Face' quite often.  The idea is that the fuzzer should be
    // able to try all faces and named instances of a font, expecting that
    // some faces don't work for various reasons, e.g., a broken subfont, or
    // an unsupported NFNT bitmap font in a Mac dfont resource that holds
    // more than a single font.

    // get number of faces
    if ( FT_New_Memory_Face( library, data, size, -1, &face ) )
      return 0;
    long  num_faces = face->num_faces;
    FT_Done_Face( face );

    // loop over all faces
    for ( long  face_index = 0;
          face_index < num_faces;
          face_index++ )
    {
      // get number of instances
      if ( FT_New_Memory_Face( library,
                               data,
                               size,
                               -( face_index + 1 ),
                               &face ) )
        continue;
      long  num_instances = face->style_flags >> 16;
      FT_Done_Face( face );

      // load face with and without instances
      for ( long  instance_index = 0;
            instance_index < num_instances + 1;
            instance_index++ )
      {
        if ( FT_New_Memory_Face( library,
                                 data,
                                 size,
                                 ( instance_index << 16 ) + face_index,
                                 &face ) )
          continue;

        // loop over all bitmap stroke sizes
        // and an arbitrary size for outlines
        for ( long  fixed_sizes_index = 0;
              fixed_sizes_index < face->num_fixed_sizes + 1;
              fixed_sizes_index++ )
        {
          FT_Int32  flags = load_flags;

          if ( !fixed_sizes_index )
          {
            // set up 20pt at 72dpi as an arbitrary size
            FT_Set_Char_Size( face, 20, 20, 72, 72 );
            flags |= FT_LOAD_NO_BITMAP;
          }
          else
          {
            FT_Select_Size( face, fixed_sizes_index - 1 );
            flags |= FT_LOAD_COLOR;
          }

          // test MM interface only for a face without a selected instance
          if ( instance_index == 0 )
            setIntermediateAxis( face );

          // loop over all glyphs
          for ( unsigned int  glyph_index = 0;
                glyph_index < (unsigned int)face->num_glyphs;
                glyph_index++ )
          {
            if ( FT_Load_Glyph( face, glyph_index, flags ) )
              continue;

            // Rendering is the most expensive and the least interesting part.
            //
            // if ( FT_Render_Glyph( face->glyph, render_mode) )
            //   continue;
            // FT_GlyphSlot_Embolden( face->glyph );

#if 0
            FT_Glyph  glyph;
            if ( !FT_Get_Glyph( face->glyph, &glyph ) )
              FT_Done_Glyph( glyph );

            FT_Outline*  outline = &face->glyph->outline;
            FT_Matrix    rot30   = { 0xDDB4, -0x8000, 0x8000, 0xDDB4 };

            FT_Outline_Transform( outline, &rot30 );

            FT_BBox  bbox;
            FT_Outline_Get_BBox( outline, &bbox );
#endif
          }
        }
        FT_Done_Face( face );
      }
    }

    return 0;
  }


/* END */
