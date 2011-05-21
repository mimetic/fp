/*!
 * shadowTilt (with Option Definition)
 * Copyright (c) 2010 Joe Watkins
 * Version: 1.0 (03-03-2010)
 * Dual licensed under the MIT and GPL licenses:
 * http://www.opensource.org/licenses/mit-license.php
 * http://www.gnu.org/licenses/gpl.html
 * Requires: jQuery v1.2.6 or later
 */

(function($){

 	$.fn.extend({ 
 		
		//pass the options variable to the function
 		shadowTilt: function(options) {
	
			//Set the default values, use comma to separate the settings, example:
			var defaults = {
				
				padding : '4px', // width of padding around image
				borderThickness : "1px", // border thickness around image
				borderType : 'solid', // border type eg. dashed, dotted, solid
				borderColor : '#ddd', // border color
				offset_1 : "5px", // off shadow offsets
				offset_2 : "5px", // off shadow offsets
				offset_3 : "5px", // off shadow offsets
				offset_1_hover : "8px", // hover shadow offsets
				offset_2_hover : "8px", // hover shadow offsets
				offset_3_hover : "8px", // hover shadow offsets
				shadowColor : '#999', // shadow color
				fadeOut : "fast", // speed of fade out
				fadeIn : "fast", // speed of fade in
				startOpac : ".8", // beginning opacity
				endOpac : "1", // ending opacity
				enableTilt : true, // turn tilt on/off - true, false
				enableFade : true, // turn fade on/off - true, false
				tilt : "10", // default degrees of tilt
				random : false, // randomize tilt
				randomMax : 20 // largest degree for random
		
			}
				
			var options =  $.extend(defaults, options);

    		return this.each(function() {
				var o = options;
				var obj = $(this);
				
				// padding & border
				$(this).css('padding',o.padding).css('border',''+ o.borderThickness +' '+ o.borderType + ' ' + o.borderColor);
				
				// shadow
				$(this).css("-moz-box-shadow",' ' + o.offset_1 +' '+ o.offset_2 + ' ' + o.offset_3 + ' ' + o.shadowColor );
				$(this).css("box-shadow",' ' + o.offset_1 +' '+ o.offset_2 + ' ' + o.offset_3 + ' ' + o.shadowColor );
				$(this).css("-webkit-box-shadow",' ' + o.offset_1 +' '+ o.offset_2 + ' ' + o.offset_3 + ' ' + o.shadowColor );
				
				$(this).stop().animate({ opacity: o.startOpac }, o.fadeOut);
				
					$(this).hover(function(){
						if(o.enableFade == true){
							$(this).stop().animate({ opacity: o.endOpac }, o.fadeOut);
						}
							if(o.enableTilt == true){
								//$(this).addClass(o.moveClass);
								if(o.random == true){ 
									var tilt=Math.floor(Math.random()*o.randomMax); 
									var x = Math.floor(Math.random()*2); 
								
									if(x == 0){
										$(this).css("-webkit-transform","rotate("+tilt+"deg)");
										$(this).css("-moz-transform","rotate("+tilt+"deg)");
									
									}
									if(x == 1){
										$(this).css("-webkit-transform","rotate(-"+tilt+"deg)");
										$(this).css("-moz-transform","rotate(-"+tilt+"deg)");
									
									}
								}else{
									$(this).css("-webkit-transform","rotate(-"+o.tilt+"deg)");
									$(this).css("-moz-transform","rotate(-"+o.tilt+"deg)");
								}
							}
					
							$(this).css("-moz-box-shadow",' ' + o.offset_1_hover +' '+ o.offset_2_hover + ' ' + o.offset_3_hover + ' ' + o.shadowColor );
							$(this).css("box-shadow",' ' + o.offset_1_hover +' '+ o.offset_2_hover + ' ' + o.offset_3_hover + ' ' + o.shadowColor );
							$(this).css("-webkit-box-shadow",' ' + o.offset_1_hover +' '+ o.offset_2_hover + ' ' + o.offset_3_hover + ' ' + o.shadowColor );
						
					
					},function(){   
						if(o.enableFade == true){
							$(this).stop().animate({ opacity: o.startOpac }, o.fadeIn);
						}
						$(this).removeClass(o.moveClass);
			
							$(this).css("-moz-box-shadow",' ' + o.offset_1 +' '+ o.offset_2 + ' ' + o.offset_3 + ' ' + o.shadowColor );
							$(this).css("box-shadow",' ' + o.offset_1 +' '+ o.offset_2 + ' ' + o.offset_3 + ' ' + o.shadowColor );
							$(this).css("-webkit-box-shadow",' ' + o.offset_1 +' '+ o.offset_2 + ' ' + o.offset_3 + ' ' + o.shadowColor );
						
							$(this).css("-webkit-transform","rotate(0deg)");
							$(this).css("-moz-transform","rotate(0deg)");
						
				
				
					}); // hover
				
				}); // return
    	
		} // function 
		
	}); // extend
	
})(jQuery); // end