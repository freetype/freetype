This directory is provided in order to compile a stand-alone
TrueType driver which should be backwards and binary compatible
with FreeType 1.1

Reason is some important design changes were introduced in FreeType
lately, in order to support several font formats transparently.

The files are :

"truetype.h"        - a replacement for the old "freetype.h" file
                      that was included by client applications and
                      font servers of 1.1

"ttapi.c"           - a front-end for the new TrueType driver, that
                      presents exactly the same interface as the one
                      in 1.1


