function change() {
	var dpi = document.getElementById('dpi').value;
	var font = document.getElementById('font').value;
	var mode = document.getElementById('mode').value;
	var size = document.getElementById('size').value;
	var frame = document.getElementById('frame_1');
    var string = "pages/"+dpi+"/"+font+"/"+mode+"/"+size+"/index.html";
    frame.src = string;
}

var people, asc1 = 1,asc2 = 1,asc3 = 1;

function sort_t(tbody, col, asc){
    var rows = tbody.rows, rlen = rows.length, arr = new Array(), i, j, cells, clen;
    // fill the array with values from the table
    for(i = 0; i < rlen; i++){
    cells = rows[i].cells;
    clen = cells.length;
    arr[i] = new Array();
        for(j = 0; j < clen; j++){
        arr[i][j] = cells[j].innerHTML;
        }
    }
    // sort the array by the specified column number (col) and order (asc)
    arr.sort(function(a, b){
        return (a[col] == b[col]) ? 0 : ((a[col] > b[col]) ? asc : -1*asc);
    });
    for(i = 0; i < rlen; i++){
        arr[i] = "<td>"+arr[i].join("</td><td>")+"</timaged>";
    }
    tbody.innerHTML = "<tr>"+arr.join("</tr><tr>")+"</tr>";
}

function frame_2_source(image){
    var path = "url("+image.src+")";

    var fr_2 = parent.frame_2.document;

    var div = fr_2.getElementById('animation');
    div.style.backgroundImage=path;
}

function set_dim() {

    var imageSrc = document
                    .getElementById('animation')
                     .style
                      .backgroundImage
                       .replace(/url\((['"])?(.*?)\1\)/gi, '$2')
                        .split(',')[0];

    var image = new Image();
    image.src = imageSrc;

    var src_w = image.width;
    var src_h = image.height;

    var win_w = window.innerWidth;
    var win_h = window.innerHeight-60;

    var r_w = (win_w/(src_w/4)).toString();
    r_w = parseInt(r_w);
    
    var r_h = (win_h/src_h).toString();
    r_h = parseInt(r_h);

    var div_w = 0;
    var div_h = 0;

    if (r_w > r_h)
    { 
        div_w = src_w * r_h;
        div_h = src_h * r_h;
    } else {
        div_w = src_w * r_w;
        div_h = src_h * r_w;
    }

    document.getElementById('animation').style.width= div_w/4 + "px";

    document.getElementById('animation').style.height= div_h + "px";
}