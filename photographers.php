<?php

/*
Photographers page:
For this group, show list of 
	- active artists (by name)
		- projects for each artist
		(image sample)
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
	
	$DEBUG = false;
	
	$error = "";
	$msg = "";
	
	session_name("fp_gallery_session");
	session_start();
	
	$LINK = StartDatabase(MYSQLDB);
	Setup ();
	
	$actions = $results['actions'];
	$action = $results['actions']['action'];
	$vars = $results['vars'];
	isset($vars['GroupID']) && $_SESSION['GroupID'] = $vars['GroupID'];
	isset($_SESSION['GroupID']) || $_SESSION['GroupID'] = 1;
	
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
	
	// this code searches for :
	// 1) Artists who have images in a project in the current group
	// 2) Get projects which have images of the artist
	// So, it shows artists who have active images, and the projects they have images in.
	
	$tables	= array (
		DB_ARTISTS 		=> DB_ARTISTS . "," . DB_PARTS, 
		DB_PROJECTS		=> DB_PROJECTS . "," . DB_PARTS
	);
	  
	
	
	$d = "DATE_FORMAT($PROJECTS.ProjectDate, '%M %D, %Y') as ProjectDate";
	$d .= ", if (length($PROJECTS.Description) < 150, concat($PROJECTS.Description, if ($PROJECTS.Description != '', '', '')), concat(substring($PROJECTS.Description,1,150),'...<BR>')) AS Lead ";
	  
	$sets 	= array (
		DB_ARTISTS	=>	"DISTINCT $ARTISTS.ID as ArtistID, $ARTISTS.Lastname, $ARTISTS.Firstname, $ARTISTS.Email as Email, CONCAT_WS(' ', $ARTISTS.Firstname, $ARTISTS.Lastname) AS Fullname, $ARTISTS.GroupID",
		DB_PROJECTS	=>	"DISTINCT " . ProjectsCalcFields("$PROJECTS.*, $d")
					 );
	
	$artists_where = "$PARTS.ArtistID != " . FP_ADMINISTRATOR . " AND $PARTS.PartTable = '$IMAGES' AND $PARTS.ArtistID = $ARTISTS.ID AND $PARTS.ProjectID IN (SELECT ID FROM $PROJECTS WHERE GroupID = '$groupID')";
	
	$projects_where = "($PROJECTS.Public = 0) AND NOT ($PROJECTS.Slides <=> 1) AND $PROJECTS.ArtistID != " . FP_ADMINISTRATOR . " AND  $PARTS.ArtistID = {Artists_ArtistID} AND $PARTS.PartTable = 'Images' AND $PARTS.ProjectID = $PROJECTS.ID AND $PROJECTS.ID IN (SELECT ID FROM $PROJECTS WHERE GroupID = '$groupID') GROUP BY $PROJECTS.Title";
	
	// Hey! uncommenting the following line
	// controls whether we show all Active (but NOT featured) projects for each artist	
	//$projects_where = GetActiveWhere($projects_where);
	
	$wheres	= array ("Artists"		=> $artists_where,
					 "Projects"		=> $projects_where
					 );
	
	$orders	= array ("Artists"		=>	"Lastname",
					 "Projects"		=>	"Title"
					 );
					 
	
	$navbar = NavBar ($groupID, FP_ACTIVE);	// 3 means navbar item 3 is selected
	$navbar2 = NavBar ($groupID, FP_ACTIVE, 2);	// 1 means navbar item 1 is selected
	
	if ($myGroup->IsSolo(true)) {
		$formats = FetchFormatSet ('photographers_page_solo');
		$groupTitle = "";
	} else {				 
		$formats = FetchFormatSet ('photographers_page');
		$groupTitle = $myGroup->title;
	}

	$skip_empty_rows = TRUE;
	$list = FetchCascade ($tables, $sets, $wheres, $orders, $formats, '', '', $skip_empty_rows);
	
	// A snippet value in the current them can turn on/off the slide show
	// Default is empty, meaning DON'T hide, or 'on'.
	$hideSlideShow = FetchSnippet ("hide_slide_show");
	if (!$hideSlideShow) {
		if ($FrontPageSlideShow) {
			$sampleImage = FetchSnippet ("smallslideshow");
			// set matted to any value to show matte
			$ssp_params = "duration=auto&audio=off&size=gallery&GroupID={$groupID}";
			$ssp_params = EncodeParamsForFlash ($ssp_params);
			$sampleImage = str_replace ("{SSP_PARAMS}", "?$ssp_params" , $sampleImage);
			$slideshow_head = FetchSnippet ("ssp_head_small");
		} else {
			$sampleImage = FetchRandomImage ("", 'sample_picture',null,null,$groupID);
			$ssp_params = "";
			$slideshow_head = "";
		}
	} else {
		$sampleImage = "";
		$slideshow_head = "";
	}
	
	// this applies to the pop-up picture...anything will do
	$framestyle = 1;
	
	$page = FetchSnippet ('master_page');
	$header = FetchSnippet ("master_page_header");
	// If there's a snippet for the picture behind, then use it, else no format for random image
	$ri = FetchSnippet("picturebehind");
	$ri || $ri = "{Images_URL}";
	$randomimage = FetchRandomImage ("", $ri,null,null,$groupID);
	$randomImageList = FetchRandomImageList($groupID);

	$showBkgdImage = FetchSnippet ("show_bkgd_image");
	$showBkgdImage 
		? $bkgd = FetchSnippet ("background_img_style")
		: $bkgd = "";
	
	
	$output = Substitutions ($page, array(
		'list'	=> $list, 
		'META_INDEX'			=> MetaIndexCode (DB_GROUPS, $groupID),
		'NAVBAR'					=> $navbar,
		'NAVBAR_2'					=> $navbar2,
		'GALLERY_STYLESHEET'	=>		"",
		'header'					=> $header,
		'BACKGROUND_IMG_STYLE'	=> $bkgd,
		//'subtitle'			=>	$groupTitle,
		'subtitle'				=> FP_SYSTEM_DISPLAY_NAME,
		'GROUPICON'			=> $GroupIcon,
		'GROUPBANNER'		=> $GroupBanner,
		'GROUPBANNERURL'		=> $GroupBannerURL,
		'sectionclass'			=>	"listpage",
		'title'					=> 	$myGroup->title,
		'pagetitle'				=> 	"{photographer_page_title}",
		'sampleimage'		=> $sampleImage,
		'SLIDESHOW_HEAD'			=> $slideshow_head,
		'SSP_PARAMS'			=> $ssp_params,
		'RANDOM_IMG'			=> $randomimage,
		'RANDOM_IMG_LIST'		=> $randomImageList,
		'FRAMESTYLE'			=> $framestyle,
		'master_page_popups'		=> FetchSnippet("client_access_dialog"),
		'message' 				=> $msg,
		'error'					=>	$error
		));
	$output = ReplaceAllSnippets ($output);
	$output = ReplaceSysVars ($output);
	$output = DeleteUnusedSnippets ($output);

	$output = compress_html($output);
	$DEVELOPING || $DEVELOPING || $Cache_Lite->save($output, $cacheid, $cachegroup);
}	
print $output;

?>