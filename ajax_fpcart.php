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

// INITIALIZE JCART AFTER SESSION START
$cart =& $_SESSION["cart"]; 
if(!is_object($cart)) 
	$cart = new FPCart();

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
$result = array ();
$cart->msg = "";
$cart->error = "";
$cart->command = "";

switch ($cmd) {
	case 'add' :
		$result['msg'] = $cart->add_item($allparams);
		$result['output'] = $cart->build_cart($allparams);
		$result['addButtonState'] = $cart->get_add_button_state($allparams['itemID']);
		break;
	case 'update' :
		$res = $cart->update_item($allparams);
		$result['msg'] = $res['msg'];
		$result['command'] = $res['command'];
		$result['output'] = $cart->build_cart($allparams);
		$result['addButtonState'] = $cart->get_add_button_state($allparams['itemID']);
		break;
	case 'find' :
		$result['item'] = $cart->find_item_by_catalogid($allparams['itemCatID']);
		break;
	// Is the item already in the cart, and it is an original?
	case 'available' :
		$result['item'] = $cart->find_item_by_id($allparams['itemID']);
		$result['available'] = $cart->item_available($allparams['itemID']);
		break;
	case 'remove' :
		$result['msg'] = $cart->del_item($allparams['itemID']);
		$result['output'] = $cart->build_cart($allparams);
		$result['addButtonState'] = $cart->get_add_button_state($allparams['itemID']);
		break;
	case 'clear' :
		$result['msg'] = $cart->empty_cart();
		$result['output'] = $cart->build_cart($allparams);
		$result['addButtonState'] = true;
		break;
	case 'build' :
		$result['output'] = $cart->build_cart($allparams);
		break;
	case 'discount' :
		//$cart->set_discount(floatval($allparams['discount']), $allparams['discountDesc']);
		$cart->set_discount_code($allparams['couponCode'], $allparams['itemCatID']);
		$result['output'] = $cart->build_cart($allparams);
		break;
	case 'update_cart_general_params' :
		$result['msg'] = $cart->set_cart_general_params($params);
		$result['output'] = $cart->build_cart($allparams);
		break;
	case 'update_cart_shipping_params' :
		$result['msg'] = $cart->set_cart_shipping_params($params);
		$result['output'] = $cart->build_cart($allparams);
		break;
	case 'checkout' :
		$result['msg'] = $cart->checkout($params);
		$result['ppvars'] = $cart->ppvars;
		$result['ppvars_encoded'] = $cart->ppvars_encoded;
		$result['buy_url'] = $cart->buy_url;
		if ( $cart->error)
			$result['output'] = $cart->build_cart($allparams);
		break;
	default:
		$result['output'] = $cart->build_cart($allparams);
}

if (isset($cart->item)) {
	$result['item'] = $cart->item;
}

if (isset($cart->shipping_params)) {
	$result['shipping_params'] = $cart->shipping_params;
}

$result['error'] = $cart->error;

mysqli_close($LINK);
//$FP_MYSQL_LINK->close();

header("Content-type: text/plain");
$x = json_encode($result);
echo $x;
?>