/*
	Admin Javascript Library
	name: fp_javascript_lib.js
*/






// Pixels to Units
// Convert pixels to system units, e.g. pixels -> inches
// rounded to 1/100 of a unit.
function ConvertPixelsToUnits (pix) {
	u = Math.round(100*(pix / FP_PRINTDPI))/100;
	return u;
}

// Convert system units to pixels, e.g. inches -> pixels
function ConvertUnitsToPixels (u) {
	pix = Math.ceil(u * FP_PRINTDPI);
	return pix;
}


// UpdateRecordViaAjax (table,id,values)
// Save a values to a table
// Values should be an object, {field : value,...}
function UpdateRecordViaAjax (table,id,values) {
	var cmd = "update_record";
	var data = {
		'cmd'			: cmd,
		'id'			: id,
		'table'			: table,
		'values'		: values
	};
	var dataString = JSON.stringify(data);
	
	$.post('ajax_admin.php',  {data: dataString}, function(res) {
		result = JSON.parse(res);
		//alert (result);
	}, "text");
}



// Given an id from a select menu, show the associated .png preview picture
// Different previews for group (gallery) vs. project (exhibition)
function ShowThemePreview (id) {
	if (id == "0")
		id = ($('#themeid-for-preview').val());

	var url = FP_THEME_PREVIEW_PATHS_OBJ['_'+id.replace(':','__')];

	if (url) {
		$('#theme-preview-box').html("<img src=\""+url+"\">");

	} else {
		$('#theme-preview-box').html("(No Preview)");
	}
	//$('#test').val(p+url);
}


// ----------------------------------------------------------------
// Multieditor (metadata editor) functions

function ProcessRows (table) {
	switch (table)
	{
	case 'Images' :
		var actions = GetCommands('Images');
		var rows = GetListOfRows();
		break;
	}
	
	if (!actions.length) {
		//alert ("No actions chosen.");
		return false;
	}
	
	if (!rows.length) {
		alert ("You did not choose any items to modify.");
	 	return false;
	}
	
	var cmd = "processmultiedit";
	
	var data = {
		'cmd'			: cmd,
		'table'			: table,
		'rows'			: rows,
		'actions'		: actions
	};
	
	var dataString = JSON.stringify(data);
	
	$.post('ajax_admin.php', {data: dataString}, function(res) {
		res = JSON.parse(res);
		//alert (result);
		// reload web page to reflect changes
		if (res == "reload") {
			window.location.reload(true);
		}

	}, "text");

}


/*
Get Commands from the image multieditor dialog.
Start with all checkboxes with class "me_command".
For each, get the value of the checkbox.
	- the value is the id of the field with the parameters for the action
	- the id of the checkbox is the name of the action
	- Multiple parameters are coded, id="myparam", then id="myparam_2", etc.
*/
function GetCommands () {
	var id, command, param;
	var actions = [];
	
	$('.me_command:checked').each(function() {
		// get ID of the input with param values from the value
		// If the value is empty, then the checkbox IS the param
		// Check for other fields with names, ID+"2", ID+"3", etc. to find more params
		id = $(this).val();
		if (!id)
			id = $(this).attr('id');
		param = $('#'+id).val();
		// check for more params
		if (param) {
			for (i=2;i<=5;i++) {
				p2 = $('#'+id+"_"+i).val();
				if (p2)
					param = param + "," + p2;
			}
		}
		command = [id,param];
		actions.push(command);
	});
	return actions;
}


function GetListOfRows () {
	var rows = [];
	$('input:checked[id^=multiedit]').each(function() {
		rows.push($(this).val());
	});
	return rows;
}



// ----------------------------------------------------------------
// Certificate of Authenticity (COA) Scripts


// Check for tables of class "remove-if-empty", and remove rows which contain
// an empty cell.
function RemoveEmptyRows () {
	$('table.remove-if-empty tr').each(function() {
		tr = this;
		$(this).children('td').each(function() {
			c = StripTagsFromText($.trim($(this).html()));
			if (c == '') {
				$(tr).hide();
			}
		});
	});
}


function StripTagsFromText(t) {
	return t.replace(/<\/?[^>]+>/gi, '');
}

