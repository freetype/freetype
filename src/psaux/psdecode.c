

#include <ft2build.h>
#include FT_INTERNAL_SERVICE_H
#include FT_SERVICE_CFF_TABLE_LOAD_H

#include "psdecode.h"
#include "psobjs.h"

#include "psauxerr.h"


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    ps_decoder_init                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Initializes a given glyph decoder.                                 */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    decoder :: A pointer to the glyph builder to initialize.           */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face      :: The current face object.                              */
  /*                                                                       */
  /*    size      :: The current size object.                              */
  /*                                                                       */
  /*    slot      :: The current glyph object.                             */
  /*                                                                       */
  /*    hinting   :: Whether hinting is active.                            */
  /*                                                                       */
  /*    hint_mode :: The hinting mode.                                     */
  /*                                                                       */
  FT_LOCAL_DEF( void )
  ps_decoder_init( PS_Decoder*     decoder,
                   TT_Face         face,
                   FT_Size         size,
                   CFF_GlyphSlot   slot,
                   FT_Byte**       glyph_names,
                   PS_Blend        blend,
                   FT_Bool         hinting,
                   FT_Render_Mode  hint_mode,
                   PS_Decoder_Get_Glyph_Callback   get_callback,
                   PS_Decoder_Free_Glyph_Callback  free_callback )
  {
  }


  /* this function is used to select the subfont */
  /* and the locals subrs array                  */
  FT_LOCAL_DEF( FT_Error )
  ps_decoder_prepare( PS_Decoder*  decoder,
                      FT_Size      size,
                      FT_UInt      glyph_index )
  {
  }
