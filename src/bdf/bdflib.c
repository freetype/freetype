/*
 * Copyright 2000 Computing Research Labs, New Mexico State University
 * Copyright 2001 Francesco Zappa Nardelli
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COMPUTING RESEARCH LAB OR NEW MEXICO STATE UNIVERSITY BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
 * OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
static char rcsid[] = "$Id$";
*/

#include <ft2build.h>

#include FT_INTERNAL_DEBUG_H
#include FT_INTERNAL_STREAM_H
#include FT_INTERNAL_OBJECTS_H

#include "bdf.h"

#include "bdferror.h"

#undef MAX
#define MAX(h, i) ((h) > (i) ? (h) : (i))

#undef MIN
#define MIN(l, o) ((l) < (o) ? (l) : (o))

/**************************************************************************
 *
 * Masks used for checking different bits per pixel cases.
 *
 **************************************************************************/

static const unsigned char onebpp[]   = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
static const unsigned char twobpp[]   = { 0xc0, 0x30, 0x0c, 0x03 };
static const unsigned char fourbpp[]  = { 0xf0, 0x0f };
static const unsigned char eightbpp[] = { 0xff };

/**************************************************************************
 *
 * Default BDF font options.
 *
 **************************************************************************/

static const bdf_options_t _bdf_opts =
{
    1,                /* Hint TTF glyphs.               */
    1,                /* Correct metrics.               */
    1,                /* Preserve unencoded glyphs.     */
    1,                /* Preserve comments.             */
    1,                /* Pad character-cells.           */
    BDF_PROPORTIONAL, /* Default spacing.               */
    12,               /* Default point size.            */
    0,                /* Default horizontal resolution. */
    0,                /* Default vertical resolution.   */
    1,                /* Bits per pixel.                */
    BDF_UNIX_EOL,     /* Line separator.                */
};

/**************************************************************************
 *
 * Builtin BDF font properties.
 *
 **************************************************************************/

/*
 * List of most properties that might appear in a font.  Doesn't include the
 * RAW_* and AXIS_* properties in X11R6 polymorphic fonts.
 */
static const bdf_property_t _bdf_properties[] =
{
    {"ADD_STYLE_NAME",          BDF_ATOM,     1},
    {"AVERAGE_WIDTH",           BDF_INTEGER,  1},
    {"AVG_CAPITAL_WIDTH",       BDF_INTEGER,  1},
    {"AVG_LOWERCASE_WIDTH",     BDF_INTEGER,  1},
    {"CAP_HEIGHT",              BDF_INTEGER,  1},
    {"CHARSET_COLLECTIONS",     BDF_ATOM,     1},
    {"CHARSET_ENCODING",        BDF_ATOM,     1},
    {"CHARSET_REGISTRY",        BDF_ATOM,     1},
    {"COMMENT",                 BDF_ATOM,     1},
    {"COPYRIGHT",               BDF_ATOM,     1},
    {"DEFAULT_CHAR",            BDF_CARDINAL, 1},
    {"DESTINATION",             BDF_CARDINAL, 1},
    {"DEVICE_FONT_NAME",        BDF_ATOM,     1},
    {"END_SPACE",               BDF_INTEGER,  1},
    {"FACE_NAME",               BDF_ATOM,     1},
    {"FAMILY_NAME",             BDF_ATOM,     1},
    {"FIGURE_WIDTH",            BDF_INTEGER,  1},
    {"FONT",                    BDF_ATOM,     1},
    {"FONTNAME_REGISTRY",       BDF_ATOM,     1},
    {"FONT_ASCENT",             BDF_INTEGER,  1},
    {"FONT_DESCENT",            BDF_INTEGER,  1},
    {"FOUNDRY",                 BDF_ATOM,     1},
    {"FULL_NAME",               BDF_ATOM,     1},
    {"ITALIC_ANGLE",            BDF_INTEGER,  1},
    {"MAX_SPACE",               BDF_INTEGER,  1},
    {"MIN_SPACE",               BDF_INTEGER,  1},
    {"NORM_SPACE",              BDF_INTEGER,  1},
    {"NOTICE",                  BDF_ATOM,     1},
    {"PIXEL_SIZE",              BDF_INTEGER,  1},
    {"POINT_SIZE",              BDF_INTEGER,  1},
    {"QUAD_WIDTH",              BDF_INTEGER,  1},
    {"RAW_ASCENT",              BDF_INTEGER,  1},
    {"RAW_AVERAGE_WIDTH",       BDF_INTEGER,  1},
    {"RAW_AVG_CAPITAL_WIDTH",   BDF_INTEGER,  1},
    {"RAW_AVG_LOWERCASE_WIDTH", BDF_INTEGER,  1},
    {"RAW_CAP_HEIGHT",          BDF_INTEGER,  1},
    {"RAW_DESCENT",             BDF_INTEGER,  1},
    {"RAW_END_SPACE",           BDF_INTEGER,  1},
    {"RAW_FIGURE_WIDTH",        BDF_INTEGER,  1},
    {"RAW_MAX_SPACE",           BDF_INTEGER,  1},
    {"RAW_MIN_SPACE",           BDF_INTEGER,  1},
    {"RAW_NORM_SPACE",          BDF_INTEGER,  1},
    {"RAW_PIXEL_SIZE",          BDF_INTEGER,  1},
    {"RAW_POINT_SIZE",          BDF_INTEGER,  1},
    {"RAW_PIXELSIZE",           BDF_INTEGER,  1},
    {"RAW_POINTSIZE",           BDF_INTEGER,  1},
    {"RAW_QUAD_WIDTH",          BDF_INTEGER,  1},
    {"RAW_SMALL_CAP_SIZE",      BDF_INTEGER,  1},
    {"RAW_STRIKEOUT_ASCENT",    BDF_INTEGER,  1},
    {"RAW_STRIKEOUT_DESCENT",   BDF_INTEGER,  1},
    {"RAW_SUBSCRIPT_SIZE",      BDF_INTEGER,  1},
    {"RAW_SUBSCRIPT_X",         BDF_INTEGER,  1},
    {"RAW_SUBSCRIPT_Y",         BDF_INTEGER,  1},
    {"RAW_SUPERSCRIPT_SIZE",    BDF_INTEGER,  1},
    {"RAW_SUPERSCRIPT_X",       BDF_INTEGER,  1},
    {"RAW_SUPERSCRIPT_Y",       BDF_INTEGER,  1},
    {"RAW_UNDERLINE_POSITION",  BDF_INTEGER,  1},
    {"RAW_UNDERLINE_THICKNESS", BDF_INTEGER,  1},
    {"RAW_X_HEIGHT",            BDF_INTEGER,  1},
    {"RELATIVE_SETWIDTH",       BDF_CARDINAL, 1},
    {"RELATIVE_WEIGHT",         BDF_CARDINAL, 1},
    {"RESOLUTION",              BDF_INTEGER,  1},
    {"RESOLUTION_X",            BDF_CARDINAL, 1},
    {"RESOLUTION_Y",            BDF_CARDINAL, 1},
    {"SETWIDTH_NAME",           BDF_ATOM,     1},
    {"SLANT",                   BDF_ATOM,     1},
    {"SMALL_CAP_SIZE",          BDF_INTEGER,  1},
    {"SPACING",                 BDF_ATOM,     1},
    {"STRIKEOUT_ASCENT",        BDF_INTEGER,  1},
    {"STRIKEOUT_DESCENT",       BDF_INTEGER,  1},
    {"SUBSCRIPT_SIZE",          BDF_INTEGER,  1},
    {"SUBSCRIPT_X",             BDF_INTEGER,  1},
    {"SUBSCRIPT_Y",             BDF_INTEGER,  1},
    {"SUPERSCRIPT_SIZE",        BDF_INTEGER,  1},
    {"SUPERSCRIPT_X",           BDF_INTEGER,  1},
    {"SUPERSCRIPT_Y",           BDF_INTEGER,  1},
    {"UNDERLINE_POSITION",      BDF_INTEGER,  1},
    {"UNDERLINE_THICKNESS",     BDF_INTEGER,  1},
    {"WEIGHT",                  BDF_CARDINAL, 1},
    {"WEIGHT_NAME",             BDF_ATOM,     1},
    {"X_HEIGHT",                BDF_INTEGER,  1},
    {"_MULE_BASELINE_OFFSET",   BDF_INTEGER,  1},
    {"_MULE_RELATIVE_COMPOSE",  BDF_INTEGER,  1},
};

static const FT_ULong _num_bdf_properties = FT_NUM_ELEMENT(_bdf_properties);

/*
 * User defined properties.
 */
/*static bdf_property_t *user_props;
  static unsigned long nuser_props = 0;*/

/**************************************************************************
 *
 * Hash table utilities for the properties.
 *
 **************************************************************************/

#define INITIAL_HT_SIZE 241

typedef void (*hash_free_func)(hashnode node);

