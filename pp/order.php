<?php 

/* 	Order a service.
	Show form to choose a service. Then, show form to choose details 
	of that service.
	
	Security issue: rather than pass all info about the package in an easily 
	spoofed way, it would be much better to have such info as a local file, as
	originally planned. Dammit.
*/

include "config.inc";
include "config_packages.inc";
include "includes/pp_functions.inc";
include "includes/PPUser.inc";
 
$error = "";
$message = "";

// first run?
$firstrun = isset($_POST['userID']);

// Start Session
session_name("pp_subscriptions");
session_start();

// Create: $secret, $service, $username, $password, $userID, $subscriptionid, $secret, $URL, $packages, $service

$firstrun && $_SESSION['oldPackageID'] = $_POST['packageID'];
empty($_SESSION['oldPackageID']) && $_SESSION['oldPackageID'] = 0;
$oldPackageID = $_SESSION['oldPackageID'];

extract ($_SESSION);
extract ($_POST);

isset($URL) ? $_SESSION['URL'] = $URL : $URL = $_POST['URL'];
// If cancel, return to calling program
if ($_POST['action'] == "cancel") {
 	header("Location: " . $_SESSION['URL']);
}

isset($secret) ? $_SESSION['secret'] = $secret : $secret = $_SESSION['secret'];
isset($userID) ? $_SESSION['userID'] = $userID : $userID = $_SESSION['userID'];
isset($password) ? $_SESSION['password'] = $password : $password = $_SESSION['password'];
isset($username) ? $_SESSION['username'] = $username : $username = $_SESSION['username'];
isset($serviceID) ? $_SESSION['serviceID'] = $serviceID : $serviceID = $_SESSION['serviceID'];

isset($coupon) ? $_SESSION['coupon'] = stripslashes ($coupon): $coupon = stripslashes($_SESSION['coupon']);
$coupon = stripslashes ($coupon);

isset($comments) ? $_SESSION['comments'] = stripslashes ($comments): $comments = stripslashes($_SESSION['comments']);
$comments = stripslashes ($comments);

// if packageID is not set, make it '0'
$packageID == 0 && $packageID = "0";
isset($packageID) ? $_SESSION['packageID'] = $packageID : $packageID = $_SESSION['packageID'];


// Other vars
isset ($first_name) ? $_SESSION['first_name'] = $first_name : $first_name = $_SESSION['first_name'];
isset ($last_name) ? $_SESSION['last_name'] = $last_name : $last_name = $_SESSION['last_name'];
isset ($address1) ? $_SESSION['address1'] = $address1 : $address1 = $_SESSION['address1'];
isset ($address2) ? $_SESSION['address2'] = $address2 : $address2 = $_SESSION['address2'];
isset ($city) ? $_SESSION['city'] = $city : $city = $_SESSION['city'];
isset ($state) ? $_SESSION['state'] = $state : $state = $_SESSION['state'];
isset ($zip) ? $_SESSION['zip'] = $zip : $zip = $_SESSION['zip'];

isset ($day_phone_a) ? $_SESSION['day_phone_a'] = $day_phone_a : $day_phone_a = $_SESSION['day_phone_a'];
isset ($day_phone_b) ? $_SESSION['day_phone_b'] = $day_phone_b : $day_phone_b = $_SESSION['day_phone_b'];
isset ($day_phone_c) ? $_SESSION['day_phone_c'] = $day_phone_c : $day_phone_c = $_SESSION['day_phone_c'];


// array for substitutions of {var} variables on output page
$vars = array ();

// Verify the order information:
// Service, user, password, package, terms of service, coupons
// This is sent to us by this script.

// Verify the secret code passed from the calling program
if (!Verify_PP_Secret ($secret,$userID)) {
	$error .= 'Security Error: Please return to your account and start again (system security indicates data was lost or altered).<BR>';
	$error_tally++;
}

