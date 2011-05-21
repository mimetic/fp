<?php

/*
Newsletter generator

	Params:
	- send=1: write the newsletter as a file to the sending queue which a mailer will pick up and send
	- save=1: if it has a value, write the newsletter to a file in the same directory as the script
	- show=1: display the HTML but don't send it. Used for direct display

Examples:
	http://www.mygallery.com/newsletter.php?show=1
	http://www.mygallery.com/newsletter.php?send=1
	http://www.mygallery.com/newsletter.php?save=1

Build a "newsletter" from a template and uploaded pictures and stories
- use statement/description/title of recently updated projects with photos

Load page template
Load project template
Load image template

For each project, say it was recently updated. Give statement and desc. of project, and short bio info of the photogs.
Then, show new photos that were uploaded, with captions that explain them.
For each picture, tell whether it's for sale, how much, and edition info.

Perhaps, a series of clever phrase with blanks we can fill in, to say, what a great X we have today, with Y and Z!

Save the whole mess as an HTML file with art in a folder.

*/

include "_config/sysconfig.inc";
include "_config/fpconfig.inc";
include "_config/config.inc";
include "includes/functions.inc";
include "includes/project_management.inc";
include "includes/image_management.inc";
include "includes/commerce.inc";

//error_reporting(E_ALL);

$error = "";
$msg = "";

session_name("fp_gallery_session");
session_start();

isset($_REQUEST['GroupID']) && $_SESSION['GroupID'] = $_REQUEST['GroupID'];
isset($_SESSION['GroupID']) || $_SESSION['GroupID'] = PUBLIC_GROUP_ID;

isset($_REQUEST['theme']) && $_SESSION['theme'] = $_REQUEST['theme'];
isset($_SESSION['theme']) || $_SESSION['theme'] = DEFAULT_THEME;

$LINK = StartDatabase(MYSQLDB);
Setup ();

$groupID = $_SESSION['GroupID'];
$myGroup = new FPGroup ($groupID);

$GroupBannerURL = $myGroup->LogoFilename();
$GroupBanner = $myGroup->LogoHTML("style='border:1px solid black;margin-right:10px;'");
$GroupIcon = $myGroup->IconHTML();

$myTheme = CurrentThemeID ();

$tables	= array (
	$PROJECTS => "$PROJECTS, $PARTS", 
	$IMAGES	=>	"$PARTS, $IMAGES, $ARTISTS"
	);
// How shall we choose pix and projects?
// new_projects = projects with images uploaded since last newsletter
// new_images = projects with images *created* since last newsletter (not just uploaded!)

isset($criterion) || $criterion = "new_projects";

switch ($criterion) {
	case "new_images" :
		// Get all images CREATED after the last publication
		// That's (today - the period of publication)
		$images_where = "$PARTS.ProjectID = '{".$PROJECTS."_ID}' AND $PARTS.PartTable = '$IMAGES' AND $PARTS.PartID = $IMAGES.ID AND $ARTISTS.ID = $IMAGES.ArtistID and DATE_SUB(CURDATE(),INTERVAL ".FP_NEWSLETTER_PERIOD." DAY) <= CreatedDate";
		break;
	default : // new_projects
		// Get all images UPLOADED after the last publication
		// That's (today - the period of publication)
		$images_where = "$PARTS.ProjectID = '{{$PROJECTS}_ID}' AND $PARTS.PartTable = '$IMAGES' AND $PARTS.PartID = $IMAGES.ID AND $ARTISTS.ID = $IMAGES.ArtistID and DATE_SUB(CURDATE(),INTERVAL ".FP_NEWSLETTER_PERIOD." DAY) <= $IMAGES.Timestamp";
		break;
}

$d = "DATE_FORMAT($PROJECTS.ProjectDate, '%M %D, %Y') as ProjectDate";
$d .= ", if (length($PROJECTS.Description) < 150, concat($PROJECTS.Description, if ($PROJECTS.Description != '', '<BR>', '')), concat(substring($PROJECTS.Description,1,150),'...<BR>')) AS Lead, ";
$d .= " if (concat(City, Country) = '', '{fp:somewhere}', CONCAT_WS(', ', if(City = '',NULL, City), Country)) AS Place ";
$sets 	= array (	$PROJECTS	=>		ProjectsCalcFields("$PROJECTS.*, $d"),
								$IMAGES		=>		"$PARTS.ProjectID, $PARTS.PartID, $PARTS.OrderinProject as ImagePosition, $ARTISTS.ID AS ArtistID, Lastname, Firstname, CONCAT_WS(' ', Firstname, Lastname) AS Fullname, $IMAGES.ID AS ImageID, Title, URL, FrameID, RollID, Caption"
							);

$projects_where = "NOT ($PROJECTS.Slides <=> 1) AND $PROJECTS.ArtistID != " . FP_ADMINISTRATOR . " AND ";
$projects_where .= "$PROJECTS.GroupID = $groupID AND $PROJECTS.ID = $PARTS.ProjectID AND $PARTS.PartTable = '$IMAGES'";
$projects_where = GetFeaturedWhere ($projects_where);
$projects_where .= " GROUP BY $PROJECTS.ID";

$artistPart_where = "$PARTS.ProjectID = '{".$PROJECTS."_ID}' AND $PARTS.PartTable = 'Images' AND $PARTS.ArtistID = Artists.ID";

$wheres	= array (	$PROJECTS	=>	$projects_where,
				 	$IMAGES		=>	$images_where
				 );

