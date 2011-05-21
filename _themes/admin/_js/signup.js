/*
	Admin Signup Javascript
	name: signup.js
	
	A valid entry in the captcha shows the final "submit" button.
*/

// jQuery-based functions
$(document).ready( function() {


	// show obfuscated email addresses which have class=obfuscate
	$(".obfuscated").defuscate();
	
	$('input.captcha').keyup(function() {
		var x = $(this).val();
		$.get("captcha/check.php", "key="+x, function(res) {
			res = JSON.parse(res);
			if (res) {
				$('#captcha-response').html('<input name="captcha" type="hidden" value="ok">');
				$("button#submit").fadeIn();
			} else {
				$('#captcha-response').html('');
				$("button#submit").fadeOut();
			}
		}
		);
	});

	// Only apply to marked buttons! Otherwise, throws off buttons of my own design.
	// Probably a bad idea, but how else to have my own buttons that don't get screwed up?
	$('.ui-button').button();

});