static hashnode*
hash_bucket(char *key, hashtable *ht)
{
  char*         kp  = key;
  unsigned long res = 0;
  hashnode*     bp  = ht->table, *ndp;

  /*
   * Mocklisp hash function.
   */
  while (*kp)
    res = (res << 5) - res + *kp++;

  ndp = bp + (res % ht->size);
  while (*ndp)
  {
    kp = (*ndp)->key;

    if (kp[0] == key[0] && ft_strcmp(kp, key) == 0)
      break;

    ndp--;
    if (ndp < bp)
      ndp = bp + (ht->size - 1);
  }
  return ndp;
}


static  FT_Error
hash_rehash ( hashtable*  ht,
              FT_Memory   memory )
{
  hashnode *obp = ht->table, *bp, *nbp;
  int i, sz = ht->size;
  FT_Error error;

  ht->size <<= 1;
  ht->limit  = ht->size / 3;

  if ( FT_NEW_ARRAY( ht->table , ht->size ) )
    return error;

  for (i = 0, bp = obp; i < sz; i++, bp++)
  {
    if (*bp)
    {
      nbp = hash_bucket((*bp)->key, ht);
      *nbp = *bp;
    }
  }
  FT_FREE(obp);

  return FT_Err_Ok;
}


static FT_Error
hash_init ( hashtable*  ht,
            FT_Memory   memory )
{
  int      sz = INITIAL_HT_SIZE;
  FT_Error error;

  ht->size  = sz;
  ht->limit = sz / 3;
  ht->used  = 0;

  if ( FT_NEW_ARRAY( ht->table, size ) )
    return error;

  return FT_Err_Ok;
}


static void
hash_free( hashtable*  ht,
           FT_Memory   memory )
{
  /* FT_Error error; */

  if ( ht != 0 )
  {
    int        i, sz = ht->size;
    hashnode*  bp    = ht->table;

    for (i = 0; i < sz; i++, bp++)
    {
      if (*bp)
        FT_FREE(*bp);
    }
    if (sz > 0)
      FT_FREE(ht->table);
  }
}


static FT_Error
hash_insert ( char*       key,
              void*       data,
              hashtable*  ht,
              FT_Memory   memory )
{
  FT_Error  error   = FT_Err_Ok;
  hashnode  nn, *bp = hash_bucket(key, ht);

  nn = *bp;
  if (!nn)
  {
    if ( FT_NEW( nn ) )
      return error;

    *bp      = nn;
    nn->key  = key;
    nn->data = data;

    if (ht->used >= ht->limit)
      error = hash_rehash(ht, memory);

    ht->used++;
  }
  else
    nn->data = data;

  return error;
}


static hashnode
hash_lookup(char *key, hashtable *ht)
{
  hashnode *np = hash_bucket(key, ht);
  return   *np;
}

#ifdef 0
static void
hash_delete(char *name, hashtable *ht , FT_Memory memory)
{
  hashnode *hp;
  /* FT_Error error; */

  hp = hash_bucket(name, ht);
  FT_FREE( *hp );
}
#endif

/*
 * The builtin property table.
 */
/*static hashtable proptbl; */ /* XXX eliminate this */



/**************************************************************************
 *
 * Utility types and functions.
 *
 **************************************************************************/

/*
 * Function type for parsing lines of a BDF font.
 */
typedef int (*_bdf_line_func_t)( char*         line,
                                 unsigned long linelen,
                                 unsigned long lineno,
                                 void*         call_data,
                                 void*         client_data );

/*
 * List structure for splitting lines into fields.
 */
typedef struct
{
  char**         field;
  unsigned long  size;
  unsigned long  used;
  char*          bfield;
  unsigned long  bsize;
  unsigned long  bused;

} _bdf_list_t;


/*
 * Structure used while loading BDF fonts.
 */
typedef struct
{
  unsigned long          flags;
  unsigned long          cnt;
  unsigned long          row;
  unsigned long          bpr;
  short                  minlb;
  short                  maxlb;
  short                  maxrb;
  short                  maxas;
  short                  maxds;
  short                  rbearing;
  char*                  glyph_name;
  long                   glyph_enc;
  bdf_font_t*            font;
  bdf_options_t*         opts;
  void*                  client_data;
  bdf_callback_t         callback;
  bdf_callback_struct_t  cb;
  unsigned long          have[2048];
  _bdf_list_t            list;

  FT_Memory              memory;

} _bdf_parse_t;

#define setsbit(m, cc) (m[(cc) >> 3] |= (1 << ((cc) & 7)))
#define sbitset(m, cc) (m[(cc) >> 3] & (1 << ((cc) & 7)))

/*
 * An empty string for empty fields.
 */
static const char empty[1] = { 0 };   /* XXX eliminate this */

/*
 * Assume the line is NULL terminated and that the `list' parameter was
 * initialized the first time it was used.
 */