// Service definition from calling website
$myService = null;
//isset($service) && $myService = unserialize(urldecode($service));
isset($PP_SERVICES[$serviceID]) && $myService = $PP_SERVICES[$serviceID];
if ($myService) {
	define ("PP_ORDERS_DB", $myService['orders']);
	define ("PP_DB_USERS", $myService['users']);
	define ("PP_ORDERS_ID", $myService['orderIDField']);
	define ("PP_USERS_ID", $myService['usersIDField']);

	$servicename = $myService['name'];
	$servicedesc = $myService['desc'];
	
	$mysqlhost = $myService['mysqlhost'];
	$mysqldb = $myService['mysqldb'];
	$mysqluser = $myService['mysqluser'];
	$mysqlpasswd = $myService['mysqlpasswd'];
	
	$LINK = StartDatabase( $mysqlhost, $mysqldb, $mysqluser, $mysqlpasswd);
} else {
	$error .= 'The definition of this service is missing. Please return to your account and start again.<BR>';
	$error_tally++;
}

// Get list of packages (options) available for this service
if ($PP_PACKAGES[$serviceID]) {
	$myPackages = $PP_PACKAGES[$serviceID];
} else {
	$error .= 'The definition of the packages for this service are missing. Please return to your account and start again.<BR>';
	$error_tally++;
}

if ($myPackages) {
	isset($packageID) || $packageID = 1;
	$pkgsToShow = $myPackages;
	unset ($pkgsToShow[$oldPackageID]);
	$package_dropdown .= BuildDropDown ($pkgsToShow, $packageID);
	
	$packagename = $myPackages[$packageID]['name'];
	$packagedesc = $myPackages[$packageID]['desc'];
}

// Verify the user exists.
// The service requested may require that the user be already signed up (probably for a demo account).
// This requires checking the service's user database.
if (!$error_tally) {
	if ( $username and $password and $userID ) {
		$username = strip_tags( $username );
		$password = strip_tags( $password );
		$userID = strip_tags( $userID );

		if ($myService['accountrequired']) {
			if (!VerifyUser ($myService, $username, $password, $userID) ) {
				$message .= "&#8226; Please join ". $myService[$service]['name'] . " before buying a subscription.<br>";
				$error_tally++;
			}
		} else {
			// create a new user/pass
			$message .= "THE SYSTEM SHOULD CREATE A NEW USER/PASS...but doesn't know how, yet.<BR>";
		}
	} else {
		$error .= 'Security Error: Please return to your account and start again. ('.__LINE__.')<BR>';
		$error_tally++;
	}
	
	// Create javascript variables containing the package info as arrays
	// Create array of keys
	if (!$error_tally) {
		$x = 1;
		$vars['PACKAGE_KEYS'] = "// Keys to packages arrays\n";
		$vars['PACKAGES'] = "// Packages arrays\n";
		$vars['PACKAGES_DESC'] = "// Package Description arrays\n";
		$pArr = array ();
		while (list($pid, $pkg) = each ($myPackages)) {
			if ($pid != $oldPackageID && !$pkg['disabled']) {
				$pkArr[$pid] = $pid;
				$pnArr[$pid] = $pkg["name"];
				$pdArr[$pid] = $pkg["desc"];
			}
		}
		$vars['PACKAGE_KEY'] .= BuildJSArrayString ("pkey", $pkArr);
		$vars['PACKAGE_NAME'] .= BuildJSArrayString ("pname", $pnArr);
		$vars['PACKAGE_DESC'] .= BuildJSArrayString ("pdesc", $pdArr);
	}


	// Verify the the  package chosen is not the one the user already has.
	if (!$firstrun && ($packageID == $oldPackageID)) {
		$message .= "&#8226; You already are subscribed to the package, <b>$packagename</b>. Please choose a different package.<BR>";
		$error_tally++;
	}

	// Verify the "terms" box has been ticked
	if (strtolower($action) == "submit") {
		if ( $Terms == 'on') {
			$terms = 'checked';
		} else {
			$terms = "";
			$message .= '&#8226; Please check the Terms of Service box ...<br>';
			$error_tally++;
		}
	}
	
	if ( isset($Change) ) {
		// let user change to a different service
		$error_tally++;
	}

}


