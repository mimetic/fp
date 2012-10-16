/*
	Admin Javascript
	name: fp_javascript.js
*/

// jQuery-based functions
$(document).ready( function() {


	// show obfuscated email addresses which have class=obfuscate
	//$(".obfuscated").defuscate();

	// Confirmations
	// Uncheck checkboxes if checked.
	$('.confirm').click(function(){
	msg = $(this).attr('alt');
		if (!confirm (msg)) {
			c = $(this).attr('checked');
			if (c)
				$(this).attr('checked', false);
			return false;
		} else {
			return true;
		}		
	});


	// Show multiuser features (hidden in default css sheet)
	var multiuser = false;
	if (FP_GALLERY_TYPE == FP_SINGLE_GALLERY_MULTI_USER || FP_GALLERY_TYPE == FP_MULTI_GALLERY_MULTI_USER)
		multiuser = true;
	if (multiuser)
		$('.multiuser').show();
	
	/*
	// No longer user --- tablesorter takes over this.
	// Striping in tables
	$('.listing tr:even').addClass ('alt');
	
	*/
	
	
	// autosubmit popup lists, where the object is a 'select' whose id begins with 'autosubmit',
	// e.g. id=autosubmit_100
	// The form must be the parent!!!
	$('select[id^=autosubmit]').change( function() {
	 	$(this).parent().submit();
	 });
	
	// show/hide event handler
	$('.toggler')
	.click(function(){
		$(this).parent().next().toggle();
		var t = $(this).attr('title');
		if (t) {
			var ts = t.split(':');
			var showtext = ts[0];
			var hidetext = ts[1];
		} else {
			var showtext = "+";
			var hidetext = "-";
		}
		$(this).text(
			($(this).parent().next().is(':hidden') ? showtext : hidetext)
		);
	});
	
	// Add tool tips
	$('.tip').bt({
		hoverIntentOpts: {
    			interval: 150,
			timeout: 1500
		},
		fill: "#FFFF33",
		strokeWidth: 1, /*no stroke*/
		spikeLength: 30,
		spikeGirth: 13,
		padding: 10,
		cornerRadius: 15,
		cssStyles: {
			fontFamily: '"lucida grande",tahoma,verdana,arial,sans-serif', 
			fontSize: '13px'
		}
	});

	$('#SlideShowSlideDuration').change(function() {
		var showTime, showTimeToShow, picCount, transition, showPause, defaultShowPause, x, pauseToShow;

		showPause = Math.abs($(this).val());
		picCount = $('#picturecount').val();
		transition = 1 * parseFloat($('#FP_SSP_TRANSITION_LENGTH').val());
		showTime = (picCount * (showPause + transition));
		$('#SlideShowDuration').val(showTime).trigger('change');


	});

	$('#SlideShowDuration').change(function() {
		var showTime, showTimeToShow, picCount, transition, showPause, defaultShowPause, x, pauseToShow;

		showTime = Math.abs($(this).val());
		picCount = $('#picturecount').val();
		transition = 1 * parseFloat($('#FP_SSP_TRANSITION_LENGTH').val());
		showPause = (showTime/picCount) - transition;
		showPause = 1 * parseFloat(showPause.toFixed(1));
		defaultShowPause = 1 * $('#FP_DEFAULT_SLIDESHOW_PAUSE').val();

		if (showPause < 0)
			showPause=0.01;

		if (showTime == 0)
			showPause = defaultShowPause;
			//showPause = 0;
		
		x = (showPause + transition) * picCount;
		//x = x.toFixed(1);
		
		if (showTime < x && showTime > 0)
			showTime = x.toFixed(0);
		
		showTimeToShow = showTime;
		
		pauseToShow = showPause.toFixed(1);
		if (showTime == 0) {
			pauseToShow = defaultShowPause;
			showTimeToShow = x.toFixed(0);
		}
		$('#slide_min_duration').html(transition * picCount);
		$('#slideshow_show_time').html(showTimeToShow);
		$(this).val(showTime);
		$('.slide_duration_output').html(pauseToShow);
		$('#SlideShowSlideDuration').val(pauseToShow);
	}).trigger('change');
	
	$('*[id^=cssEditSwitch]').cssEditSwitch();
	$('*[id^=cssEditActivate]').cssEditActivate();



	// Set preview switchs
//	$('*[id^=PreviewSwitch]').toggle (
	$('.PreviewSwitch').toggle (
		function () {
			var myID = this.title;
			info = $('#info'+myID).val().split(',');
			url = info[0];
			this.src = url;
			$('#preview'+myID).attr('src', url).show();
		},
		function () {
			$('#preview'+this.title).hide();
		}
	);


	// Preview pictures switch
	// For lists of images, etc.
	$('#AllPreviewsSwitch')
	.toggle(
		function() {
			showAllPreviews();
			$(this).html("<span class='ui-button-text'>Hide All Previews</span>");
		},
		function() {
			hideAllPreviews();
			$(this).html("<span class='ui-button-text'>Show All Previews</span>");
		}
	);

	// Preview pictures switch for a subset of pix
	// For lists of images, etc.
	$('.GroupPreviewSwitch')
	.toggle(
		function() {
			showAllPreviews(this.title);
			//label = $(this).html();
			//$('.GroupPreviewSwitch[title='+this.title+']').html("Hide Previews");
		},
		function() {
			hideAllPreviews(this.title);
		}
	);

	
// ========================================================================
// Clickables to hide/show
// Set all items with the class, "my-ui-revealer", to reveal the object whose
// ID is the Title of the clickable.
// example: <a title="targetID">Click here</a> <div id="targetID">ShowMe</div>
// ========================================================================

// Clickables to show/hide hidden stuff
	$('.my-ui-revealer').click(function() {
		$('#'+this.title).slideToggle()
	});
		
	
	

// ========================================================================
// Make Buttons into Links using an href
// ========================================================================

	$('button.buttonlink').click(function() {
		//alert ($(this).attr('href'));
		t = $(this).attr('target');
		if (t == "_blank")
			window.open($(this).attr('href'), 'Certificate of Authenticity');
		else
			location.href = $(this).attr('href');
		return false;
	});


// ========================================================================
// Delete buttons for lists
// ========================================================================

	// 'Delete' buttons
	$('button.delete-button').click(function() 
	{
		k = $(this).attr('title');
		if (confirm("Delete "+k+"?"))
			{
			$('form').submit();
			}
			else 
			{
			return false;
			}
	});



	
// ========================================================================
// Dim entries using id = "dimmer-x" and class = "dimmable-x"
// Anything with id=dimmer-x will dim/undim anything of class=dimmable-x,
// where the value of dimmer-x is "1" for dim, anything else for not-dim
// ========================================================================

	// Dim "On Hold" entries in the list
	$("*[id^=dimmer]").each(function() {
		var i = $(this).attr('id').match(/-(.*)/);
		if ($(this).val() == "1" ) {
			$(".dimmable-" + i[1]).addClass('DimRow');
		}
	}).change(function() {
		var i = $(this).attr('id').match(/-(.*)/);
		if ($(this).val() == "1" ) {
			$(".dimmable-" + i[1]).addClass('DimRow');
		} else {
			$(".dimmable-" + i[1]).removeClass('DimRow');
		}
	});
	
	

	
// ========================================================================
// Image Input Page
// ========================================================================
	
	// Reset image edition size to use price set values
	$('#UsePriceSetForEditionSize').click(function() {
		$('input#param_05').val('');
		$('#editionsizeshow').html($(this).val());
	});
	
	// If video picture, then hide stuff	
	$('#FP_PARAM_IMAGE_IS_VIDEO').change(function() {
		if ($('#FP_PARAM_IMAGE_IS_VIDEO:checked').length) {
			$('.hide-for-video').fadeOut();
			$('.show-for-video').fadeIn();
		} else {
			$('.hide-for-video').fadeIn();
			$('.show-for-video').fadeOut();
		}
	});
	if ($('#FP_PARAM_IMAGE_IS_VIDEO:checked').length) {
		$('.hide-for-video').fadeOut();
		$('.show-for-video').fadeIn();
	} else {
		$('.hide-for-video').fadeIn();
		$('.show-for-video').fadeOut();
	}


	// If Original Art, 
	// - set fixed size
	// - show the "fixed size" settings
	// - show the "fixed price" settings
	// - hide custom edition sizing
	$('#FP_PARAM_IMAGE_IS_ORIGINAL_ART').change(function() {
		$('#FP_PARAM_IMAGE_IS_FIXED_SIZE').attr('checked',true);
		showOnChecked ('FP_PARAM_IMAGE_IS_FIXED_SIZE', 'fixed_size_dims');
		//showOnChecked ('FP_PARAM_IMAGE_IS_ORIGINAL_ART', 'custom_price');
		hideOnChecked ('FP_PARAM_IMAGE_IS_ORIGINAL_ART', 'custom_edition_size');
	});
	hideOnChecked ('FP_PARAM_IMAGE_IS_ORIGINAL_ART', 'custom_edition_size');
	

	$('#FP_PARAM_IMAGE_IS_FIXED_SIZE').change(function() {
		showOnChecked ('FP_PARAM_IMAGE_IS_FIXED_SIZE', 'fixed_size_dims');
		// lock offsite sizing, since we calc automatically from fixed size
		if ($(this).attr('checked') ) {
			$('#FP_PARAM_IMAGE_WIDTH').attr("disabled", true);
			$('#FP_PARAM_IMAGE_HEIGHT').attr("disabled", true);
		} else {
			$('#FP_PARAM_IMAGE_WIDTH').removeAttr("disabled");
			$('#FP_PARAM_IMAGE_HEIGHT').removeAttr("disabled");
		}
	
	});
	if ($('#FP_PARAM_IMAGE_IS_FIXED_SIZE').attr('checked') ) {
		$('#FP_PARAM_IMAGE_WIDTH').attr("disabled", true);
		$('#FP_PARAM_IMAGE_HEIGHT').attr("disabled", true);
	} else {
		$('#FP_PARAM_IMAGE_WIDTH').removeAttr("disabled");
		$('#FP_PARAM_IMAGE_HEIGHT').removeAttr("disabled");
	}
	
	// If user enters the file size at the printer's, in units (e.g. inches):
	$('#enter_width_in_units, #enter_height_in_units').change(function() {
		var t = $(this).attr('target');
		$('#'+t).val(ConvertUnitsToPixels($(this).val()));
	});
	
	// If user enters the file size at the printer's in pixels
	$('#FP_PARAM_IMAGE_WIDTH, #FP_PARAM_IMAGE_HEIGHT').change(function() {
		var t = $(this).attr('target');
		$('#'+t).val(ConvertPixelsToUnits($(this).val()));
	}).change();
	
	showOnChecked ('FP_PARAM_IMAGE_IS_FIXED_SIZE', 'fixed_size_dims');
	//showOnChecked ('FP_PARAM_IMAGE_IS_ORIGINAL_ART', 'custom_price');

	$('#FP_PARAM_IMAGE_OFFSITE').change(function() {
		showOnChecked ('FP_PARAM_IMAGE_OFFSITE', 'offsitedimensions');
	});
	showOnChecked ('FP_PARAM_IMAGE_OFFSITE', 'offsitedimensions');


// ========================================================================
// Show Theme Preview
// ========================================================================
	
	$('select[id=themelist]').not('select[id^=autosubmit]').change(function() {
		var t = $(this).val();
		//$('#test2').val(t);
		ShowThemePreview(t);
	}).each(function() {
	// do it first time
		var t = $(this).val();
		ShowThemePreview(t);
	});

	
// ========================================================================
// Price Set Input Page
// ========================================================================
	
	var FP_ROW_PRICING_DATA = {};

	/*
	// Legacy: only useful if we want different edition amounts for size in an edition,
	// e.g. 8x10 is 10 prints, while 20x30 is 15 prints.
	// Total up the edition entries to get the total edition size.
	$('input[name^=EditionSize]').change(function() {
		CalcTotalEditionSize('EditionSize', 'TotalEditionSize');
	});
	CalcTotalEditionSize('EditionSize', 'TotalEditionSize');
	*/
	
	
	// Update estimate of pricing
	$('button#updatepricesetform').click(function() {
		GetSamplePricingData(this.title);
	});
	
	
	// Calculate and show image, paper, and frame sizing
	$('input[name^=Size]').change(function() {
		target = "FrameSize" + $(this).attr('title');
		ShowPrintFrameSizes(this, target);
	});
	ShowAllPrintFrameSizes();


	// 'Delete' buttons
	$('button[id^=DeletePriceSetRow]').click(function() 
	{
		k = $(this).attr('title');
		if (confirm("Delete entry #"+k+"?"))
			{
			$('#size-'+k).val(0);
			//$('#action').val('update');
			return true;
			}
		else
			{
			return false;
			}
	});

	$('#pricesetform button[type=submit]').click(function() {
		res = ValidatePriceSetEntry();
		return res;
	});

	
	// ========================================================================
	// jQuery UI Formatting
	
	$("button, input:submit, input:reset").button();
	
	$("#accordion").accordion({ header: "h3" });


	// Format currency INPUT fields (class="currency") as xx.xx
	$('.twodecimals').each(function() {
		FormatAsFloat(this);
	}).blur(function() {
		FormatAsFloat(this);

	});


	// ========================================================================

	// Autosubmit items (similar to autosubmit with ID, above)
	$('.autosubmit').change(function() {
			$(this).parent().submit();
	});

	// ========================================================================




// ========================================================================
//	 ImageSorter
// ========================================================================

	// ------------
	
	$('#imagesort_select_project').change(function(){
		pid = $(this).val();
		if (pid)
			HideImagesInOtherProjects (pid);
	});
		
	
	// Show only pix in the archives from the chosen project in the popup chooser
	pid = $('#imagesort_select_project').val();
	if (pid)
		HideImagesInOtherProjects (pid);
	
	// Start the image sorter
	InitImageSorter();
	StartImageSorter();


	// ====== slider to control slide size on screen
	
	// Copy h/w of slide images to a safe place
	$img = $('.gallery li img');
	$img.each(function() {
		$(this).attr('originalwidth', $(this).width()).attr('originalheight', $(this).height());
	});
	
	$("#slider").slider({
		value 	: 20,
		min		: 12,
		max		: 40,
		change: function(event, ui) {
			r = ui.value/10;
			$('.gallery li').width(r*80).height(r*80);
			h = $img.each(function() {
				$(this).height( $(this).attr('originalheight')*(r/2)).width( $(this).attr('originalwidth')*(r/2) );
			});
			//$('#inlist, #outlist').sortable('refreshPositions');
		}
	});
	
	// 
	

// ========================================================================
// Remove empty rows from tables with TD with class "remove-if-empty"
// ========================================================================
	RemoveEmptyRows();


// ========================================================================
// COA - Certificate of Authenticity
// ========================================================================
	

	$('#coa-show-complete').click(function() {
		$('.coa-filled-form').fadeTo(300,1);
		$('.coa-skeleton').fadeTo(300,1);
		$('#coa-blank-container').fadeTo(300,0);
		$('#coa-filled-container').removeClass('coa-no-border').fadeTo(300,1);
	});

	$('#coa-show-skeleton').click(function() {
		$('#coa-blank-container').fadeTo(300,0);
		$('.coa-filled-form').fadeTo(300,0);
		$('.coa-skeleton').fadeTo(300,1);
		$('#coa-filled-container').removeClass('coa-no-border').fadeTo(300,1);
	});

	$('#coa-show-filled').click(function() {
		$('.coa-filled-form').fadeTo(300,1);
		$('#coa-filled-container').addClass('coa-no-border').fadeTo(300,1);
		$('#coa-blank-container').fadeTo(300,0);
		$('.coa-skeleton').fadeTo(300,0);
	});

	$('#coa-show-blank').click(function() {
		$('#coa-filled-container').removeClass('coa-no-border').fadeTo(300,0);
		$('#coa-blank-container').fadeTo(300,1);
	});

	// value set on the actual page in Javascript
	if (window.COA_SHOWBLANKFORMONLY != undefined)
		if (COA_SHOWBLANKFORMONLY) {
			$('#coa-show-complete').hide();
			$('#coa-show-filled').hide();
			$('#coa-show-skeleton').trigger('click');
		}

	$('#coa-show-blank').trigger();
	



// ========================================================================
// Navbar, Link to top
// ========================================================================
	
	pageid = $('body').attr('id');
	$('#navbar-'+pageid).addClass('ui-tabs-selected ui-state-active');

	$('.linktotop').click(function() {
		window.location.href = '#top';
	});

	$('.toggleadvanced').click(function() {
		ToggleAdvanced();
	});


// ========================================================================
// JQUERY - PASSWORD GENERATOR, VERIFY
// ========================================================================
	
	
	
	// GENERATE PASSWORD BUTTON
	var link = $( '<button type="button" href="#" style="margin-left: 20px;" class="ui-widget ui-button ui-state-default ui-corner-all ui-button-text-only" id="passgen"><span class="ui-button-text">Generate Password</span></button>' );
	$('#passpgen').append( link );	
	$('#passgen').click(function() {
		p = $.password(12,false);
		alert ("Write down this password: "+p);
		$('#Password').val(p);
		$('#ConfirmPassword').val(p);
	});
	
	// GENERATE COUPON CODE BUTTON
	var link = $( '<button type="button" href="#" style="margin-left: 20px;" class="ui-widget ui-button ui-state-default ui-corner-all ui-button-text-only" id="passgen"><span class="ui-button-text">Generate Buyer&lsquo;s Code</span></button>' );
	$('#coupongen').append( link );
	$('#coupongen').click(function() {
		p = $.password(12,false);
		desc = prompt("Buyer's code description:","description");
		disc = prompt("Buyer's code discount (1-100):","");
		if (disc <= 100 && disc > 0) {
			x =  $.trim($('#param_05').val()) + "\n" + desc + ", " + p + ", " + disc;
			$('#param_05').val(x);
		} else {
			alert ("The discount you entered was not between 1 and 100.");
		}
	});
	
	
	// Verify: password == retype password
	$("#myform").Fvalidate( {
		submitButton : $('#myform').find(".savebutton,.saveclosebutton")
	});
 	// this will attach the confirm validation to both the fields
	$("input[name=ConfirmPassword]").sameAs($("input[name=Password]"));

// ========================================================================
// JQUERY - jQuery UI stuff
// ========================================================================

	//all hover and click logic for buttons
	$(".my-button:not(.ui-state-disabled)")
	.hover(
		function(){ 
			$(this).addClass("ui-state-hover"); 
		},
		function(){ 
			$(this).removeClass("ui-state-hover"); 
		}
	)
	.mousedown(function(){
		$(this).parents('.my-buttonset-single:first').find(".my-button.ui-state-active").removeClass("ui-state-active");
			if( $(this).is('.ui-state-active.my-button-toggleable, .my-buttonset-multi .ui-state-active') ){ $(this).removeClass("ui-state-active"); }
			else { $(this).addClass("ui-state-active"); }	
	})
	.mouseup(function(){
		if(! $(this).is('.my-button-toggleable, .my-buttonset-single .my-button,  .my-buttonset-multi .my-button') ){
			$(this).removeClass("ui-state-active");
		}
	});

	//$('#switcher').themeswitcher({height: 150});


// ========================================================================
// Picture Multi Editor
// A checkbox with .me_clear class will be cleared after close/apply, to 
// avoid reuse. Esp. good for "delete"!
// ========================================================================



	$("#image-editor").dialog({
		position: ['center','center'],
		bgiframe: true,
		autoOpen: false,
		width: 650,
		modal: true,
		open: function(event,ui) {
			
		},
		buttons: {
			'Close': function() {
				$(this).dialog('close');
			},
			'Apply' : function () {
				ProcessRows('Images');
				$(this).dialog('close');
			}
		},
		close: function() {
			$('.me_clear').attr('checked',false);
		}
	});
	
	// activate image metadata editor dialog
	$('#open-image-editor').click(function() {
			$('#image-editor').dialog('open');
	});



// ========================================================================
// Picture Quick Editor
// ========================================================================



	$(".imageQuickEditor").dialog({
		position: ['center','center'],
		bgiframe: true,
		autoOpen: false,
		width: 650,
		modal: true
	});
	
	// activate image metadata editor dialog
	$('.imageQEopener').click(function() {
		var i = this.title;
		$('#'+i).dialog('open');
	});
	
	// If the PriceSet is locked, then make it so you can't change the edition size.
	
	// Make sure the number of outside sales is not greater than the edition size.
	$(".outsidesales").change(function() {
		var id = $(this).attr('title');
		var edID = id + "_param_05";
		var ed = $('#'+edID).val();
		var os = this.value;
		if (os > ed) {
			$(this).val(ed);
			alert ("Warning: your outside sales were higher than the edition size! I've changed the outside sales amount to the edition size.");
		}
	});


// ========================================================================
// Ajax Update Record
// ========================================================================

	$('.ajaxsave').change(function() {
		var v = $(this).val()
		var id = this.title;
		var k = this.name;
		//alert ( 'id='+id+", "+k + " =>"  + v);
		var values = {}
		values[k] = v;
		UpdateRecordViaAjax ('Images', id, values)
	});


// ========================================================================
// Picture Quick Edit in row
// ========================================================================

	// Reset image edition size to use price set values
	$('.image-edit-restore-edsize').click(function() {
		var id = this.title;
		$('input#'+id+'_param_05').val('').trigger('change');
		$('#'+id+'_editionsizeshow').html($(this).val());
	});
	
	// Param_05 is custom edition size. If it changes, also update the text
	// that says the edition size
	$('[id$=param_05]').change(function() {
		var id = $(this).attr('title');
		$('#'+id+'_editionsizeshow').html($(this).val());
	});

	// select:PriceSetID objects are popup menus. When one is changed, we should update the row
	// text to match it.
	// <SELECT NAME="PriceSetID"  class="ajaxsave" title="1403">
	$('select[name=PriceSetID]').change(function()
		{
		var i = this.title;
		var myid = "#PriceSetID"+i;
		var v = $(myid + " option:selected").text();
		$("#PriceSetName"+i).html( v );
		}
	);


// ========================================================================
// TableSorter
// ========================================================================
	var myTextExtraction = function(node)  
	{  
	    var x = node.title;
	    return x;
	}
	$("table.tablesorter").tablesorter({
		widthFixed: true, 
		widgets: ['zebra'],
		textExtraction: myTextExtraction
	});
	/*
	.tablesorterPager({
		size:40,
		container: $("#pager"),
		positionFixed: false
	});
	*/

	// Highlight row you're pointing to
	$('tr.listing').hover( function() {
		$(this).addClass('over');
	},
	function () {
		$(this).removeClass('over');
	});

/*
	$(".listing tr").mouseover(function() {$(this).addClass("over");}).mouseout(function() {$(this).removeClass("over");});

*/
// ========================================================================
// Inline Text Editor
// ========================================================================
	$('.editable_text').editable(
		function(value, settings) {
			var t = $(this).attr('table');
			var id = $(this).attr('fieldid');
			var k = $(this).attr('name');
			var values = {};
			values[k] = value;
			//alert (t+", " + id + ", "+k+", "+value);
			UpdateRecordViaAjax (t, id, values)
			return value;
		}, 
		{
		style     : "inherit",
		type      : 'text',
		submit    : 'Save',
//		submit    : '&#10003;',		//checkmark
		indicator : '<img src="images/indicator.gif">',
		tooltip   : 'Click to edit...'
		}
	);

	$('.editable_textarea').editable(
		function(value, settings) {
			var t = $(this).attr('table');
			var id = $(this).attr('fieldid');
			var k = $(this).attr('name');
			var values = {};
			values[k] = value;
			//alert (t+", " + id + ", "+k+", "+value);
			UpdateRecordViaAjax (t, id, values)
			return value;
		}, 
		{
		style     : "inherit",
		type      : 'textarea',
		submit    : 'Save',
//		submit    : '&#10003;',		//checkmark
		indicator : '<img src="images/indicator.gif">',
		tooltip   : 'Click to edit...'
		}
	);

	/*
	.mouseover(function() {
		$(this).addClass('isEditable');
	})
	.mouseout(function() {
		$(this).removeClass('isEditable');
	});
	*/

 
 
// ========================================================================
// SELECT ALL CHECKBOX:
// ========================================================================
 
 $('#checkall:input').change(function() {
 	var c = $(this).attr('checked');
 	$('[name^=multiedit]').attr('checked',c);
 });
	

// ========================================================================
// ROW FILTER:
// ========================================================================
 
 $('#rowfilter').blur(function() 
 	{
 	



 	var t = $(this).val();
/*
	if (t == "") 
 		{
 		t = ".";
		$("table.tablesorter").tablesorterPager({
			size:500,
			page:0,
			container: $("#pager"),
			positionFixed: false
			});
		}
	else
		{
		$("table.tablesorter").tablesorterPager({
			size:500,
			page:0,
			container: $("#pager"),
			positionFixed: false
			});
		}		
*/

 	if (t == "") 
 		{
 		t = ".";
		}		


	var s = new RegExp (t, 'gi');
 	$('tr.listing').each(function() 
 		{
 		showrow = false;
 		$(this).children().filter('td').each (function() {
			var c = $(this).html().replace(/<.*?>/gi,'');
			if (c.search(s) >  0)
				{
				showrow = true;
				}
	 	});
	 	//alert (showrow);
	 	if (showrow)
		 	{
		 	$(this).show();
		 	}
		 	else
		 	{
		 	$(this).hide();
		 	}
	 	
 		}
 	);
	}
);

$('#clearrowfilter').click(function()
	{
	$('#rowfilter').val('').blur();
	}
);

// ========================================================================
// Add a "Are you sure" to anything with a "verify" class.
// A checkbox with a 'verify' will uncheck if not verified
// ========================================================================

$('.verify').click(function() {
	var t = $(this).attr('title');
	if (!confirm(t)) {
		$(this).attr('checked',false);
	}
});

// ========================================================================
// Animated collapsing
// ========================================================================

/*
animatedcollapse.addDiv('jason', 'optional_attribute_string')
//additional addDiv() call...
//additional addDiv() call...
animatedcollapse.init()
*/

// ========================================================================
// Activate the Save, Save and Close, Cancel buttons
// ========================================================================

$("button.close-window").click(function()
	{
	window.close();
	}
);



// Well, I worry this is over-complicated; let's try using plain ol' submits for now.
/*
	$('.savebutton').click(function() {
		$('input[name=action]').val('update');
	 	$(this).parent().submit();
	});
	
	$('.saveclosebutton').click(function() {
		$('input[name=action]').val('update_close');
	 	$(this).parent().submit();
	});

	$('.cancelbutton').click(function() {
		$('input[name=action]').val('show_list');
	 	$(this).parent().submit();
	});
*/


/*
	================================================================
	APPLY JQUERY UI TO STUFF (SO IT SHOWS)
	================================================================
*/
	// Only apply to marked buttons! Otherwise, throws off buttons of my own design.
	// Probably a bad idea, but how else to have my own buttons that don't get screwed up?
	$('.ui-button').button();



});