static FT_Error
_bdf_split ( char*          separators,
             char*          line,
             unsigned long  linelen,
             _bdf_list_t*   list,
             FT_Memory      memory )
{
  int mult, final_empty;
  char *sp, *ep, *end;
  unsigned char seps[32];
  FT_Error error;

  /*
   * Initialize the list.
   */
  list->used = list->bused = 0;

  /*
   * If the line is empty, then simply return.
   */
  if ( linelen == 0 || line[0] == 0 )
    return FT_Err_Ok;

  /*
   * If the `separators' parameter is NULL or empty, split the list into
   * individual bytes.
   */
  if ( separators == 0 || *separators == 0 )
  {
    if ( linelen > list->bsize )
    {
      if ( list->bsize )
      {
        if ( FT_ALLOC ( list->bfield , linelen) )
          return error;
      }
      else
      {
        if ( FT_REALLOC ( list->bfield , list->bsize, linelen) )
          return error;
      }
      list->bsize = linelen;
    }
    list->bused = linelen;

    FT_MEM_COPY (list->bfield, line, linelen);
    return FT_Err_Ok;
  }

  /*
   * Prepare the separator bitmap.
   */
  FT_MEM_ZERO( seps, 32 );

  /*
   * If the very last character of the separator string is a plus, then set
   * the `mult' flag to indicate that multiple separators should be
   * collapsed into one.
   */
  for ( mult = 0, sp = separators; sp && *sp; sp++ )
  {
    if ( sp[0] == '+' && sp[1] == 0)
      mult = 1;
    else
      setsbit( seps, sp[0] );
  }

  /*
   * Break the line up into fields.
   */
  final_empty = 0;
  sp          = ep = line;
  end         = sp + linelen;
  for ( ; sp < end && *sp;)
  {
    /*
     * Collect everything that is not a separator.
     */
    for ( ; *ep && !sbitset( seps, *ep ); ep++ ) ;

    /*
     * Resize the list if necessary.
     */
    if ( list->used == list->size )
    {
      if ( list->size == 0 )
      {
        if ( FT_NEW_ARRAY( list->field , 5) )
          return error;
      }
      else
      {
        if ( FT_RENEW_ARRAY( list->field , list->size, list->size+5 )
          return error;
      }
      list->size += 5;
    }

    /*
     * Assign the field appropriately.
     */
    list->field[ list->used++ ] = (ep > sp) ? sp : empty;

    sp = ep;
    if (mult)
    {
      /*
       * If multiple separators should be collapsed, do it now by
       * setting all the separator characters to 0.
       */
      for ( ; *ep && sbitset(seps, *ep); ep++ )
        *ep = 0;

    }
    else if (*ep != 0)
    {
      /*
       * Don't collapse multiple separators by making them 0, so just
       * make the one encountered 0.
       */
      *ep++ = 0;
    }

    final_empty = ( ep > sp && *ep == 0 );
    sp          = ep;
  }

  /*
   * Finally, NULL terminate the list.
   */
  if ( list->used + final_empty + 1 >= list->size )
  {
    if ( list->used == list->size )
    {
      if ( list->size == 0 )
      {
        if ( FT_NEW_ARRAY( list->field, 5 ) )
          return error;
      }
      else
      {
        if ( FT_RENEW_ARRAY( list->field , list->size, list->size+5 ) )
          return error;
      }
      list->size += 5;
    }
  }

  if (final_empty)
    list->field[ list->used++ ] = empty;

  if ( list->used == list->size )
  {
    if ( list->size == 0 )
    {
      if ( FT_NEW_ARRAY( list->field , 5 ) )
        return error;
    }
    else
    {
      if ( FT_NEW_ARRAY( list->field, list->size, list->size + 5 ) )
        return error;
    }
    list->size += 5;
  }
  list->field[ list->used ] = 0;

  return FT_Err_Ok;
}


static void
_bdf_shift( unsigned long  n,
            _bdf_list_t*   list)
{
  unsigned long i, u;

  if ( list == 0 || list->used == 0 || n == 0 )
    return;

  if ( n >= list->used )
  {
    list->used = 0;
    return;
  }
  for ( u = n, i = 0; u < list->used; i++, u++ )
    list->field[i] = list->field[u];

  list->used -= n;
}


static char*
_bdf_join( int             c,
           unsigned long*  len,
           _bdf_list_t*    list)
{
  unsigned long i, j;
  char *fp, *dp;

  if ( list == 0 || list->used == 0 )
    return 0;

  *len = 0;

  dp = list->field[0];

  for ( i = j = 0; i < list->used; i++ )
  {
    fp = list->field[i];
    while (*fp)
      dp[j++] = *fp++;
      
    if (i + 1 < list->used)
      dp[j++] = c;
  }
  dp[j] = 0;

  *len = j;
  return dp;
}

/*
 * High speed file reader that passes each line to a callback.
 */
int ftreadstream( FT_Stream  stream,
                  char*      buffer,
                  int        count )
{
  int read_bytes;
  int pos = stream->pos;

  if ( pos >= stream->size )
  {
    FT_ERROR(( "FT_Read_Stream_At:" ));
    FT_ERROR(( " invalid i/o; pos = 0x%lx, size = 0x%lx\n",
               pos, stream->size ));
    return 0;
  }

  if ( stream->read )
    read_bytes = stream->read( stream, pos, buffer, count );
  else
  {
    read_bytes = stream->size - pos;
    if ( read_bytes > count )
      read_bytes = count;

    ft_memcpy( buffer, stream->base + pos, read_bytes );
  }

  stream->pos = pos + read_bytes;

  return read_bytes;
}

static int
_bdf_readstream( FT_Stream         stream,
                 _bdf_line_func_t  callback,
                 void*             client_data,
                 unsigned long*    lno)
{
  _bdf_line_func_t cb;
  unsigned long lineno;
  int n, res, done, refill, bytes, hold;
  char *ls, *le, *pp, *pe, *hp;
  char buf[65536];

  if (callback == 0)
    return -1;

  cb     = callback;
  lineno = 1;
  buf[0] = 0;
  res    = done = 0;
  pp     = ls = le = buf;
  bytes  = 65536;

  while ( !done && (n = ftreadstream(stream, pp, bytes)) > 0 )
  {
    /*
     * Determine the new end of the buffer pages.
     */
    pe = pp + n;

    for (refill = 0; done == 0 && refill == 0; )
    {
      while (le < pe && *le != '\n' && *le != '\r')
        le++;

      if (le == pe)
      {
        /*
         * Hit the end of the last page in the buffer.  Need to find
         * out how many pages to shift and how many pages need to be
         * read in.  Adjust the line start and end pointers down to
         * point to the right places in the pages.
         */
        pp = buf + (((ls - buf) >> 13) << 13);
        n = pp - buf;
        ls -= n;
        le -= n;
        n   = pe - pp;
        (void) ft_memcpy(buf, pp, n);
        pp = buf + n;
        bytes = 65536 - n;
        refill = 1;
      }
      else
      {
        /*
         * Temporarily NULL terminate the line.
         */
        hp   = le;
        hold = *le;
        *le  = 0;

        if (callback && *ls != '#' && *ls != 0x1a && le > ls &&
            (res = (*cb)(ls, le - ls, lineno, (void *) &cb,
                         client_data)) != 0)
          done = 1;
        else {
            ls = ++le;
            /*
             * Handle the case of DOS crlf sequences.
             */
            if (le < pe && hold == '\n' && *le =='\r')
              ls = ++le;
        }

        /*
         * Increment the line number.
         */
        lineno++;

        /*
         * Restore the character at the end of the line.
         */
        *hp = hold;
      }
    }
  }
  *lno = lineno;
  return res;
}


FT_LOCAL_DEF( void )
_bdf_memmove(char *dest, char *src, unsigned long bytes)
{
    long i, j;

    i = (long) bytes;
    j = i & 7;
    i = (i + 7) >> 3;

    /*
     * Do a memmove using Ye Olde Duff's Device for efficiency.
     */
    if (src < dest) {
        src += bytes;
        dest += bytes;

        switch (j) {
          case 0: do {
              *--dest = *--src;
            case 7: *--dest = *--src;
            case 6: *--dest = *--src;
            case 5: *--dest = *--src;
            case 4: *--dest = *--src;
            case 3: *--dest = *--src;
            case 2: *--dest = *--src;
            case 1: *--dest = *--src;
          } while (--i > 0);
        }
    } else if (src > dest) {
        switch (j) {
          case 0: do {
              *dest++ = *src++;
            case 7: *dest++ = *src++;
            case 6: *dest++ = *src++;
            case 5: *dest++ = *src++;
            case 4: *dest++ = *src++;
            case 3: *dest++ = *src++;
            case 2: *dest++ = *src++;
            case 1: *dest++ = *src++;
          } while (--i > 0);
        }
    }
}

static const unsigned char a2i[128] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const unsigned char odigits[32] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const unsigned char ddigits[32] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x03,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const unsigned char hdigits[32] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x03,
    0x7e, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

#define isdigok(m, d) (m[(d) >> 3] & (1 << ((d) & 7)))

/*
 * Routine to convert an ASCII string into an unsigned long integer.
 */
static unsigned long
_bdf_atoul(char *s, char **end, int base)
{
  unsigned long v;
  unsigned char *dmap;

  if (s == 0 || *s == 0)
    return 0;

  /*
   * Make sure the radix is something recognizable.  Default to 10.
   */
  switch (base)
  {
    case 8: dmap = odigits; break;
    case 16: dmap = hdigits; break;
    default: base = 10; dmap = ddigits; break;
  }

  /*
   * Check for the special hex prefix.
   */
  if ( s[0] == '0' && ( s[1] == 'x' || s[1] == 'X'))
  {
    base = 16;
    dmap = hdigits;
    s   += 2;
  }

  for ( v = 0; isdigok(dmap, *s); s++ )
    v = (v * base) + a2i[(int) *s];

  if (end != 0)
    *end = s;

  return v;
}


/*
 * Routine to convert an ASCII string into an signed long integer.
 */
static long
_bdf_atol(char *s, char **end, int base)
{
  long v, neg;
  unsigned char *dmap;

  if (s == 0 || *s == 0)
    return 0;

  /*
   * Make sure the radix is something recognizable.  Default to 10.
   */
  switch (base) {
    case 8: dmap = odigits; break;
    case 16: dmap = hdigits; break;
    default: base = 10; dmap = ddigits; break;
  }

  /*
   * Check for a minus sign.
   */
  neg = 0;
  if (*s == '-') {
      s++;
      neg = 1;
  }

  /*
   * Check for the special hex prefix.
   */
  if (*s == '0' && (*(s + 1) == 'x' || *(s + 1) == 'X')) {
      base = 16;
      dmap = hdigits;
      s += 2;
  }

  for (v = 0; isdigok(dmap, *s); s++)
    v = (v * base) + a2i[(int) *s];

  if (end != 0)
    *end = s;

  return (!neg) ? v : -v;
}


/*
 * Routine to convert an ASCII string into an signed short integer.
 */
static short
_bdf_atos(char *s, char **end, int base)
{
    short v, neg;
    unsigned char *dmap;

    if (s == 0 || *s == 0)
      return 0;

    /*
     * Make sure the radix is something recognizable.  Default to 10.
     */
    switch (base) {
      case 8: dmap = odigits; break;
      case 16: dmap = hdigits; break;
      default: base = 10; dmap = ddigits; break;
    }

    /*
     * Check for a minus.
     */
    neg = 0;
    if (*s == '-') {
        s++;
        neg = 1;
    }

    /*
     * Check for the special hex prefix.
     */
    if (*s == '0' && (*(s + 1) == 'x' || *(s + 1) == 'X')) {
        base = 16;
        dmap = hdigits;
        s += 2;
    }

    for (v = 0; isdigok(dmap, *s); s++)
      v = (v * base) + a2i[(int) *s];

    if (end != 0)
      *end = s;

    return (!neg) ? v : -v;
}

/*
 * Routine to compare two glyphs by encoding so they can be sorted.
 */
static int
by_encoding(const void *a, const void *b)
{
    bdf_glyph_t *c1, *c2;

    c1 = (bdf_glyph_t *) a;
    c2 = (bdf_glyph_t *) b;
    if (c1->encoding < c2->encoding)
      return -1;
    else if (c1->encoding > c2->encoding)
      return 1;
    return 0;
}



static FT_Error
bdf_create_property( char*        name,
                     int          format,
                     bdf_font_t*  font )
{
  unsigned long    n;
  bdf_property_t*  p;
  FT_Memory        memory = font->memory;
  FT_Error         error;

  /*
   * First check to see if the property has
   * already been added or not.  If it has, then
   * simply ignore it.
   */
  if ( hash_lookup( name, &(font->proptbl)) )
    return FT_Err_Ok;

  if (font->nuser_props == 0)
  {
    if ( FT_NEW( font->user_props ) )
      return error;
  }
  else
  {
    if ( FT_RENEW_ARRAY( font->user_props, font->nuser_props,
                                          (font->nuser_props + 1) ) )
      return error;
  }

  p = font->user_props + font->nuser_props;
  
  FT_ZERO( p );
  
  n = (unsigned long) (ft_strlen(name) + 1);
  
  if ( FT_ALLOC ( p->name , n) )
    return error;
    
  FT_MEM_COPY(p->name, name, n);
  p->format  = format;
  p->builtin = 0;

  n = _num_bdf_properties + font->nuser_props;

  error = hash_insert(p->name, (void *) n, &(font->proptbl) , memory);
  if (error) return error;

  font->nuser_props++;
  return FT_Err_Ok;
}


FT_LOCAL_DEF( bdf_property_t* )
bdf_get_property(char *name, bdf_font_t *font)
{
  hashnode hn;
  unsigned long propid;

  if (name == 0 || *name == 0)
    return 0;

  if ((hn = hash_lookup(name, &(font->proptbl))) == 0)
    return 0;

  propid = (unsigned long) hn->data;
  if (propid >= _num_bdf_properties)
    return font->user_props + (propid - _num_bdf_properties);

  return _bdf_properties + propid;
}


/**************************************************************************
 *
 * BDF font file parsing flags and functions.
 *
 **************************************************************************/

/*
 * Parse flags.
 */
#define _BDF_START     0x0001
#define _BDF_FONT_NAME 0x0002
#define _BDF_SIZE      0x0004
#define _BDF_FONT_BBX  0x0008
#define _BDF_PROPS     0x0010
#define _BDF_GLYPHS    0x0020
#define _BDF_GLYPH     0x0040
#define _BDF_ENCODING  0x0080
#define _BDF_SWIDTH    0x0100
#define _BDF_DWIDTH    0x0200
#define _BDF_BBX       0x0400
#define _BDF_BITMAP    0x0800

#define _BDF_SWIDTH_ADJ 0x1000

#define _BDF_GLYPH_BITS (_BDF_GLYPH|_BDF_ENCODING|_BDF_SWIDTH|\
                         _BDF_DWIDTH|_BDF_BBX|_BDF_BITMAP)

