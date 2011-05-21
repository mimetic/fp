<?php

// made by robin kohli (robin@19.5degs.com) for 19.5 Degrees (http://www.19.5degs.com)
// upated by David Gross for himself

//error_reporting  (E_ERROR | E_WARNING | E_PARSE | E_NOTICE); 
//error_reporting(E_ERROR | E_WARNING | E_PARSE );

// --- DEBUGGING ---
// note: FP_USE_PAYPAL_SANDBOX controls whether we're using the Paypal 
// sandbox development site or the real Paypal site.
// ------------------

include "_config/sysconfig.inc";
include "_config/fpconfig.inc";
include "_config/config.inc";
include "_config/paypal_masspay_config.inc"; 

include "includes/functions.inc";
include "includes/project_management.inc";
include "includes/image_management.inc";
include "includes/commerce.inc";

include("includes/paypal_ipn.inc");
include("includes/paypal_order.inc");
include "includes/paypal_masspay.inc"; 

include "includes/functions_autosender.inc"; 


$LINK = StartDatabase(MYSQLDB);
Setup ();

session_name("fp_gallery_session");
session_start();


$DEBUG = 0;

define ("DO_NOT_VERIFY", false);


// -----------------
if ($DEBUG) {
	print "<h2>DEBUG is on</h2>";

	// delete testing entry
	print "<h3>Deleting test entry in Paypal and Sales, txn_id = 20S24260NX0893647</h3>";
	print "<h3>Resetting DB_SALES entries</h3>";
	UpdateRecord( DB_SALES, "368", array ("txn_id"=>"", "order_time"=>"2008-05-27 13:12:11.0", "invoice"=>"4cef0589f0232", "item_number"=>"13991" ));
	UpdateRecord( DB_SALES, "369", array ("txn_id"=>"", "order_time"=>"2008-05-27 13:12:11.0", "invoice"=>"4cef0589f0232", "item_number"=>"13992"));
	//UpdateRecord( DB_SALES, "342", array ("txn_id"=>"", "order_time"=>"2008-05-27 13:12:11.0", "invoice"=>"4cef0589f0232", "item_number"=>"1415" ));
	DeleteRow ( DB_PAYPAL, "txn_id = '07973598XS698962T'" );
	
}

// Clean up old orders..."Maintenance" does this, also, but it can't hurt
// to do it here, too.
DeleteUnusedOrders();

// -----------------

$paypal_info = $_POST;
$DEBUG && $paypal_info = $_REQUEST;

// dump of $_POST
//fp_error_log("-----\n".__FILE__.": line ".__LINE__.": RAW PAYPAL POST: ".print_r($paypal_info,true)."-----\n", 3, FP_ORDER_LOG );



// The 'secret' is sent through $_GET
// Weird, huh? We could check $_REQUEST, but I was warned not to because
// a possible future "clash of NVPs", whatever that means.
isset($_GET['secret']) && $paypal_info['secret'] = $_GET['secret'];

$paypal_ipn = new paypal_ipn($paypal_info);
$paypal_order = new paypal_order ($paypal_ipn->paypal_post_vars);

if (0) {
	ksort ($paypal_ipn->paypal_post_vars);
	foreach ($paypal_ipn->paypal_post_vars as $key=>$value) {
		if (getType($key)=="string") {
			$$key = $value;
		}
	}
}

if (!$DEBUG) {
	$paypal_ipn->send_response();
	$paypal_ipn->error_email = PAYPAL_ERROR_EMAIL;
	$paypal_order->error_email = PAYPAL_ERROR_EMAIL;
}


if (!FP_USE_PAYPAL_SANDBOX) {
	if (!$paypal_ipn->is_verified()) {
		$paypal_ipn->error_out("Bad order (PayPal says it's invalid)" . $paypal_ipn->paypal_response , FP_EMAIL_HEADERS);
		die();
	}
}

