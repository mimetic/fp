<?php

/*
Photographers page
Show list of active artists (by name)
		projects
		(image sample)
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
	
	// isset($_REQUEST['theme']) && $_SESSION['theme'] = $_REQUEST['theme'];
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
	
	$ArtistID = $vars['ArtistID'];
	$list = FetchProjectList ($ArtistID);
	$list2 = FetchStoryList ($ArtistID);
	
	$artistinfo = FetchArtist ($ArtistID);
	
	// Fetch remote artist info
	$shortname = $artistinfo['ShortName'];
	
	// name of an uploaded portrait
	//$extrastyle = "style='max-width:350px;max-height:350px;'";
	$extrastyle = "";
	$artistPortraitIMG = FetchArtistPortrait ($ArtistID,null,$extrastyle);
	
	if ($artistPortraitIMG) {	
		$sampleImage = Substitutions (FetchSnippet ("artist_page_portrait"), array  ('picture'=>$artistPortraitIMG));
	} else {
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
	}
	$randomImageList = FetchRandomImageList($groupID);
	
	// Statement is on the master template, and not where we want the Artist's Statement.
	// We're going to move the Statement field to ArtistStatement, then remove the Statement field.
	$artistinfo['ArtistStatement'] = $artistinfo['Statement'];
	unset($artistinfo['Statement']);
	
	// Auto-format artist info fields:
	$arr = array (	"Biography"	=> "Biography", 
					"Statement"	=> "ArtistStatement", 
					"Awards"	=> "Awards", 
					"Exhibitions"	=> "Exhibitions", 
					"Publications"	=> "Publications", 
					"Full Biography"	=> "FullBiography"
					);
	
	$format = FetchSnippet ("artist_page_info_block");
	
	while (list ($n,$fn) = each ($arr)) {
		$f = $artistinfo[$fn];
		if ($f) {
			$f = LimitHTLM ($f);
			$f = str_replace ("\n", "<br>", $f);
			$f = Substitutions ($format, array ("title"=>$n, "text"=>$f));
		} else {
			$f = "<!-- no entry for $fn -->";
		}
		$artistinfo[$fn] = $f;
	}
	
	// Artist's email, prepped for cloaking using the "mailme" php function
	$email = $artistinfo['Email'];
	$obfuscatedEmail = base64_encode($email);
	
	// Other contact info
	$telformat = FetchSnippet ("telephone");
	
	$artistinfo['Tel1'] ? $tel1 = Substitutions ($telformat, array("telephone"=>$artistinfo['Tel1'])) : $tel1 = "";
	$artistinfo['Tel2'] ? $tel2 = Substitutions ($telformat, array("telephone"=>$artistinfo['Tel2'])) : $tel2 = "";
	$artistinfo['Tel3'] ? $tel3 = Substitutions ($telformat, array("telephone"=>$artistinfo['Tel3'])) : $tel3 = "";
	
	$artistinfo['Agency'] ? $agency= Substitutions (FetchSnippet ("agency"), array("agency"=>$artistinfo['Agency'])) : $agency="";
	
	$artistinfo['Website'] ? $website = Substitutions (FetchSnippet ("website_link"), array("Website"=>$artistinfo['Website'])) : $website = "";

	$artistinfo['ShortName'] ? $lightstalkers = Substitutions (FetchSnippet ("lightstalkers_link"), array("shortname"=>$artistinfo['ShortName'])) : $lightstalkers = "";

	$navbar = NavBar ($groupID, "about");	// 4 means navbar item 4 is selected
	$navbar2 = NavBar ($groupID, "about", 2);	// 1 means navbar item 1 is selected
	
	/*
	// LS doesn't do XML anymore, I think...
	$LSArtistInfo = "";
	if ($shortname) {
		//$URL = "http://www.lightstalkers.org/remote-xml-profile.cfm?s=$shortname";
		$URL = "http://www.lightstalkers.org/xml/member/$shortname";
		$LSArtistInfo = FetchRemoteLSData ($URL, "LS_");
	}
	*/

	$info = FetchSnippet ("artist_page_info");
	$info = Substitutions ($info, array(
		'list'		=> $list, 
		'list2' 		=> $list2,
		'LSArtistInfo' 	=> $LSArtistInfo
		));
							
	$params = unserialize($artistinfo['Params']);
	$blogblock = "<!-- no blog -->";		// default
	$blog = GetParam ($params, FP_PARAM_ARTIST_BLOG);
	if ($blog) {
		$blogblock = Substitutions (FetchSnippet ("artist_blog_block"), array ("BLOG" => $blog) );
	}
	
	
	$page = FetchSnippet ("master_page");
	$header = FetchSnippet ("master_page_header");
	/*
	// If there's a snippet for the picture behind, then use it, else no format for random image
	// $ri = FetchSnippet("picturebehind");
	$ri || $ri = "{Images_URL}";
	$randomimage = FetchRandomImage ('', $ri);
	*/
	// DON'T show background image!
	$randomimage = "";
	$bkgd = "";
	
	// NOTE: We don't show background pictures on this page
	$output = Substitutions ($page, array(
		'META_INDEX'				=> MetaIndexCode (DB_ARTISTS, $ArtistID),
		'list'					=> $info,
		'NAVBAR'					=> $navbar,
		'NAVBAR_2'				=> $navbar2,
		'GALLERY_STYLESHEET'		=>		"",
		'header'					=> $header,
		'BACKGROUND_IMG_STYLE'	=> $bkgd,
		'title' 					=> "{fp:About} " . $artistinfo['Fullname'],
		'subtitle'				=> FP_SYSTEM_DISPLAY_NAME,
		'GROUPICON'				=> $GroupIcon,
		'GROUPBANNER'			=> $GroupBanner,
		'GROUPBANNERURL'			=> $GroupBannerURL,
		'sectionclass'			=> "fullpage",
		'sampleimage'			=> $sampleImage,
		'SLIDESHOW_HEAD'			=> $slideshow_head,
		'SSP_PARAMS'				=> $ssp_params,
		'RANDOM_IMG'				=> $randomimage,
		'RANDOM_IMG_LIST'		=> $randomImageList,
		'pagetitle'				=> "{fp:About} " . $artistinfo['Fullname'],
		"email"					=> $obfuscatedEmail,
		"tel1"					=> $tel1,
		"tel2"					=> $tel2,
		"tel3"					=> $tel3,
		"website"				=> $website,
		"lightstalkers"			=> $lightstalkers,
		"agency"				=> $agency,
		"blog"					=> $blogblock,
		'message' 				=> $msg,
		'error'					=> $error,
		'master_page_popups'		=> FetchSnippet("client_access_dialog")
		));
							
	
	$output = Substitutions ($output, $artistinfo);
	$output = ReplaceAllSnippets ($output);
	$output = ReplaceSysVars ($output);
	$output = DeleteUnusedSnippets ($output);
	
	mysql_close($LINK);
	$FP_MYSQL_LINK->close();

	$output = compress_html($output);
	$DEVELOPING || $Cache_Lite->save($output, $cacheid, $cachegroup);
}	
print $output;



