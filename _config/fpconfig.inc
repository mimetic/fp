<?php

// SYSTEM CONFIG FILE FOR FP PICTURE GALLERY SYSTEM
// Localizing system settings, e.g. database
//
// Unchanging settings are in sysconfig.inc
// Localizing visual and functional site settings are in config.inc
// version January 22, 2010 1:40:31 AM PST

// ============================================================
// Name and contact info

// Server system account name
$FP_ACCOUNT_NAME = "dgross";
define ("FP_ACCOUNT_NAME", $FP_ACCOUNT_NAME);

// Domain name
$FP_DOMAIN_NAME = "mimetic.com";
define ("FP_DOMAIN_NAME", $FP_DOMAIN_NAME);

// MUST be UNIX file name friendly! No spaces, etc.
$SYSTEMNAME = "laptop_galleries";
define("SYSTEMNAME", $SYSTEMNAME);

// Name of system to show to users, e.g. in titles
$SYSTEM_DISPLAY_NAME = "Laptop & Jonés, Galleries";
define("FP_SYSTEM_DISPLAY_NAME", $SYSTEM_DISPLAY_NAME);

$FP_INFO_EMAIL = "info@mimetic.com";
define("FP_INFO_EMAIL", $FP_INFO_EMAIL);

$FP_COMMERCE_EMAIL = "sales@mimetic.com";
define ("FP_COMMERCE_EMAIL", $FP_COMMERCE_EMAIL);

// This will block everything, excluding admin.php
// Not so useful...you can't test!
$FP_UNDER_CONSTRUCTION = 0;
$FP_FORCE_SHOW = isset($_REQUEST['forceshow']);

if (!$FP_FORCE_SHOW && $FP_UNDER_CONSTRUCTION && basename ($_SERVER['PHP_SELF']) != "admin.php") {
	$page = file_get_contents ("underconstruction.html");
	$page = str_replace ("{SYSTEM_DISPLAY_NAME}", $SYSTEM_DISPLAY_NAME, $page);
	print $page;
	exit;
}

// MYSQL connect settings
// Do these early, so we can check the DB if we need to
// ---------------------
define ("MYSQLHOST", "localhost");
define ("MYSQLDB", FP_ACCOUNT_NAME."_fp");
define ("MYSQLUSER", FP_ACCOUNT_NAME."_fp");
define ("MYSQLPASSWORD", "SHI79W2qTdUy");


/*
USER TYPE (USER LEVEL)

This setting applies to the SYSTEM ADMINISTRATOR, and limits how the system can be used.

The setting here effects the initial entry into the site via index.php
FP_SINGLE_GALLERY_SINGLE_USER jumps to list of projects
FP_SINGLE_GALLERY_MULTI_USER jumps to the public gallery, to the list of exhibitions

1=FP_SINGLE_GALLERY_SINGLE_USER cannot add galleries or users
2=FP_SINGLE_GALLERY_MULTI_USER cannot add galleries
3=FP_MULTI_GALLERY_SINGLE_USER can add new galleries, but not users
4=FP_MULTI_GALLERY_MULTI_USER can add new galleries, and new users

We set the default here, but it could change if the admin user changes his subscription.
*/

$FP_GALLERY_TYPE = FP_MULTI_GALLERY_MULTI_USER;
define ("FP_GALLERY_TYPE", $FP_GALLERY_TYPE);

/*
INSTALLATION TYPE
Private installation or Public?
A private installation is for a person or group, and the subscription 
is for the whole server. 
A public installation is owned by me and subscriptions are for accounts
in system. 
The difference is that private installations have admin control, which
we have to limit, and they buy subscriptions differently from users of
public installations.
FP_INSTALLATION_PRIVATE
FP_INSTALLATION_PUBLIC
*/

$FP_INSTALLATION_TYPE = FP_INSTALLATION_PRIVATE;
define ("FP_INSTALLATION_TYPE", $FP_INSTALLATION_TYPE);

/*
SUBSCRIPTION SERVICE ID
Which subscription set do we offer in the signup system (called 'pp')?
*/
define ('PP_SERVICE_ID', PP_INSTALLATION_PRIVATE);

// used to find the fp_private directory, outside the www directory
$FP_HOME_REALPATH = "/Users/dgross/Sites/fp";


//-------------------
// Sign-up

// Is sign-up available?
$FP_SIGNUP_OPEN = true;
define ("FP_SIGNUP_OPEN", $FP_SIGNUP_OPEN);

// Special code to get in...
// This should be something editable, no? Not sure...
define ("FP_SIGNUP_CODE", "");


// ================ Development ================
// If this is true, the system URL is set (below) to a testing URL
// which matches the PayPal Sandbox IPN URL. If TRUE, the URL of the
// system should match the URL of the real PayPal IPN URL.
$DEVELOPING = true;

// IP or Domain name for developing, e.g. "24.34.56.123" or "testingsite.com"
// If developing, e.g. on the laptop, set the system URL to work
// This could be an IP or a domain name, plus any path to a subdirectory needed.
// It MUST end with a slash.
// Example: "mytestsite.com/"
$DEV_SYSTEMURL = "localhost/fp/";
// used to find the fp_private directory, outside the www directory
$DEV_FP_HOME_REALPATH = "/Users/dgross/Sites/fp";

// Make payments?
// If set, then no payments are made to supplier/artist/commission
// Orders are not sent by email to supplier, only to sysadmin
// Instead, any money just sits there.
define ("FP_NO_ORDERS", true);

