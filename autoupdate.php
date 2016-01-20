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
 * UPDATER CODE HERE:
 *
 */

$version = "2013.07.04-07";
$updaterdescription = "Updated system to version $version";
$updaterdescription .= "\n Add htaccess files to _user, mailed photos, and photos directories.";

//-----------------

// Get current version
$fn = "$BASEDIR/_config/sysconfig.inc";
$f = file_get_contents ("_config/sysconfig.inc");
preg_match("/Auto-Update Version (.*)/i", $f, $vv);
$currentVersion = trim($vv[1]);

//print_r($vv);
//print ("Current version vs new version : $currentVersion vs $version<BR>");

if ($version > $currentVersion) {


		//----------------- WRITE LOG -----------------
		fp_error_log("AUTO-UPDATER: $updaterdescription", 3, FP_UPDATES_LOG);

		if ( file_exists("$BASEDIR/$ORIGINALS/.htaccess") && !unlink ("$BASEDIR/$ORIGINALS/.htaccess") ) 
			fp_error_log("AUTO-UPDATER: failed to delete $BASEDIR/$ORIGINALS/.htaccess", 3, FP_UPDATES_LOG);

		if ( file_exists("$BASEDIR/$ORIGINALS/.htaccess") ) {
			unlink ("$BASEDIR/$ORIGINALS/.htaccess") ;
		} else {
			fp_error_log("*** Cannot delete $BASEDIR/$ORIGINALS/.htaccess", 3, FP_UPDATES_LOG);
		}

		if ( file_exists("$BASEDIR/$FP_DIR_USER/.htaccess") && !unlink ("$BASEDIR/$FP_DIR_USER/.htaccess") ) 
			fp_error_log("AUTO-UPDATER: failed to delete $BASEDIR/$FP_DIR_USER/.htaccess", 3, FP_UPDATES_LOG);

		if ( file_exists("$BASEDIR/$MAILED_DIR/.htaccess") && !unlink ("$BASEDIR/$MAILED_DIR/.htaccess") ) 
			fp_error_log("AUTO-UPDATER: failed to delete $BASEDIR/$MAILED_DIR/.htaccess", 3, FP_UPDATES_LOG);
			
		ConfirmSetup();
	
		
		
		// --------------------------
		// Update the version in the sysconfig.inc file
		$fn = "$BASEDIR/_config/sysconfig.inc";
		$f = file_get_contents ("_config/sysconfig.inc");
		$s = "/Auto-Update Version .*/i";
		$r = "Auto-Update Version $version";
		$f = preg_replace($s,$r,$f);
		rename ($fn, $fn."-".date("Y-m-d,H-m-s").".bak");
		file_put_contents($fn, $f);
		
	
} else {
	//----------------- WRITE LOG -----------------
	fp_error_log("AUTO-UPDATER: Attempt to update the system, but it is already version $version", 3, FP_UPDATES_LOG);

} // end of the version check

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