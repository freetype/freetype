#include <stdio.h>
#include <stdlib.h>

#ifdef UNIX
#ifndef HAVE_POSIX_TERMIOS
#include <sys/ioctl.h>
#include <termio.h>
#else
#ifndef HAVE_TCGETATTR
#define HAVE_TCGETATTR
#endif /* HAVE_TCGETATTR */
#ifndef HAVE_TCSETATTR
#define HAVE_TCSETATTR
#endif /* HAVE_TCSETATTR */
#include <termios.h>
#endif /* HAVE_POSIX_TERMIOS */
#endif

/* Define the `getch()' function.  On Unix systems, it is an alias  */
/* for `getchar()', and the debugger front end must ensure that the */
/* `stdin' file descriptor is not in line-by-line input mode.       */
#ifndef UNIX
#include <conio.h>
#else
#define getch  getchar
#endif


#include "freetype.h"
#include "ttobjs.h"
#include "ttdriver.h"
#include "ttinterp.h"


FT_Library      library;    /* root library object */
FT_Memory       memory;     /* system object */
FT_Driver       driver;     /* truetype driver */
TT_Face         face;       /* truetype face */
TT_Size         size;       /* truetype size */
TT_GlyphSlot    glyph;      /* truetype glyph slot */
TT_ExecContext  exec;       /* truetype execution context */
TT_Error        error;

TT_CodeRange_Tag  debug_coderange = tt_coderange_glyph;

  typedef FT_Byte ByteStr[2];
  typedef FT_Byte WordStr[4];
  typedef FT_Byte LongStr[8];
  typedef FT_Byte DebugStr[128];

  static  DebugStr tempStr;

