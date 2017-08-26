rm -rf ./html/pages
rm -f ./html/top.html
#####################################################################
FT_TEST_DPI=${FT_TEST_DPI:-72 96};
FT_TEST_FONT_FILE=${FT_TEST_FONT_FILE:-test.ttf};
FT_TEST_RENDER_MODE=${FT_TEST_RENDER_MODE:-AA RGB};
FT_TEST_PT_SIZE=${FT_TEST_PT_SIZE:-16 20};

FT_TEST_BASE_DIR=${FT_TEST_BASE_DIR:-$HOME/base};
FT_TEST_TEST_DIR=${FT_TEST_TEST_DIR:-../..};

FT_TEST_BASE_DLL=${FT_TEST_BASE_DLL:-$FT_TEST_BASE_DIR/objs/.libs/libfreetype.so};
FT_TEST_TEST_DLL=$FT_TEST_TEST_DIR/objs/.libs/libfreetype.so
#####################################################################
declare -A arr
arr["MONO"]=0
arr+=(["AA"]=1 ["RGB"]=2 ["BGR"]=3 ["VRGB"]=4 ["VBGR"]=5)
#####################################################################
mkdir ./html/pages
touch ./html/top.html;
#####################################################################
echo '
<!DOCTYPE html>
  <head>
    <script type="text/javascript" src ="scripts/jquery-3.2.1.js"></script>
    <script type="text/javascript" src ="scripts/jquery.animateSprite.js"></script>
    <script type="text/javascript" src ="scripts/top.js" ></script>
    <link rel="stylesheet" type="text/css" href="styles/top.css">
  </head>
  <html>
    <body onload="change()">
      <iframe id="frame_1" name="frame_1" src="" ></iframe>
      <iframe id="frame_2" name="frame_2" src="diff.html" ></iframe>
      <div class="select">
    '>./html/top.html;
#####################################################################
for i in $FT_TEST_DPI; do
  mkdir ./html/pages/$i
  for j in $FT_TEST_FONT_FILE; do
    mkdir ./html/pages/$i/$j
    for k in $FT_TEST_RENDER_MODE; do
      mkdir ./html/pages/$i/$j/${arr[$k]}
      for l in $FT_TEST_PT_SIZE; do
        mkdir ./html/pages/$i/$j/${arr[$k]}/$l
        mkdir ./html/pages/$i/$j/${arr[$k]}/$l/images
        ./tests $FT_TEST_BASE_DLL $FT_TEST_TEST_DLL $j $l ${arr[$k]} $i
      done
    done
  done
done
#####################################################################
echo '<label>DPI&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp:<select name="dpi" id="dpi" onchange="change()">'>>./html/top.html;
for i in $FT_TEST_DPI; do
  echo "  <option value= $i > $i </option>">>./html/top.html;
done
echo '</select>
    </label><br>'>>./html/top.html;
#####################################################################
echo '<label>Font&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp:<select name="font" id="font" onchange="change()">'>>./html/top.html;
for i in $FT_TEST_FONT_FILE; do
  echo "  <option value= $i > $i </option>">>./html/top.html;
done
echo '</select>
    </label><br>'>>./html/top.html;
#####################################################################
echo '<label>Render Mode:<select name="mode" id="mode" onchange="change()">'>>./html/top.html;
for i in $FT_TEST_RENDER_MODE; do
  echo "  <option value= ${arr[$i]} > $i </option>">>./html/top.html;
done
echo '</select>
    </label><br>'>>./html/top.html;
#####################################################################
echo '<label>Size&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp:<select name="size" id="size" onchange="change()">'>>./html/top.html;
for i in $FT_TEST_PT_SIZE; do
  echo "  <option value= $i > $i </option>">>./html/top.html;
done
echo '</select>
    </label><br>'>>./html/top.html;
#####################################################################
echo '</div>
    </body>
  </html>'>>./html/top.html;
#####################################################################
echo "Font       : " $FT_TEST_FONT_FILE
echo "Size       : " $FT_TEST_PT_SIZE
echo "Render_Mode: " $FT_TEST_RENDER_MODE
echo "DPI        : " $FT_TEST_DPI




