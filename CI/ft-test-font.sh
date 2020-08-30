#!/bin/bash

# Include our configuration
. ${PREVIOUS_PWD}/CI/ft-tests.config

# Arguments
COMMIT=$1
FONT=$2
SIZE=$3
DPI=$4
DUMP=$5
BENCH=$6
VIEW=$7
STRING=$8
START_GLYPH=$9
END_GLYPH=${10}

OUT_DIR="${TEST_OUTDIR}/${COMMIT}/$(basename ${FONT})/${SIZE}/"

# Ensure directory we want to write to exists.
mkdir -p $OUT_DIR

# This script dumps specified metrics for a specified font. The arguments listed
# above allow you to specify which metricts to dum when this script is called 
# from ft-test.sh


# Below sets up a virtual X framebuffer to run the demos in. It is 1024x768 at 
# 24b depth. If you need a larget screen to fit the demos change the resolution
# here.
function startX {
  if [ ! -f "/tmp/.X${1}-lock" ]; then
    Xvfb :"$1" -screen 0 1024x768x24 &
    # Wait 2 seconds to ensure X is ready.
    sleep 2
  fi
}

# Below is a utility function to run a command inside the virtual frame buffer,
# wait 1 second and capture the screen and kill it.The first two arguments are 
# the xvfb ID and the output image file name.
function xvfbRunAndScreenShot {
  display="$1"
  shift
  screenshot_name="$1"
  shift
  DISPLAY=:"$display" $@ &
  PID=$!
  sleep 1
  DISPLAY=:"$display" xwd -root -silent\
   | convert -colorspace sRGB -define png:color-type=2 -trim xwd:- png:${OUT_DIR}/${screenshot_name}
  kill $PID
}

# Because witing 1 second for every command can be slow we set up multiple xvfbs
# in order to expedite the proccess.
for worker in $(seq 1 $WORKERS)
do
  startX $((98 + ${worker})) &
done

# Below are simple metricts to test that only require one run per font. 
if [[ "$DUMP" -eq 1 ]]; then
  ${DEMOS_DIR}/ftdump ${FONT} &> ${OUT_DIR}/dump.txt
fi

if [[ "$BENCH" -eq 1 ]]; then
  ${DEMOS_DIR}/ftbench ${FONT} &> ${OUT_DIR}/bench.txt
fi

if [[ "$STRING" -eq 1 ]]; then
  xvfbRunAndScreenShot 99 "ftstring.png" ${DEMOS_DIR}/ftstring\
   -r $DPI ${SIZE} ${FONT}
fi

if [[ "$VIEW" -eq 1 ]]; then
  xvfbRunAndScreenShot 99 "ftview.png" ${DEMOS_DIR}/ftview -r $DPI ${SIZE} ${FONT}
fi

# For ftgrid we run it on thousands of glyphys so this loop takes advantage of
# the above mentioned xvfb workers to speed it up.
for GLYPH in $(seq ${START_GLYPH} ${END_GLYPH})
do 
   ((i=i%WORKERS)); ((i++==0)) && wait
   echo "$(basename $FONT) ftgrid ${GLYPH}"
   # Pad name with 0s to maintain order
   GLYPH_padded=$(printf "%04d" ${GLYPH})
   xvfbRunAndScreenShot $((98 + ${i})) "ftgrid_${GLYPH_padded}.png"\
    ${DEMOS_DIR}/ftgrid -r $DPI -f ${GLYPH} ${SIZE} ${FONT} &
done

sleep 2 # wait for workers to finish up
#killall Xvfb
sleep 2 # wait for all xvfb to die