// PROCESS FORM
// If user clicked "Continue" AND no errors on the form, process it
if ( (strtolower($action) == "submit") and !$error_tally ) {

	// Look for discounts, update database, offer a recap, then post to PayPal
	// These coupon values replace previously set variables of the same name, e.g. t3, extracted above
	$coupondesc = "";
	$coupondiscount = 0;
	if($coupon) {
		$coupon = strtolower(strip_tags($coupon));
		if ($coupon_key = VerifyCoupon ($coupon, $username)) {
			isset($coupon_codes["$coupon_key"]) ? $cc = $coupon_codes[$coupon_key] : $cc = array();
			extract($cc);
			$coupondesc && $coupondesc = "(We will apply $coupondesc)";
			// If the coupon has a discount factor, e.g. 25%, then apply the discount to the prices, $a1, $a2, $a3.
		}
	}
	
	// Is the rate = $0, which means unsubscribe?
	if ($myPackages[$packageID]['rate'] == 0) {
		$ppform['cmd'] = "_subscr-find";
		//$ppform['notify_url'] = urlencode("http://{$_SERVER['HTTP_HOST']}/pp/process.php");
		//$ppform['notify_url'] = "http://{$_SERVER['HTTP_HOST']}/pp/process.php";
		$ppform['notify_url'] = "http://{$_SERVER['HTTP_HOST']}".dirname($_SERVER['PHP_SELF'])."/process.php";

		$TESTING ? $ppform['alias'] = $myService['sandbox'] : $ppform['alias'] = $myService['business'];
		$target = "_blank";	// open a new screen, since we can't come back this way!
		$unsubnote = FetchSnippet ("unsubscribe");
		$status = "active";	// status tells us if this is an active order, or an old one
	} else {
		//$ppform['cmd'] = "_xclick-subscriptions";
		$ppform['cmd'] = "_ext-enter";	// allows prepopulation of name/address
		$ppform['redirect_cmd'] = '_xclick-subscriptions';
		
		$TESTING ? $ppform['business'] = $myService['sandbox'] : $ppform['business'] = $myService['business'];
	
		//$ppform['notify_url'] = urlencode("http://{$_SERVER['HTTP_HOST']}/pp/process.php");
		//$ppform['notify_url'] = urlencode("http://24.6.117.227/pp/process.php");
		//$ppform['notify_url'] = "http://{$_SERVER['HTTP_HOST']}/pp/process.php";
		$ppform['notify_url'] = "http://{$_SERVER['HTTP_HOST']}".dirname($_SERVER['PHP_SELF'])."/process.php";
		
		$ppform['item_name'] = "Subscription to $servicename for $username : $packagename $coupondesc";
		$ppform['item_number'] = $myService['ID'] . "-" . $packageID;
	
		$ppform['no_shipping'] = $PP_NO_SHIPPING;
		$ppform['return'] = $URL;
		$ppform['cancel_return'] = $URL;	// return to service calling page if cancelled
		$ppform['rm'] = "2";	//	Return Method. Set to '2' this makes all the Subscription vars available to your 'return' URL via POST method. Set to '1' and they'll be available via GET method (too visible). Set to '0' and the vars won't be available at all.
		
		$rate = (1 - ($coupondiscount/100)) * $myPackages[$packageID]['rate'];
		// regular rate
		$ppform['a3']  = "$rate";
		$ppform['p3']  = "{$myPackages[$packageID]['p']}";
		$ppform['t3']  = "{$myPackages[$packageID]['t']}";
		
		// ** We cannot use trials because Paypal doesn't handle cancellations and modifications properly **
		//$firstmonth = $rate +  $myPackages[$packageID]['setup'];
	
		// trial period rate, 
		//$ppform['a1']  = "$firstmonth";
		//$ppform['p1']  = "{$myPackages[$packageID]['p']}";
		//$ppform['t1']  = "{$myPackages[$packageID]['t']}";
	
		// unused: (2nd trial period)
		//$ppform['a2']  = "0";	//regular rate
		//$ppform['p2']  = "0";	//regular rate
		//$ppform['t2']  = "0";	//regular rate
	
		
		$ppform['src'] = $PP_SRC;	// recurring payments, 1=repeat until cancelled
		//$ppform['sra'] = $PP_SRA;	// reattempt on failure, 1=repeat 3 times then cancel. Omit for immediate failure
		$ppform['no_note'] = $PP_NO_NOTE;	// absolutely required for subscription processing... This field must be included, and the value must be set to 1
		$ppform['no_shipping'] = $PP_NO_SHIPPING;	// Shipping address. If set to "1," your customer will not be prompted for a shipping address. If omitted or set to "0," your customer will be prompted to include a shipping address
	
		// option field 1 name & value
		$ppform['on0']  = "Service";
		$ppform['os0']  = $myService['name'];
	
		// option field 2 name & value
		$ppform['on1']  = "Package";
		$ppform['os1']  = $myPackages[$packageID]['desc'];
		
		// currency code
		$ppform['currency_code'] = "USD";
		//$ppform['currency_code'] = "EUR";
		//$ppform['currency_code'] = "GBP";
		
		// Modify means you can't have trial periods!
		// So, if this is a new subscription, i.e. user is starting from
		// package=0, then turn off modify.
		$ppform['modify'] = $PP_MODIFY;	//	'0'= new subscription signup only, '1'= modify existing or signup for new subscriptions, '2'= modify existing subscription only
		
		//$ppform['page_style'] = "";
	
		// Prepopulate name/address
		$ppform['first_name'] = $first_name;
		$ppform['last_name'] = $last_name;
		$ppform['address1'] = $address1;
		$ppform['address2'] = $address2;
		$ppform['city'] = $city;
		$ppform['state'] = $state;
		$ppform['zip'] = $zip;
	
		$ppform['day_phone_a'] = $day_phone_a;
		$ppform['day_phone_b'] = $day_phone_b;
		$ppform['day_phone_c'] = $day_phone_c;

		$target = "";	// continue in this window
		$status = "active";	// status tells us if this is an active order, or an old one

		// "custom" gets the service ID (this won't change over the life of the subscription!)
		//$ppform['custom'] = $myService['ID'];
	}
	
	// Write the request to the orders database
	// We will check the Paypal response against this order, for security
	$pp_user_info = array (	
							"pp_comments"		=>	$comments,
							"pp_user_id"			=>	$userID,
							"pp_service_id"		=>	$serviceID,
							"pp_package_id"		=>	$packageID,
							"pp_status"			=>	$status
							);
	// Create new order if one doesn't exist
	$orderID = SaveOrder (array_merge ($ppform, $pp_user_info), $userID);
	WriteLog (date("D M j G:i:s T Y")." : Saved order ID# {$orderID} to database = " . MYSQLDB . ", table = " . PP_ORDERS_DB . "\n");
	$ppform['invoice'] = $orderID;

	//$confirm creates a 'post-to-PayPal' string 
	$form = FetchSnippet ("confirm");

	// build form variables
	$formvalues = "<!-- values for paypal -->\n";
	while (list($k,$v) = each ($ppform)) {
		$formvalues .= "<input name=\"$k\" value=\"$v\" type=\"hidden\" >\n";
	}

} else {
	// $form gathers subscription information from user. 
	// It may be pre-populated with user's previous answers.
	$form = FetchSnippet ("form");
}

