#include <ft2build.h>
#include FT_GASP_H
#include FT_INTERNAL_TRUETYPE_TYPES_H

  FT_EXPORT_DEF( FT_Int )
  FT_Get_Gasp( FT_Face    face,
               FT_UInt    ppem )
  {
    FT_Int  result = FT_GASP_NO_TABLE;

    if ( face && FT_IS_SFNT(face) )
    {
      TT_Face  ttface = (TT_Face)face;

      if ( ttface->gasp.numRanges > 0 )
      {
        TT_GaspRange  range     = ttface->gasp.gaspRanges;
        TT_GaspRange  range_end = range + ttface->gasp.numRanges;

        while ( ppem > range->maxPPEM )
        {
          range++;
          if ( range >= range_end )
            goto Exit;
        }

        result = range->gaspFlag;

        /* ensure we don't have spurious bits */
        if ( ttface->gasp.version == 0 )
          result &= 3;
      }
    }
  Exit:
    return result;
  }


