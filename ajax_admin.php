<?php
/*
	JSON connection to php scripts for FP
	
	cmd=getsamplepricingonesizeforjs
		GetSamplePricingOneSizeForJS
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
} else {
	$data = array (
		"cmd"		=> $_REQUEST['cmd'],
		"order"		=> $_REQUEST['order'],
		"id"		=> $_REQUEST['id']
	);
}

$cmd = $data['cmd'];

// Perform command
switch ($cmd) {
	case "processmultiedit" :
		//$res = TestPME ();
		$res = ProcessMultiEdit ($data['table'], $data['rows'], $data['actions']);
		header("Content-type: text/plain");
		echo json_encode($res);
		break;
		
	case "getsamplepricingonesizeforjs" :
		GetSamplePricingOneSizeForJS ();
		break;
	case "update_project_image_order" :
		// Two ways to get the array out of the jQuery UI:
		// using sortable('toArray')
		// OR using sortable('serialize')
		// Must clean up the array...dumb, but I don't know how to get around it.
		if (is_array($data['order'])) {
			$order = join(",", $data['order']);
			$order = str_replace("pic_","",$order);
			$order = explode(",", $order);	// back to an array
		} else {
			$order = str_replace("&id[]=",",",$data['order']);
			$order = str_replace("id[]=","",$order);
			$order = explode(",", $order);	// back to an array
		}
		$vars['neworder'] = $order;
		$projectID = $data['id'];
		if ($projectID) {
			$project = new FPProject ($projectID);
			$project->SetProjectImages ($order);
			$res = true;
		} else {
			$res = false;
		}
		//fp_error_log("Update order of project #{$projectID} as ".join(", ", $order), 3, FP_ACTIVITY_LOG);
		
		$res
		? $res = "Saved"
		: $res = "Not Saved: Tell David immediately, there's a bug in the system!";
		
		ClearProjectCache ($projectID);
		
		header("Content-type: text/plain");
		echo json_encode($res);
		break;
	case "delete_image" :
		$imageID = $data['id'];
		$res = EditTable ("delete", DB_IMAGES, $imageID, array() );
		$res = __FILE__.":".__LINE__.":".$error;
		ClearAllCache ();

		header("Content-type: text/plain");
		echo json_encode($res);
		break;
	case "update_record" :
		$ID = $data['id'];
		$table = $data['table'];
		$values = $data['values'];
		$res = EditTable ("update", $table, $ID, $values );
		
		//fp_error_log(__FILE__.":".__FUNCTION__.": Update $table:$ID with ".preg_replace("/ +/"," ", preg_replace("/[\r\n\t]/","", print_r($values, true))), 3, FP_ACTIVITY_LOG);

		ClearAllCache ();

		header("Content-type: text/plain");
		echo json_encode($res);
		break;
		
	case "update_project_picture_settings" :
		$ID = $data['id'];
		$values = $data['values'];
		$project = new FPProject($ID);
		$res = $project->SaveProjectPictureSettings ($values);
		ClearAllCache ();

		header("Content-type: text/plain");
		echo json_encode($res);
		break;
		
		
}

mysql_close($LINK);
$FP_MYSQL_LINK->close();


?>