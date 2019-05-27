<?php

// Mass Pay testing

$DEBUG = TRUE;

$testing = true;

include "_config/sysconfig.inc";
include "_config/fpconfig.inc";
include "_config/config.inc";
include "_config/paypal_masspay_config.inc"; 

include "includes/functions.inc";
include "includes/project_management.inc";
include "includes/image_management.inc";
include "includes/commerce.inc";
include "includes/paypal_masspay_cls.inc"; 


$DEBUG && print "<H1>MASSPAY.PHP</H1>";

$emailsubject = "Test of Mass Pay";

$masspay = new paypal_masspay ();

$masspay->params = array (	"USER"	=>	DEFAULT_USER_NAME,
	"PWD"				=>	DEFAULT_PASSWORD,
	"VERSION"			=>	"3.2",
	"SIGNATURE"			=>	DEFAULT_SIGNATURE,
	"METHOD"			=>	"MassPay",
	"CURRENCYCODE"		=>	"USD",
	"EMAILSUBJECT"		=>	$emailsubject,
	"RECEIVERTYPE"		=>	"EmailAddress"
);


// start item params with 0, e.g. L_AMT0
$my_txn_id = time();
$note1 = "This is note # 1";
$note2 = "This is note # 2";

// One payment to Mr. Tricky
$masspay->payments[] = array (	"L_AMT"	=> 1.11,
						"L_EMAIL"		=> "bear@mimetic.com",
						"L_UNIQUEID"	=> $my_txn_id . "-1",
						"L_NOTE"		=> $note1
					);

$masspay->payments[] = array (	"L_AMT"	=> 2.22,
						"L_EMAIL"		=> "bear@mimetic.com",
						"L_UNIQUEID"	=> $my_txn_id. "-2",
						"L_NOTE"		=> $note2
					);

	
	
// Built NVP string
$masspay->error_email = $error_email;
$masspay->currentrow = null;
$masspay->url = MASSPAY_PAYPAL_URL;
$response = $masspay->PPHttpPost(true);	// returned a parsed response from paypal, not raw response


// Interpret response

// Display the API response back to the browser.
// If the response from PayPal was a success, alert all receivers they were paid
// If the response was an error, display the errors received using APIError.php.
   
if ($masspay->response["response"]['ACK'] != "Failure") {
	$masspay->alert_payees ($em_headers);
	$DEBUG && print "SUCCESS:" . print_r ($masspay->response);
} else {
	$error = "";
	foreach ($masspay->response["response"] as $k => $v) {
		$error = "$k = $v\n";
	}
	$masspay->error_out($error, $em_headers);
	$DEBUG && print "ERROR:" . print_r ($masspay->response);
}



?>