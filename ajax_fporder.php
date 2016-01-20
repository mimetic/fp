<?php
/*
	JSON connection to php scripts for FP
	
*/


include "_config/sysconfig.inc";
include "_config/fpconfig.inc";
include "_config/config.inc";
include "includes/functions.inc";
include "includes/image_management.inc";
include "includes/commerce.inc";


// INCLUDE JCART BEFORE SESSION START
//include_once 'includes/fpcart.inc';

//include "includes/fpcart.inc";

$LINK = StartDatabase(MYSQLDB);
Setup ();

// START SESSION
session_name("fpcart");
session_start();

// INITIALIZE FPCART AFTER SESSION START
$cart =& $_SESSION["cart"]; 

$result = array ();
if(is_object($cart)) {
	// Get command
	if ($_POST) {
		$data = json_decode(stripslashes($_POST['data']), true);
	}
	$cmd = $data['cmd'];
	($data['data']) ? $params = $data['data'] : $params = array();
	
	is_array ($params) || $params = array ();
	
	$data['defaults'] ? $defaults = $data['defaults'] : $defaults = array();
	
	// params overwrite defaults
	$allparams = $params;
	if ($defaults)
		$allparams = array_merge($defaults, $params);
	
	// Get params as variables
	// $itemID, $itemQty, $itemPrice, $itemName
	//is_array($allparams) && extract ($allparams);
	isset($allparams['itemID']) ? $itemID = $allparams['itemID'] : $itemID = null;
	
	// Perform command
	
	$order = new FPOrder();
	
	switch ($cmd) {
		case 'checkout' :
			break;
		default:
			$result['output'] = $order->allItemsAreAvailable($allparams);
}

} else {
	$result['error'] = "No cart";
}
mysqli_close($LINK);
//$FP_MYSQL_LINK->close();

header("Content-type: text/plain");
$x = json_encode($result);
echo $x;
?>