#define _BDF_GLYPH_WIDTH_CHECK 0x40000000
#define _BDF_GLYPH_HEIGHT_CHECK 0x80000000

/*
 * Auto correction messages.
 */
#define ACMSG1 "FONT_ASCENT property missing.  Added \"FONT_ASCENT %hd\"."
#define ACMSG2 "FONT_DESCENT property missing.  Added \"FONT_DESCENT %hd\"."
#define ACMSG3 "Font width != actual width.  Old: %hd New: %hd."
#define ACMSG4 "Font left bearing != actual left bearing.  Old: %hd New: %hd."
#define ACMSG5 "Font ascent != actual ascent.  Old: %hd New: %hd."
#define ACMSG6 "Font descent != actual descent.  Old: %hd New: %hd."
#define ACMSG7 "Font height != actual height. Old: %hd New: %hd."
#define ACMSG8 "Glyph scalable width (SWIDTH) adjustments made."
#define ACMSG9 "SWIDTH field missing at line %ld.  Set automatically."
#define ACMSG10 "DWIDTH field missing at line %ld.  Set to glyph width."
#define ACMSG11 "SIZE bits per pixel field adjusted to %hd."
#define ACMSG12 "Duplicate encoding %ld (%s) changed to unencoded."
#define ACMSG13 "Glyph %ld extra rows removed."
#define ACMSG14 "Glyph %ld extra columns removed."
#define ACMSG15 "Incorrect glyph count: %ld indicated but %ld found."

/*
 * Error messages.
 */
#define ERRMSG1 "[line %ld] Missing \"%s\" line."
#define ERRMSG2 "[line %ld] Font header corrupted or missing fields."
#define ERRMSG3 "[line %ld] Font glyphs corrupted or missing fields."

static FT_Error
_bdf_add_acmsg ( bdf_font_t*    font,
                 char*          msg,
                 unsigned long  len )
{
  char*      cp;
  FT_Memory  memory = font->memory;
  FT_Error   error;

  if ( font->acmsgs_len == 0 )
  {
    if ( FT_ALLOC ( font->acmsgs , len + 1 ) )
      return error;
  }
  else
  {
    if ( FT_REALLOC ( font->acmsgs , font->acmsgs_len ,
                      font->acmsgs_len + len + 1 ) )
      return error;
  }

  cp = font->acmsgs + font->acmsgs_len;
  FT_MEM_COPY(cp, msg, len);
  cp   += len;
  *cp++ = '\n';
  font->acmsgs_len += len + 1;

  return FT_Err_Ok;
}


static FT_Error
_bdf_add_comment ( bdf_font_t*    font,
                   char*          comment,
                   unsigned long  len )
{
  char *cp;
  FT_Memory memory = font->memory;
  FT_Error error;

  if (font->comments_len == 0) {
    if ( FT_ALLOC ( font->comments , len + 1 ) )
      return error;
  }
  else
  {
    if ( FT_REALLOC ( font->comments , font->comments_len,
                      font->comments_len + len + 1) )
      return error;
  }

  cp = font->comments + font->comments_len;
  FT_MEM_COPY(cp, comment, len);
  cp += len;
  *cp++ = '\n';
  font->comments_len += len + 1;

  return FT_Err_Ok;
}

/*
 * Set the spacing from the font name if it exists, or set it to the default
 * specified in the options.
 */
static void
_bdf_set_default_spacing( bdf_font_t*     font,
                          bdf_options_t*  opts)
{
  unsigned long  len;
  char            name[128];
  _bdf_list_t     list;
  FT_Memory       memory;
  /*    FT_Error error; */

  if ( font == 0 || font->name == 0 || font->name[0] == 0 )
    return;

  memory = font->memory;

  font->spacing = opts->font_spacing;

  len = (unsigned long) ( ft_strlen(font->name) + 1 );
  (void) ft_memcpy(name, font->name, len);
  
  list.size = list.used = 0;
  _bdf_split("-", name, len, &list, memory);
  
  if (list.used == 15) {
      switch (list.field[11][0]) {
        case 'C': case 'c': font->spacing = BDF_CHARCELL; break;
        case 'M': case 'm': font->spacing = BDF_MONOWIDTH; break;
        case 'P': case 'p': font->spacing = BDF_PROPORTIONAL; break;
      }
  }
  if (list.size > 0)
    FT_FREE(list.field);
}


/*
 * Determine if the property is an atom or not.  If it is, then clean it up so
 * the double quotes are removed if they exist.
 */
static int
_bdf_is_atom( char*          line,
              unsigned long  linelen,
              char*         *name,
              char*         *value,
	      bdf_font_t*    font)
{
    int hold;
    char *sp, *ep;
    bdf_property_t *p;

    *name = sp = ep = line;
    while (*ep && *ep != ' ' && *ep != '\t')
      ep++;

    hold = -1;
    if (*ep)
    {
      hold = *ep;
      *ep  = 0;
    }

    p = bdf_get_property(sp, font);

    /*
     * Restore the character that was saved before any return can happen.
     */
    if (hold != -1)
      *ep = hold;

    /*
     * If the propert exists and is not an atom, just return here.
     */
    if (p && p->format != BDF_ATOM)
      return 0;

    /*
     * The property is an atom.  Trim all leading and trailing whitespace and
     * double quotes for the atom value.
     */
    sp = ep;
    ep = line + linelen;

    /*
     * Trim the leading whitespace if it exists.
     */
    *sp++ = 0;
    while (*sp && (*sp == ' ' || *sp == '\t'))
      sp++;

    /*
     * Trim the leading double quote if it exists.
     */
    if (*sp == '"')
      sp++;
    *value = sp;

    /*
     * Trim the trailing whitespace if it exists.
     */
    while (ep > sp && (*(ep - 1) == ' ' || *(ep - 1) == '\t'))
      *--ep = 0;

    /*
     * Trim the trailing double quote if it exists.
     */
    if (ep > sp && *(ep - 1) == '"')
      *--ep = 0;

    return 1;
}


