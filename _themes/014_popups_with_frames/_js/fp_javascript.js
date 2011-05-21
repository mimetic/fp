$(document).ready( function() {

	// Capture right click
	//$("#selector").rightClick( function(el) {
		// Do something
	//});
	
	// Capture right mouse down
	//$("#selector").rightMouseDown( function(el) {
		// Do something
	//});
	
	// Capture right mouseup
	//$("#selector").rightMouseUp( function(el) {
		// Do something
	//});
	
	// Disable context menu on popup big picture
	//$("#popup_img").noContext();

	// Disable context menu everywhere in the main CSS Container in the page
	$("#container").noContext();
	// Disable context menu for the big popup picture
	$("#popup_img").noContext();		
	// show obfuscated email addresses which have class=obfuscate
	$(".obfuscated").defuscate();

	// Dropdown info boxes, e.g. limited edition and size info in a gallery
	$(".showtext").bind('mouseover', function (event) {
		if (!event) event = window.event;
		var target = (event.target) ? event.target : event.srcElement;
		myID = '#' + this.id + "_text";
		$(myID).slideDown('fast');
		
	});
	
	$(".showtext").bind('mouseout', function (event) {
		if (!event) event = window.event;
		var target = (event.target) ? event.target : event.srcElement;
		myID = this.id + "_text";
		$('#'+myID).slideUp('fast');		
	});
	
});

// ----------------
// FP JAVASCRIPTS

var ptpServer = "www.pictopia.com";
//providerID = {PrintSalesID};	//42 is David Gross's number!

// Navigation vars
var current_pic	=	0;




// Get scrolling offsets
function getScrollXY() {
  var scrOfX = 0, scrOfY = 0;
  if( typeof( window.pageYOffset ) == 'number' ) {
    //Netscape compliant
    scrOfY = window.pageYOffset;
    scrOfX = window.pageXOffset;
  } else if( document.body && ( document.body.scrollLeft || document.body.scrollTop ) ) {
    //DOM compliant
    scrOfY = document.body.scrollTop;
    scrOfX = document.body.scrollLeft;
  } else if( document.documentElement && ( document.documentElement.scrollLeft || document.documentElement.scrollTop ) ) {
    //IE6 standards compliant mode
    scrOfY = document.documentElement.scrollTop;
    scrOfX = document.documentElement.scrollLeft;
  }
  return [ scrOfX, scrOfY ];
}


// Mail Obfuscated Me...we have to hide the purpose of this function
// Used by the PHP "mailMe" function
function mom(sDom, sUser){
  return("mail"+"to:"+sUser+"@"+sDom.replace(/%23/g,"."));
}

// e is the encoded mail, s is the count between characters
// to extract the string. 
// Example: string is "abc"
// 			encoded is "axxbxxc", where s=3
function mymobfu (e, s)
{
	var m="";
	for (k=0;k<=e.length;k=k+s) {
		m = m + e.charAt(k);
	}
	return m;
}



function createCookie(name,value,days) {
	if (days) {
		var date = new Date();
		date.setTime(date.getTime()+(days*24*60*60*1000));
		var expires = "; expires="+date.toGMTString();
	}
	else var expires = "";
	document.cookie = name+"="+value+expires+"; path=/";
	
	//alert ("Cookie" + readCookie('fp_spec'));
}

function readCookie(name) {
	var nameEQ = name + "=";
	var ca = document.cookie.split(';');
	for(var i=0;i < ca.length;i++) {
		var c = ca[i];
		while (c.charAt(0)==' ') c = c.substring(1,c.length);
		if (c.indexOf(nameEQ) == 0) return c.substring(nameEQ.length,c.length);
	}
	return null;
}

function eraseCookie(name) {
	createCookie(name,"",-1);
}