// Prepare errors & msgs for output
$error && $error = Substitutions (FetchSnippet ("error_wrapper"), array ("ERROR"=>$error));
$message && $message = Substitutions (FetchSnippet ("message_wrapper"), array ("MESSAGE"=>$message));

$head = FetchSnippet ("head");
$foot = FetchSnippet ("foot");

// Display terms of service
if ($_REQUEST['tos']) {
	$form = FetchSnippet ("tos");
}

$page = $head.$form.$foot;

$varlist = GetVarList ($page);

foreach ($varlist as $v) { $$v && ($vars[$v] = ${$v});	}

$vars['oldPackageName'] = $myPackages[$oldPackageID]['name'] . "(#{$oldPackageID})";
$vars['CouponCode'] = CreateCoupon ($username, "ep");

//!OrderExists($userID) && print __LINE__.": No order for user #$userID<BR>";

$page = Substitutions ($page, $vars);
$page = ReplaceSysVars ($page);
$page = StripEmptyVars ($page);

print $page;

mysqli_close($LINK);
// We don't use mysqli in pp (yet!)
//$FP_MYSQL_LINK->close();



function KillSession () {
	// Unset all of the session variables.
	$_SESSION = array();
	print __FUNCTION__.": ".session_name()."<br>";
	// If it's desired to kill the session, also delete the session cookie.
	// Note: This will destroy the session, and not just the session data!
	if (isset($_COOKIE[session_name()])) {
		setcookie(session_name(), '', time()-3600, '/');
	}
	// Finally, destroy the session.
	session_destroy();
	error_reporting(E_ALL ^ E_NOTICE);
}


?>