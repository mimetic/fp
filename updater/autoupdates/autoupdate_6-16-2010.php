<?php
/*
 * An autoupdater for the FP system.
 * This PHP script is run by the index.php
 */
require_once "_config/sysconfig.inc";
require_once "_config/fpconfig.inc";
require_once "_config/config.inc";
require_once "includes/functions.inc";
require_once "includes/project_management.inc";

$LINK = StartDatabase(MYSQLDB);
Setup ();
/*
 *
 * UPDATER CODE HERE
 */

$updaterdescription = "Add ";

//-----------------
$fn = "$BASEDIR/_config/fpconfig.inc";
$f = file_get_contents ("_config/fpconfig.inc");

if (!strpos ($f, "FP_ADMIN_CHANGE_COMMISSION")) {

	$s = '// This determines whether we use the PayPal sandbox for testing';
	$r = "// Can the admin user change the website commission for each artist? Lock this if you want to take
	// a commission from all users, so they can't change it.
	define (\"FP_ADMIN_CHANGE_COMMISSION\", false);
	
	";
	$r = $r.$s;
	$f = str_replace($s,$r,$f);
	rename ($fn, $fn.".bak");
	file_put_contents($fn, $f);
	
	
	//----------------- WRITE LOG -----------------
	fp_error_log("AUTOUPDATER: $updaterdescription", 3, FP_MAINTENANCE_LOG);
}

/*
 * END UPDATER CODE
 */
mysqli_close($LINK);
//$FP_MYSQL_LINK->close();

/*
 * RENAMER:
 * This part of the script renames the file after running, so it won't be run again.
 */
$BASEDIR = dirname(__FILE__);

if (!$DEVELOPING) {

	// legacy fix
	$x = "$BASEDIR/_user/_audio/slideshow/.htaccess";
	if (file_exists($x)) {
		unlink ($x);
		fp_error_log("Fix access to slide show audio files.", 3, FP_MAINTENANCE_LOG);
	}
	$f = __FILE__;
	$k = 1;
	$ff = $f;
	while (file_exists($ff)) {
		$ff = $f.".completed.".$k;
		$k++;
	}
	rename(__FILE__, $ff);
}


?>