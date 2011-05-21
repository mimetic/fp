	function Validate() {
		if (document.loginform.Firstname.value.length <= 0) {
			alert ("Enter your first name.");
			document.loginform.Firstname.focus();
			return false;	
		}
	
		if (document.loginform.Lastname.value.length <= 0) {
			alert ("Enter your last name.");
			document.loginform.Lastname.focus();
			return false;	
		}
	
		if (document.loginform.Email.value.length <= 0) {
			alert ("Enter your email address.");
			document.loginform.Email.focus();
			return false;	
		}
	
		var passed = validatePassword(document.loginform.Password.value, {
			length:   [12, Infinity],
			lower:    1,
			upper:    1,
			numeric:  1,
			special:  0,
			badWords: ["Password", document.loginform.Lastname.value, document.loginform.Firstname.value],
			badSequenceLength: 4
		});
		if (!passed ) {
			alert ("Your password must be at least 12 characters long, contain upper and lowercase letters, at least one number, cannot contain a series (e.g. 1234) and cannot contain your name.");
			document.loginform.Email.focus();
			return false;	
		}
	
		if (document.loginform.Email.value != document.loginform.EmailConfirm.value) {
			alert ("Your confirmed email address does not match the entered email address.");
			document.loginform.EmailConfirm.focus();
			return false;	
		}
	
		if (document.loginform.Referral.value.length < 10 ) {
			alert ("Please enter at least 10 letters to tell us where you heard about us.");
			document.loginform.Referral.focus();
			return false;	
		}
		
		if ( !document.loginform.user_terms_of_service.checked ) {
			alert ("Please agree to the terms of service.");
			return false;	
		}
		
		/*
		// IF USING CAPTCHA
		if (document.loginform.captchaOK.value != "OK") {
			alert ("Fill out the Captcha and prove you are a human!");
			return false;	
		}
		*/	
	
	/*
		if (document.loginform.Password.value.length < 8) {
			alert ("Enter a password at least 8 characters long.");
			var problem = true;	
		}
		
		if (document.loginform.Password.value != document.loginform.PasswordConfirm.value) {
			alert ("Your confirmed password does not match the entered password.");
			var problem = true;	
		}
	*/
		
		return true;
	}
	
	// Generate Password
	function getRandomNum(lbound, ubound) {
		return (Math.floor(Math.random() * (ubound - lbound)) + lbound);
	}
	
	function getRandomChar(number, lower, upper, other, extra) {
		var numberChars = "0123456789";
		var lowerChars = "abcdefghijklmnopqrstuvwxyz";
		var upperChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		var otherChars = "`~!@#$%^&*()-_=+[{]}\\|;:'\",<.>/? ";
		var charSet = extra;
		
		if (number == true)
			charSet += numberChars;
		if (lower == true)
			charSet += lowerChars;
		if (upper == true)
			charSet += upperChars;
		if (other == true)
			charSet += otherChars;
		return charSet.charAt(getRandomNum(0, charSet.length));
		}
	
	function getPassword(length, extraChars, firstNumber, firstLower, firstUpper, firstOther, latterNumber, latterLower, latterUpper, latterOther) {
		var rc = "";
		if (length > 0)
		rc = rc + getRandomChar(firstNumber, firstLower, firstUpper, firstOther, extraChars);
		for (var idx = 1; idx < length; ++idx) {
			rc = rc + getRandomChar(latterNumber, latterLower, latterUpper, latterOther, extraChars);
		}
		return rc;
	}
	
	/*
		Password Validator 0.1
		(c) 2007 Steven Levithan <stevenlevithan.com>
		MIT License
	*/
	
	function validatePassword (pw, options) {
		// default options (allows any password)
		var o = {
			lower:    0,
			upper:    0,
			alpha:    0, /* lower + upper */
			numeric:  0,
			special:  0,
			length:   [0, Infinity],
			custom:   [ /* regexes and/or functions */ ],
			badWords: [],
			badSequenceLength: 0,
			noQwertySequences: false,
			noSequential:      false
		};
	
		for (var property in options)
			o[property] = options[property];
	
		var	re = {
				lower:   /[a-z]/g,
				upper:   /[A-Z]/g,
				alpha:   /[A-Z]/gi,
				numeric: /[0-9]/g,
				special: /[\W_]/g
			},
			rule, i;
	
		// enforce min/max length
		if (pw.length < o.length[0] || pw.length > o.length[1])
			return false;
	
		// enforce lower/upper/alpha/numeric/special rules
		for (rule in re) {
			if ((pw.match(re[rule]) || []).length < o[rule])
				return false;
		}
	
		// enforce word ban (case insensitive)
		for (i = 0; i < o.badWords.length; i++) {
			if (pw.toLowerCase().indexOf(o.badWords[i].toLowerCase()) > -1)
				return false;
		}
	
		// enforce the no sequential, identical characters rule
		if (o.noSequential && /([\S\s])\1/.test(pw))
			return false;
	
		// enforce alphanumeric/qwerty sequence ban rules
		if (o.badSequenceLength) {
			var	lower   = "abcdefghijklmnopqrstuvwxyz",
				upper   = lower.toUpperCase(),
				numbers = "0123456789",
				qwerty  = "qwertyuiopasdfghjklzxcvbnm",
				start   = o.badSequenceLength - 1,
				seq     = "_" + pw.slice(0, start);
			for (i = start; i < pw.length; i++) {
				seq = seq.slice(1) + pw.charAt(i);
				if (
					lower.indexOf(seq)   > -1 ||
					upper.indexOf(seq)   > -1 ||
					numbers.indexOf(seq) > -1 ||
					(o.noQwertySequences && qwerty.indexOf(seq) > -1)
				) {
					return false;
				}
			}
		}
	
		// enforce custom regex/function rules
		for (i = 0; i < o.custom.length; i++) {
			rule = o.custom[i];
			if (rule instanceof RegExp) {
				if (!rule.test(pw))
					return false;
			} else if (rule instanceof Function) {
				if (!rule(pw))
					return false;
			}
		}
	
		// great success!
		return true;
	}
