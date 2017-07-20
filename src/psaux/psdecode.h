#ifndef PSDECODE_H_
#define PSDECODE_H_


#include <ft2build.h>
#include FT_INTERNAL_POSTSCRIPT_AUX_H


FT_BEGIN_HEADER

  FT_LOCAL( void )
  ps_decoder_init( void*        decoder,
                   FT_Bool      is_t1,
                   PS_Decoder*  ps_decoder );


FT_END_HEADER

#endif

