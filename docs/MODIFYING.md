# Modifying FreeType

FreeType follows a modular architecture, i.e. all the features are
implemented as separate modules. There are separate modules for 
rasterizers, font parsers, hinting etc. located under the `src/` directory.

(See https://freetype.org/freetype2/docs/design/design-5.html)

To add new features you have to either modify the existing modules or
add a new module to FreeType.

## Adding a new module to FreeType

Suppose we want to add a new module "example" to FreeType:

1. Create a directory under `src/` having the same name as the module.
   i.e. `src/example/`

2. Add source files under src/example having `#define FT_MAKE_OPTION_SINGLE_OBJECT`
   which includes the other files to create the module.

   (See `src/sdf/sdf.c` for reference)

3. Add the module to `include/freetype/config/ftmodule.h` according to whether
   it is a renderer, font driver or another module like:

    ```C
    FT_USE_MODULE( FT_Renderer_Class, ft_example_renderer_class )
    ```

4. Add the module to `modules.cfg` in the toplevel directory:
    ```
    RASTER_MODULES += example
    ```

5. Under `src/example` add `module.mk` and `rules.mk` files to enable compilation
   with `make`. (See `src/smooth/rules.mk` for reference).

6. Now you can simply compile by using `make` in the toplevel directory and the module
   should compile.

   You can check out pre-existing modules for reference under `src/`
