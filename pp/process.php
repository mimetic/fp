<?PHP

// process.php for the PP paypal subscription system
// 
// This page receives IPN data from PayPal and processes it. 
// The subscription order is sent by order.php. Paypal then responds by talking with
// process.php, which confirms and records the order. It then updates the calling service
// with the package parameters and results, by recording the package bought and setting userlevel
// accesslevels, as spec'd in the package.

include "config.inc";
include "config_packages.inc";
include "includes/pp_functions.inc";

$error = "";
$DEBUG = 0;

WriteLog ("\n===== ".date("D M j G:i:s T Y")." : ".__FILE__." =====\n");

if (TRUE) {
	if ($_REQUEST) {
		foreach ($_REQUEST as $k => $v) {
			$out .= "$k = $v\n";
		}
	} else {
		$out = "NO DATA SENT";
	}
	file_put_contents ("process_POST_".date("D-M-j G_i_s").".txt", $d.$out);
}

if ($DEBUG) {
	$_POST = array (
		"amount3" => "35.00",
		"business" => "bear@mimetic.com",
		"charset" => "UTF-8",
		"custom" => "2",
		"first_name" => "David",
		"invoice" => "737",
		"item_name" => "Subscription to Frontline Galleries for dgross@mimetic.com : Solo Artist with Domain",
		"item_number" => "2-3",
		"last_name" => "Gross",
		"mc_amount3" => "35.00",
		"mc_currency" => "USD",
		"notify_version" => "2.5",
		"option_name1" => "Service",
		"option_name2" => "Package",
		"option_selection1" => "Frontline Galleries",
		"option_selection2" => "$35 per month, private server, 500MB storage, unlimited solo exhibitions, solo gallery",
		"payer_email" => "tricky@whitehouse.gov",
		"payer_id" => "78TB5NG66FLDQ",
		"payer_status" => "verified",
		"period3" => "1 M",
		"reattempt" => "0",
		"receiver_email" => "bear@mimetic.com",
		"recurring" => "1",
		"residence_country" => "US",
		"subscr_date" => "01:49:57 Oct 05, 2008 PDT",
		"subscr_id" => "S-9WU96484LC3189204",
		"test_ipn" => "1",
		"txn_type" => "subscr_signup",
		"verify_sign" => "Au4EpK12dqfk3LKne7ztxoc-SwJJAQAhy1ArODFKrGg-nmhtSG66mnBr"
	);
}

list ($SERVICEID, $packageID) = split ("-", trim($_POST['item_number']));
isset($PP_SERVICES[$SERVICEID]) ? $myService = $PP_SERVICES[$SERVICEID] : $myService = null;

if ($myService) {
	$servicename = $myService['name'];
	$servicedesc = $myService['desc'];
	
	define ("PP_ORDERS_DB", $myService['orders']);
	define ("PP_DB_USERS", $myService['users']);
	define ("PP_ORDERS_ID", $myService['orderIDField']);
	define ("PP_USERS_ID", $myService['usersIDField']);

	$mysqlhost = $myService['mysqlhost'];
	$mysqldb = $myService['mysqldb'];
	$mysqluser = $myService['mysqluser'];
	$mysqlpasswd = $myService['mysqlpasswd'];
	$LINK = StartDatabase( $mysqlhost, $mysqldb, $mysqluser, $mysqlpasswd);
	
	$orderID = $_POST['invoice'];
	$order = GetOrder($orderID);
	if ($order) {
		$packageID = $order['pp_package_id'];
		$userID = $order['pp_user_id'];
	} else {
		WriteLog ("No matching record in ".PP_ORDERS_DB." for order $orderID");
		exit;
	}

} else {
	WriteLog ("ERROR: Service wasn't found (value sent is {$PP_SERVICES[$SERVICEID]}, based on serviceID = {$SERVICEID}");
	exit;
}


// Get field list of our payments database
$db_fields = GetFieldList ( PP_ORDERS_DB );

