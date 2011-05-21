/*
 * jQuery Exhibit
 *
 * converts an HTML image list to a single picture display with prev/next
 *
 * Copyright (C) 2009 David Gross (dgross@mimetic.com)
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */
 
 /*
	exhibit (settings)
		Creates a new page element to show the wrapped set of encoded images.
		We encode images to prevent loading them, which would happen if we used "img", even if hidden.
		If the height/width of the images isn't set in the encoding, we use preset stage sizing.
		The height/width must set in the style of the source DIV, i.e. of class 'source'; other methods can't be read
		when the div is hidden, as it probably will be.
		
		
		Example:
		JAVASCRIPT:
		var options = {
			stage:	'#exhibit'			// selector of the stage element ... where to show the pictures
		}
		$('.exhibit-set').exhibit(options);
	
		HTML:
		<div id="#myshow">
			<div class="exhibit-image-entry">
				<div class="description">my HTML caption info</div>
				<div class="source" style="height:100px;width:200px;" title="My Title">path/to/hires.jpg</div>
			<div>
			...
		</div>
	
	
	Parameters:
		options:	(Object) An object has that specifies the settings for the Exhibit. See '$.fn.exhibit.defaults'
	
	Returns:
		The wrapped set.




 */
 
(function ($) {

	////////////////////////////////////////////////////////////////////////////
	//
	// $.fn.exhibit
	// exhibit definition
	//
	////////////////////////////////////////////////////////////////////////////
	$.fn.exhibit = function (options) {
	
		// set context vars
		//----------------------------------------------------------------------
		var obj = $(this);
		
		// build main options before element iteration
		// The defaults are defined below.
		//----------------------------------------------------------------------
		var opts = $.extend(
			defaults, 
			options||{}
		);
		
		////////////////////////////////////////////////////////////////////////
		//
		// -> start
		// apply exhibit to all calling instances
		//
		////////////////////////////////////////////////////////////////////////
		//return this.each(function(){
		if (true) {

			////////////////////////////////////////////////////////////////////
			//
			// initial setup
			//
			////////////////////////////////////////////////////////////////////
			
			// exhibit vars
			//------------------------------------------------------------------
			
			debug ('--------- BEGIN "EXHIBITION" PLUGIN ---------');
			
			// dispose of no-js warning
			if(opts.nojsclass) {
				$(opts.stage).find('.' + opts.nojsclass).remove();
			}
				
			// fixedwidth uses the resizing machine, so we turn it on.
			if (opts.fixedwidth) {
				opts.resizing = 1;
				if (!opts.fixedheight)
					opts.fixedheight = opts.fixedwidth;
			}
			debug('$.fn.exhibit -> There are '+obj.length+' sets of images.');
			
			// image preloaders
			var preloader = new Image();
			var preloaderNext = new Image();
			var preloaderPrev = new Image();
			var preloaders = [];
			var loadingtimer = 0;
		
			// html nodes
			var exhPlaceholder	 = $('<div class="exh-placeholder" />');
			var exhStageWrap 	= $('<div id="exh-stagewrap" class="exh-stagewrap" />');
			var exhStage			= $('<div class="exh-stage" />');
			if (opts.resizing) {
				var exhImage		= $('<img class="exh-image">');
			} else {
				var exhImage		= $('');
			}
			var exhTools			= $('<div class="exh-tools" />');
			var exhPrev			= $('<div class="exh-prev">' + opts.plabel + '</div>');
			var exhPrevNull			= $('<div class="exh-prev-null"></div>');
			var exhNext			= $('<div class="exh-next">' + opts.nlabel + '</div>');
			var exhNextNull			= $('<div class="exh-next-null"></div>');
			var exhSwitch			= $('<div class="exh-enlarge">' + opts.blabel + '</div>');
			var exhCount			= $('<div class="exh-count" />');
			var exhCur			= $('<em class="exh-cur" />');
			var exhTotal			= $('<em class="exh-total" />');
			var exhTextWrap			= $('<div class="exh-textwrap" />');
			var exhCap			= $('<div class="exh-cap" />');
			var exhText			= $('<span class="exh-text" />');
			
			// holds pictures after .load
			var exhBuffer		= $('<div id="exh-buffer"></div>');

			// declare CSS vars
			var cssCompactStage = {};
			var cssexhTools = {};
		
			// Calculate matte values
			matteRatio = opts.matteshow * opts.mattescale;
			matteBottomRatio = opts.matteshow * opts.mattebottom;
			
			// declare sizing vars
			var widestImage = {height:0,width:0};
			var tallestImage = {height:0,width:0};
			var smallestImgWidth = 10000;
			var smallestImgHeight = 10000;
			var largestImgWidth = 0;
			var largestImgHeight = 0;
			//matteWidth = 0;
			//matteWidthBottom = 0;

			// declare image object arrays
			var im	= {
				small: [],
				title: [],
				large: [],
				width: [],
				height: [],
				caption: [],
				id: [],
				captionid: []
			};
			
			// counter vars
			var cur = 0;						// array index of currently displayed image
			opts.cur ? cur = opts.cur : cur = 0;

			// total number of images in the wrapped set of sets
			var tot = obj.find(opts.imageElement).length;
			debug('$.fn.exhibit -> ' + tot + ' thumbnails found.');

			var togo = tot;

			debug('$.fn.exhibit -> Current image to show: ' + cur);
			
			// flag to indicate we called init, used below so we don't double call init after preloading
			var inited = false;
			
			// for figuring max image width/height
			var maxHP = 10000;
			var maxWP = 0;
			var p = 0;

			// Load image information
			obj.find(opts.imageElement).each(function(i){
				imgObj = $(this).find('.source');
				im.small[i] = imgObj.text();							// the thumbnail url
				im.title[i] = imgObj.attr('title');					// the image title
				im.width[i] = imgObj.css('width').replace('px','');	// the image width
				im.height[i] = imgObj.css('height').replace('px','');	// the image height
				
				im.id[i] = this.id;
				
				debug('$.fn.exhibit -> Loading "' + im.small[i] + '",  title:"' + im.title[i] + '", w:"' + im.width[i] + '", h:"' + im.height[i] + ', cap id ('+opts.caption+'): ');

				
				debug('$.fn.exhibit -> Loading "' + im.small[i] + '",  title:"' + im.title[i] + '", w:"' + im.width[i] + '", h:"' + im.height[i] + '"');
				debug('$.fn.exhibit -> id "' + im.id[i]);
				

				capObj = $(this).find('.'+opts.caption);
				im.captionid[i] = capObj.attr('id');

				if (opts.caption == "title") {
					im.caption[i] = im.title[i]
				} else {
					im.caption[i] = capObj.html();
				}

				// widest picture has largest p (width/height)
				// tallest picture has smallest p
				p = im.width[i]/im.height[i];
				
				if(maxWP < p) {
					maxWP = p;
					widestImage.width = im.width[i];
					widestImage.height = im.height[i];
				}
				// tallest picture
				if(maxHP > p) {
					maxHP = p;
					tallestImage.width = im.width[i];
					tallestImage.height = im.height[i];
				}


			});	//each

			// Build the stage, show the picture.
			init();
		//});
		}
		return this;
		
		// ----- END MAIN ------
		// ----------------------------------------------------------------------
		
		
		// -------- LOCAL UTILITY FUNCTIONS
		
		


		////////////////////////////////////////////////////////////////////
		//
		// $.fn.exhibit.display
		// display thumbnail on stage, update toolbar
		//
		////////////////////////////////////////////////////////////////////
		function display(i, transition) {
			var maxDisplayArea;
			var resizedPictureDimensions;
			var stageCSS;
			var imageCSS;
			
			// optional parameter transition
			transition = transition || false;
			
			if (opts.border > 0) {
				var border = opts.border+"px solid "+opts.bordercolor;
			} else {
				var border = "none";
			}
			
			stageCSS = {};

			if (opts.resizing) {
				maxDisplayArea = GetMaxDisplayArea ();
				resizedPictureDimensions = GetResizedPictureDimensions (im.width[i], im.height[i], maxDisplayArea.width, maxDisplayArea.height );
				imageCSS = {
					height: resizedPictureDimensions.height,
					width: resizedPictureDimensions.width,
					marginTop:resizedPictureDimensions.matteWidth+'px',
					marginLeft:resizedPictureDimensions.matteWidth+'px',
					marginRight:resizedPictureDimensions.matteWidth+'px',
					marginBottom:resizedPictureDimensions.matteWidthBottom+'px',
					border: opts.imageborder+"px solid "+opts.imagebordercolor
				}
				stageCSS = {
					backgroundPosition: 'center',
					backgroundColor: opts.mattecolor,
					width:	resizedPictureDimensions.fullWidth,
					height: resizedPictureDimensions.fullHeight,
					border: border
				};
			} else {
				// set selected thumb as background image of stage
				stageCSS = {
					backgroundImage: 'url(' + escape(im.small[i]) + ')',
					backgroundPosition: 'center',
					backgroundColor: opts.mattecolor,
					border: border
				};
			}
			

			// once img has loaded...
			//--------------------------------------------------------------
			$(preloader).unbind('load');
			preloader.onload = function() {
				debug ('$.fn.exhibit.display: preloader.onload set for '+im.small[i]);
				
				clearTimeout(loadingtimer);
				exhStageWrap.removeClass(opts.lclass);
				
				//if transition, show transition on change of image
				if(opts.transition) {
					debug ('$.fn.exhibit.display B: '+exhStage.css('opacity'));
					exhImage.attr('src', this.src);
					exhStage.fadeTo(opts.duration,1);
					caption = im.caption[i];
					// show caption
					initCaption(i);
				} else {
					exhImage.attr('src', this.src);
				}

			};


			// if transition is desired, fade out, update counter and load
			if(transition) {
				// show loading graphic after a 1 sec delay
				loadingtimer = setTimeout("$('#exh-stagewrap').addClass('"+opts.lclass+"')", 1000);
				// transition (fade) the stage away in preparation for load of pic
				debug('A image source = '+exhImage.attr('src')+" WTF?");
				exhStage.fadeTo(opts.duration, 0, function() {
					debug('B image source = '+exhImage.attr('src')+" WTF?");
					debug('$.fn.exhibit.display: Display with transition ('+opts.duration+') '+im.small[i]);
					debug ('$.fn.exhibit.display A: '+exhStage.css('opacity'));
					updateCounter(i);
					if (opts.resizing) {
						exhImage.css(imageCSS);
						exhStage.css(stageCSS);
						//exhImage.attr('src', im.small[i]);
						preloader.src = im.small[i];
						debug ('$.fn.exhibit.display: preloader.src set for '+im.small[i]);
					} else {
						exhStage.css(stageCSS);
					}
				});
			} else {
				updateCounter(i);
				if (opts.resizing) {
					debug('$.fn.exhibit.display: Display resizing, without transition '+im.small[i]);
					exhImage.css(imageCSS);
					exhStage.css(stageCSS);
					//exhImage.attr('src', im.small[i]);
					preloader.src = im.small[i];
				} else {
					debug('$.fn.exhibit.display: Display not resizing, without transition '+im.small[i]);
					exhStage.css(stageCSS);
				}
			}

			// set REL of the popup button
			//--------------------------------------------------------------
			$(exhSwitch).attr('rel',im.id[i]);
			
			// preload thumb
			// preloader.src = im.small[i];
			// debug('$.fn.exhibit.display: Thumbnail ' + im.small[i] + ' loaded');
			
			preloadAdjacent(i);
		}
		
		
		////////////////////////////////////////////////////////////////////
		//
		// $.fn.exhibit.updateCounter
		// update image counter
		//
		////////////////////////////////////////////////////////////////////
		function updateCounter(i) {
			exhTotal.text(' ' + tot);			// total images
			exhCur.text((i + 1) + ' ');			// current image number
			debug('$.fn.exhibit.updateCounter: Displaying image ' + (i + 1) + ' of ' + tot);
		}
		
		////////////////////////////////////////////////////////////////////
		//
		// $.fn.exhibit.preloadAdjacent
		// preload next and previos image
		//
		////////////////////////////////////////////////////////////////////
		function preloadAdjacent(i) {
			
			// preload next and previous image
			var next = i;
			if( next < ( tot - 1) ) {
				next++; 
			} else {
				next = 0;
			}
			preloaderNext.onload = function() {
				debug('$.fn.exhibit.preloadAdjacent: Next image (' + next + ') loaded');
			};
			preloaderNext.src = im.small[next];
			
			var prev = i;
			if( prev <= 0 ) {
				prev = tot - 1;
			} else {
				prev--;
			}
			preloaderPrev.onload = function() {
				debug('$.fn.exhibit.preloadAdjacent: Previous image (' + prev + ') loaded');
			};
			preloaderPrev.src = im.small[prev];
		}
		
		
		
		
		////////////////////////////////////////////////////////////////////
		//
		// $.fn.exhibit.initCaption
		// 
		//	2=show, 1=popup, 3=hide caption
		////////////////////////////////////////////////////////////////////
		function initCaption(i) {
			// hide caption
			if (opts.showcaption == 2) {	//2=show
				//displayCaption(im.caption[i]);
				displayCaption(i);
			} else {
				hideCaption();
			}
		}
		
		////////////////////////////////////////////////////////////////////
		//
		// $.fn.exhibit.displayCaption
		// 
		//
		////////////////////////////////////////////////////////////////////
		function displayCaption(i) {
			
			if (opts.showcaption == 3)	//3=hide
				return;
			
			var newcap = $('#'+im.captionid[i]);

			cap = newcap.html();

			// Clear the text area then replace with a cloned caption DOM element 
			// (much better than copying HTML, because we copy handlers, too. We can even
			// use the theme editor this way)
			$(exhTextWrap).empty();
			$('#'+im.captionid[i]).children().clone(true).appendTo('.exh-textwrap');


			// OLD: copy the HTML content of the caption blocks on the page, not the DOM elements
			// update text box
			//exhText.html(cap);
			
			// update caption box below picture
			// exhCap.html(cap);
			
			if (opts.countpos == 'caption') {
				updateCounter(cur);
			}
			
			// if caption string is not empty...
			if (cap) {
				// make caption box visible and set it to width of stage
				var cssexhCaption = {
					visibility:		'visible'
				};
				if (opts.captiontarget == '') {
					// set caption box to width of stage
					cssexhCaption.width = exhStage.outerWidth();
					exhText.css(cssexhCaption);

					// slide caption box to height of inner text box
					exhText.animate({"height": exhTextWrap.outerHeight()}, {
						queue:		false,
						duration:	opts.captionshowspeed,
						easing:		opts.easing
					});
				} else {
					exhTextWrap.fadeTo(opts.captionshowspeed, 1);
				}
				
			}
			// if there's no caption, hide caption box completely to prevent
			// additional padding or margins of empty caption box to show
			else {
				hideCaption(true);
			}
			//debug('$.fn.exhibit.displayCaption -> exhTextWrap.outerHeight(): ' + exhTextWrap.outerHeight() + ', exhStage.outerWidth(): ' + exhStage.outerWidth());
		}
		
		
		////////////////////////////////////////////////////////////////////
		//
		// $.fn.exhibit.hideCaption
		// 
		//
		////////////////////////////////////////////////////////////////////
		function hideCaption(transition) {

			// optional parameter transition
			transition = transition || false;
			
			// css to hide caption but allow its inner text box to expand to content height
			var cssexhCaption = {
				visibility:		'hidden',
				overflow:		'hidden'
			};
			
			// set animation speed to 0 if transition is not desired
			var duration = false;
			if(transition) {
				duration = opts.captionhidespeed;
			}
			else {
				duration = 0;
			}

			// set width to width of compact stage
			// to allow enlarged box to transition to that width
			if (opts.captiontarget == '') {
				cssexhCaption.width = maxWidth;
				
				// slide up caption box and hide it when done
				exhCap.animate( {"height": '0px'}, {
					queue:		false,
					duration:	duration,
					easing:		opts.easing,
					complete:	function() {
						exhCap.css(cssexhCaption);
					}
				});
			} else {
				exhTextWrap.fadeTo(duration,0);
			}
		}
		

		////////////////////////////////////////////////////////////////////
		//
		// $.fn.exhibit.init
		// setup of exhibit DOM and events
		//
		////////////////////////////////////////////////////////////////////
		function init () {
			
			debug('$.fn.exhibit.init ->');			

			BuildStage();
			
			// display first image
			display(cur, opts.transition);
			
			// hide caption initially
			exhTextWrap.fadeTo(0,0);
			// show/hide caption depending on settings
			initCaption(cur);

			// Tools (navigation)
			// Show if showtools = show
			// opts.showtools: 1=popup, 2=show, 3=hide
			opts.showtools == 2 && exhTools.stop().fadeTo(100,opts.opacity);
			debug ('BuildStage --> showtools='+opts.showtools);

			AddEventHandlers();
			inited = true;
		};

	
		////////////////////////////////////////////////////////////////////
		//
		// BuildStage
		// Build the stage inside the 'stage' element.
		// 
		////////////////////////////////////////////////////////////////////
		function BuildStage () {

			$(opts.stage).html(exhStageWrap);

			// immediately set to 'loading'
			exhStageWrap.addClass(opts.lclass)
			
			exhStageWrap.append(exhStage);

			if (opts.resizing) {
				exhStage.append(exhImage);
			}
			exhStageWrap.after(exhTools);
			
			if (opts.countpos == 'overlay') {
				exhStage.append(exhCount);
			} else if (opts.countpos == 'below') {
				exhStageWrap.after(exhCount);
			}
			
			// Tools
			if (tot <= 1) {
				exhTools.append(exhPrevNull);
				exhTools.append(exhSwitch);
				exhTools.append(exhNextNull);
			} else {
				exhTools.append(exhPrev);
				exhTools.append(exhSwitch);
				exhTools.append(exhNext);
			}
			
			// Build Caption Box
			if (opts.caption) {
				exhStageWrap.after(exhCap);
			}
			exhCount.append(exhCur);
			exhCount.append(exhTotal);
			exhCur.after(opts.oflabel);
			exhCap.append(exhTextWrap);
			if (opts.countpos == 'caption') {
				exhTextWrap.prepend(exhCount);
			}
			exhTextWrap.append(exhText);
			
			// If we have a target object for the caption
			// put our caption object there.
			if (opts.captiontarget != '') {
				$('#'+opts.captiontarget).html(exhTextWrap);
				//exhTextWrap.appendTo('#'+opts.captiontarget);
			}
			
			// add class for additional styling
			if(opts.jsclass) {
				obj.addClass(opts.jsclass);
			}
			
			// give tools a directional class
			if (opts.direction == 'left') {
				exhTools.addClass(opts.dlclass);
			}
			else if (opts.direction == 'right') {
				exhTools.addClass(opts.drclass);
			}
			
			if (!opts.resizing) {
				// CSS: set stage big enough to handle biggest picture
				cssCompactStage = {
					width:			largestImgWidth,
					height:			largestImgHeight
				};
				exhStage.css(cssCompactStage);
				exhStageWrap.css(cssCompactStage);
			}
			// debug ('exhStageWrap -> '+largestImgWidth+', '+largestImgHeight);
			
			if (true || opts.resizing) {
				ResizeStage ();
			}

			// CSS: hide tools
			cssexhTools = {
				opacity:		0
			};
			exhTools.css(cssexhTools);
			
		}


		////////////////////////////////////////////////////////////////////
		//
		// $.fn.exhibit.ResizeStage
		// Calculate the size of the stage based on the largest height and
		// largest width of all pix, so it won't have to resize to show all pix.
		// 
		////////////////////////////////////////////////////////////////////
		function ResizeStage () {
			
			if (opts.resizing) {
				// Size to maximum picture, resized for this window.
				var maxDisplayArea = GetMaxDisplayArea ();

				debug('--- $.fn.exhibit.ResizeStage ');			
				debug('maxDisplayArea.height: ' + maxDisplayArea.height);			
				debug('maxDisplayArea.width: ' + maxDisplayArea.width);			
				
				if (!opts.fixedwidth) {
					var resizedPictureDimensions = GetResizedPictureDimensions (widestImage.width, widestImage.height, maxDisplayArea.width, maxDisplayArea.height );
					var largestImgWidthR = resizedPictureDimensions.fullWidth;
				} else {
					largestImgWidthR = opts.fixedwidth;
				}
				var resizedPictureDimensions = GetResizedPictureDimensions (tallestImage.width, tallestImage.height, maxDisplayArea.width, maxDisplayArea.height );
				var largestImgHeightR = resizedPictureDimensions.fullHeight;
			
				debug('$.fn.exhibit.ResizeStage -> largestImgWidthR: ' + largestImgWidthR);			
				debug('$.fn.exhibit.ResizeStage -> largestImgHeightR: ' + largestImgHeightR);			

				var cssStage = {
					width:			largestImgWidthR + (2*opts.border),
					height:			largestImgHeightR+ (2*opts.border)
				};
				exhStageWrap.css(cssStage);
			} else {

				var cssStage = {
					width:			widestImage.width,
					height:			tallestImage.height
				};
				exhStage.css(cssStage);
				exhStageWrap.css(cssStage);
			}
			
		}

		
		// AddEventHandlers
		// Add event handlers.
		//----------------------------------------------------------------------
		function AddEventHandlers () {
			// Bind to window resize...when screen is resized, the picture redraws to fit.
			if (opts.resizing) {
				debug('$.fn.exhibit.AddEventHandlers -> Set dynamic resizing of stage');
				$(window).resize(function () {
					if (inited) {
						debug('$.fn.exhibit.AddEventHandlers -> Dynamic resize of stage');
						ResizeStage();
						display(cur);
					}
				});
			}

			// opts.showtools: 1=popup, 2=show, 3=hide
			// show tools on hover
			exhStage.hover(
				function(e){
					opts.showtools == 1 && exhTools.stop().fadeTo(100,opts.opacity);
				},
				function(e){
					opts.showtools == 1 && exhTools.stop().fadeTo(500,0);
				}
			);
			exhTools.mouseleave(
				function(e){
					opts.showtools == 1 && exhTools.stop().fadeTo(500,0);
				}
			);
			exhTools.mouseenter(
				function(e){
					opts.showtools == 1 && exhTools.stop().fadeTo(100,opts.opacity);
				}
			);
			
			// show caption on hover
			// Hide only if we can be sure the caption is inside the stage area
			// and we can point to it, for "buy this picture" links.
			exhStage.hover(
				function(e){
					//opts.showcaption == 1 && displayCaption(im.caption[cur]);
					opts.showcaption == 1 && displayCaption(cur);
				},
				function(e){
					if (!opts.captiontarget) {
						opts.showcaption == 1 && hideCaption(true);
					}
				}
			);

			// previous image button
			exhPrev.click(function(){
				if (tot <= 1)
					return false;
					
				if( cur <= 0 ) {
					cur = tot - 1;
				} else {
					cur--;
				}
				display(cur, opts.transition);
				// Set global
				FP_CURRENT_IMAGE_INDEX = cur;
				return false;
			});
			
			// next image button
			exhNext.click(function(){
				if (tot <= 1)
					return false;

				if( cur < ( tot - 1) ) {
					cur++;
				} else {
					cur = 0;
				}
				display(cur, opts.transition);
				// Set global
				FP_CURRENT_IMAGE_INDEX = cur;
				return false;
			});
			
			// enlarge image button

			exhSwitch.click(function(){
				// Add lightbox switch
				if (opts.lightbox) {
					// build group from im
					var ilist = '#' + im.id.join(',#');
					
					debug ('Show image #'+cur+' in lightbox.');
					
					var lightboxopts = {
						current_image: cur,
						start: true,
						show_linkback:	false,
						ie6_support: true,
						ie6_upgrade: false,
						opacity: 0.5,
						download_link: false,
						padding: matteRatio,
						padding_extra: matteBottomRatio,
						default_caption_padding: 10,
						caption_separator: '<br>',
						show_info: opts.showcaption,
						show_button_nav: false,
						use_body_background_color : true
					};
					//debug ('Show '+ilist+' in lightbox.');
					$(ilist).lightbox(lightboxopts);
				}
				
				return false;
			});
		}
	

		// Debugging console output
		//----------------------------------------------------------------------
		function debug(msg) {
			if (window.console && window.console.log && opts.debug) {
				window.console.log(msg);
			}
		}
		
		// Resizing functions (DG)
		//----------------------------------------------------------------------
		

		/*
		--------------------------------------------------------------------------------------------------
		Calculate new image dimensions based on desired new height or width.
		We must supply the height/width, because if we take it from the current version of the picture 
		(which might have been resized), we quickly get distortions to due rounding errors.
		iWidth/iHeight	:	width/height of the original unresized image
		maxWidth			:	desired maximum width (i.e. fit inside this width)
		maxHeight		:	desired maximum height (i.e. fit inside this height)
		maxImageWidth	:	maximum width of the image, usually the original dimension
		maxImageHeight	:	maximum height of the image, usually the original dimension
		
		Return:
		result={
			iWidth			:	image width
			iHeight			:	image height
			newWidth			:	enclosing object width (usually a DIV, to simulate a matte)
			newHeight			:	enclosing object height (usually a DIV, to simulate a matte)
			matteWidth		:	width of matte around the picture
			matteWidthBottom	:	width of bottom of matte
			}
		*/
		
		function GetResizedPictureDimensions (iWidth, iHeight, maxWidth, maxHeight ) {

			var w = iWidth;
			var h = iHeight;
			var p = iWidth/iHeight;
			var b = 2*opts.imageborder;
			
			// subtract space for border around pic/matte
			maxWidth = maxWidth;
			maxHeight = maxHeight;

			// size to fit width
			var mw = 1 + (1*matteRatio);
			var mh = 1 + (1*matteRatio) + matteBottomRatio;
			
			var fullWidth = maxWidth;
			var iWidth = Math.floor((maxWidth)/mw);
			var iHeight = Math.floor(iWidth/p);
			var matteWidth = Math.floor((iWidth * matteRatio)/2) - opts.imageborder;
			if (matteWidth < 0)
					matteWidth = 0;
			var matteWidthBottom = Math.floor(iWidth*matteBottomRatio) + matteWidth;
			if (matteWidthBottom < 0)
				matteWidthBottom = 0;
			var fullWidth = iWidth + (2*matteWidth) + (2*opts.imageborder);
			var fullHeight = iHeight + matteWidth + matteWidthBottom + (2*opts.imageborder);

			//var fullHeight = Math.floor(iHeight * mh);
			//debug('A) GetResizedPictureDimensions: w/h='+iWidth+', '+iHeight+", fullWidth/fullHeight="+fullWidth+"/"+fullHeight+", maxWidth/maxHeight="+maxWidth+"/"+maxHeight+ ' (matteWidth='+matteWidth+', matteWidthBottom='+matteWidthBottom+')');

			// if image is too high, size to fit height
			if (fullHeight > maxHeight) {
				fullHeight = maxHeight;
				
				// *** issue is, calc matte from height or width? If we switch, we get a weird jump.
				// If not, what is 'mh', below?
				
				iHeight = Math.floor(maxHeight/mh);
				iWidth = Math.floor(iHeight*p);
				matteWidth = Math.floor((iHeight * matteRatio)/2) - opts.imageborder;
				if (matteWidth < 0)
					matteWidth = 0;
				matteWidthBottom = Math.floor(iHeight*matteBottomRatio) + matteWidth;
				if (matteWidthBottom < 0)
					matteWidthBottom = 0;
				fullWidth = iWidth + (2*matteWidth) + (2*opts.imageborder);
				fullHeight = iHeight + matteWidth + matteWidthBottom + (2*opts.imageborder);
				//debug('B) GetResizedPictureDimensions: w/h='+iWidth+', '+iHeight+", fullWidth/fullHeight="+fullWidth+"/"+fullHeight+", maxWidth/maxHeight="+maxWidth+"/"+maxHeight+ ' (matteWidth='+matteWidth+', matteWidthBottom='+matteWidthBottom+')');
				
			}

			//debug('C) fullWidth: '+fullWidth+", fullHeight:"+fullHeight);
			var result = {width:iWidth, height:iHeight, fullWidth:fullWidth, fullHeight:fullHeight, matteWidth:matteWidth, matteWidthBottom:matteWidthBottom};

			return result;
		}
		
		/* UNUSED
		// Calculate basic matte width for top/left/right sides, for tall vs. wide pictures
		// Bottom is based on this calc.
		// p = w/h
		$.exhibit.MatteWidth = function (w,h) {
			if (w > h) {
				// wide
				mw = Math.floor(h * matteRatio)/2;
			} else {
				// tall
				mw = Math.floor(w * matteRatio)/2;
			}
			return mw;
		};
		*/

		/*
		--------------------------------------------------------------------------------------------------
		Get the maximum area to show a picture, based on the window size.
		Subtract params for padding to h/w, to compensate for objects outside the image.
		Subtract the border around the picture/matte.
		
		Return:
			result = {height:height, width:width}
		*/
		
		function GetMaxDisplayArea () {
		
			if (opts.fixedwidth > 0) {
				var maxWidth = opts.fixedwidth;
				var maxHeight = opts.fixedheight;	
			} else {
				var wWidth	= $(window).width();
				var wHeight = $(window).height();
				var screenWRatio = .98;
				var screenHRatio = .98;
				var maxWidth  = Math.floor(wWidth*screenWRatio) - opts.hpad - (2*opts.border);
				var maxHeight = Math.floor(wHeight*screenHRatio) - opts.vpad - (2*opts.border);
				
				// Abitrary max/min sizes
				if (maxWidth < 300)
					maxWidth = 300;
				if (maxHeight < 200)
					maxHeight = 200;
				//debug('$.fn.exhibit.GetMaxDisplayArea: MaxWidth = '+maxWidth+", MaxHeight="+maxHeight);
			}
			return {height:maxHeight, width:maxWidth};
		}


	
	};	// end $.fn.exhibit = function...
	
	////////////////////////////////////////////////////////////////////////////
	//
	// defaults
	// set default options
	//
	////////////////////////////////////////////////////////////////////////////
	var defaults = {
		stage:				'#exhibit',				// id of the stage element...where to show the pictures
		imageElement:		'.exhibit-image-entry',	// class of the DIV element which contains images, caption, etc.
		jsclass:				'exh-js',				// class applied to exhibit root element when JS is active
		nojsclass:			'exh-no-js',				// class of optional element with warning to browsers without JS - element will be removed once exhibit has loaded
		lclass:				'exh-loading',			// class to be applied to stage while loading image
		dlclass:				'exh-left',				// class to be applied to exh-tools if exhibit opens to the left
		drclass:				'exh-right',				// class to be applied to exh-tools if exhibit opens to the right
		direction:			'left',					// direction that exhibit-box opens, can be "left" or "right"
		duration:			250,						// duration of transitional effect when enlarging or closing the box
		transition:			true,					// use a fadein/fadeout transition between images

		opacity:				0.7,						// opacity of navigational overlay
		showtools:			1,						// navigation: 1=popup, 2=show, 3=hide
		countpos:			'caption',				// position of image-counter - can be false, "overlay" or "caption"

		caption:				'description',			// 'title': display caption based on title attribute,
													// other: text inside of class of a div/span inside an image element
		showcaption:			1,						// caption: 2=show, 1=popup, 3=hide. 'showCaptions' is an FP global var
		captiontarget: 		'',						// id of where to show the caption; if empty, show below pic
		easing:				'swing',					// easing type, can be 'swing', 'linear' or any of jQuery Easing Plugin types (Plugin required)
		captionshowspeed:	150,						// transition speed to show/hide/animate the caption
		captionhidespeed:	500,						// transition speed to hide. Must be slow so user can click on it
		nlabel:				'',						// label for next button
		plabel:				'',						// label for previous button
		oflabel:				'of',					// label for image count text (e.g. 1 of 14)
		blabel:				'',						// label for enlarge button
		clabel:				'Click to close',		// label for expanded stage (to hint closing),
		hpad:				100,						// extra padding around resizing image
		vpad:				100,						// extra padding around resizing image
		resizing:			1,						// 1: resize image to fit screen, 0: use fixedwidth/fixedheight
		fixedwidth:			0,						// if fixedwidth>0: force stage to fixedwidth pixels WIDE.
		fixedheight:			0,						// Height of picture area when fixedwidth is used.
		matteshow:			0,						// show a matte
		mattecolor:			'#FDFDFD',				// color of the matte
		mattescale:			0.25,					// = 75% : multiplier to reduce an image to fit inside matte. Do not set to zero!
		mattebottom:			0.05,
		border:				1,						// show a border around the picture (outside the matte)
		bordercolor:			'#666',					// border color
		imageborder:			1,
		imagebordercolor:	'#888',
		lightbox:			false,					// use lightbox.jquery to show a slide show when enlarge switch is clicked
		debug:				false					// turn on console output (slows down IE8!)

	};


// end of closure, bind to jQuery Object
})(jQuery); 
