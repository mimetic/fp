<?php
/*
Catalog page
Uses SlideShowPro Flash catalog
*/

// =========== BEGIN CACHING CODE ================
// Include the package
require_once('Cache/Lite.php');

// Get the id for this cache
preg_match("|^/\w+/(.+?)/|",__FILE__, $m);
$username = $m[1];
$cachegroup = "project{$username}";
$cacheid = preg_replace ("/clearcache=\w+/i", "", $_SERVER['REQUEST_URI']);
$cacheid = $cachegroup . preg_replace ("/\W/","",$cacheid);

// Force a clear of cache, e.g .after admin change?
isset($_REQUEST['clearcache'])
? $clearcache = $_REQUEST['clearcache']
: $clearcache = null;

// Set a few options
$options = array(
	'cacheDir' => 'tmp/cache/',
	'automaticCleaningFactor' => 1,
	'lifeTime' => 86400	// one day
);

// Create a Cache_Lite object
$Cache_Lite = new Cache_Lite($options);
//$Cache_Lite->setToDebug();

$clearcache && $Cache_Lite->remove($cacheid, $cachegroup);

// Test if there is a valid cache for this id
if (!($output = $Cache_Lite->get($cacheid, $cachegroup))) {
	// =========== NO CACHE, BUILD THE PAGE ================
	
	include "_config/sysconfig.inc";
	include "_config/fpconfig.inc";
	include "_config/config.inc";
	include "includes/functions.inc";
	include "includes/project_management.inc";
	include "includes/image_management.inc";
	include "includes/commerce.inc";
	
	$error = "";
	$msg = "";
	
	session_name("fp_gallery_session");
	session_start();
	
	$LINK = StartDatabase(MYSQLDB);
	Setup ();
	
	$results = GetFormInput();
	$actions = $results['actions'];
	$action = $results['actions']['action'];
	$vars = $results['vars'];
	isset($vars['GroupID']) && $_SESSION['GroupID'] = $vars['GroupID'];
	isset($_SESSION['GroupID']) || $_SESSION['GroupID'] = PUBLIC_GROUP_ID;
	
	// isset($vars['theme']) && $_SESSION['theme'] = $vars['theme'];
	isset($_SESSION['theme']) || $_SESSION['theme'] = DEFAULT_THEME;
	
	$groupID = $_SESSION['GroupID'];
	$myGroup = new FPGroup ($groupID);
	
	$GroupBannerURL = $myGroup->LogoFilename();
	$GroupBanner = $myGroup->LogoHTML("style='border:1px solid black;margin-right:10px;'");
	$GroupIcon = $myGroup->IconHTML();
	
	$myTheme = CurrentThemeID ();
	
	// Get user info, if it exists. It would come because the admin passed a user ID variable
	$vars['fp_user'] && $_SESSION['fp_user'] = $vars['fp_user'];
	
	
	$projectID = $vars['ProjectID'];
	$_SESSION['projectid'] = $projectID;
	$project = FetchProject ($projectID);
	$title = $project['Title'];
	$zoomOutPercent = "10";
	
	$slideshow = FetchSnippet ("gallery_slideshow_pc");
	
	$page = FetchSnippet ('master_page_slideshow');
	$navbar = FetchSnippet ('navbar_slideshow');	// we use of limited navbar on this page
	$output = Substitutions ($page, 	array(
		'list'			=> $list,
		'META_INDEX'		=> FetchSnippet ('meta_robots_noindex'),
		'NAVBAR'			=> $navbar,
		'NAVBAR_2'		=> $navbar2,
		'SLIDESHOW_HEAD'		=> FetchSnippet('sv_head'),
		'title' 			=> $title,	// must come after navbar
		'grouptitle'		=> $myGroup->title,
		'GROUPICON'		=> $GroupIcon,
		'sectionclass'		=> "slideshow",		//note the add space
		'SLIDESHOW' 		=> $slideshow,
		'ProjectID' 		=> $projectID,		// must be AFTER prev. item
		'ZOOMOUTPERCENT'		=> $zoomOutPercent,
		'message'		=> $msg,
		'error' 			=> $error,
		'master_page_popups'		=> FetchSnippet("client_access_dialog"),
		'pps' 			=> $project['pps'],
		'picturestyle' 		=> $picturestyle,
		'pagetitle'		=> $SYSTEMNAME,
		'slideshowpopup'		=> $sspop,
		'GROUPID'		=> $groupID
		));
	$output = ReplaceAllSnippets ($output);
	$output = ReplaceSysVars ($output);
	$output = DeleteUnusedSnippets ($output);

	mysqli_close($LINK);
	//$FP_MYSQL_LINK->close();

	$output = compress_html($output);
	$DEVELOPING || $Cache_Lite->save($output, $cacheid, $cachegroup);
}	
print $output;

?>