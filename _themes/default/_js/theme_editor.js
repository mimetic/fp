/*
	CSS Theme Editor activation
	This script uses my jquery.cssedit.js plugin to allow CSS editing of marked items on any page.
	Also requires json2.js
	
*/

$(document).ready(function() {
	// Is the editing cookie set?
	var cookieName = "cssEditSwitch";
	var c = $.cookies.get(cookieName);
	var myCookie;

	if (c) {
		myCookie = JSON.parse(c);
	}
	
	if (myCookie && myCookie.status == "edit") {
		$('#message, #error').appendTo('BODY').addClass('cssEdit');
		
		// Make the following all editable classes
		// Do NOT add 'body' to this list. Oh, what a nightmare that is.
		//var list = ".navbar, .navbar a, .navbar .unselected, .navbardivider, .sspop a, .captionblock .title, .navbar-container, .gallery-description-divider, .list .item, .list .place, .list .caption, .list .subtitle, .list .title, #mainlist";
		var list = "a.navbar.selected, a.navbar.unselected";
		$(list).addClass('editable');

		$('.editable').cssEdit(FP_THEME_ID, FP_THEME_NAME);
	}
});

/*
// http://frinity.blogspot.com/2008/06/switch-css-stylesheets-using-jquery.html
function switchStyle(styleName) {
	$('link[@rel*=style][title]').each(function(i) {
		this.disabled = true;
		if (this.getAttribute('title') == styleName) this.disabled = false;
	});
}
*/