// read the post from PayPal system and add 'cmd'
$IPNsend = 'cmd=_notify-validate';

$IPNvars = array ();
foreach ($_POST as $key => $value) {
	if (get_magic_quotes_gpc()) 
		$value = stripslashes ($value);
	if (!eregi("^[_0-9a-z-]{1,30}$",$key) || !strcasecmp ($key, 'cmd')) {
		unset ($key); 
		unset ($value); 
	}

	// built HTTP post to send back to PayPal
	$IPNsend .= "&$key=$value";
	
	// For each field, if it is a field we want to save, build the SQL 
	// data to write it. We save it if it's a field in our DB (i.e. in $db_fields).
	if (in_array($key, $db_fields))
		$IPNvars[trim($key)] = trim($value);
}

// These are optional and might not be set
isset($IPNvars['mc_amount1']) ? settype($IPNvars['mc_amount1'], "float") : $IPNvars['mc_amount1'] = 0;
isset($IPNvars['mc_amount2']) ? settype($IPNvars['mc_amount2'], "float") : $IPNvars['mc_amount2'] = 0;
settype($IPNvars['mc_amount3'], "float");

WriteLog (__LINE__ . " :: POST received at " . date("D M j G:i:s T Y"));

// ====================
// Verify the $_POST with Paypal

if (!$DEBUG) {
	WriteLog ("Sending transaction acceptance to Paypal");
	set_time_limit(60); 
	$socket = @fsockopen($post_to_URL,80,$errno,$errstr,30);
	$header = "POST /cgi-bin/webscr HTTP/1.0\r\n";
	$header.= "User-Agent: PHP/".phpversion()."\r\n";
	$header.= 'Referer: '.$_SERVER['HTTP_HOST'].$_SERVER['PHP_SELF'].'@'.$_SERVER['QUERY_STRING']."\r\n";
	$header.= 'Server: '.$_SERVER['SERVER_SOFTWARE']."\r\n";
	$header.= 'Host: '.$post_to_URL.":80\r\n";
	$header.= "Content-Type: application/x-www-form-urlencoded\r\n";
	$header.= 'Content-Length: '.strlen($IPNsend)."\r\n";
	$header.= "Accept: */*\r\n\r\n";
	if (!$socket)
		{
		$response = file_get_contents('http://'.$post_to_URL.':80/cgi-bin/webscr?'.$IPNsend); 
		}
	else
		{
		fputs ($socket,$header.$IPNsend."\r\n\r\n"); 
		while (!feof($socket))
			{
			$response = fgets ($socket,1024); 
			}
		}
	$response = trim ($response); 
	fclose ($socket); 
} else {
	$response = "VERIFIED";
}

// ====================
// Process the $_POST from Paypal



if ( $response == "VERIFIED" ) {
	// check the payment_status is Completed
	// check that receiver_email is your Primary PayPal email
	if ($IPNvars['receiver_email'] != RECEIVER_MAIL ) {
		$error .= "Verified response, but receiver mail is '{$IPNvars['receiver_email']}' but should be '" . RECEIVER_MAIL . "'\n :: IP= {$_SERVER['REMOTE_ADDR']}";
	}
	
	// Record payment and update user settings based on purchase
	if (!$error) {
		$success = ProcessResponse ($IPNvars, $orderID);
		$success & WriteLog ("Success.");
	} else {
		$error .= "Verified response, but failed to process response.";
	}
} else {
	$error .= "Verified response, but failed to verify the payment";
}

$error && WriteLog (__LINE__ . "\nResponse: $response\nIP= {$_SERVER['REMOTE_ADDR']}\nError: $error\nIPN variables = \"" . $IPNsend . "\"");

mysqli_close($LINK);
//$FP_MYSQL_LINK->close();

//----- END ----
// ====================


// ====================
// FUNCTIONS