static FT_Error
_bdf_add_property ( bdf_font_t*  font,
                    char*        name,
                    char*        value)
{
    unsigned long propid;
    hashnode hn;
    int len;
    bdf_property_t *prop, *fp;
    FT_Memory memory = font->memory;
    FT_Error error;
    /*    hashtable proptbl = font->proptbl;
    bdf_property_t *user_props = font->user_props;
    unsigned long nuser_props = font->nuser_props;
    */

     /*
     * First, check to see if the property already exists in the font.
     */
    if ((hn = hash_lookup(name, (hashtable *) font->internal)) != 0) {
        /*
         * The property already exists in the font, so simply replace
         * the value of the property with the current value.
         */
        fp = font->props + (unsigned long) hn->data;

        switch (prop->format)
        {
          case BDF_ATOM:
            {
              /*
               * Delete the current atom if it exists.
               */
              FT_FREE ( fp->value.atom );
  
              if (value == 0)
                len = 1;
              else
                len = ft_strlen(value) + 1;
              if (len > 1)
              {
  	      if ( FT_ALLOC ( fp->value.atom , len ) )
                  return error;
                  
                FT_MEM_COPY(fp->value.atom, value, len);
              }
              else
                fp->value.atom = 0;
            }
            break;
            
          case BDF_INTEGER:
            fp->value.int32 = _bdf_atol(value, 0, 10);
            break;
            
          case BDF_CARDINAL:
            fp->value.card32 = _bdf_atoul(value, 0, 10);
            break;
          
          default:
            ;
        }
        return FT_Err_Ok;
    }

    /*
     * See if this property type exists yet or not.  If not, create it.
     */
    hn = hash_lookup(name, &(font->proptbl));
    if (hn == 0) {
        bdf_create_property(name, BDF_ATOM, font);
        hn = hash_lookup(name, &(font->proptbl));
    }

    /*
     * Allocate another property if this is overflow.
     */
    if (font->props_used == font->props_size)
    {
      if (font->props_size == 0)
      {
        if ( FT_NEW( font->props ) )
          return error;
      }
      else
      {
        if ( FT_RENEW_ARRAY( font->props, font->props_size,
                                         (font->props_size + 1) ) )
          return error;
      }
      fp = font->props + font->props_size;
      FT_ZERO( fp );
      font->props_size++;
    }

    propid = (unsigned long) hn->data;
    if (propid >= _num_bdf_properties)
      prop = font->user_props + (propid - _num_bdf_properties);
    else
      prop = _bdf_properties + propid;

    fp = font->props + font->props_used;

    fp->name    = prop->name;
    fp->format  = prop->format;
    fp->builtin = prop->builtin;

    switch (prop->format)
    {
      case BDF_ATOM:
        {
          fp->value.atom = NULL;
          
          if ( value && value[0] != 0 )
          {
            len = ft_strlen(value) + 1;

  	    if ( FT_ALLOC ( fp->value.atom , len ) )
                return error;
                
             FT_MEM_COPY (fp->value.atom, value, len);
          }
        }
        break;

    case BDF_INTEGER:
      fp->value.int32 = _bdf_atol(value, 0, 10);
      break;

    case BDF_CARDINAL:
      fp->value.card32 = _bdf_atoul(value, 0, 10);
      break;
    
    default:
      ;
    }

    /*
     * If the property happens to be a comment, then it doesn't need
     * to be added to the internal hash table.
     */
    if ( ft_memcmp(name, "COMMENT", 7) != 0 )
      /*
       * Add the property to the font property table.
       */
      hash_insert( fp->name, (void *) font->props_used,
                   (hashtable *) font->internal, memory);

    font->props_used++;

    /*
     * Some special cases need to be handled here.  The DEFAULT_CHAR property
     * needs to be located if it exists in the property list, the FONT_ASCENT
     * and FONT_DESCENT need to be assigned if they are present, and the
     * SPACING property should override the default spacing.
     */
    if ( ft_memcmp(name, "DEFAULT_CHAR", 12) == 0 )
      font->default_glyph = fp->value.int32;
      
    else if ( ft_memcmp(name, "FONT_ASCENT", 11) == 0 )
      font->font_ascent = fp->value.int32;
      
    else if ( ft_memcmp(name, "FONT_DESCENT", 12) == 0 )
      font->font_descent = fp->value.int32;
      
    else if ( ft_memcmp(name, "SPACING", 7) == 0 )
    {
      if (fp->value.atom[0] == 'p' || fp->value.atom[0] == 'P')
        font->spacing = BDF_PROPORTIONAL;
        
      else if (fp->value.atom[0] == 'm' || fp->value.atom[0] == 'M')
        font->spacing = BDF_MONOWIDTH;
        
      else if (fp->value.atom[0] == 'c' || fp->value.atom[0] == 'C')
        font->spacing = BDF_CHARCELL;
    }

    return FT_Err_Ok;
}

/*
 * Actually parse the glyph info and bitmaps.
 */
