                       The FreeType Build System Internals
                       -----------------------------------

Introduction:

  This document describes the details of the FreeType build system. The
  build system is a set of Makefiles and other configuration files used
  to select, compile and link together the various FreeType components
  according to the current platform, compiler and requested feature set.

  This document also explains how to use the build system to develop
  third-party font drivers or extensions to the engine, without altering
  the general FreeType hierarchy;


I. Portability issues :

  Given that the design of FreeType 2 is much more modular and flexible than
  in previous versions, its build system is entirely based on GNU Make. There
  are several reasons for this :

    - It is by far the most available make tool on the planet, and
      has probably been ported to every development environment known
      to homo programmaticus.

    - It provides useful features (like conditional defines, pattern
      and wildcard matching) which are essential when implementing a
      flexible configuration system, as described below

  Note that you do not need to have a unix-like shell (like "sh" or "csh")
  on your system in order to build FreeType.
  



II. The library design :

  FreeType is made of several components, each with a specific role :

    - THE BASE LAYER:
      It is used to implement generic font services as well as provide
      the high-level API used by client applications.

    - ONE OR MORE FONT DRIVERS:
      Each driver is a small component used to read and process a given
      font format. Note that with FreeType 2, it is possible to add,
      remove or upgrade a font driver at *runtime*.

    - ONE OR MORE RASTERS:
      A raster is a module used to render a vectorial glyph outline into
      a bitmap or an anti-aliased pixmap. They differ in their output
      quality, speed and memory usage.

    - A LOW-LEVEL MODULE, CALLED "FTSYSTEM":
      It is used to implement memory management and file i/o. Uses the
      Ansi C Library by default, though some system-specific replacements
      are provided in order to improve performance.

    - AN "INIT" LAYER:
      A tiny module used to implement the library initialisation routine,
      i.e. FT_Init_FreeType. It is in charge of registering the font drivers
      and rasters selected at build time.

    - AN "OLD API" LAYER:
      A simple layer used to link legacy applications using the FreeType
      1.x API. Note that it is binary backwards compatible, which means that
      applications do not need to be recompiled, only re-linked to this
      layer.

  For more details, please read the "FreeType Internals" Document.      


  The FreeType build system is in charge of the following tasks :

    - detect (or select) the current platform in order to select the
      best version of the "ftsystem" module. By default, it will use
      the pure-ANSI version.

    - determine which font drivers, and which rasters, should be
      statically linked to the library at build time. These will always
      be available after the library has been initialised through a call
      to FT_Init_FreeType.

    - eventually compile other font drivers or rasters in order to later
      link them dynamically to the library at runtime, through
      FT_Add_Driver / FT_Upgrade_Driver..

    - compile the "init" layer, putting code in the implementation of
      the FT_Init_FreeType function to register each selected font driver
      or raster to the library.



