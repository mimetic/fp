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

// ========= clear the website page caches ==================
// Assume that any change in Admin could change a page. Don't risk it,
// Clear all the caches for this website. The cache is now in tmp, locally,
// so we're not trashing cache for all users on the server
// However, we still have a problem with a multi-user system:
// we're clearing all cache... very inefficient!!!!
require_once('Cache/Lite.php');

// Set a few options
$options = array(
	'cacheDir' => 'tmp/cache/'
);

$Cache_Lite = new Cache_Lite($options);
$Cache_Lite->clean();
unset ($Cache_Lite);



$LINK = StartDatabase(MYSQLDB);
Setup ();
/*
 *
 * UPDATER CODE HERE
 */

$updaterdescription = "Update 1/31/2010: Add PrintNumber to Sales";
//-----------------


mysqli_query ("ALTER TABLE `Sales` ADD COLUMN `PrintNumber` INT;");


 
//----------------- WRITE LOG -----------------
fp_error_log("AUTOUPDATER: $updaterdescription", 3, FP_MAINTENANCE_LOG);

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