static int
_bdf_parse_glyphs( char*          line,
                   unsigned long  linelen,
                   unsigned long  lineno,
                   void*          call_data,
                   void*          client_data)
{
  int c;
  char *s;
  unsigned char *bp;
  unsigned long i, slen, nibbles;
  double ps, rx, dw, sw;
  _bdf_line_func_t *next;
  _bdf_parse_t *p;
  bdf_glyph_t *glyph;
  bdf_font_t *font;
  char nbuf[128];
  FT_Memory memory;
  FT_Error error;

  next = (_bdf_line_func_t *) call_data;
  p = (_bdf_parse_t *) client_data;

  font = p->font;
  memory = font->memory;

  /*
   * Check for a comment.
   */
  if (ft_memcmp(line, "COMMENT", 7) == 0) {
      linelen -= 7;
      s = line + 7;
      if (*s != 0) {
          s++;
          linelen--;
      }
      _bdf_add_comment(p->font, s, linelen);
      return 0;
  }

  /*
   * The very first thing expected is the number of glyphs.
   */
  if (!(p->flags & _BDF_GLYPHS)) {
      if (ft_memcmp(line, "CHARS", 5) != 0) {
          sprintf(nbuf, ERRMSG1, lineno, "CHARS");
          _bdf_add_acmsg(p->font, nbuf, ft_strlen(nbuf));
          return BDF_MISSING_CHARS;
      }
      _bdf_split(" +", line, linelen, &p->list, memory);
      p->cnt = font->glyphs_size = _bdf_atoul(p->list.field[1], 0, 10);

      /*
       * Make sure the number of glyphs is non-zero.
       */
      if (p->cnt == 0)
        font->glyphs_size = 64;

if ( FT_ALLOC ( font->glyphs , sizeof(bdf_glyph_t) *
                      font->glyphs_size ) )
        return FT_Err_Out_Of_Memory;

      /*
       * Set up the callback to indicate the glyph loading is about to
       * begin.
       */
      if (p->callback != 0) {
          p->cb.reason = BDF_LOAD_START;
          p->cb.total = p->cnt;
          p->cb.current = 0;
          (*p->callback)(&p->cb, p->client_data);
      }
      p->flags |= _BDF_GLYPHS;
      return 0;
  }

  /*
   * Check for the ENDFONT field.
   */
  if (ft_memcmp(line, "ENDFONT", 7) == 0) {
      /*
       * Sort the glyphs by encoding.
       */
    qsort((char *) font->glyphs, font->glyphs_used, sizeof(bdf_glyph_t),
          by_encoding);

      p->flags &= ~_BDF_START;
      return 0;
  }

  /*
   * Check for the ENDCHAR field.
   */
  if (ft_memcmp(line, "ENDCHAR", 7) == 0) {
      /*
       * Set up and call the callback if it was passed.
       */
    if (p->callback != 0) {
      p->cb.reason = BDF_LOADING;
      p->cb.total = font->glyphs_size;
      p->cb.current = font->glyphs_used;
      (*p->callback)(&p->cb, p->client_data);
    }
    p->glyph_enc = 0;
    p->flags &= ~_BDF_GLYPH_BITS;
    return 0;
  }

  /*
   * Check to see if a glyph is being scanned but should be ignored
   * because it is an unencoded glyph.
   */
  if ((p->flags & _BDF_GLYPH) &&
      p->glyph_enc == -1 && p->opts->keep_unencoded == 0)
    return 0;

  /*
   * Check for the STARTCHAR field.
   */
  if (ft_memcmp(line, "STARTCHAR", 9) == 0) {
      /*
       * Set the character name in the parse info first until the
       * encoding can be checked for an unencoded character.
       */
    if (p->glyph_name != 0)
      FT_FREE(p->glyph_name);
    _bdf_split(" +", line, linelen, &p->list,memory);
    _bdf_shift(1, &p->list);
    s = _bdf_join(' ', &slen, &p->list);
    if ( FT_ALLOC ( p->glyph_name , (slen + 1) ) )
      return BDF_OUT_OF_MEMORY;
    FT_MEM_COPY(p->glyph_name, s, slen + 1);
    p->flags |= _BDF_GLYPH;
    return 0;
  }

  /*
   * Check for the ENCODING field.
   */
  if (ft_memcmp(line, "ENCODING", 8) == 0) {
    if (!(p->flags & _BDF_GLYPH)) {
      /*
       * Missing STARTCHAR field.
       */
      sprintf(nbuf, ERRMSG1, lineno, "STARTCHAR");
      _bdf_add_acmsg(font, nbuf, ft_strlen(nbuf));
      return BDF_MISSING_STARTCHAR;
    }
    _bdf_split(" +", line, linelen, &p->list, memory);
    p->glyph_enc = _bdf_atol(p->list.field[1], 0, 10);

    /*
     * Check to see if this encoding has already been encountered.  If it
     * has then change it to unencoded so it gets added if indicated.
     */
    if (p->glyph_enc >= 0) {
      if (_bdf_glyph_modified(p->have, p->glyph_enc)) {
        /*
         * Add a message saying a glyph has been moved to the
         * unencoded area.
         */
        sprintf(nbuf, ACMSG12, p->glyph_enc, p->glyph_name);
        _bdf_add_acmsg(font, nbuf, ft_strlen(nbuf));
        p->glyph_enc = -1;
        font->modified = 1;
      } else
        _bdf_set_glyph_modified(p->have, p->glyph_enc);
    }

    if (p->glyph_enc >= 0) {
      /*
       * Make sure there are enough glyphs allocated in case the
       * number of characters happen to be wrong.
       */
      if (font->glyphs_used == font->glyphs_size) {
        if ( FT_REALLOC ( font->glyphs,
                          sizeof(bdf_glyph_t) * font->glyphs_size,
                          sizeof(bdf_glyph_t) * (font->glyphs_size + 64) ) )
          return BDF_OUT_OF_MEMORY;
        FT_MEM_SET ((char *) (font->glyphs + font->glyphs_size),
                    0, sizeof(bdf_glyph_t) << 6); /* FZ inutile */
        font->glyphs_size += 64;
      }

      glyph = font->glyphs + font->glyphs_used++;
      glyph->name = p->glyph_name;
      glyph->encoding = p->glyph_enc;

      /*
       * Reset the initial glyph info.
       */
      p->glyph_name = 0;
    } else {
      /*
       * Unencoded glyph.  Check to see if it should be added or not.
       */
      if (p->opts->keep_unencoded != 0) {
        /*
         * Allocate the next unencoded glyph.
         */
        if (font->unencoded_used == font->unencoded_size) {
          if (font->unencoded_size == 0) {
            if ( FT_ALLOC ( font->unencoded , sizeof(bdf_glyph_t) << 2 ) )
              return BDF_OUT_OF_MEMORY;
          }
          else {
            if ( FT_REALLOC ( font->unencoded ,
                              sizeof(bdf_glyph_t) * font->unencoded_size,
                              sizeof(bdf_glyph_t) *
                              (font->unencoded_size + 4) ) )
              return BDF_OUT_OF_MEMORY;
          }
          font->unencoded_size += 4;
        }

        glyph = font->unencoded + font->unencoded_used;
        glyph->name = p->glyph_name;
        glyph->encoding = font->unencoded_used++;
      } else
        /*
         * Free up the glyph name if the unencoded shouldn't be
         * kept.
         */
        FT_FREE( p->glyph_name );

      p->glyph_name = 0;
    }

    /*
     * Clear the flags that might be added when width and height are
     * checked for consistency.
     */
    p->flags &= ~(_BDF_GLYPH_WIDTH_CHECK|_BDF_GLYPH_HEIGHT_CHECK);

    p->flags |= _BDF_ENCODING;
    return 0;
  }

  /*
   * Point at the glyph being constructed.
   */
  if (p->glyph_enc == -1)
    glyph = font->unencoded + (font->unencoded_used - 1);
  else
    glyph = font->glyphs + (font->glyphs_used - 1);

  /*
   * Check to see if a bitmap is being constructed.
   */
  if (p->flags & _BDF_BITMAP) {
      /*
       * If there are more rows than are specified in the glyph metrics,
       * ignore the remaining lines.
       */
      if (p->row >= glyph->bbx.height) {
          if (!(p->flags & _BDF_GLYPH_HEIGHT_CHECK)) {
              sprintf(nbuf, ACMSG13, glyph->encoding);
              _bdf_add_acmsg(font, nbuf, ft_strlen(nbuf));
              p->flags |= _BDF_GLYPH_HEIGHT_CHECK;
              font->modified = 1;
          }
          return 0;
      }

      /*
       * Only collect the number of nibbles indicated by the glyph metrics.
       * If there are more columns, they are simply ignored.
       */
      nibbles = p->bpr << 1;
      bp = glyph->bitmap + (p->row * p->bpr);
      for (i = 0, *bp = 0; i < nibbles; i++) {
          c = line[i];
          *bp = (*bp << 4) + a2i[c];
          if (i + 1 < nibbles && (i & 1))
            *++bp = 0;
      }

      /*
       * If any line has extra columns, indicate they have been removed.
       */
      if ((line[nibbles] == '0' || a2i[(int) line[nibbles]] != 0) &&
          !(p->flags & _BDF_GLYPH_WIDTH_CHECK)) {
          sprintf(nbuf, ACMSG14, glyph->encoding);
          _bdf_add_acmsg(font, nbuf, ft_strlen(nbuf));
          p->flags |= _BDF_GLYPH_WIDTH_CHECK;
          font->modified = 1;
      }

      p->row++;
      return 0;
  }

  /*
   * Expect the SWIDTH (scalable width) field next.
   */
  if (ft_memcmp(line, "SWIDTH", 6) == 0) {
      if (!(p->flags & _BDF_ENCODING)) {
          /*
           * Missing ENCODING field.
           */
          sprintf(nbuf, ERRMSG1, lineno, "ENCODING");
          _bdf_add_acmsg(font, nbuf, ft_strlen(nbuf));
          return BDF_MISSING_ENCODING;
      }
      _bdf_split(" +", line, linelen, &p->list, memory);
      glyph->swidth = _bdf_atoul(p->list.field[1], 0, 10);
      p->flags |= _BDF_SWIDTH;
      return 0;
  }

  /*
   * Expect the DWIDTH (scalable width) field next.
   */
  if (ft_memcmp(line, "DWIDTH", 6) == 0) {
      _bdf_split(" +", line, linelen, &p->list,memory);
      glyph->dwidth = _bdf_atoul(p->list.field[1], 0, 10);

      if (!(p->flags & _BDF_SWIDTH)) {
          /*
           * Missing SWIDTH field.  Add an auto correction message and set
           * the scalable width from the device width.
           */
          sprintf(nbuf, ACMSG9, lineno);
          _bdf_add_acmsg(font, nbuf, ft_strlen(nbuf));
          ps = (double) font->point_size;
          rx = (double) font->resolution_x;
          dw = (double) glyph->dwidth;
          glyph->swidth = (unsigned short) ((dw * 72000.0) / (ps * rx));
      }

      p->flags |= _BDF_DWIDTH;
      return 0;
  }

  /*
   * Expect the BBX field next.
   */
  if (ft_memcmp(line, "BBX", 3) == 0) {
      _bdf_split(" +", line, linelen, &p->list, memory);
      glyph->bbx.width = _bdf_atos(p->list.field[1], 0, 10);
      glyph->bbx.height = _bdf_atos(p->list.field[2], 0, 10);
      glyph->bbx.x_offset = _bdf_atos(p->list.field[3], 0, 10);
      glyph->bbx.y_offset = _bdf_atos(p->list.field[4], 0, 10);

      /*
       * Generate the ascent and descent of the character.
       */
      glyph->bbx.ascent = glyph->bbx.height + glyph->bbx.y_offset;
      glyph->bbx.descent = -glyph->bbx.y_offset;

      /*
       * Determine the overall font bounding box as the characters are
       * loaded so corrections can be done later if indicated.
       */
      p->maxas = MAX(glyph->bbx.ascent, p->maxas);
      p->maxds = MAX(glyph->bbx.descent, p->maxds);
      p->rbearing = glyph->bbx.width + glyph->bbx.x_offset;
      p->maxrb = MAX(p->rbearing, p->maxrb);
      p->minlb = MIN(glyph->bbx.x_offset, p->minlb);
      p->maxlb = MAX(glyph->bbx.x_offset, p->maxlb);

      if (!(p->flags & _BDF_DWIDTH)) {
          /*
           * Missing DWIDTH field.  Add an auto correction message and set
           * the device width to the glyph width.
           */
          sprintf(nbuf, ACMSG10, lineno);
          _bdf_add_acmsg(font, nbuf, ft_strlen(nbuf));
          glyph->dwidth = glyph->bbx.width;
      }

      /*
       * If the BDF_CORRECT_METRICS flag is set, then adjust the SWIDTH
       * value if necessary.
       */
      if (p->opts->correct_metrics != 0) {
          /*
           * Determine the point size of the glyph.
           */
          ps = (double) font->point_size;
          rx = (double) font->resolution_x;
          dw = (double) glyph->dwidth;
          sw = (unsigned short) ((dw * 72000.0) / (ps * rx));

          if (sw != glyph->swidth) {
              glyph->swidth = sw;
              if (p->glyph_enc == -1)
                _bdf_set_glyph_modified(font->umod,
                                        font->unencoded_used - 1);
              else
                _bdf_set_glyph_modified(font->nmod, glyph->encoding);
              p->flags |= _BDF_SWIDTH_ADJ;
              font->modified = 1;
          }
      }
      p->flags |= _BDF_BBX;
      return 0;
  }

  /*
   * And finally, gather up the bitmap.
   */
  if (ft_memcmp(line, "BITMAP", 6) == 0) {
      if (!(p->flags & _BDF_BBX)) {
          /*
           * Missing BBX field.
           */
          sprintf(nbuf, ERRMSG1, lineno, "BBX");
          _bdf_add_acmsg(font, nbuf, ft_strlen(nbuf));
          return BDF_MISSING_BBX;
      }
      /*
       * Allocate enough space for the bitmap.
       */
      p->bpr = ((glyph->bbx.width * p->font->bpp) + 7) >> 3;
      glyph->bytes = p->bpr * glyph->bbx.height;
      if ( FT_ALLOC ( glyph->bitmap , glyph->bytes ) )
        return BDF_OUT_OF_MEMORY;
      p->row = 0;
      p->flags |= _BDF_BITMAP;
      return 0;
  }

  return BDF_INVALID_LINE;
}

