$!---------------vms_make.com for Freetype2------------------------------------
$! make Freetype2 under OpenVMS
$!
$! In case of problems with the build you might want to contact me at
$! zinser@decus.de (preferred) or zinser@sysdev.deutsche-boerse.com (Work)
$!
$! This procedure currently does support the following commandline options
$! in arbitrary order 
$!
$! * DEBUG - Compile modules with /noopt/debug and link shareable image 
$!           with /debug
$! * LOPTS - Options to be passed to the link command
$! * CCOPT - Options to be passed to the C compiler
$!------------------------------------------------------------------------------
$! 
$! Just some general constants
$!
$ true   = 1
$ false  = 0
$ Make   = ""
$!
$! Setup variables holding "config" information
$!
$ name    = "Freetype2"
$ mapfile =  name + ".map"
$ optfile =  name + ".opt"
$ s_case  = false
$ libdefs = ""
$ libincs = ""
$ liblist = ""
$ ccopt   = ""
$ lopts   = ""
$!
$! Check for MMK/MMS
$!
$ If F$Search ("Sys$System:MMS.EXE") .nes. "" Then Make = "MMS"
$ If F$Type (MMK) .eqs. "STRING" Then Make = "MMK"
$!
$! Which command parameters were given
$!
$ gosub check_opts
$!
$! Create option file
$!
$ open/write optf 'optfile'
$!
$! Pull in external libraries
$!
$ gosub check_create_vmslib
$!
$! Create objects
$!
$ if libdefs .nes. "" then ccopt = ccopt + "/define=(" + libdefs + ")"
$!
$ if f$locate("AS_IS",f$edit(ccopt,"UPCASE")) .lt. f$length(ccopt) - 
    then s_case = true
$ gosub crea_mms
$!
$ 'Make' /macro=(comp_flags="''ccopt'")
$ delete/nolog/noconf temp.mms;*,descrip.fdl;*
$ purge/nolog [...]descrip.mms
$!
$! Add them to options
$!
$FLOOP:
$  file = f$edit(f$search("[...]*.obj"),"UPCASE")
$  if (file .nes. "")
$  then
$    if f$locate("DEMOS",file) .eqs. f$length(file) then write optf file
$    goto floop
$  endif 
$!
$! Pull in external libraries
$!
$ gosub check_create_vmslib
$!
$ if s_case then WRITE optf "case_sensitive=YES"
$ close optf
$!
$!
$! Alpha gets a shareable image
$!
$ If f$getsyi("HW_MODEL") .gt. 1024
$ Then
$   LINK_/NODEB/NOSHARE/NOEXE/MAP='mapfile'/full 'optfile'/opt
$   call anal_map_axp 'mapfile' _link.opt
$   LINK_/NODEB/SHARE=[.lib]freetype2shr.exe 'optfile'/opt,_link.opt/opt
$   dele/noconf 'mapfile';*
$ endif
$!
$ exit
$!
$!------------------------------------------------------------------------------
$!
$! If MMS/MMK are available dump out the descrip.mms if required 
$!
$CREA_MMS:
$ write sys$output "Creating descrip.mms files ..."
$ write sys$output "... Main directory"
$ copy sys$input: descrip.mms
$ deck
#
# FreeType 2 build system -- top-level Makefile for OpenVMS
#


# Copyright 2001 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


all :
        define freetype [--.include.freetype] 
        define psaux [-.psaux] 
        define autohint [-.autohint] 
        define base [-.base] 
        define cache [-.cache] 
        define cff [-.cff] 
        define cid [-.cid] 
        define pcf [-.pcf] 
        define psnames [-.psnames] 
        define raster [-.raster] 
        define sfnt [-.sfnt] 
        define smooth [-.smooth] 
        define truetype [-.truetype] 
        define type1 [-.type1] 
        define winfonts [-.winfonts] 
        if f$search("lib.dir") .eqs. "" then create/directory [.lib]
        set default [.builds.vms]
        $(MMS)$(MMSQUALIFIERS)
        set default [--.src.autohint]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.base]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.bdf]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.cache]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.cff]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.cid]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.gzip]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.pcf]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.pfr]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.psaux]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.pshinter]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.psnames]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.raster]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.sfnt]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.smooth]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.truetype]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.type1]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.type42]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.winfonts]
        $(MMS)$(MMSQUALIFIERS)
        set default [--]

# EOF
$ eod
$ anal/rms/fdl descrip.mms
$ create/fdl=descrip.fdl temp.mms 
$ open/append mmsf temp.mms 
$ write mmsf "CFLAGS = ", ccopt
$ close mmsf
$ copy temp.mms,descrip.mms;-1 descrip.mms 
$ write sys$output "... [.src.gzip] directory"
$ copy sys$input: [.src.gzip]descrip.mms
$ deck
#
# FreeType 2 GZip support compilation rules for VMS
#


# Copyright 2002 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


CFLAGS=$(COMP_FLAGS)$(DEBUG)/include=($(LIBINCS)[--.include],[--.src.gzip])

OBJS=ftgzip.obj

all : $(OBJS)
        library [--.lib]freetype.olb $(OBJS)

