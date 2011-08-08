// ----------------
// FP JAVASCRIPTS


/*
// function resizeOneGalleryPicture
// Resize an object and the enclosed IMG in a gallery
// The original proportions of the image must be pre-stored by jQuery as an attribute of the image.
// It is the ratio of width to height, i.e. proportions = width / height.
// If not, floating point errors rapidly distort the shape of the image as we resize it.
// obj: object containing the image and the caption

	ui object in resizable:
	ui.originalPosition - {top, left} before resizing started
	ui.originalSize - {width, height} before resizing started
	ui.position - {top, left} current position
	ui.size - {width, height} current size
*/

function resizeOneGalleryPicture ( obj, ui, pictureclass, captionclass ) {
//, oWidth, oHeight, newWidth, newHeight, resizeHeight, pictureclass, captionclass) {
	var DEBUG = false;
	
	var oWidth = StripPx(ui.originalSize.width);
	var oHeight = StripPx(ui.originalSize.height);

	var newWidth = StripPx(ui.size.width);
	var newHeight = StripPx(ui.size.height);
	
	var resizeRatio = newWidth / oWidth;
	
	if ( typeof obj === 'undefined' || obj === null )
		return false;

	if ( typeof animate === 'undefined' || animate === null || isNaN(animate) )
		animate = false;
		
	var myspeed = "fast";
	
	// Get image object from inside of obj
	if (pictureclass) {
		var image = $(pictureclass);
	} else {
		var image = obj.find('img.gallerypic');
		debug(DEBUG,'resizeOneGalleryPicture: Using img.gallerypic');
	}
	
	// Get caption object
	if (captionclass) {
		var captionblock = $(captionclass);
	} else {
		var captionblock = obj.find('.gallerypicinfo');
		debug(DEBUG,'resizeOneGalleryPicture: Using .gallerypicinfo');
	}
	
	if ( !image || !image.width )
	{	// No image or no visible lightbox, so we don't care
		$.log('WARNING', 'A resize occured while no image or no lightbox...');
		return false;
	}
	
	// Calc new size of image in the object
	var imageOriginalWidth  = image.width();
	var imageOriginalHeight = image.height();
	debug(DEBUG,'resizeOneGalleryPicture:  image.width:' + imageOriginalWidth + ', image.height: '+oHeight);
	
	// Get proportions, or calculate them
	var proportions = image.attr('proportions');
	if (proportions == "NaN" || proportions == undefined) {
		proportions = imageOriginalWidth/imageOriginalHeight;
		image.attr('proportions', proportions );
		debug(DEBUG, 'Recalc Proportions');

	}
	debug(DEBUG, 'proportions: '+proportions);
	
	var newImageWidth = newWidth;
	var newImageHeight = newWidth / proportions;
	
	// Now, resize picture with matte to fit new size
	// scale to determine width of matte (padding)
	// and extra matte at the bottom
	var dims = GetNewPictureDimensions (imageOriginalWidth, imageOriginalHeight, newImageWidth, newImageHeight, proportions, 10000, 10000);

	msg = "iWidth:" + dims.iWidth +", iHeight:"+dims.iHeight +", nWidth:"+dims.nWidth +", nHeight:"+dims.nHeight +", MatteWidth:"+dims.matteWidth;
	debug(DEBUG, msg);
	
	msg = "resizeToWindow: Ratio: " + dims.iWidth / dims.iHeight;
	debug(DEBUG, msg);
	
	// RESIZING
	
	// image matting (padding)
	image.css('padding', dims.matteWidth+"px "+dims.matteWidth+"px "+dims.matteWidthBottom+"px "+dims.matteWidth+"px");
	// image size
	image.width(dims.iWidth).height(dims.iHeight);
	
	//obj.width(dims.nWidth);
	// Resizing height kills captions below the image
	//if (resizeHeight)
	//	obj.height(dims.nHeight);
	
	
	captionblock.css('padding', dims.matteWidth+"px "+dims.matteWidth+"px "+dims.matteWidthBottom+"px "+dims.matteWidth+"px").width(dims.iWidth).height(dims.iHeight);

	// Done
	debug(DEBUG, "---------- END -----------");
	return true;
}



// ----------------------------------------------------------------
// Validate a coupon code and get a discount rate via JSON
function ValidateCoupon () {

	var cmd = "validatecoupon";
	var data = {
		'cmd'			: cmd,
		'artistid'		: $('#currentartistID').val(),
		'couponcode'		: $('#couponcode').val()
	};
	
	if ($('#couponcode').val()) {	
		var dataString = JSON.stringify(data);	
		$.post('ajax_frameshop.php', {data: dataString}, function(res) {
			res = JSON.parse(res);
			//alert (res.value + ", error=" + res.error);
			discount = res.value;
			UpdateTotal (FP_CURRENCY, FP_CURRENCY_POSITION);
			$('#coupon_price_display_error').html(res.error);	// this will show the error msg
			$('#discount').val(discount).change();
			$('#coupon_description').val(res.description);
	
		}, "text");
	} else {
		$('#discount').val(0).change();
		$('#coupon_description').val("");
	}
}


// ----------------------------------------------------------------
// Get an object which contains all pricing data for an image
// This calls a PHP script and gets the object via JSON
function GetAllPricingData (callUpdate) {

	var cmd = "getcartpricingforjs";
	
	var data = {
		'cmd'			: cmd,
		'imageID'		: $('#imageid').val(),
		'supplierID'		: $('#supplierid').val(),
		'pricesetID'		: $('#pricesetid').val()
	};
	
	var dataString = JSON.stringify(data);
	
	$.post('ajax_frameshop.php', {data: dataString}, function(res) {
		//alert(res);
		FP_PRICING_DATA = JSON.parse(res);
		// don't calc shipping if we're not on a checkout page & we're using the shopping cart
		if (callUpdate)
			UpdateTotal (FP_CURRENCY, FP_CURRENCY_POSITION);
			//alert ('loaded');
	}, "text");
		
	return FP_PRICING_DATA;
}

// Dim button by ID
// Also disable click functionality!
function DimButton(myID) {
	$('#'+myID).addClass("fp_dim").fadeTo(100,0.1);
}

function UnDimButton(myID) {
	$('#'+myID).removeClass("fp_dim").fadeTo(100,1.0);
}


// Dim button by passing an object
function DimButtonObj(obj) {
	$(obj).addClass("fp_dim").fadeTo(100,0.1);
}

// Dim button by passing an object
function UnDimButtonObj(obj) {
	$(obj).removeClass("fp_dim").fadeTo(100,1.0);
}

function DimBuyButton() {
	$('#BuyButton').addClass("fp_dim").fadeTo(100,0.1);
}

function DimAddButton() {
	$('#AddButton').addClass("fp_dim").fadeTo(100,0.1);
}

function UnDimBuyButton() {
	$('#BuyButton').removeClass("fp_dim").fadeTo(100,1.0);
}

function UnDimAddButton() {
	$('#AddButton').removeClass("fp_dim").fadeTo(100,1.0);
}

// ----------------------------------------------------------------
// Go to the Frame/Print shop page to buy a print.
function BuyAPrint (imageID, projectID, currentframe, currentmatte, currentsize, linkonly ) {
	var url = "frameshop.php";
	if (!projectID)
		projectID = null;
		
	var params = 'projectID=' + projectID + '&ImageID=' + imageID + '&currentframe=' + currentframe + '&currentmatte=' + currentmatte + '&currentsize=' + currentsize ;

	// add return URL, possibly with current page index value
	var r = document.URL;
	var i = 0;
	
	try { i = FP_CURRENT_IMAGE_INDEX; }
	catch (err) { i = 0; }

	if (i)
		r = r + '&FP_CURRENT_IMAGE_INDEX=' + i;
		
	params = params + "&FP_PREVIOUS_URL=" + encodeURI(r);
	
	// add current page index value
	if (i)
		params = params + '&FP_CURRENT_IMAGE_INDEX=' + i;
		
	url = url + "?" + params;
	
	// Jump to frame/print shop unless linkonly is set.
	if (!linkonly) {
		document.location = url;
	}

	return url;
}

// Same as above, but more objecty...
function GoToPrintShop (obj) {
	var url = "frameshop.php";
	var params = 'ProjectID=' + obj.projectID + '&ImageID=' + obj.imageID + '&currentframe=' + obj.currentframe + '&currentmatte=' + obj.currentmatte + '&currentsize=' + obj.currentsize ;

	// add return URL, possibly with current page index value
	var r = document.URL;
	var i = 0;
	
	try { i = FP_CURRENT_IMAGE_INDEX; }
	catch (err) { i = 0; }

	if (i)
		r = r + '&FP_CURRENT_IMAGE_INDEX=' + i;
	params = params + "&FP_PREVIOUS_URL=" + encodeURI(r);
	
	// add current page index value
	if (i)
		params = params + '&FP_CURRENT_IMAGE_INDEX=' + i;
	url = url + "?" + params;
	document.location = url;
}

// Go Back: if there's a URL as a param, go to it, else go back using history
function FPGoBack (url,pageindex,anchor) {
	if (url) {
		if (pageindex)
			url = url + "&FP_CURRENT_IMAGE_INDEX=" + pageindex;
		if (anchor)
			url = url + '#' + anchor;
		document.location = url;
	} else {
		window.history.back();
	}
}

//---------------
// Resize background picture when screen resized
// ID=pictureBehind
// Resize proportionally, crop to fit
function ResizeBackgroundPicture (myID, margin, noCrop) {
	// If no ID passed, e.g. called from an event handle
	// use the ID of the calling object
	myID || (myID = this);
	
	if (!margin) {
		margin = {
			top	: $(myID).css('margin-top'), 
			bottom	: $(myID).css('margin-bottom'), 
			left	: $(myID).css('margin-left'), 
			right	: $(myID).css('margin-right')
		};
	}

	//$('#message').html('margin ' + myID + " : " + margin.top);

	var border = 0;
	var wWidth  = $(window).width();
	var wHeight = $(window).height();

	var pictureBehindH = $(myID).attr('originalHeight');
	var pictureBehindW = $(myID).attr('originalWidth');
	var h,w;
	var marginV, marginH;

	if (pictureBehindH + pictureBehindW) {
		var pictureBehindR = pictureBehindW / pictureBehindH;
		h = pictureBehindH;
		w = pictureBehindW;
		
		marginV = parseInt(margin.top.replace('px','')) + parseInt(margin.bottom.replace('px',''));
		marginH = parseInt(margin.left.replace('px','')) + parseInt(margin.right.replace('px',''));
		
		if (isNaN(marginV))
			marginV = 0;

		if (isNaN(marginH))
			marginH = 0;
		
		// 'noCrop' means noCrop-fit to screen: reduce and enlarge the picture
		// 'noCrop' is false means enlarge only, and crop to fit the screen
		noReduce = false;
		if (noCrop) {
			// reduce/enlarge the height to fit
			h = wHeight-(2*border)-marginV;
			w = Math.floor(h * pictureBehindR);
			// if the width is still too large, reduce the width
			if (w > wWidth-(2*border)-marginH) {
				w = wWidth-(2*border)-marginH;
				h = Math.floor(w / pictureBehindR);
			}
		} else {
			if (noReduce) {
				if (pictureBehindH < wHeight-(2*border)-marginV) {
					h = wHeight-(2*border)-marginV;
					w = Math.floor(h * pictureBehindR);
				}
			} else {
				h = wHeight-(2*border)-marginV;
				w = Math.floor(h * pictureBehindR);
			}
			// If width too small, fit the width
			if (w < wWidth-(2*border)-marginH) {
				w = wWidth-(2*border)-marginH;
				h = Math.floor(w / pictureBehindR);
			}
		}
		
		// Must account for the #container's padding and margins, if it is the parent.
		// This will happen for resizing images that don't fill the page.
		if ( $(myID).parent().is('#container') ) {
			var parentID = $(myID).parent().attr('id');
			if (parentID != undefined) {
				var extraTop = parseInt($('#container').css('padding-top')) + parseInt($('#container').css('margin-top'));
				var extraLeft = parseInt($('#container').css('padding-left')) + parseInt($('#container').css('margin-left'));
				h = h - extraTop;
				w = w - extraLeft;
			}
		}

		// DEBUG
		//$('#error').html('h='+h+", w="+w+", r="+pictureBehindR+", WinW="+(wHeight-(2*border)-marginV)+", WinH="+(wWidth-(2*border)-marginH) + ', marginH='+marginH);
		
		// RESIZE
		//$(myID).height(h).width(w).css('top',margin.top).css('left',margin.left);
		$(myID).height(h).width(w);
	}
}


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



