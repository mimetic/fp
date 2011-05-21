<?php

// Credential: API Signature
// Request Date: Apr. 20, 2008 13:01:24 PDT

if (FP_USE_PAYPAL_SANDBOX) {
	// PayPal Sandbox API Credentials
	// for bear@mimetic.com
	
	define("MASSPAY_USER_NAME", "bear_api1.mimetic.com");
	define("MASSPAY_PASSWORD", "KZ97X5LQQTG9WKN4");
	define("MASSPAY_SIGNATURE", "AZsoPCx2cm9jxVOxQrvd3jXGUuYwA0DHhKhapFzQokoHEOIhmc.GTXMR");
	define("MASSPAY_PAYPAL_URL", "https://api-3t.sandbox.paypal.com/nvp");
} else {
	// PayPal API Credentials
	define("MASSPAY_USER_NAME", "dgross_api1.mimetic.com");
	define("MASSPAY_PASSWORD", "YL7GZ89RKCNXAX8X");
	define("MASSPAY_SIGNATURE", "AHEnEPj7ffrzfpx.jfMlxc3p0J1TASOQsR8t1YnEAgZnSRNAUdV5Faas");
	define("MASSPAY_PAYPAL_URL", "https://api-3t.paypal.com/nvp");

}

define ("MASSPAY_VERSION", "3.2") ;

?>