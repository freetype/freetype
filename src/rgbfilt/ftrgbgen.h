/* this file contains a template for the RGB color filter algorithm
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
  FT_Byte*   out_line  = oper->out_line;
  FT_Long    out_pitch = oper->out_pitch;
  FT_Fixed*  mults     = oper->factors;

  for ( ; hh > 0; hh--, in_line += in_pitch*VMUL, out_line += out_pitch )
  {
    int         ww    = oper->width;
    FT_Byte*    read  = in_line;
    FT_UInt32*  write = (FT_UInt32*)out_line;

    for ( ; ww > 0; ww--, read += HMUL, write += 1 )
    {
      FT_UInt32  rr, gg, bb;
      FT_UInt    val;

      val = read[OFF_R];
      rr  = mults[0]*val;
      gg  = mults[3]*val;
      bb  = mults[6]*val;

      val = read[OFF_G];
      rr += mults[1]*val;
      gg += mults[4]*val;
      bb += mults[7]*val;

      val = read[OFF_B];
      rr += mults[2]*val;
      gg += mults[5]*val;
      bb += mults[8]*val;

      rr = (rr >> 16) & 255;
      gg = (gg >> 16) & 255;
      bb = (bb >> 16) & 255;

      write[0] = (FT_UInt32)( (gg << 24) | (rr << 16) | (gg << 8) | bb );
    }
  }

#undef  OFF_R
#undef  OFF_G
#undef  OFF_B
#undef  HMUL
#undef  VMUL
