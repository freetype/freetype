#!/bin/bash

# Include our configuration
. ${PREVIOUS_PWD}/CI/ft-tests.config

# Arguments
FONT=$1
SIZE=$2
COMMIT_A=$3
COMMIT_B=$4

# This script generates diffs and html reports from the metrics previously 
# generated.

EXIT=0

metrics_dir="$(basename ${FONT})/${SIZE}/"
diff_dir="${TEST_OUTDIR}/${COMMIT_A}_${COMMIT_B}_diffs/${metrics_dir}"
mkdir -p "${diff_dir}"

OUTDIR_A="${TEST_OUTDIR}/${COMMIT_A}/$(basename ${FONT})/${SIZE}/"
OUTDIR_B="${TEST_OUTDIR}/${COMMIT_B}/$(basename ${FONT})/${SIZE}/"

RELATIVE_OUTDIR_A="${COMMIT_A}/$(basename ${FONT})/${SIZE}/"
RELATIVE_OUTDIR_B="${COMMIT_B}/$(basename ${FONT})/${SIZE}/"

# Function below generates a page to compare images. The image should change 
# between the two versions on mouse over to easily spot differences.
function write_img_comp() {
  echo "<!DOCTYPE html>
  <html>
  <head>
  <style>
  .comp {
    width: $(identify -format '%w' $1)px;
    height: $(identify -format '%h' $1)px;
    background: url(\"${RELATIVE_OUTDIR_A}/$(basename $1)\") no-repeat;
    display: inline-block;
  }
  .comp:hover {
    background: url(\"${RELATIVE_OUTDIR_B}/$(basename $1)\") no-repeat;
  }
  .diff {
    width: $(identify -format '%w' $1)px;
    height: $(identify -format '%h' $1)px;
    background: url(\"${diff_dir}/$(basename $1 .png)_diff.png\") no-repeat;
    display: inline-block;
  }
  </style>
  </head>
  <body>
  <h2>
  Comparison of $(basename $1) between commits ${COMMIT_A} and ${COMMIT_B}
  </h2>
  <div class=\"comp\"></div>
  <div class=\"diff\"></div>
  </body>
  </html>"
}

# Here we start generating html report. It is just a simple table of pass / fail
# results.
echo "<!DOCTYPE html>
<html>
<head>
<style>
table, tr {
  border: 1px solid black;
  display:table;
  margin-right:auto;
  margin-left:auto;
  width:100%;
}

th.fail {
 color: red       
}

th.pass {
 color: green      
}

</style>
</head>
<body>

<h2>Freetype2 Difference Report</h2>
<h3>
Commit A: ${COMMIT_A}
Commit B: ${COMMIT_B}
</h3>
<h3>
Font: $(basename $1)
</h3>
<h3>
Size: $2
</h3>
<table>"

PASS=()
FAIL=()
FILES=${OUTDIR_A}/*

for f in $FILES
do
  filename=$(basename $f)
  extension="${filename##*.}"
  
  # If is png compare with imagick
  if [[ "$extension" == "png" ]];
  then
    diif_cmd=$(compare -metric AE -highlight-color Red -lowlight-color PaleGreen $f ${OUTDIR_B}/$(basename $f)\
     "${diff_dir}/$(basename $f .png)_diff.png" &> /dev/null)
    result=$?
  # Else if txt compare with
  else
    diif_cmd=$(diff $f ${OUTDIR_B}/$(basename $f) &>\
     $diff_dir/$(basename $f .$extension).diff) # we only care about return result
    result=$?
  fi  
    
  # Generate approriate diff page
  PAGE="$(basename $f .$extension)_${COMMIT_A}_${COMMIT_B}_diff.html"
  if [ "$result" -eq "0" ]; 
  then
    PASS+="$f "
  else
    if [[ "$extension" == "png" ]];
    then
      write_img_comp $f &> "${TEST_OUTDIR}/${COMMIT_A}_${COMMIT_B}_diffs/$PAGE"
    else
      DISPLAY=-1 pretty-diff $f ${OUTDIR_B}/$(basename $f)
      mv /tmp/diff.html "${TEST_OUTDIR}/${COMMIT_A}_${COMMIT_B}_diffs/$PAGE"
    fi
    FAIL+="$f "
  fi
done

# Below we fill out the results table
echo "<tr><th class=\"fail\">FAIL</th></tr>"

for f in $FAIL
do
  filename=$(basename $f)
  extension="${filename##*.}"
  if [[ "$filename" != "bench.txt" ]];
  then
    EXIT=1
  fi
  PAGE="$(basename $f .$extension).html"
  echo "
  <tr>
    <td><a href=\"${COMMIT_A}_${COMMIT_B}_diffs/$(basename $f .$extension)_${COMMIT_A}_${COMMIT_B}_diff.html\">$(basename $f)</a></td>
  </tr>"
done

echo "<tr><th class=\"pass\">PASS</th></tr>"

for f in $PASS
do
  echo "
  <tr>
    <td><a href=\"${COMMIT_A}_${COMMIT_B}_diffs/${COMMIT_A}/${metrics_dir}/$(basename $f)\">$(basename $f)</a></td>
  </tr>"
done

echo "
</table>

</body>
</html>"

# Move our tests results into dir so html refrences work
cp -R ${TEST_OUTDIR}/${COMMIT_A}/ ${TEST_OUTDIR}/${COMMIT_A}_${COMMIT_B}_diffs &> /dev/null
cp -R ${TEST_OUTDIR}/${COMMIT_B}/ ${TEST_OUTDIR}/${COMMIT_A}_${COMMIT_B}_diffs &> /dev/null

exit $EXIT