/*
* Load the font properties.
*/
static int
_bdf_parse_properties(char *line, unsigned long linelen, unsigned long lineno,
                    void *call_data, void *client_data)
{
  unsigned long vlen;
  _bdf_line_func_t *next;
  _bdf_parse_t *p;
  char *name, *value, nbuf[128];
  FT_Memory memory;

  next = (_bdf_line_func_t *) call_data;
  p = (_bdf_parse_t *) client_data;

  memory = p->font->memory;
  /*
   * Check for the end of the properties.
   */
  if (ft_memcmp(line, "ENDPROPERTIES", 13) == 0) {
      /*
       * If the FONT_ASCENT or FONT_DESCENT properties have not been
       * encountered yet, then make sure they are added as properties and
       * make sure they are set from the font bounding box info.
       *
       * This is *always* done regardless of the options, because X11
       * requires these two fields to compile fonts.
       */
      if (bdf_get_font_property(p->font, "FONT_ASCENT") == 0) {
          p->font->font_ascent = p->font->bbx.ascent;
          sprintf(nbuf, "%hd", p->font->bbx.ascent);
          _bdf_add_property(p->font, "FONT_ASCENT", nbuf);
          sprintf(nbuf, ACMSG1, p->font->bbx.ascent);
          _bdf_add_acmsg(p->font, nbuf, ft_strlen(nbuf));
          p->font->modified = 1;
      }
      if (bdf_get_font_property(p->font, "FONT_DESCENT") == 0) {
          p->font->font_descent = p->font->bbx.descent;
          sprintf(nbuf, "%hd", p->font->bbx.descent);
          _bdf_add_property(p->font, "FONT_DESCENT", nbuf);
          sprintf(nbuf, ACMSG2, p->font->bbx.descent);
          _bdf_add_acmsg(p->font, nbuf, ft_strlen(nbuf));
          p->font->modified = 1;
      }
      p->flags &= ~_BDF_PROPS;
      *next = _bdf_parse_glyphs;
      return 0;
  }

  /*
   * Ignore the _XFREE86_GLYPH_RANGES properties.
   */
  if (ft_memcmp(line, "_XFREE86_GLYPH_RANGES", 21) == 0)
    return 0;

  /*
   * Handle COMMENT fields and properties in a special way to preserve
   * the spacing.
   */
  if (ft_memcmp(line, "COMMENT", 7) == 0) {
      name = value = line;
      value += 7;
      if (*value)
        *value++ = 0;
      _bdf_add_property(p->font, name, value);
  } else if (_bdf_is_atom(line, linelen, &name, &value, p->font))
    _bdf_add_property(p->font, name, value);
  else {
      _bdf_split(" +", line, linelen, &p->list, memory);
      name = p->list.field[0];
      _bdf_shift(1, &p->list);
      value = _bdf_join(' ', &vlen, &p->list);
      _bdf_add_property(p->font, name, value);
  }

  return 0;
}

/*
 * Load the font header.
 */
static int
_bdf_parse_start(char *line, unsigned long linelen, unsigned long lineno,
                 void *call_data, void *client_data)
{
    unsigned long slen;
    _bdf_line_func_t *next;
    _bdf_parse_t *p;
    bdf_font_t *font;
    char *s, nbuf[128];
    /*    int test; */
    FT_Memory memory;
    FT_Error error;

    next = (_bdf_line_func_t *) call_data;
    p = (_bdf_parse_t *) client_data;
    if (p->font)
      memory = p->font->memory;

    /*
     * Check for a comment.  This is done to handle those fonts that have
     * comments before the STARTFONT line for some reason.
     */
    if (ft_memcmp(line, "COMMENT", 7) == 0) {
        if (p->opts->keep_comments != 0 && p->font != 0) {
            linelen -= 7;
            s = line + 7;
            if (*s != 0) {
                s++;
                linelen--;
            }
            _bdf_add_comment(p->font, s, linelen);
	    /* here font is not defined ! */
        }
        return 0;
    }

    if (!(p->flags & _BDF_START)) {

        memory = p->memory;

        if (ft_memcmp(line, "STARTFONT", 9) != 0)
          /*
           * No STARTFONT field is a good indication of a problem.
           */
          return BDF_MISSING_START;
        p->flags = _BDF_START;
        font = p->font = 0;

	if ( FT_ALLOC ( font, sizeof(bdf_font_t) ) )
	  return BDF_OUT_OF_MEMORY;
	p->font = font;

	font->memory = p->memory;
	p->memory = 0;

	/*	if (font == 0) {
          fprintf(stderr,"failed font\n");
	  }*/ /* XXX */

	{ /* setup */
	  unsigned long i;
	  bdf_property_t *prop;

	  hash_init(&(font->proptbl), memory);
	  for (i = 0, prop = _bdf_properties;
		 i < _num_bdf_properties; i++, prop++)
	    hash_insert(prop->name, (void *) i, &(font->proptbl) , memory);
	}

	if ( FT_ALLOC ( p->font->internal , sizeof(hashtable) ) )
	  return BDF_OUT_OF_MEMORY;
	hash_init((hashtable *) p->font->internal,memory);
	p->font->spacing = p->opts->font_spacing;
        p->font->default_glyph = -1;
	return 0;
    }

    /*
     * Check for the start of the properties.
     */
    if (ft_memcmp(line, "STARTPROPERTIES", 15) == 0) {
        _bdf_split(" +", line, linelen, &p->list, memory);
        p->cnt = p->font->props_size = _bdf_atoul(p->list.field[1], 0, 10);

	if ( FT_ALLOC ( p->font->props , (sizeof(bdf_property_t) * p->cnt) ) )
	  return BDF_OUT_OF_MEMORY;
        p->flags |= _BDF_PROPS;
        *next = _bdf_parse_properties;
        return 0;
    }

    /*
     * Check for the FONTBOUNDINGBOX field.
     */
    if (ft_memcmp(line, "FONTBOUNDINGBOX", 15) == 0) {
        if (!(p->flags & _BDF_SIZE)) {
            /*
             * Missing the SIZE field.
             */
            sprintf(nbuf, ERRMSG1, lineno, "SIZE");
            _bdf_add_acmsg(p->font, nbuf, ft_strlen(nbuf));
            return BDF_MISSING_SIZE;
        }
        _bdf_split(" +", line, linelen, &p->list , memory);
        p->font->bbx.width = _bdf_atos(p->list.field[1], 0, 10);
        p->font->bbx.height = _bdf_atos(p->list.field[2], 0, 10);
        p->font->bbx.x_offset = _bdf_atos(p->list.field[3], 0, 10);
        p->font->bbx.y_offset = _bdf_atos(p->list.field[4], 0, 10);
        p->font->bbx.ascent = p->font->bbx.height + p->font->bbx.y_offset;
        p->font->bbx.descent = -p->font->bbx.y_offset;
        p->flags |= _BDF_FONT_BBX;
        return 0;
    }

    /*
     * The next thing to check for is the FONT field.
     */
    if (ft_memcmp(line, "FONT", 4) == 0) {
        _bdf_split(" +", line, linelen, &p->list , memory);
        _bdf_shift(1, &p->list);
        s = _bdf_join(' ', &slen, &p->list);
	if ( FT_ALLOC ( p->font->name , slen + 1 ) )
	  return BDF_OUT_OF_MEMORY;
        (void) ft_memcpy(p->font->name, s, slen + 1);
        /*
         * If the font name is an XLFD name, set the spacing to the one in the
         * font name.  If there is no spacing fall back on the default.
         */
        _bdf_set_default_spacing(p->font, p->opts);
        p->flags |= _BDF_FONT_NAME;
	return 0;
    }

    /*
     * Check for the SIZE field.
     */
    if (ft_memcmp(line, "SIZE", 4) == 0) {
        if (!(p->flags & _BDF_FONT_NAME)) {
            /*
             * Missing the FONT field.
             */
            sprintf(nbuf, ERRMSG1, lineno, "FONT");
            _bdf_add_acmsg(p->font, nbuf, ft_strlen(nbuf));
            return BDF_MISSING_FONTNAME;
        }
        _bdf_split(" +", line, linelen, &p->list, memory);
        p->font->point_size = _bdf_atoul(p->list.field[1], 0, 10);
        p->font->resolution_x = _bdf_atoul(p->list.field[2], 0, 10);
        p->font->resolution_y = _bdf_atoul(p->list.field[3], 0, 10);

        /*
         * Check for the bits per pixel field.
         */
        if (p->list.used == 5) {
            p->font->bpp = _bdf_atos(p->list.field[4], 0, 10);
            if (p->font->bpp > 1 && (p->font->bpp & 1)) {
                /*
                 * Move up to the next bits per pixel value if an odd number
                 * is encountered.
                 */
                p->font->bpp++;
                if (p->font->bpp <= 4) {
                    sprintf(nbuf, ACMSG11, p->font->bpp);
                    _bdf_add_acmsg(p->font, nbuf, ft_strlen(nbuf));
                }
            }
            if (p->font->bpp > 4) {
                sprintf(nbuf, ACMSG11, p->font->bpp);
                _bdf_add_acmsg(p->font, nbuf, ft_strlen(nbuf));
                p->font->bpp = 4;
            }
        } else
          p->font->bpp = 1;

        p->flags |= _BDF_SIZE;
        return 0;
    }

    return BDF_INVALID_LINE;
}

