<?php
/*
 * An autoupdater for the FP system.
 * This PHP script is run by the index.php
 */
/*
require_once "_config/sysconfig.inc";
require_once "_config/fpconfig.inc";
require_once "_config/config.inc";
require_once "includes/functions.inc";
require_once "includes/project_management.inc";
*/

//$LINK = StartDatabase(MYSQLDB);
//Setup ();

/*
 *
 * UPDATER CODE HERE
 */

$updaterdescription = "Add Domain and Account values to fpconfig.inc.";

//-----------------
$DEVELOPING = false;

$fn = "_config/fpconfig.inc";
$t = file_get_contents($fn);
!file_exists("_backups") && mkdir ("_backups");
rename ($fn, "_backups/fpconfig.inc.bak");

$account = str_replace("/home/","",$_ENV["DOCUMENT_ROOT"]);
$account = str_replace("/public_html","",$account);
$domain = str_replace("www.","",$_SERVER["HTTP_HOST"]);

$s = "// Name and contact info";
$r = "$s

// Server system account name
\$FP_ACCOUNT_NAME = \"$account\";
define (\"FP_ACCOUNT_NAME\", \$FP_ACCOUNT_NAME);

// Domain name
\$FP_DOMAIN_NAME = \"$domain\";
define (\"FP_DOMAIN_NAME\", \$FP_DOMAIN_NAME);

// Mail settings for receiving pictures and files by email
// Path to new mail that should be parsed: used to build /home/username/mail/mydomain.com/pix/new)
\$FP_EMAIL_MAILDIR = \"/home/\".FP_ACCOUNT_NAME.\"/mail/\".FP_DOMAIN_NAME.\"/\$FP_EMAIL_ACCOUNT/new\";
";
$t = str_replace ($s, $r, $t);

//print $t;

file_put_contents ($fn, $t);


//----------------- WRITE LOG -----------------
error_log("AUTOUPDATER: $updaterdescription", 3, "log/maintenance.log");


/*
 * END UPDATER CODE
 */

//mysqli_close($LINK);
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