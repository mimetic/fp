<?php
/*
Projects page
	New projects
		artists
			images
			
&showmode = 'active' = active projects, default is featured
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

file_exists('tmp/cache/') || mkdir ('tmp/cache', 0755);

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
	$_SESSION['projectid'] = null;
	
	// Get user info, if it exists. It would come because the admin passed a user ID variable
	$vars['fp_user'] && $_SESSION['fp_user'] = $vars['fp_user'];
	
	$groupID = $_SESSION['GroupID'];
	$myGroup = new FPGroup ($groupID);
	$_SESSION['theme'] = $myGroup->theme;

	$hidelist = $myGroup->GetParam(FP_PARAM_GROUP_HIDE_LISTING);

	$GroupBannerURL = $myGroup->LogoFilename();
	$GroupBanner = $myGroup->LogoHTML("style='border:1px solid black;margin-right:10px;'");
	$GroupIcon = $myGroup->IconHTML();
	
	$showmode = $vars['showmode'];
	$projects_where = "";
	
	switch ($showmode) {
		case 'active' : 
			$pageid = FP_ACTIVE;
			$hidelist
			? $list = "<!-- hidden -->"
			: $list = GetProjectCascade ($myGroup, FP_ACTIVE);
			break;
		case 'nolist' :
			$list = "";
			break;
		default:
			$pageid = FP_FEATURED;
			$hidelist
			? $list = "<!-- hidden -->"
			: $list = GetProjectCascade ($myGroup, FP_FEATURED);
			break;
	}

	$navbar = NavBar ($groupID, $pageid);	// 1 means navbar item 1 is selected
	$navbar2 = NavBar ($groupID, $pageid, 2);	// 1 means navbar item 1 is selected, 2 is bottom navbar

	$DEBUG && $msg .= __FILE__.__LINE__.": Time elapsed: ". round (microtime(true) - $starttime, 2) . " seconds<BR>";
	
	// A snippet value in the current them can turn on/off the slide show
	// Default is empty, meaning DON'T hide, or 'on'.
	$hideSlideShow = FetchSnippet ("hide_slide_show");
	//$DEVELOPING && $hideSlideShow = true;
	if (!$hideSlideShow) {
		if ($FrontPageSlideShow) {
			$sampleImage = FetchSnippet ("smallslideshow");
			// set matted to any value to show matte
			$ssp_params = "action=featured&duration=auto&audio=off&size=gallery&GroupID={$groupID}";
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

	$page = FetchSnippet ('master_page');
	$header = FetchSnippet ("master_page_header");
	// If there's a snippet for the picture behind, then use it, else no format for random image
	$ri = FetchSnippet("picturebehind");
	$ri || $ri = "{Images_URL}";
	$randomimage = FetchRandomImage ("", $ri,null,null,$groupID);
	$randomImageList = FetchRandomImageList($groupID, $projectID);

	$showBkgdImage = FetchSnippet ("show_bkgd_image");
	$showBkgdImage 
		? $bkgd = FetchSnippet ("background_img_style")
		: $bkgd = "";
	
	$output = Substitutions ($page, array(
		'list'				=> $list, 
		'META_INDEX'			=> MetaIndexCode (DB_GROUPS, $groupID),
		'NAVBAR'			=> $navbar,
		'NAVBAR_2'			=> $navbar2,
		'GALLERY_STYLESHEET'		=>		"",
		'header'			=> $header,
		'BACKGROUND_IMG_STYLE'		=> $bkgd,
		'parent'			=> $myGroup->title,
		'title'				=> $myGroup->title,
		'subtitle'			=> FP_SYSTEM_DISPLAY_NAME,
		'GROUPICON'			=> $GroupIcon,
		'GROUPBANNER'			=> $GroupBanner,
		'GROUPBANNERURL'		=> $GroupBannerURL,
		'pagetitle'			=> $myGroup->title,
		'Description'			=> $myGroup->info['Description'],
		'Statement'			=> $myGroup->info['Statement'],
		'sampleimage'			=> $sampleImage,
		'SLIDESHOW_HEAD'		=> $slideshow_head,
		'SSP_PARAMS'			=> $ssp_params,
		'RANDOM_IMG'			=> $randomimage,
		'RANDOM_IMG_LIST'		=> $randomImageList,
		'sectionclass'			=> "listpage",
		'message'			=> $msg,
		'error'				=> $error,
		'master_page_popups'		=> ""	// don't need these
		));

	$output = ReplaceAllSnippets ($output);
	$output = ReplaceSysVars ($output);
	$output = DeleteUnusedSnippets ($output);
	
	mysql_close($LINK);
	$FP_MYSQL_LINK->close();

	$output = compress_html($output);
	$DEVELOPING || $DEVELOPING || $Cache_Lite->save($output, $cacheid, $cachegroup);
}	
print $output;


// ------------------------------------------------------------------------

function GetProjectCascade ($myGroup, $showmode = null) {
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	
	$myGroup || $myGroup = new FPGroup();
	$groupID = $myGroup->ID;
	
	$tables	= array (
		"Projects" => "$PROJECTS, $PARTS", 
		"ArtistPart" => "$ARTISTS, $PARTS"
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
		"Projects"		=>	"DISTINCT " . ProjectsCalcFields("$PROJECTS.*, $d"),
		"ArtistPart"		=>	"DISTINCT $PARTS.ProjectID, $ARTISTS.ID AS ArtistID, $ARTISTS.Lastname, $ARTISTS.Firstname, CONCAT_WS(' ', $ARTISTS.Firstname, $ARTISTS.Lastname) AS Fullname",
		"Images"		=> "DISTINCT $IMAGES.*, $IMAGES.ID AS ImageID"
	);
	
	
	// WHERE

	// Don't show sys admin projects and slide projects
	$projects_where = "($PROJECTS.Public = 0) AND NOT ($PROJECTS.Slides <=> 1) AND $PROJECTS.ArtistID != " . FP_ADMINISTRATOR . " AND $PROJECTS.GroupID = $groupID AND $PROJECTS.ID = $PARTS.ProjectID AND $PARTS.PartTable = '{$IMAGES}'";
	switch ($showmode) {
		case FP_FEATURED :
			$projects_where = GetFeaturedWhere ($projects_where);
			break;
		case FP_ACTIVE :
			$projects_where = GetActiveWhere ($projects_where);
			break;
		default :
			break;
	}

	$ArtistPart_where = "$PARTS.ProjectID = '{Projects_ID}' AND $PARTS.PartTable = '{$IMAGES}' AND $PARTS.ArtistID = $ARTISTS.ID";
	$Images_where = "$PARTS.ArtistID = {ArtistPart_ArtistID} AND $PARTS.ProjectID = {Projects_ID} AND $PARTS.PartTable = '$IMAGES' and $PARTS.PartID = $IMAGES.ID";
	
	$wheres	= array (
						"Projects"		=>	$projects_where,
						"ArtistPart"	=>	$ArtistPart_where,
						"Images"		=>	$Images_where
					 );
			
	$orders	= array (
						"Projects"		=>	"$PROJECTS.Title",
						"ArtistPart"	=>	"$ARTISTS.Lastname, $ARTISTS.Firstname",
						"Images"		=>	 "$PARTS.OrderInProject"
					 );

	if ($myGroup->IsSolo(true)) {
		$formats = FetchFormatSet ('projects_page_solo');
	} else {				 
		$formats = FetchFormatSet ('projects_page');
	}
	
	$list = FetchCascade ($tables, $sets, $wheres, $orders, $formats, '', '', true);

	return $list;
}


?>