function goPtp (name, ppset, pps, providerID) {
	
	if (pps == undefined) {
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

function newImage(arg) {
	if (document.images) {
		rslt = new Image();
		rslt.src = arg;
		return rslt;
	}
}

function changeImages() {
	if (document.images && (preloadFlag == true)) {
		for (var i=0; i<changeImages.arguments.length; i+=2) {
			document[changeImages.arguments[i]].src = changeImages.arguments[i+1];
		}
	}
}

var preloadFlag = false;
function pop_me_up (pURL,features,winW,winH) {
	var noPx = document.childNodes ? 'px' : 0;

	if (!winW)
		winW = 900;
	if (!winH)
		winH = screen.availHeight;

	BrowserName=navigator.appName;
	BrowserVersion=parseInt(navigator.appVersion);
	if(BrowserName=="Netscape"){
		winW = winW-8;
		winH = winH-25;
	}	

	cmd = "menubar=no,scrollbars=yes,status=yes,width=" + winW + ",height=" + winH;
	new_window=window.open(pURL,"displayWindow", cmd );
	new_window.focus();
	
	cX = Math.round ((screen.availWidth/2)  - (winW/2));
	cY = Math.round ((screen.availHeight/2) - (winH/2));
	new_window.moveTo(cX+noPx,cY+noPx);
}


// global variable to capture the currently popped up image
var CurrentPopupID ;

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

// THIS DOES NOT WORK BECAUSE THE PRICELISTS ARE 
// IN THE FADED LAYER, BELOW THE POPUP.
// Popup a pricelist (whereID, whichpricelistID)
function popupPriceList (whereID) {
	var noPx = document.childNodes ? 'px' : 0;
	
	// CurrentPopupID is a global
	pricelistID = "big_popup_pricelist_" + CurrentPopupID;
	setLyr (document.getElementById(whereID), pricelistID);
	changeOpac(100, "pricelistID");
}

// Normally, this just sets an object to "position:fixed"
// However, for IE6, we have to do something else.
function FixObjectPosition (id) {
	if ( navigator.userAgent.indexOf('MSIE 6') >= 0 )
	{	$('#'+id).css({
			position:	'absolute',
			top:		'0px',
			left:		'0px'
		});
	} 
	else
	{
		$('#'+id).css('position', 'fixed');
	}
		
}

// Popup an image in the gallery
// Can be used elsewhere, however: the 'id' is a prefix for id's,
// e.g. id=cowdung and <div id=cowdung_img>
// I'm using id="popup" for gallery pix popups
// Fade background and show a picture 
// id= the ID of the popup DIV on the masterpage
// imageID is the database ID of the image
// This version depends on hidden fields for values
function popUpImage (id, imageID, noFade) {

	var noPx = document.childNodes ? 'px' : 0;
	var minWindowTop = 10;
	
	// Read settings from hidden fields
	pURL = document.getElementById("pURL_"+imageID).value;
	framepath = document.getElementById("FRAMEPATH_"+imageID).value;
	framestyle = document.getElementById("FRAMESTYLE_"+imageID).value;
	imgW = document.getElementById("ImgW_"+imageID).value * 1;	// Convert to Number
	imgH = document.getElementById("ImgH_"+imageID).value * 1;	// Convert to Number
	picW = document.getElementById("SlideW_"+imageID).value * 1;	// Convert to Number
	picH = document.getElementById("SlideH_"+imageID).value * 1;	// Convert to Number
	frameW = document.getElementById("PROJECTFRAMEWIDTH_"+imageID).value * 1;	// Convert to Number
	darkness = document.getElementById("GALLERYFADEDARKNESS_"+imageID).value * 1;	// Convert to Number
	fullname = document.getElementById("Fullname_"+imageID).value;
	title = document.getElementById("Title_"+imageID).value;
	year = document.getElementById("YEAR_"+imageID).value;
	yeardiv = document.getElementById("YEAR_DIVIDER_"+imageID).value;
	month = document.getElementById("MONTH_"+imageID).value;
	monthdiv = document.getElementById("MONTH_DIVIDER_"+imageID).value;
	prevID = document.getElementById("PrevLinkID_"+imageID).value;
	nextID = document.getElementById("NextLinkID_"+imageID).value;

	myText = document.getElementById("PopupMsg_"+imageID).innerHTML;
	buybutton = document.getElementById("buy_"+imageID).innerHTML;

	if (!darkness)
		darkness = 0;
	if (!myText)
		myText = "Click to hide";
	
	// Write the prepared caption html text (created mostly by function FetchCascade) to the popup
	// Don't move...needed to determine final popup size, for readjustment to the window, below.
	$('#'+id+"_text").html(myText);
	$('#'+id+"_buy").html(buybutton);


	//q= pURL + "\n" + framepath + "\n" + framestyle + "\n" + imgW + "\n" + imgH + "\n" + picW + "\n" + picH + "\n" + frameW + "\n" + darkness + "\n" + myText + "\n" + prevID + "\n" + nextID + "\n";
	//alert (q);
		
	CurrentPopupID = imageID;

	// id of picture object in the popup window 
	picid = id + "_img";
	picid_coverup = id + "_img_coverup";
	bkgd_coverup = id + "_bkgd_coverup";

	// Adjust height of the slide to a percentage of the height of the screen
	myWindow = new WindowDimensions();
	slideAdjustment = document.getElementById("SlideAdjustment").value;
	slideMaxW = document.getElementById("SlideMaxW").value;
	slideMaxH = document.getElementById("SlideMaxH").value;
	r = ((myWindow.height * slideAdjustment) /  imgH);
	imgWUnadjusted = imgW;
	imgHUnadjusted = imgH;

	imgW = Math.floor (imgW * r);
	imgH = Math.floor (myWindow.height * slideAdjustment);
	// if we go beyond the pixel h/w of the image,
	// revert to max pixel settings, otherwise
	// also resize the picH/W settings (of picture
	// without frame).
	if ((imgH > slideMaxH) || (imgW > slideMaxW)) {
		imgH = imgHUnadjusted;
		imgW = imgWUnadjusted;
	} else {
		picW = Math.floor (picW * r);
		picH = Math.floor (picH * r);
	}

	// Get window dimensions (NOT screen dimensions)
	// Position window in center of browser window (not screen)
	t = Math.floor ((myWindow.height/2.5) - (imgH/2));
	l = Math.floor ((myWindow.width/2)  - (imgW/2));
	if (t <= 0)
		t=minWindowTop;
	
	FixObjectPosition (id);
	$('#'+id).css({top:t,left:l});
	//$('#'+id).css({top:t + noPx,left:l + noPx});

	// Size of frame + picture element
	document.getElementById(id).style.width = (frameW + imgW + frameW) + noPx;

	$('#'+picid).width(picW).height(picH).attr('src',pURL);

	// set coverup transparent block to prevent theft of the picture
	// picW,H could be smaller then imgW,H to create a matte effect
	FixObjectPosition (picid_coverup);
	$('#'+picid_coverup).width(picW).height(picH).css({width:frameW + imgW + frameW,height:frameW + imgH + frameW,top:t,left:l});

	// Frame
	$('#pf1').css({width:frameW,height:frameW}).attr('src', framepath+framestyle+"_tl.jpg");
	$('#pf2').css({width:imgW,height:frameW}).attr('src', framepath+framestyle+"_t.jpg");
	$('#pf3').css({width:frameW,height:frameW}).attr('src', framepath+framestyle+"_tr.jpg");

	$('#pf4').css({width:frameW,height:imgH}).attr('src', framepath+framestyle+"_l.jpg");
	$('#pf6').css({width:frameW,height:imgH}).attr('src', framepath+framestyle+"_r.jpg");

	$('#pf7').css({width:frameW,height:frameW}).attr('src', framepath+framestyle+"_bl.jpg");
	$('#pf8').css({width:imgW,height:frameW}).attr('src', framepath+framestyle+"_b.jpg");
	$('#pf9').css({width:frameW,height:frameW}).attr('src', framepath+framestyle+"_br.jpg");


	// Set global vars
	current_pic = imageID;

	// show/hide prev/next buttons
	prevID ? $('#'+id+"_prev").css('visibility','visible'):  $('#'+id+"_prev").css('visibility','hidden');
	nextID ? $('#'+id+"_next").css('visibility','visible'):  $('#'+id+"_next").css('visibility','hidden');

	// Dim the background
	if (!noFade)
		$('#container').fadeTo(300, darkness);
	
	// Cover up the whole page behind to prevent inadvertent clicks that
	// make strange things happen.
	// Must used the FixObjectPosition function to deal with IE6
	FixObjectPosition (bkgd_coverup);
	$('#'+bkgd_coverup).css({width:screen.width + noPx,height:screen.height + noPx,top:'0',left:'0'}).show();

	$('#'+picid_coverup).show();
	$('#'+id).fadeIn(400);
}


function unPopUpImage (id, darkness) {
	if (!darkness)
		darkness = 0;
		
	$('#container').fadeTo(400, 1.0);
 	$('#'+id).fadeOut(400);
 	$('#'+id+'_bkgd_coverup').hide();
	$('#'+id+'_img_coverup').hide();
// 	$('#'+id+'_prev').hide();
// 	$('#'+id+'_next').hide();
		
	//opacity('container', darkness, 100, 250)
// 	document.getElementById(id + "_img_coverup").style.visibility = "hidden";
// 	document.getElementById(id + "_bkgd_coverup").style.visibility = "hidden";
// 	document.getElementById(id).style.visibility = "hidden";
// 	document.getElementById(id+"_prev").style.visibility = "hidden";
// 	document.getElementById(id+"_next").style.visibility = "hidden";
}



// UNUSED
// Show/Hide popup text in the <div id=popup_text></div>
function displayPopupInfoX (id, viz) {
	if (!id)
		return false;
	viz == "show" ? document.getElementById(id+"_text").style.visibility = "visible" :  document.getElementById(id+"_text").style.visibility = "hidden";

}


// Show/Hide popup text in the <div id=popup_text></div>
function displayPopupInfo (id, viz) {
	if (!id)
		return false;
	viz == "show" ? document.getElementById(id+"_text").style.display = "block" :  document.getElementById(id+"_text").style.display = "none";
}

// Show/Hide object id=id
function showhide (id, disableflag) {
	if (!id || disableflag == true)
		return false;
	$('#'+id).toggle();
}



// Goes prev/next image in the gallery
// Uses global var current_pic
// Looks up prev/next from hidden fields
function changePopupImage (direction) {

	if (!current_pic)
		return false;
	
	if (direction == "prev") {
		imageID = document.getElementById("PrevLinkID_"+current_pic).value;
	} else {
		imageID = document.getElementById("NextLinkID_"+current_pic).value;
	}		
	popUpImage ("popup",imageID, true);	// true means don't fade/unfade
	
}


function popUpStoryCSS (id, myTitle, myByline, myText, myWidth, frameW, darkness) {

	var noPx = document.childNodes ? 'px' : 0;
	var minWindowTop = 50;

	if (!darkness)
		darkness = 0;
	if (!myText)
		myText = "Click to hide";
	
	// Get window dimensions (NOT screen dimensions)
	// Position window in center of browser window (not screen)
	myWindow = new WindowDimensions();
	l = Math.round ((myWindow.width/2)  - (myWidth/2));
	t=minWindowTop;
	
	document.getElementById(id).style.position = "absolute";
	document.getElementById(id).style.top = t + noPx;
	document.getElementById(id).style.left = l + noPx;
	
	// Size of frame + picture element
	document.getElementById(id).style.width = (frameW + myWidth + frameW) + noPx;

	
	// Write the message to the popup
	if (myText)
		$('#'+id+"_title").html(myTitle);
	
	// Write the message to the popup
	if (myByline)
		$('#'+id+"_byline").html(myByline);
	
	// Write the message to the popup
	if (myText)
		$('#'+id+"_text").html(myText);
	
	// Dim the background
	$('div#container').fadeTo (100, darkness);
	$('#'+id).fadeIn('normal');
	//document.getElementById(id).style.visibility = "visible";
}

function ChangePic (f) {
	i = document.picmgmtform.PictureList.selectedIndex
	pic = document.picmgmtform.PictureList.options[i].value
	picpath = pic;
	document.Picture.src = picpath;
	document.picmgmtform.PictureName.value = pic;
}

function ChooseIconPic (f) {
	i = document.myform.PictureList.selectedIndex
	pic = document.myform.PictureList.options[i].value
	picpath = pic;
	document.Picture.src = picpath;
	document.myform.PictureName.value = pic;
	document.myform.Icon.value = pic;
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
// 	var ch = xHeight('cc');
// 	if (ch < h) {
// 		xHeight('content', h);
// 		xHeight('cc', h);
// 	}
// Make all external links open in a new window
	if (!document.getElementsByTagName) return;
	var anchors = document.getElementsByTagName("a");
	for (var i=0;  i < anchors.length;  i++) {
		var anchor = anchors[i];
		if (anchor.getAttribute("href") && anchor.getAttribute("rel") == "ext") 
			anchor.target = "_blank";
	}
}

// if ((screen.width<=1024) || (screen.height<=768)) {
// 	window.location.href = "small.html";
// }

// myFlag lets us turn this off w/o removing the code
function center(myFlag){
	window.focus();
	BrowserName=navigator.appName;
	BrowserVersion=parseInt(navigator.appVersion);
	if (myFlag) {
		if(BrowserName=="Netscape"){
			self.moveTo(0,0)
			self.resizeTo((screen.availWidth-8),(screen.availHeight-25))
		}else{
			self.moveTo(0,0)
			self.resizeTo((screen.availWidth),(screen.availHeight))
		}
	}
}

// FADING SCRIPTS

function opacity(id, opacStart, opacEnd, millisec) {
	//speed for each frame
	var speed = Math.round(millisec / 100);
	var timer = 0;

	//determine the direction for the blending, if start and end are the same nothing happens
	if(opacStart > opacEnd) {
		for(i = opacStart; i >= opacEnd; i--) {
			setTimeout("changeOpac(" + i + ",'" + id + "')",(timer * speed));
			timer++;
		}
	} else if(opacStart < opacEnd) {
		for(i = opacStart; i <= opacEnd; i++)
			{
			setTimeout("changeOpac(" + i + ",'" + id + "')",(timer * speed));
			timer++;
		}
	}
}

//change the opacity for different browsers
function changeOpac(opacity, id) {
	var object = document.getElementById(id).style; 
	object.opacity = (opacity / 100);
	object.MozOpacity = (opacity / 100);
	object.KhtmlOpacity = (opacity / 100);
	object.filter = "alpha(opacity=" + opacity + ")";
}

function shiftOpacity(id, millisec) {
	//if an element is invisible, make it visible, else make it ivisible
	if(document.getElementById(id).style.opacity == 0) {
		opacity(id, 0, 100, millisec);
	} else {
		opacity(id, 100, 0, millisec);
	}
}

function blendimage(divid, imageid, imagefile, millisec) {
	var speed = Math.round(millisec / 100);
	var timer = 0;
	
	//set the current image as background
	document.getElementById(divid).style.backgroundImage = "url(" + document.getElementById(imageid).src + ")";
	
	//make image transparent
	changeOpac(0, imageid);
	
	//make new image
	document.getElementById(imageid).src = imagefile;

	//fade in image
	for(i = 0; i <= 100; i++) {
		setTimeout("changeOpac(" + i + ",'" + imageid + "')",(timer * speed));
		timer++;
	}
}

function currentOpac(id, opacEnd, millisec) {
	//standard opacity is 100
	var currentOpac = 100;
	
	//if the element has an opacity set, get it
	if(document.getElementById(id).style.opacity < 100) {
		currentOpac = document.getElementById(id).style.opacity * 100;
	}

	//call for the function that changes the opacity
	opacity(id, currentOpac, opacEnd, millisec)
}

function WindowDimensions () {
  var myWidth = 0, myHeight = 0;
  if( typeof( window.innerWidth ) == 'number' ) {
    //Non-IE
    myWidth = window.innerWidth;
    myHeight = window.innerHeight;
  } else if( document.documentElement && ( document.documentElement.clientWidth || document.documentElement.clientHeight ) ) {
    //IE 6+ in 'standards compliant mode'
    myWidth = document.documentElement.clientWidth;
    myHeight = document.documentElement.clientHeight;
  } else if( document.body && ( document.body.clientWidth || document.body.clientHeight ) ) {
    //IE 4 compatible
    myWidth = document.body.clientWidth;
    myHeight = document.body.clientHeight;
  }
  this.width = myWidth;
  this.height = myHeight;
}

function URLDecode( encoded )
{
   // Replace + with ' '
   // Replace %xx with equivalent character
   // Put [ERROR] in output if %xx is invalid.
   var HEXCHARS = "0123456789ABCDEFabcdef"; 
   var plaintext = "";
   var i = 0;
   while (i < encoded.length) {
	   var ch = encoded.charAt(i);
	   if (ch == "+") {
		   plaintext += " ";
		   i++;
	   } else if (ch == "%") {
			if (i < (encoded.length-2) 
					&& HEXCHARS.indexOf(encoded.charAt(i+1)) != -1 
					&& HEXCHARS.indexOf(encoded.charAt(i+2)) != -1 ) {
				plaintext += unescape( encoded.substr(i,3) );
				i += 3;
			} else {
				alert( 'fp_javascript.js : URLDecode : Bad escape combination near ...' + encoded.substr(i) );
				plaintext += "%[ERROR]";
				i++;
			}
		} else {
		   plaintext += ch;
		   i++;
		}
	} // while
   return plaintext;
}


// FrameShop

function UpdatePrices (myCurrency, currencyPosition) {
	var unitPrice, totalPrice;

	framed = document.getElementById("framemattepricelist").value.split(",");
	unframed = document.getElementById("printpricelist").value.split(",");
	
	size = document.getElementById("currentsize").value;
	frame = document.getElementById("currentframe").value;
	matte = document.getElementById("currentmatte").value;
	quantity = document.getElementById("currentquantity").value;
	shipping = document.getElementById("currentshipping").value;

	extrashippinglist = document.getElementById("extrashipping").value.split(",");

	weights = document.getElementById("weightlist").value.split(",");
	weightsframed = document.getElementById("weightsframedlist").value.split(",");
	
	if (frame > 0) {
		unitPrice = framed[size];
		weight = weightsframed[size];
	} else {
		unitPrice = unframed[size];
		weight = weights[size];
	}
	
	if (shipping > 0) {
		extraShipping = 1 * extrashippinglist[size]; 	// 1*  forces a type change to numeric
	} else {
		extraShipping = 0;
	}

	totalPrice = (quantity * unitPrice) + (1 * extraShipping);
	document.getElementById("amount").value = totalPrice;
	document.getElementById("weight").value = weight;

	unitPrice = formatCurrency (unitPrice, myCurrency, currencyPosition);
	totalPrice = formatCurrency (totalPrice, myCurrency, currencyPosition);
	extraShipping = formatCurrency (extraShipping, myCurrency, currencyPosition);
	
	//alert (totalPrice + " : " + unitPrice);
	
	totalPrice = totalPrice + " " + FP_CURRENCY_NAME;
	
	
	$('#unit_price').html(addCommas(unitPrice));
	$('#total_price').html(addCommas(totalPrice));
	$('#extra_shipping').html(addCommas(extraShipping));
	
}

// Update the encoded spec's for the sale, e.g. size, frame, paper, etc.
// Put in 'spec' field and 'custom' field
function UpdateSpec ()
{
	var system_units, system_matte_width;
	
	system_units = document.getElementById("system_units").value;
	system_matte_width = document.getElementById("system_matte_width").value;

	size = document.getElementById("currentsize").value;
	frame = document.getElementById("currentframe").value;
	matte = document.getElementById("currentmatte").value;
	paper = document.getElementById("currentpaper").value;
	inkset = document.getElementById("currentinkset").value;
	glazing = document.getElementById("currentglazing").value;
	artistID = document.getElementById("currentartistID").value;
	filename = document.getElementById("currentfilename").value;
	imageID = document.getElementById("currentimageID").value;
	quantity = document.getElementById("currentquantity").value;
	mymatchprint = document.getElementById("matchprint").value;
	supplierID = document.getElementById("supplierid").value;

	shipping = document.getElementById("currentshipping").value;
	extrashippinglist = document.getElementById("extrashipping").value.split(",");
	if (shipping > 0) {
		extraShipping = 1 * extrashippinglist[size]; 	// 1*  forces a type change to numeric
	} else {
		extraShipping = 0;
	}
	
	sizes = document.getElementById("maxdimslist").value.split(",");
	maxside = sizes[size];

	framecodes = document.getElementById("framecodeslist").value.split(",");
	framecode = framecodes[frame];

	mattecodes = document.getElementById("mattecodeslist").value.split(",");
	mattecode = mattecodes[matte];

	glazingcodes = document.getElementById("glazingcodeslist").value.split(",");
	glazingcode = glazingcodes[glazing];

	papercodes = document.getElementById("papercodeslist").value.split(",");
	papercode = papercodes[paper];

	inksetcodes = document.getElementById("inksetcodeslist").value.split(",");
	inksetcode = inksetcodes[inkset];

	rows = document.getElementById("rowslist").value.split(",");

	// row in the pricelist record, e.g. editionsize1, 2, 3,
	row = rows[size];
	
	sizes = document.getElementById("dimslist").value.split(",");
	dims = sizes[size].split("-");

	mattewidth = Math.round (system_matte_width * dims[1] *10 ) / 10;

//	spec = "ar=" + artistID + "&pw=" + dims[0] + "&ph=" + dims[1] + "&fr=" + frame + "&mt=" + matte;
//	spec = spec + "&fn=" + filename + "&id=" + imageID;
//	FP_ORDER_SIZE: ro = the index of the size selection, used to know which size was chosen

	// these globals must be set in the main code
	// this is done by inserting the frameshop_codes snippet in a Javascript block in the main page code
	// so the ReplaceSystemVariables can insert the system values for it.
	
	//THIS CAN ONLY BE 200 CHARACTERS AFTER THE URL ENCODE, SO BE CAREFUL ABOUT CODE LENGTHS!
	
	spec = FP_ORDER_ROW + "=" + row;
	spec = spec + "&" + FP_QUANTITY + "=" + quantity;
	spec = spec + "&" + FP_ORDER_SIZE + "=" + maxside;
	spec = spec + "&" + FP_ORDER_ARTISTID + "=" + artistID;
	spec = spec + "&" + FP_ORDER_FRAMECODE + "=" + framecode;
	spec = spec + "&" + FP_ORDER_MATTECODE + "=" + mattecode;
	spec = spec + "&" + FP_ORDER_MATTEWIDTH + "=" + mattewidth;
	spec = spec + "&" + FP_ORDER_SYSTEM_UNITS + "=" + system_units;
	spec = spec + "&" + FP_ORDER_PAPERCODE + "=" + papercode;
	spec = spec + "&" + FP_ORDER_INKSETCODE + "=" + inksetcode;
	spec = spec + "&" + FP_ORDER_GLAZINGCODE + "=" + glazingcode;
	spec = spec + "&" + FP_ORDER_FILENAME + "=" + filename;
	spec = spec + "&" + FP_ORDER_IMAGEID + "=" + imageID;
	spec = spec + "&" + FP_ORDER_PRINTWIDTH + "=" + dims[0];
	spec = spec + "&" + FP_ORDER_PRINTHEIGHT + "=" + dims[1];
	spec = spec + "&" + FP_ORDER_MATCHPRINT + "=" + mymatchprint;
	spec = spec + "&" + FP_ORDER_SUPPLIER_ID + "=" + supplierID;
	spec = spec + "&" + FP_ORDER_EXTRA_SHIPPING + "=" + extraShipping;

	//spec = Url.encode(spec);	// URL encode
	spec.length > 200 && alert ("Warning: Please tell the webmaster that 'the spec code is too long for Paypal'. There will be a problem with this order. We apologize and will fix this problem as soon as we can.");
	
	document.getElementById("spec").value = spec;
	//document.getElementById("spec64").value = encode64 (spec);	// testing
	
}

function UpdateDesc ()
{
	var system_units, system_matte_width;
	
	system_units = document.getElementById("system_units").value;
	system_matte_width = document.getElementById("system_matte_width").value;

	size = document.getElementById("currentsize").value;
	frame = document.getElementById("currentframe").value;
	matte = document.getElementById("currentmatte").value;
	glazing = document.getElementById("currentglazing").value;
	paper = document.getElementById("currentpaper").value;
	inkset = document.getElementById("currentinkset").value;
	mymatchprint = document.getElementById("matchprint").value;
	if (mymatchprint > 0) {
		mymatchprint = "matchprint required";
	} else{
		mymatchprint = "no matchprint";
	}

	myartistname = document.getElementById("artistname").value;
	imageID = document.getElementById("currentimageID").value;
	image_catalog_num = document.getElementById("image_cat_num").value;
	myimagename = document.getElementById("imagename").value;

	curshipping = document.getElementById("currentshipping").value;
	
	mattenames = document.getElementById("mattenameslist").value.split(",");
	mattecolor = mattenames[matte];

	framenames = document.getElementById("framenameslist").value.split(",");
	framename = framenames[frame];
	if (frame == 0) {
		framename = "unframed";
	} else {
		framename += " frame";
	}

	sizes = document.getElementById("dimslist").value.split(",");
	sizename = sizes[size].replace("-","x");
	
	glazings = document.getElementById("glazinglist").value.split(",");
	glazingname = glazings[glazing];

	papers = document.getElementById("paperslist").value.split(",");
	papername = papers[paper];
	
	inksets = document.getElementById("inksetslist").value.split(",");
	inksetname = inksets[inkset];

	sizes = document.getElementById("dimslist").value.split(",");
	dims = sizes[size].split("-");

	mattewidth = Math.round (system_matte_width * dims[1] *10 ) / 10;

	quantity = document.getElementById("currentquantity").value;
	pword = "print";
	if (quantity > 1)
		pword = pword + "s";
	
	shipping = "";
	if (curshipping > 0) 
		shipping = ", Int'l Shipping";
	
	desc = "Catalog #" + image_catalog_num + ", " + quantity + " " + pword + ", " + sizename + " " + system_units + ", " + framename + ", " + mattecolor + " matte"  + " (" + mattewidth + " " + system_units + " wide), " + papername + " paper, " + inksetname + " inkset, " + glazingname + "" + shipping;

	
	//desc = "Artwork by " + myartistname + " (#" + imageID + ":" + myimagename + "), " + quantity + " " + pword + ", " + sizename + " " + system_units + " print size, " + framename + ", " + mattecolor + " matte"  + " (" + mattewidth + " " + system_units + " wide), " + papername + " paper, " + inksetname + " inkset, " + glazingname + " glazing, " + mymatchprint;
	
	desc.length > 198 && alert ("Warning: the description is too long ("+desc.length+" chars, only 200 chars will show)\n" + desc);
	
	document.getElementById("desc").value = desc;
	
}


function formatCurrency (amount, myCurrency, currencyPosition) {
	if (currencyPosition == "before") {
		s = myCurrency + amount;
	} else {
		s = amount + myCurrency;
	}
	return s;
}


// Frameshop:
// params: pURL=picture local path, frames path, framestyle (a number), full framed picture w/h, 
// inset picture w/h, thickness of the frame
function UpdatePicture (pURL,framepath,imgW,imgH,referencesize) {
	var noPx = document.childNodes ? 'px' : 0;

	id = "frameshop_picture";
	picid = "fs5";
	wholeObj = "fs_picture";
	matteObj = "fs_pic_matte";
	picBoxObj = "fs_pic";
	matteTopObj = "fs_matte_top";
	matteBottomObj = "fs_matte_bottom";
	
	size = document.getElementById("currentsize").value;
	frame = document.getElementById("currentframe").value;
	matte = document.getElementById("currentmatte").value;

	framed = document.getElementById("framemattepricelist").value.split(",");
	unframed = document.getElementById("printpricelist").value.split(",");
	
	mattes = document.getElementById("matteslist").value.split(",");
	mattecolor = mattes[matte];

	framewidths = document.getElementById("framewidthslist").value.split(",");
	frameW = framewidths[frame];

	maxdims = document.getElementById("maxdimslist").value.split(",");
	//maxprintsize = document.getElementById("maxprintsizelist").value;
	maxprintsize = referencesize;
	
	
	// Set scaling to simulate real sizes
	scalefactor = (maxdims[size] / maxprintsize);
	// However, if FRAMESHOP_PICTURE_SCALING is zero, don't scale the picture
	if (document.getElementById("FRAMESHOP_PICTURE_SCALING").value == 0)
		scalefactor = 1;
	imgW = Math.floor(imgW * scalefactor);
	imgH = Math.floor(imgH * scalefactor);
// 	picW = Math.floor(picW * scalefactor);
// 	picH = Math.floor(picH * scalefactor);


	//Matte settings
	fp_matte_scale = document.getElementById("system_matte_scale").value;
	fp_matte_bottom = document.getElementById("system_matte_bottom").value;

	// If no matte and no frame, show an art border by resetting the matte settings!
	// If the system art border setting is less than 2, it must be a percentage, not a pixel amount.
	// Note, if we use a FIXED width, we have the funky
	// calculation, which is really the reverse of the calculate used
	// by the matting function
	if  ( (1 * matte) + ( 1 * frame) == 0 ) {
		var system_artborder_width = document.getElementById("system_artborder_width").value;
		if (system_artborder_width < 2) {
			// use artborder_width as a percentage of the greater side
			if (imgW > imgH) {
				var artborder_width = system_artborder_width * imgW;
			} else {
				var artborder_width = system_artborder_width * imgH;
			}
		} else {
			// use artborder_width as a fixed pixel amount
			var artborder_width = system_artborder_width;
		}
		fp_matte_scale = 1-((2*artborder_width)/imgW);
		fp_matte_bottom = 0;
		mattecolor = document.getElementById("system_artborder_color").value;
		matte = true;
	}

	if (matte > 0) {
		arr = GetMatteDimensions (imgW, imgH, fp_matte_scale, fp_matte_bottom);
		picW = arr[0];
		picH = arr[1];
		imgW = arr[2];
		imgH = arr[3];
		mattetop = arr[4];
		mattebottom = arr[5];
		//alert (arr);
	} else {
		picW = imgW;
		picH = imgH;
		mattetop = 0;
		mattebottom = 0;
	}



	// size of frame + matte + picture element
	document.getElementById(wholeObj).style.width = (2 * frameW + imgW) + noPx;
	
	document.getElementById(wholeObj).style.height = (2 * frameW + imgH) + noPx;

	// size of matte + picture element
	document.getElementById(matteObj).style.width = imgW + noPx;
	document.getElementById(matteObj).style.height = imgH + noPx;
	
	document.getElementById(matteTopObj).style.width = imgW + noPx;
	document.getElementById(matteTopObj).style.height = mattetop + noPx;
	document.getElementById(matteBottomObj).style.width = imgW + noPx;
	document.getElementById(matteBottomObj).style.height = mattebottom + noPx;

 	//Set the picture src URL
 	document.getElementById(picid).src = pURL;
 	 	
	if (matte != 0) {
		// Set picture size inside frame
		// picW,H could be smaller then imgW,H to create a matte effect
		document.getElementById(picid).style.width = picW+noPx;
		document.getElementById(picid).style.height = picH+noPx;
		// Set matte color
		document.getElementById(matteObj).style.backgroundColor = mattecolor;
		document.getElementById(matteTopObj).style.backgroundColor = mattecolor;
		document.getElementById(matteBottomObj).style.backgroundColor = mattecolor;
	} else {
		document.getElementById(picid).style.width = imgW+noPx;
		document.getElementById(picid).style.height = imgH+noPx;
		// matte is zero
		document.getElementById(matteTopObj).style.height = "0" + noPx;
		document.getElementById(matteTopObj).style.width = "0" + noPx;
		document.getElementById(matteBottomObj).style.height = "0" + noPx;
		document.getElementById(matteBottomObj).style.width = "0" + noPx;
		// hide the matte
		document.getElementById(matteTopObj).style.visibility = "hidden";
		document.getElementById(matteBottomObj).style.visibility = "hidden";
	}


	
	// Frame style
	if (frame != 0) {
		document.getElementById("fs1").src = framepath+frame+"_tl.jpg";
		document.getElementById("fs2").src = framepath+frame+"_t.jpg";
		document.getElementById("fs3").src = framepath+frame+"_tr.jpg";
		document.getElementById("fs4").src = framepath+frame+"_l.jpg";
		document.getElementById("fs6").src = framepath+frame+"_r.jpg";
		document.getElementById("fs7").src = framepath+frame+"_bl.jpg";
		document.getElementById("fs8").src = framepath+frame+"_b.jpg";
		document.getElementById("fs9").src = framepath+frame+"_br.jpg";
	}

	// Frame size
	document.getElementById("fs1").style.width = frameW+noPx;
	document.getElementById("fs1").style.height = frameW+noPx;

	document.getElementById("fs2").style.width = imgW+noPx;
	document.getElementById("fs2").style.height = frameW+noPx;

	document.getElementById("fs3").style.width = frameW+noPx;
	document.getElementById("fs3").style.height = frameW+noPx;

	document.getElementById("fs4").style.width = frameW+noPx;
	document.getElementById("fs4").style.height = imgH+noPx;

	document.getElementById("fs6").style.width = frameW+noPx;
	document.getElementById("fs6").style.height = imgH+noPx;


	document.getElementById("fs7").style.width = frameW+noPx;
	document.getElementById("fs7").style.height = frameW+noPx;

	document.getElementById("fs8").style.width = imgW+noPx;
	document.getElementById("fs8").style.height = frameW+noPx;

	document.getElementById("fs9").style.width = frameW+noPx;
	document.getElementById("fs9").style.height = frameW+noPx;

	
	// WALL COLOR
	changeWallColor ();

}

// Frameshop:
function GetMatteDimensions (width, height, fp_matte_scale, fp_matte_bottom) {
	mattewidth = Math.ceil((width/2) * (1 - fp_matte_scale));

	new_interior_width =  Math.ceil (width * fp_matte_scale);
	new_interior_height = Math.ceil (height * fp_matte_scale);

	mattewidth = Math.ceil ((width / 2) * (1 - fp_matte_scale));

	new_height = new_interior_height + (2 * mattewidth);		

	bottommatte = new_height - new_interior_height - mattewidth;

	var arr = new Array();
	arr[0] = new_interior_width;
	arr[1] = new_interior_height;
	arr[2] = width;
	arr[3] = new_height;
	arr[4] = mattewidth;
	arr[5] = bottommatte;
	
	return arr;
}


// This code was written by Tyler Akins and has been placed in the
// public domain.  It would be nice if you left this header intact.
// Base64 code from Tyler Akins -- http://rumkin.com

var keyStr = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

function encode64(input) {
   var output = "";
   var chr1, chr2, chr3;
   var enc1, enc2, enc3, enc4;
   var i = 0;

   do {
      chr1 = input.charCodeAt(i++);
      chr2 = input.charCodeAt(i++);
      chr3 = input.charCodeAt(i++);

      enc1 = chr1 >> 2;
      enc2 = ((chr1 & 3) << 4) | (chr2 >> 4);
      enc3 = ((chr2 & 15) << 2) | (chr3 >> 6);
      enc4 = chr3 & 63;

      if (isNaN(chr2)) {
         enc3 = enc4 = 64;
      } else if (isNaN(chr3)) {
         enc4 = 64;
      }

      output = output + keyStr.charAt(enc1) + keyStr.charAt(enc2) + 
         keyStr.charAt(enc3) + keyStr.charAt(enc4);
   } while (i < input.length);
   
   return output;
}

function decode64(input) {
   var output = "";
   var chr1, chr2, chr3;
   var enc1, enc2, enc3, enc4;
   var i = 0;

   // remove all characters that are not A-Z, a-z, 0-9, +, /, or =
   input = input.replace(/[^A-Za-z0-9\+\/\=]/g, "");

   do {
      enc1 = keyStr.indexOf(input.charAt(i++));
      enc2 = keyStr.indexOf(input.charAt(i++));
      enc3 = keyStr.indexOf(input.charAt(i++));
      enc4 = keyStr.indexOf(input.charAt(i++));

      chr1 = (enc1 << 2) | (enc2 >> 4);
      chr2 = ((enc2 & 15) << 4) | (enc3 >> 2);
      chr3 = ((enc3 & 3) << 6) | enc4;

      output = output + String.fromCharCode(chr1);

      if (enc3 != 64) {
         output = output + String.fromCharCode(chr2);
      }
      if (enc4 != 64) {
         output = output + String.fromCharCode(chr3);
      }
   } while (i < input.length);

   return output;
}

function addCommas(nStr)
{
	nStr += '';
	x = nStr.split('.');
	x1 = x[0];
	x2 = x.length > 1 ? '.' + x[1] : '';
	var rgx = /(\d+)(\d{3})/;
	while (rgx.test(x1)) {
		x1 = x1.replace(rgx, '$1' + ',' + '$2');
	}
	return x1 + x2;
}

/**
*
* URL encode / decode
* http://www.webtoolkit.info/
*
**/

var Url = {

	// public method for url encoding
	encode : function (string) {
		return escape(this._utf8_encode(string));
	},

	// public method for url decoding
	decode : function (string) {
		return this._utf8_decode(unescape(string));
	},

	// private method for UTF-8 encoding
	_utf8_encode : function (string) {
		string = string.replace(/\r\n/g,"\n");
		var utftext = "";

		for (var n = 0; n < string.length; n++) {

			var c = string.charCodeAt(n);

			if (c < 128) {
				utftext += String.fromCharCode(c);
			}
			else if((c > 127) && (c < 2048)) {
				utftext += String.fromCharCode((c >> 6) | 192);
				utftext += String.fromCharCode((c & 63) | 128);
			}
			else {
				utftext += String.fromCharCode((c >> 12) | 224);
				utftext += String.fromCharCode(((c >> 6) & 63) | 128);
				utftext += String.fromCharCode((c & 63) | 128);
			}

		}

		return utftext;
	},

	// private method for UTF-8 decoding
	_utf8_decode : function (utftext) {
		var string = "";
		var i = 0;
		var c = c1 = c2 = 0;

		while ( i < utftext.length ) {

			c = utftext.charCodeAt(i);

			if (c < 128) {
				string += String.fromCharCode(c);
				i++;
			}
			else if((c > 191) && (c < 224)) {
				c2 = utftext.charCodeAt(i+1);
				string += String.fromCharCode(((c & 31) << 6) | (c2 & 63));
				i += 2;
			}
			else {
				c2 = utftext.charCodeAt(i+1);
				c3 = utftext.charCodeAt(i+2);
				string += String.fromCharCode(((c & 15) << 12) | ((c2 & 63) << 6) | (c3 & 63));
				i += 3;
			}

		}

		return string;
	}

}


function changeWallColor () {
	// WALL COLOR
	mywallcolor = document.getElementById("wallcolor").value;
	document.getElementById("fs_picture_container").style.backgroundColor = mywallcolor;
	//document.getElementById("fs_frameshop").style.backgroundColor = wallcolor;
}

function changeTearsheetColor () {
	// WALL COLOR
	wallcolor = document.getElementById("wallcolor").value;
	document.getElementById("container_tearsheet").style.backgroundColor = wallcolor;
//	document.body.style.backgroundColor = wallcolor;
}

// UNUSED.
// It seems I can't count on this happening before the order is sent to Paypal.

// Pass the description and spec to the orders db table
// If order is confirmed, we have all the data. This
// way we don't have to pass all data to PayPal
// This is my first AJAX!
//
// Pass the ID of the order plus the time (just for uniqueness) in the "custom"
// variable through Paypal.

function StorePreOrder()
{
	var xmlHttp;
	try
		{
		// Firefox, Opera 8.0+, Safari
		xmlHttp=new XMLHttpRequest();
		}
	catch (e)
		{
		// Internet Explorer
		try
			{
			xmlHttp=new ActiveXObject("Msxml2.XMLHTTP");
			}
		catch (e)
			{
			try
				{
				xmlHttp=new ActiveXObject("Microsoft.XMLHTTP");
				}
			catch (e)
				{
				alert("Your browser does not support AJAX!");
				return false;
				}
			}
		}
	
	xmlHttp.onreadystatechange = function()
								{
								if(xmlHttp.readyState==4)
									{
									document.getElementById("item_number").value = xmlHttp.responseText ;
									}
								}
	item_number = document.getElementById("currentimageID").value;
	item_number = item_number;
	spec = document.getElementById("spec").value;
	spec = Url.encode(spec);
	desc = document.getElementById("desc").value;
	desc = Url.encode(desc);
	xmlHttp.open("GET","writepreorder.php?item_number=" + item_number + "&spec=" + spec + "&desc=" + desc,true);
	xmlHttp.send(null);
}



// Resize image to fit screen
function ResizeToFitScreen (picid, imgW, imgH, vPad) {
	var noPx = document.childNodes ? 'px' : 0;
	
	if (!vPad)
		vPad = 0;
	myWindow = new WindowDimensions();
	slideMaxW = imgW;
	slideMaxH = imgH;
	imgWUnadjusted = imgW;
	imgHUnadjusted = imgH;

	imgH = Math.floor (myWindow.height - vPad);
	imgW = Math.floor (imgWUnadjusted * (imgH / imgHUnadjusted) );
	
	// if we go beyond the pixel h/w of the image,
	// revert to max pixel settings, otherwise
	if ((imgH > slideMaxH) || (imgW > slideMaxW)) {
		imgH = imgHUnadjusted;
		imgW = imgWUnadjusted;
	}
	// Set picture size inside frame
	// picW,H could be smaller then imgW,H to create a matte effect
	document.getElementById(picid).style.width = imgW+noPx;
	document.getElementById(picid).style.height = imgH+noPx;
	
}

