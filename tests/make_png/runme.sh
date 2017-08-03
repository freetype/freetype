TEST_DIR=$1
BASE_DIR=$PWD

cd $TEST_DIR/include/freetype/config/
sed -i 's/\/\* #define FT_CONFIG_OPTION_SUBPIXEL_RENDERING \*\//#define FT_CONFIG_OPTION_SUBPIXEL_RENDERING /g' ftoption.h

cd $TEST_DIR
./autogen.sh
./configure --prefix=$BASE_DIR/test/
make 
make install 

cd $BASE_DIR/../..
./autogen.sh
./configure --prefix=$BASE_DIR/base/
make 
make install

cd $BASE_DIR
make

BASE_LIB=./base/lib/libfreetype.so
TEST_LIB=./test/lib/libfreetype.so

FONT_FILE=$2
PT_SIZE=$3
RENDER_MODE=$4

./sprite $BASE_LIB $TEST_LIB $FONT_FILE $PT_SIZE $RENDER_MODE

