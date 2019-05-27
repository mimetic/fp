<?PHP
include "_config/sysconfig.inc";
include "_config/fpconfig.inc";
include "_config/config.inc";
include "includes/functions.inc";
include "includes/project_management.inc";
include "includes/image_management.inc";

// Run automatic updating if it exists
if (file_exists("autoupdate.php"))
	require_once ("autoupdate.php");


// FP_INSTALLATION_TYPE DEFAULT IS SET IN fpconfig
// However it will be overridden by the Admin User settinging for UserLevel.
$LINK = StartDatabase(MYSQLDB);
$fp_user = CleanUserInput($_REQUEST['fp_user']);

Setup ();
list ($UserType) = mysqli_fetch_array (mysqli_query ($LINK, "select UserLevel from ".DB_ARTISTS." where ID=".FP_ADMINISTRATOR ), MYSQLI_NUM);
$UserType && $FP_GALLERY_TYPE == $UserType;
mysqli_close($LINK);
//$FP_MYSQL_LINK->close();

switch ($UserType) {
	// 1=single user
	case FP_SINGLE_GALLERY_MULTI_USER :
		$link = "projects.php";
		break;
	// 2=gallery
	case FP_SINGLE_GALLERY_SINGLE_USER :
		$link = "projects.php";
		break;
	// 3=gallery
	case FP_MULTI_GALLERY_SINGLE_USER :
		$link = "groups.php";
		break;
	// 4=gallery
	case FP_MULTI_GALLERY_MULTI_USER :
		$link = "groups.php";
		break;
	default :
		$link = "projects.php";
		break;
}

$FP_FORCE_SHOW ? $forcelink = '?forceshow=true' : $forcelink = '';

// FIX THIS: SHOULD PASS ALL PARAMS, NO?
if ($fp_user) {
	$fp_user = "fp_user=$fp_user";
	$forcelink ? $fp_user = '&'.$fp_user : $fp_user = '?'.$fp_user;
}

$link = "https://{$SYSTEMURL}{$link}{$forcelink}{$fp_user}";
$link && header("Location: $link");

?>