#undef  PACK
#define PACK( x, y )  ((x << 4) | y)

  static const TT_Byte  Pop_Push_Count[256] =
  {
    /* opcodes are gathered in groups of 16 */
    /* please keep the spaces as they are   */

    /*  SVTCA  y  */  PACK( 0, 0 ),
    /*  SVTCA  x  */  PACK( 0, 0 ),
    /*  SPvTCA y  */  PACK( 0, 0 ),
    /*  SPvTCA x  */  PACK( 0, 0 ),
    /*  SFvTCA y  */  PACK( 0, 0 ),
    /*  SFvTCA x  */  PACK( 0, 0 ),
    /*  SPvTL //  */  PACK( 2, 0 ),
    /*  SPvTL +   */  PACK( 2, 0 ),
    /*  SFvTL //  */  PACK( 2, 0 ),
    /*  SFvTL +   */  PACK( 2, 0 ),
    /*  SPvFS     */  PACK( 2, 0 ),
    /*  SFvFS     */  PACK( 2, 0 ),
    /*  GPV       */  PACK( 0, 2 ),
    /*  GFV       */  PACK( 0, 2 ),
    /*  SFvTPv    */  PACK( 0, 0 ),
    /*  ISECT     */  PACK( 5, 0 ),

    /*  SRP0      */  PACK( 1, 0 ),
    /*  SRP1      */  PACK( 1, 0 ),
    /*  SRP2      */  PACK( 1, 0 ),
    /*  SZP0      */  PACK( 1, 0 ),
    /*  SZP1      */  PACK( 1, 0 ),
    /*  SZP2      */  PACK( 1, 0 ),
    /*  SZPS      */  PACK( 1, 0 ),
    /*  SLOOP     */  PACK( 1, 0 ),
    /*  RTG       */  PACK( 0, 0 ),
    /*  RTHG      */  PACK( 0, 0 ),
    /*  SMD       */  PACK( 1, 0 ),
    /*  ELSE      */  PACK( 0, 0 ),
    /*  JMPR      */  PACK( 1, 0 ),
    /*  SCvTCi    */  PACK( 1, 0 ),
    /*  SSwCi     */  PACK( 1, 0 ),
    /*  SSW       */  PACK( 1, 0 ),

    /*  DUP       */  PACK( 1, 2 ),
    /*  POP       */  PACK( 1, 0 ),
    /*  CLEAR     */  PACK( 0, 0 ),
    /*  SWAP      */  PACK( 2, 2 ),
    /*  DEPTH     */  PACK( 0, 1 ),
    /*  CINDEX    */  PACK( 1, 1 ),
    /*  MINDEX    */  PACK( 1, 0 ),
    /*  AlignPTS  */  PACK( 2, 0 ),
    /*  INS_$28   */  PACK( 0, 0 ),
    /*  UTP       */  PACK( 1, 0 ),
    /*  LOOPCALL  */  PACK( 2, 0 ),
    /*  CALL      */  PACK( 1, 0 ),
    /*  FDEF      */  PACK( 1, 0 ),
    /*  ENDF      */  PACK( 0, 0 ),
    /*  MDAP[0]   */  PACK( 1, 0 ),
    /*  MDAP[1]   */  PACK( 1, 0 ),

    /*  IUP[0]    */  PACK( 0, 0 ),
    /*  IUP[1]    */  PACK( 0, 0 ),
    /*  SHP[0]    */  PACK( 0, 0 ),
    /*  SHP[1]    */  PACK( 0, 0 ),
    /*  SHC[0]    */  PACK( 1, 0 ),
    /*  SHC[1]    */  PACK( 1, 0 ),
    /*  SHZ[0]    */  PACK( 1, 0 ),
    /*  SHZ[1]    */  PACK( 1, 0 ),
    /*  SHPIX     */  PACK( 1, 0 ),
    /*  IP        */  PACK( 0, 0 ),
    /*  MSIRP[0]  */  PACK( 2, 0 ),
    /*  MSIRP[1]  */  PACK( 2, 0 ),
    /*  AlignRP   */  PACK( 0, 0 ),
    /*  RTDG      */  PACK( 0, 0 ),
    /*  MIAP[0]   */  PACK( 2, 0 ),
    /*  MIAP[1]   */  PACK( 2, 0 ),

    /*  NPushB    */  PACK( 0, 0 ),
    /*  NPushW    */  PACK( 0, 0 ),
    /*  WS        */  PACK( 2, 0 ),
    /*  RS        */  PACK( 1, 1 ),
    /*  WCvtP     */  PACK( 2, 0 ),
    /*  RCvt      */  PACK( 1, 1 ),
    /*  GC[0]     */  PACK( 1, 1 ),
    /*  GC[1]     */  PACK( 1, 1 ),
    /*  SCFS      */  PACK( 2, 0 ),
    /*  MD[0]     */  PACK( 2, 1 ),
    /*  MD[1]     */  PACK( 2, 1 ),
    /*  MPPEM     */  PACK( 0, 1 ),
    /*  MPS       */  PACK( 0, 1 ),
    /*  FlipON    */  PACK( 0, 0 ),
    /*  FlipOFF   */  PACK( 0, 0 ),
    /*  DEBUG     */  PACK( 1, 0 ),

    /*  LT        */  PACK( 2, 1 ),
    /*  LTEQ      */  PACK( 2, 1 ),
    /*  GT        */  PACK( 2, 1 ),
    /*  GTEQ      */  PACK( 2, 1 ),
    /*  EQ        */  PACK( 2, 1 ),
    /*  NEQ       */  PACK( 2, 1 ),
    /*  ODD       */  PACK( 1, 1 ),
    /*  EVEN      */  PACK( 1, 1 ),
    /*  IF        */  PACK( 1, 0 ),
    /*  EIF       */  PACK( 0, 0 ),
    /*  AND       */  PACK( 2, 1 ),
    /*  OR        */  PACK( 2, 1 ),
    /*  NOT       */  PACK( 1, 1 ),
    /*  DeltaP1   */  PACK( 1, 0 ),
    /*  SDB       */  PACK( 1, 0 ),
    /*  SDS       */  PACK( 1, 0 ),

    /*  ADD       */  PACK( 2, 1 ),
    /*  SUB       */  PACK( 2, 1 ),
    /*  DIV       */  PACK( 2, 1 ),
    /*  MUL       */  PACK( 2, 1 ),
    /*  ABS       */  PACK( 1, 1 ),
    /*  NEG       */  PACK( 1, 1 ),
    /*  FLOOR     */  PACK( 1, 1 ),
    /*  CEILING   */  PACK( 1, 1 ),
    /*  ROUND[0]  */  PACK( 1, 1 ),
    /*  ROUND[1]  */  PACK( 1, 1 ),
    /*  ROUND[2]  */  PACK( 1, 1 ),
    /*  ROUND[3]  */  PACK( 1, 1 ),
    /*  NROUND[0] */  PACK( 1, 1 ),
    /*  NROUND[1] */  PACK( 1, 1 ),
    /*  NROUND[2] */  PACK( 1, 1 ),
    /*  NROUND[3] */  PACK( 1, 1 ),

    /*  WCvtF     */  PACK( 2, 0 ),
    /*  DeltaP2   */  PACK( 1, 0 ),
    /*  DeltaP3   */  PACK( 1, 0 ),
    /*  DeltaCn[0] */ PACK( 1, 0 ),
    /*  DeltaCn[1] */ PACK( 1, 0 ),
    /*  DeltaCn[2] */ PACK( 1, 0 ),
    /*  SROUND    */  PACK( 1, 0 ),
    /*  S45Round  */  PACK( 1, 0 ),
    /*  JROT      */  PACK( 2, 0 ),
    /*  JROF      */  PACK( 2, 0 ),
    /*  ROFF      */  PACK( 0, 0 ),
    /*  INS_$7B   */  PACK( 0, 0 ),
    /*  RUTG      */  PACK( 0, 0 ),
    /*  RDTG      */  PACK( 0, 0 ),
    /*  SANGW     */  PACK( 1, 0 ),
    /*  AA        */  PACK( 1, 0 ),

    /*  FlipPT    */  PACK( 0, 0 ),
    /*  FlipRgON  */  PACK( 2, 0 ),
    /*  FlipRgOFF */  PACK( 2, 0 ),
    /*  INS_$83   */  PACK( 0, 0 ),
    /*  INS_$84   */  PACK( 0, 0 ),
    /*  ScanCTRL  */  PACK( 1, 0 ),
    /*  SDVPTL[0] */  PACK( 2, 0 ),
    /*  SDVPTL[1] */  PACK( 2, 0 ),
    /*  GetINFO   */  PACK( 1, 1 ),
    /*  IDEF      */  PACK( 1, 0 ),
    /*  ROLL      */  PACK( 3, 3 ),
    /*  MAX       */  PACK( 2, 1 ),
    /*  MIN       */  PACK( 2, 1 ),
    /*  ScanTYPE  */  PACK( 1, 0 ),
    /*  InstCTRL  */  PACK( 2, 0 ),
    /*  INS_$8F   */  PACK( 0, 0 ),

    /*  INS_$90  */   PACK( 0, 0 ),
    /*  INS_$91  */   PACK( 0, 0 ),
    /*  INS_$92  */   PACK( 0, 0 ),
    /*  INS_$93  */   PACK( 0, 0 ),
    /*  INS_$94  */   PACK( 0, 0 ),
    /*  INS_$95  */   PACK( 0, 0 ),
    /*  INS_$96  */   PACK( 0, 0 ),
    /*  INS_$97  */   PACK( 0, 0 ),
    /*  INS_$98  */   PACK( 0, 0 ),
    /*  INS_$99  */   PACK( 0, 0 ),
    /*  INS_$9A  */   PACK( 0, 0 ),
    /*  INS_$9B  */   PACK( 0, 0 ),
    /*  INS_$9C  */   PACK( 0, 0 ),
    /*  INS_$9D  */   PACK( 0, 0 ),
    /*  INS_$9E  */   PACK( 0, 0 ),
    /*  INS_$9F  */   PACK( 0, 0 ),

    /*  INS_$A0  */   PACK( 0, 0 ),
    /*  INS_$A1  */   PACK( 0, 0 ),
    /*  INS_$A2  */   PACK( 0, 0 ),
    /*  INS_$A3  */   PACK( 0, 0 ),
    /*  INS_$A4  */   PACK( 0, 0 ),
    /*  INS_$A5  */   PACK( 0, 0 ),
    /*  INS_$A6  */   PACK( 0, 0 ),
    /*  INS_$A7  */   PACK( 0, 0 ),
    /*  INS_$A8  */   PACK( 0, 0 ),
    /*  INS_$A9  */   PACK( 0, 0 ),
    /*  INS_$AA  */   PACK( 0, 0 ),
    /*  INS_$AB  */   PACK( 0, 0 ),
    /*  INS_$AC  */   PACK( 0, 0 ),
    /*  INS_$AD  */   PACK( 0, 0 ),
    /*  INS_$AE  */   PACK( 0, 0 ),
    /*  INS_$AF  */   PACK( 0, 0 ),

    /*  PushB[0]  */  PACK( 0, 1 ),
    /*  PushB[1]  */  PACK( 0, 2 ),
    /*  PushB[2]  */  PACK( 0, 3 ),
    /*  PushB[3]  */  PACK( 0, 4 ),
    /*  PushB[4]  */  PACK( 0, 5 ),
    /*  PushB[5]  */  PACK( 0, 6 ),
    /*  PushB[6]  */  PACK( 0, 7 ),
    /*  PushB[7]  */  PACK( 0, 8 ),
    /*  PushW[0]  */  PACK( 0, 1 ),
    /*  PushW[1]  */  PACK( 0, 2 ),
    /*  PushW[2]  */  PACK( 0, 3 ),
    /*  PushW[3]  */  PACK( 0, 4 ),
    /*  PushW[4]  */  PACK( 0, 5 ),
    /*  PushW[5]  */  PACK( 0, 6 ),
    /*  PushW[6]  */  PACK( 0, 7 ),
    /*  PushW[7]  */  PACK( 0, 8 ),

    /*  MDRP[00]  */  PACK( 1, 0 ),
    /*  MDRP[01]  */  PACK( 1, 0 ),
    /*  MDRP[02]  */  PACK( 1, 0 ),
    /*  MDRP[03]  */  PACK( 1, 0 ),
    /*  MDRP[04]  */  PACK( 1, 0 ),
    /*  MDRP[05]  */  PACK( 1, 0 ),
    /*  MDRP[06]  */  PACK( 1, 0 ),
    /*  MDRP[07]  */  PACK( 1, 0 ),
    /*  MDRP[08]  */  PACK( 1, 0 ),
    /*  MDRP[09]  */  PACK( 1, 0 ),
    /*  MDRP[10]  */  PACK( 1, 0 ),
    /*  MDRP[11]  */  PACK( 1, 0 ),
    /*  MDRP[12]  */  PACK( 1, 0 ),
    /*  MDRP[13]  */  PACK( 1, 0 ),
    /*  MDRP[14]  */  PACK( 1, 0 ),
    /*  MDRP[15]  */  PACK( 1, 0 ),

    /*  MDRP[16]  */  PACK( 1, 0 ),
    /*  MDRP[17]  */  PACK( 1, 0 ),
    /*  MDRP[18]  */  PACK( 1, 0 ),
    /*  MDRP[19]  */  PACK( 1, 0 ),
    /*  MDRP[20]  */  PACK( 1, 0 ),
    /*  MDRP[21]  */  PACK( 1, 0 ),
    /*  MDRP[22]  */  PACK( 1, 0 ),
    /*  MDRP[23]  */  PACK( 1, 0 ),
    /*  MDRP[24]  */  PACK( 1, 0 ),
    /*  MDRP[25]  */  PACK( 1, 0 ),
    /*  MDRP[26]  */  PACK( 1, 0 ),
    /*  MDRP[27]  */  PACK( 1, 0 ),
    /*  MDRP[28]  */  PACK( 1, 0 ),
    /*  MDRP[29]  */  PACK( 1, 0 ),
    /*  MDRP[30]  */  PACK( 1, 0 ),
    /*  MDRP[31]  */  PACK( 1, 0 ),

    /*  MIRP[00]  */  PACK( 2, 0 ),
    /*  MIRP[01]  */  PACK( 2, 0 ),
    /*  MIRP[02]  */  PACK( 2, 0 ),
    /*  MIRP[03]  */  PACK( 2, 0 ),
    /*  MIRP[04]  */  PACK( 2, 0 ),
    /*  MIRP[05]  */  PACK( 2, 0 ),
    /*  MIRP[06]  */  PACK( 2, 0 ),
    /*  MIRP[07]  */  PACK( 2, 0 ),
    /*  MIRP[08]  */  PACK( 2, 0 ),
    /*  MIRP[09]  */  PACK( 2, 0 ),
    /*  MIRP[10]  */  PACK( 2, 0 ),
    /*  MIRP[11]  */  PACK( 2, 0 ),
    /*  MIRP[12]  */  PACK( 2, 0 ),
    /*  MIRP[13]  */  PACK( 2, 0 ),
    /*  MIRP[14]  */  PACK( 2, 0 ),
    /*  MIRP[15]  */  PACK( 2, 0 ),

    /*  MIRP[16]  */  PACK( 2, 0 ),
    /*  MIRP[17]  */  PACK( 2, 0 ),
    /*  MIRP[18]  */  PACK( 2, 0 ),
    /*  MIRP[19]  */  PACK( 2, 0 ),
    /*  MIRP[20]  */  PACK( 2, 0 ),
    /*  MIRP[21]  */  PACK( 2, 0 ),
    /*  MIRP[22]  */  PACK( 2, 0 ),
    /*  MIRP[23]  */  PACK( 2, 0 ),
    /*  MIRP[24]  */  PACK( 2, 0 ),
    /*  MIRP[25]  */  PACK( 2, 0 ),
    /*  MIRP[26]  */  PACK( 2, 0 ),
    /*  MIRP[27]  */  PACK( 2, 0 ),
    /*  MIRP[28]  */  PACK( 2, 0 ),
    /*  MIRP[29]  */  PACK( 2, 0 ),
    /*  MIRP[30]  */  PACK( 2, 0 ),
    /*  MIRP[31]  */  PACK( 2, 0 )
  };


  static const FT_String*  OpStr[256] = {
            "SVTCA y",       /* Set vectors to coordinate axis y    */
            "SVTCA x",       /* Set vectors to coordinate axis x    */
            "SPvTCA y",      /* Set Proj. vec. to coord. axis y     */
            "SPvTCA x",      /* Set Proj. vec. to coord. axis x     */
            "SFvTCA y",      /* Set Free. vec. to coord. axis y     */
            "SFvTCA x",      /* Set Free. vec. to coord. axis x     */
            "SPvTL //",      /* Set Proj. vec. parallel to segment  */
            "SPvTL +",       /* Set Proj. vec. normal to segment    */
            "SFvTL //",      /* Set Free. vec. parallel to segment  */
            "SFvTL +",       /* Set Free. vec. normal to segment    */
            "SPvFS",         /* Set Proj. vec. from stack           */
            "SFvFS",         /* Set Free. vec. from stack           */
            "GPV",           /* Get projection vector               */
            "GFV",           /* Get freedom vector                  */
            "SFvTPv",        /* Set free. vec. to proj. vec.        */
            "ISECT",         /* compute intersection                */

            "SRP0",          /* Set reference point 0               */
            "SRP1",          /* Set reference point 1               */
            "SRP2",          /* Set reference point 2               */
            "SZP0",          /* Set Zone Pointer 0                  */
            "SZP1",          /* Set Zone Pointer 1                  */
            "SZP2",          /* Set Zone Pointer 2                  */
            "SZPS",          /* Set all zone pointers               */
            "SLOOP",         /* Set loop counter                    */
            "RTG",           /* Round to Grid                       */
            "RTHG",          /* Round to Half-Grid                  */
            "SMD",           /* Set Minimum Distance                */
            "ELSE",          /* Else                                */
            "JMPR",          /* Jump Relative                       */
            "SCvTCi",        /* Set CVT                             */
            "SSwCi",         /*                                     */
            "SSW",           /*                                     */

            "DUP",
            "POP",
            "CLEAR",
            "SWAP",
            "DEPTH",
            "CINDEX",
            "MINDEX",
            "AlignPTS",
            "INS_$28",
            "UTP",
            "LOOPCALL",
            "CALL",
            "FDEF",
            "ENDF",
            "MDAP[-]",
            "MDAP[r]",

            "IUP[y]",
            "IUP[x]",
            "SHP[0]",
            "SHP[1]",
            "SHC[0]",
            "SHC[1]",
            "SHZ[0]",
            "SHZ[1]",
            "SHPIX",
            "IP",
            "MSIRP[0]",
            "MSIRP[1]",
            "AlignRP",
            "RTDG",
            "MIAP[-]",
            "MIAP[r]",

            "NPushB",
            "NPushW",
            "WS",
            "RS",
            "WCvtP",
            "RCvt",
            "GC[0]",
            "GC[1]",
            "SCFS",
            "MD[0]",
            "MD[1]",
            "MPPEM",
            "MPS",
            "FlipON",
            "FlipOFF",
            "DEBUG",

            "LT",
            "LTEQ",
            "GT",
            "GTEQ",
            "EQ",
            "NEQ",
            "ODD",
            "EVEN",
            "IF",
            "EIF",
            "AND",
            "OR",
            "NOT",
            "DeltaP1",
            "SDB",
            "SDS",

            "ADD",
            "SUB",
            "DIV",
            "MUL",
            "ABS",
            "NEG",
            "FLOOR",
            "CEILING",
            "ROUND[G]",
            "ROUND[B]",
            "ROUND[W]",
            "ROUND[?]",
            "NROUND[G]",
            "NROUND[B]",
            "NROUND[W]",
            "NROUND[?]",

            "WCvtF",
            "DeltaP2",
            "DeltaP3",
            "DeltaC1",
            "DeltaC2",
            "DeltaC3",
            "SROUND",
            "S45Round",
            "JROT",
            "JROF",
            "ROFF",
            "INS_$7B",
            "RUTG",
            "RDTG",
            "SANGW",
            "AA",

            "FlipPT",
            "FlipRgON",
            "FlipRgOFF",
            "INS_$83",
            "INS_$84",
            "ScanCTRL",
            "SDPVTL[0]",
            "SDPVTL[1]",
            "GetINFO",
            "IDEF",
            "ROLL",
            "MAX",
            "MIN",
            "ScanTYPE",
            "IntCTRL",
            "INS_$8F",

            "INS_$90",
            "INS_$91",
            "INS_$92",
            "INS_$93",
            "INS_$94",
            "INS_$95",
            "INS_$96",
            "INS_$97",
            "INS_$98",
            "INS_$99",
            "INS_$9A",
            "INS_$9B",
            "INS_$9C",
            "INS_$9D",
            "INS_$9E",
            "INS_$9F",

            "INS_$A0",
            "INS_$A1",
            "INS_$A2",
            "INS_$A3",
            "INS_$A4",
            "INS_$A5",
            "INS_$A6",
            "INS_$A7",
            "INS_$A8",
            "INS_$A9",
            "INS_$AA",
            "INS_$AB",
            "INS_$AC",
            "INS_$AD",
            "INS_$AE",
            "INS_$AF",

            "PushB[0]",
            "PushB[1]",
            "PushB[2]",
            "PushB[3]",
            "PushB[4]",
            "PushB[5]",
            "PushB[6]",
            "PushB[7]",
            "PushW[0]",
            "PushW[1]",
            "PushW[2]",
            "PushW[3]",
            "PushW[4]",
            "PushW[5]",
            "PushW[6]",
            "PushW[7]",

            "MDRP[G]",
            "MDRP[B]",
            "MDRP[W]",
            "MDRP[?]",
            "MDRP[rG]",
            "MDRP[rB]",
            "MDRP[rW]",
            "MDRP[r?]",
            "MDRP[mG]",
            "MDRP[mB]",
            "MDRP[mW]",
            "MDRP[m?]",
            "MDRP[mrG]",
            "MDRP[mrB]",
            "MDRP[mrW]",
            "MDRP[mr?]",
            "MDRP[pG]",
            "MDRP[pB]",

            "MDRP[pW]",
            "MDRP[p?]",
            "MDRP[prG]",
            "MDRP[prB]",
            "MDRP[prW]",
            "MDRP[pr?]",
            "MDRP[pmG]",
            "MDRP[pmB]",
            "MDRP[pmW]",
            "MDRP[pm?]",
            "MDRP[pmrG]",
            "MDRP[pmrB]",
            "MDRP[pmrW]",
            "MDRP[pmr?]",

            "MIRP[G]",
            "MIRP[B]",
            "MIRP[W]",
            "MIRP[?]",
            "MIRP[rG]",
            "MIRP[rB]",
            "MIRP[rW]",
            "MIRP[r?]",
            "MIRP[mG]",
            "MIRP[mB]",
            "MIRP[mW]",
            "MIRP[m?]",
            "MIRP[mrG]",
            "MIRP[mrB]",
            "MIRP[mrW]",
            "MIRP[mr?]",
            "MIRP[pG]",
            "MIRP[pB]",

            "MIRP[pW]",
            "MIRP[p?]",
            "MIRP[prG]",
            "MIRP[prB]",
            "MIRP[prW]",
            "MIRP[pr?]",
            "MIRP[pmG]",
            "MIRP[pmB]",
            "MIRP[pmW]",
            "MIRP[pm?]",
            "MIRP[pmrG]",
            "MIRP[pmrB]",
            "MIRP[pmrW]",
            "MIRP[pmr?]"
          };