// ----------------------------------------------------------------
// Price Set scripts
// ----------------------------------------------------------------

function DeletePriceSetRowx (k) {
	$('button[name^=DeletePriceSetRow]').click(function() 
	{
		k = $(this).attr('title');
		if (confirm("Delete this entry: image size "+k+"?"))
			{
			$('#size-{k}').val(0);
			$(this).parent().submit();
			}
	});
}
	


// Get an object which contains all pricing data for an image
// This calls a PHP script and gets the object via JSON
function GetSamplePricingData (index) {

	var cmd = "getsamplepricingonesizeforjs";
	
	var data = {
		'cmd'			: cmd,
		'imageID'		: $('#imageid').val(),
		'supplierID'		: $('#SupplierID').val(),
		'pricesetID'		: $('#pricesetid').val(),
		'index'			: index
	};
	
	var dataString = JSON.stringify(data);
	
	$.post('ajax_admin.php', {data: dataString}, function(res) {
		//alert(dataString);
		result = JSON.parse(res);
		//alert (result);
	}, "text");

}

// print cost
function CalcPrintCost (i) {
	if (false && !FP_ROW_PRICING_DATA[0]) {
		GetSamplePricingData (true);			// the price loader calls UpdatePrices when it's done.
		return false;
	}
	
	for (var n in FP_ROW_PRICING_DATA) {
		num = formatCurrency (FP_ROW_PRICING_DATA[n], FP_CURRENCY, FP_CURRENCY_POSITION);
// 		num = Math.round(num*100)/100;
// 		num = num.toFixed(2).toString();
		$('#show'+n+i).html(num);
	}
	
	
	

}

// Validate Price Set Entry
function ValidatePriceSetEntry () {
	err = "";
	ok = true;
	// Weights entered?
	$('input[name^=Size]').each(function() {
		i = $(this).attr('title');
		s = parseInt($(this).val());

		if (parseInt(s) > 0)
			{
			ok = ok && ($('#PrintShipWeight'+i).val() > 0) 
				&& ($('#MatteShipWeight'+i).val() > 0)
				&& ($('#FrameToPrintShipWeight'+i).val() > 0)
				&& ($('#FrameMatteShipWeight'+i).val() > 0);
				
			if (!ok)
				{
				if (isNaN(i))
					err = err + "The new entry in Print Pricing for size "+s+" is missing weight information.\n ";
				else
					err = err + "Print Pricing entry #"+i+" is missing weight information.\n ";
				}
			if (err)
				alert (err);
			}
	});
		
	return ok;
}



// Calc sum of edition size fields to show the total edition size
function CalcTotalEditionSize (src, target) {
	var t = 0;
	$('input[name^='+src+']').each(function() {
		t = 1*$(this).val() + t;
		});
	$('#'+target).html(t);
}

function ShowAllPrintFrameSizes (src) {
	if (!src)
		src = "Size";
	$('input[name^='+src+']').each(function() {
		k = $(this).attr('title');
		if (parseInt(k) > 0) {
			var targetD = "FrameSize" + k;
			var targetT = "FrameSizeText" + k;
			ShowPrintFrameSizes(this,targetD, targetT);
		}
	});
}

