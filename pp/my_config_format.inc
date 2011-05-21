<?php
/*
USER's config file for PP.
The config.inc is now the default config file.
This file will not be changed by updates.
*/


// ============== MY CONFIG ==============

$SYSTEMNAME = "Mimetic Galleries";

// Server account user: used to determine the working
// directory and the mysql db name
$PP_USER = "dgross";
$PP_DB_NAME = "fp";

/// MYSQL connect settings
// ---------------------
$MYSQLHOST = "localhost";
$MYSQLDB = "{$PP_USER}_{$PP_DB_NAME}";
$MYSQLUSER = "{$PP_USER}_{$PP_DB_NAME}";
$MYSQLPASSWORD = "SHI79W2qTdUy";

// the fp_private directory, outside the www directory
$PP_SECRET_DIR = "/home/$PP_USER/fp_private";
// Testing:
//$PP_SECRET_DIR = "/users/dgross/Sites/fp/fp_private";

/*
PayPal Subscription variables and default values
These are the variables and values sent to PayPal unless you change them here OR 
override them by placing new values in the $PP_PACKAGES array below.
*/

$TESTING = false;

if ($TESTING) {
	// TESTING DATA
	$post_to_URL = "www.sandbox.paypal.com"; 	//	use this URL for testing
	//	This is YOUR Sandbox PayPal ID, or email address. This email address must be confirmed and linked to your Verified Business or Premier account
	$PP_RECEIVER_MAIL = "bear@mimetic.com";	
	$business = "bear@mimetic.com";
} else {
	// LIVE DATA
	$post_to_URL = "www.paypal.com"; 	//	use this URL when you go live 
	//	This is YOUR real PayPal ID, or email address. This email address must be confirmed and linked to your Verified Business or Premier account
	$PP_RECEIVER_MAIL = "dgross@mimetic.com";
	$business = "dgross@mimetic.com";	
}

?>