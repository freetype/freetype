rm -f ./html/images/*

BASE_DIR=$1
TEST_DIR=../..

BASE_LIB=$BASE_DIR/objs/.libs/libfreetype.so
TEST_LIB=$TEST_DIR/objs/.libs/libfreetype.so

FONT_FILE=$2
PT_SIZE=$3

echo
echo "*** Generating Images ***"
echo 

./tests $BASE_LIB $TEST_LIB $FONT_FILE $PT_SIZE

echo "Font:  " $FONT_FILE
echo "Size:  " $PT_SIZE

# Removing the current DPI and Render Mode settings
# for future compilations. 
rm -f ./render_modes ./dpi