// Calculate and show sizes for image, paper, and matte/frame for a given 
// The target element is "framesize"+the 
function ShowPrintFrameSizes (src, targetD, targetT) {
	if (!targetD)
		targetD = "FrameSize" + $(src).attr('title');

	if (!targetT)
		targetT = "FrameSizeText" + $(src).attr('title');
		
	var w = $(src).val();
	if (w) {
		var h = (2/3)*w;
	} else {
		return false;
	}
	w = parseFloat(w);
	h = parseFloat(h);
	
	var dims = GetAllDimensions (w,h);
	var hw = w.toFixed(2) + " &times; " + h.toFixed(2) + ' ' + FP_UNITS;
	var phw = dims.pw.toFixed(2) + " &times; " + dims.ph.toFixed(2) + ' ' + FP_UNITS;
	var hwm = dims.w.toFixed(2) + " &times; " + dims.h.toFixed(2) + ' ' + FP_UNITS;
	
	
	// create diagram
	// 100px wide
	var text = "";
	var diagram = "";
	var r = 200/dims.w;
	
	//outer
	var fullw = Math.floor(r * dims.w);
	var fullh = Math.floor(r * dims.h);
	
	// matte
	var pd = Math.floor(r * dims.matteBorder);
	var pbd = Math.floor(r * dims.matteWidthBottom);
	
	// inner
	var wd = fullw-(2*pd);
	var hd = fullh-(pd + pbd);
	
	fullw = fullw + "px";
	fullh= fullh+ "px";
	pd = pd + "px";
	pbd = pbd + "px";
	wd = wd + "px";
	hd = hd + "px";

	
//	text = text + '<div class="helptext">';
	var text = text + '<div>';
	text = text + "<i>For a 3:2 image (35mm film shape):</i><br>";	
	text = text + 'Image: '+hw+'<br>Paper: '+phw+'<br>Matte/Frame: '+hwm + '<br><br>';

	diagram = diagram + '<div align="center">' + '' + hwm + '' + '<br><div style="height:'+fullh+';width:'+fullw+';background-color:white;border:5px solid #333;"><div style="height:'+hd+';width:'+wd+';background-color:#CDD;margin:'+pd+' '+pd+' '+pbd+' '+pd+';">'+'' + hw + ''+'</div></div></div><br>';


	//alert (fs);
	diagram = diagram + '</div>';
	
	$('#'+targetD).html(diagram);
	$('#'+targetT).html(text);
}

// Given a fixed image size, calculate the matte dimensions
// Assume width > height!!!
// FPMatteRatio is the percentage of the whole that is matte, 
// e.g. 80% image + 20% matte, measured along largest side
// FPMatteBottomRatio is an additional amount of matte to add to the bottom.
// The art border is raised to the inch.
// The constants (FPMatteRatio, etc.) are set in the system_vars snippet.
function GetAllDimensions (width, height, fp_matte_scale, fp_matte_bottom, fp_artborder) {
	if (!fp_matte_scale)
		fp_matte_scale = FPMatteRatio;
	if (!fp_matte_bottom)
		fp_matte_bottom = FPMatteBottomRatio;
	if (!fp_artborder)
		fp_artborder = FPartborder;

	if (width < height) {
		var temp = width;
		width = height;
		height = temp;
	}
	width = parseFloat(width);
	height = parseFloat(height);
	
	var matteBorder = height * fp_matte_scale;
	matteBorder = Math.round( matteBorder * 4)/4;
	var matteWidthBottom = (height * FPMatteBottomRatio) + matteBorder;
	matteWidthBottom = Math.round( matteWidthBottom * 4)/4;
	
	var matteWidth = width + (2 * matteBorder);
	var matteHeight = height + matteBorder + matteWidthBottom;
	
	var artborderWidth = ArtBorder(width);
	
	var nWidth = width + (2*matteBorder);
	var nHeight = height + matteBorder + matteWidthBottom;
	
	var pw = RoundTo100(2*artborderWidth+width);
	var ph = RoundTo100(2*artborderWidth+height);
	
	return {w:nWidth, h:parseFloat(nHeight), matteBorder:matteBorder,matteWidthBottom:matteWidthBottom, pw:pw, ph:ph, artborderWidth:artborderWidth};
}

// Calculate the art border. It is a percentage of the greater side, but always between 1" and 3", 
// depending on the size of the image, and it jumps in 1" increments.
// FPartborder is a global.
function ArtBorder (width) {
	var abw = Math.ceil(FPartborder * width);
	if (abw > 3)
		abw = 3;
	return abw;
}

function RoundTo100 (x) {
	var x = Math.round(x * 100)/100;
	return x;
}

function FormatAsFloat(obj) {
	num = $(obj).val();
	if (isNaN(num))
		num = 0;
	num = Math.round(num*100)/100;
	num = num.toFixed(2).toString();
	$(obj).val(num);
}