$FP_SYSTEM_EMAIL = "dgross@mimetic.com";
define ("FP_SYSTEM_EMAIL", $FP_SYSTEM_EMAIL);

// Google Analytics Tracking code
define ("FP_GOOGLE_ANALYTICS_CODE", "UA-2020862-2");

// Google Adwords Conversion codes
// Signup page:
define ("FP_GOOGLE_CONVERSION_ID", "");
define ("FP_GOOGLE_CONVERSION_LABEL", "");
// Checkout page:
define ("FP_GOOGLE_CONVERSION_ID_CHECKOUT", "");
define ("FP_GOOGLE_CONVERSION_LABEL_CHECKOUT", "");

// Can the admin user change the website commission// Can the admin user change the website commission for each artist? Lock this if you want to take
// a commission from all users, so they can't change it.
define ("FP_ADMIN_CHANGE_COMMISSION", false);

// This determines whether we use the PayPal sandbox for testing
// or we use the real system for payments (both receiving AND mass payments)
$FP_USE_PAYPAL_SANDBOX = true;
define ("FP_USE_PAYPAL_SANDBOX", $FP_USE_PAYPAL_SANDBOX);

// PAYPAL SALES SETTINGS
if (FP_USE_PAYPAL_SANDBOX) {
	// TESTING DATA
	define ("PAYPAL_POST_TO_URL", "www.sandbox.paypal.com");
	$post_to_URL = "www.sandbox.paypal.com"; 	//	use this URL for testing
	//	This is YOUR Sandbox PayPal ID, or email address. This email address must be confirmed and linked to your Verified Business or Premier account
	define ("PAYPAL_BUSINESS_EMAIL", "bear@mimetic.com") ;	
	$business = "bear@mimetic.com";	
	// SYSTEM OWNER PAYPAL BUSINESS EMAIL for receiving commissions
	// This is not the same as the sales account. It's the owners personal account.
	define ("PAYPAL_COMMISSIONS_EMAIL", "buyer2_1210406142_per@mimetic.com") ;	
	// email address where script should send notifications
	define ("PAYPAL_ERROR_EMAIL", "dgross@mimetic.com") ;
} else {
	// LIVE DATA
	define ("PAYPAL_POST_TO_URL", "www.paypal.com");
	//	This is YOUR real PayPal ID, or email address. This email address must be confirmed and linked to your Verified Business or Premier account
	define ("PAYPAL_BUSINESS_EMAIL", "dgross@mimetic.com") ;	
	$business = "dgross@mimetic.com";	
	// SYSTEM OWNER PAYPAL BUSINESS EMAIL for receiving commissions
	// This is not the same as the sales account. It's the owners personal account.
	define ("PAYPAL_COMMISSIONS_EMAIL", "private@mimetic.com") ;	
	// email address where script should send notifications
	define ("PAYPAL_ERROR_EMAIL", "dgross@mimetic.com") ;
}

$PAYPAL_POST_TO_URL = PAYPAL_POST_TO_URL;


// Site language
// "" = english
// fr = french
$FP_LANGUAGE = "";
define ("FP_LANGUAGE", $FP_LANGUAGE);


// Shippers: ups, usps, fedex
// Code is used to call different code snippets, e.g ups_entry_form or usps_entry_form
// where the user must enter shipping info.
$FP_SHIPPER_CODE = "fpcart_ups";
define ("FP_SHIPPER_CODE", $FP_SHIPPER_CODE);


// ===== DON'T CHANGE BELOW HERE ===========================
// Set the system emailer URL, e.g. www.frontline-photos.com/phpemaillist/addmessage.php
if ($DEVELOPING) {
	define ("FP_SMTP_PORT", 587);
	$SYSTEMURL = $DEV_SYSTEMURL;
	$FP_HOME_REALPATH = $DEV_FP_HOME_REALPATH;
	$FP_EMAIL_MAILDIR = "/Users/dgross/Documents/FP/_DEV/image_extractor/pix/new";
	define ("FP_DISABLE_USERVOICE_FEEDBACK","disable");
} else {
// Mail settings for receiving pictures and files by email
	// Path to new mail that should be parsed: used to build /home/username/mail/mydomain.com/pix/new)
	$FP_EMAIL_MAILDIR = "/home/".FP_ACCOUNT_NAME."/mail/".FP_DOMAIN_NAME."/$FP_EMAIL_ACCOUNT/new";
	define ("FP_SMTP_PORT", 25);
	define ("FP_DISABLE_USERVOICE_FEEDBACK","");
}

$FP_EMAILER_UNSUBSCRIBE = $SYSTEMURL."phpemaillist/unsubscribe.php";
$FP_EMAILER = $SYSTEMURL . "phpemaillist/addmessage.php";
$FP_EMAILER_ADMIN = $SYSTEMURL . "phpemaillist/";
define ('FP_EMAILER_ADMIN', $FP_EMAILER_ADMIN);
define ("FP_EMAILER", $FP_EMAILER);
define ("FP_EMAILER_UNSUBSCRIBE", $FP_EMAILER_UNSUBSCRIBE);
define ("FP_HOME_REALPATH", $FP_HOME_REALPATH);

if (file_exists(FP_HOME_REALPATH . "/fp_private/pp_secret.txt")) {
	$PP_SECRET = trim(file_get_contents (FP_HOME_REALPATH . "/fp_private/pp_secret.txt"));
} else {
	$PP_SECRET = "";
}
define ('PP_SECRET', $PP_SECRET);
?>