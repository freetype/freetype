Compile with old:
gcc generate-old-hashes.c bitmap.c murmur3.c -I <old ft dir>/include -L <old ft dir>/objs -lfreetype -o generate-old-hashes

Compile with new:
gcc compare-new-to-old.c bitmap.c murmur3.c -I <new ft dir>/include -L <new ft dir>/objs -lfreetype -o compare-new-to-old

For consolidated gen.c:
gcc gen.c bitmap.c murmur3.c -I/usr/local/include/freetype2/ -lfreetype -o gen
./genc <base ft.so dir> <test ft.so dir> <font dir> <char size>