function formatCurrency (amount, myCurrency, currencyPosition) {
	amount = parseFloat(amount);
	amount = amount.toFixed(2);
	if (currencyPosition == "before") {
		s = myCurrency + amount;
	} else {
		s = amount + myCurrency;
	}
	return s;
}

// ----------------------------------------------------------------
	
// Show class=advanced topics and hide class=unadvanced
function ShowAdvanced () {
	$(function() {
		$(".advanced").show();
		$(".unadvanced").hide();
		$(".advanced:hidden").prev().find('.toggler').text('-');
		$(".advanced:visible").prev().find('.toggler').text('+');
	});
}

// show/hide <div class="advanced">
function ToggleAdvanced () {
	$(function() {
		$(".advanced").toggle();
		$(".unadvanced").toggle();
		$(".advanced:hidden").prev().find('.toggler').text('-');
		$(".advanced:visible").prev().find('.toggler').text('+');
	});
}

// show/hide previews
function showpreviews (myIDprefix) {
	$('*[id^='+myIDprefix+']').css('display', '');
}

// show hide previews
function hidepreviews (myIDprefix) {
	$('*[id^='+myIDprefix+']').css('display', 'none');
}


// Show all preview images, id begins with "preview", the src is in the title
// Note, src is empty so we don't load unnecessarily
function showAllPreviews (pid) {
	var url,id,info,projectid,title;
	$('*[id^=preview]').each(function(n) {
		id = this.title;
		info = $('#info'+id).val().split(',');
		url = info[0];
		projectid = info[1];
		title = info[2];
		this.src = url;
		if (pid && (projectid == pid)) {
			$(this).show();
		} else if (!pid) {
			$(this).show();
		}
	});
}

function hideAllPreviews () {
	$('*[id^=preview]').each(function(n) {
		$(this).hide();
	});
}


// --- my crappy functions ---


function newImage(arg) {
	if (document.images) {
		rslt = new Image();
		rslt.src = arg;
		return rslt;
	} else {
		return false;
	}
}

function changeImages() {
	if (document.images && (preloadFlag == true)) {
		for (var i=0; i<changeImages.arguments.length; i+=2) {
			document[changeImages.arguments[i]].src = changeImages.arguments[i+1];
		}
	}
}


function pop_me_up (pURL,features) {
	new_window=window.open(pURL,"displayWindow","menubar=no,scrollbars=yes,status=yes,width=800,height=700");		
	new_window.focus();
}


function ChangePic (f) {
	i = document.picmgmtform.PictureList.selectedIndex
	pic = document.picmgmtform.PictureList.options[i].value
	picpath = pic;
	document.Picture.src = picpath;
	document.picmgmtform.PictureName.value = pic;
}

// requires a select named "PictureList"
// Updates an IMG named PictureImg and
// a field named PictureName
function ChoosePic (PictureImg,PictureName,dir) {
	i = document.myform.PictureList.selectedIndex
	pic = document.myform.PictureList.options[i].value
	document.PictureImg.src = dir + "/" + pic;
	document.myform.PictureName.value = pic;
	document.myform.PictureName.value = pic;
}

function changeLocation(menuObj)
{
   var i = menuObj.selectedIndex;
   if(i > 0)
   {
      window.location = menuObj.options[i].value;
   }
}

function init(h) {
	var ch = xHeight('cc');
	if (ch < h) {
		xHeight('content', h);
		xHeight('cc', h);
	}
	if (!document.getElementsByTagName) return;
	var anchors = document.getElementsByTagName("a");
	for (var i=0;  i < anchors.length;  i++) {
		var anchor = anchors[i];
		if (anchor.getAttribute("href") && anchor.getAttribute("rel") == "ext") 
			anchor.target = "_blank";
	}
}

