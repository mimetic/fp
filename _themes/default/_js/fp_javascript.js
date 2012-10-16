// ----------------
// FP JAVASCRIPTS


// ----------------
// jQuery initialization scripts
$(document).ready( function() {

	// Disable context menu everywhere in the main CSS Container in the page
	$("#container").noContext();
	// Disable context menu for the big popup picture
	$("#popup_img").noContext();		
	// show obfuscated email addresses which have class=obfuscate
	$(".obfuscated").defuscate();
	
	/*
	// ------------------
	// Make all price-info id tags unique, if the PHP didn't do so already.
	// This happens when the price info stuff is generated twice, once for big pix and once for small pix...I think
	// This seems clumsy...
	$('*[id^=priceinfo_]').filter('*[id$=_buyprint]').each(function(n){
		this.id = "x"+n+this.id;
	}).end().filter('*[id*=_text]').each(function(n){
		this.id = "x"+n+this.id;
	});
	*/
	
	
	// =================================================
	// Dropdown info boxes
	// e.g. show limited edition and size info in a gallery
	$(".showtext").hover (function (event) {
		if (!event) event = window.event;
		var target = (event.target) ? event.target : event.srcElement;
		var myID = '#' + this.id + "_text";
		$(myID).slideDown('fast');
	}, function (event) {
		// prevent event propogation...
		//if (!event) event = window.event;
		//var target = (event.target) ? event.target : event.srcElement;
		var myID = '#' + this.id + "_text";
		$(myID).slideUp('fast');		
	});
	

	// =================================================
	// Buy now link to frameshop
	// THIS DOESN'T WORK WITH CSSEDIT...HOW TO FIX? 
	// Problem is, CssEdit doesn't disable a javascript method attached to the objects, e.g. click
	// It could, but how to reattach? 
	// We don't want to check for CSSEdit in any function we build!
	
	$('*[ID^=buynow]').click(function() {
		var params = $(this).attr('rel').split(",");
		var obj = {
			imageID		:  params[0],
			projectID	:  params[1],
			currentframe	:  params[2],
			currentmatte	:  params[3],
			currentsize	:  params[4]
		}
		GoToPrintShop(obj);
	});
	
	
/*
	================================================================
	RESIZER FOR RESIZING BACKGROUND IMAGES UPON LOAD
	================================================================
*/

	// ---------
	// Set resizer function and initial params for resizing background picture 
	$('img#pictureBehind').load(function() {
		// Get the proportions of the background image, and save as attribute
		$(this).attr('originalWidth', $(this).width() );
		$(this).attr('originalHeight', $(this).height() );
		$(this).attr('proportions', $(this).width() / $(this).height() );
		myID = '#' + $(this).attr('id');
		//alert (url + " : " + $(this).height() + ", " + $(this).width() );
		//margin = {top:0,bottom:0,left:0,right:0};
		margin=false;
		ResizeBackgroundPicture(myID,margin,FP_BKGD_RESIZE_NOCROP);
		$(this).fadeTo(250,1);
	});
				


/*
	================================================================
	RANDOM BACKGROUND IMAGE (cacheable version)
	================================================================
*/
	if (FP_SET_BKGD_PICTURE) {
		url = getRandomImage(FP_RANDOM_IMG_LIST);
		css = { src : url };

		// Set body background if the img#pictureBehind is not in use.
		if (!$('img#pictureBehind').length) {
			// Load function for bkgd image cache
			// if FP_RESIZE_TO_BKGD_IMAGE is set, resize the window to fit.
			var imageloader = new Image();
			
			$(imageloader).load(function() {
				if (FP_RESIZE_TO_BKGD_IMAGE) {
					// Get the size of the background image
					var w = $(this).attr('width');
					var h = $(this).attr('height');
					
					// Min width is 600px
					if (parseInt(w) < 600)
						w = 600;
					
					// Min height is 500px;
					if (parseInt(h) < 500)
						h = 500;
					
					window.resizeTo (w,h);
					
					dw = w - $(window).width();
					dh = h - $(window).height();

					window.resizeTo (w,h+dh);
				}
				setBackground(url);
			}).attr(css);
		} else {
			// LEGACY:
			// Set picture in the pictureBehind image, used for resizing backgrounds, etc.
			$('img#pictureBehind').hide().attr(css);
		}
	}

	
/*
	================================================================
	Hide/Show Captions
	================================================================

	- show_info = 1, popup caption when mouse passes over the image
	- show_info = 2, show caption (does not hide)
	- show_info = 3, do not show caption
	When we show a caption, we resize any 'flexible' pictures
	to accomodate the caption on the screen. Be sure animation is 'false'
	in the resizeToWindow call or the width of the caption gets screwed up!
	'overlay' means don't slide down, but overlay the caption on the picture
	//$('#error').html("captions="+showCaptions);
*/
		
	switch ( showCaptions ) {
	// POPUP
	case '1': 
		$("*[id^=gallery_picture]").hover(function (event) {
			myID = '#' + this.id + "_text";
			w = $('#'+this.id).width();
			h = $('#'+this.id).height();
			$(myID).width(w).show();
			// make overlay captions fit
			//$(myID+" .captionblock.overlay").width(w).height(h);
		}, function (event) {
			myID = '#' + this.id + "_text";
			$(myID).fadeOut('fast');	
		});
		// Hide caption below picture
		$('*[id^=gallery_picture] .showinfo').hide();
		break;
	// SHOW
	case '2':
		$("*[id^=gallery_picture]").each(function (event) {
			myID = '#' + this.id + "_text";
			w = $('#'+this.id).width();
			$(myID).width(w).show();
			// Set padding of overlaid caption to match picture padding (i.e. matting)
		});
		$('*[id^=gallery_picture] .popupinfo').hide();
		$('*[id^=gallery_picture] .showinfo').show();
		break;
	// HIDE CAPTIONS
	case '3':
		/*
		$("*[id^=gallery_picture]").each(function(){
			myID = '#' + this.id + "_text";
			w = $('#'+this.id).width();
			h = $('#'+this.id).height();
			$('.gallerypicinfo').show();
			$(myID).width(w).height(h).show();
		});
		*/
		$('*[id^=gallery_picture] .gallerypicinfo').hide();
		break;
	}


// =================================================
// For Flexible pictures...
	// Set maximum picture height to the actual height of the stored gallery pictures, PLUS the matting
	m = 1 + 1*FPMatteRatio + 1*FPMatteBottomRatio;
	var maxImageHeight = Math.floor(FP_GALLERY_MAX_PIC_HEIGHT * m);
	// Set maximum picture height to the actual height of the stored gallery pictures, PLUS the matting
	var maxImageWidth = FP_GALLERY_MAX_PIC_WIDTH + Math.floor(2 * FP_GALLERY_MAX_PIC_HEIGHT * FPMatteRatio);
	


// =================================================
//SETTING UP OUR FULLSCREEN, POPUP PICTURES IN GALLERIES
//0 means disabled; 1 means enabled;  
	var popupStatus = 0;
	var noFade = false;
	
	
	// This is now done by the jQuery plugin. Hmmm.

	// Add a popup functon to each picture
	// jQuery Lightbox Plugin : baluptin/dgross version
	$.Lightbox.construct({
				show_linkback:	false,
				ie6_support: true,
				ie6_upgrade: false,
				opacity: 1.0,
				download_link: false,
				padding: FPMatteRatio,
				padding_extra: FPMatteBottomRatio,
				default_caption_padding: 10,
				//speed:100,
				caption_separator: '<br>',
				show_info: showCaptions,
				show_button_nav: false,
				use_body_background_color : true
			});
	

	// ------------------------------------------
	// FLEXIBLE: Resize Tagged Pictures In The Project So It Fits The Window
	// Class tag is "flexible".
	//
	if ($('.flexible')) {
		// Animate resizing of pictures.
		var extraH = 0;
		var extraW = 0;
		FPAnimateResize = false; // (don't animate...too slow)
		
		// Disabled...causes problems with some styles. Should only be done with flexible styles?
		// Resize the gallery divider line, on the left of the actual pictures
		// $('#gallery_divider_line').height(maxImageHeight);
	
		// Leave space for the caption below the picture, if the flag for that is set.
		// Don't leave space if we're using popup captions on top of the picture
		// The theme snippet determines whether the caption pops up on top of the picture
		// or below.
		var captionHeight=0;
		if ((showCaptions != 1 && showCaptions != 3) && FP_SPACE_FOR_CAPTION) {
			$("*[id^=gallery_picture]").filter("*[id$=_text]").each( function (n) {
				if (captionHeight < $(this).height())
					captionHeight = $(this).height();
				//$('#message').append(this.id + ": " + captionHeight + ", ");
			});
			extraH = captionHeight;
		} else {
			extraH = 0;
		}
	
		// Get the proportions of the image and save it as an attribute of
		// each image, so resizing won't get thrown off by floating point errors.
		$('.flexible').each (function (n) 
		{
			thisImage = $(this).find('img.gallerypic');
			$(thisImage).attr('proportions', $(thisImage).width() / $(thisImage).height() );
			$(thisImage).attr('maxImageWidth', maxImageWidth );
			$(thisImage).attr('maxImageHeight', maxImageHeight );
		});
		
		// Get the proportions of the background image, and save as attribute
		if ($('#pictureBehind').size() > 0) {
			$('#pictureBehind').attr('originalWidth', $('#pictureBehind').width() );
			$('#pictureBehind').attr('originalHeight', $('#pictureBehind').height() );
			$('#pictureBehind').attr('proportions', $('#pictureBehind').width() / $('#pictureBehind').height() );
		}
		
		// If the window resizes, act appropriately
		$(window).resize(function ()
		{	// The window has been resized
			wWidth  = $(window).width();
			wHeight = $(window).height();
			screenWRatio = .98;
			screenHRatio = .98;
			var maxWidth  = (wWidth*screenWRatio) - $('.gallery-description').width() - extraW;
			containerPadding = $('#container').css('padding-top');
			if (containerPadding)
				containerPadding = containerPadding.replace('px','');
			// Note, we subtract 3 x footer because our floating footer has negative top margin equal to its height.
			var maxHeight = Math.floor((wHeight*screenHRatio) - containerPadding - $('#topper').height() - (3*$('#foot').height()) - extraH);
			// Abitrary max/min sizes
			if (maxWidth < 500)
				maxWidth = 500;
			if (maxHeight < 200)
				maxHeight = 200;
		
			// debug:
			//$('#error').html('<div style="z-index:200;left:20px;top:00px;border:1px solid blue;position:fixed;height:'+maxHeight+'px;width:100px">test</div>');
			$('.flexible').each
			(function (n) {
				resizeToWindow( $(this), 'resized', maxWidth, maxHeight, FPAnimateResize);
			});
	
			//---------------
			// Resize background picture when screen resized
			// ID=pictureBehind
			// Resize proportionally, crop to fit
			// margin = {top:0,bottom:0,left:0,right:0};
			margin = false;
			ResizeBackgroundPicture('img#pictureBehind',margin, FP_BKGD_RESIZE_NOCROP);
			
			
			if ( $('img#pictureBehind').size() == "ego") {
				//$('#error').html('h='+h+", w="+w+", r="+pictureBehindR+", WinW="+wWidth+", WinH="+wHeight);
			}
			
			//-------------
			// Reposition any centered floating stuff
			$('.centered-in-window').each(function() {
				CenterInWindow($(this));
			});
			
		});
	} else {
		$(window).resize(function ()
			{
			//-------------
			// Reposition any centered floating stuff
			$('.centered-in-window').each(function() {
				CenterInWindow($(this));
			});
		});	
	}



	//margin=false;
	//$('img #pictureBehind').load(ResizeBackgroundPicture,margin,FP_BKGD_RESIZE_NOCROP);
	//ResizeBackgroundPicture();
	$(window).trigger('resize');

// =================================================
// Exhibit pictures
// These are images that are displayed in a single picture on the page.
	if (  $.fn.exhibit ) {
		if (FPSHOWMATTED) {
			exhimageborder = FP_EXHIBIT_IMAGEBORDER;
			exhborder = FP_EXHIBITBORDER;
		} else {
			exhimageborder = 0;
			exhborder = 0;
		}

		var options = {
			cur:			FP_CURRENT_IMAGE_INDEX,		// current item in page index of images to show
			stage:		'#exhibit',			// selector of the stage element ... where to show the pictures
			lclass:     'exh-loading',      //class to be applied to stage while loading image
			direction:  'center',             //direction that exhibit-box opens, can be "left" or "right"
			toolsize:	'small',			// use large tools (alt is "small" or "medium")
			duration:   250,                //duration of transitional effect when enlarging or closing the box
			opacity:    0.7,                //opacity of navigational overlay
			countpos:   'below',          //position of image-counter - can be false, "overlay" or "caption"
			caption:    'description',				//display caption based on title attribute ('title') or a div inside the <li>
			showcaption:    showCaptions,		// caption: 1=popup, 2=show, 3=hide. 'showCaptions' is an FP global var
			showtools: 2,									// 1=popup, 2=show, 3=hide
			captiontarget: 'captionstage',	// id of where to show the caption; if empty, show below
			easing:     'swing',            //easing type, can be 'swing', 'linear' or any of jQuery Easing Plugin types (Plugin required)
			captionshowspeed:	150,			// transition speed to show/hide/animate the caption
			oflabel:    'of',               //label for image count text (e.g. 1 of 14)
			nlabel:     '',                 //label for next button
			plabel:     '',                 //label for previous button
			blabel:     '',                 //label for enlarge button
			clabel:     'Click to close',   //label for expanded stage (to hint closing)
			hpad: 570,						// width of non-picture stuff on page (not inc. "picture spacing")
			vpad: 200,						// Reduce pic by this amount when trying to fit to window 
			//resizing: FP_EXHIBIT_RESIZING,	// Don't use: we always use resizing; if you want fixed width/height, set it below
			fixedwidth: FP_EXHIBIT_FIXEDWIDTH,	// forced width of widest image in a set. No resizing.
			fixedheight: FP_EXHIBIT_FIXEDHEIGHT,	// forced height of tallest image in a set. No resizing.
			matteshow: FPSHOWMATTED,
			mattecolor: MATTECOLOR,
			mattescale: FPMatteRatio,		// Percent of picture given to matte, e.g. .25 = 25% of image is matte, 75% is picture.
			mattebottom: FPMatteBottomRatio,
			imageborder: exhimageborder,					// Border on edge of image, between matte and image
			imagebordercolor: FP_EXHIBIT_IMAGEBORDERCOLOR,
			border: exhborder,					// (set in Themevars.txt) Border (like a picture frame) around the picture (or matte, if matted)?
			bordercolor: FP_EXHIBIT_BORDERCOLOR,			// (set in Themevars.txt) border color around the picture (or matte, if matted)?
			lightbox: true,
			debug:      false
		}
		$('.exhibit-set').exhibit(options);
		
		$('.exhibit-link').click(function() {
			$('.exhibit-set.project'+$(this).attr('rel')).exhibit(options);
			//options.sourceselector = '.exhibit-set.project'+$(this).attr('rel');
			//$('#exhibit').exhibit(options);
		});
		
	}


	// -----------
	// MB Menu
	$(".navbar-container").buildMenu(
	{
		openOnRight		: false,
		openOnClick		: false,
		menuSelector		: ".menuContainer",
		hasImages		: false,
		fadeInTime		: 200,
		fadeOutTime		: 100,
		adjustLeft		: 2,
		adjustTop		: 10,
		opacity			: .95,
		shadow			: true,
		closeOnMouseOut	: true,
		closeAfter		: 100,
		minZindex		: 200,
		hoverIntent		: 0
	});

	// -----------
	// Embed media players
	$('.media').media({bgColor : 'transparent'});
	
	// ================================================================
	// Solidify:
	// If something has 'solid' class, set its background-color to the body bkgd color.
	if (FP_SOLIDIFY) {
		x = $('body').css('background-color');
		$('.solid').css('background-color',x);
	}
	
/*
	================================================================
	FRAMESHOP
	================================================================
*/
	
	if (FP_PAGE_TYPE == "frameshop")
		{
		// If we don't allow matte w/o frame, then hide matte when frame is not chosen
		// or show when frame is chosen
		if (!FP_SELL_MATTE_NO_FRAME) {
			$('#currentframe').change(function() {
				if ($(this).val() == 0) {
					// hide matte menu and set value to zero
					$('#currentmatte').val(0);
					$('#frameshop-mattelist').fadeTo(200,0);
				} else {
					// show matte menu
					$('#frameshop-mattelist').fadeTo(200,1);
				}
			});
		}
		
		// trigger the change checker
		$('#currentframe').change();
		
		$("#ui-dialog-masterprinter").dialog({
			position: ['center',80],
			width: 700,
			bgiframe: true,
			autoOpen: false,
			modal: true,
			buttons: {
				'Close': function() {
							$(this).dialog('close');
				}
			},
			close: function() {
			}
		});
	
		$('#ui-open-masterprinter').click(function() {
				$('#ui-dialog-masterprinter').dialog('open');
			});
	
	

		// activate the UI functionality of jQuery UI buttons
		$('.ui-button').hover(
				function(){ 
					$(this).addClass("ui-state-hover"); 
				},
				function(){ 
					$(this).removeClass("ui-state-hover"); 
				}
			).mousedown(function(){
				$(this).addClass("ui-state-active"); 
			})
			.mouseup(function(){
					$(this).removeClass("ui-state-active");
			});
				
	
		// Format currency INPUT fields (class="currency") as xx.xx
		$('.twodecimals').each(function() {
			FormatAsFloat(this);
		}).change(function() {
			FormatAsFloat(this);
		});
		
		// FRAMESHOP
		// Submit button
		$('form.order').submit(function() {
		return true;
			return FrameshopValidate();
		});
	
		UpdateDesc();
		UpdateSpec();
		}	// end 	#pagetype').val() == "frameshop"


	/*
		================================================================
		SHIPPING CALC DIALOG for Javascript cart
		================================================================
	*/
	if ( $('#shippingform').length > 0 )
	{
			
		$('#shipping-destPostalCode').change(function() {
			setState($(this).val(), document.getElementById("state"));
			//UpdateTotal();
		});
	
		$('#shipping-destCountry').change(function() {
			if ($(this).val() != "US")
				$('#ship_to_state_box').fadeOut(250);
			else 
				$('#ship_to_state_box').fadeIn(250);
		});
	
		$("#ups-shipping-calculator").dialog({
			position: ['center',150],
			bgiframe: true,
			autoOpen: false,
			modal: true,
			open: function(event,ui) {
				$('#shipping-destPostalCode').focus();
			},
			buttons: {
	/*			'Calculate Shipping': function() {
					UpdatePrices();
				},
	*/			'Save': function() {
	
					// What if the state doesn't update before the 'save' gets called?
					// Let's be sure, just in case the call doesn't happen.
					setState($('#shipping-destPostalCode').val(), document.getElementById("state"));
					var s = $("#state").val()
					$.UpdateCartShippingParams();	//update fpcart shipping params
					$(this).dialog('close');
				}
			},
			close: function() {
				UpdateTotal();		
			}
		});
		
		// activate edit shipping button in sales (frame shop)
		$('#ui-edit-shipping').live("click", function() {
				$('#ups-shipping-calculator').dialog('open');
			});
			
		// Pick up print option
		// Hide shipping and set shipping price to zero
		$('#pickup').change(function() {
			if ($('#pickup:checked').val() != null) {
				$(this).val('1');
				// hide shipping info
				$('#shippingform').hide();
				UpdateTotal();
				
			} else {
				$(this).val('0');
				$('#shippingform').show();
				UpdateTotal();
			}
			
		});
	
		// ========== COUPON DIALOG
		// Shipping: update price when an entry changes
		$('#couponform input').change(function() {
			ValidateCoupon();
			
	
		});
	
	
		$("#coupon-validator").dialog({
			position: ['center',150],
			bgiframe: true,
			autoOpen: false,
			modal: true,
			open: function(event,ui) {
				$('#couponcode').focus();
			},
			buttons: {
				'Clear' : function() {
					$('#couponcode').val('');
				},
				'OK': function() {
							$(this).dialog('close');
				}				
			},
			close: function() {
				if ($('#couponform input').val() != "")
				{
					UpdateTotal();
				} else {
					ValidateCoupon();
				}
			}
		});
		
		// activate edit coupon button in sales (frame shop)
		$('#ui-edit-coupon').live("click", function() {
				$('#coupon-validator').dialog('open');
			});

		// Update price when an entry changes
		//$('#shippingform:input').change(function() {
		$('#shippingform input').change(function() {
			UpdateTotal();
		});
		

	} // end if ups-shipping-calc exists
	

/*
	================================================================
	POPUP VIDEOS
	================================================================
*/
	/*
	$(".VideoPopup").click( function(event) {
		var i = $(this).attr('rel').split(",");
		var myID = i[0];
		var w = i[1];
		var h = i[2];
		$('#'+myID).clone().appendTo('#popup_video_content');
		CenterInWindow($("#popup_video_content"));
		$("#popup_video_player").fadeIn();
		return false;
	});
	
	$("#popup_video_close").click(function() {
		$("#popup_video_player").fadeOut("normal", function() {
			$('#popup_video_content').empty();
		});
		return false;
	});
	*/
 



/*
	================================================================
	SHADOWTILT: jquery plugin that adds shadows and tilt to anything.
	See http://thatgrafix.com/shadowTilt/
	
	Currently only used in Print Shop. 
	================================================================
*/
	$(".shadow").shadowTilt({
		padding : '0px', // width of padding around image
		borderThickness : "0px", // border thickness around image
		borderType : 'none', // border type eg. dashed, dotted, solid
		borderColor : 'transparent', // border color
		offset_1 : "5px", // off shadow offsets
		offset_2 : "5px", // off shadow offsets
		offset_3 : "5px", // off shadow offsets
		offset_1_hover : "5px", // hover shadow offsets
		offset_2_hover : "5px", // hover shadow offsets
		offset_3_hover : "5px", // hover shadow offsets
		//shadowColor : '#000', // shadow color
		shadowColor : 'rgba(0, 0, 0, 0.3)', // shadow color (rgba allows for transparency)
		fadeOut : "fast", // speed of fade out
		fadeIn : "fast", // speed of fade in
		startOpac : "1", // beginning opacity
		endOpac : "1", // ending opacity
		enableTilt : false, // turn tilt on/off - true, false
		enableFade : false, // turn fade on/off - true, false
		tilt : "0", // default degrees of tilt
		random : false, // randomize tilt
		randomMax : 20 // largest degree for random


	});

	
/*
	================================================================
	FPCART SHOPPING CART
	================================================================
*/
	if ( $.fn.fpcart ) {
		var options = {
			finishing_name :	'Shop',		// Name of the link to the finishing page, e.g. Frame Shop
			debug:				true
		}
		$('form.fpcart').fpcart(options);
		
		// Add collapsing to the cart
		// The text includes open/closed triangles based on jquery UI.
		// Cart is initially hidden
		//$('#fpcart').hide();
		
		// Cart compacted/expanded view switch
		$('#cart_compact_switch').click(function() {
			if ($(this).is(':checked'))
			{
				$("#fpcart_cart_compact").val('1');
			} else {
				$("#fpcart_cart_compact").val('0');
			}
			$.updateCartOnPage();
		});
		
		// Add actions to the quantity field
		$('#currentquantity, #currentframe, #currentmatte, #currentglazing, #currentpaper, #currentsize').change(function(){
			UpdateTotal();
			var currentfilepath = $('#currentfilepath').val();
			var pictureframespath = $('#pictureframespath').val();
			var previewwidth = $('#previewwidth').val();
			var previewheight = $('#previewheight').val();
			var frameshoprefsize = $('#frameshoprefsize').val();
			
			// Update which finishing to show
			ShowFinishingBasedOnSize();
			
			UpdatePicture(currentfilepath, pictureframespath, previewwidth, previewheight, frameshoprefsize);
			UpdateDesc();
			UpdateSpec();
		});		
		
		var openHTML = '<span id="fpcart_collapser_icon" class="ui-state-default ui-corner-all ui-icon ui-icon-triangle-1-e" style="float:left;"></span><span style="margin-left:10px;">Shopping Cart</span>';
		var collapsedHTML = '<span id="fpcart_collapser_icon" class="ui-state-default ui-corner-all ui-icon ui-icon-triangle-1-s" style="float:left;"></span><span style="margin-left:10px;">Shopping Cart</span>';
		$('#fpcart_collapser').collapser(
			{
			target: '.fpcart_collapsable',
			targetOnly: null,
			effect: 'slide',
			changeText: true,
			expandHtml: openHTML,
			collapseHtml: collapsedHTML,
			expandClass: '',
			collapseClass:''
			});
	}
	
	// This shows finishing (framing, etc.) which is hidden until now.
	ShowFinishingBasedOnSize();
			
	$('#wallcolor').change(function(){
		changeWallColor ();
	});



/*
	================================================================
	APPLY JQUERY ACCORDION MENU
	================================================================
*/
	$( "#accordion" ).accordion();

    
/*
	================================================================
	APPLY JQUERY UI TO STUFF (SO IT SHOWS)
	================================================================
*/
	// Only apply to marked buttons! Otherwise, throws off buttons of my own design.
	// Probably a bad idea, but how else to have my own buttons that don't get screwed up?
	$('.ui-button').button();


/*
	================================================================
	DEALER/CONSULTANT LIGHTBOX
	================================================================
*/
/*
	// Is the lightbox cookie set?
	var cookieName = "fpLightbox";
	var c = $.cookies.get(cookieName);
	var myCookie;

	if (c) {
		myCookie = JSON.parse(c);
	}
	
	if (myCookie && myCookie.status == "edit") {
		$('#message, #error').appendTo('BODY').addClass('fp-lightbox');
	}
	
	$('.fp-lightbox').append("CHECK OFF STUFF");

*/



/*
	================================================================
	Scrollable screen.
	================================================================
*/
	
	if(typeof $().scrollable == 'function') {
		$('#scrollable').scrollable(
			{
			//circular: true,
			}
		);
	}



/*
	================================================================
	POSITION ANY IMAGE IN A GALLERY, ANYWHERE ON THE WALL.
	
	ui object in resizable:
	ui.originalPosition - {top, left} before resizing started
	ui.originalSize - {width, height} before resizing started
	ui.position - {top, left} current position
	ui.size - {width, height} current size

	================================================================
*/
	/*
	Resizable items:
	- If they have settings from being resized by the user, then use JS to 
	redraw them with those settings. Rather than store all the calculations and
	settings resizing/positioning created, we take the basic resize and redraw it.
	
	Get top, left, height, width: these are the values set by resize/position.
	The "original" values are set as non-standard attributes.
	*/


	/*
	We store the object positions in the project DB, all in one
	parameter, #09, also known as "FP_PARAM_GALLERY_OBJ_POSITIONS".
	
	TESTING: PUT IT INTO 'STATEMENT'
	*/
	function UpdateSizePosition(imageID, ui) {
		
		var table = "Projects";
		var id = FP_PROJECT_ID;
		var values = {};

		try 
		{ values.originalTop = Math.floor(ui.originalPosition.top); }
			
		catch(err){}
		try { values.originalLeft = Math.floor(ui.originalPosition.left); }
		catch(err){}
		try { values.originalSizeWidth = Math.floor(ui.originalSize.width); }
		catch(err){}
		try { values.originalSizeHeight = Math.floor(ui.originalSize.height); }
		catch(err){}
		try { values.positionTop = Math.floor(ui.position.top); }
		catch(err){}
		try { values.positionLeft = Math.floor(ui.position.left); }
		catch(err){}
		try { values.originalTop = Math.floor(ui.originalPosition.top); }
		catch(err){}
		try { values.height = Math.floor(ui.size.height); }
		catch(err){}
		
		values.id = imageID;
		
		/*
		var values = {
			id:			imageID
			originalTop: 		Math.floor(ui.originalPosition.top),
			originalLeft:		Math.floor(ui.originalPosition.left),
			originalSizeWidth:	Math.floor(ui.originalSize.width),
			originalSizeHeight:	Math.floor(ui.originalSize.height),
			positionTop:		Math.floor(ui.position.top),
			positionLeft:		Math.floor(ui.position.left),
			width:			Math.floor(ui.size.width),
			height:			Math.floor(ui.size.height)
		}
		*/
		var cmd = "update_project_picture_settings";
		AdminAjax (cmd, table,id,values);
		alert ("Saved image "+imageID+", for project ID = "+id);
	}



	$(".resizable").resizable({
			aspectRatio: true,
			grid: 5,
			minHeight: 20,
			minWidth: 20,
			handles: 'se',
			resize: function (event, ui){
				// function to resize one picture
				// function resizeOneGalleryPicture ( obj, animate, resizeHeight, pictureclass, captionclass)
				//debug(true, ui.);
				resizeOneGalleryPicture( $(this), ui );
			},
			stop: function (event, ui) {
				var image = $(this).find('img.gallerypic');
				var imageID = image.attr('id').replace(/Image_/i, "");
				UpdateSizePosition(imageID, ui);
			}
		}).each(function() {
		
		var settings = {
				position: {top: $(this).css('top'),
				left:$(this).css('left')
				},
			size:	{
				width: $(this).css('width'),
				height: $(this).css('height')
				},
			originalPosition:
				{
				top: $(this).attr('originalPositionTop'),
				left: $(this).attr('originalPositionLeft')
				},
			originalSize: 
				{
				width:	$(this).attr('originalSizeWidth'),
				height:	$(this).attr('originalSizeHeight')
				}
			
			};
		resizeOneGalleryPicture( $(this), settings );
	});


	/*
	
	$(".positionable").draggable({
			containment: '#scrollable',
			scrollSensitivity: 100,
			//distance: 30,
			grid: [10, 10],
			scroll: true,
			cursor: 'crosshair',
			opacity: 0.75,
			stack: '.positionable',
			stop: function (event, ui) {
				var image = $(this).find('img.gallerypic');
				var imageID = image.attr('id').replace(/Image_/i, "");
				UpdateSizePosition(imageID, ui);
			}

		});
		
		*/



});