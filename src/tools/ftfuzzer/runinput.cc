#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>


  extern "C" void
  LLVMFuzzerTestOneInput( const uint8_t*  data,
                          size_t          size );


  unsigned char a[1 << 24];


  int
  main( int     argc,
        char*  *argv )
  {
    assert( argc >= 2 );

    for ( int i = 1; i < argc; i++ )
    {
      fprintf( stderr, "%s\n", argv[i] );

      FILE*  f = fopen( argv[i], "r" );
      assert( f );

      size_t  n = fread( a, 1, sizeof ( a ), f );
      fclose( f );
      if ( !n )
        continue;

      unsigned char*  b = (unsigned char*)malloc( n );
      memcpy( b, a, n );

      LLVMFuzzerTestOneInput( b, n );

      free( b );
    }
  }


/* END */
