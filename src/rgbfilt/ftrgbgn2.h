/* this file contains a template for the in-place RGB color filter algorithm
 * it is included several times by "ftrgb.c"
 */
#ifndef OFF_R
#error "OFF_R must be defined as the offset of the red channel"
#endif

#ifndef OFF_G
#error "OFF_G must be defiend as the offset of the green channel"
#endif

#ifndef OFF_B
#error "OFF_B must be defined as the offset of the blue channel"
#endif

#ifndef HMUL
#error "HMUL must be defined as the horizontal multiplier, either 1 or 3"
#endif

#ifndef VMUL
#error "VMUL must be defined as the vertical multipler, either 1 or 3"
#endif


  int        hh        = oper->height;
  FT_Byte*   in_line   = oper->in_line;
  FT_Long    in_pitch  = oper->in_pitch;
  FT_Fixed*  mults     = oper->factors;

  for ( ; hh > 0; hh--, in_line += in_pitch*VMUL )
  {
    int         ww   = oper->width;
    FT_Byte*    pix  = in_line;

    for ( ; ww > 0; ww--, pix += HMUL )
    {
      FT_UInt32  rr, gg, bb;
      FT_UInt    val;

      val = pix[OFF_R];
      rr  = mults[0]*val;
      gg  = mults[3]*val;
      bb  = mults[6]*val;

      val = pix[OFF_G];
      rr += mults[1]*val;
      gg += mults[4]*val;
      bb += mults[7]*val;

      val = pix[OFF_B];
      rr += mults[2]*val;
      gg += mults[5]*val;
      bb += mults[8]*val;

      pix[OFF_R] = (FT_Byte)(rr >> 16);
      pix[OFF_G] = (FT_Byte)(gg >> 16);
      pix[OFF_B] = (FT_Byte)(bb >> 16);
    }
  }

#undef  OFF_R
#undef  OFF_G
#undef  OFF_B
#undef  HMUL
#undef  VMUL