//----------------------------
function FetchProjectList ($ArtistID) {
	$tables	= array ('Projects' => 'Parts, Projects');
	
	$sets 	= array ("Artists"		=>	"ID, Firstname, Lastname",
					 "Projects"	=>	"Projects.ID AS ID, Parts.ProjectID, Title",
					 );
	
	$artists_where = "ID = $ArtistID";
	$projects_where = "PartID = '$ArtistID' 	AND PartTable = 'Artists' AND Projects.ID = Parts.ProjectID";
	$wheres	= array ("Artists"	=>	$artists_where,
					 "Projects"		=>	$projects_where
					 );
	
	$orders	= array ("Projects"		=>	"Title",
					 "Artists"	=>	"Lastname, Firstname"
					 );
					 
	$formats = FetchFormatSet ('artist_page');
	$skip_empty_rows = TRUE;
	$list = FetchCascade ($tables, $sets, $wheres, $orders, $formats, '', '', $skip_empty_rows);
	return $list;
}

function FetchStoryList ($ArtistID) {
	$tables	= array ('Stories' => 'Stories');
	
	$sets 	= array ("Artists"		=>	"ID, Firstname, Lastname",
					 "Stories"	=>	"ID, Title, ProjectID",
					 );
	
	$artists_where = "ID = $ArtistID";
	$Stories_where = "Stories.ArtistID = '$ArtistID'";
	$wheres	= array ("Artists"	=>	$artists_where,
					 "Stories"		=>	$Stories_where
					 );
	
	$orders	= array ("Stories"		=>	"Title",
					 "Artists"	=>	"Lastname, Firstname"
					 );
					 
	$formats = FetchFormatSet ('artist_page');
	$skip_empty_rows = TRUE;
	$list = FetchCascade ($tables, $sets, $wheres, $orders, $formats, '', '', $skip_empty_rows);
	return $list;
}

?>