<?PHP

/*
	PP Config file for LOCAL TESTING

/*
Configuration file for the storefront for selling subscriptions and such
using PayPal.

This site sells the services, and updates other sites' databases to reflect 
that a sale was made. There is a "standardized" database table that it writes
to in each site, and the site then does with that info whatever it wants.

SETUP: 

IMPORTANT: Remember to set your PayPal settings in your sellers account:
	- My Account : Profile : Instant Payment Notification Preferences
	- Set the IPN URL to http://www.mysellersite.com/pp/process.php (assuming pp is in a pp directory)

*/

// Report simple running errors
error_reporting  (E_ERROR | E_WARNING | E_PARSE); 

$BASEDIR = dirname (__FILE__);

// ============== DEFAULT CONFIG ==============

$SYSTEMNAME = "Mimetic Galleries";

// Server account user: used to determine the working
// directory and the mysql db name
$PP_USER = "dgross";

/// MYSQL connect settings
// It's unlikely you'll have to change these for each service,
// assuming your PP is on the SAME SERVER as your service. It's OK to
// have different accounts, as long as they share the same MYSQL server localhost.
// ---------------------
$MYSQLHOST = "localhost";
$MYSQLDB = "{$PP_USER}_fp";
$MYSQLUSER = "{$PP_USER}_fp";
$MYSQLPASSWORD = "SHI79W2qTdUy";

// the fp_private directory, outside the www directory
$PP_SECRET_DIR = "/home/$PP_USER/fp_private";
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

// Use user config file if it exists
file_exists ("my_config.inc") && include ("my_config.inc");

// ============== MAKE NO CHANGES BELOW THIS LINE ==============
isset ($_SERVER['SCRIPT_FILENAME']) && ($BASEDIR = dirname($_SERVER['SCRIPT_FILENAME']));

define ("PP_SYSTEM_DISPLAY_NAME", $SYSTEMNAME);

define ("MYSQLHOST", $MYSQLHOST);
define ("MYSQLDB", $MYSQLDB);
define ("MYSQLUSER", $MYSQLUSER);
define ("MYSQLPASSWORD", $MYSQLPASSWORD);
define ("RECEIVER_MAIL", $PP_RECEIVER_MAIL) ;	

// ================================
// HTTP_HOST works when using an IP address, for example, while SERVER_NAME fails
// ** if called from the command line, there will be no server name or http_host **
if (isset ($_SERVER['HTTP_HOST'])) {
	$SERVERNAME = $_SERVER['HTTP_HOST'];
} else {
	if (isset($_SERVER['SERVER_NAME'])) {
		$SERVERNAME = $_SERVER['SERVER_NAME'];
	} else {
		$SERVERNAME = "";
	}
}
$SYSTEMURL	= $SERVERNAME .  dirname ($_SERVER['PHP_SELF']) . "/";
$SYSTEMURL = preg_replace ("|//$|", "/", $SYSTEMURL);

$PP_SECRET = trim(file_get_contents ("$PP_SECRET_DIR/pp_secret.txt"));
define ('PP_SECRET', $PP_SECRET);

// Log file
define ("LOG", "../log/pp.log");
touch (LOG);

// directory of code pieces
$SNIPPETS 	=	"snippets";

$SERVICEURL = $SYSTEMURL."admin.php";	// return home link in case of cancellation (same as $return below?)
$COOKIELIFE = 1*2*3600;	 // cookie lifetime in seconds (e.g. here: 2 hour)

// IPN transfer back URL. Where PayPal sends info about transactions
// By filling this in, you override the setting on the Paypal site,
// allowing multiple sites to use the PayPal account
$notify_url = "https://" . $SYSTEMURL . "process.php";

// If True, send a confirming email to users after they subscribe
// PayPal already does this, by the way
define ("SEND_EMAIL", FALSE);


$PP_NO_NOTE = "1";	//	absolutely required for subscription processing... This field must be included, and the value must be set to 1
$PP_SRA = "1";	//	Reattempt on failure. If set to "1," and the payment fails, the payment will be reattempted two more times. After the third failure, the subscription will be cancelled. If omitted and the payment fails, payment will not be reattempted and the subscription will be immediately cancelled
$PP_SRC = "1";	//	Recurring payments. If set to "1," the payment will recur unless your customer cancels the subscription before the end of the billing cycle. If omitted, the subscription payment will not recur at the end of the billing cycle
$PP_SRT = null;	//	Recurring Times. This is the number of payments which will occur at the regular rate. If omitted, payment will continue to recur at the regular rate until the subscription is cancelled
$PP_NO_SHIPPING = "1";	//	Shipping address. If set to "1," your customer will not be prompted for a shipping address. If omitted or set to "0," your customer will be prompted to include a shipping address
$PP_RM = "2";	//	Return Method. Set to '2' this makes all the Subscription vars available to your 'return' URL via POST method. Set to '1' and they'll be available via GET method (too visible). Set to '0' and the vars won't be available at all.
$PP_MODIFY = "1";	//	'0'= new subscription signup only, '1'= modify existing or signup for new subscriptions, '2'= modify existing subscription only
$PP_CURRENCY_CODE = "USD";

?>
