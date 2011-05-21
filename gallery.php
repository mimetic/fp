<?php
// =========== BEGIN CACHING CODE ================
// Include the package
require_once('Cache/Lite.php');

// Get the id for this cache
preg_match("|^/\w+/(.+?)/|",__FILE__, $m);
$username = $m[1];
$cachegroup = "project{$username}";
$cacheid = "project".$_REQUEST['ProjectID'];

// Force a clear of cache, e.g .after admin change?
isset($_REQUEST['clearcache']) && isset($_REQUEST['clearcache'])
	? $clearcache = $_REQUEST['clearcache']
	: $clearcache = null;

// Set a few options
// Cache is set to 1 hour: If pix arrive by email, or other non-admin method
// then we won't know the gallery needs updating.
$options = array(
	'cacheDir' => 'tmp/cache/',
	'automaticCleaningFactor' => 1,
	'lifeTime' => 3600	// one hour
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

	// Get user info, if it exists. It would come because the admin passed a user ID variable
	$vars['fp_user'] && $_SESSION['fp_user'] = $vars['fp_user'];

	// array used to substitute vars into the page with {keyname} codes
	$fields = array ();

	// GET PROJECT INFO
	$projectID = $vars['ProjectID'];
	$_SESSION['projectid'] = $projectID;
	$project = FetchProject ($projectID);
	$params = DecodeArrayFromDB($project['Params']);

	//$groupID = $_SESSION['GroupID'];
	$groupID = $project['GroupID'];
	$groupID && $_SESSION['GroupID'] = $groupID;
	isset($_SESSION['GroupID']) || $_SESSION['GroupID'] = PUBLIC_GROUP_ID;

	$myGroup = new FPGroup ($groupID);
	// Set the session vars for the current them
	CurrentThemeID ();

	$GroupBannerURL = $myGroup->LogoFilename();
	$GroupBanner = $myGroup->LogoHTML("style='border:1px solid black;margin-right:10px;'");
	$GroupIcon = $myGroup->IconHTML("style='border:1px solid black;margin-right:10px;'");


	// Captions: show, hide, popup
	// 'FP_DISPLAY_CAPTION_SHOW_HIDE' is a style setting for 'display:DISPLAY_CAPTION'
	// 'FP_DISPLAY_CAPTION_FLAG' is part a javascript call, e.g. myjs('DISPLAY_CAPTION_FLAG', param1)
	// to tell the the javascript whether or not to hide/show the caption
	// If 'false' then captions are hidden by default
	switch ($params[FP_PARAM_GALLERY_SHOWCAPTIONS]) {
	// 0=popup captions: originally hidden, switch on mouseover
	case "0" :
		$fields['FP_DISPLAY_CAPTION_SHOW_HIDE'] = FP_CSS_HIDE;
		$fields['FP_DISPLAY_CAPTION_FLAG'] = "0";
		break;
	// 1=show captions: originally show, don't switch on mouseover
	case "1" :
		$fields['FP_DISPLAY_CAPTION_SHOW_HIDE'] = FP_CSS_SHOW;
		$fields['FP_DISPLAY_CAPTION_FLAG'] = "1";
		break;
	// 2=hide captions" originally hidden, don't switch on mouseover
	case "2" :
		$fields['FP_DISPLAY_CAPTION_SHOW_HIDE'] = FP_CSS_HIDE;
		$fields['FP_DISPLAY_CAPTION_FLAG'] = "1";
		break;
	}

	// Show/Hide link to commenting system
	if ($params[FP_PARAM_GALLERY_COMMENTS]) {
	$commentlink = FetchSnippet('commentlink');
	} else {
	$commentlink = "";
	}

	// We add 1 to show captions because starting with zero is unwieldy...often shows up as "false".
	$fields['FP_SHOW_CAPTIONS'] = $params[FP_PARAM_GALLERY_SHOWCAPTIONS] + 1;

	$tables	= array ($PROJECTS => "$PROJECTS", $IMAGES => "Parts, Images, Artists");

	$comma = "if ((City != '') AND (Country != ''), ', ', '')";
	$d = 'DATE_FORMAT(Projects.ProjectDate, "%M %D, %Y") as ProjectDate';
	$c = "if (concat(City, Country) = '', '{fp:somewhere}', concat(City, $comma, Country)) AS Place";

	$URL = "URL";
	// CHOOSE TO MATTE OR NOT, BASED ON PROJECT SETTING
	if ($project['Matted']) {
	$picturestyle = $project['Matted'];
	}
	$fields['FP_SHOW_MATTED'] = $project['Matted'];
	
	// If the width is zero, then set to No Frame
	$project['Framewidth'] || $project['Framestyle'] = 0;
	// Show frame if there's a frame style, the width>0, or matte is set
	$frameShowHide = CSSShowHide (($project['Framestyle'] or $project['Matted']) and $project['Framewidth']);
	$framestyleID = $project['Framestyle'];

	if ($framestyleID) {
		$projectframewidth = $project['Framewidth'];	// not the same as the config.sys "framewidth", which has a setting for graphically drawing frames
	} else {
		$projectframewidth = 0;		// no frame style, then width = 0
	}
	
	$sets 	= array (
	$PROJECTS		=> "*, $d, $c",
	//$IMAGES			=> "Parts.ProjectID, Parts.PartID, Parts.OrderinProject, Artists.ID AS ArtistID, Lastname, Firstname, CONCAT_WS(' ', Firstname, Lastname) AS Fullname, $IMAGES.ID AS ImageID, Title, $URL, FrameID, RollID, Caption, $IMAGES.Params as ImageParams",
	$STORIES			=> "*, substr(Story,1,225) AS Lead"
	);

	// Isn't this better than the one above? It gets all fields.
	$sets[$IMAGES] = "*, Parts.ProjectID, Parts.PartID, Parts.OrderinProject, Artists.ID AS ArtistID, CONCAT_WS(' ', Firstname, Lastname) AS Fullname, $IMAGES.ID AS ImageID, $URL, $IMAGES.Params as Params";


	$projects_where = "$PROJECTS.ID = '" . $projectID . "'";
	$images_where = "Parts.ProjectID = '" . $projectID . "' AND Parts.PartTable = '$IMAGES' AND Parts.PartID = $IMAGES.ID AND Artists.ID = $IMAGES.ArtistID";

	$stories_where = "Parts.ProjectID = '" . $projectID . "' AND Parts.PartTable = '$STORIES' AND Parts.PartID = $STORIES.ID AND Artists.ID = $STORIES.ArtistID";

	$wheres	= array (
	$PROJECTS	=> $projects_where,
	$IMAGES		=> $images_where,
	$STORIES		=> $stories_where
	);

	$orders	= array (
	$PROJECTS	=> "Projects.Title",
	$IMAGES		=> "$PARTS.OrderInProject",
	$STORIES		=> "Title"
	);

	$navbar = NavBar ($groupID, 0);	// 0 means nothing is selected
	$navbar2 = NavBar ($groupID, "", 2);	// 1 means navbar item 1 is selected

	if ($myGroup->IsSolo()) {
		$formats = FetchFormatSet ('gallery_page_solo');
	} else {
		$formats = FetchFormatSet ('gallery_page');
	}


	$artists_limit = "";
	$title = $project['Title'];
	$skip_empty_rows = TRUE;

	$sspop = FetchSnippet ("gallery_slideshow_link_text");
	FP_SLIDESHOW_SV ? $sspopsv = FetchSnippet ("gallery_slideshow_sv_link_text") : $sspopsv = "";
	FP_SLIDESHOW_PC ? $sspoppc = FetchSnippet ("gallery_slideshow_pc_link_text") : $sspoppc = "";
	FP_SLIDESHOW_TV ? $sspoptv = FetchSnippet ("gallery_slideshow_tv_link_text") : $sspoptv = "";

	$DEBUG && $timer .= __FILE__.__LINE__.": Time elapsed: ". round (microtime(true) - $starttime, 2) . " seconds<BR>";

	$list = FetchCascade ($tables, $sets, $wheres, $orders, $formats, '', '', $skip_empty_rows, $picturestyle, $params);

	$DEBUG && $timer .= __FILE__.__LINE__.": Time elapsed: ". round (microtime(true) - $starttime, 2) . " seconds<BR>";

	// Build JS associative array of pictures for prev/next
	$l = FetchProjectImageOrder ($projectID);
	$piclist = join (",", array_keys($l));

	// Get Stories for this project
	$lineformat = FetchSnippet ("gallery_stories_list");
	$storyIDs = FetchProjectStories ($projectID);
	if ($storyIDs) {
	foreach ($storyIDs as $id) {
		$story = FetchStory ($id);
		$story['ProjectID'] = $projectID;
		$storylist .= Substitutions ($lineformat, $story);
	}
	$storylist = Substitutions (FetchSnippet ("gallery_stories_block"), array (	"list"			=> $storylist));
	} else {
	$storylist = "<!--no stories-->\n";
	}

	$output = FetchSnippet ('master_page');
	$header = FetchSnippet ("master_page_header");

	// If developing, don't use the google analytics stuff (slows us down)
	if ($DEVELOPING) {
		$Themes->FP_Themes[DEFAULT_THEME]['snippets']['google_analytics_js'] = '';
	}

	// If there's a snippet for the picture behind, then use it, else no format for random image
	$ri = FetchSnippet("picturebehind");
	$ri || $ri = "{Images_URL}";
	$randomimage = FetchRandomImage ("", $ri,null,null,$groupID);
	$randomImageList = FetchRandomImageList($groupID, $projectID);

	//$sampleImage = FetchSnippet ("smallslideshow");
	$sampleImage = "";
	$bkgd = "";

	$DEBUG && $timer .= __FILE__.__LINE__.": Time elapsed: ". round (microtime(true) - $starttime, 2) . " seconds<BR>";
	$processingTime = EndTimer();
	$msg .= $processingTime;

	$output = Substitutions ($output, array(
	'list'				=> $list,
	'META_INDEX'			=> MetaIndexCode (DB_PROJECTS, $projectID),
	'NAVBAR'				=> $navbar,
	'NAVBAR_2'			=> $navbar2,
	'header'				=> $header,
	'storylist'			=> $storylist,
	'master_page_popups'		=> FetchSnippet ("master_page_popups"),
	'parent'				=> $myGroup->title,
	'title' 				=> $title,
	'pagetitle' 			=> $title,
	'subtitle'			=> $myGroup->title,
	//'subtitle'			=> FP_SYSTEM_DISPLAY_NAME,
	// Don't add a background picture
	'BACKGROUND_IMG_STYLE'		=> $bkgd,
	'GROUPICON'			=> $GroupIcon,
	'GROUPBANNER'			=> $GroupBanner,
	'GROUPBANNERURL'			=> $GroupBannerURL,
	'sectionclass'			=> "gallery",	//class name for some objects
	'message'	 		=> $msg,
	'error' 			=> $error,
	'pps' 				=> $project['pps'],
	'picturestyle' 			=> $picturestyle,
	'commentlink'			=> $commentlink,
	'slideshowpopupsv'		=> $sspopsv,
	'slideshowpopuppc'		=> $sspoppc,
	'slideshowpopuptv'		=> $sspoptv,
	'slideshowpopup'		=> $sspop,
	'sampleimage'			=> $sampleImage,
	'RANDOM_IMG'			=> $randomimage,
	'RANDOM_IMG_LIST'		=> $randomImageList,
	'PROJECTFRAMEWIDTH'		=> $projectframewidth,
	'FRAME_SHOW_HIDE'		=> $frameShowHide,
	'FRAMESTYLE_ID'			=> $framestyleID,
	'PROJECT_ID'			=> $projectID
	), false);
	// the FALSE means no debugging (true means debug)

	$DEBUG && $timer .= __FILE__.__LINE__.": Time elapsed: ". round (microtime(true) - $starttime, 2) . " seconds<BR>";
	$fields['piclist'] = $piclist;

	$output = Substitutions ($output, $fields);
	$output = ReplaceAllSnippets ($output);

	$output = ReplaceSysVars ($output);
	$output = DeleteUnusedSnippets ($output);

	mysql_close($LINK);
	$FP_MYSQL_LINK->close();
	
	$DEBUG && print $timer;

	$output = compress_html($output);
	$DEVELOPING || $Cache_Lite->save($output, $cacheid, $cachegroup);
}
print $output;

?>