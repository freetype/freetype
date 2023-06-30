# Compiling FreeType on Android using Termux

If you want to test FreeType on arm devices, compiling it on an Android
phone is one of the easiest ways.

## 1. Compiling FreeType

1. Install and open termux

2. Update termux packages using:

   ```bash
   pkg update
   pkg upgrade
   ```
   Select `y` if any configuration changes are asked

3. Install `git`:

   ```bash
   pkg install git
   ```
4. Install packages required for compilation:

   ```bash
   pkg install automake autoconf libtool make clang binutils
   ```

5. Clone FreeType's git repository:

   ```bash
   git clone https://gitlab.freedesktop.org/freetype/freetype.git
   ```

6. Enter into the source directory:

   ```
   cd freetype/
   ```

7. Now you can compile FreeType according to [INSTALL_UNIX.md]:

   ```bash
   ./autogen.sh
   ./configure
   make -j$(nproc)
   ```
   The compiled binaries can be accessed under `objs/.libs`

## 2. Compiling FreeType Demo programs
If you need to test your changes you may want to use ft-demo
programs. They can be compiled and run by follwing these steps:

1. First compile FreeType using the steps above.

2. Clone FreeType demos adjacent to the `freetype/` directory:

   ```bash
   git clone https://gitlab.freedesktop.org/freetype/freetype-demos.git
   ```
   i.e. If you can access FreeType repo under `~/freetype`, you should
   be able to access `~/freetype-demos`.

3. Enter into the ft-demos directory:

   ```bash
   cd freetype-demos
   ```

4. Compile the programs:

   ```bash
   make
   ```

5. The compiled binaries can be accessed and executed under `bin/` like:

   ```bash
   cd bin
   ./ftlint
   ```

<!---->

[INSTALL_UNIX.md]: ./INSTALL_UNIX.md
