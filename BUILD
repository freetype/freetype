FreeType 2 compilation how-to


Introduction:

Welcome to this new beta of the FreeType 2 library. You'll find in this
document instructions on how to compile the library on your favorite
platform.

  *** UNIX USERS : Even though the FT2 build system doesn't
  ************** : use the Autoconf/Automake tools, these will
  ************** : be introduced in the Unix-specific parts of
  ************** : the build in our final release..


I. QUICK COMMAND-LINE GUIDE:
----------------------------

  Install GNU Make, then try the following on Unix or any system with gcc:
  
     make    // this will setup the build
     make    // this will build the library
     
  On Win32+Visual C++:
  
     make setup visualc    // setup the build for VisualC++ on Win32
     make                  // build the library
     
  Then, go to the "demos" directory and type
  
     make
     
  To compile the demo programs..

  If this doesn't work, read the following..



II. COMMAND-LINE COMPILATION:
-----------------------------

  Note that if you do not want to compile FreeType 2 from a command line
  shell, please skip to section III below (DETAILED COMPILATION)

  FreeType 2 includes a powerful and flexible build system that allows you
  to easily compile it on a great variety of platforms from the command
  line. To do so, just follow these simple instructions:
  
  a/ Install GNU Make:
  
     Because GNU Make is the only Make tool supported to compile FreeType 2,
     you should install it on your machine.

     Because the FT2 build system relies on many important features of GNU
     Make, trying to build the library with any other Make tool will *fail*.
    

  b/ Invoke "make":
  
     Go to the root FT2 directory, then simply invoke GNU Make from the
     command line, this will launch the FreeType 2 Host Platform detection
     routines. A summary will be displayed, for example, on Win32:
     
     ========================================================================
        FreeType build system -- automatic system detection
         
        The following settings are used:
         
          platform                     win32
          compiler                     gcc
          configuration directory      ./config/win32
          configuration rules          ./config/win32/w32-gcc.mk  
         
        If this does not correspond to your system or settings please remove
        the file 'config.mk' from this directory then read the INSTALL file
        for help.
         
        Otherwise, simply type 'make' again to build the library.
     =========================================================================
     
     If the detected settings correspond to your platform and compiler,
     skip to step e/. Note that if your platform is completely alien to
     the build system, the detected platform will be "ansi".
     
     
  c/ Configure the build system for a different compiler:

     If the build system correctly detected your platform, but you want to
     use a different compiler than the one specified in the summary (for
     most platforms, gcc is the defaut compiler), simply invoke GNU Make
     like :
     
         make setup <compiler>
         
     For example:
     
            to use Visual C++ on Win32, type:  "make setup visualc"
            to use LCC-Win32 on Win32, type:   "make setup lcc"
     
     The <compiler> name to use is platform-dependent. The list of available
     compilers for your system is available in the file
     "config/<system>/detect.mk" (note that we hope to make the list
     displayed at user demand in the final release)..

     If you're satisfying by the new configuration summary, skip to step e/


  d/ Configure the build system for an unknown platform/compiler:
  
     What the auto-detection/setup phase of the build system does is simply
     copy a file to the current directory under the name "config.mk".
     
     For example, on OS/2+gcc, it would simply copy "config/os2/os2-gcc.mk"
     to "./config.mk"
     
     If for some reason your platform isn't correctly detected, simply copy
     manually the configuration sub-makefile to "./config.mk" and go to
     step e/.
     
     Note that this file is a sub-Makefile used to specify Make variables
     used to invoke the compiler and linker during the build, you can easily
     create your own version from one of the existing configuration files,
     then copy it to the current directory under the name "./config.mk".
     
  
  e/ Build the library:
  
     The auto-detection/setup phase should have copied a file in the current
     directory, called "./config.mk". This file contains definitions of various
     Make variables used to invoke the compiler and linker during the build.
     
     To launch the build, simply invoke GNU Make again: the top Makefile will
     detect the configuration file and run the build with it..
     
     
  f/ Build the demonstration programs:
     
     Once the library is compiled, go to "demos", then invoke GNU Make.
     
     Note that the demonstration programs include a tiny graphics sub-system
     that includes "drivers" to display Windows on Win32, X11 and OS/2. The
     build system should automatically detect which driver to use based on
     the current platform.

     UNIX USERS TAKE NOTE: XXXXXX
     
     When building the demos, the build system tries to detect your X11 path
     by looking for the patterns "X11R5/bin", "X11R6/bin" or "X11/bin" in
     your current path. If no X11 path is found, the demo programs will not
     be able to display graphics and will fail. Change your current path
     if you encounter this problem.
     
     Note that the release version will use Autoconf to detect everything
     on UNix, so this will not be necessary !!


II. DETAILED COMPILATION PROCEDURE:
-----------------------------------

  If you don't want to compile FreeType 2 from the command-line (for example
  from a graphical IDE on a Mac or Windows), you'll need to understand how the
  FreeType files are organized.
  
  First of all, all configuration files are located in "freetype2/config",
  with system-specific overrides in "freetype2/config/<system>". You should
  always place "config/<system>" and "config" in your compilation include
  path, **in this order**
  
  Also, place the directory "include" in the compilation include path, as
  well as "src/base" and "src/shared"
  
  Now, FreeType 2 is a very modular design, made of several distinct components.
  Each component can be compiler either as a stand-alone object file, or as a
  list of independent objects.
  
  For example, the "base layer" is made of the following independent source
  files:
  
     freetype2/
        src/
           base/
              ftcalc.c
              ftdebug.c
              ftextend.c
              ftlist.c
              ftobjs.c
              ftstream.c
              ftraster.c
              ftoutln.c
              ftsystem.c

  You can compile each of these files separately.
  
  Another method is to compile the file "src/base/ftbase.c" which performs
  a simple include on all these individual files. This will compile the whole
  base layer as a single object file.
  
  Note that through careful macro definitions, compiling a module as a single
  component avoids the generation of many externals (that really correspond
  to intra-module dependencies) and provides greater optimisations
  opportunities.
  
  Similarly, each component has a single "englobing" C file to compile it
  as a stand-alone object, i.e. :
  
     src/base/ftbase.c         - the base layer, high-level interface
     src/sfnt/sfnt.c           - the "sfnt" module
     src/psnames/psnames.c     - the Postscript Names module
     src/truetype/truetype.c   - the TrueType font driver
     src/type1/type1.c         - the Type 1 font driver

  Now, you can decide how to compile each module, and add the corresponding
  object files to your library..
  
  The directory "freetype2/include" contains all public header files that
  may be included by client applications..