//Ordering of pictures 
//Original:  Roelof Bos (roelof667@hotmail.com) 
//Web Site:  http://www.refuse.nl 
function move(index,to) {
	var list = document.orderpixform.list;
	var list2 = document.orderpixform.list2;
	var total = list.options.length-1;
	if (index == -1) return false;
	if (to == +1 && index == total) return false;
	if (to == -1 && index == 0) return false;
	var items = new Array;
	var values = new Array;
	for (i = total; i >= 0; i--) {
		items[i] = list.options[i].text;
		values[i] = list.options[i].value;
	}
	var items2 = new Array;
	var values2 = new Array;
	for (i = total; i >= 0; i--) {
		items2[i] = list2.options[i].text;
		values2[i] = list2.options[i].value;
	}

	for (i = total; i >= 0; i--) {
		if (index == i) {
			list.options[i + to] = new Option(items[i],values[i + to], 0, 1);
			list.options[i] = new Option(items[i + to], values[i]);

			list2.options[i + to] = new Option(items2[i],values2[i + to], 0, 1);
			list2.options[i] = new Option(items2[i + to], values2[i]);

			i--;
			}
		else {
			list.options[i] = new Option(items[i], values[i]);
			list2.options[i] = new Option(items2[i], values2[i]);
		}
	}
	
	// Update the text version of the list
	var theList = "";
	for (i = 0; i <= list2.options.length-1; i++) { 
		theList += list2.options[i].text;
		if (i != list2.options.length-1) theList += ",";
	}
	document.orderpixform.neworder.value = theList;

	list.focus();
}


// Show/Hide popup image. Note that the image is set by this script, meaning the
// the <img> can have an empty src at first and we don't have to load the image.
// This is good, because we want to minimize bandwidth (assuming satellite phone!)
function popupImage (PosObj, id, viz, imagesrc) {
	if (!id || !imagesrc)
		return false;
	if (viz == "show") {
		setLyr (PosObj, id);
		document.getElementById(id).src = imagesrc;
		document.getElementById(id).style.display = "block";
	} else {
		document.getElementById(id).style.display = "none";
	}
}

// Same as popup, only instead of overlaying the picture,
// we make room on the page
function revealImage (PosObj, id, viz, imagesrc) {
	if (!id || !imagesrc)
		return false;
		
	if (document.getElementById(id).style.visibility != "visible") {
		document.getElementById(id).src = imagesrc;
		document.getElementById(id).style.display = "block";
		document.getElementById(id).style.visibility = "visible";		
	} else {
		document.getElementById(id).style.display = "none";
		document.getElementById(id).style.visibility = "hidden";
	}
}

function shrink (obj) {
	r = obj.height / obj.width;
	obj.width = obj.width / 2;
	obj.height = obj.height / 2;
}

function grow (obj) {
	r = obj.height / obj.width;
	obj.width = obj.width * 2;
	obj.height = obj.height * 2;
}

function popupPriceList (whereID) {
	var noPx = document.childNodes ? 'px' : 0;
	
	// CurrentPopupID is a global
	pricelistID = "big_popup_pricelist_" + CurrentPopupID;
	setLyr (document.getElementById(whereID), popUpID);
	changeOpac(100, "pricelistID");
}

function findPos(obj) {
	var curleft = curtop = 0;
	if (obj.offsetParent) {
		do {
			curleft += obj.offsetLeft;
			curtop += obj.offsetTop;
		} while (obj = obj.offsetParent);
	}
	return [curleft,curtop];
}


function setLyr(obj,lyr)
{
	var coors = findPos(obj);
	var x = document.getElementById(lyr);
	x.style.top = coors[1] + 'px';
	x.style.left = coors[0] + 'px';
}

//change the opacity for different browsers
function changeOpac(opacity, id) {
	var object = document.getElementById(id).style; 
	object.opacity = (opacity / 100);
	object.MozOpacity = (opacity / 100);
	object.KhtmlOpacity = (opacity / 100);
	object.filter = "alpha(opacity=" + opacity + ")";
}


// Used to popup images
function getRef(obj){
	return (typeof obj == "string") ?
		 document.getElementById(obj) : obj;
}

function setStyle(obj,style,value){
	getRef(obj).style[style]= value;
}

function showPreview (path) {
	getRef('previewimage').src = path;
	setStyle('previewbutton', 'display', 'none');
	setStyle('previewimage', 'display', 'block');
}

