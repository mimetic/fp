<?php

/*
Places page
	project (by country, city)
		artists
			images
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
	
	$tables	= array (
		'Places' => "$PROJECTS, $PARTS",
		'Projects' => "$PROJECTS, $PARTS",
		'Artists' => "$PARTS, $ARTISTS"
		);
	
	
	// Most themes won't show a list of images with each project, but one might wish to.
	// We use a snippet...if it resolves to TRUE, we create images!
	// By making this option, we speed up the searches. Why grab a large stack of image records unnecessarily?
	$showimages = FetchSnippet("projects_show_images");
	if ($showimages) {
		$tables['Images'] = "$PARTS, $IMAGES";
	}

	$comma = "if ((City != '') AND (Country != ''), ', ', '')";
	
	// NOTE THAT CONCAT_WS really sucks...it can't deal with empty strings which are not null.
	$sets 	= array (
		"Places"	=>	"DISTINCT City, Country, 
						if (concat(City, Country) = '', '{fp:Elsewhere}', CONCAT_WS(', ', if(City = '',NULL, City), Country)) AS Place",
		"Projects"	=>	"DISTINCT $PROJECTS.ID, $PROJECTS.ArtistID as OwnerID, $PROJECTS.Title, City, Country, 
						if (concat(City, Country) = '', '{fp:somewhere}', CONCAT_WS(', ', if(City = '',NULL, City), Country)) AS Place,
						(TO_DAYS(NOW()) - TO_DAYS(Projects.LastUpdate)) AS Age",
		"Artists"	=>	"DISTINCT $ARTISTS.ID AS ArtistID, CONCAT_WS(' ', Firstname, Lastname) AS ArtistFullname, $ARTISTS.Lastname, $ARTISTS.Firstname",
		"Images"		=> "DISTINCT $IMAGES.*, $IMAGES.ID AS ImageID"
		);
	

	$places_where =  "GroupID = '$groupID'";
	
	$projects_where = "($PROJECTS.Public = 0) AND NOT ($PROJECTS.Slides <=> 1) AND $PROJECTS.ArtistID != " . FP_ADMINISTRATOR . " AND $PROJECTS.City = '{Places_City}' AND $PROJECTS.Country = '{Places_Country}' AND $PROJECTS.ID IN (SELECT ID FROM $PROJECTS WHERE GroupID = '$groupID')";
	
	$projects_where .= " AND $PROJECTS.ID = $PARTS.ProjectID AND $PARTS.PartTable = '$IMAGES'"; // be sure project has pictures
					
	$projects_where = GetFeaturedWhere ($projects_where);
		
	$artists_where = "($PARTS.PartTable = '$ARTISTS' AND $PARTS.ProjectID = {Projects_ID} AND $PARTS.ArtistID = $ARTISTS.ID) OR ($ARTISTS.ID = {Projects_OwnerID})";
	
	$Images_where = "$PARTS.ArtistID = {Artists_ArtistID} AND $PARTS.ProjectID = {Projects_ID} AND $PARTS.PartTable = '$IMAGES' and $PARTS.PartID = $IMAGES.ID";

	$wheres	= array (
		"Places"	=>	$places_where,
		"Projects"	=>	$projects_where,
		"Artists"	=>	$artists_where,
		"Images"	=>	$Images_where
		);
	
	$orders	= array (
		"Places"	=> "GROUP BY Place",
		"Projects"	=> "Title",
		"Artists"	=> "Lastname",
		"Images"	=> "$PARTS.OrderInProject"
		);
	
	// select Title,Description, concat(City, ", ", Country) AS Place 
	// from Projects, Topics 
	// where TopicID = Topics.ID 
	// ORDER BY Topics.Country;
	
	$navbar = NavBar ($groupID, "place");	// 2 means navbar item 2 is selected
	$navbar2 = NavBar ($groupID, "place", 2);	// 1 means navbar item 1 is selected
	
	if ($myGroup->IsSolo(true)) {
		$formats = FetchFormatSet ('places_page_solo');
	} else {				 
		$formats = FetchFormatSet ('places_page');
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
		'list'					=> $list, 
		'META_INDEX'			=> MetaIndexCode (DB_GROUPS, $groupID),
		'NAVBAR'					=> $navbar,
		'NAVBAR_2'				=> $navbar2,
		'GALLERY_STYLESHEET'		=>		"",
		'header'					=> $header,
		'BACKGROUND_IMG_STYLE'	=> $bkgd,
		'title'					=> $myGroup->title,
		'subtitle'				=> FP_SYSTEM_DISPLAY_NAME,
		'pagetitle'				=> 	"{places_page_title}",
		'GROUPICON'				=> $GroupIcon,
		'GROUPBANNER'		=> $GroupBanner,
		'GROUPBANNERURL'		=> $GroupBannerURL,
		'sampleimage'		=> $sampleImage,
		'SLIDESHOW_HEAD'			=> $slideshow_head,
		'SSP_PARAMS'			=> $ssp_params,
		'RANDOM_IMG'			=> $randomimage,
		'RANDOM_IMG_LIST'		=> $randomImageList,
		'sectionclass'			=> "listpage",
		'master_page_popups'		=> FetchSnippet("client_access_dialog"),
		'message'				=> $msg,
		'error'					=> $error
		));
	
	$output = ReplaceAllSnippets ($output);
	$output = ReplaceSysVars ($output);
	$output = DeleteUnusedSnippets ($output);
	
	mysqli_close($LINK);
	//$FP_MYSQL_LINK->close();

	$output = compress_html($output);
	$DEVELOPING || $DEVELOPING || $Cache_Lite->save($output, $cacheid, $cachegroup);
}	
print $output;

?>