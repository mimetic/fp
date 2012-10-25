<?php
/*
Shows Page
Uses SlideShowPro Flash catalog to show all slideshows on a slideshow page.
*/

// =========== BEGIN CACHING CODE ================
// Include the package
require_once('Cache/Lite.php');

// Get the id for this cache
preg_match("|^/\w+/(.+?)/|",__FILE__, $m);
$username = $m[1];
$cachegroup = "{$username}";
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
	
	$DEBUG = 0;
	
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
	
	// Get user info, if it exists. It would come because the admin passed a user ID variable
	$vars['fp_user'] && $_SESSION['fp_user'] = $vars['fp_user'];
	
	$ProjectID = $vars['ProjectID'];
	
	$groupID = $_SESSION['GroupID'];
	$myGroup = new FPGroup ($groupID);
	
	$GroupBannerURL = $myGroup->LogoFilename();
	$GroupBanner = $myGroup->LogoHTML("style='border:1px solid black;margin-right:10px;'");
	$GroupIcon = $myGroup->IconHTML();
	
	$myTheme = CurrentThemeID ();
	
	$DEBUG && $msg .= __FILE__.__LINE__.": Time elapsed: ". round (microtime(true) - $starttime, 2) . " seconds<BR>";
	
	//$page = FetchSnippet ('master_page');
	$page = FetchSnippet ('master_page_slideshow');
	$header = FetchSnippet ("master_page_header");
	// If there's a snippet for the picture behind, then use it, else no format for random image
	$ri = FetchSnippet("picturebehind");
	$ri || $ri = "{Images_URL}";
	$randomimage = FetchRandomImage ("", $ri,null,null,$groupID);
	$randomImageList = FetchRandomImageList($groupID);

	$bkgd = "";
	
	$ssp = FetchSnippet ("gallery_slideshow");
	$ssp_params = "action=active&albumtnsize=gallery&GroupID={$groupID}{CLEARCACHE}";
	$ssp_params = EncodeParamsForFlash ($ssp_params);

	$navbar = FetchSnippet ('navbar_shows');	// we use of limited navbar on this page
	$output = Substitutions ($page, array(
		'list'				=> $ssp,
		'META_INDEX'			=> MetaIndexCode (DB_GROUPS, $groupID),
		'NAVBAR'				=> $navbar,
		'NAVBAR_2'			=> $navbar2,
		'GALLERY_STYLESHEET'	=>		"",
		'header'				=> $header,
		'BACKGROUND_IMG_STYLE'	=> $bkgd,
		'title'				=> $myGroup->title,
		'subtitle'			=> FP_SYSTEM_DISPLAY_NAME,
		'GROUPICON'			=> $GroupIcon,
		'GROUPBANNER'		=> $GroupBanner,
		'GROUPBANNERURL'		=> $GroupBannerURL,
		'pagetitle'			=> "{fp:Shows}",
		'sampleimage'		=> '',
		'SLIDESHOW_HEAD'		=> FetchSnippet('ssp_shows_head'),
		'sectionclass'		=> "shows",
		'SLIDESHOW'			=> $ssp,
		'SSP_PARAMS'			=> $ssp_params,
		'PROJECTID' 			=> $ProjectID,		// must be AFTER prev. item
		'GROUPID' 			=> $groupID,		// must be AFTER prev. item
		'message'			=> $msg,
		'error'				=> $error,
		'CLEARCACHE'			=> $clearcache ? ",clearcache=1" : "",
		'master_page_popups'		=> FetchSnippet("client_access_dialog")
		));

	$output = ReplaceAllSnippets ($output);
	$output = ReplaceSysVars ($output);
	//$output = DeleteUnusedSnippets ($output);
	
	mysql_close($LINK);
	$FP_MYSQL_LINK->close();

	$output = compress_html($output);
	$DEVELOPING || $Cache_Lite->save($output, $cacheid, $cachegroup);
}	
print $output;

?>