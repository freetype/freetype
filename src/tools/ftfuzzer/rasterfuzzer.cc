// rasterfuzzer.cc
//
//   A fuzzing function to test FreeType's rasterizers with libFuzzer.
//
// Copyright 2016 by
// David Turner, Robert Wilhelm, and Werner Lemberg.
//
// This file is part of the FreeType project, and may only be used,
// modified, and distributed under the terms of the FreeType project
// license, LICENSE.TXT.  By continuing to use, modify, or distribute
// this file you indicate that you have read the license and
// understand and accept it fully.


#include <stdint.h>

#include <vector>


  using namespace std;


#include <ft2build.h>

#include FT_FREETYPE_H
#include FT_IMAGE_H
#include FT_OUTLINE_H


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


  extern "C" int
  LLVMFuzzerTestOneInput( const uint8_t*  data,
                          size_t          size_ )
  {
    unsigned char  pixels[4];

    FT_Bitmap  bitmap_mono = {
      1,                  // rows
      1,                  // width
      4,                  // pitch
      pixels,             // buffer
      2,                  // num_grays
      FT_PIXEL_MODE_MONO, // pixel_mode
      0,                  // palette_mode
      NULL                // palette
    };

    FT_Bitmap  bitmap_gray = {
      1,                  // rows
      1,                  // width
      4,                  // pitch
      pixels,             // buffer
      256,                // num_grays
      FT_PIXEL_MODE_GRAY, // pixel_mode
      0,                  // palette_mode
      NULL                // palette
    };

    short  n_points = short( size_ / sizeof ( FT_Vector ) );
    if ( n_points <= 2 )
      return 0;

    FT_Vector*  points = reinterpret_cast<FT_Vector*>(
                           const_cast<uint8_t*>( data ) );

    short  contours[1];
    contours[0] = n_points - 1;

    vector<char>  tags( (size_t)n_points );
    fill( tags.begin(), tags.end(), 1 );

    FT_Outline  outline =
    {
      1,                                      // n_contours
      n_points,                               // n_points
      points,                                 // points
      reinterpret_cast<char*>( tags.data() ), // tags
      contours,                               // contours
      FT_OUTLINE_NONE                         // flags
    };

    FT_Outline_Get_Bitmap( library, &outline, &bitmap_mono );
    FT_Outline_Get_Bitmap( library, &outline, &bitmap_gray );

    return 0;
  }


// END
