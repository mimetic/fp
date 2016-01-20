<?php
/*
	JSON connection to php scripts for FP
	
	For AJAX calls from the gallery and viewer side of the website.
	
	cmd=getsamplepricingonesizeforjs
		GetSamplePricingOneSizeForJS
*/

include "_config/sysconfig.inc";
include "_config/fpconfig.inc";
include "_config/config.inc";
include "includes/functions.inc";

/*
		header("Content-type: text/plain");
		echo json_encode("success");
*/
$LINK = StartDatabase(MYSQLDB);
Setup ();

// Get command
$data = $_REQUEST['data'];
$cmd = $data['cmd'];

// write values to log for testing
//fp_error_log( __FILE__." : BEGIN\r".print_r($_POST['data'], true), 3, FP_ACTIVITY_LOG);
//fp_error_log( __FILE__." : Command = $cmd", 3, FP_ACTIVITY_LOG);

// Perform command
switch ($cmd) {
	case "clientaccess" :
		$res = clientLogin($data['clientid']);
		
		// If login successful, add username to the session
		if ($res) {
			session_name("fp_gallery_session");
			session_start();		
			$_SESSION['clientid'] = $data['clientid'];
		}

		$res ? $res = "success" : $res = "failure";
		header("Content-type: text/plain");
		echo json_encode($res);
		break;

}

mysqli_close($LINK);
//$FP_MYSQL_LINK->close();

?>