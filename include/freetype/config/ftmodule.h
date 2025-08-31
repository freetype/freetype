/*
 * This file registers the FreeType modules compiled into the library.
 *
 * If you use GNU make, this file IS NOT USED!  Instead, it is created in
 * the objects directory (normally `<topdir>/objs/`) based on information
 * from `<topdir>/modules.cfg`.
 *
 * Please read `docs/INSTALL.ANY` and `docs/CUSTOMIZE` how to compile
 * FreeType without GNU make.
 *
 */

FT_USE_MODULE( FT_Module_Class, autofit_module_class )
FT_USE_MODULE( FT_Driver_Class, tt_driver_class )
FT_USE_MODULE( FT_Driver_Class, t1_driver_class )
FT_USE_MODULE( FT_Driver_Class, cff_driver_class )
FT_USE_MODULE( FT_Driver_Class, t1cid_driver_class )
FT_USE_MODULE( FT_Driver_Class, pfr_driver_class )
FT_USE_MODULE( FT_Driver_Class, t42_driver_class )
FT_USE_MODULE( FT_Driver_Class, winfnt_driver_class )
FT_USE_MODULE( FT_Driver_Class, pcf_driver_class )
FT_USE_MODULE( FT_Driver_Class, bdf_driver_class )
FT_USE_MODULE( FT_Module_Class, psaux_module_class )
FT_USE_MODULE( FT_Module_Class, psnames_module_class )
FT_USE_MODULE( FT_Module_Class, pshinter_module_class )
FT_USE_MODULE( FT_Module_Class, sfnt_module_class )
FT_USE_MODULE( FT_Renderer_Class, ft_smooth_renderer_class )
FT_USE_MODULE( FT_Renderer_Class, ft_raster1_renderer_class )
FT_USE_MODULE( FT_Renderer_Class, ft_sdf_renderer_class )
FT_USE_MODULE( FT_Renderer_Class, ft_bitmap_sdf_renderer_class )
FT_USE_MODULE( FT_Renderer_Class, ft_svg_renderer_class )

/* EOF */