III. General overview :

  The FreeType build system uses a hierarchy of included sub-Makefiles
  to compile and link the library.

  Each included sub-Makefile is called a "rules" file, and has a very
  specific purpose. The suffix for rules files is ".mk" as in :

       detect.mk
       config.mk
       rules.mk
       etc...


  Here's a simple diagram of the build hierarchy, which is then explained
  with details :



                       Makefile            ( ./Makefile )

                          |
                          |
                          v

                     Config Rules          ( ./config/<system>/config.mk )

                          |
                          |
                          v

                    Library  Rules         ( ./config/freetype.mk )

                      |   |   |
                      |   |   |
                      v   v   v

                  Component(s) Rules       ( ./src/<component>/rules.mk )



  1. The "root" Makefile :

     This file must be invoked from the "freetype" directory with GNU Make.

     a. Host platform auto-detection:

       When run for the first time, this Makefile will try to auto-detect
       the current host platform, by running the rules file named
       `./config/detect.mk'. If the host system cannot be detected,
       it will default to the `ansi' system.

       It will then copy the rules file `./config/<system>/config.mk' to
       the current directory and display the results of the auto-detection.

       You can, at any time, re-run the auto-detection routine by invoking
       the root Makefile with the "setup" target, as in :

              % make setup

       Note also that it is possible to use a second argument to indicate
	   a specific compiler. For example, here are the lignes to be used
	   in order to configure a build with LCC, Visual C++ and Visual Age
	   on a Win32 machine
	   
	          > gmake setup lcc
			  > gmake setup visualc
			  > gmake setup visualage

       The list of compilers is platform-specific and should be contained
	   in `config/<system>/detect.mk'.

       If the detection results do not correspond to your platform or
       settings, refer to chapter VI which describes the auto-detection
       system in great details..


     b. Building the library:

       Once the host platform has been detected, you can run `make' once
       again. The root Makefile will then detect the configuration rules
       file in the current directory then include it.

     Note also that the root Makefile is responsible for defining, if it
     is not already part of the current environment, the variable TOP, which
     designates the top of the FreeType source hierarchy.

     When undefined, it defaults to `.'


  2. The Configuration file :

     The configuration rules file is used to set many important variables
     before including/calling the library rules file (see below).

     These variables are mainly used to describe the host environment
     and compilers. Indeed, this file defines, among others, the following:

       SEP     The directory path separator. This can be `/',`\' or ':'
               depending on the current platform. Note that all pathnames
               are composed with $(SEP) in all rules file (except in
               `include' statements which work well with '/' on all
               platforms)

       CC      The compiler to use

       CFLAGS  The compiler flags used to compile a given source to an
               object file. Usually contains flags for optimisation,
               debugging and/or ansi-compliance

       I       The flag to be used to indicate an additionnal include path
               to the compiler. This defaults to `-I' for an "ansi" system,
               but can be different for others (e.g. `/i=',`-J ', etc..)

       D       The flag to be used to indicate a macro definition to the
               compiler. This defaults to `-D' for an ANSI system.

       T       The flag to be used to indicate a target object file to the
               compiler. This defaults to `-o ' for an ANSI system. Note the
               space after the `o'.        

       O       The object file extension to be used on the current platform.
               Defaults to `o' for an ANSI system, but can be `obj', `coff'
               or others.. There is no dot in the extension !

       A       The library file extension to be used on the current platform.
               Defaults to 'a' for an ANSI system, but can be `lib', `so',
               `dll' or others.. There is no dot in the extension !


       BUILD   The directory where the build system should grab the
               configuration header file `ftconfig.h' as well as the
               system-specific implementation of `ftsystem'.

       OBJ     The directory where all object files will be placed


  3. The Library Rules files :

     Once the variables defined in the configuration rules file, the
     library rules file is included. This one contains all rules required
     to build the library objects into OBJ

     Its structure works as follows:

       - provide rules to compile the low-level `ftsystem' module

       - include the rules files from each font driver or component

       - include the rules file for the "old api" layer

       - provide rules to compile the initialisation layer

       - provide additional targets like `clean', ..


     Note that linking all objects files together into a library is not
     performed in this file, though it might seem reasonable at first
     glance. The reason for this is that not all linkers have a simple
     syntax of the form:

         librarian archive_file  object1 object2 ....

     hence, linking is performed through rules provided in the configuration
     rules file, using the phony `library' target, which has been defined for
     this very specific purpose.


  4. The Components Rules files :

     Each font driver has its own rules file, called `rules.mk' located
     in its own directory. The library rules file includes these component
     rules for each font driver.

     These rules must perform the following:

       - provide rules to compile the component, either into a single `large'
         object, or into multiple small ones

       - for font drivers and rasters, update some variables, that are
         initially defined in the library rules file, which indicate wether
         the component must be registered in the library initialisation code


     a. Component Compile Modes :     

       There are two ways to compile a given component :

         i. Single-object compilation:

            In this mode, the component is compiled into a single object
            file. This is performed easily by defining a single C file whose
            sole purpose is to include all other component sources. For
            example, the truetype driver is compiled as a single object
            named `truetype.o'.


         ii. Multiple objects compilation:

            In this mode, all source files for a single component are compiled
            individually into an object file.

       Due to the way the FreeType source code is written, single mode
       has the following advantages over multiple mode:

         - with many compilers, the resulting object code is smaller than
           the concatenation of all individual objects from multiple mode.
           this, because all functions internal to the component as a whole
           are declared static, allowing more optimisation. It often also
           compiles much faster.


         - most importantly, the single object only contains the external
           symbols it needs to be linked to the base layer (all extern that
           are due to inter-source calls within the component are removed).
           this can reduce tremendously the size of dynamic libraries on
           some platforms

       Multiple mode is useful however to check some dependencies problems
       that might not appear when compiling in single mode, so it has been
       kept as a possibility.


     b. Driver initialisation code :

       The source file `./src/base/ftinit.c' contains the implementation
       of the FT_Init_FreeType function which must, among other things,
       register all font drivers that are statically linked to the library.

       Controlling which drivers are registered at initialisation time is
	   performed by exploiting the state of the C-preprocessor in order to
	   build a linked list (a "chain") of driver interfaces.

       More precisely, each font driver interface file (like `ttdriver.h'
	   or `t1driver.h') has some special lines that look like this :
	   
  			
    			#ifdef FTINIT_DRIVER_CHAIN
 			
    			static
    			const FT_DriverChain  ftinit_<FORMAT>_driver_chain =
    			{
      			FT_INIT_LAST_DRIVER_CHAIN,
      			&<FORMAT>_driver_interface
    			};
  			
    			#undef  FT_INIT_LAST_DRIVER_CHAIN
    			#define FT_INIT_LAST_DRIVER_CHAIN   &ftinit_<FORMAT>_driver_chain
 			
    			#endif 
				
  		As one can see, this code is strictly reserved for `ftinit.c' which
		defines FTINIT_DRIVER_CHAIN before including all font driver header
		files.
		
		When the C-processor parses these headers, it builds a linked list of
		FT_DriverChain element. For exemple, the sequence :
		
		   #define FTINIT_DRIVER_CHAIN
		   #include <ttdriver.h>
		   #include <t1driver.h>
		   
		Will really generate something like:
		
    			static
    	*---->	const FT_DriverChain  ftinit_tt_driver_chain =
    	|		{
      	|		0,
      	|		&tt_driver_interface
    	|		};
  		|	
    	|		static
    	|		const FT_DriverChain  ftinit_t1_driver_chain =
    	|		{
      	*------	&ftinit_tt_driver_chain,
      			&t1_driver_interface
    			};
 	   
	   with the FT_INIT_LAST_DRIVER_CHAIN set to "&ftinit_t1_driver_chain"
	   
	   Hence, the last included driver will be registered first in the library
	   