/*********************************************************************
 *
 * Init_Keyboard : set the input file descriptor to char-by-char
 *                 mode on Unix..
 *
 *********************************************************************/

#ifdef UNIX

 struct termios  old_termio;
 
 static
 void Init_Keyboard( void )
 {
   struct termios  termio;

#ifndef HAVE_TCGETATTR
   ioctl( 0, TCGETS, &old_termio ); 
#else
   tcgetattr( 0, &old_termio );
#endif

   termio = old_termio;
   
/*   termio.c_lflag &= ~(ICANON+ECHO+ECHOE+ECHOK+ECHONL+ECHOKE); */
   termio.c_lflag &= ~(ICANON+ECHO+ECHOE+ECHOK+ECHONL);

#ifndef HAVE_TCSETATTR
   ioctl( 0, TCSETS, &termio );
#else
   tcsetattr( 0, TCSANOW, &termio );
#endif
 }

 static
 void Reset_Keyboard( void )
 {
#ifndef HAVE_TCSETATTR
   ioctl( 0, TCSETS, &old_termio );
#else
   tcsetattr( 0, TCSANOW, &old_termio );
#endif

 }

#else

 static
 void Init_Keyboard( void )
 {
 }

 static
 void Reset_Keyboard( void )
 {
 }

#endif


  void Panic( const char* message )
  {
    fprintf( stderr, "%s\n  error code = 0x%04x\n", message, error );
    Reset_Keyboard();
    exit(1);
  }