// Show/Hide object id=id
function showhide (id, disableflag) {
	if (!id || disableflag == true)
		return false;
	$('#'+id).toggle();
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


function CenterInWindow (myObj) {
	var wWidth  = $(window).width();
	var wHeight = $(window).height();
	var w = $(myObj).width();
	var h = $(myObj).height();
	var l = Math.floor((wWidth - w)/2);
	var t = Math.floor((wHeight - h)/2);
	var css = {top: t, left: l};
	$(myObj).css(css);
}

// Set the background image
function setBackground(i) {
	if (i && document.body) {
		document.body.style.backgroundImage = 'url('+i+')';
	}
}

function getRandomImage(theImages) {
	var p = theImages.length;
	var whichImage = Math.floor(Math.random()*p);
	var url = theImages[whichImage];
	//alert ('length='+p+', which='+whichImage+', url='+url);
	return url;
	
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

/*
	UpdateTotal
	If "pickup" is checked, we don't need shipping, so don't recalc, but 
	set values to zero.
	1) UpdatePrices gets fresh shipping params, e.g. size & weight.
	2) Update shipping calculations
	3) Remember, UpdateShippingCalc calls UpdatePrices() when it finishes the AJAX call!!!
	
	If the variable, FP_DONT_CALC_SHIPPING, is set, we don't calc shipping. This saves some AJAX calls.
*/
function UpdateTotal (myCurrency, currencyPosition) {

	var pickup = $('#pickup:checked').val();
	if (pickup) {
		$('#handling').val(0);
		$('#shipping').val(0);
		$('#shipping_method').val(FP_CUSTOMER_PICKUP_TEXT);
		$('#no_shipping').val(1);	//1 – do not prompt for an address
		UpdatePrices();
		UpdateShippingParams();
	} else if (FP_DONT_CALC_SHIPPING) {
		UpdatePrices();
		UpdateShippingParams();
	} else {
		UpdateShippingCalc();
		// UpdatePrices is called by UpdateShippingCalc
	}
}


/*
	UpdateShippingParams
*/
function UpdateShippingParams () {
	var unitPrice, totalPrice, unitCost, totalCost, unitWeight, totalWeight;
	var pkg, loc, tax_applied;
	
	if (!FP_PRICING_DATA[0]) {
		GetAllPricingData (true);			// the price loader calls UpdatePrices when it's done.
		return false;
	}

	var framed = document.getElementById("framemattepricelist").value.split(",");
	var unframed = document.getElementById("printpricelist").value.split(",");
	
	var size = document.getElementById("currentsize").value;
	var frame = document.getElementById("currentframe").value;
	var matte = document.getElementById("currentmatte").value;
	var quantity = document.getElementById("currentquantity").value;

	var extrashippinglist = document.getElementById("extrashipping").value.split(",");


	// PACKAGE?
	// Is this a print only, print+matte, framed to print + print, frame+matte+print?
	if (frame * matte > 0) {
		pkg = "Frame";
//		pkg = "FrameMatte";
	} else if (frame > 0) {
		pkg = "FrameToPrint";
	} else if (matte > 0) {
		pkg = "Matte";
	} else {
		pkg = "Print";
	}

	// SHIPPING
	var prices = FP_PRICING_DATA[size];
	// Copy weight and dimension to shipping fields
	$('#shipping-packageWeight').val(prices[pkg+'ShipWeight']);
	
	
	var handling_rates = document.getElementById("shipping_handling_rates").value.split(",");
	switch ( pkg ) {
		case "Print" :
			$('#handling').val(handling_rates[0]);
			$('#handling_intl').val(handling_rates[1]);
			break;
		case "Matte" :
			$('#handling').val(handling_rates[2]);
			$('#handling_intl').val(handling_rates[3]);
			break;
		case "FrameMatte" :
			$('#handling').val(handling_rates[4]);
			$('#handling_intl').val(handling_rates[5]);
			break;
		case "FrameToPrint" :
			$('#handling').val(handling_rates[4]);
			$('#handling_intl').val(handling_rates[5]);
			break;
	}			


	if ( prices[pkg+"PackageDims"] && prices[pkg+"PackageDims"].width > 0) {
		$('#shipping-shippingWidth').val(prices[pkg+'PackageDims']['width']);
		$('#shipping-shippingHeight').val(prices[pkg+'PackageDims']['height']);
		$('#shipping-shippingLength').val(prices[pkg+'PackageDims']['depth']);
	}
	
	// PRICE + COST
	unitPrice = prices['Total'+pkg+'Price'];
	unitCost = prices['Pretax'+pkg+'Cost'];

	// Source state for shipping
	// ???
	
	$('#shipping-shippingValue').val(quantity * unitCost);
	$('#customValue').val(quantity * unitCost);
}



/*
	UpdatePrices
	FP_PRICING_DATA has all pricing data!

*/
function UpdatePrices (myCurrency, currencyPosition) {
	var unitPrice, totalPrice, unitCost, totalCost, unitWeight, totalWeight;
	var pkg, loc, tax_applied;
	var shipping,handling;
	
	if (!FP_PRICING_DATA[0]) {
		GetAllPricingData (true);			// the price loader calls UpdatePrices when it's done.
		return false;
	}
	
	if (myCurrency == undefined)
		myCurrency = FP_CURRENCY;
	if (currencyPosition == undefined)
		currencyPosition = FP_CURRENCY_POSITION;

	var framed = document.getElementById("framemattepricelist").value.split(",");
	var unframed = document.getElementById("printpricelist").value.split(",");
	
	var size = document.getElementById("currentsize").value;
	var frame = document.getElementById("currentframe").value;
	var matte = document.getElementById("currentmatte").value;
	var quantity = document.getElementById("currentquantity").value;

	var extrashippinglist = document.getElementById("extrashipping").value.split(",");
	//var weights = document.getElementById("weightlist").value.split(",");
	//var weightsframed = document.getElementById("weightsframedlist").value.split(",");

	
	// PRICING DATA
	var prices = FP_PRICING_DATA[size];

	// PACKAGE?
	// 1)Is this a print only, print+matte, framed to print + print, frame+matte+print?
	// 2)If this kind of item can be grouped with others for shipping,
	// then set the shipping_group. "1" is fine for prints, we could user other numbers
	// for other things. "0" means no grouping.
	if (frame * matte > 0) {
		pkg = "FrameMatte";
		$('#shipping_group').val(0);
	} else if (frame > 0) {
		pkg = "FrameToPrint";
		$('#shipping_group').val(0);
	} else if (matte > 0) {
		pkg = "Matte";
		$('#shipping_group').val(0);
	} else {
		pkg = "Print";
		$('#shipping_group').val(1);
	}

	// Originals always ship by themselves.
	if (FP_EDITION_TYPE == "original")
		$('#shipping_group').val(0);


	// PRICE + COST
	unitPrice = prices['Total'+pkg+'Price'];
	unitCost = prices['Pretax'+pkg+'Cost'];
	
	
	// APPLY DISCOUNT
	var discount = $('#discount').val();
	if (!discount)
		discount = 0;
	var discountrate = discount/100;
	unitPrice = unitPrice - (unitPrice * discountrate);
	if (unitPrice < unitCost)
		unitPrice = unitCost;
	if (discount > 0) {
		$('#coupon_price_display').html(formatPercent (discount));
		$('#coupon_description_display').html($('#coupon_description').val());
		$('#discountdisplay').fadeIn(250);
	} else {
		$('#discountdisplay').fadeOut(250);
	}
	
	
	// SHIPPING

	// check if "pickup" checkbox is checked, meaning no shipping
	var pickup = $('#pickup:checked').val();
		
	$('#shipping-packageWeight').val(prices[pkg+'ShipWeight']);

	if (	prices[pkg+"PackageDims"] && prices[pkg+"PackageDims"].width > 0) {
		$('#shipping-shippingWidth').val(prices[pkg+'PackageDims']['width']);
		$('#shipping-shippingHeight').val(prices[pkg+'PackageDims']['height']);
		$('#shipping-shippingLength').val(prices[pkg+'PackageDims']['depth']);
	}
	
	$('#shipping-shippingValue').val(quantity * unitCost);
	$('#customValue').val(quantity * unitCost);

	// HANDLING
	// * this depends on knowing the shipping destination (domestic/intl)
	// Get handling based on size, location
	var destcountry = $('#shipping-destCountry').val();
	if (destcountry) {
		if (destcountry != prices.SupplierCountry)
			loc = "Intl";
		else
			loc = "";
	}
	handling = prices[pkg+'Handling'+loc];
	$('#handling').val(handling);

	// SALES TAX
	var salestax = prices[pkg+'SalesTax'];
	var deststate = $('#state').val();
	
	// If pickup, then deststate is definitely the sale tax state!
	if (pickup) {
		deststate = prices.SupplierState;
	}
	
	
	if (deststate == prices.SupplierState) {
		$('#tax_display').html(formatCurrency (salestax, FP_CURRENCY, FP_CURRENCY_POSITION));
		$('#salestaxdisplay').fadeIn(250);
		tax_applied = salestax;
	} else {
		$('#salestaxdisplay').fadeOut(250);
		$('#tax_display').html('0.00');
		tax_applied = 0;
	}
	$('#tax').val(tax_applied);	
	$('#TaxMethodMsg').html(prices['TaxMethodMsg']);

	// Fill in what we know about shipping/handling/tax
	// Even if FP_DONT_CALC_SHIPPING is set, some of this comes from the supplier info,
	// not from a shipper AJAX calculation
	
	if (!FP_DONT_CALC_SHIPPING) {

		// Copy weight and dimension to shipping fields
		/*
		unitWeight = prices[pkg+'Weight'];
		if (unitWeight) {
			$('#shipping-packageWeight').val(unitWeight);
		}
		*/
		
		// Hopefully, this is filled in.
		shipping = $('#shipping_price').val();
	
		if ((1*shipping) == shipping) {
			shipping = Math.ceil(shipping);
			$('#shipping_price_display').html(formatCurrency (shipping, FP_CURRENCY, FP_CURRENCY_POSITION));
			$('#shipping_price_display_entry').html(formatCurrency (shipping, FP_CURRENCY, FP_CURRENCY_POSITION));
			//totalPrice = (1*totalPrice) + (1*shipping);
			$('#shipping_price_display_error').html('');	// this will show the error msg
		} else {
			$('#shipping_price_display_error').html("Error: "+shipping);	// this will show the error msg
			$('#shipping_price_display_entry').html(0);	// this will show the error msg
			shipping = 0;
		}
		$('#shipping').val(shipping);

		if (shipping > 0) {
			handling = prices[pkg+'Handling'+loc];
		} else {
			handling = 0;
		}	
		$('#handling').val(handling);
	

	} else {
		var handling = 0;
		var shipping = 0;
		var tax_applied = 0;
	}
	

	var itemsPrice = quantity * unitPrice;
	totalPrice = (quantity * unitPrice) + (1*handling) + (1*shipping) + (1*tax_applied);
	$('#amount').val(itemsPrice);	// price of the item w/o shipping/handling/tax
	$('#cost').val(unitCost);	// cost of the item w/o shipping/handling/tax
	$('#total').val(totalPrice); // total amount charged to client
	$('#weight').val(prices[pkg+'ShipWeight']);

	// Update hidden fields for a shopping cart
	//$('#price').val(itemsPrice);
	
	// Update display values
	var unitPrice_display = formatCurrency (itemsPrice, myCurrency, currencyPosition);
	var totalPrice_display = formatCurrency (totalPrice, myCurrency, currencyPosition);
	var shipping_display = formatCurrency (shipping, myCurrency, currencyPosition);
	var handling_display = formatCurrency (handling, myCurrency, currencyPosition);
	
	// unit price for cart & calculations
	$('#unitPrice').val(unitPrice);

	// Display of the unit price
	$('#unit_price').html(addCommas(unitPrice_display));

	// No handling if no shipping (nat'l/int'l determines handling)
	//if (!shipping)
	//	shipping_display = 0;

	if (pickup) {
		$('#shipping_price_display').html('(pickup)');
	} else {
		$('#shipping_price_display').html(addCommas(shipping_display));
	}

	// No handling if no shipping (nat'l/int'l determines handling)
	//if (shipping === "")
	//	handling_display = "";

	$('#handling_display').html(addCommas(handling_display));
		
	// No total if incomplete
	
	if (!FP_DONT_CALC_SHIPPING && ( !pickup && (!shipping || !handling))) {
		totalPrice_display = "";
	}
	
	$('#total_price').html(addCommas(totalPrice_display));
	

	// WE'LL MOVE SHIPPING/HANDLING TO A CHECKOUT PAGE
	/*
	
	// BUY BUTTON
	// If the shipping/handling are NOT set, dim the buy button
	if (!pickup && !shipping) {
		DimBuyButton();
		DimAddButton();
	} else {
		UnDimBuyButton();
		UnDimAddButton();
	}
	*/
	
	//$.log('UpdatePrices', 'pkg = '+pkg, unitPrice);

}


/*
	Update the shipping calculations
*/
function UpdateShippingCalc () {
	// Remember, UPSShippingCalculator calls UpdatePrices when finished loading data
	if ($('#pickup:checked').val() != null) {
		$('#shipping_price_display').html("(pickup)");
		$('#shipping_price').val(0);
		$('#handling').val(0);
		UpdatePrices();
	} else {
		$('#shipping_price_display').html("(loading)");
		shipcalc = new UPSShippingCalculator('#shipping_price');
		shipcalc.getQuote('#shipping_price','#shipping_method');
		// UpdatePrices is called by getQuote, I think...
		//shippingMethod = document.getElementById("13_product").options;
		//alert (shippingMethod[0].text);
	}
}

/*
Collect the spec's the user defines in the print shop, e.g. frame, matte, glazing, etc.
This collection is a unique code for an item in the cart, and it allows us to
link bank to the frame shop to edit an item.

*** called below, by UpdateSpec ***
*/
function UpdateSpecForCart ()
{
	var size = parseInt(0 + $('#currentsize').val());
	var frame = parseInt(0 + $('#currentframe').val());
	var matte = parseInt(0 + $('#currentmatte').val());
	var paper = parseInt(0 + $('#currentpaper').val());
	var inkset = parseInt(0 + $('#currentinkset').val());
	var glazing = parseInt(0 + $('#currentglazing').val());
	var imageID = $('#currentimageID').val();
	
	if (FP_EDITION_TYPE != "original") {
		var sep = "x";
		var uniqueID = size + sep + frame + sep + matte + sep + paper + sep + inkset + sep + glazing + sep + imageID;
	} else {
		var uniqueID = imageID;
	}
	
	// remove "undefined" entries
	var match = new RegExp("undefined"+sep, "ig");
	var uniqueID = uniqueID.replace(match,"");
	$('#cart_unique_id').val(uniqueID);
	
	$('#cart_finishing_url').val(encodeURI(document.URL));
	
	// Update the shipping param fields, e.g. height, width, etc.
	// UpdateShippingParams();	
}

// Update the encoded spec's for the sale, e.g. size, frame, paper, etc.
// Put in 'spec' field and 'custom' field
function UpdateSpec ()
{
	var system_units, system_matte_width;
	var artistID, dims, extrashippinglist, filename, fp_artborder, frame, framecode, framecodes, glazing, glazingcode, glazingcodes, image_catalog_num, imageID, inkset, inksetcode, inksetcodes, isizes, matte, mattecode, mattecodes, maxpside, maxside, mymatchprint, paper, papercode, papercodes, pdims, psizes, quantity, row, rows, size, sizes, spec, supplierID, system_matte_width, system_units;
	var spec;

	UpdateSpecForCart();
	
	system_units = $('#system_units').val();
	system_matte_width = $('#system_matte_width').val();

	size = parseInt($('#currentsize').val());
	frame = parseInt($('#currentframe').val());
	matte = parseInt($('#currentmatte').val());
	paper = parseInt($('#currentpaper').val());
	inkset = parseInt($('#currentinkset').val());
	glazing = parseInt($('#currentglazing').val());
	artistID = parseInt($('#currentartistID').val());
	filename = $('#currentfilename').val();
	imageID = parseInt($('#currentimageID').val());
	quantity = parseInt($('#currentquantity').val());
	mymatchprint = $('#matchprint').val();
	supplierID = parseInt($('#supplierid').val());
	
	if (!FP_DONT_CALC_SHIPPING) {
		// Check for extra handling for international orders when using a shipping calculator
		// If source country != dest country, its an int'l order
		if ($('#shipping-originCountry').val() != $('#shipping-destCountry').val()) {
			$('#handling').val(1);
		} else {
			$('#handling').val(0);
		}
	}
	extrashippinglist = $('#extrashipping').val().split(",");
	
	if ( typeof shipping === 'undefined' || shipping === null )
		shipping = 0;

	if (shipping > 0) {
		extraShipping = 1 * extrashippinglist[size]; 	// 1*  forces a type change to numeric
	} else {
		extraShipping = 0;
	}
	
	sizes = $('#dimslist').val().split(",");
	maxside = sizes[size];
	
	$('#MaxSide').val(maxside);

	psizes = $('#maxdimslist').val().split(",");
	maxpside = psizes[size];
	
	// Image sizes
	isizes = $('#dimslist').val().split(",");
	dims = isizes[size].split("-");

	// Paper sizes
	// If framed, but not matted, and the flag is on, then paper & image are same size
	// If it is original artwork, paper size is the same as image size
	fp_artborder = $('#system_artborder_noframe').val();

	if  (FP_EDITION_TYPE == "original" || ((frame > 0) && (matte == 0) && (fp_artborder == 0))) {
		psizes = sizes;
	} else {
		// there is an art border
		psizes = $('#pdimslist').val().split(",");
	}
	pdims = psizes[size].split("-");

	framecodes = $('#framecodeslist').val().split(",");
	framecode = framecodes[frame];

	mattecodes = $('#mattecodeslist').val().split(",");
	mattecode = mattecodes[matte];
	mattewidth = Math.round (system_matte_width * dims[1] *10 ) / 10;

	glazingcodes = $('#glazingcodeslist').val().split(",");
	glazingcode = glazingcodes[glazing];

	papercodes = $('#papercodeslist').val().split(",");
	papercode = papercodes[paper];

	inksetcodes = $('#inksetcodeslist').val().split(",");
	inksetcode = inksetcodes[inkset];

	rows = $('#rowslist').val().split(",");

	// row in the pricelist record, e.g. editionsize1, 2, 3,
	row = rows[size];
	
	// Catalog number
	image_catalog_num = $('#image_cat_num').val();

	// these globals must be set in the main code
	// this is done by inserting the frameshop_codes snippet in a Javascript block in the main page code
	// so the ReplaceSystemVariables can insert the system values for it.
	
	//THIS CAN ONLY BE 200 CHARACTERS AFTER THE URL ENCODE, SO BE CAREFUL ABOUT CODE LENGTHS!
	
	spec = FP_ORDER_ROW + "=" + row;
	spec = spec + "&" + FP_QUANTITY + "=" + quantity;
	spec = spec + "&" + FP_ORDER_SIZE + "=" + maxside;
	spec = spec + "&" + FP_ORDER_PSIZE + "=" + maxpside;
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
	spec = spec + "&" + FP_ORDER_IMAGEWIDTH + "=" + dims[0];
	spec = spec + "&" + FP_ORDER_IMAGEHEIGHT + "=" + dims[1];
	spec = spec + "&" + FP_ORDER_PRINTWIDTH + "=" + pdims[0];
	spec = spec + "&" + FP_ORDER_PRINTHEIGHT + "=" + pdims[1];
	spec = spec + "&" + FP_ORDER_MATCHPRINT + "=" + mymatchprint;
	spec = spec + "&" + FP_ORDER_SUPPLIER_ID + "=" + supplierID;
	spec = spec + "&" + FP_ORDER_EXTRA_SHIPPING + "=" + extraShipping;
	spec = spec + "&" + FP_ORDER_CAT_NUM + "=" + image_catalog_num;

	//spec = Url.encode(spec);	// URL encode
	spec.length > 200 && alert ("Warning: Please tell the webmaster that 'the spec code is too long for Paypal'. There will be a problem with this order. We apologize and will fix this problem as soon as we can.");

	
	$('#spec').val(spec);
	//document.getElementById("spec64").value = encode64 (spec);	// testing
	
}

// Update the description field to be sent to the printer
// Also, update the display information about the print, e.g. size, etc.
function UpdateDesc ()
{
	var system_units, system_matte_width;
	var a1, a2, desc, dims, frame, framename, framenames, glazing, glazingname, glazings, image_catalog_num, imageID, inkset, inksetname, inksets, matte, mattecolor, matteinfo, mattenames, mattewidth, myartistname, myimagename, mymatchprint, paper, paperinfo, papername, papers, psizename, psizenamef, psizes, psizesf, pword, quantity, shortSize, size, sizeAsText, sizename, sizenamef, sizes, sizesf, fp_artborder, shortdesc, i, x, y;
	
	system_units = $('#system_units').val();
	system_matte_width = parseInt($('#system_matte_width').val());

	size = parseInt($('#currentsize').val());
	frame = parseInt($('#currentframe').val());
	matte = parseInt($('#currentmatte').val());
	glazing = parseInt($('#currentglazing').val());
	paper = parseInt($('#currentpaper').val());
	inkset = parseInt($('#currentinkset').val());
	mymatchprint = parseInt($('#matchprint').val());
	if (mymatchprint > 0) {
		mymatchprint = "matchprint required";
	} else{
		mymatchprint = "no matchprint";
	}

	myartistname = $('#artistname').val();
	imageID = $('#currentimageID').val();
	image_catalog_num = $('#image_cat_num').val();
	myimagename = $('#imagename').val();

	mattenames = $('#mattenameslist').val().split(",");
	mattecolor = mattenames[matte];

	framenames = $('#framenameslist').val().split(",");
	framename = framenames[frame];
	if (frame == 0) {
		framename = "unframed";
	} else {
		framename += " frame";
	}

	sizes = $('#dimslist').val().split(",");
	sizesf = $('#sizeslist').val().split(",");
	sizeAsText = sizes[size].replace("-","x");
	sizename = "image " + sizes[size].replace("-","x");
	
	// If framed, but not matted, and the flag is on, then paper & image are same size
	var fp_artborder = $('#system_artborder_noframe').val();
	//alert (frame + ',' +matte+','+fp_artborder);
	if  ((frame > 0) && (matte == 0) && (fp_artborder == 0)) {
		psizes = sizes;
		psizesf = sizesf;
	} else {
		// there is an art border
		psizes = $('#pdimslist').val().split(",");
		psizesf = $('#papersizeslist').val().split(",");
	}
	psizename = "paper " + psizes[size].replace("-","x");

	sizenamef = sizesf[size];
	
	psizenamef = psizesf[size];
	
	glazings = $('#glazinglist').val().split(",");
	glazingname = glazings[glazing];
	
	if (FP_CUSTOMER_CHOOSES_PAPER) {
		papers = $('#paperslist').val().split(",");
		papername = papers[paper];
		paperinfo = papername + " paper";
	} else {
		paperinfo = "";
	}
	
	inksets = $('#inksetslist').val().split(",");
	inksetname = inksets[inkset];

	sizes = $('#dimslist').val().split(",");
	dims = sizes[size].split("-");

	mattewidth = Math.round (system_matte_width * dims[1] *10 ) / 10;

	quantity = $('#currentquantity').val();
	pword = "print";
	if (quantity > 1)
		pword = pword + "s";
	
	if (matte > 0) {
		matteinfo = mattecolor + " matte";
		//matteinfo = mattecolor + " matte (" + mattewidth + " " + system_units + " wide)";
	} else {
		matteinfo = "";
	}
	
	a1 = [];
	a2 = [];
	
	a1.push("Cat#" + image_catalog_num);
	if (quantity > 1)
		a1.push(quantity + " " + pword + "s");
	a1.push(sizename + " " + system_units);
	a1.push(psizename + " " + system_units);
	a1.push(framename);
	a1.push(matteinfo);
	a1.push(paperinfo);
	
	for (x in a1) {
		if (a1[x] != "")
			a2.push(a1[x]);
	}
	
	desc = a2.join(', ');
	
	desc.length > 198 && alert ("Warning: the description is too long ("+desc.length+" chars, only 200 chars will show)\n" + desc);
	
	$('#desc').val(desc);
	
	// Short description (for shopping carts)

	// Get a rounded-up version of the size, e.g. 3.35 x 4.6 => 3 x 5
	shortSize = Math.round(dims[0]) + "&times;" + Math.round(dims[1]);

	var shortdesc = shortSize;
	if (parseInt(frame) > 0)
		shortdesc = shortdesc + ", frame";
	if (parseInt(matte) > 0)
		shortdesc = shortdesc + ", matte";
	if (parseInt(glazing) > 0)
		shortdesc = shortdesc + ", glazing";
		
	if (FP_CUSTOMER_CHOOSES_PAPER) {
		shortdesc = shortdesc + ", "+papername;
	}	
	
	$('#cart_item_short_desc').val(shortdesc);

	//printinfo = "Artwork by " + myartistname + " (#" + imageID + ":" + myimagename + ")<br>" + quantity + " " + pword + ", " + sizename + " " + system_units + " print size, " + framename + ", " + mattecolor + " matte"  ;

	//printinfo = "Artwork by " + myartistname + " (#" + imageID + ":" + myimagename + ")<br>" + quantity + " " + pword + ", " + sizename + " " + system_units + " print size, " + framename + ", " + mattecolor + " matte"  + " (" + mattewidth + " " + system_units + " wide), " + papername + " paper, " + inksetname + " inkset, " + glazingname + " glazing, " + mymatchprint;

	$("#display_size").html(sizenamef );
	$("#display_psize").html(psizenamef );
	
}

/*
	Confirm the Frameshop purchase form.
	- Shipping chosen
	???- Copy exists to buy (at the last minute)* This might be done by PHP buy script....
*/
function VerifyPurchase() {
	shipping = $('#shipping_price').val();
	var pickup = $('#pickup:checked').val();
	if ((shipping > 0) || pickup ) {
		return true;
	} else {
		alert ("Please enter your shipping information. We cannot calculate the final price until you do.");
		return false;
	}
}


// CalcPrintCost (x,y)
// x and y should be the same units as the rate calculate unit, i.e. if the
// rate is $15/square foot, then x,y should be in feet.
// Calculate the cost of printing based on the size given, area = x * y
// Need a print's price per square unit
// (UNUSED HERE) costMethod = medium/image...do we measure by paper size or image size? 

function CalcPrintCost (x,y) {
	var cost;
	
	// *** Values from the printer..UNUSED FOR NOW. Units are the system units,
	// and area is always square feet.
	
	var units = $('#print_cost_unit').val();
	var rateUnit = $('#print_cost_area_unit').val();	
	var rate = $('#print_cost_rate').val();
	// PrintCostMethod: 0=paper, 1=image
	//var costMethod = $('#print_cost_method').val();
	alert ("CalcPrintCost: length units: " + units + ", area: "+rateUnit);
	// convert measurements to metric
	// 0=cm, 1=inches
	if (units == 1) {
		x = x * 2.54;
		y = y * 2.54;
	}
	area = Math.ceil(x * y);
	
	// convert sq ft rate to sq cm rate
	// 1 square foot = 929.0304 square centimeter
	// 0 = cm2, 1=ft2
	if (rateUnit == 1) {
		rate = (rate / 929.0304);
	}
	
	cost = Math.round(area * rate * 100) / 100;
	return cost;
}
	
// ===============
// Formatting functions: currency, numbers
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

function formatPercent (amount) {
	amount = parseFloat(amount);
	amount = amount.toFixed(0) + "%";
	return amount;
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
	
	size = $('#currentsize').val();
	frame = $('#currentframe').val();
	matte = $('#currentmatte').val();

	framed = $('#framemattepricelist').val().split(",");
	unframed = $('#printpricelist').val().split(",");
	
	mattes = $('#matteslist').val().split(",");
	mattecolor = mattes[matte];

	framewidths = $('#framewidthslist').val().split(",");
	frameW = framewidths[frame];

	maxdims = $('#maxdimslist').val().split(",");
	//maxprintsize = $('#maxprintsizelist').val();
	maxprintsize = referencesize;
	
	
	// Set scaling to simulate real sizes
	scalefactor = (maxdims[size] / maxprintsize);
	// However, if FP_FRAMESHOP_PICTURE_SCALING is zero, don't scale the picture
	if (FP_FRAMESHOP_PICTURE_SCALING == 0)
		scalefactor = 1;
	imgW = Math.floor(imgW * scalefactor);
	imgH = Math.floor(imgH * scalefactor);
// 	picW = Math.floor(picW * scalefactor);
// 	picH = Math.floor(picH * scalefactor);


	//Matte settings
	fp_matte_scale = document.getElementById("system_matte_scale").value;
	fp_matte_bottom = document.getElementById("system_matte_bottom").value;
	fp_artborder = document.getElementById("system_artborder_noframe").value;

	// If no matte and no frame, show an art border.
	// Unless this is original artwork
	// Calculate the matte scaling based on the art border -- we use the matte drawing engine to show it.
	
	if (FP_EDITION_TYPE != "original") {
		if  ((matte + frame)==0) {
			fp_matte_scale = 1 - document.getElementById("system_artborder_width").value;
			fp_matte_bottom = 0;
			mattecolor = document.getElementById("system_artborder_color").value;
			matte = true;
		}
	
		// If frame, no matte, show an art border, unless config flag is off.
		if  ((frame > 0) && (matte < 1) && (fp_artborder > 0)) {
			fp_matte_scale = 1 - document.getElementById("system_artborder_width").value;
			fp_matte_bottom = 0;
			mattecolor = document.getElementById("system_artborder_color").value;
			matte = true;
		}
	}
	
	// if matte not set above...
	if (matte > 0) {
		dims = GetMatteDimensions (imgW, imgH, fp_matte_scale, fp_matte_bottom);
		picW = dims.iWidth;
		picH = dims.iHeight;
		imgW = dims.nWidth;
		imgH = dims.nHeight;
		mattetop = dims.matteWidth;
		mattebottom = dims.matteWidthBottom;
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


// ShowFinishingBasedOnSize
// Check the maximum size for which we offer finishing (framing, etc.)
// If the chosen size is greater, hide finishing options. The print will be sold
// only as a print.
// If chosen size is <= max size, then show finishing options
function ShowFinishingBasedOnSize () {
	var size = parseInt($('#currentsize').val());
	var maxdimslist = $('#maxdimslist');
	if (maxdimslist.length) {
		var maxdims = $('#maxdimslist').val().split(",");
		var maxside = parseInt(maxdims[size]);
		if (FP_MAX_FRAMED_SIZE > 0 && maxside > FP_MAX_FRAMED_SIZE) {
			// hide finishing
			DisplayFinishingElement ('frameshop-framelist', 'hide');
			DisplayFinishingElement ('frameshop-mattelist', 'hide');
			DisplayFinishingElement ('frameshop-glazinglist', 'hide');
			DisplayFinishingElement ('frameshop-paperlist', 'hide');
			DisplayFinishingElement ('frameshop-inksetlist', 'hide');
		} else {
			// show finishing
			DisplayFinishingElement ('frameshop-framelist', 'show');
			DisplayFinishingElement ('frameshop-mattelist', 'show');
			DisplayFinishingElement ('frameshop-glazinglist', 'show');
			DisplayFinishingElement ('frameshop-paperlist', 'show');
			DisplayFinishingElement ('frameshop-inksetlist', 'show');
		}
	}

}

// Swap showing two elements, one of which is the "show" and the other is "hide.
// For example, you can have a "Available" or "Unavailable".
function DisplayFinishingElement (id, status) {
	var t = 250;
	if (status != "show") {
		$('#'+id).fadeOut(t, function() {
			$('#'+id+'-hidden').fadeIn(t);
		});
	} else {
		$('#'+id+'-hidden').fadeOut(t, function() {
			$('#'+id).fadeIn(t);
		});
	}
}


// Calculate the art border (paper around the pic)
// If no matte and no frame, show an art border.
// If the edition type is "original", meaning original artwork, there's no border: just show the image
// Calculate the matte scaling based on the art border -- we use the matte drawing engine to show it.
// If frame, no matte, show an art border, unless config flag is off.
function ArtBorder (matte, frame) {

	var fp_artborder = document.getElementById("system_artborder_noframe").value;

	// If no matte and no frame, show an art border.
	// Calculate the matte scaling based on the art border -- we use the matte drawing engine to show it.
	if  (FP_EDITION_TYPE != "original" && (matte + frame)==0) {
		fp_matte_scale = 1 - document.getElementById("system_artborder_width").value;
		fp_matte_bottom = 0;
		mattecolor = document.getElementById("system_artborder_color").value;
		matte = true;
	}

	// If frame, no matte, show an art border, unless config flag is off.
	if  (FP_EDITION_TYPE != "original" || ((frame > 0) && (matte < 1) && (fp_artborder > 0))) {
		fp_matte_scale = 1 - document.getElementById("system_artborder_width").value;
		fp_matte_bottom = 0;
		mattecolor = document.getElementById("system_artborder_color").value;
		matte = true;
	}
}

// ==================================================================
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

// ==================================================================


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

// Resize image to fit screen
function ResizeObjToFitScreen (obj, imgW, imgH, vPad) {
	var noPx = document.childNodes ? 'px' : 0;
	
	if (!vPad)
		vPad = 0;
	myWindow = new WindowDimensions();
	slideMaxW = obj.width;
	slideMaxH = obj.height;
	imgWUnadjusted = obj.width;
	imgHUnadjusted = obj.height;

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
	obj.style.width = imgW+noPx;
	obj.style.height = imgH+noPx;
	
}

function test (obj) {
	alert (this.id);
}


// Resize an object and enclosed IMG to the gallery
// The height is the window, less header and footer. 
// The width is dependant on the window
// The image is resized so it fits vertically, not horizontally
// The original proportions of the image must be pre-stored by jQuery as an attribute of the image.
// It is the ratio of width to height, i.e. proportions = width / height.
// If not, floating point errors rapidly distort the shape of the image as we resize it.
// obj: object containing the image and the caption
function resizeToWindow ( obj, type, maxWidth, maxHeight, animate, resizeHeight, pictureclass, captionclass) {
	var DEBUG = false;

	// Handle cases of calls to function
	switch ( type )
	{
		case 'general': // general resize (window opened...sometimes...)
			return true;
			break;
		case 'resized': // window was resized
			break;
		default: // unknown
			break;
	}
	
	if ( typeof obj === 'undefined' || obj === null )
		return false;

	if ( typeof animate === 'undefined' || animate === null || isNaN(animate) )
		animate = false;
		
	debug(DEBUG,'fp_javascript_lib.resizeToWindow: maxWidth:' + maxWidth + ', maxheight: '+maxHeight);
	// Resize the pictures so they fit in the window
	// Used on transition or window resize
	
	var myspeed = "fast";
	
	// Get image object from inside of obj
	if (pictureclass) {
		var image = $(pictureclass);
	} else {
		var image = obj.find('img.gallerypic');
		debug(DEBUG,'Using img.gallerypic');
	}
	
	// Get caption object
	if (captionclass) {
		var captionblock = $(captionclass);
	} else {
		var captionblock = obj.find('.gallerypicinfo .captionblock');
		debug(DEBUG,'Using .gallerypicinfo .captionblock');
	}
	
	if ( !image || !image.width )
	{	// No image or no visible lightbox, so we don't care
		$.log('WARNING', 'A resize occured while no image or no lightbox...');
		return false;
	}
	
	//DEBUG && $(image).addClass('wrapped');
	
	var oWidth  = image.width();
	var oHeight = image.height();
	debug(DEBUG,' image.width:' + oWidth + ', image.height: '+oHeight);

	var proportions = image.attr('proportions');
	debug(DEBUG, 'proportions: '+proportions);

	if (proportions == "NaN" || proportions == undefined) {
		proportions = oWidth/oHeight;
		image.attr('proportions', proportions );
		debug(DEBUG, 'Recalc Proportions');

	}
	debug(DEBUG, 'proportions: '+proportions);
	
	var maxImageWidth = image.attr('maxImageWidth')
	var maxImageHeight = image.attr('maxImageHeight');
	
	(maxHeight > maxImageHeight) && (maxHeight = maxImageHeight);

	// Now, resize picture with matte to fit new size
	// scale to determine width of matte (padding)
	// and extra matte at the bottom
	dims = GetNewPictureDimensions (oWidth, oHeight, maxWidth, maxHeight, proportions, maxImageWidth, maxImageWidth);

	debug(DEBUG, 'fp_javascript_lib.js:resizeToWindow: Display maxWidth='+maxWidth);

	msg = "iWidth:" + dims.iWidth +", iHeight:"+dims.iHeight +", nWidth:"+dims.nWidth +", nHeight:"+dims.nHeight +", MatteWidth:"+dims.matteWidth;
	debug(DEBUG, msg);
	
	msg = "resizeToWindow: Ratio: " + dims.iWidth / dims.iHeight;
	debug(DEBUG, msg);
	
	// Resize the images and containers	
	if (animate) {
		image.animate({padding: dims.matteWidth+"px "+dims.matteWidth+"px "+dims.matteWidthBottom+"px "+dims.matteWidth+"px", width: dims.iWidth, height: dims.iHeight}, myspeed);
		obj.animate({width: dims.nWidth}, myspeed);
		// Resizing height kills captions below the image
		if (resizeHeight)
			obj.animate({height: dims.nHeight}, myspeed);
		captionblock.css('padding', dims.matteWidth+"px "+dims.matteWidth+"px "+dims.matteWidthBottom+"px "+dims.matteWidth+"px").width(dims.iWidth).height(dims.iHeight);
	} else {
		image.css('padding', dims.matteWidth+"px "+dims.matteWidth+"px "+dims.matteWidthBottom+"px "+dims.matteWidth+"px");
		image.width(dims.iWidth).height(dims.iHeight);
		obj.width(dims.nWidth);
		// Resizing height kills captions below the image
		if (resizeHeight)
			obj.height(dims.nHeight);
		captionblock.css('padding', dims.matteWidth+"px "+dims.matteWidth+"px "+dims.matteWidthBottom+"px "+dims.matteWidth+"px").width(dims.iWidth).height(dims.iHeight);
	}
	// Done
	return true;
}



// GetNewDimensions (Floating Point version)
// Resize an image to fit the given width/height with padding (for a matte). 
// The height will stay the same, but the width might change to accommodate the 
// new proportions from the matting.
// Start with image dimensions, then figure matted (framed?) dimensions
// which must fit in given limits.
// Depends on global MATTESCALE, MATTEBOTTOM
// Globals used:
// FPMatteRatio is a percentage of the image size, to determine matte width (padding)
// FPMatteBottomRatio is the same, but only applies to the bottom of the matte 
// and is added to the bottom matting
// Must use parseFloat(a.toFixed(2)) to fix JS math errors!
// 'proportions' is the width/height of the original image. Need to get around floating point
// math errors.
function GetNewPictureDimensions (iWidth, iHeight, maxWidth, maxHeight, proportions, maxImageWidth, maxImageHeight ) {
	var DEBUG = false;

	var matteWidth;
	var matteWidthBottom;

	oWidth = iWidth;
	oHeight= iHeight;
	
	// If no proportion is given, the following works but floating point calculation
	// errors quickly resize the pictures.
	if (proportions == 0)
		proportions = oWidth/oHeight;

	// Calc new width based on old width and change in height
	// MUST use .floor because the browser always chops off the decimal
	// when it uses the image height/width. If we don't chop it, there is 
	// a cumulative error since oHeight (image height) != iHeight (calc'd image height)
	m = 1 + 1*FPMatteRatio + 1*FPMatteBottomRatio;
	iHeight = Math.floor(maxHeight/m);
	if (DEBUG) {
		$('#error').append("<BR>m="+m+", FPMatteRatio: " +FPMatteRatio+", maxHeight: "+maxHeight+", iHeight="+iHeight+" Orig. Height= "+oHeight + "<BR>");
	}
	iWidth = Math.floor(iHeight * proportions);
	nHeight = Math.floor(iHeight +  (iHeight*FPMatteRatio) +  (iHeight*FPMatteBottomRatio));
	nWidth = Math.floor(iWidth + (iHeight * FPMatteRatio*2));
	
	// Figure new size of block to contain the picture
	// This loops, doesn't work. It's so we can fit H and W, not just H
	while (false)
	while ( nWidth > maxWidth || nHeight > maxHeight )
	{	// We need to resize
		if ( nWidth > maxWidth )
		{	// Resize width, then height proportionally
			resizeRatio = maxWidth/nWidth;
			maxHeight=resizeRatio*maxHeight;
			
			iHeight = maxHeight/(1+(2*FPMatteRatio)+FPMatteBottomRatio);
			iWidth = Math.floor(iHeight * proportions);
			nHeight = maxHeight;
			nWidth = Math.floor(iWidth + (iHeight * FPMatteRatio*2));
		}
		if ( nHeight > maxHeight )
		{	// Resize height, then width proportionally
			resizeRatio = maxHeight/nHeight;
			maxWidth = resizeRatio * maxWidth;
			nHeight = maxHeight;
			
			iHeight = Math.floor(maxHeight/(1+(2*FPMatteRatio)+FPMatteBottomRatio));
			iWidth = Math.floor(iHeight * proportions);
			nWidth = Math.floor(iWidth + ((iHeight * FPMatteRatio)/2));
		}
	}


	var matteWidth = Math.floor(iHeight * FPMatteRatio)/2;
	var matteWidthBottom = Math.floor(iHeight*FPMatteBottomRatio) + matteWidth;
	
	if (DEBUG) {
		msg = "newMaxW="+maxWidth+" newMaxH="+maxHeight+" oldW=" + oWidth + ", oldH=" + oHeight +" iW=" + iWidth + ", iH=" + iHeight + ", nW=" + nWidth+", nH=" + nHeight+ " matte="+matteWidth;
		$('#error').append(msg);
	}
	

	nWidth = iWidth + (2*matteWidth);

	//alert ("diffW:"+ (width-nWidth) + "diffH:"+ (height-nHeight));
	
	
	return {iWidth:iWidth, iHeight:iHeight, nWidth:nWidth, nHeight:nHeight, matteWidth:matteWidth, matteWidthBottom:matteWidthBottom};
}

// UNUSED - Deal with JavaScript floating point errors, esp. in IE
function FixMath (x) {
	return ( parseFloat(x.toFixed(4)) );
}


// Frameshop:
// Doesn't use extra for bottom matting!
function GetMatteDimensions (width, height, fp_matte_scale, fp_matte_bottom) {
	if (fp_matte_scale == 0)
		fp_matte_scale = FPMatteRatio;
	if (fp_matte_bottom == 0)
		fp_matte_bottom = FPMatteBottomRatio;
	
	var matteWidth = Math.ceil((width*(1-fp_matte_scale)) / 2);

	var new_interior_width =  Math.ceil (width * fp_matte_scale);
	var new_interior_height = Math.ceil (height * fp_matte_scale);

	var matteWidth = Math.ceil ((width / 2) * (1 - fp_matte_scale));

	var nHeight = new_interior_height + (2 * matteWidth);		
	var nWidth = width;

	var matteWidthBottom = nHeight - new_interior_height - matteWidth;

	return {iWidth:new_interior_width, iHeight:new_interior_height, nWidth:nWidth, nHeight:nHeight, matteWidth:matteWidth, matteWidthBottom:matteWidthBottom};
}

// Given an ID, e.g. 'myInput', use AJAX to set the project/group to a new themeID.
// It calles the PHP file, variation_update.php with the appropriate commands.
// It can be called from an HTML select, a popup menu of themes:
// if the ID is 'myID', then add onChange="ChangeTheme(\'myID\')"'
// *** the page is reloaded after this is called ***
function ChangeTheme (myID) {
	myID = '#'+myID;
	var themeid = $(myID).val();
	var data = {'command' : 'change', 'themeid' : themeid, 'projectid' : FP_PROJECT_ID, 'groupid' : FP_GROUP_ID, 'userid' : FP_USER_ID};
	var dataString = JSON.stringify(data);
	$.post('variation_update.php', {
		data: dataString
	}, function(res) {window.location.reload();}, "text");
}

// status = true, then show debugging
function debug(status,msg) {
	if (status && window.console && window.console.log) {
		window.console.log(msg);
	}
}

function FormatAsFloat(obj) {
	num = $(obj).val();
	if (isNaN(num))
		num = 0;
	num = Math.round(num*100)/100;
	num = num.toFixed(2).toString();
	$(obj).val(num);
}



/*
	Get US State from ZIP code:
	Original:  Daniel Dailey (jesusfreak@cyberdude.com)

	This script and many more are available free online at
	The JavaScript Source!! http://javascript.internet.com
*/
zipString = "0000000000000000000000000000000000000000000000000000CF000000000000000000000000000000000000000000000000000000000000000000000002000000000010000000000000E7C73C98072C43959647FA5748B0CFB6FFF738420EAB6D78D0E3000F1C0030C0000000000006CFFFFFFEF10EFFF6F8FEF100EBF707CFFF3046BF4FFAF77F7EFBCD0032C018100000000008E0000DF687DB0FFDD400000006000000C0EFDEC01D7AD100000000010C1530C30D9FF200000AF78FFFAD7F640075F00F6500EFFF000000000E00000000000E300C30C007E353104B501000AE5B06DCF0F3C70DF0091E708EF5A0E0E071BE1036E00C30000000C3EC94BBDF97EC1020420EFFFFFFFFFF3FB180E031FF09EFF934000C2000004200810A2E30004BC7EBFF8F7D5886000000000300000E87F7F9DF600F000000000DFF007FE39E1010002000000CFFEFFBAFDFB000000C00FEF700CFF100FDF0A90C006D3F7CFFBF7005EF38FFFEBF5EFFDF300001000000000000000000000877FBBF2F1C2B4D000EFF000000000000000000000000087FFEF6FF8FBF53FD79E2CEB000000000000000000000000000000880FFF976F70000000000000000000000204C9FB796EB300000000000000000000000000000000B6FF106F6A73000E3EFF7F4BFFFFFF5FFF4CF443EFF!0000000000000000000000EFFEB7977EFCFE3E5FFFEFDF7EF711C100000000000000000000C1BFFD4FB04FFFEDC5FFF500000000435EFFFE910000000064DFFFDFC4E1FEBF9BFD2AFB3000000048FA9BF5CBDFD000000EAFBCFFC2D77F75FBFBB7C6000000004FFFFC0FF77FBFE1000000000000ACFFFF300000000A0C96FF7BFFBEF7EFEFDEF3102020000CFFFEFFD7AFFBA7B002000000000E5FFD2000000000200000000000CF27000000000E100000000F7CFFF000000000EF00000008FFFFFF7F7FEEFD02000000000010000000000000E12001002038FF7FFEFF700006000000CFFFFD2EFF7F70000000008FF73FFEFAEFC9F000000EF0000000000000000000000064C2DFFFFAB2E3EFFFFFFBFF5EFFDE9CB76FFCF138001900080000004CFEEFFCDDF30E700000000010CFB10EEA12CFFDF700A3FFFD5C1FF9CCB18FFEF9FF6EFFFF33CF71000000000000006FDD000000004000002000008E7511F700000EFFC040FEFCE529C7310E1150CFF1048FB0303EFC707E100000000000000000EFFFFFDFF9FFCDEFFDFFF8DFBEFF5000000000000008041BFEEF10000000000000000000000CFF0000000000000000000008AA40FF159D55C17F244230084EFF7041424810000000000000EF1003DD10FF733F448000000E1E9FFDDFE1CDF7D300200008E4000FEF7FDFFB396EC!F72400600007DDFD16400F044FB0008EFFDDFEFFFE6FFBFFFFFBFFFFEF70000000000000000000000E14FEB7C50FFD10104000000040CFFFD710FF7307040000000E70000000000000000000000063CD41E5E1F1CEF7040000000EFF0C123043E5000400200C8020000F1CFD70CFF0000000000EB717FFFF3145FEEAFBF5F709E7000400200000000000003000825020B634646D98251346CDDDC508490FD0042072B6D6200008705E24A03410CF28000000E300000000100000001000807002083000008349A17000004DEFF022105183E478F7EF6CDE9A163EFFD05CB20000000000008FFE7FFFF0000000000AF1FF1C3FF9EFFFFE10F40E9015075E0000000000000000000000000EFFFFFFFFFBF18010218784CDEFFFFF7F3400EFFFFFFF0300B80830000004020F32FFF76010EFF70000000000000000000000000000000008FFFFFF000008EFE5FFD5B7FEF3E5CC3FB8FD7EF40002220004000000000000EF70000000000000000000000E30000000000000000000000020CFFFFD30B047DD37EBFF7F7E3400FF400F1CF00000000009E720010000000000000000000EFFFFF7FFFFBBD100000000000000000000008DFFFFFE36400028FFFFFF90080000000010082864510C814AFFFB7F9BF05FF0000000000000000000000CF2EF7EF7FEBFFEEF7DFFFFDDFF7E308800000!000E000000000002000000CFA7DFFFFBFB7100000878FDBBB3FECFFFFFF7CF6F00CD87F3D7FB9FEF7F7FBFF7F7EFF50DFFFFFF7937000000100EF300020000200000000000006FE7FF9FFDF5FFFB7FF2FFEF5EDDF7FFF66FFFB0AFFD6F7770E10000000000000000000000020008FF27FFEF5CDFF8FFFF10A1FDBF99FFEFFFFF7C7C8F000A0CFFFD7FBE76BF7F6FFBE2B7000007DFFBF3873F7B77F9730E9DFFF5ADFFF1FFDF00000000EFFDE3300001C103000000400E37FFFFBFEF65F2FB10000000EDFA7379AF706DB33FBFD7FF0E300000000000000000000008EEDFFFFDFFFEFDF7FFFFFBDFB0000000C7F9FFF7F00C7DD5C3EDFF01E4FEFFF09F300000000E300000000000000000000000A7F303FFFFF89DFFE790F68D4EAF301EEFDB3C000CF7000000EFFFFFF92030000BB63110000E30000000000000000000000000CE5DB7B2A2671EDF9F7E3004FDFF7E67FEFF8FB1B1016F10EFFFFFF00CCFE301102080050EDFDFFDFBFFBDF71C6DFEF100EF7FFFFC0FFFCFFBBE7FFFAF6E300002000000000000000000AF6DE7FDF7CFEFBFFBD7EFF8165511DCEFBDF1000000000200EFFFFFFFFFF1CFEFF5DFE640020CF3FBE7F7FF7EFF50F1000020CBBFBFFFDFFCDFF7FFD77106040070CFF7FFFEF30000000026CBFFFFFF7D485F7DFFFFFB722CF3FBF7F7FD7FFBCFFB10002!0000EBB503EB23B7500000006F280FFFC7D7BFFF000000000E0007FB7EF36FFFA400000000EADF3BBC73F6DEB300100000020CF7F77FD577EFF0000000002087BECBF0FFC707FFF000100EFCB3FD5F5F10000008000000EFFD040C74254648200000000E086BFBEEF3FDDBFFFFEF7AFC200001FFF497F000000000000E3000FE7FFBBFFDB7DFF6000020CD6FE7BFFE5000000000000EFF7FFFFFFFFBFFFFFF7FFF6EEFF30FF70010000000020000020CFAF3BBFBDEF3FFD1000040EDEFFFFA7ADF6F5F7D9000000EF08000000000000000000000EF31F783F79AED2DF7BFF3000EF10000000000000000000008E2000FDF8EDBF5DDBB7F00000204F0FBFFBFFCAF7F517FF30020000FAEF5E7B7ABF55DF2000E0CF712CDFF7EFDDFF7FBF91CE72000000000000000800008020C5DF3EDAFFFD1000000000020000FFDF3FDFF310C3000000AACEB3FDEFBFBFFF7E3000000EB7DE40000100000000200000E0CDFEFFD1F18E33000000400EFF000000000000EF36000000204F7FFF7ADFD60000000000020CF7FFFF5DDEE7D5F5B30800ED7FFFEFFFBD7FFAF4FDFEFFDEFFFDFFFFFFFFF070CBEEB973000000000001080000000000020C1F10CA8FFDF6DEEF1F048EAF300F7797F5CF3535F6FF7F1AFD1FD66EDFBFCA5B00000000EFF1000000100000000000000ECFF736CF10000000!00000000EF7000000000400000001FEFFE300000C6DFFCF5FDFAA10000EFFBF156BFFFEBFFFFE770D0EEFFBF71FDFFDBBD9FF977FDFFEDDFFFDFBE73C407650200208AC4CE0041710400004F010FF4EFFFDFFBEB798219071000000FF580FFCFFFDFFE16CBF36C6AFDFBFFF7DB9642F794C75F7B2AFFDFF78B7FFFFEC3FFFEBD4A000F70B43E68FFA749DFD7ADFEFDF500000000000000000002226F6D8FD3FFDF3708C75B57447962D0D39154F7F000000000EFFFFFFFEB31C10935ECFF7060000000000000000000000000E381000000000000000000000E3000F1F1DF248370000000002C63ADFBDDBBFFFFFFBFEF500E3EEFF80677C7CF4FEEFF9DF6E1C6265687F0E33EFED000400E18FF70400000000000000000C02800CCFFFD0001C008600B1E420D730C000C3D30010F38F8EFEFFCE640F70000000000000EFFF030871000000000000000EF110085B7C494190A3172100A9B62957CC4F1F41A41F30000E1C00DF40EF76D3910000000020AFDFFFFFFC300000000000068C9039CF0FFF200000000000EFF00D189FDE37E9BB7118200E3AC6EBC9C9A4CDFFF99A7E30EEFBB9FF0C3C7E1F07A7BFD100000CFFFF770680346FD57F76ED5E1FF1032A44F000000028F2B7FFFDEFFF0CDFFF908DC100EFFFEB0600208000000000000EF31000D0000820EF70020C71EB300000!00000000000000000A700069F7FFFC031C5FDEBF6E2028AF944FE94DCDD54100000EFFFFDFFF7D35ECFF73FCFFF0638F07DF7FECFA36A9CF03000E0E9F3750B87D0596E3AF2630208FFCF52098D50B45EE70000688B164CBE7250E9BB7BCF000E70F2FCFEDB1EE7C6E5F367FEEF37C59DCA3CA070000000000200D905A3F188000000000000209A7FFF59EFFFB7CF7EF93DF6048F1B8C5E7C46E4443A7820CBF8C67DA99FB6AF9C89E65A0CCFFCE7479F875973362FE020E7B60008C8ABD3D7FD89AE000E7AF2FFFFF00407730800821160C68FAF56F50000000000000EFDDAB56C23B48A5BE30000002C952A35DF0FE702EE554ED5BEFFFFFF3000008000CFF0000060C379E83BBFBEB57DAD500006DBFF7C955C08046EA1F273008000000CFF3848D504C000000E7000300E7ECF98796A43D000E678E417472046ABDFA5DAB7564000BA5DECCF40642D7070000BD9967CC799911A000000000EF1087BAE2ED0C8F4DD56EF0020C9EA9A4D20801588CD31C000BC4B54848804A2A900000000695D6810250A890060000500000000000000000000000000000CE7FFBD00FEFF0000000004CEFFE31840000C93000000000CE93F310FAAC933DF3004E3BFC66CF3F6C00F1E8F30CEB00000EFFBB1A28400080000001008CEFFFD77E71F7ABBD3F33EDB7BEFFF9FF3C110405201810048!EEFFB240000000000000000000EFF7FFFFFFFFEFFFFFFFFFFF30C68BFF79FED76F5F77DF60006C61FBF87FF00F100CFFF4F36EFFDA9F7EF0FC189D76000000EFFFFFFFF3FD4E5766F6FF70BEFFCC9EFFEFBFBFFFFFFDFDFDEFF10F3FFDEFFFFFFF1C10000E1FBDFFF3EFF8F3F1CFEFF640EFFBFEFFE7FFEFFFFFFFFBF7E6BFFFFFFFFFFFF7F6DFFEDF30E74E100000000000000000000E720000000000000000000000E46C510C3F7F9FDEAEFC700006F9F7CF7B6CED70F5F9710000EFFDFFF700104001000000410EF00DF3EF574BA1137FF07D80EF6FFD7EFF5EFB727FDBEB130E7C1D1E511FFCC19FFFFFBE77EFFF3FFDFDFFFFF6FEBFFFFB7C57D77DF70E30000000000000E373B6DBDDFFF213000000008E7CF5FFBFDB30000000000000CFFFFE7FFFFF1D5FFFDFFFDFFEFFEFFF2FEFFF72E3DBFDB300070FFD177730D3FB7BF32FFF7EFFFF7FF7FFF7EF75FCF1F75D20CF2BF34D6EFFF39CE200008EDCFFFAFEEFF3FEDF9B37000FEF318FBFECEE581DFB2690000AAD0877DFD7CCB30000000000CEFFFF1520000000000000000AFE7B00000000000000000008EFFFFFBFFF7FED6FFFBFEFFFF0C008048082640100000008BFEFF33300000000000000000CFEB7FDFDB20000000000000000EFFDD7000000000000000000CE04BDFB0FACFFF7BF3000000CE7000DFEDFEE700!000000000CEFFF7DF7FFFFE7FFFFFFFFEFFAFDD1FFE31000000000000000EF342000000000000000000EE09B20034E4F54FDFF938CF8DB42CFFFFEFF7AB0E117EFFA6B4EFFDFFFFFF3F4CF1C00300400EFFF3FDFF3BFDBF7000000088EFF32FFDFEDF3DBF700000000EFB71F5CEF79708FFFFFF0FF7EFFFF6E9E8B11E5CD300B4073ED7FD7F5BD0DC5BCF9FFF1EFDEFFFF7FFFB000EF783160CE2EEFF18DF8D330FFFFFF050000061FFFFFFFBF3CD1FFFF3E143268E596FFFFFFFFFFF7FFFF7FB000000086CCA0832620090009EFFFFFFFDE16B880000000140EFFFF7FFFFFF1CCFFFC7FD12F80E60AF44EB97010FF718F0F6EFFFFF7EE208C80F10FA7F742EFFF100F77F908FFFEFF77000EFEA09ECCFBFFFFDAD32F3500EFFFF79D950FFF312C90F0C60C0200F3CFF70CE41000E04CBCEF77B100EFF60000000000000EF78FF1FFFF0C001F5D77F0B0000000000000000000000000000000B0F714EF3033CFFF9B140000000000000000000000000EEF6000009000F13836AF3F860283000A4BFF3AD70E360E302000000000000000000000000000000000000EF7700078EFDD30FCE9F08F5F7BF7F1D7EDAA47008DFBFDABCDF00308BDFF100EFFFFF7FF7D72E8B400EFEFFF0000000000000000000000000EF00000000FD33FF5FE77EC00E100000000F7FF893CFF7F070E0EFF7!0C709EF7070C720000800000000087FFDFFFBF100000EFFF1010000000000000000EFEF0000000000C7FFD9FDBD700867A3DFFFFFEB726FB9CF8B30EFFFFB3CF0700000000200800EF0000000000CE7FFFFF10000E3C7FF0C0097E3070CE100000200001EA9967AE778CE3F0000EBAB4EF7BDFBFAD79FDFFF400EFFFFE6D203101090C8002FA0E0000FC11573F7DEF3ADC7430E100000C1000EFDFEFDA000002D5B96200000000000000000000CFDDFFF3FDF9FFFFFFDEF8720C8D68FF7FFFB07E8DCBFF00EFFFF797DFFF7000000000000EFFF7FFFFFFAFFBFFEFFFBBC7EFFD9610000040000000000002000000000000000000000000630C732000F340FD3200FDD582AFEBEFE36C31F3D7E7E00000EDEED77F00FEB72FFF7F7ED30602DFF9F3C1040000000004876DDDEAB74B7F7FFBCFCB90100AFFFF5FDFFFED1AAE1E2ED5E220000F7EB7708EF3080020000E2DE73DF7FFF99FEEFDF78F30600000200000EFBF1BF7EE100E700000000EBFFFDB7EEFA300E4E4EFEFFEEFEDCA7FFB96000E1000F6C3F17B6DFBB7570000E10003F3ECADFDFD7FEF10000600F3FF780BFFFEFF00000000000000000CFF84FFCA7EFC5D708FFB7040017F7FFEFEBF0E70EFFB7020910040000A10C5305EB200DF59F6E5DDFF00000000ED000EE30BF089BF74CFF0000EE00072EF0000B5FFB7320!080E000000EB4AA373EF30000000E34000048BBFCF95CA3100000000000000000000000000000020000000000000000000000008DF7CDE8333EF9FFEF870000009F8E20000D74BA73C8700000EFFFFF3B30E2CBF24710FAEFF00C3B5174B5CDAFF4C599FC10C3608E00425F111391B125390CFFF3C502105480000CFFFFF1E100C700000000000000000006000001642FF9C880A0000000AC5AD9DF009B2C5C256000000C40E0A8C98B5AA44301AE018AEFDDF40CEAF7BF9E38F1B68FF620402939AE560915FE8B0200A10C945D54104C3DB9400000028C620003C499105F102064020052866020018080301200000E007A5D9CEDD3EEFDB1000000EF09960C91DAEA89C2000000060078EAE899EA48D50F7000000149A679FF9B38C60000000000000000000000000000000000E0000FBBBF3FFE5B5E0DEF100E1000D9FE070C7B9DC0000000EDECBB00D13085B6E4E19E100E1000FF57E8BFD12FFD700000C5F281080373CBFF100000000E000D00128210240F00000000C000000AF4482200000000000608BF75BB0F43EE63001C510000000000000000000000000000000000000000000000000000EFFEFEEFB33F5C57FD77F7024EEEBB8E587DFFF7D100000008EFEFFFDFD11080016D18088086CCCFF7CF7FFF0F1000000000CC57110CFFFEE17DF3000000067C9FFFFF3FAF!F71EE0000000EFFFF91080000FB3C0006000A6082FFBDFD1C7DF7D93EDCB20C381061513D30000000000000EE7F31EDFEFFD30EDD73A7300A3C12E55B93F7F37B5737F7FBEFFFFFFFFFFF0000000E27FFEE0CD3E1CF7F0CE5574F57E100EFFFB7616000000000100002FE7DB7DBF1FF7EF010C20EFD30EFEF00000000080000000000C2CFFE7FF7FBFD7BFEE9FFAFAAEFF543048000400180000000CEBAE75F52FFFFEB7FCC4F8700EF00000000000000000000008E38F496479F1CF1EFF2000008EECFCF0CB45F5FF5FB5300000EFFFFFFFFFFFFF4C9FE20000DEFFDFFFDFFFFF7DFAF76FB700EF7FFDDFFA3034C8068860400E7000000000000000000000002007FF1F65DFD7FF0AFEFF5FF20CFBB92D8F74003DFBFFD3006F7FB7CDBFB7FEDFFFFBFFFEF0000000000010000000000008208F700CFD3FF97BBFD340000CFCF6FFFA0DDFC7FFDB1F5000EFB4FFFFCB79EE9141C6D2C85E9D18FB0007E791369FBF3C70EFF00000000000000000000006DDE30DD5FD74F1EEC7070480210FB153C2000001000010008E7C60B2C36FE588BFE7FFFBFFEF3CD0208002CFFFF3000208FE1CFBF4FBFFEC3E7ADCE5FFD5A4C1FF3CFFF000010000000004DDDFD7EFFFFC00FF4D200408E00009FFD19F600F3470F0000CF1005FCCF779FF7CFE0FF571EF1003DEFFA6BFFDBF0000400200F!9FBB936EE37691EF37D0720CFF10EEE183401640080000E7DF97BDFB394000000000000EBB7000C777EFFFF7FDD73000EF207F77FFF5FDEDFFC7FDFF3EF2D2EE5DFFFF48FFFF9F5EFFAFFF7FFDEFB7C7E7F6EFDFF96EFFFFFFFFFD10C41F3DE0018AEDFF7FFFF1FC52BFCCC3FCC2620CFFFC7FF77EE7FFFB100000EF20800A1000CFB0000000000EFFFFFFFF378EFFE844004000AE500FFFFFFF7CFFF700080006FBBFFFEF7FFF7FFFCFFFFFF32CFBF61622004010000010000EFFFFFDFFDFECB7FFFFFFBFFFED8BB3E700000000000000000E1000397FF7EFFFEFFDFFF300EFEDFFEFBFFFB830000000000E7FBFFFFC3F7FFFB110000000EF7D58240005580100000010820CFFFEFFFD7FEB9F5EBD77012EEBFFF7EF9FBBB777CFF7DBB6E1FF7EAEF3A3F4E7D7FBF8F1E2DBF7AEC44E5BCFBF0000000EEFF07EBFFFDFDFFFFF500000EFF9FDFFFFFFFF7FFF62000062CD7FDEFFFFDFDFE7BF700000EFFFFF2FFD18408F9300308F760000BDDFD3DFFF3FFBFF100020CCFFFFBDF795FFEFEFE6DFEEFB79FFFF5EBFFE7FFF370000ED000000000000000000000082000000CABFEB6EF1000000000000000001184000000030000EFFDFDFFEF3FFFDF000000000E7B00000000000000000000002000000CDEFF70000000000002000000C64FFEC697000000002000000C39FFFFCFE000!00000E0400BEFF3F7FFFF7CFF00820E000000C1F1EF710000000000000000000000000000000000000000000000000000000000000000000000000000000000000E100000CBFFF7F317FFF0000820000008F17DACFF7F9000000EFFFFFFBB3FFBF200000000006E7FFFF7FF7FFFD6000000000EFF000000000000000000000E2000000CE7D1FF3FEF73B7DE020008BFC9E7FFFF1000000000200007CF3F4EFFF3EB7700000EF300000000000000000000000000000000000000000000000EFFFFFDAFF5FF1FF76FDF6FFEE37CC7E71EFDF7F08D1FEFFD0EFFFFFF1E20000888546335A00000000000000000000000000EF10000000000000000000400EFD97FEFF7FFEEFF6EFFFFFEF0000000000000000000000000EFFEF06000010000000EDFF70EFF373F00000000000000000020C2BDFFFBFDFFFF330000000EFEF3FF100000000000040000EDCF8BFF8FB0EFB76EE250000EBFF33C7D03E0000000000000EF92001000010000000000000EFFF7FF7FBFDFDFDFEF93FFFC208FA3FE5FFE5DFF710000000E0CFDFFFDFF7FFF7F60000000E0000FFFFFFFFCFFFF70000002647F7D7DFFFFEF6BDF110FA1E5AF8ECFF3FFF13FFFCFF7400EEF7F3FE37FFAF39EFFFDB710EFFFFFF3200748027F9248F000000000000000000000000000EFFFFFBFFFFF7FFFFFFFFFFFFEFFFFFFFFFF!F7CC1057C9F100000000000000CFFFFFFFFFFFFEF2F100000000000000000000EFF3FF7CF56FF379FB9AFFCB7EFDD100000000000000000000E761EFFAF9FFEFBFBFFBEAD10ECE6BFFFFBEFDD7D3BF5FBF2720CEFFAB5FFFEBF7FFF7BE00028FDDFFF9BB7BB7DFC6FFBBF3E9FFFBFFFDFB6FFF9BFF6F2AF2000002CE7FD7BD2F7FE6100060CEFFBBE7FFF73FEFFFBFF7020008ACF6B2DEFEFCF3DFF10020CF1FFBF771DCFF00000000000000000000000000000000000000000000000000000000000EF4F77FFFFFFFFFEFFFF00000AF786000000000000000003F76007FBF8FFEFBBFF7F7D00000208FAE7FDA777EFEFFE9F7008600007D7FF76FD37EFFB2000020000F4B77F99BADDF5F1300020000F6F7DFFF3F3000000000E424F536976BDDDFF6FFDAD78000000000000000000000000000000000000000000000000006FBF728EBDFEB7FFBBDF20000CF300E7000000000000000000EF57DDBFAFB1ECF7EEF76000020C2FBBE9EA79BBEE75A77000620A5F7F3DF39D5BD0DFFD7D36F000B1FB47327CF7FBB3000060000FEC5DFFEF10000000000EBC35EACE33D59FF2FBDEB7F06000000CA48B27100000000000000000000000000000000000EDDFD5FFFFAD4FEEFFFFEF300EF9D300000000000000000000208FEE6C00FB5FFF04E100000208FFCD43FABB600000000000E7!C5F7B4F1FFD73FFB7ADF20020000FFF965F000000000000061000978FDFB1000000000000E8CEF7BFB9BEFDD7083000000ED110BFFF3FF9ED9FDF000000E1CFFFEFF30000000000000006FFFF76EEEFFFA17BFFFFFFFFEFFBFF6FDFFFDFAF3DFFFFFFFE130000000000000000000000E3000000000000000000000086CF9BF7EFFFFFFFFFFFF704082FCFFF6C5FF7FFFBF5100000EEFFFFF7F7FBFAFBBFFFEBBD6A2C05000400004000000000000830000800000000000000000060CFE7DFFD77FFBFD760000002CDFD3BD1FEDF7EFBDFF3BB00EFFD006C100000000000000002100000CFFF1C5FF74FFE300820CFFFFFF77776DF9FFF0000060CFFFFDFFF8FFC5EFFFD7F002000F97FFBFF5FCFBF1000000EFCF0027FFB1CF1000000000061600FFFFFFBFFD31EF700C08E0CF777C78FFFFFC7CF7D100000C7A13F37F1AA3000000000064EFFEFDF31FF7DFAECFFFFF60000000000000000000000000EF1CFF7C7FFF7FFF7BFEF73F626C7DB764DBFBFFFE4ED1000020CEEFF9FDFFF5CFFBEF3000020CFEFFCFFBFFF7BAD300000020CB66FFECD1E89FFDF2E7FF0EF3ADF400904003EF20324841AEFDFFBFFFFFFFFFFFDFFFFFFEE7FFFDDFCFDFFFFFEFBCBDFF26CFBF7D7FFBFB336F7A19F08EFFFFFFFFFBFCA79DA0E5018F0000000000000000000000000E10000047DEFF3AEE6!2FEF7002000000CFFFED7FFFB70000002000000CFFBFB9FBE0000000020000FFF3716DE1F74E000000E20000049FDFDB49DED9FB00020000FF76E3EFB2DCFEF7000060000008FFFBCF3FF00000000208FF717C310DF7FFFFFFF73766F7FBFF7F6FFF7BFD389BFFF0000000000000000000000000000000000000000000000000060000FDF7FFFBEFAEFEFFFE7FEF1000000000000000000000020000D3DBFFFFFF39D79FC30020000D75ECFFD99EEE1FB1700E100000CF1F97EFEF76000000000000000001000000000000820C67D78DFFCFFCEF3DFF3000EFF0000000000000000000000E289700C3B9DDCFF34CDFF00862000FF6FF1BFC01000000000602000002DB523BE55288500020000002D1F459B1FC30D4F00AFFFFDFEF73F7FFFD3F9767106FDF2FFFFE7F0FB7FFFFAFF20EF7420000000000000000040C00000000000000000000000004C5FFBFCB9FFF75FDCBF9EDF2E7EBE00000000001000000000EFFFFFF00000C0000009C60000000000000000000000000000EDBFFDFB5D70E000000000000EE7D7FD9FFFDFD00000000000AFFDF7734748034080808401820C731B1FDF4BEFF8EBFF000020000004ECD6EFFF7F300000020000004BFF7BBDFD50000000E373FF7F8F6AEFFFED7000000EFF5ED99F6DDED90000000000EFFFFBCDB10080B100870000020000004F!37FAF3B10000000060407DAD1FEFFDF1D5E5FBC1063CFDF76008F57BBF5FDA300020000EEBDEDFF6FBBEF0000002000000CFFBBB37610000000020000008CF75EEAFBF0F50000220000000000C700000000000EB7CFD6FEF7F5CFEFF3000000EFF7FD9DFF4B0D20109DF00040000000000000000000000000AB6EFFFFBFFFFDE7FDD770000EF6EFF1F6FFF5F7FF00000000EF753FDB1041000000108010060000FDBF7F9BFCFF2000000060CEFDFFFFFEBBFBFF7FBFF10E04FFFF9F7FF7FDF7EFDF000060000DF7FFFF7DFBDEFF7000020000FFF7FF70000000000000A0000FAFF4FFFF3FEF000040020C5F3000000000000000000020000008EBBEFF7BE00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000E7E0000C3FFDEFFDFFBDFFC51000FFEFF08F6F7FFF4DFEF68000000000000000000000000006040000008F70FFB18F330CF2E10007CAE7EFFFFFFE0000000EFFFFFFBF6F5FDDD0E87F7A356EA9100E7EDFDFFF3300000000DCDFFE77F99FFBFB6F7FFB10EFFFFFE8A30000000048810F50000000000000000000000000ED30DABF7F9FF94BFFBC50000EFFBC106E700B71E781000000E8F3CFE6F6FAE69B57BB758D0!EC281DFF69F6DEFFEBA600000AFECFCFFFCBD7DFFEBFAB60030000000000000000000000000E083000C8CDDEFFFECC7000002080077D00DBF7CCF7100000060000DE9FF7F8FE7700000000E0660F02B2F7E3F77F300000CEFCF7FFFBFFF9BEBFFFDFF308EFDFFFFFFBF26371F7ADF0608EFEDBEA800000000000000080A0CF97FFBD7CDFC1F77BAFED1E1CFB6FFFFFBEEF7FEFD40000A00BBFFF7FD74E37FBBFAB00060CAB85FFD73DEEF479ADE000E18DF687FF7BA37777610000060810BD7FEFE8F79200000000E716FAD5F7BFF9F0000000000EFFFFFDF75FBFFFEFFFED76FFEFFFFFFFDBFFFFBDB63E136EF00000000000000000000000002000000000010000000000000E0000024FFFD7ADB000128900EE000FCF7FFDFFF7FFB000000200005F11C6FDCCFD32000000E600F5DFEFED6EFF5D300000060000000D4F99BE1000000000200000083E4FE000000000000E7FF7FCFFFEFDFDFCDFFF3000EF5DB3CFF32E7BF00E10CD27000000000000000000000000002000000CBEDF6DCFF70000000E0000EEFD77FCDFFFF120000060000E2FE5DE2FFFAE5700000E100000C33BDC30000000000060000FDFE7BA5F93500000000610043FE7BD3FDB1FBB4B10006000000C9BFFBEBD7000000006CFEBFF7DEFFF77FA7C7FEF7EE74CFBF0D6FDFFEBF3307A200EFFDFFFFFFFFF34F!F4828F0C0A04B094009440EB99D7D7CFF7E9EF7F7BFFFDFFF70FFFFEFF3AB0000000000CCDFFF7000008EF18000C2FF9C7FFCF10FFB70EFF300000000CCFD7C3CD3F1C60000008FC1FFBD7200055000E108006FFFEDB4DF53FF10400EFFFFF09E9B1FFCBFD264D1FFEFFFFFDEF318580E100E728BFEF1000E56C3FFBCEFD30000086EF0000000008529FE7A13300600000DEBBC77FEFFFD1AFC00E392CDFD6EFF7DF2FE6E0000F00000EDFFFF3FF33FCAF7FE30EFDD10000000000000000008EE1000BB953633FF6FEF6DF500E7300004FBB2EA60000000000EFFFFFFFFFFFFFFFFFFFFFFFF0000000000000000000000000EF7BFFFFE7F3BFFEEFFA79F3EE70851E9FFFFFEFDBBFFFFB2965CFED4FF3FFFFFFFFEFFFB73EFDDF70CFD6EEF0F79F6FFD16008FBCFE18F0C8BF340000000EF72016000000000000000000EE10000CEFF34BBEFFBD70000E30000000000C42F9FFFCFFE0EFFFBEF308F76FFFFEF10000069BFFE3CB0FFDD9F100000000EFFFFFFFFFFFFFFF778C9FBFF00000004BDF9EBEF1FFBFBE20EFFFF0C00000000BEFF710000E3001F408F9AF3EFF7BFD7FFFCFFFFFDFDDF7DFFFFEF3F0800E3FFFFFFFF6FFFFFFB7CFE300600000877FF8C3030C3AB700000000008F4FFF7BF000000000EAFFD6BAF9F787FE753EFF9F7EFF4FB1000000089FDD47F20020000FF!EFFBBD9F200000000000871D7E0DCFCAF9DE3FF0000EFFF181400000320100000C28E7D1F3E7FFBF721FE00000000EF1000000000000000000000EEFD7C30CECF33DDF771FF63000000020CDFCEFF00000000000EFDF7DFED7FFFF4341E7DFFFFEFCFF7FDE77F0000000000000EFC17FF9F700CC0F600000000EFFDFFFFFFA5FDBF7CE830CF8ED740E0300000000000000000E0008FF7FFDDBFBAEFEFF8106E1CF3FF5FFFFE20000000000020CD0F187CD7F70000000000022000FCBF3FFECE00000000006B7FFFFFF130000F500000000AFFFF76FF3FF6005040200082EFFF8FB66DFF6DECDA6369E1060000FFFF7BDBF2000000000020C90FFDBBFD930000000040060000BFFD000000000000000060CEDFFFF0000000000000000E7000FF00000000000000000060C90B7CBFFE7F70000000000000000000000000000000000000000000000000000000000000000000000000000000000000EE7000000000CFCB0C30E10000000000000000000000000400204FEEB200D1000000000000020400EBAD100000000000000020C50FCDF0F04000000000000204F1910000000000000000006328010CAFD5300000000000020CF7BAE1000000000000000020000008FF53000000000000060000C2EFFE30000000000000608700200000000000000000020CFDFD1000000000000000!00E7EF6BC7FFA7CF1E0857AE000E0875FB5F7FD7F00000000000E7080FBBE5FFFD0DF3000000020000D7CABFFBF000000000006FC9BDDFFFEBDF334E131C000EFBFBFFFB0010030000000108E7FF1EF4FBFFFF77FF73001000000000000000000000000000EDDE7FFFFFFDEFFFFFFDDFFF6EFFFFFFDFFFBD3602C001160820000000000100000000000006FFFFFFFFF300000000000000EB3D00000000000000000000020CB5FEFFB500000000000000E7000FDFFFDFFF3DB0000000020CFFF7FFFFFEFFFFCF9F1C0000000000000000000000000000000000000000000000000000EFFFFFFFFFB7DE0FFFFF5E00E0000000000000000000000000EFDFBFFDFB6FFFFFDBFEFBE19EFF34FFBFFFFFFFFFAFF7A40000000000000000000000000006000000CF9FFD700000000000EEFEFFFFFFB75F040C0000000EFFFFF7DFFFFF500008200000000000000000000000000000060810DFFFF700000000000000E1887F3FFDDF070000000000000000000000000000000000000000000000000000000000000E3070FFEFFF08000000000000E78300CEFFF70000000000000CFDD0100841B201000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000006FFF7FFBD7FFB3BF151080000EFFFFF38000006!4000412BCFF200000000000000000000000062CBBFE100080020308000000604BF30000000802000000008F7EFFFBFBE93B775FAEFF0050000000000000000000000000020CF4C1FF1DE630000000000020000BBF100000000000000002000000CABF00000000000000E7B10FFF7DFFBF5E201000000E00FDF74F100000000000000060C2030C10714F1DB1000000020C57A340FFFFF0000000000020CCF6DEEB20000000000000000CFFBFFFDFF7EFFEFFFFF7800000000000000000000000000000000000000000000000000000000000000000000000000002200000000000000000000000AFFFFFFF39FE3300040000000EDFFFFFFFEEFFF991422128A8000000000000000000000000020CCF00000000000000000000CFFFCFFFFD5FF100000000001EFEB0900200000200400000880000000000000000000000000E787020000000000000000000E0080F7DF00000000000800000000000000000000000000000EFFFFFFFFFFFFFFFFFFFFDBEBE00000000000000000400200060E30F1C387AFC0FF4D610CF1EFF3000000000000000000006EFF0000000000000000000000EF70000000000000000000000EF710F1C3E10C1872C0010000EDCF3B0870DF3000000000000EF7E0408F05F12000000001000000000000000000000000000ACE13B3C00F4C000400200000EF7C7!A7A0000000000804FB00EF74067000000000000000000EFF397FFE2F7CFFFB813F71F9EFF318400100000004004008BEF500E4000000000000000000EFFD70000000000000000000064FC5C1EFABF7DDEFFB517FBEE100000000200000000000009E6FF3208F0EFD00C000F184008936CFF72F14ADF3FFDCC7D31EFFFFFDFFFDA6FCFFFFF4DCFFE0C00DECF9F1EFFFD5E7F70102BFF7FF3FF7E45F87BF747A7EEFDF499000000000000000000EF3EF60C1987BF3E9C10EFEB9CF4DD9F7000CFFFF00FF93D76EFF1000180000000000070008EF9D3E28F7F740BFFF0EFE0080000000000000000000000000EFEF1F1CF0F1000FF00000068EEF0730400104001000000408EF59DFFCDEFFED7FEDDFDE810EFB30000000000000000FF700EE710FD7F7FFFF7B300080000634FFCDFD9E7EF5F000025C19EFFF5FFDEEFFF737F7A000000EFFD77F3003B488738FF55C7000000000000100000000001006E190FF530D14E05000000000C2CDFFFF8EF3008FFC409C70DEFFDFFFFFFFFCFBFFF92001008FB00006DB9F77FE8B6EDFFEFEF70000000000000000000000EF300000000000000000000026CFFFFFFAFFFFBFFFFFFBEF9FEFFFEBF000802087400000000EF71010000000000000000000EF10010000004000000000000A10F0DFF2E7EFF397CFF0000CEFFFEB5CBFE7CF1FFCA30!0000E4FFFFFFFF70DFF31C3000C71EFFB8FFF737EFF60820000407AFEFFFF7E7FFDF7BF7FDFE702EF79FFFFBBE7F7FF7F317E572E084C2F957DEFFDDBF200A280EFFFFBFFFF77DFDFFFFFFFFFFA02F275181EC00CC20010000C000FFFFFF7F08321F02008048A04FFDFFDFFFFEFFEFFFFD930ECFEBFBBFE3DFFFEBFED1FFFBAFFFFFF7F3F6DD77000000000EF1FE310000080E5DA858100204628E650788F004B5FF0D100E0000000000000000000004000E99670ECFF7BF600080100CFEFF92ED3C0C9302FFFFFED004AF7FF7EF7EFFFFBFFFFFF7FBBEFFFFFF58FFF76EF00000000400CFFEEF10F1C1010400000006FFEFFDFFF73FE7DBD8710000ECFFFEBDF7BF3000000000000EFFFFFBF35508ED0CB10F4C7DEFF702FEADEFD9E7FFF7B7F51EFFFB9FFDFFDFFFFF73F7DFFFE1000DFDFFB10000000000000E1000FF4FF30000000000000068F1070CFF30C7D30000000002047EBFDEFF750B6B7F3D7000EFF6610000000000000000000EFFF8FFBFFFECFFE3D30C0030EFFDF67A75079C435D4623D8CAF100FFDBFBBFBFFDCFFBDDF383CD0FFFBFD7FFFFFDF7FF5F7EFBB561C2CCF4801F8022003EEFB315DDBF7D5DFF7F9BFFFBBEFE7BF7B9EFBF00FFDF0CF0000000000000000000000000000688FBEDF73BFFBF5000000000E9300F66FFFDD300000000000A7BFFFEE7B10!0000000000000EBEEFEFFEFBDFFFBFE6932000EFFFF9110000815100000000860000FD7B7AF73BF080000000E000000000000000000000000EFFFFF10001CFFFAFBFFBFF08CFFF87FDFFBBFFFFFFDFFFFF2EFF51FF67FFFFFFFFFFFF7F00E08003F6B1304000000000000A000CEE300004";

stateRange = "00215NH00544NY00795PR00851VI00988PR02791MA02940RI03897NH04992ME05495VT05544MA05907VT06389CT06390NY06928CT08989NJ09899AE14925NY19640PA19980DE20099DC20199VA20599DC21930MD24658VA26886WV28909NC29945SC31999GA33994FL34099AA34997FL36925AL38589TN39776MS39901GA42788KY45999OH47997IN49971MI52809IA54990WI56763MN57799SD58856ND59937MT62999IL65899MO67954KS69367NE71497LA72959AR73199OK73344TX74966OK79999TX81658CO83128WY83422ID83422WY83888ID84791UT86556AZ88441NM88595TX89883NV96162CA96698AP96797HI96799AS96898HI96932GU96940PW96944FM96952MP96970MH97920OR99403WA99950AK";

function getState(zip) {
	if ((parseInt(zipString.substr(zip / 4, 1), 16) & Math.pow(2, zip % 4)) && (zip.length == 5))
		for (var i = 0; i < stateRange.length; i += 7)
			if (zip <= 1 * stateRange.substr(i, 5))
				return stateRange.substr(i + 5, 2);
	return null;
}


/*
Given a zip code as a string or number, find the state
*/

function setState(txtZip, optionBox) {
	if (parseInt(txtZip) < 0 && txtZip.length != 5 || isNaN(txtZip / 4)) {
		optionBox.options[0].selected = true;
		alert("Please enter a 5 digit, numeric zip code.");
		return;
	}
	
	var state = getState(txtZip);
	if (!state)
		return;
		
	if (optionBox.type == "text") {
		// If the state option box is a text field, just set the value
		optionBox.value = state;
	} else {
		// If the state option box is a popup menu, change the selected value
		// Find popup menus and set the state
		for (var i = 0; i < optionBox.options.length; i++)
			if (optionBox.options[i].value == state)
				return optionBox.options[i].selected = true;
				
		for (var i = 0; i < optionBox.options.length; i++)
			if (optionBox.options[i].value == "XX")
				return optionBox.options[i].selected = true;
	}
}


// AdminAjax (table,id,values)
// Save a values to a table
// Values should be an object, {field : value,...}
function AdminAjax (cmd,table,id,values) {
	var data = {
		'cmd'			: cmd,
		'id'			: id,
		'table'			: table,
		'values'		: values
	};
	var dataString = JSON.stringify(data);
	
	$.post('ajax_admin.php',  {data: dataString}, function(res) {
		var result = JSON.parse(res);
		//alert (result);
	}, "text");
}

// Remove "px" from a value. Sometimes a value, e.g width = 10px, comes through
// with the "px" still on the value.
function StripPx (s) {
	var q = String(s);
	//debug(true, "1) "+q);
	var result = q.replace("px"," ");
	//debug(true, "2) "+result);
	return result;
}

