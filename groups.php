<?php
/*
Groups page
Show list of active groups

A 'group' is a 'gallery,' just a more generic name.
*/

// =========== BEGIN CACHING CODE ================
// Include the package
require_once('Cache/Lite.php');

// Get the id for this cache (use site name, this is the top level)
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
	
	$error = "";
	$msg = "";
	
	$LINK = StartDatabase(MYSQLDB);
	Setup ();
	
	$DEBUG = false;
	
	session_name("fp_gallery_session");
	session_start();
	
	$_SESSION['GroupID'] = PUBLIC_GROUP_ID;
	$_SESSION['projectid'] = null;
	
	$results = GetFormInput();
	$actions = $results['actions'];
	$action = $results['actions']['action'];
	$vars = $results['vars'];
	
	// Get user info, if it exists. It would come because the admin passed a user ID variable
	$vars['fp_user'] && $_SESSION['fp_user'] = $vars['fp_user'];

	$groupID = $_SESSION['GroupID'];
	$myGroup = new FPGroup ($groupID);
	$myTheme = CurrentThemeID ();

	$GroupBannerURL = $myGroup->LogoFilename();
	$GroupBanner = $myGroup->LogoHTML("style='border:1px solid black;margin-right:10px;'");
	$GroupIcon = $myGroup->IconHTML();

	$hidelist = $myGroup->GetParam(FP_PARAM_GROUP_HIDE_LISTING);
	
	$formats = FetchFormatSet ('groups_page');

	// A snippet value in the current them can turn on/off the slide show
	// Default is empty, meaning DON'T hide, or 'on'.
	$hideSlideShow = FetchSnippet ("hide_slide_show");
	$DEVELOPING && $hideSlideShow = true;

	if (!$hideSlideShow) {
		if ($FrontPageSlideShow) {
			$sampleImage = FetchSnippet ("smallslideshow");
			// set matted to any value to show matte
			$ssp_params = "duration=auto&audio=off&size=gallery&action=active";
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
	
	// Unused for now...
	$showmode = $vars['showmode'];
	
	// Set with a snippet — if it has a value, then do it.
	$displayCascade = trim(FetchSnippet ("groups_page_display_cascade"));
	if (trim(strtolower($displayCascade)) == "false") {
		$displayCascade = false;
	}
	
	// testing
	//$displayCascade = true;
	
	if ($hidelist) {
		$list = "<!-- hidden -->";
	} else {
		if ($displayCascade) {
			$list = GetGroupCascade($showmode);
		} else {
			$list = DisplayGroupsList ($vars['start']);
		}
	}
	
	// We use the site logo on this page!
	
	$page = FetchSnippet ("master_page");
	$header = FetchSnippet ("master_page_header_groups");
	// If there's a snippet for the picture behind, then use it, else no format for random image
	$ri = FetchSnippet("picturebehind");
	// if the picturebehind snippet exists, the contents of picturebehind snippet is the name of the snippet to use for the background image.  
	$ri || $ri = "{Images_URL}";
	$randomimage = FetchRandomImage ("", $ri,null,null,$groupID);

	$randomImageList = FetchRandomImageList($groupID);
	
	
	$navbar = FetchSnippet ('navbar-off');	// we use of limited navbar on this page
	$navbar2 = NavBar ($groupID, "new", 2);	// 1 means navbar item 1 is selected, 2 is bottom navbar
	$showBkgdImage = FetchSnippet ("show_bkgd_image");
	$showBkgdImage 
		? $bkgd = FetchSnippet ("background_img_style")
		: $bkgd = "";
	
	$output = Substitutions ($page, array(
		'META_INDEX'			=> MetaIndexCode (DB_GROUPS, $groupID),
		'NAVBAR'				=> $navbar,
		'NAVBAR_2'			=> $navbar2,
		'GALLERY_STYLESHEET'	=> "",
		'header'				=> $header,
		'background_img_style'		=> $bkgd,
		'list'				=> $list, 
		'sectionclass'		=> "groups",
		'GROUPICON'			=> $GroupIcon,
		'GROUPBANNER'		=> $GroupBanner,
		'GROUPBANNERURL'		=> $GroupBannerURL,
		'GROUPLOGO'			=> "",	// unused
		'title'				=> FP_SYSTEM_DISPLAY_NAME,
		'subtitle' 			=> "&nbsp;",
		'pagetitle' 			=> "{groups_page_title}",
		'Description'		=> $myGroup->info['Description'],
		'Statement'			=> $myGroup->info['Statement'],
		'sampleimage' 		=> $sampleImage,
		'SLIDESHOW_HEAD'		=> $slideshow_head,
		'SSP_PARAMS'			=> $ssp_params,
		'RANDOM_IMG'			=> $randomimage,
		'RANDOM_IMG_LIST'		=> $randomImageList,
		'message' 			=> $msg,
		'error' 				=> $error,
		'master_page_popups'		=> FetchSnippet("client_access_dialog")
		));
	$output = ReplaceAllSnippets ($output);
	$output = ReplaceSysVars ($output);
	$output = DeleteUnusedSnippets ($output);
	
	mysql_close($LINK);
	$FP_MYSQL_LINK->close();

	$output = compress_html($output);
	$DEVELOPING || $Cache_Lite->save($output, $cacheid, $cachegroup);
}	
print $output;





// ------------------------------------------------------------------------
// Get a cascade, starting with groups
// NOT ACTIVE YET: $showmode : FP_FEATURED = show featured projects, FP_ACTIVE = show active projects

function GetGroupCascade ($showmode = null) {
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	
	$myGroup || $myGroup = new FPGroup();
	$groupID = $myGroup->ID;
	
	$tables	= array (
		"Groups"	=> "$GROUPS, $PROJECTS, $PARTS",
		"Projects" => "$PROJECTS, $PARTS", 
		"ArtistInfo" => "$ARTISTS, $PARTS"
	);
	// Most themes won't show a list of images with each project, but one might wish to.
	// We use a snippet...if it resolves to TRUE, we create images!
	// By making this option, we speed up the searches. Why grab a large stack of image records unnecessarily?
	$showimages = FetchSnippet("projects_show_images");
	if ($showimages) {
		$tables["Images"] = "$PARTS, $IMAGES";
	}
		

	// SETS

	$d = "DATE_FORMAT($PROJECTS.ProjectDate, '%M %D, %Y') as ProjectDate";
	$d .= ", if (length($PROJECTS.Description) < 150, concat($PROJECTS.Description, if ($PROJECTS.Description != '', '<BR>', '')), concat(substring($PROJECTS.Description,1,150),'...<BR>')) AS Lead ";
	
	$sets = array (
		"Groups"		=>	"DISTINCT $GROUPS.*, TRIM(LEADING ' ' from $GROUPS.Title) as Title, TRIM(LEADING 'The ' from $GROUPS.Title) as SortTitle",
		"Projects"		=>	"DISTINCT $PROJECTS.ID, ".ProjectsCalcFields("$PROJECTS.*, $d"),
		"ArtistInfo"	=>	"DISTINCT $ARTISTS.ID AS ArtistID, $ARTISTS.Firstname as fn, $ARTISTS.Lastname as qq, $PARTS.ProjectID, $ARTISTS.Firstname, CONCAT_WS(' ', $ARTISTS.Firstname, $ARTISTS.Lastname) AS Fullname"
	);


	$subquery = "SELECT DISTINCT $PARTS.projectID from $PROJECTS, $PARTS WHERE $PROJECTS.ID = $PARTS.ProjectID AND $PARTS.PartTable = 'Images' AND $PROJECTS.Public < 1 AND $PROJECTS.Slides < 1";
	$Groups_where = "$PROJECTS.GroupID = $GROUPS.ID AND $PROJECTS.ID IN ($subquery)";

	$Projects_where = "($PROJECTS.Public = 0) AND NOT ($PROJECTS.Slides <=> 1) AND $PROJECTS.ArtistID != " . FP_ADMINISTRATOR . " AND $PROJECTS.GroupID = {Groups_ID} AND $PROJECTS.ID = $PARTS.ProjectID AND $PARTS.PartTable = '{$IMAGES}'";

	//$Artists_where = "$PARTS.ProjectID = '{Projects_ID}' AND $ARTISTS.ID = $PARTS.ArtistID";
	$Artists_where = "Artists.ID = Parts.ArtistID AND $PARTS.ProjectID = '{Projects_ID}'";
	
	$wheres	= array (
						"Groups"		=>	$Groups_where,
						"Projects"		=>	$Projects_where,
						"ArtistInfo"		=>	$Artists_where
					 );
			
	$orders	= array (
						"Groups"		=>	"SortTitle",
						"Projects"		=>	"$PROJECTS.Title",
						"ArtistInfo"	=>	"$ARTISTS.Lastname, $ARTISTS.Firstname"
					 );

	if ($myGroup->IsSolo(true)) {
		$formats = FetchFormatSet ('groups_page_solo');
	} else {				 
		$formats = FetchFormatSet ('groups_page');
	}
	
	$list = FetchCascade ($tables, $sets, $wheres, $orders, $formats, '', '', true);

	return $list;
}


?>