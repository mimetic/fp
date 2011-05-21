<?php
/*
	JSON connection to php scripts for FP
	
	cmd=getcartpricingforjs
		GetCartPricingForJS
*/


include "_config/sysconfig.inc";
include "_config/fpconfig.inc";
include "_config/config.inc";
include "includes/functions.inc";
include "includes/image_management.inc";
include "includes/commerce.inc";

$LINK = StartDatabase(MYSQLDB);
Setup ();

// Get command
if ($_POST) {
	$data = json_decode(stripslashes($_POST['data']), true);
}
$cmd = $data['cmd'];
// Perform command
switch ($cmd) {
	case "getcartpricingforjs" :
		GetCartPricingForJS ();
		break;
	case "validatecoupon" :
		GetCouponDiscountForJS ();
		break;
	default:
		GetCartPricingForJS ();
}

mysql_close($LINK);
$FP_MYSQL_LINK->close();


?>