// Calculate per-slide time in seconds based on entered show length in sec.
// f=id of field with show length (input)
// o=id of field with single slide duration (output)
// k=number of slides in show
// d=>calculated per-slide time
// 0 means use built-in time, so 0 is allowed.
function updateSlideDuration (f,o,k,fpdefault) {
	showtime = document.getElementById(f).value;
	if ((showtime>0) && (showtime < k)) {
		//showtime = k;
	}
	// round to 10th of sec.
	d = Math.ceil(showtime*10/k)/10;
	showtime = Math.ceil(d * k);
	document.getElementById(o).value = d;
	if (d > 0) {
		showd = d;
	} else {
		showd = fpdefault;
	}
	//document.getElementById(o).value = showd;
	//document.getElementById(f).value = showtime;
	$(f).val(showtime);
	$(o).html(showd);
}

// Show idToShow if id is checked, else hide
function showOnChecked (id, idToShow) {
	if ($('#'+id).attr('checked') ) {
		$('#'+idToShow).fadeIn();
	} else {
		$('#'+idToShow).fadeOut();
	}
	/*
	if (document.getElementById(id).checked) {
		document.getElementById(idToShow).style.visibility = "visible";
	} else {
		document.getElementById(idToShow).style.visibility = "hidden";
	}
	*/
}

// Show idToShow if id is UNchecked, else hide
function hideOnChecked (id, idToShow) {
	if ($('#'+id).attr('checked') ) {
		$('#'+idToShow).fadeOut();
	} else {
		$('#'+idToShow).fadeIn();
	}
}


// ======= IMAGE SORTER FUNCTIONS ==========

// Show only pix in the archives from the chosen project PID.
// The objects to control have an attribute, 'pid', which must match
// the 'pid' passed to the function.
function HideImagesInOtherProjects (pid) {
/*
	if (pid != '') {
		$('*[pid]').not('*[pid=active]').not('*[pid='+pid+']').fadeOut();
		$('*[pid=active], *[pid='+pid+']').fadeIn();
	} else {
		$('*[pid]').fadeIn();
	}
*/

	if (parseInt(pid) > 0) {
		//$('.imagesorter-item').not('.pid-'+pid+', .pid-active').fadeOut(500).appendTo("#outlist-hidden");
		$('#outlist .imagesorter-item').not('.pid-'+pid+', .pid-active').fadeOut(500).appendTo("#outlist-hidden");
		$('#outlist-hidden .pid-'+pid).appendTo("#outlist").fadeIn(500);
		StartImageSorter();
	} else if (pid == "all") {
		$('#outlist-hidden .imagesorter-item').appendTo("#outlist");
		$('#outlist .imagesorter-item').fadeIn(500);
		StartImageSorter();
	}
	
}

function InitImageSorter () {

	$('button#imagesorter-save').click(function() {
		var cmd = "update_project_image_order";
		var order = $gallery.sortable('toArray');
		var projectID = $('#projectID').val();
		var data = {
			'cmd'			: cmd,
			'order'			: order,
			'id'				: projectID
		};
		var dataString = JSON.stringify(data);
		$.post('ajax_admin.php',  {data: dataString}, function(res) {
			result = JSON.parse(res);
			alert (result);
		}, "text");
	});

	$('button#imagesorter-revert').click(function() {
	 	window.location.reload();
	});
	
	// resolve the icons behavior with event delegation
	$('ul.gallery > li').click(function(ev) {
		var $item = $(this);
		var $target = $(ev.target);

		if ($target.is('a.ui-icon-trash')) {
			DeleteImage($item);
		} else if ($target.is('a.ui-icon-zoomin')) {
			viewLargerImage($target);
		} else if ($target.is('a.ui-icon-plus')) {
			addImage($item);
		} else if ($target.is('a.ui-icon-minus')) {
			removeImage($item);
		} else if ($target.is('a.ui-icon-info')) {
			//var i = $this.find('imagesorter-edit').title;
			//alert (i);
			
			//$('#'+i).dialog('open');
		}

		return false;
	});
}