//fp_error_log("-----\n".__FILE__.": line ".__LINE__.": Received an order: ".implode (", ", $paypal_ipn->paypal_post_vars), 3, FP_ORDER_LOG );
fp_error_log("-----\n".__FILE__.": line ".__LINE__.": Received an order: ".print_r($paypal_ipn->paypal_post_vars,true), 3, FP_ORDER_LOG );
//	fp_error_log("-----\nOrdering Cookie (".FP_ORDER_COOKIE."): ".$_COOKIE[FP_ORDER_COOKIE], 3, FP_ORDER_LOG );


//$DEBUG && print __FILE__.":".__LINE__.": Status=".$paypal_ipn->get_payment_status()."<BR>";

switch( $paypal_ipn->get_payment_status() )
{
	case 'Pending':
		
		$pending_reason=$paypal_ipn->paypal_post_vars['pending_reason'];
					
		if ($pending_reason!="intl") {
			$paypal_ipn->error_out("Pending Payment - $pending_reason", FP_EMAIL_HEADERS);
			break;
		}


	case 'Completed':
		
		switch ($paypal_ipn->paypal_post_vars['txn_type']) {
		
			case "reversal" :
				$reason_code=$paypal_ipn->paypal_post_vars['reason_code'];
				$paypal_ipn->error_out("PayPal reversed an earlier transaction.", FP_EMAIL_HEADERS);
				break;
				
			case "masspay" :
				fp_error_log("-----\nReceived a 'masspay' txn: ".implode (", ", ($this->paypal_post_vars)), 3, FP_PAYMENTS_LOG );
				break;
				
			default :
				if ( !$paypal_order->soldout() ) {
					if ( DO_NOT_VERIFY or ( 
						(strtolower(trim($paypal_ipn->paypal_post_vars['business'])) == PAYPAL_BUSINESS_EMAIL) and $paypal_order->OrderExists() and $paypal_order->OrderIsAuthentic() )) {
						$result = $paypal_order->RecordNewPaypalVars();

						if ($result and $paypal_order->RecordCartItems() ) {
							$DEBUG && $paypal_ipn->error_out("This was a successful transaction", FP_EMAIL_HEADERS);
							// CreateImagePriceSets is done upon ordering:
							//$paypal_order->CreateImagePriceSets();
							$paypal_order->SendOrderToSupplier ();
							$paypal_order->DistributeClientPayment ();
							// UpdateSalesCount done upon ordering, to avoid selling more than edition size.
							//$paypal_order->UpdateSalesCount();	// must be last, prev funcs check sales figures, e.g. for matchprint
							fp_error_log("SUCCESSFUL SALE: ".$paypal_order->OrderSummary(), 3, FP_ORDER_LOG );
						} else {
							$paypal_ipn->error_out("This was a duplicate transaction (" . $paypal_ipn->paypal_post_vars['txn_id'] . ")", FP_EMAIL_HEADERS);
						} 
					} else {
						$paypal_ipn->error_out("Someone attempted a sale using a manipulated URL", FP_EMAIL_HEADERS);
					}
				} else {
					$paypal_ipn->error_out("One of these pictures is sold out and should not have been offered", FP_EMAIL_HEADERS);
				}				
		}
		break;
		
	case 'Failed':
		// this will only happen in case of echeck.
		$paypal_order->update_all_paypal_vars ();
		$paypal_ipn->error_out("Failed Payment", FP_EMAIL_HEADERS);
	break;

	case 'Denied':
		// denied payment by us
		$paypal_order->update_all_paypal_vars ();
		$paypal_ipn->error_out("Denied Payment", FP_EMAIL_HEADERS);
	break;

	case 'Refunded':
		// payment refunded by us
		$paypal_order->update_all_paypal_vars ();
		$paypal_ipn->error_out("Refunded Payment", FP_EMAIL_HEADERS);
	break;

	case 'Canceled':
		// reversal cancelled
		// mark the payment as dispute cancelled		
		$paypal_order->update_all_paypal_vars ();
		$paypal_ipn->error_out("Cancelled reversal", FP_EMAIL_HEADERS);
	break;

	default:
		// order is not good
		$paypal_ipn->error_out("Unknown Payment Status - " . $paypal_ipn->get_payment_status(), FP_EMAIL_HEADERS);
	break;

} 

mysql_close($LINK);
$FP_MYSQL_LINK->close();
?>