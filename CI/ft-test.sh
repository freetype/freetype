#!/bin/bash

# Include our configuration
. ${PREVIOUS_PWD}/CI/ft-tests.config

# Arguments to /ft-test-font.sh
#COMMIT=$1
#FONT=$2
#SIZE=$3
#DPI=$4
#DUMP=$5
#BENCH=$6
#VIEW=$7
#STRING=$8
#START_GLYPH=$9
#END_GLYPH=$10

# This script is where one might add additional tests. Currently, it cycles
# through the below directory and call the metric dumping script for any files
# in that directory. Currently, it only tests all files at pt 16 and dpi 72 and
# with ft-grid's default rendering mode.
  
GIT_HASH=$(git log --pretty=format:'%h' -n 1)

EXIT=0
PASS=()
FAIL=()

# Ensure directory we want to write to exists.
mkdir -p $TEST_OUTDIR

if [[ "$RUN_TEST" == "all" ]]; then
  for ((i = 0; i < ${#FT_TESTS[@]}; i++))
  do
    args=(${FT_TESTS[$i]})
    ${PREVIOUS_PWD}/CI/ft-test-font.sh ${GIT_HASH} ${FT_TESTS[$i]}
    if [ ! -z "$1" ]; then
     ${PREVIOUS_PWD}/CI/ft-report.sh ${args[0]} ${args[1]} ${1} ${2}\
      &> ${TEST_OUTDIR}/ft-$(basename ${args[0]})-${args[1]}-report.html
     result=$?
     if [ "$result" -eq "0" ];
     then
       RESULT_STR="PASS"
       PASS+="ft-$(basename ${args[0]})-${args[1]}-report.html "
     else
       RESULT_STR="FAIL"
       FAIL+="ft-$(basename ${args[0]})-${args[1]}-report.html "
       # We store any failure to use later in the exit command.
       EXIT=1
     fi
     echo "ft-$(basename ${args[0]})-${args[1]}-report.html [$RESULT_STR]"
    fi
  done
else
  args=(${FT_TESTS[$RUN_TEST]})
  ${PREVIOUS_PWD}/CI/ft-test-font.sh ${GIT_HASH} ${FT_TESTS[$RUN_TEST]}
  if [ ! -z "$1" ]; then
   ${PREVIOUS_PWD}/CI/ft-report.sh ${args[0]} ${args[1]} ${1} ${2}\
    &> ${TEST_OUTDIR}/ft-$(basename ${args[0]})-${args[1]}-report.html
   result=$?
   if [ "$result" -eq "0" ];
   then
     RESULT_STR="PASS"
     PASS+="ft-$(basename ${args[0]})-${args[1]}-report.html "
   else
     RESULT_STR="FAIL"
     FAIL+="ft-$(basename ${args[0]})-${args[1]}-report.html "
     # We store any failure to use later in the exit command.
     EXIT=1
   fi
   echo "ft-$(basename ${args[0]})-${args[1]}-report.html [$RESULT_STR]"
  fi
fi

# Below we generate an index of all reports generated
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
<h2>Freetype2 Difference Reports Index</h2>
<table>" &> "${TEST_OUTDIR}/index.html"

echo "<tr><th class=\"fail\">FAIL</th></tr>" &>> "${TEST_OUTDIR}/index.html"

for f in $FAIL
do
  echo "
  <tr>
    <td><a href=\"$f\">$(basename $f .html)</a></td>
  </tr>" &>> "${TEST_OUTDIR}/index.html"
done

echo "<tr><th class=\"pass\">PASS</th></tr>" &>> "${TEST_OUTDIR}/index.html"

for f in $PASS
do
  echo "
  <tr>
        <td><a href=\"$f\">$(basename $f .html)</a></td>
  </tr>" &>> "${TEST_OUTDIR}/index.html"
done

echo "
</table>
</body>
</html>" &>> "${TEST_OUTDIR}/index.html"

exit $EXIT