function StartImageSorter () {
	if ($('#imagesorter').length > 0) {
		// there's the gallery and the trash
		$gallery = $('.imagesorter #inlist'), $archive = $('.imagesorter #outlist'), $trash = $('.imagesorter #trash');
		add_icon = '<a href="#" title="Add image" class="imagesorter-icon imagesorter-add ui-icon ui-icon-plus">Add image</a>';	
		remove_icon = '<a href="#" title="Remove image" class="imagesorter-icon imagesorter-remove ui-icon ui-icon-minus">Remove image</a>';

		//edit_icon = '<a href="#" title="imageQuickEditor{ID}" class="imageQEopener imagesorter-icon imagesorter-edit ui-icon ui-icon-info">Edit image</a>';
	
		$('.imagesorter li .ui-state-highlight').css('background-color','#F2f');
	
		// let the gallery items be sortable
		$('.imagesorter #inlist, .imagesorter #outlist').sortable({
			placeholder: 'ui-state-highlight',
			revert: 100,
			connectWith: '.connectedSortable',
			containment: $('#imagesorter'),
			receive : function(event, ui) {
				FixIcons(ui.sender, ui.item);
				},
			// We have to reset the highlight size...it doesn't happen automatically.
			start : function (event, ui) {
				w = $(ui.item).width();
				h = $(ui.item).height();
				$('.imagesorter .ui-state-highlight').width(w).height(h);
				}
				
		})
		.disableSelection();
		//alert ("StartImageSorter");
		// Add remove, add, and edit info icons
		$('.imagesorter #inlist > .imagesorter-item').append(remove_icon);//.append(edit_icon);
		$('.imagesorter #outlist > .imagesorter-item').append(add_icon);//.append(edit_icon);
			
		$(".imagesorter ul, .imagesorter li").disableSelection();
	}
}	
	

// ---- image sorter functions ---- //
// Remove from gallery
function removeImage($item) {
	$item.fadeOut(function() {
		$item.find('a.ui-icon-minus').remove();
		$item.append(add_icon).prependTo($archive).fadeIn(function() {
			//$item.animate({ width: '48px' }).find('img').animate({ height: '36px' });
		});
	});
}

// Add to gallery
function addImage($item) {
	$item.fadeOut(function() {
		$item.find('a.ui-icon-plus').remove();
		$item.append(remove_icon).prependTo($gallery).fadeIn(function() {
			//$item.animate({ width: '48px' }).find('img').animate({ height: '36px' });
		});
	});
}

function FixIcons(sender, item) {
	if (sender.attr('id') == 'inlist') {
		AddAddIcon(item);
	} else {
		AddRemoveIcon(item);
	}
}

function AddAddIcon(item) {
	item.find('a.ui-icon-minus').remove();
	item.append(add_icon);
}

function AddRemoveIcon(item) {
	item.find('a.ui-icon-plus').remove();
	item.append(remove_icon);
}

// Delete Image
function DeleteImage ($item) {
	var title = $item.attr('title');
	var id = $item.attr('id').replace('pic_','');

	if (confirm("Delete the image, \""+title+"\" ?\nThis will completely remove the image from the entire system, and you cannot get it back!"))
		{
		var cmd = "delete_image";
		var data = {
			'cmd'			: cmd,
			'id'				: id
		};
		var dataString = JSON.stringify(data);
		
		$.post('ajax_admin.php',  {data: dataString}, function(res) 
			{
				result = JSON.parse(res);
				// false means error, so report it
				if (result) {
					$item.remove();
				} else {
					alert (result);
				}
			}
			, "text");
	}
}
// ---- image sorter functions END ----- //



// ======= PICTOPIA CODE ==========


var ptpServer = "www.pictopia.com";
//providerID = {PrintSalesID};	//42 is David Gross's number!

function goPtp (name, ppset, pps, providerID) {
	
	if (pps === undefined) {
		pps="";
	}
	
	if (ppset == '') {
		ppset="default";
		pps = "";
	} else {
		ppset = ppset + "-";
	}
	
	win = window.open('http://' + ptpServer + '/perl/ptp?provider_id='
		+ providerID + '&photo_name=' + name  +
		'&pps=' + ppset + pps + '&time=' +
		new Date().getTime(),
		'ptp_' + providerID,
		'scrollbars=yes, resizable=yes, toolbar=no, '
		+ 'status=yes, width=670,height=700, menubar=no');
	win.focus();
}


