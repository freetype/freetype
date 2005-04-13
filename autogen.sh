#!/bin/sh

run ()
{
  echo "running \`$*'"
  eval $*

  if test $? != 0 ; then
    echo "error while running \`$*'"
    exit 1
  fi
}

if test ! -f ./builds/unix/configure.ac; then
  echo "You must be in the same directory as \`autogen.sh'."
  echo "Bootstrapping doesn't work if srcdir != builddir."
  exit 1
fi

cd builds/unix

run aclocal -I . --force
run libtoolize --force --copy
run autoconf --force

chmod +x mkinstalldirs
chmod +x install-sh

cd ../..

chmod +x ./configure

# EOF
