# Microsoft Developer Studio Project File - Name="freetype" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=freetype - Win32 Debug Multithreaded
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "freetype.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "freetype.mak" CFG="freetype - Win32 Debug Multithreaded"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "freetype - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "freetype - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "freetype - Win32 Debug Multithreaded" (based on "Win32 (x86) Static Library")
!MESSAGE "freetype - Win32 Release Multithreaded" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/DEV/freetype", KAOAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "freetype - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "obj/release"
# PROP Intermediate_Dir "obj/release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /Za /W3 /GX /O2 /I "..\freetype\include\\" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "FT_FLAT_COMPILE" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"lib\freetype200b8.lib"

!ELSEIF  "$(CFG)" == "freetype - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "obj/debug"
# PROP Intermediate_Dir "obj/debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /Za /W3 /Gm /GX /ZI /Od /I "..\freetype\include\\" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "FT_FLAT_COMPILE" /YX /FD /GZ /c
# SUBTRACT CPP /X
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"lib\freetype200b8_D.lib"

!ELSEIF  "$(CFG)" == "freetype - Win32 Debug Multithreaded"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "freetype___Win32_Debug_Multithreaded"
# PROP BASE Intermediate_Dir "freetype___Win32_Debug_Multithreaded"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "obj/debug_mt"
# PROP Intermediate_Dir "obj/debug_mt"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Za /W3 /Gm /GX /ZI /Od /I "..\freetype\include\\" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "FT_FLAT_COMPILE" /YX /FD /GZ /c
# SUBTRACT BASE CPP /X
# ADD CPP /nologo /MTd /Za /W3 /Gm /GX /ZI /Od /I "..\freetype\include\\" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "FT_FLAT_COMPILE" /YX /FD /GZ /c
# SUBTRACT CPP /X
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"lib\freetype200b8_D.lib"
# ADD LIB32 /nologo /out:"lib\freetype200b8MT_D.lib"

!ELSEIF  "$(CFG)" == "freetype - Win32 Release Multithreaded"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "freetype___Win32_Release_Multithreaded"
# PROP BASE Intermediate_Dir "freetype___Win32_Release_Multithreaded"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "obj/release_mt"
# PROP Intermediate_Dir "obj/release_mt"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Za /W3 /GX /O2 /I "..\freetype\include\\" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "FT_FLAT_COMPILE" /YX /FD /c
# ADD CPP /nologo /MT /Za /W3 /GX /O2 /I "..\freetype\include\\" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "FT_FLAT_COMPILE" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"lib\freetype200b8.lib"
# ADD LIB32 /nologo /out:"lib\freetype200b8MT.lib"

!ENDIF 

# Begin Target

# Name "freetype - Win32 Release"
# Name "freetype - Win32 Debug"
# Name "freetype - Win32 Debug Multithreaded"
# Name "freetype - Win32 Release Multithreaded"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\autohint\autohint.c
# End Source File
# Begin Source File

SOURCE=.\src\cff\cff.c
# End Source File
# Begin Source File

SOURCE=.\src\base\ftbase.c
# End Source File
# Begin Source File

SOURCE=.\src\base\ftdebug.c
# End Source File
# Begin Source File

SOURCE=.\src\base\ftglyph.c
# End Source File
# Begin Source File

SOURCE=.\src\base\ftinit.c
# End Source File
# Begin Source File

SOURCE=.\src\base\ftmm.c
# End Source File
# Begin Source File

SOURCE=.\src\base\ftsystem.c
# End Source File
# Begin Source File

SOURCE=.\src\psnames\psmodule.c
# End Source File
# Begin Source File

SOURCE=.\src\raster\raster.c
# End Source File
# Begin Source File

SOURCE=.\src\sfnt\sfnt.c
# End Source File
# Begin Source File

SOURCE=.\src\smooth\smooth.c
# End Source File
# Begin Source File

SOURCE=.\src\truetype\truetype.c
# End Source File
# Begin Source File

SOURCE=.\src\cid\type1cid.c
# End Source File
# Begin Source File

SOURCE=.\src\type1\type1.c
# End Source File
# Begin Source File

SOURCE=.\src\winfonts\winfnt.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\include\freetype\freetype.h
# End Source File
# Begin Source File

SOURCE=.\include\freetype\config\ftconfig.h
# End Source File
# Begin Source File

SOURCE=.\include\freetype\fterrors.h
# End Source File
# Begin Source File

SOURCE=.\include\freetype\config\ftoption.h
# End Source File
# Begin Source File

SOURCE=.\include\freetype\fttypes.h
# End Source File
# End Group
# End Target
# End Project
