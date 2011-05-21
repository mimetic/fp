<?php
/* 
 * Return an HTML popup list of themes
 * so user can choose one. Param is JSON encoded POST
 * with themeid=>themeid
 */


$error = "";
$msg = "";

require_once "_config/sysconfig.inc";
require_once "_config/fpconfig.inc";
require_once "_config/config.inc";
require_once "includes/functions.inc";


// If testing, no POST data
if ($_POST) {
	$res = json_decode(stripslashes($_POST['data']), true);
}

$systemdefault = array('value'		=> "0",
		'checked'		=> "",
		'label'			=> "System Default"
		);

$systemdefault = null;


$Themes->userID = $res['userid'];
$Themes->LoadAllThemes(true);
// THEME_LIST_FIELD popup
$checked = array ($res['themeid']);
$ThemeList = OptionListFromArray (
	$Themes->FP_THEMES_LIST,
	"ChangeToThemeID",
	$checked,
	TRUE,
	OPTION_LIST_IS_POPUP,
	'id="ChangeToThemeID" onChange="ChangeTheme(\'ChangeToThemeID\')"',
	$systemdefault
		);

header("Content-type: text/plain");
echo $ThemeList;

?>
