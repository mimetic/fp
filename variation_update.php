<?php

/* AJAX processor for variation editor javascript.

This script will process AJAX input from the in-place, javascript, CSS variation editor for the FP gallery.

Method:
	Save: This theme will be a user version of the system variation of the same  id. 
		- Get variation id
		- If variation exists, add changes to the existing variation, overwriting identical entries
		- Else, create a new user variation with entries
		- Write variation to file
		- Return new variation id
	
	
	Save As: A new user variation will be created with a new name.
	




*/


$error = "";
$msg = "";

require_once "_config/sysconfig.inc";
require_once "_config/fpconfig.inc";
require_once "_config/config.inc";
require_once "includes/functions.inc";
require_once "includes/project_management.inc";

// Clear all caches
require_once('Cache/Lite.php');
$options = array(
	'cacheDir' => 'tmp/cache/'
);
$Cache_Lite = new Cache_Lite($options);
$Cache_Lite->clean();
unset ($Cache_Lite);

// report all errors except 'Notice', which is for undefined vars, etc. 
// All the isset clauses are to avoid 'notice' errors. 
error_reporting(E_ALL ^ E_NOTICE);

$LINK = StartDatabase(MYSQLDB);
Setup ();

$DEBUG = 0;

session_name("fp_gallery_session");
session_start();

$logFile = FP_DIR_LOG.'/cssedit.log';

// Use test data if no data sent to the script
if ($DEBUG) {
	if (!$_POST['data']) {
		$res = unserialize(file_get_contents('testdata.txt'));
		$res = json_decode(stripslashes($res), true);
	} else {
		$testdata = serialize($_POST['data']);
		file_put_contents('testdata.txt', $testdata);
	}
}
	
// If testing, no POST data
if ($_POST) {
	$res = json_decode(stripslashes($_POST['data']), true);
}

if ($DEBUG) {
	while (list($k,$v)=each($res)) {
		if (!is_array($v))
			fp_error_log("\n$k: $v", 3, $logFile);
	}
	fp_error_log("\nResult: ".json_encode($res), 3, $logFile);
	fp_error_log("\n-----------------------------\n", 3, $logFile);

}
// ------------------------------------------------------------

// $Themes is loaded with all themes by the sysconfig.inc
// There will be a themeid, since the editor only works on variations of themes
// (However, we may be creating a new variation)
$command = $res['command'];
isset($res['themeid']) ? $themeID = $res['themeid'] : $themeID = null;
isset($res['userid']) ? $userID = $res['userid'] : $userID = FP_ADMINISTRATOR;

// If we did a 'save as' to create a new variation, then
//- if there's a projectID, we're showing a project, so set the current project's theme to the new variation
//- otherwise, if there's a groupID, change the group
// Don't change both...there's a groupID sent when we're showing a project, and we don't want to change it

if ($userID != FP_ADMINISTRATOR && $Themes->userID != $userID) {
	$Themes->userID = $userID;
	$Themes->LoadAllThemes(true);
}

$Themes->userID = $userID;
// If we did a "delete", set the $_SESSION theme
switch ($command) {
		case "delete" :
			$Themes->Command($command, $res, $themeID);
			$_SESSION['theme'] = $Themes->themeID;
			break;
		case "save" :
			$Themes->Command($command, $res, $themeID);
			if ($res['newname']) {
				$projectID = $res['projectid'];
				if ($projectID) {
					SetProjectTheme ($projectID, $Themes->themeID);
					$_SESSION['theme'] = $Themes->themeID;
					$DEBUG && $msg .= "Set project $projectID to theme {$Themes->themeID}<br>";
				} else {
					$groupID = $res['groupid'];
					if ($groupID) {
						$myGroup = new FPGroup ($LINK, $groupID);
						$myGroup->SetTheme ($Themes->themeID);
						$_SESSION['theme'] = $Themes->themeID;
						$DEBUG && $msg .= "Set group $groupID to theme {$Themes->themeID}<br>";
					}
				}
			}
			break;
		case "change" :
			if (isset($res['themeid'])) {
				$Themes->themeID = $themeID;
				$res['projectid']? $projectID = $res['projectid'] : $projectID = null;
				if ($projectID) {
					SetProjectTheme ($projectID, $Themes->themeID);
					$_SESSION['theme'] = $Themes->themeID;
					$msg .= "Set project $projectID to theme {$Themes->themeID}<br>";
				} else {
					$res['groupid'] ? $groupID = $res['groupid'] : $groupID = null;
					if ($groupID) {
						$myGroup = new FPGroup ($LINK, $groupID);
						$myGroup->SetTheme ($Themes->themeID);
						$_SESSION['theme'] = $Themes->themeID;
						$msg .= "Set group $groupID to theme {$Themes->themeID}<br>";
					}
				}

			}
}


$result = "";
$msg && $result .= "$msg";
$error && $result .= "Error: $error";
$result .= $Themes->result;

if (!$DEBUG) {
	header("Content-type: text/plain");
	echo json_encode($result);
	$result = str_replace ("<BR>", "\n", $result);
	$result = str_replace ("<br>", "\n", $result);
	fp_error_log("\nResult: $result\n==========", 3, $logFile);
} else {
	header("Content-type: text/plain");
	echo json_encode($result);
	fp_error_log("\nResult: $result\n==========", 3, $logFile);
}

mysqli_close($LINK);
//$FP_MYSQL_LINK->close();

?>