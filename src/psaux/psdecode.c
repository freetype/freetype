

#include <ft2build.h>
#include FT_INTERNAL_SERVICE_H

#include "psdecode.h"
#include "psobjs.h"

#include "psauxerr.h"


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    ps_decoder_init                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Creates a decoder for the combined Type 1 / CFF interpreter.       */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    decoder :: A pointer to the glyph builder to initialize.           */
  /*                                                                       */
  /* <Input>                                                               */
  /*                                                                       */
  /*                                                                       */
  /*                                                                       */
  /*                                                                       */
  /*                                                                       */
  FT_LOCAL_DEF( void )
  ps_decoder_init( void*        decoder,
                   FT_Bool      is_t1,
                   PS_Decoder*  ps_decoder )
  {
    FT_ZERO( ps_decoder );

    if ( is_t1 )
    {
      T1_Decoder  t1_decoder = (T1_Decoder)decoder;

      ps_builder_init( &t1_decoder->builder,
                       is_t1,
                       &ps_decoder->builder );

      ps_decoder->psnames            =  t1_decoder->psnames;

      ps_decoder->num_glyphs         =  t1_decoder->num_glyphs;
      ps_decoder->glyph_names        =  t1_decoder->glyph_names;
      ps_decoder->hint_mode          =  t1_decoder->hint_mode;
      ps_decoder->blend              =  t1_decoder->blend;
      /* ps_decoder->t1_parse_callback  =  t1_decoder->parse_callback; */

      ps_decoder->num_locals         =  t1_decoder->num_subrs;
      ps_decoder->locals             =  t1_decoder->subrs;
      ps_decoder->locals_len         =  t1_decoder->subrs_len;
      ps_decoder->locals_hash        =  t1_decoder->subrs_hash;

      ps_decoder->buildchar          =  t1_decoder->buildchar;
      ps_decoder->len_buildchar      =  t1_decoder->len_buildchar;

      ps_decoder->lenIV              =  t1_decoder->lenIV;
    }
    else
    {
      CFF_Decoder*  cff_decoder = (CFF_Decoder*)decoder;

      ps_builder_init( &cff_decoder->builder,
                       is_t1,
                       &ps_decoder->builder );

      ps_decoder->cff                 =  cff_decoder->cff;
      ps_decoder->current_subfont     =  cff_decoder->current_subfont;

      ps_decoder->num_globals         =  cff_decoder->num_globals;
      ps_decoder->globals             =  cff_decoder->globals;
      ps_decoder->globals_bias        =  cff_decoder->globals_bias;
      ps_decoder->num_locals          =  cff_decoder->num_locals;
      ps_decoder->locals              =  cff_decoder->locals;
      ps_decoder->locals_bias         =  cff_decoder->locals_bias;

      ps_decoder->glyph_width         =  cff_decoder->glyph_width;
      ps_decoder->nominal_width       =  cff_decoder->nominal_width;
      ps_decoder->width_only          =  cff_decoder->width_only;

      ps_decoder->hint_mode           =  cff_decoder->hint_mode;

      ps_decoder->get_glyph_callback  =  cff_decoder->get_glyph_callback;
      ps_decoder->free_glyph_callback =  cff_decoder->free_glyph_callback;
    }
  }