# EOF
$ eod
$ create/fdl=descrip.fdl temp.mms
$ if libincs .nes. ""
$ then
$   open/append mmsf temp.mms
$   write mmsf "LIBINCS = ", libincs, ","
$   close mmsf
$   copy temp.mms,[.src.gzip]descrip.mms;-1 [.src.gzip]descrip.mms 
$ endif
$ return
$!------------------------------------------------------------------------------
$!
$! Check command line options and set symbols accordingly
$!
$ CHECK_OPTS:
$ i = 1
$ OPT_LOOP:
$ if i .lt. 9
$ then
$   cparm = f$edit(p'i',"upcase")
$   if cparm .eqs. "DEBUG"
$   then
$     ccopt = ccopt + "/noopt/deb"
$     lopts = lopts + "/deb"
$   endif
$!   if cparm .eqs. "LINK" then linkonly = true
$   if f$locate("LOPTS",cparm) .lt. f$length(cparm)
$   then
$     start = f$locate("=",cparm) + 1
$     len   = f$length(cparm) - start
$     lopts = lopts + f$extract(start,len,cparm)
$   endif
$   if f$locate("CCOPT",cparm) .lt. f$length(cparm)
$   then
$     start = f$locate("=",cparm) + 1
$     len   = f$length(cparm) - start
$     ccopt = ccopt + f$extract(start,len,cparm)
$   endif
$   i = i + 1
$   goto opt_loop
$ endif
$ return
$!------------------------------------------------------------------------------
$!
$! Take care of driver file with information about external libraries
$!
$CHECK_CREATE_VMSLIB:
$!
$ if f$search("VMSLIB.DAT") .eqs. ""
$ then
$   type/out=vmslib.dat sys$input
!
! This is a simple driver file with information used by make.com to
! check if external libraries (like t1lib and freetype) are available on
! the system.
!
! Layout of the file:
!
!    - Lines starting with ! are treated as comments
!    - Elements in a data line are separated by # signs
!    - The elements need to be listed in the following order
!      1.) Name of the Library 
!      2.) Location where the object library can be found
!      3.) Location where the include files for the library can be found
!      4.) Include file used to verify library location
!      5.) CPP define to pass to the build to indicate availability of
!          the library
!
! Example: The following  lines show how definitions
!          might look like. They are site specific and the locations of the
!          library and include files need almost certainly to be changed.
!
! Location: All of the libaries can be found at the following addresses
!
!   ZLIB:     http://www.decus.de:8080/www/vms/sw/zlib.htmlx
!
!ZLIB # pubbin:libz.olb # public$Root:[util.libs.zlib] # zlib.h # FT_CONFIG_OPTION_SYSTEM_ZLIB
$   write sys$output "New driver file vmslib.dat created."
$   write sys$output "Please customize libary locations for your site"
$   write sys$output "and afterwards re-execute vms_make.com"
$   write sys$output "Exiting..."
$   close/nolog optf
$   exit
$ endif
$!
$! Open data file with location of libraries
$!
$ open/read/end=end_lib/err=lib_err libdata VMSLIB.DAT
$LIB_LOOP:
$ read/end=end_lib libdata libline
$ libline = f$edit(libline, "UNCOMMENT,COLLAPSE")
$ if libline .eqs. "" then goto LIB_LOOP ! Comment line
$ libname = f$edit(f$element(0,"#",libline),"UPCASE")
$ liblist = liblist + "#" + libname
$ write sys$output "Processing ''libname' setup ..."
$ libloc  = f$element(1,"#",libline)
$ libsrc  = f$element(2,"#",libline)
$ testinc = f$element(3,"#",libline)
$ cppdef  = f$element(4,"#",libline)
$ old_cpp = f$locate("=1",cppdef)
$ if old_cpp.lt.f$length(cppdef) then cppdef = f$extract(0,old_cpp,cppdef)
$ if f$search("''libloc'").eqs. ""
$ then
$   write sys$output "Can not find library ''libloc' - Skipping ''libname'"
$   goto LIB_LOOP
$ endif
$ libsrc_elem = 0
$ libsrc_found = false
$LIBSRC_LOOP:
$ libsrcdir = f$element(libsrc_elem,",",libsrc)
$ if (libsrcdir .eqs. ",") then goto END_LIBSRC
$ if f$search("''libsrcdir'''testinc'") .nes. "" then libsrc_found = true
$ libsrc_elem = libsrc_elem + 1
$ goto LIBSRC_LOOP
$END_LIBSRC:
$ if .not. libsrc_found
$ then
$   write sys$output "Can not find includes at ''libsrc' - Skipping ''libname'"
$   goto LIB_LOOP
$ endif
$ if cppdef .nes. "" then libdefs = libdefs +  "," + cppdef
$ libincs = libincs + "," + libsrc
$ lqual = "/lib"
$ libtype = f$parse(libloc,,,"TYPE")
$ if f$locate("EXE",libtype) .lt. f$length(libtype) then lqual = "/share"
$ write optf libloc , lqual
$!
$! Yet another special treatment for Xpm/X11
$!
$ if (libname .eqs. "XPM")
$ then
$   my_x11 = f$parse("''libsrc'xpm.h",,,"device") + - 
             f$parse("''libsrc'xpm.h",,,"directory")
