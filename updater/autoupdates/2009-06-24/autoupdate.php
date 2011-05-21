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

// Change every theme_id parameter in all groups. Space->underline
$result = FetchAllGroupsArrays();
foreach ($result as $group) {
	$theme = preg_replace("/\s+/", "_", $group['Theme']);
	$groups[$group['ID']] = $group;
	$result = EditTable ("update", DB_GROUPS, $group['ID'], array ("Theme"=>$theme));
}


// Change every theme_id parameter in all projects. Space->underline
$projects = FetchAllProjectArrays();
foreach ($projects as $project) {
	$p = FetchParams (DB_PROJECTS, $project['ID']);
	$themeID = GetParam ($p, FP_PARAM_GALLERY_THEME);
	$themeID = preg_replace("/\s+/", "_", $themeID);
	$p = SetParam ($p, FP_PARAM_GALLERY_THEME, $themeID);
	SaveParams (DB_PROJECTS, $project['ID'], $p);
}

fp_error_log("AUTOUPDATER: Fix gallery ID names: Change all space to underline in all theme IDs, in groups and projects.", 3, FP_MAINTENANCE_LOG);


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