$orders	= array (	$PROJECTS		=>	"$PROJECTS.Title",
				 	$IMAGES			=>	"ImagePosition"
				 );


$limits	= array (	$PROJECTS		=>	 0,
				 	$IMAGES			=>	 4
				 );

$rowlengths	= array (	$PROJECTS	=>	 0,
				 		$IMAGES		=>	 4
				 );

if ($myGroup->IsSolo(true)) {
	$formats = FetchFormatSet ('newsletter_page_solo');
} else {				 
	$formats = FetchFormatSet ('newsletter_page');
}

$list = FetchCascade ($tables, $sets, $wheres, $orders, $formats, $limits, '', true, null, $rowlengths);	// true = skip empty rows, null = variation

if ($list) {	

	$output = FetchSnippet ('newsletter_page');
	
	// NOTE: we embed the CSS in the file because it will be sent as HTML. We don't want it
	// to have to reach onto the 'net to get the CSS with a reference.
	// Get the CSS path/name
	if (file_exists (FP_THEMES_DIR."/".$myTheme."/".FP_CSSDIR."/".FP_THEME_NEWSLETTER_CSS_NAME)) {
		$csspath = FP_THEMES_DIR."/".$myTheme."/".FP_CSSDIR."/".FP_THEME_NEWSLETTER_CSS_NAME;
	} else {
		$csspath = FP_THEMES_DIR."/".DEFAULT_THEME."/".FP_CSSDIR."/".FP_THEME_NEWSLETTER_CSS_NAME;
	}

	if ($_REQUEST['show']) {
		$csslink = "<link rel=\"Stylesheet\" rev=\"Stylesheet\" href=\"$csspath\"  type=\"text/css\">\n";
		$css = "";
	} else {
		$csslink = "";
		$css = file_get_contents ($csspath);
	}
	
	// navbar
	$f = array ();
	//$f["date"] = date('l dS \of F Y');
	$f["date"] = date('F dS, Y');
	$navbar = Substitutions (FetchSnippet ("newsletter_navbar"), $f);
	
	// footer
	$footer = FetchSnippet ("newsletter_footer");
	
	// If there's a snippet for the picture behind, then use it, else no format for random image
	$ri = FetchSnippet("picturebehind");
	$ri || $ri = "{Images_URL}";
	$randomimage = FetchRandomImage ("", $ri,null,null,$groupID);
	$randomImageList = FetchRandomImageList($groupID);

	$showBkgdImage = FetchSnippet ("show_bkgd_image");
	$showBkgdImage 
		? $bkgd = FetchSnippet ("background_img_style")
		: $bkgd = "";

	// Some things change depending on the gallery installation type
	if (FP_INSTALLATION_TYPE ==  FP_INSTALLATION_PRIVATE) {
		$title = FP_SYSTEM_DISPLAY_NAME;
		$subtitle = "{FP:newsletter_private_subtitle}";
	} else {
		$title = $myGroup->title;
		$subtitle = "{FP:newsletter_public_subtitle}";
	}


	$output = Substitutions (
		$output, array(
		'META_INDEX'			=> FetchSnippet ('meta_robots_noindex'),
		'NAVBAR'				=> $navbar,
		'NAVBAR_2'			=> $navbar2,
		'GALLERY_STYLESHEET'	=>		"",
		'header'				=> $header,
		'BACKGROUND_IMG_STYLE'	=> $bkgd,
		'list'				=> $list, 
		'grouptitle'			=> "",
		'GROUPICON'			=> $GroupIcon,
		'GROUPBANNER'		=> $GroupBanner,
		'GROUPBANNERURL'		=> $GroupBannerURL,
		'sectionclass'		=> "newsletter",
		'title'				=> $title,
		'subtitle'			=> $subtitle,
		'pagetitle'			=> "{fp:newsletter_page_title}",
		'footer'					=> $footer,
		'RANDOM_IMG'			=> $randomimage,
		'RANDOM_IMG_LIST'		=> $randomImageList,
// Mailer vars
		'MAILER_AUTHOR'		=> FP_MAILER_AUTHOR,
		'MAILER_SUBJECT'		=> FP_MAILER_SUBJECT,
		'MAILER_LIST'			=> FP_MAILER_LIST,
		'CSSLINK'			=> $csslink,

		'message'				=> $msg,
		'error'					=> $error
	));
	
	
	$output = ReplaceAllSnippets ($output);
	$output = ReplaceSysVars ($output);
	// Why do this here? Does something else load above?
	$output = Substitutions ($output, array( 'css'	=> $css ));
	$output = DeleteUnusedSnippets ($output);
	
//	$output = compress_html ($output);
	$filename = "newsletter-" .date("Y-m-d") . ".html";
	switch ($_REQUEST['action']) {
		case 'save' :
			$path = FP_DIR_NEWSLETTER . "/$filename";
			file_put_contents ($path, $output);
			break;
		case 'send' :
			AddNewMailingToQueue ( FP_MAILER_SUBJECT, $output, FP_MAILER_LIST ) ;
			break;
		case 'rss' :
			$myrss = new NewsletterRSS (FP_DIR_NEWSLETTER);
			$myrss->message = $output;
			print $myrss->GetFeed();
			break;
		default :
			print $output;
			break;
	}
} else {
	print "FP Newsletter Generator : there were no pictures new enough to create the newsletter. Remember, the newsletter looks for pictures created since the last issue. If none are found, the newsletter is not created.<hr>Errors:<br>$error<hr>Messages:<BR>$msg\n";
}


mysql_close($LINK);
$FP_MYSQL_LINK->close();
	
?>