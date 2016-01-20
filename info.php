<?php

/*

Page for showing information such as FAQ or About this Website.

*/
	
// =========== BEGIN CACHING CODE ================
// Include the package
require_once('Cache/Lite.php');

// Get the id for this cache
preg_match("|^/\w+/(.+?)/|",__FILE__, $m);
$username = $m[1];
$cachegroup = "{$username}_info";
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
	
	$DEBUG && $starttime = microtime(true);
	
	$error = "";
	$msg = "";
	
	$DEBUG && $timer .= __FILE__.__LINE__.": Time elapsed: ". round (microtime(true) - $starttime, 2) . " seconds<BR>";
	
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
	
	$groupID = $_SESSION['GroupID'];
	$myGroup = new FPGroup ($LINK, $groupID);
	
	$GroupBannerURL = $myGroup->LogoFilename();
	$GroupBanner = $myGroup->LogoHTML("style='border:1px solid black;margin-right:10px;'");
	$GroupIcon = $myGroup->IconHTML();
	
	$myTheme = CurrentThemeID ();
	
	$info = trim($vars["subject"]);
	$info || $info = "terms";
	// To choose which subject is active in the navbar
	
	$navbar = NavBar ($groupID, $info);	// 5 means navbar item 5 is selected
	$navbar2 = NavBar ($groupID, $info, 2);	// 5 means navbar item 5 is selected
	
	$list = FetchSnippet ("info_wrapper_open") . FetchSnippet ("info_".$info) . FetchSnippet ("info_wrapper_close");
	
	$output = FetchSnippet ('master_page');
	$header = FetchSnippet ("master_page_header");
	$title = FP_SYSTEM_DISPLAY_NAME . " {fp:InfoPageTitle}";
	
	// If there's a snippet for the picture behind, then use it, else no format for random image
	$ri = FetchSnippet("picturebehind");
	$ri || $ri = "{Images_URL}";
	$randomimage = FetchRandomImage ('', $ri);
	$randomImageList = FetchRandomImageList($groupID);
	
	// Don't show background image on info pages
	/*
	$showBkgdImage = FetchSnippet ("show_bkgd_image");
	$showBkgdImage 
		? $bkgd = FetchSnippet ("background_img_style")
		: $bkgd = "";
	*/
	$bkgd = "";
	
	//NOTE: We DON'T use background images on Info pages!
	$output = Substitutions ($output, array(
		'META_INDEX'			=> FetchSnippet ('meta_robots_noindex'),
		'list'				=> $list, 
		'NAVBAR'				=> $navbar,
		'NAVBAR_2'			=> $navbar2,
		'GALLERY_STYLESHEET'	=> "",
		'header'				=> $header,
		'BACKGROUND_IMG_STYLE'	=> $bkgd,
		//'grouptitle'		=> $groupTitle,
		'subtitle'			=> FP_SYSTEM_DISPLAY_NAME,
		'GROUPICON'			=> $GroupIcon,
		'GROUPBANNER'		=> $GroupBanner,
		'GROUPBANNERURL'		=> $GroupBannerURL,
		'grouplogo'			=> $myGroup->icon,
		'title'				=> $title,
		'pagetitle' 			=> $title,
		'sectionclass'		=> "fullpage",
		'sampleimage'		=> $sampleImage,
		'RANDOM_IMG'			=> $randomimage,
		'RANDOM_IMG_LIST'		=> $randomImageList,
		'message'			=> $msg,
		'error'				=> $error,
		'master_page_popups'		=> FetchSnippet("client_access_dialog")
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