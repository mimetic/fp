<?php

//error_reporting  (E_ERROR | E_WARNING | E_PARSE | E_NOTICE); 
error_reporting(E_ERROR | E_WARNING | E_PARSE );

include "_config/sysconfig.inc";
include "_config/fpconfig.inc";
include "_config/config.inc";
include "includes/functions.inc";
include "includes/project_management.inc";
include "includes/image_management.inc";
include "includes/commerce.inc";

include("includes/paypal_ipn.inc");
include("includes/paypal_order.inc");

$LINK = StartDatabase(MYSQLDB);
Setup ();

$DEBUG = 0;
$write_log = true;

$vars = CleanRequest();

// Check there really are prints for sale. Back buttons can mean user can get to a sales
// page without the system checking for available prints.
if (!imageIsForSale ($vars['item_number'])) 
	{
	$url = $vars['return'];
	header("Cache-Control: no-cache, must-revalidate");
	header("Expires: Mon, 26 Jul 1997 05:00:00 GMT");	 // Date in the past
	header('Location: '.$url . "?". http_build_query($vars) );
	}
else
	{
	
	$result = SaveCart ($vars);
	
	if ($result) 
		{	
		list ($orderID, $order_time, $secret) = $result;
		$vars['item_number'] = $orderID;
		$vars['order_time'] = $order_time;
		$vars['notify_url'].= "?secret={$secret}";
		$imageID = $vars['ImageID'];
		
		// Make a unique priceset and record the new sale in the amounts sold
		// If the sale is not confirmed (in X minutes) we'll have to delete the amount sold later.
		RecordImageSaleBySale ($orderID);
		
		//array_merge ($vars, $result);	// result overwrites vars
		$url = "https://".PAYPAL_POST_TO_URL."/cgi-bin/webscr";
		$logme = $url;
		
		
		$logme .= http_build_query($vars);
		
		if ($DEBUG) 
			{
			print http_build_query($vars)."<HR>secret={$secret} <hr>";
			print http_build_query($result);
			var_dump($vars);
			}
		else 
			{
			header("Cache-Control: no-cache, must-revalidate");
			header("Expires: Mon, 26 Jul 1997 05:00:00 GMT");	 // Date in the past
			header('Location: '.$url . "?". http_build_query($vars) );
			}
		
		}
	else 
		{
		print "Error with order. Try again.<BR>";
		}
	}
mysqli_close($LINK);
//$FP_MYSQL_LINK->close();

// Save output form. Could also be added to FP_PAYMENTS_LOG?
// Don't keep around too long...security issues, I think.
//$datetime = date ("Y-m-d, h-m-s", $datetime);
//$f = "last_paypal_sale_form__{$datetime}.txt";
$f = "last_paypal_sale_form.txt";
$write_log && file_put_contents ("$LOGS/$f", $logme);

?>