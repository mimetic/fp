// ----------------
// FP JAVASCRIPTS

var preloadFlag = false;

// global variable to capture the currently popped up image
var CurrentPopupID ;

function obfuscate(name,domain) {
			var scheme = new Array('m','a','i','l','t','o',':');
			var directive = '<a href="' + scheme.join('') + name + '&#64;' + domain + '">' + name + '&#64' + domain + '<\/a>';
			document.write(directive); 
	} 

function closeme () {
	window.close ();
}

function clearAllSelect (selectId)
{
	document.getElementById(selectId).length = 0;
}

function appendOptionLast(selectId, t, v)
{
	var elOptNew = document.createElement('option');
	elOptNew.text = t;
	elOptNew.value = v;
	var elSel = document.getElementById(selectId);

	try {
		elSel.add(elOptNew, null); // standards compliant; doesn't work in IE
	}
	catch(ex) {
		elSel.add(elOptNew); // IE only
	}
}

// Show/Hide popup text in the <div id=popup_text></div>
function showhide (id) {
	viz = document.getElementById(id).style.display;
	viz == "none" ? document.getElementById(id).style.display = "block" :	 document.getElementById(id).style.display = "none";
}

// show package description
function showPackageDescription (myID) {
	x = document.getElementById("packageID").selectedIndex;
	k = package_keys[x];
	content = package_desc[x];
	dq(myID, content);
}

// Write content to the screen
function dq(myID, content){
	if (document.getElementById(myID) != null) { 
		content = URLDecode (content);
		var ie4=document.all&&navigator.userAgent.indexOf("Opera")==-1;
		var ns4=document.layers;
		var ns6=document.getElementById&&navigator.userAgent.indexOf("Opera")==-1;
	
		if(ie4){showContentObj=document.all.myID;}
		if(ns6){showContentObj=document.getElementById(myID);}
		
		if(ie4||ns6){
			showContentObj.innerHTML=content;
		}
		if(ns4){
			myID = "ns_" + myID;
			document.myID.document.write(content);
			document.myID.document.close();
		}
	}
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
				alert( 'Bad escape combination near ...' + encoded.substr(i) );
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


// This code was written by Tyler Akins and has been placed in the
// public domain.	 It would be nice if you left this header intact.
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