function ProcessResponse ($IPNvars, $orderID = '' ) {
	global $msg, $error;

	$DEBUG = 0;
	
	$DEBUG && print __FUNCTION__."<BR>";
	WriteLog ("\n------ ProcessResponse ------");

	// assign posted variables to local variables
	extract ($IPNvars);
	
	//$DEBUG && print ArrayToTable ($IPNvars);
	
	// $item_number holds the record ID in the IPN database
	// The user's ID in the service database is $item_number
	// Retrieve the order from our DB. This is safer than getting it from PayPal, no?
	$orderID || $orderID = $_POST['invoice'];
	$order = GetOrder($orderID);
	$order || WriteLog ("No matching record in ".PP_ORDERS_DB." for order $orderID");

	$SERVICEID = $order['pp_service_id'];
	$packageID = $order['pp_package_id'];
	$userID = $order['pp_user_id'];
	
	WriteLog (__LINE__.": Service=$SERVICEID, package=$packageID, userID=$userID");

	// DEBUGGIN
	//WriteLog (__LINE__.": ORDER:\n".join ("\n", $order));
	//WriteLog (__LINE__.": IPN VARS:\n".join ("\n", $IPNvars));

	// Be sure we have an order record (for security!)
	// and we don't already have this transaction ID, 
	// and the $orderID is set
	if ($order && !TransactionExists ($txn_id) && $orderID ) {
		WriteLog ("Order exists; id = $orderID, txn_type=$txn_type");
		
		$DEBUG && print __FUNCTION__ . " : Completed ";
		$userID = $order["pp_user_id"];
		
		switch ($txn_type) {
			case 'subscr_payment' :
				if (!amounts_match_original_order ($order, $IPNvars, $txn_type))
					break;
				// update DB record
				$IPNvars['comment'] = "IPN Payment";
				$IPNvars['pp_status'] = "inactive";
				if (trim($IPNvars['payment_status']) == "Completed") {
					UpdateOrder ( $IPNvars, $orderID );
					//WriteLog ("subscr_payment: Updated database, ID=$orderID, with ". join (", ", $IPNvars));
					$DEBUG && print ": updated $orderID<BR>";
					// UPDATE INFO IN SERVICES
					UpdateServiceUserInfo ($SERVICEID, $packageID, $userID);
				} else {
					// Payment Status indicates not completed, cancelled, etc.
					WriteLog (__LINE__ . " :: The process not completed\n :: payment_status = ".$IPNvars['payment_status']."\n :: $IPNsend");
				}
				break;
			case 'subscr_signup' :
				if (!amounts_match_original_order ($order, $IPNvars, $txn_type))
					break;
				if ($order['pp_status'] != 'active')
					$error .= "Record $orderID in ".PP_ORDERS_DB." is not an active order.";
				// update DB record
				$IPNvars['comment'] = "Subscription Accepted";
				$IPNvars['init_pass'] = $init_pass;
				$IPNvars['pp_status'] = "inactive";
				UpdateOrder ( $IPNvars, $orderID );
				//WriteLog ("subscr_signup: Updated database, ID=$orderID, with ". join (", ", $IPNvars));
				// UPDATE INFO IN SERVICES 	
				UpdateServiceUserInfo ($SERVICEID, $packageID, $userID);
				break;
				
			case 'subscr_cancel' :
				$IPNvars['comment'] = "Subscription Cancelled";
				$IPNvars['pp_status'] = "inactive";
				UpdateOrder ( $IPNvars, $orderID );
				//WriteLog ("subscr_cancel: Updated database, ID=$orderID, with ". join (", ", $IPNvars));
				//	suspend for now, terminate later
				// UPDATE INFO IN SERVICES 
				$packageID = "0";
				UpdateServiceUserInfo ($SERVICEID, $packageID, $userID);
				// update subscriber's Contact Email Address in their WHM Account

				break;
			case 'subscr_failed' :
				$IPNvars['comment'] = "Failed IPN Payment";
				$IPNvars['pp_status'] = "inactive";
				UpdateOrder ( $IPNvars, $orderID );
				// update DB record
				break;
				
			case 'subscr_eot' :
				// update DB record
				$IPNvars['comment'] = "Subscription term has expired";
				$IPNvars['pp_status'] = "inactive";
				UpdateOrder ( $IPNvars, $orderID );
				// suspend for now, terminate later
				// UPDATE INFO IN SERVICES 
				$packageID = "0";
				UpdateServiceUserInfo ($SERVICEID, $packageID, $userID);
				break;
				
			case 'subscr_modify' :
				if (!amounts_match_original_order ($order, $IPNvars, $txn_type))
					break;
				if ($order['pp_status'] != 'active')
					$error .= "Record $orderID in ".PP_ORDERS_DB." is not an active order.";
				$IPNvars['comment'] = "Subscription has been modified";
				$IPNvars['pp_status'] = "inactive";
				UpdateOrder ( $IPNvars, $orderID);
				UpdateServiceUserInfo ($SERVICEID, $packageID, $userID);
				break;
			default:	// something to do if 'txn_type' doesn't fit any case above
				$error .= "The transaction PayPal sent is an unknown type: $txn_type";
				break;
		} // switch
	} // if txn_id and ID exist
	else 
	{
		$error .= "Failed: ";
		$order || $error .= " No subscriber";
		TransactionExists ($txn_id) && $error .= " Transaction exists";
		$orderID || $error .= " No ID";
	}
	$result = empty($error);
	return $result;
}