/******************************************************************
 *
 *  Function    :  Calc_Length
 *
 *  Description :  Computes the length in bytes of current opcode.
 *
 *****************************************************************/

#define CUR  (*exc)


  static void  Calc_Length( TT_ExecContext  exc )
  {
    CUR.opcode = CUR.code[CUR.IP];

    switch ( CUR.opcode )
    {
    case 0x40:
      if ( CUR.IP + 1 >= CUR.codeSize )
        Panic( "code range overflow !!" );

      CUR.length = CUR.code[CUR.IP + 1] + 2;
      break;

    case 0x41:
      if ( CUR.IP + 1 >= CUR.codeSize )
        Panic( "code range overflow !!" );

      CUR.length = CUR.code[CUR.IP + 1] * 2 + 2;
      break;

    case 0xB0:
    case 0xB1:
    case 0xB2:
    case 0xB3:
    case 0xB4:
    case 0xB5:
    case 0xB6:
    case 0xB7:
      CUR.length = CUR.opcode - 0xB0 + 2;
      break;

    case 0xB8:
    case 0xB9:
    case 0xBA:
    case 0xBB:
    case 0xBC:
    case 0xBD:
    case 0xBE:
    case 0xBF:
      CUR.length = (CUR.opcode - 0xB8) * 2 + 3;
      break;

    default:
      CUR.length = 1;
      break;
    }

    /* make sure result is in range */

    if ( CUR.IP + CUR.length > CUR.codeSize )
      Panic( "code range overflow !!" );
  }


  /* Disassemble the current line */
  /*                              */
  const FT_String* Cur_U_Line( TT_ExecContext  exec )
  {
    FT_String  s[32];
    FT_Int op, i, n;

    op = exec->code[ exec->IP ];

    sprintf( tempStr, "%04lx: %02hx  %s", exec->IP, op, OpStr[op] );

    if ( op == 0x40 )
    {
      n = exec->code[ exec->IP+1 ];
      sprintf( s, "(%d)", n );
      strncat( tempStr, s, 8 );

      if ( n > 20 ) n = 20; /* limit output */

      for ( i = 0; i < n; i++ )
      {
        sprintf( s, " $%02hx", exec->code[ exec->IP+i+2 ] );
        strncat( tempStr, s, 8 );
      }
    }
    else if ( op == 0x41 )
    {
      n = exec->code[ exec->IP+1 ];
      sprintf( s, "(%d)", n );
      strncat( tempStr, s, 8 );

      if (n > 20) n = 20; /* limit output */

      for ( i = 0; i < n; i++ )
      {
        sprintf( s, " $%02hx%02hx", exec->code[ exec->IP+i*2+2 ],
                                    exec->code[ exec->IP+i*2+3 ] );
        strncat( tempStr, s, 8 );
      }
    }
    else if ( (op & 0xF8) == 0xB0 )
    {
      n = op-0xB0;

      for ( i=0; i <= n; i++ )
      {
        sprintf( s, " $%02hx", exec->code[ exec->IP+i+1 ] );
        strncat( tempStr, s, 8 );
      }
    }
    else if ( (op & 0xF8) == 0xB8 )
    {
      n = op-0xB8;

      for ( i = 0; i <= n; i++ )
      {
        sprintf( s, " $%02hx%02hx", exec->code[ exec->IP+i*2+1 ],
                                  exec->code[ exec->IP+i*2+2 ] );
        strncat( tempStr, s, 8 );
      }
    }

    return (FT_String*)tempStr;
  }


  static
  TT_Error  RunIns( TT_ExecContext  exc )
  {
    FT_Int    A, diff, key;
    FT_Long   next_IP;
    FT_Char   ch, oldch = '\0', *temp;

    TT_Error  error = 0;

    FT_GlyphZone  save;
    FT_GlyphZone  pts;

    const FT_String*  round_str[8] =
    {
      "to half-grid",
      "to grid",
      "to double grid",
      "down to grid",
      "up to grid",
      "off",
      "super",
      "super 45"
    };

    /* only debug the requested code range */
    if (exc->curRange != (TT_Int)debug_coderange)
      return TT_RunIns(exc);

    exc->pts.n_points   = exc->zp0.n_points;
    exc->pts.n_contours = exc->zp0.n_contours;

    pts = exc->pts;
    

    save.n_points   = pts.n_points;
    save.n_contours = pts.n_contours;

    save.org   = (TT_Vector*)malloc( 2 * sizeof( TT_F26Dot6 ) *
                                       save.n_points );
    save.cur   = (TT_Vector*)malloc( 2 * sizeof( TT_F26Dot6 ) *
                                       save.n_points );
    save.tags = (TT_Byte*)malloc( save.n_points );

    exc->instruction_trap = 1;

    do
    {
      if ( CUR.IP < CUR.codeSize )
      {
        Calc_Length( exc );

        CUR.args = CUR.top - (Pop_Push_Count[CUR.opcode] >> 4);

        /* `args' is the top of the stack once arguments have been popped. */
        /* One can also interpret it as the index of the last argument.    */

        /* Print the current line.  We use a 80-columns console with the   */
        /* following formatting:                                           */
        /*                                                                 */
        /* [loc]:[addr] [opcode]  [disassemby]          [a][b]|[c][d]      */
        /*                                                                 */

        {
          char      temp[80];
          int       n, col, pop;
          int       args = CUR.args;

          sprintf( temp, "%78c\n", ' ' );

          /* first letter of location */
          switch ( CUR.curRange )
          {
          case tt_coderange_glyph:
            temp[0] = 'g';
            break;
          case tt_coderange_cvt:
            temp[0] = 'c';
            break;
          default:
            temp[0] = 'f';
          }

          /* current IP */
          sprintf( temp+1, "%04lx: %02x  %-36.36s",
                   CUR.IP,
                   CUR.opcode,
                   Cur_U_Line(&CUR) );

          strncpy( temp+46, " (", 2 );

          args = CUR.top - 1;
          pop  = Pop_Push_Count[CUR.opcode] >> 4;
          col  = 48;
          for ( n = 6; n > 0; n-- )
          {
            if ( pop == 0 )
              temp[col-1] = (temp[col-1] == '(' ? ' ' : ')' );

            if ( args < CUR.top && args >= 0 )
              sprintf( temp+col, "%04lx", CUR.stack[args] );
            else
              sprintf( temp+col, "    " );

            temp[col+4] = ' ';
            col += 5;
            pop--;
            args--;
          }
          temp[78] = '\n';
          temp[79] = '\0';
          printf( "%s", temp );
        }

        /* First, check for empty stack and overflow */
        if ( CUR.args < 0 )
        {
          printf( "ERROR : Too Few Arguments\n" );
          CUR.error = TT_Err_Too_Few_Arguments;
          goto LErrorLabel_;
        }

        CUR.new_top = CUR.args + (Pop_Push_Count[CUR.opcode] & 15);

      /* new_top  is the new top of the stack, after the instruction's */
      /* execution. top will be set to new_top after the 'case'        */

        if ( CUR.new_top > CUR.stackSize )
        {
          printf( "ERROR : Stack overflow\n" );
          CUR.error = TT_Err_Stack_Overflow;
          goto LErrorLabel_;
        }
      }
      else
        printf( "End of program reached.\n" );

      key = 0;
      do
      {
       /* read keyboard */

        ch = getch();

        switch ( ch )
        {
        /* Help - show keybindings */
        case '?':
          printf( "TTDebug Help\n\n" );
          printf( "?   Show this page\n" );
          printf( "q   Quit debugger\n" );
          printf( "n   Skip to next instruction\n" );
          printf( "s   Step into\n" );
          printf( "v   Show vector info\n" );
          printf( "g   Show graphics state\n" );
          printf( "p   Show points zone\n\n" );
          break;

        /* Show vectors */
        case 'v':
          printf( "freedom    (%04hx,%04hx)\n", exc->GS.freeVector.x,
                                                exc->GS.freeVector.y );
          printf( "projection (%04hx,%04hx)\n", exc->GS.projVector.x,
                                                exc->GS.projVector.y );
          printf( "dual       (%04hx,%04hx)\n\n", exc->GS.dualVector.x,
                                                  exc->GS.dualVector.y );
          break;

        /* Show graphics state */
        case 'g':
          printf( "rounding   %s\n", round_str[exc->GS.round_state] );
          printf( "min dist   %04lx\n", exc->GS.minimum_distance );
          printf( "cvt_cutin  %04lx\n", exc->GS.control_value_cutin );
          break;

        /* Show points table */
        case 'p':
          for ( A = 0; A < exc->pts.n_points; A++ )
          {
            printf( "%02hx  ", A );
            printf( "%08lx,%08lx - ", pts.org[A].x, pts.org[A].y );
            printf( "%08lx,%08lx\n",  pts.cur[A].x, pts.cur[A].y );
          }
          printf(( "\n" ));
          break;

        default:
          key = 1;
        }
      } while ( !key );

      MEM_Copy( save.org,   pts.org, pts.n_points * sizeof ( TT_Vector ) );
      MEM_Copy( save.cur,   pts.cur, pts.n_points * sizeof ( TT_Vector ) );
      MEM_Copy( save.tags, pts.tags, pts.n_points );

      /* a return indicate the last command */
      if (ch == '\r')
        ch = oldch;

      switch ( ch )
      {
      /* Quit debugger */
      case 'q':
        goto LErrorLabel_;

      /* Step over */
      case 'n':
        if ( exc->IP < exc->codeSize )
        {
          /* `step over' is equivalent to `step into' except if  */
          /* the current opcode is a CALL or LOOPCALL            */
          if ( CUR.opcode != 0x2a && CUR.opcode != 0x2b )
            goto Step_into;

          /* otherwise, loop execution until we reach the next opcode */
          next_IP = CUR.IP + CUR.length;
          while ( exc->IP != next_IP )
          {
            if ( ( error = TT_RunIns( exc ) ) )
              goto LErrorLabel_;
          }
        }
        oldch = ch;
        break;

      /* Step into */
      case 's':
        if ( exc->IP < exc->codeSize )

      Step_into:
          if ( ( error = TT_RunIns( exc ) ) )
            goto LErrorLabel_;
        oldch = ch;
        break;

      default:
        printf( "unknown command. Press ? for help\n" );
        oldch = '\0';
      }

      for ( A = 0; A < pts.n_points; A++ )
      {
        diff = 0;
        if ( save.org[A].x != pts.org[A].x ) diff |= 1;
        if ( save.org[A].y != pts.org[A].y ) diff |= 2;
        if ( save.cur[A].x != pts.cur[A].x ) diff |= 4;
        if ( save.cur[A].y != pts.cur[A].y ) diff |= 8;
        if ( save.tags[A] != pts.tags[A] ) diff |= 16;

        if ( diff )
        {
          printf( "%02hx  ", A );

          if ( diff & 16 ) temp = "(%01hx)"; else temp = " %01hx ";
          printf( temp, save.tags[A] & 7 );

          if ( diff & 1 ) temp = "(%08lx)"; else temp = " %08lx ";
          printf( temp, save.org[A].x );

          if ( diff & 2 ) temp = "(%08lx)"; else temp = " %08lx ";
          printf( temp, save.org[A].y );

          if ( diff & 4 ) temp = "(%08lx)"; else temp = " %08lx ";
          printf( temp, save.cur[A].x );

          if ( diff & 8 ) temp = "(%08lx)"; else temp = " %08lx ";
          printf( temp, save.cur[A].y );

          printf( "\n" );

          printf( "%02hx  ", A );

          if ( diff & 16 ) temp = "[%01hx]"; else temp = " %01hx ";
          printf( temp, pts.tags[A] & 7 );

          if ( diff & 1 ) temp = "[%08lx]"; else temp = " %08lx ";
          printf( temp, pts.org[A].x );

          if ( diff & 2 ) temp = "[%08lx]"; else temp = " %08lx ";
          printf( temp, pts.org[A].y );

          if ( diff & 4 ) temp = "[%08lx]"; else temp = " %08lx ";
          printf( temp, pts.cur[A].x );

          if ( diff & 8 ) temp = "[%08lx]"; else temp = " %08lx ";
          printf( temp, pts.cur[A].y );

          printf( "\n\n" );
        }
      }
    } while ( TRUE );

  LErrorLabel_:

    if (error)
      Panic( "error during execution" );
    return error;
  }





  static
  void Usage()
  {
    fprintf( stderr, "ttdebug  -  a simply TrueType font debugger\n" );
    fprintf( stderr, "(c) The FreeType project - www.freetype.org\n" );
    fprintf( stderr, "-------------------------------------------\n\n" );

    fprintf( stderr, "usage :   ttdebug  [options]  glyph  size  fontfile[.ttf]\n\n" );

    fprintf( stderr, "    glyph - glyph index within the font file. Can be negative to query \n" );
    fprintf( stderr, "            the debugging of the font 'cvt' program\n\n" );

    fprintf( stderr, "    size - size of glyph in pixels\n\n" );

    fprintf( stderr, "    fontfile - a valid TrueType file.\n\n" );

    fprintf( stderr, "  valid options are:\n\n" );
    fprintf( stderr, "    -d    : Dump mode. Shows the glyph program and exit immediately\n" );
    fprintf( stderr, "    -n    : Non-interactive mode. Dumps the execution trace and exit\n" );

    exit(1);
  }


