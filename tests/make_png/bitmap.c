#include "bitmap.h"

HASH_128 * Generate_Hash_x64_128(FT_Bitmap * bitmap, HASH_128 * murmur)
{    
    int seed = 99; // Dont change

    MurmurHash3_x64_128(bitmap->buffer, (bitmap->pitch * bitmap->rows), seed, murmur->hash);

    return murmur;
}

HASH_128 * Generate_Hash_x86_128(FT_Bitmap * bitmap, HASH_128 * murmur)
{    
    int seed = 99; // Dont change

    MurmurHash3_x86_128(bitmap->buffer, (bitmap->pitch * bitmap->rows), seed, murmur->hash);

    return murmur;
}

HASH_32 * Generate_Hash_x86_32(FT_Bitmap * bitmap, HASH_32 * murmur)
{    
    int seed = 99; // Dont change

    MurmurHash3_x86_32(bitmap->buffer, (bitmap->pitch * bitmap->rows), seed, murmur->hash);

    return murmur;
}