/* ============================================= */

// Update the 'user' table spec'd in the config file,
// to reflect the subscription/sale.
function UpdateServiceUserInfo ($SERVICEID, $packageID, $userID) {
	global $msg, $error;
	global $PP_SERVICES, $PP_PACKAGES;
	
	$DEBUG = 0;
	
	$DEBUG && print __FUNCTION__ ."<br>";
	
	if (!isset($PP_SERVICES[$SERVICEID])) {
		$error .= "Requested service doesn't exist in the config file!<BR>";
		$DEBUG && print __FUNCTION__ .":". __LINE__ .": I quit : Service = $SERVICEID<BR>";
		WriteLog ("Requested service doesn't exist in the config file!");
		return false;
	}
	
	$DEBUG && print __FUNCTION__ . " :: userID=$userID :: service=$SERVICEID, package=$packageID <br>";
	$pkg = $PP_PACKAGES["$SERVICEID"]["$packageID"]['data'];
	$result = AddPackageToUser ($pkg, $userID );
	
	$DEBUG && print __FUNCTION__ .": line ". __LINE__ .": $userID, $packageID ".ArrayToTable ($pkg) . "<BR>";
	WriteLog (__LINE__ . " :: Updated ".PP_DB_USERS." :: userID=$userID :: service=$SERVICEID, package=$packageID");
	
	return $result;
}

// check if there is a matching transaction in the database
// return false if no match.
function TransactionExists ($txn_id=0) {
	$DEBUG = TRUE;
	$exists = false;
	if ($txn_id) {
		$order = FetchRowsByValue ( PP_ORDERS_DB, "*", "txn_id", $txn_id, "", true); // get one row as array
		if ($order)
			$exists = ($order['pp_status'] == "active");

	}
	return $exists;
}

function amounts_match_original_order ($order, $IPNvars, $txn_type = "") {
	global $error;
	
	if ($txn_type == 'subscr_payment') {
		$amount = $IPNvars['mc_gross'];
	} else {
		$amount = $IPNvars['mc_amount3'];
	}
	
	if (( $order['mc_currency'] != $IPNvars['mc_currency']) or ( ($order['mc_amount3'] - $amount) > 0) ) {
		$error .= "Payment amounts or currency in the internal order does not match Paypal IPN variables:  ({$order['mc_currency']} != {$IPNvars['mc_currency']} OR {$order['mc_amount3']} != {$amount}";
		return false;
	} else {
		return true;
	}
}

?>

