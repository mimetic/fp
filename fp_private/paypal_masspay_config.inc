<?php

// Credential: API Signature
// Request Date: Apr. 20, 2008 13:01:24 PDT

// for SECURITY, WE KEEP REAL CREDENTIALS LOCKED AWAY.

if (FP_USE_PAYPAL_SANDBOX) {
	// PayPal Sandbox API Credentials
	// for bear@mimetic.com
	
	define("MASSPAY_USER_NAME", "bear_api1.mimetic.com");
	define("MASSPAY_PASSWORD", "KZ97X5LQQTG9WKN4");
	define("MASSPAY_SIGNATURE", "AZsoPCx2cm9jxVOxQrvd3jXGUuYwA0DHhKhapFzQokoHEOIhmc.GTXMR");
	define("MASSPAY_PAYPAL_URL", "https://api-3t.sandbox.paypal.com/nvp");
} else {
	// PayPal API Credentials
	define("MASSPAY_USER_NAME", "bear_api1.mimetic.com");
	define("MASSPAY_PASSWORD", "KZ97X5LQQTG9WKN4");
	define("MASSPAY_SIGNATURE", "AZsoPCx2cm9jxVOxQrvd3jXGUuYwA0DHhKhapFzQokoHEOIhmc.GTXMR");
	define("MASSPAY_PAYPAL_URL", "https://api-3t.sandbox.paypal.com/nvp");
}

define ("MASSPAY_VERSION", "3.2") ;

?>