#!/bin/bash

# Include our configuration
. ./CI/ft-tests.config

# Usage: ./CI/ft-regression.sh <commit to test against> <test to run (a number or "all")>

# The following script checks the source code in the current folder for
# regressions or changes against previous commits. This is the main script you 
# should be calling and the rest are more for utility.

# These scripts requires xvfb (for capturing demos), imagick 
# (for comparing images) and pretty-diff which can be installed via npm  
# (for diffing text)

# Below is utility function used to build both versions of freetype and 
# respective demos exes linked against the specific builds
function build() {
  # Here we set the script to exit on any failed command because if the build 
  # fails, then there's nothing to compare so no point in continuing.
  set -e
  ./autogen.sh
  ./configure
  make
  pushd ..
  [[ -d ./freetype2-demos ]]\
  || git clone ${DEMOS_URL}
  pushd freetype2-demos
  git checkout `git rev-list -n 1 --first-parent --before="$(git show -s --format=%ci $1)" master`
  make
  popd
  popd
  # Unset exit on error so the tests will continue to run despie any errors.
  set +e
}

# The script wasn't passed a git hash to compare so we exit early and print.
if [ -z "$1" ]; then
  echo "No commit specified to check out master for regression tests."
  echo "Aborting!"
  exit 1
fi

if [ -z "$2" ]; then
  echo "No tests specified running all tests"
  export RUN_TEST="all"
else
  export RUN_TEST="$2"
fi

# We want a fresh checkout and work dir in order to prevent any stale changes 
# from slipping through.
rm -rf "${COMP_COMMIT_DIR}" "${TEST_OUTDIR}"

# Here we store the current directories shortened commit hash which is used as 
# the subdirectory name for where to dump the logs and images.
PREV_GIT_HASH=$(git log --pretty=format:'%h' -n 1)

# Again we export this because it's used in the other scripts later.
export PREVIOUS_PWD=${PWD}

# This is the first build/run of the current dir that will dump our "A" 
# metricts.
build ${PREV_GIT_HASH}
./CI/ft-test.sh

# Next we set up our "B" directory by copyiing our current sources.
echo "Copying ${PWD} to ${COMP_COMMIT_DIR}"
cp -p -r "${PWD}" "${COMP_COMMIT_DIR}"
pushd "${COMP_COMMIT_DIR}"

# Clean before we checkout
make clean

# Here we stash / clean in the copied "B" directory to prevent any issues
# checking out the desired test commit.
if [[ "${PWD}" == "${COMP_COMMIT_DIR}" ]]; then
  git stash
  git clean -f -d
  git checkout "$1"
  GIT_HASH=$(git log --pretty=format:'%h' -n 1)
  # Here we start the build / metrics for build "B" 
  build ${GIT_HASH}
  # We call the scripts from the current directory as they may not exist in 
  # previous commits and to prevent confusion when modifying them. One set of
  # scripts is called but on each call they call different versions of the demos
  # exes.
  ${PREVIOUS_PWD}/CI/ft-test.sh ${PREV_GIT_HASH} ${GIT_HASH}
  EXIT=$?
  popd
else
  echo "Failed to change directory to ${COMP_COMMIT_DIR}.\
    Something is horribly wrong..."
  echo "Aborting!"
  exit 1
fi

exit 0
