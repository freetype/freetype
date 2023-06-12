/*
 *  This is a cheap replacement for getopt() because that routine is not
 *  available on some platforms and behaves differently on other platforms.
 *
 *  This code is hereby expressly placed in the public domain.
 *  mleisher@crl.nmsu.edu (Mark Leisher)
 *  10 October 1997
 */

#ifndef MLGETOPT_H_
#define MLGETOPT_H_

#ifdef VMS
#include <stdio.h>
#define getopt local_getopt
#define optind local_optind
#define opterr local_opterr
#define optarg local_optarg
#endif

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

#ifdef __cplusplus
  }
#endif

#endif /* MLGETOPT_H_ */


/* End */
