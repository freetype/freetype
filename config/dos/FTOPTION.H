#ifndef FTOPTION_H
#define FTOPTION_H

  /*************************************************************************/
  /*                                                                       */
  /*                 USER-SELECTABLE CONFIGURATION MACROS                  */
  /*                                                                       */
  /* These macros can be toggled by developers to enable or disable        */
  /* certain aspects of FreeType.  This file contains macros that apply to */
  /* all of FreeType.  Driver-specific configurations are placed in each   */
  /* driver directory (e.g. `freetype/drivers/ttlib/ttconfig.h').          */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* Alternate Glyph Image Format support                                  */
  /*                                                                       */
  /*   By default, the glyph images returned by the FreeType glyph loader  */
  /*   can either be a pixmap or a vectorial outline defined through       */
  /*   bezier control points. When defining the following configuration    */
  /*   macro, some font drivers will be able to register alternate         */
  /*   glyph image formats.                                                */
  /*                                                                       */
  /*   Unset this macro if you're sure that you'll never use a font driver */
  /*   with an alternate glyph format, this will reduce the size of the    */
  /*   base layer code.                                                    */
  /*                                                                       */
#define FT_CONFIG_OPTION_ALTERNATE_GLYPH_FORMATS

  /*************************************************************************/
  /*                                                                       */
  /* GCC provides the non-ANSI `long long' 64-bit type.  You can activate  */
  /* it by defining the FTCALC_USE_LONG_LONG macro here.  Note however     */
  /* that we did not experience any improvement in speed with gcc, and     */
  /* that the final code seems bigger when linked.                         */
  /*                                                                       */
#undef FTCALC_USE_LONG_LONG


  /*************************************************************************/
  /*                                                                       */
  /* When compiling FreeType as a DLL, some systems/compilers need a       */
  /* special keyword in front of each function definition instead of       */
  /* `extern'.                                                             */
  /*                                                                       */
  /* The macros EXPORT_DEF and EXPORT_FUNC are thus used to define         */
  /* exported library function interfaces and exported library functions   */
  /* implementations respectively.                                         */
  /*                                                                       */
  /* If not defined here, they automatically default to `extern' and void  */
  /* later in this header file.                                            */
  /*                                                                       */
#undef EXPORT_DEF
#undef EXPORT_FUNC


  /*************************************************************************/
  /*                                                                       */
  /* Debug level                                                           */
  /*                                                                       */
  /*   FreeType can be compiled in debug or trace mode.  In debug mode,    */
  /*   errors are reported through the `ftdebug' component.  In trace      */
  /*   mode, additional messages are sent to the standard output during    */
  /*   execution.                                                          */
  /*                                                                       */
  /*   Define FT_DEBUG_LEVEL_ERROR to build the library in debug mode.     */
  /*   Define FT_DEBUG_LEVEL_TRACE to build it in trace mode.              */
  /*                                                                       */
  /*   Don't define any of these macros to compile in `release' mode.      */
  /*                                                                       */
#undef  FT_DEBUG_LEVEL_ERROR
#undef  FT_DEBUG_LEVEL_TRACE


  /*************************************************************************/
  /*                                                                       */
  /* Anti-aliasing support                                                 */
  /*                                                                       */
  /*   Undefine this macro only if you want to disable the anti-aliasing   */
  /*   support in FreeType.  This will save you about 5 Kb of code.  It    */
  /*   may be important for some embedded systems.                         */
  /*                                                                       */
#define FT_CONFIG_OPTION_ANTI_ALIAS


  /*************************************************************************/
  /*                                                                       */
  /* Endianess performance improvement                                     */
  /*                                                                       */
  /*   FreeType is completely endian-independent, and can thus be compiled */
  /*   directly on _any_ machine.  However, some components of the library */
  /*   provide improved routines for the cases where endianess is known.   */
  /*                                                                       */
  /*   It usually results in speed-ups and reduced code size.  Note that   */
  /*   you should not define both of these macros.                         */
  /*                                                                       */
  /*                                                                       */
  /*   NOTE: For now, only the scan-line converter (base/ftraster.c) uses  */
  /*         these macros to speed-up some anti-alias rendering routines.  */
  /*                                                                       */
#undef  FT_CONFIG_OPTION_LITTLE_ENDIAN
#undef  FT_CONFIG_OPTION_BIG_ENDIAN


  /*************************************************************************/
  /*                                                                       */
  /* Define this configuration macro whenever you want to build a version  */
  /* of FreeType that does not include a default `system' component.       */
  /*                                                                       */
  /* Note that this will prevent the compilation of `ftinit', hence the    */
  /* function FT_Init_FreeType                                             */
  /*                                                                       */
#undef FT_CONFIG_OPTION_NO_DEFAULT_SYSTEM


  /*************************************************************************/
  /*                                                                       */
  /*    The size in bytes of the render pool used by the scan-line         */
  /*    converter to do all of its work.                                   */
  /*                                                                       */
  /*    This must be greater than 4 Kb                                     */
  /*                                                                       */
#define FT_RENDER_POOL_SIZE   32768


  /*************************************************************************/
  /*                                                                       */
  /*    FT_MAX_DRIVERS                                                     */
  /*                                                                       */
  /*    The maximum number of font drivers that can be registered in a     */
  /*    single FreeType library object.  8 seems to be a good choice due   */
  /*    to the relative low actual number of drivers ;-)                   */
  /*                                                                       */
#define FT_MAX_DRIVERS  8


  /*************************************************************************/
  /*                                                                       */
  /*    FT_MAX_GLYPH_FORMATS                                               */
  /*                                                                       */
  /*    The maximum number of glyph image formats that might be registered */
  /*    in a given library instance. 8 seems to be a good choice due to    */
  /*    the relatively low number of current formats ;-)                   */
  /*                                                                       */

#define FT_MAX_GLYPH_FORMATS  8



#endif /* FTOPTION_H */
