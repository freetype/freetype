/*
 *  This is a cheap replacement for getopt() because that routine is not
 *  available on some platforms and behaves differently on other platforms.
 *
 *  This code is hereby expressly placed in the public domain.
 *  mleisher@crl.nmsu.edu (Mark Leisher)
 *  10 October 1997
 */

#ifndef _H_COMMON
#define _H_COMMON

/* Note that by default, both functions are implemented in common.c */

#ifdef __cplusplus
  extern "C" {
#endif

  extern int    opterr;
  extern int    optind;
  extern char*  optarg;

  extern int  getopt(
#ifdef __STDC__
    int           argc,
    char* const*  argv,
    const char*   pattern
#endif
  );


  extern char*  ft_basename(
#ifdef __STDC__
    const char*  name
#endif
  );

  /* print a message and exit */
  extern  void  Panic  (
#ifdef __STDC__
    const char*  fmt, ...
#endif
  );


#ifdef __cplusplus
  }
#endif

#endif /* _H_COMMON */


/* End */