int    dump_mode;
int    non_interactive_mode;
char*  file_name;
int    glyph_index;
int    glyph_size;

  int  main( int argc, char**  argv )
  {
    char  valid;

    /* Check number of arguments */
    if ( argc < 4 ) Usage();

    /* Check options */
    dump_mode            = 0;
    non_interactive_mode = 0;

    argv++;
    while (argv[0][0] == '-')
    {
      valid = 0;
      switch (argv[0][1])
      {
        case 'd':
          dump_mode = 1;
          valid     = 1;
          break;

        case 'n':
          non_interactive_mode = 1;
          valid                = 1;
          break;

        default:
          ;
      }

      if (valid)
      {
        argv++;
        argc--;
        if (argc < 4) Usage();
      }
      else
        break;
    }

    /* Get Glyph index */
    if ( sscanf( argv[0], "%d", &glyph_index ) != 1 )
    {
      printf( "invalid glyph index = %s\n", argv[1] );
      Usage();
    }

    /* Get Glyph size */
    if ( sscanf( argv[1], "%d", &glyph_size ) != 1 )
    {
      printf( "invalid glyph size = %s\n", argv[1] );
      Usage();
    }

    /* Get file name */
    file_name = argv[2];

    Init_Keyboard();

    /* Init library, read face object, get driver, create size */
    error = FT_Init_FreeType( &library );
    if (error) Panic( "could not initialise FreeType library" );

    memory = library->memory;
    driver = FT_Get_Driver( library, "truetype" );
    if (!driver) Panic( "could not find the TrueType driver in FreeType 2\n" );
    
    FT_Set_Debug_Hook( library,
                       FT_DEBUG_HOOK_TRUETYPE,
                       (FT_DebugHook_Func)RunIns );

    error = FT_New_Face( library, file_name, 0, (FT_Face*)&face );
    if (error) Panic( "could not open font resource" );

    /* find driver and check format */
    if ( face->root.driver != driver )
    {
      error = FT_Err_Invalid_File_Format;
      Panic( "This is not a TrueType font" );
    }
  
    size = (TT_Size)face->root.size;

    if (glyph_index < 0)
    {
      exec = TT_New_Context( face );
      size->debug   = 1;
      size->context = exec;

      error = FT_Set_Char_Size( (FT_Face)face, glyph_size << 6, glyph_size << 6, 72, 72 );
      if (error) Panic( "could not set character sizes" );
    }
    else
    {
      error = FT_Set_Char_Size( (FT_Face)face, glyph_size << 6, glyph_size << 6, 72, 72 );
      if (error) Panic( "could not set character sizes" );

      glyph = (TT_GlyphSlot)face->root.glyph;

      /* Now load glyph */
      error = FT_Load_Glyph( (FT_Face)face, glyph_index, FT_LOAD_DEFAULT );
      if (error) Panic( "could not load glyph" );
    }

    Reset_Keyboard();
    return 0;
  }