$   x11_save = f$trnlnm("X11")
$   define x11 'my_x11',decw$include   
$ endif 
$ goto LIB_LOOP
$END_LIB:
$ close libdata
$ libincs = libincs - ","
$ libdefs = libdefs - ","
$ return
$!------------------------------------------------------------------------------$$!------------------------------------------------------------------------------
$!
$! Analyze Map for OpenVMS AXP
$!
$ ANAL_MAP_AXP: Subroutine   
$ V = 'F$Verify(0)
$ SET SYMBOL/GENERAL/SCOPE=(NOLOCAL,NOGLOBAL)
$ SAY := "WRITE_ SYS$OUTPUT"
$ 
$ IF F$SEARCH("''P1'") .EQS. ""
$ THEN
$    SAY "  ANAL_MAP_AXP:  Error, no mapfile provided"
$    goto exit_aa
$ ENDIF
$ IF "''P2'" .EQS. ""
$ THEN
$    SAY "  ANALYZE_MAP_AXP:  Error, no output file provided"
$    goto exit_aa
$ ENDIF
$
$ LINK_TMP  = F$PARSE(P2,,,"DEVICE")+F$PARSE(P2,,,"DIRECTORY")+F$PARSE(P2,,,"NAME")+".TMP"
$
$ SAY "  creating PSECT list in ''P2'"
$ OPEN_/READ IN 'P1'
$ OPEN_/WRITE OUT 'P2'
$ WRITE_ OUT "!"
$ WRITE_ OUT "! ### PSECT list extracted from ''P1'"
$ WRITE_ OUT "!" 
$ LOOP_PSECT_SEARCH:
$    READ_/END=EOF_PSECT IN REC
$    if F$EXTRACT(0,5,REC) .nes. "$DATA" then goto LOOP_PSECT_SEARCH
$ LAST = ""
$ LOOP_PSECT:
$    READ_/END=EOF_PSECT IN REC
$    if F$EXTRACT(0,1,REC) .eqs. "$" .and. F$EXTRACT(0,5,REC) .nes. "$DATA" then goto EOF_PSECT
$    if REC - "NOPIC,OVR,REL,GBL,NOSHR,NOEXE,  WRT,NOVEC" .nes. REC
$    then 
$       J = F$LOCATE(" ",REC)
$       S = F$EXTRACT(0,J,REC)
$       IF S .EQS. LAST THEN GOTO LOOP_PSECT
$       WRITE_ OUT "symbol_vector = (" +  S + " = PSECT)"
$       P$_'S= 1
$       LAST = S
$    endif
$    GOTO LOOP_PSECT
$
$ EOF_PSECT:
$    CLOSE_ IN
$    CLOSE_ OUT 
$!
$ OPEN_/READ IN 'P1'
$ OPEN_/APPEND OUT 'P2'
$ WRITE_ OUT "!"
$ WRITE_ OUT "! ### Global definition list extracted from ''P1'"
$ WRITE_ OUT "!" 
$ LOOP_DATA_SEARCH:
$   READ_/END=EOF_DATA IN REC
$   if f$locate("NOPIC,OVR,REL,GBL,NOSHR,NOEXE",rec) .eq. f$length(rec) -
      then goto LOOP_DATA_SEARCH
$   s = f$element(0," ",rec)      
$!   write_ out "symbol_vector = (" + s + " = DATA)"
$   p$_'s' =1
$   goto loop_data_search 
$ EOF_DATA:
$ CLOSE_ IN
$ CLOSE_ OUT
$ SAY "  appending list of UNIVERSAL procedures to ''P2'"
$ SEARCH_/NOHIGH/WINDOW=(0,0) 'P1' " R-"/OUT='LINK_TMP
$ OPEN_/READ IN 'LINK_TMP
$ OPEN_/APPEND OUT 'P2'
$ WRITE_ OUT "!"
$ WRITE_ OUT "! ### UNIVERSAL procedures and global definitions extracted from ''P1'"
$ WRITE_ OUT "!" 
$ LOOP_UNIVERSAL:
$    READ_/END=EOF_UNIVERSAL IN REC
$    data = 0
$    J = F$LOCATE(" R-",REC)
$    S = F$EXTRACT(J+3,F$length(rec),REC)
$    IF (F$TYPE(P$_'S').EQS."").and.(data.ne.1) 
$    THEN
$       WRITE_ OUT "symbol_vector = ("+S+"      = PROCEDURE)"
$    ELSE
$       WRITE_ OUT "symbol_vector = ("+S+"      = DATA)"
$    ENDIF
$    GOTO LOOP_UNIVERSAL
$ EOF_UNIVERSAL:
$    CLOSE_ IN
$    CLOSE_ OUT
$    if f$search("''LINK_TMP'") .nes. "" then DELETE_/NOLOG/NOCONFIRM 'LINK_TMP';*
$
$ EXIT_AA:
$ if V then set verify
$ endsubroutine 
