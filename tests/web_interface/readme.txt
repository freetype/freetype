Compile with baseline:
gcc main.c qdbmp.c -I ./freetype-2.5.1/include -L ./freetype-2.5.1/objs -lfreetype -o baseline

Compile with test:
gcc main-2.c qdbmp.c -I ./freetype-2.8/include -L ./freetype-2.8/objs -lfreetype -o test
