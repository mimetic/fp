
/*	plugin 
	jquery.linkcolors.js
	
	Handle link highlighting with javascript instead of CSS, so
	we can dynamically change it.
	
*/

$.fn.LinkColors = function() {
	return this.each(function() {
		$(this).mouseover(function() {
			$(this).addClass('hover');
		})
		.mouseout(function() {
			$(this).removeClass('hover');
		})
		;
	});

};