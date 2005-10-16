/* this little program is used to parse the FreeType headers and
 * find the declaration of all public API. This is easy, because
 * they all look like the following:
 *
 *   FT_EXPORT( return_type )
 *   function_name( function arguments );
 *
 * you must pass it the list of header files as arguments, wildcards
 * accepted if you're using GCC on Windows
 *
 * Author: David Turner
 *
 * This code is explicitely placed in the public domain
 */
#include <stdio.h>
#include <stdlib.h>

#define  PROGRAM_NAME     "apinames"
#define  PROGRAM_VERSION  "0.1"

#define  LINEBUFF_SIZE  1024

static void
panic( const char*  message )
{
  fprintf( stderr, "PANIC: %s\n", message );
  exit(2);
}


typedef struct
{
  char*         name;
  unsigned int  hash;

} NameRec, *Name;

static Name  the_names;
static int   num_names;
static int   max_names;

static void
names_add( const char*  name,
           const char*  end )
{
  int   nn, len, h;
  Name  nm;

  if ( end <= name )
    return;

  /* compute hash value */
  len = (int)(end - name);
  h   = 0;
  for ( nn = 0; nn < len; nn++ )
    h = h*33 + name[nn];

  /* check for an pre-existing name */
  for ( nn = 0; nn < num_names; nn++ )
  {
    nm = the_names + nn;

    if ( nm->hash                      == h &&
         memcmp( name, nm->name, len ) == 0 &&
         nm->name[len]                 == 0 )
      return;
  }

  /* add new name */
  if ( num_names >= max_names )
  {
    max_names += (max_names >> 1) + 4;
    the_names  = realloc( the_names, sizeof(the_names[0])*max_names );
    if ( the_names == NULL )
      panic( "not enough memory" );
  }
  nm = &the_names[num_names++];

  nm->hash = h;
  nm->name = malloc( len+1 );
  if ( nm->name == NULL )
    panic( "not enough memory" );

  memcpy( nm->name, name, len );
  nm->name[len] = 0;
}


static int
name_compare( const void*  name1,
              const void*  name2 )
{
  Name  n1 = (Name)name1;
  Name  n2 = (Name)name2;

  return strcmp( n1->name, n2->name );
}

static void
names_sort( void )
{
  qsort( the_names, (size_t)num_names, sizeof(the_names[0]), name_compare );
}

static void
names_dump( FILE*  out )
{
  int  nn;

  for ( nn = 0; nn < num_names; nn++ )
    fprintf( out, "%s\n", the_names[nn].name );
}


static void
names_dump_windef( FILE*  out )
{
  int  nn;

 /* note that we assume that __cdecl was used when compiling the
  * DLL object files.
  */
  fprintf( out, "DESCRIPTION  FreeType 2 DLL\n" );
  fprintf( out, "EXPORTS\n" );
  for ( nn = 0; nn < num_names; nn++ )
    fprintf( out, "  %s\n", the_names[nn].name );
}


/* states of the line parser */

typedef enum
{
  STATE_START = 0,  /* waiting for FT_EXPORT keyword and return type */
  STATE_TYPE,       /* type was read, waiting for function name      */

} State;

static int
read_header_file( FILE*  file, int  verbose )
{
  static char  buff[ LINEBUFF_SIZE+1 ];
  State        state = STATE_START;

  while ( !feof( file ) )
  {
    char*  p;

    if ( !fgets( buff, LINEBUFF_SIZE, file ) )
      break;

    p = buff;

    while ( *p && (*p == ' ' || *p == '\\') )  /* skip leading whitespace */
      p++;

    if ( *p == '\n' || *p == '\r' )  /* skip empty lines */
      continue;

    switch ( state )
    {
      case STATE_START:
        {
          if ( memcmp( p, "FT_EXPORT(", 10 ) != 0 )
            break;

          p += 10;
          for (;;)
          {
            if ( *p == 0 || *p == '\n' || *p == '\r' )
              goto NextLine;

            if ( *p == ')' )
            {
              p++;
              break;
            }

            p++;
          }

          state = STATE_TYPE;

         /* sometimes, the name is just after the FT_EXPORT(...), so
          * skip whitespace, and fall-through if we find an alphanumeric
          * character
          */
          while ( *p == ' ' || *p == '\t' )
            p++;

          if ( !isalpha(*p) )
            break;
        }
        /* fall-through */

      case STATE_TYPE:
        {
          char*   name = p;
          size_t  func_len;

          while ( isalpha(*p) || *p == '_' )
            p++;

          if ( p > name )
          {
            if ( verbose )
              fprintf( stderr, ">>> %.*s\n", p-name, name );

            names_add( name, p );
          }

          state = STATE_START;
        }
        break;

      default:
        ;
    }

  NextLine:
    ;
  }

  return 0;
}


static void
usage( void )
{
  fprintf( stderr,
           "%s %s: extract FreeType API names from header files\n\n"
           "this program is used to extract the list of public FreeType API\n"
           "functions. It receives the list of header files as argument and\n"
           "generates a sorted list of unique identifiers\n\n"

           "usage: %s header1 [options] [header2 ...]\n\n"

           "options:   -  : parse the content of stdin, ignore arguments\n"
           "           -v : verbose mode\n",
           "           -w : output windows .DEF file\n"
           ,
           PROGRAM_NAME,
           PROGRAM_VERSION,
           PROGRAM_NAME
           );
  exit(1);
}


int  main( int argc, const char* const*  argv )
{
  int  from_stdin = 0;
  int  verbose = 0;
  int  do_win_def = 0;

  if ( argc < 2 )
    usage();

  /* '-' used as a single argument means read source file from stdin */
  while ( argc > 1 && argv[1][0] == '-' )
  {
    switch ( argv[1][1] )
    {
      case 'v':
        verbose = 1;
        break;

      case 'w':
        do_win_def = 1;
        break;

      case 0:
        from_stdin = 1;
        break;

      default:
        usage();
    }

    argc--;
    argv++;
  }

  if ( from_stdin )
  {
    read_header_file( stdin, verbose );
  }
  else
  {
    for ( --argc, argv++; argc > 0; argc--, argv++ )
    {
      FILE*  file = fopen( argv[0], "rb" );

      if ( file == NULL )
        fprintf( stderr, "unable to open '%s'\n", argv[0] );
      else
      {
        if ( verbose )
          fprintf( stderr, "opening '%s'\n", argv[0] );

        read_header_file( file, verbose );
        fclose( file );
      }
    }
  }

  if ( num_names == 0 )
    panic( "could not find exported functions !!\n" );

  names_sort();

  if ( do_win_def )
    names_dump_windef( stdout );
  else
    names_dump( stdout );

  return 0;
}
