#!/usr/bin/bash
# Download test fonts used by the FreeType regression test programs.
# These will be copied to $FREETYPE/tests/data/
# Each font file contains an 8-hexchar prefix corresponding to its md5sum

set -e

export LANG=C
export LC_ALL=C

PROGDIR=$(dirname "$0")
PROGNAME=$(basename "$0")

# Download a file from a given URL
#
# $1: URL
# $2: Destination directory
# $3: If not empty, destination file name. Default is to take
# the URL's basename.
#
download_file () {
  local URL=$1
  local DST_DIR=$2
  local DST_FILE=$3
  if [[ -z "$DST_FILE" ]]; then
    DST_FILE=$(basename "$URL")
  fi
  echo "URL: $URL"
  wget -q -O "$DST_DIR/$DST_FILE" "$URL"
}

# $1: URL
# $2: Destination directory
# $3+: Optional file list, otherwise the full archive is extracted to $2
download_and_extract_zip () {
  local URL=$1
  local DST_DIR=$2
  shift
  shift
  TEMP_DST_DIR=$(mktemp -d)
  TEMP_DST_NAME="a.zip"
  download_file "$URL" "$TEMP_DST_DIR" "$TEMP_DST_NAME"
  unzip -qo "$TEMP_DST_DIR/$TEMP_DST_NAME" -d "$DST_DIR" "$@"
  rm -rf "$TEMP_DST_DIR"
}

# $1: File path
# $2: Expected md5sum
md5sum_check () {
  local FILE=$1
  local EXPECTED=$2
  local HASH=$(md5sum "$FILE" | cut -d" " -f1)
  if [[ "$EXPECTED" != "$HASH" ]]; then
    echo "$FILE: Invalid md5sum $HASH expected $EXPECTED"
    return 1
  fi
}

INSTALL_DIR=$(cd $PROGDIR/.. && pwd)/data

mkdir -p "$INSTALL_DIR"

# See https://gitlab.freedesktop.org/freetype/freetype/-/issues/1063
download_and_extract_zip "https://github.com/python-pillow/Pillow/files/6622147/As.I.Lay.Dying.zip" "$INSTALL_DIR"
mv "$INSTALL_DIR/As I Lay Dying.ttf" "$INSTALL_DIR/As.I.Lay.Dying.ttf"
md5sum_check "$INSTALL_DIR/As.I.Lay.Dying.ttf" e153d60e66199660f7cfe99ef4705ad7
