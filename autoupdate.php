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

$version = "2012.10.24-02";
$updaterdescription = "Updated system to version: $version";

//-----------------

// Get current version
$fn = "$BASEDIR/_config/sysconfig.inc";
$f = file_get_contents ("_config/sysconfig.inc");
preg_match("/Auto-Update Version (.*)/i", $f, $vv);
$currentVersion = trim($vv[1]);

//print_r($vv);
//print ("Current version vs new version : $currentVersion vs $version<BR>");

if ($version > $currentVersion) {
	// Update MYSQL tables
	
	$query = "ALTER TABLE  `Projects` ADD `OwnerAccessOnly` BOOLEAN DEFAULT 0 AFTER `GroupID`;";
	$result = mysql_query($query);
	
	mysql_query("ALTER TABLE `Projects` ADD `client_list` TEXT;");
	
	
	$query = "ALTER TABLE  `PriceSets` ADD `MaxFramedSize` INT DEFAULT 0 AFTER `GroupID`;";
	$result = mysql_query($query);
	
	//mysql_query("ALTER TABLE `Sales` ADD `Cart` TEXT;");
	mysql_query("ALTER TABLE `Sales` ADD `discount_amount` float;");
	mysql_query("ALTER TABLE `Sales` ADD `discount_rate` float;");
	mysql_query("ALTER TABLE `Sales` ADD `invoice` varchar(127);");
	mysql_query("ALTER TABLE `Sales` ADD `currency_code` varchar(3);");
	
	mysql_query("ALTER TABLE `Sales` ADD `address_name` varchar(128);");
	mysql_query("ALTER TABLE `Sales` ADD `address_state` varchar(2);");
	mysql_query("ALTER TABLE `Sales` ADD `address_street` varchar(200);");
	mysql_query("ALTER TABLE `Sales` ADD `address_zip` varchar(20);");
	mysql_query("ALTER TABLE `Sales` ADD `address_city` varchar(40);");
	mysql_query("ALTER TABLE `Sales` ADD `address_country` varchar(64);");
	mysql_query("ALTER TABLE `Sales` ADD `address_country_code` varchar(2);");
	
	mysql_query("ALTER TABLE `Sales` ADD `contact_phone` varchar(20);");
	mysql_query("ALTER TABLE `Sales` ADD `first_name` varchar(64);");
	mysql_query("ALTER TABLE `Sales` ADD `last_name` varchar(64);");
	mysql_query("ALTER TABLE `Sales` ADD `payer_business_name` varchar(127);");
	mysql_query("ALTER TABLE `Sales` ADD `payer_email` varchar(127);");
	mysql_query("ALTER TABLE `Sales` ADD `payer_id` varchar(13);");
	
	mysql_query("ALTER TABLE `Sales` ADD `item_id` bigint(20) AFTER item_number;");
	
	// Update MYSQL tables
	mysql_query("ALTER TABLE `PriceSets` ADD `Params` TEXT;");
	


	$fn = "$BASEDIR/_config/fpconfig.inc";
	$f = file_get_contents ("_config/fpconfig.inc");
	if (!strpos ($f, "FP_GOOGLE_CONVERSION_ID")) {
		
		
		// ad words to fpconfig
	
		$s = '// Can the admin user change the website commission';
		$r = '// Google Adwords Conversion codes
	// Signup page:
	define ("FP_GOOGLE_CONVERSION_ID", "");
	define ("FP_GOOGLE_CONVERSION_LABEL", "");
	// Checkout page:
	define ("FP_GOOGLE_CONVERSION_ID_CHECKOUT", "");
	define ("FP_GOOGLE_CONVERSION_LABEL_CHECKOUT", "");
		
	// Can the admin user change the website commission';
		$r = $r.$s;
		
		$f = str_replace($s,$r,$f);
		rename ($fn, $fn."-".date("Y-m-d,H-m-s").".bak");
		file_put_contents($fn, $f);

	
		// --------------------------
		// Update the version in the sysconfig.inc file
		$fn = "$BASEDIR/_config/sysconfig.inc";
		$f = file_get_contents ("_config/sysconfig.inc");
		$s = "/Auto-Update Version .*/i";
		$r = "Auto-Update Version $version";
		$f = preg_replace($s,$r,$f);
		rename ($fn, $fn."-".date("Y-m-d,H-m-s").".bak");
		file_put_contents($fn, $f);
	
	
		//----------------- WRITE LOG -----------------
		fp_error_log("AUTO-UPDATER: $updaterdescription", 3, FP_UPDATES_LOG);
		
	}
	
} else {
	//----------------- WRITE LOG -----------------
	fp_error_log("AUTO-UPDATER: Attempt to update the system, but it is already version $version", 3, FP_UPDATES_LOG);

} // end of the version check

/*
 * END UPDATER CODE
 */
 
 
mysql_close($LINK);
$FP_MYSQL_LINK->close();

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