/**************************************************************************
 *
 * API.
 *
 **************************************************************************/


FT_LOCAL_DEF( bdf_font_t* )
bdf_load_font( FT_Stream       stream,
               FT_Memory       extmemory,
               bdf_options_t*  opts,
               bdf_callback_t  callback,
               void*           data)
{
    int n;
    unsigned long lineno;
    char msgbuf[128];
    _bdf_parse_t p;
    FT_Memory memory;
    FT_Error error;

    (void) ft_memset((char *) &p, 0, sizeof(_bdf_parse_t));
    p.opts = (opts != 0) ? opts : &_bdf_opts;
    p.minlb = 32767;
    p.callback = callback;
    p.client_data = data;

    p.memory = extmemory;  /* only during font creation */

    n = _bdf_readstream(stream, _bdf_parse_start, (void *) &p, &lineno);

    if (p.font != 0) {
        /*
         * If the font is not proportional, set the fonts monowidth
         * field to the width of the font bounding box.
         */
      memory = p.font->memory;

        if (p.font->spacing != BDF_PROPORTIONAL)
          p.font->monowidth = p.font->bbx.width;

        /*
         * If the number of glyphs loaded is not that of the original count,
         * indicate the difference.
         */
        if (p.cnt != p.font->glyphs_used + p.font->unencoded_used) {
            sprintf(msgbuf, ACMSG15, p.cnt,
                    p.font->glyphs_used + p.font->unencoded_used);
            _bdf_add_acmsg(p.font, msgbuf, ft_strlen(msgbuf));
            p.font->modified = 1;
        }

        /*
         * Once the font has been loaded, adjust the overall font metrics if
         * necessary.
         */
        if (p.opts->correct_metrics != 0 &&
            (p.font->glyphs_used > 0 || p.font->unencoded_used > 0)) {
            if (p.maxrb - p.minlb != p.font->bbx.width) {
                sprintf(msgbuf, ACMSG3, p.font->bbx.width, p.maxrb - p.minlb);
                _bdf_add_acmsg(p.font, msgbuf, ft_strlen(msgbuf));
                p.font->bbx.width = p.maxrb - p.minlb;
                p.font->modified = 1;
            }
            if (p.font->bbx.x_offset != p.minlb) {
                sprintf(msgbuf, ACMSG4, p.font->bbx.x_offset, p.minlb);
                _bdf_add_acmsg(p.font, msgbuf, ft_strlen(msgbuf));
                p.font->bbx.x_offset = p.minlb;
                p.font->modified = 1;
            }
            if (p.font->bbx.ascent != p.maxas) {
                sprintf(msgbuf, ACMSG5, p.font->bbx.ascent, p.maxas);
                _bdf_add_acmsg(p.font, msgbuf, ft_strlen(msgbuf));
                p.font->bbx.ascent = p.maxas;
                p.font->modified = 1;
            }
            if (p.font->bbx.descent != p.maxds) {
                sprintf(msgbuf, ACMSG6, p.font->bbx.descent, p.maxds);
                _bdf_add_acmsg(p.font, msgbuf, ft_strlen(msgbuf));
                p.font->bbx.descent = p.maxds;
                p.font->bbx.y_offset = -p.maxds;
                p.font->modified = 1;
            }
            if (p.maxas + p.maxds != p.font->bbx.height) {
                sprintf(msgbuf, ACMSG7, p.font->bbx.height, p.maxas + p.maxds);
                _bdf_add_acmsg(p.font, msgbuf, ft_strlen(msgbuf));
            }
            p.font->bbx.height = p.maxas + p.maxds;

            if (p.flags & _BDF_SWIDTH_ADJ)
              _bdf_add_acmsg(p.font, ACMSG8, ft_strlen(ACMSG8));
        }
    }

    /*
     * Last, if an error happened during loading, handle the messages.
     */
    if (n < 0 && callback != 0) {
        /*
         * An error was returned.  Alert the client.
         */
        p.cb.reason = BDF_ERROR;
        p.cb.errlineno = lineno;
        (*callback)(&p.cb, data);
    } else if (p.flags & _BDF_START) {
        if (p.font != 0) {
            /*
             * The ENDFONT field was never reached or did not exist.
             */
            if (!(p.flags & _BDF_GLYPHS))
              /*
               * Error happened while parsing header.
               */
              sprintf(msgbuf, ERRMSG2, lineno);
            else
              /*
               * Error happened when parsing glyphs.
               */
              sprintf(msgbuf, ERRMSG3, lineno);

            _bdf_add_acmsg(p.font, msgbuf, ft_strlen(msgbuf));
        }

        if (callback != 0) {
            p.cb.reason = BDF_ERROR;
            p.cb.errlineno = lineno;
            (*callback)(&p.cb, data);
        }
    } else if (callback != 0) {
        /*
         * This forces the progress bar to always finish.
         */
        p.cb.current = p.cb.total;
        (*p.callback)(&p.cb, p.client_data);
    }

    /*
     * Free up the list used during the parsing.
     */
    if (p.list.size > 0)
      FT_FREE( p.list.field );

    if (p.font != 0) {
        /*
         * Make sure the comments are NULL terminated if they exist.
         */
        memory = p.font->memory;

        if (p.font->comments_len > 0) {
	  if ( FT_REALLOC ( p.font->comments , p.font->comments_len ,
			 p.font->comments_len + 1 ) )
	    return 0;
            p.font->comments[p.font->comments_len] = 0;
        }

        /*
         * Make sure the auto-correct messages are NULL terminated if they
         * exist.
         */
        if (p.font->acmsgs_len > 0) {
	  memory = p.font->memory;

	  if ( FT_REALLOC ( p.font->acmsgs , p.font->acmsgs_len ,
			 p.font->acmsgs_len + 1 ) )
	    return 0;
            p.font->acmsgs[p.font->acmsgs_len] = 0;
        }
    }

    return p.font;
}


FT_LOCAL_DEF( void )
bdf_free_font( bdf_font_t *font )
{
  bdf_property_t *prop;
    unsigned long i;
    bdf_glyph_t *glyphs;
    FT_Memory memory;

    if (font == 0)
        return;

    memory = font->memory;

    if (font->name != 0)
      FT_FREE(font->name);

    /*
     * Free up the internal hash table of property names.
     */
    if (font->internal) {
      hash_free((hashtable *) font->internal, memory);
      FT_FREE(font->internal);
    }
    /*
     * Free up the comment info.
     */
    if (font->comments_len > 0)
      FT_FREE(font->comments);

    /*
     * Free up the auto-correction messages.
     */
    if (font->acmsgs_len > 0)
      FT_FREE(font->acmsgs);

    /*
     * Free up the properties.
     */
    for (i = 0; i < font->props_size; i++) {
        if (font->props[i].format == BDF_ATOM && font->props[i].value.atom)
          FT_FREE(font->props[i].value.atom);
    }

    if (font->props_size > 0 && font->props != 0)
      FT_FREE(font->props);

    /*
     * Free up the character info.
     */
    for (i = 0, glyphs = font->glyphs; i < font->glyphs_used; i++, glyphs++) {
        if (glyphs->name)
          FT_FREE(glyphs->name);
        if (glyphs->bytes > 0 && glyphs->bitmap != 0)
          FT_FREE(glyphs->bitmap);
    }

    for (i = 0, glyphs = font->unencoded; i < font->unencoded_used;
         i++, glyphs++) {
        if (glyphs->name)
          FT_FREE(glyphs->name);
        if (glyphs->bytes > 0)
          FT_FREE(glyphs->bitmap);
    }

    if (font->glyphs_size > 0)
      FT_FREE( font->glyphs);

    if (font->unencoded_size > 0)
      FT_FREE( font->unencoded);

    /*
     * Free up the overflow storage if it was used.
     */
    for (i = 0, glyphs = font->overflow.glyphs; i < font->overflow.glyphs_used;
         i++, glyphs++) {
      if (glyphs->name != 0)
	FT_FREE(glyphs->name);
      if (glyphs->bytes > 0)
	FT_FREE( glyphs->bitmap);;
    }
    if (font->overflow.glyphs_size > 0)
      FT_FREE(font->overflow.glyphs);

    /* bdf_cleanup */
    hash_free(&(font->proptbl),memory);

    /*
     * Free up the user defined properties.
     */
    for (prop = font->user_props, i = 0; i < font->nuser_props; i++, prop++) {
      FT_FREE(prop->name);
      if (prop->format == BDF_ATOM && prop->value.atom != 0)
	FT_FREE(prop->value.atom);
    }
    if (font->nuser_props > 0)
      FT_FREE(font->user_props);

    /*FREE( font);*/ /* XXX Fixme */
}



FT_LOCAL_DEF( bdf_property_t* )
bdf_get_font_property( bdf_font_t*  font,
                       char*        name)
{
    hashnode hn;

    if (font == 0 || font->props_size == 0 || name == 0 || *name == 0)
      return 0;

    hn = hash_lookup(name, (hashtable *) font->internal);
    return (hn) ? (font->props + ((unsigned long) hn->data)) : 0;
}
