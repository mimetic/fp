<?php

/*
RSS
FP GALLERY SYSTEM
Mar 31, 2009
Feb 4, 2006

rss.php?medium=pictures&action=$action&id=$id&shortname=$shortname

Params:
	$action:
		active	:	a list of items from all active projects
		featured:	a list of items from all featured projects* (DEFAULT)
		project :	a list of items from project ID = $id
		artist	:	a list of featured items for artist, where shortname=$shortname

Defaults:
	action = "featured"
	medium = "pictures"

The snippet of text, $medium_channelinfo, is inserted as channel information
e.g. for medium=itunes, we have a full channel description in XML in the Snippets folder,
call 'itunes_channelinfo'


Flags/Config:

// Return all images in a project, or just one? RSS readers will group all images by the link. If the link points to the project, not the picture in the project, you'll only see one item per project. Which isn't bad, just not all pix.
If we set the link on each picture to something unique, i.e. include #anchor to the picture itself then we see all pix.
constant: FP_RSS_ALL_IN_PROJ : true/false

// Add an HTML img in the description field.
constant: FP_RSS_IMG_IN_DESC : true/false



*/

include "_config/sysconfig.inc";
include "_config/fpconfig.inc";
include "_config/config.inc";
include "_config/rss_config.inc";
include "includes/functions.inc";
include "includes/project_management.inc";
include "includes/image_management.inc";
include "includes/commerce.inc";

error_reporting	 (E_ERROR | E_WARNING | E_PARSE);

$DEBUG = 0;


$DEBUG && print "DEBUGGING<BR>";


// set the umask, so setting permissions work properly
$old_umask = umask(0);
$LINK = StartDatabase(MYSQLDB);
Setup ();
$ADMINFILENAME = basename (__FILE__);
$old_umask = umask(0);

ConfirmSetup();
$vars = GetFormInput();
$error = "";
$msg = "";
$actions = $vars['actions'];
$action = $actions['action'];
$record = $vars['vars'];
// The medium indicates which namespace to use.
// Default for pictures so you don't have to specify it
// although, I guess, it would be the right and proper thing to do.
$medium = $record['medium'];		// What to fetch...pictures, video, whatever
$medium || $medium = "pictures";	// default is pictures

$action || $action = "featured";	// default is featured

$picturesize = $PHOTOS_GALLERY; // could be slides, photos, thumbnails...

// Do not show hidden projects!
$projects_where = "($PROJECTS.Public = 0) AND NOT ($PROJECTS.Slides <=> 1)";

$chaninfo = array(
"generator" => "Mimetic Galleries",
"ttl"		=> 15,
"copyright" => "All pictures and text are copyrighted (c) by the authors and creators. No pictures or text may be used with written consent of their creators.",
"language"	=> "en"
);

// default is all featured pictures
switch ($action) {

case "active" :
// example: rss.php?action=active
	$set = "ID";
	$where = GetActiveWhere($projects_where);
	$table = $PROJECTS;
	$order = "Title";
	$query = "SELECT $set FROM $table WHERE $where ORDER BY $order";
	$result = mysqli_query ($LINK, $query);

	if ($result) {
	$chaninfo['title'] = FP_SYSTEM_DISPLAY_NAME;
	$chaninfo['link'] = $SYSTEMURL;
	$chaninfo['description'] = "Photographs from Featured Projects";
	$rss = SetUpRSS ($chaninfo, $medium);

	while ($project = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
		$projectIDList[] = $project['ID'];
	}
	$rss = AddProjectsToRSS ($rss, $projectIDList);
	}
	$rss->serialize();
	// active projects
	break;

case "project" :
// example: rss.php?action=project&ID=x, where x is a project ID number
// get one user's pictures
	$chaninfo['title'] = FP_SYSTEM_DISPLAY_NAME;
	$chaninfo['link'] = $SYSTEMURL;
	$project = FetchProject ($record['ID']);
	$chaninfo['description'] = "Photographs from " . $project['Title'];
	$rss = SetUpRSS ($chaninfo, $medium);
	$rss = AddProjectsToRSS ($rss, array ('ID'=>$record['ID']));
	$rss->serialize();
	break;

case "artist" :
// example: rss.php?action=artist&shortname=x, where x is the ShortName of the artist
// get pictures from an artist by his LightStalkers shortname
// We want to get *featured* pictures of this artist
	if ($shortname) {
	$shortname = $record['shortname'];
	$artist = FetchArtistByShortname ($shortname);
	$artistID = $artist['ID'];

	// Get images from the artist, from the Parts table, where the
	// project is featured.
	$query = "SELECT DISTINCT $PARTS.PartID AS ImageID FROM $PARTS, $PROJECTS WHERE ($PROJECTS.Public = 0) AND $PARTS.PartTable = '$IMAGES' AND $PARTS.ProjectID = $PROJECTS.ID and $PARTS.ArtistID = " . $artistID . " AND (((TO_DAYS(NOW()) - TO_DAYS($PROJECTS.LastUpdate)) <= $PROJECTS.Lifespan) OR ($PROJECTS.Lifespan = 0))";
	$result = mysqli_query ($LINK, $query);

	//if ($result) {
	$chaninfo['title'] = FP_SYSTEM_DISPLAY_NAME;
	$chaninfo['link'] = $SYSTEMURL;
	$chaninfo['description'] = "Photographs from Featured Projects";
	$chaninfo['pubDate'] = $pubDate;
	$rss = SetUpRSS ($chaninfo, $medium);
	$rss = AddProjectsToRSS ($rss, array ('ID'=>$record['ID']));
	while ($parts = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
		$imageID = $parts['ImageID'];
		$image = FetchImage($imageID);
		$rss = AddPictureToRSS ($rss, $image);
	}
	} else {
	// not found!
	$chaninfo['title'] = FP_SYSTEM_DISPLAY_NAME;
	$chaninfo['link'] = $SYSTEMURL;
	$chaninfo['description'] = "This artist wasn't found: $shortname" ;
	$rss = SetUpRSS ($chaninfo, $medium);
	}

	$rss->serialize();
	break;
case "featured" :
default :
// featured projects
// example: rss.php?action=featured


// Get pubDate of this RSS feed, i.e. of the most recent change
	$where = GetFeaturedPreciseWhere ($projects_where);

	// Get pubDate - most recently updated project
	//$query = "SELECT UNIX_TIMESTAMP(max(Timestamp)) from $PROJECTS WHERE $where";
	$query = "SELECT UNIX_TIMESTAMP(max(LastUpdate)) from $PROJECTS WHERE $where";
	$result = mysqli_query ($LINK, $query);
	if ($result) {	
		$result->data_seek($row); 
		$datarow = $result->fetch_array(); 
		$date = $datarow[0];
		if ($date) {
			$pubDate = date ("D, d M Y H:i:s O", $date);
		} else {
			$pubDate = "";
		}
	}

	// Get projects
	$set = "ID";
	$where = GetFeaturedPreciseWhere ($projects_where);
	$table = $PROJECTS;
	$order = "Title";
	$query = "SELECT $set FROM $table WHERE $where ORDER BY $order";
	$result = mysqli_query ($LINK, $query);

	$chaninfo['title'] = FP_SYSTEM_DISPLAY_NAME;
	$chaninfo['link'] = $SYSTEMURL;
	$chaninfo['description'] = "Photographs from Featured Projects";
	$chaninfo['pubDate'] = $pubDate;
	$rss = SetUpRSS ($chaninfo, $medium);
	if ($result) {
	while ($project = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
		$projectIDList[] = $project['ID'];
	}
	$rss = AddProjectsToRSS ($rss, $projectIDList);
	$rss->serialize();
	}
	break;
}

// ### END main section


function SetUpRSS ($chaninfo, $medium) {
	global $SYSTEMURL;
	global $LINK;
	
	// add in site channel info
	$extraChanInfo = FetchSnippet ($medium . "_channelinfo");	// this has returns, so let's make it a file
	$rss = new RSSWriter($chaninfo, $extraChanInfo, $medium);
	
	// Get icon from group 1 (main group)
	$mygroup = new FPGroup ($LINK, 1);
	$icon = $mygroup->IconFilename();
	
	($medium == "itunes") || $rss->setImage($chaninfo['title'],
		"http://{$SYSTEMURL}{$icon}",
		"http://{$SYSTEMURL}",
		$description,
		144, 22);
return $rss;
}

// -----------------------------------------------------------------------
// Add items from a project to an RSS feed
//	<item>
//		<description>This is picture 030528-DG-014.JPG</description>
//		<title>item 030528-DG-014.JPG</title>
//		<link>http://www.mimetic.com/rss/030528-DG-014.JPG</link>
//	</item>

function AddProjectsToRSS ($rss, $projectIDList) {
global $IMAGES, $PRICE, $ARTISTS, $TOPICS, $KEYWORDS, $PARTS, $THUMBNAILS, $PHOTOS_GALLERY, $SLIDES, $picturesize;
global $medium;
global $COMMENTS, $RATINGS, $SETS, $PROJECTS, $SNIPPETS, $STORIES;
global $baseURL, $SYSTEMURL;
global $msg, $error;
global $LINK;

$DEBUG = false;
if (count($projectIDList)) {
	foreach ($projectIDList as $projectID) {
	$project = FetchProject ($projectID);

	switch ($medium) {
		case "itunes"	:
		$chaninfo = array();
		$chaninfo["title"] = FP_SYSTEM_DISPLAY_NAME;
		$chaninfo["link"] = $SYSTEMURL;
		$chaninfo["description"] = $description;
		$record = GetRecord ("Projects", $ID);
		$result = mysqli_query ($LINK, $query);
		while ($part = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
			$pixlist[$part['PartID']] = FetchImage ($part['PartID']);
		}
		$pixlist && ksort ($pixlist);

		break;

		default :
		$chaninfo = array();

		$chaninfo["title"] = FP_SYSTEM_DISPLAY_NAME;
		$chaninfo["link"] = $SYSTEMURL;
		$chaninfo["description"] = $description;
		//$chaninfo['pubDate'] = date ("D, d M Y H:i:s O", $project['Timestamp']);
		$chaninfo['pubDate'] = date ("D, d M Y H:i:s O", $project['LastUpdate']);
		$pixlist = array();
		//$record = FetchProject ($projectID);

		if (FP_RSS_ALL_IN_PROJ) {
		// GET ALL ENTRIES IN THE PROJECT
			$query = "select DISTINCT * from $PARTS where ProjectID = '$projectID' and PartTable = '$IMAGES'";
		} else {
		// GET ONE ENTRY PER PROJECT
		// Random image:
		//$order = "ORDER BY rand()";
		// Most recent image:
			$order = "ORDER BY ID DESC";
			$query = "select DISTINCT * from $PARTS where ProjectID = '$projectID' and PartTable = '$IMAGES' $order LIMIT 1";
		}

		$result = mysqli_query ($LINK, $query);
		while ($part = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
			$image = FetchImage ($part['PartID']);
			$image['ProjectID'] = $projectID;
			$pixlist[$part['PartID']] = $image;
		}
		$pixlist && ksort ($pixlist);

		break;
		;
	}
	// Create the RSS feed
	if (!isset ($rss)) {
		$rss = new RSSWriter(
		$chaninfo,
		$medium
		);
	}
	foreach ($pixlist as $image) {
		$rss = AddPictureToRSS ($rss, $image, $project);
	}
	}
}
return $rss;
}

// Add an image record (an array) to the RSS feed
function AddPictureToRSS ($rss, $image, $project=array() ) {
	global $IMAGES, $PRICE, $ARTISTS, $TOPICS, $KEYWORDS, $PARTS, $THUMBNAILS, $PHOTOS_GALLERY, $SLIDES, $picturesize;
	global $COMMENTS, $RATINGS, $SETS, $PROJECTS, $SNIPPETS, $STORIES;
	global $baseURL, $SYSTEMURL;
	global $FP_COMMENTS_URL;
	global $msg, $error;
	
	$DEBUG = 0;
	
	//$row['$IMG_THUMBNAIL'] = URLtoIMG ($THUMBNAILS . "/" . $b);
	$DEBUG && print "picture ".ArrayToTable ($image);
	// ***** FIX THIS ***** TO BE GENERIC
	
	$date = $image['CreatedDate'];
	// must use RFC822 date-time format
	if ($date) {
		$pubDate = date ("D, d M Y H:i:s O", mysqldate_to_timestamp($date));
		$shortdate = date ("M j, Y", mysqldate_to_timestamp($date));
	} else {
		$pubDate = "";
		$shortdate = "";
	}
	$artist = FetchArtist($image['ArtistID']);
	
	$byline = $artist['FullLongName'];
	$description = $image['Caption'];
	$description .= " (photo by " . $byline;
	$shortdate && $description .= ", $shortdate)";
	$copyright = $image['CopyrightNotice'];
	$copyright || $copyright = "Copyright \xA9 $byline. All rights reserved.";
	$coverage = $image['Country'];
	$image['City'] && ($coverage = $image['City'].", ".$coverage);
	
	if(isset($project['Title'])) {
		$description = "<b>{$project['Title']}</b>: ". $description;
	}
	
	if (FP_RSS_IMG_IN_DESC) {
		$img = URLtoIMG ("$THUMBNAILS/{$image['URL']}", null, FP_RSS_IMG_MAX_W, FP_RSS_IMG_MAX_H,"Photo",1,"http://{$SYSTEMURL}");
		$img = "<div style=\"float:left;margin:10px;border:1px solid black;\">$img</div><br>";
		$description = $img.$description;
	}
	
	// If show only one pic, change title to indicate a project was updated
	$title = $image['Title'];
	if (!FP_RSS_ALL_IN_PROJ) {
		$title = "Exhibition Updated: {$project['Title']}";
	}
	// To show all pix in a project, the link includes the anchor to the pix in the gallery,
	// making it unique. If no link, all links which are the same are grouped into one item
	// in the RSS feed (that's how RSS works).
	$link = "http://{$SYSTEMURL}gallery.php?ProjectID={$image['ProjectID']}";
	if (FP_RSS_ALL_IN_PROJ) {
		$link .= "#gallery_picture{$image['ID']}";
	}
	
	$description = utf8_to_html($description);
	$description = preg_replace("/(\r|\n)+/", "", trim($description));
	$description = "<![CDATA[".$description."]]>";

	$item = array (
		'title'				=>	ConvertTextforRSS($title),
		'link'				=>	$link,
		'description'		=>	$description,
		//'pubDate'			=>	date ("D, d M Y H:i:s O", strtotime($project['Timestamp'])),
		'pubDate'			=>	date ("D, d M Y H:i:s O", strtotime($project['LastUpdate'])),
		//					'pubDate'			=>	$project['Timestamp'],	// make the date of the item the project last update
		'category'			=>	"NA",	// Includes the item in one or more categories., e.g. photography/nudes
		'comments'			=>	$FP_COMMENTS_URL,	// URL of a page for comments relating to the item.
		'dc:creator'			=>	$byline,
		'dc:rights'			=>	$copyright,
		'dc:coverage'		=>	$coverage,
		'photo:imgsrc'		=>	"http://{$SYSTEMURL}{$picturesize}/{$image['URL']}",	//$picturesize is gallery,slide, etc., name of a directory with pictures
		'photo:thumbnail'	=>	"http://{$SYSTEMURL}$THUMBNAILS/{$image['URL']}",
		99					=>	"<media:thumbnail url=\"http://{$SYSTEMURL}{$THUMBNAILS}/".$image["URL"] . "\" />",
		98					=>	"<media:content url=\"http://{$SYSTEMURL}{$picturesize}/".$image["URL"] . "\" />",
		//GUID is used by Safari/Mail RSS as part of the URL, so we must do that
		'guid'			=>	"http://{$SYSTEMURL}gallery.php?ProjectID={$image['ProjectID']}"
	);
	
	$DEBUG && print __LINE__.": added picture ".ArrayToTable ($item);


	/*
	// Flickr style picture -- doesn't seem to work
	list($width, $height, $type, $attr) = GetImageSize("$SLIDES/".$image['URL']);
	$content = "<content type=\"text/html\" mode=\"escaped\" >\n";
	$content .= "&lt;img src=&quot;http://{$SYSTEMURL}{$picturesize}/" . $image['URL'] . "&quot; ".htmlentities($attr)." style=&quot;border: 1px solid #ddd;&quot; /&gt;\n";
	$content .= "</content>";
	$item[] = $content;
	*/

	// Enclosure adds the picture as a clickable item, but doesn't show it in the feed itself
	// $filesize = filesize ("$SLIDES/" .  $image['URL']);
	// $enclosure = "<enclosure url=\"" . "http://{$SYSTEMURL}$THUMBNAILS/" . $image['URL'] . "\" length=\"$filesize\" type=\"image/jpeg\" />";
	// $item[] = $enclosure;
	
	$rss->addItem($item);
	return $rss;
}

// Add an image record (an array) to the RSS feed
function AddPodCastToRSS ($rss, $image) {
	global $IMAGES, $PRICE, $ARTISTS, $TOPICS, $KEYWORDS, $PARTS, $THUMBNAILS, $PHOTOS_GALLERY, $SLIDES, $picturesize;
	global $AV;
	global $COMMENTS, $RATINGS, $SETS, $PROJECTS, $SNIPPETS, $STORIES;
	global $baseURL;
	global $msg, $error;
	global $SYSTEMURL;
	
	$DEBUG = false;
	
	$date = $image['CreatedDate'];
	// must use RFC822 date-time format
	$pubDate = date ("D, d M Y", mysqldate_to_timestamp($date)) . " -0000";
	$shortdate = date ("M j, Y", mysqldate_to_timestamp($date));
	
	$artist = FetchArtist($image['ArtistID']);
	
	$byline = $artist['FullLongName'];
	$description = $image['Caption'];
	$description .= " (photo by " . $byline;
	$description .= ", $shortdate)";
	$copyright = $image['CopyrightNotice'];
	$copyright || $copyright = "Copyright \xA9 $byline. All rights reserved.";
	$coverage = $image['Country'];
	$image['City'] && ($coverage = $image['City'].", ".$coverage);
	$duration = "HELP";
	$subtitle = $image['Subtitle'];
	$block = $image['Block'];
	
	$item = array ( 'title'				=>	$image['Title'],
		'author'			=>	$byline,
		'link'				=>	$SYSTEMURL,
		'guid'				=>	"http://{$SYSTEMURL}{$picturesize}/" . $image['URL'],
		'pubDate'			=>	$pubDate,
		'itunes:author'		=>	$byline,
		'itunes:block'		=>	$block,
		'itunes:duration'	=>	$duration,
		'itunes:explicit'	=>	$explicit,
		'itunes:keywords'	=>	$keywords,
		'itunes:subtitle'	=>	$subtitle,
		'itunes:summary'	=>	$description
	);
	// Enclosure
	$filesize = filesize ("$AV/" .	$image['URL']);
	$mimetype = mime_content_type ("$AV/" . $image['URL']);
	$enclosure = "<enclosure url=\"" . "http://{$SYSTEMURL}$AV/" . $image['URL'] . "\" length=\"$filesize\" type=\"$mimetype\" />";
	$item[] = $enclosure;


	/*
	// Flickr style picture -- doesn't seem to work
	list($width, $height, $type, $attr) = GetImageSize("$SLIDES/".$image['URL']);
	$content = "<content type=\"text/html\" mode=\"escaped\" >\n";
	$content .= "&lt;img src=&quot;http://{$SYSTEMURL}{$picturesize}/" . $image['URL'] . "&quot; ".htmlentities($attr)." style=&quot;border: 1px solid #ddd;&quot; /&gt;\n";
	$content .= "</content>";
	$item[] = $content;
	*/


$rss->addItem($item);
return $rss;
}

// FOLLOWING IS BASED ON:
// $Id: rss10.inc,v 1.3 2001/05/20 17:58:02 edmundd Exp $
// A convenience class to make it easy to write RSS classes
// Edd Dumbill <mailto:edd+rsswriter@usefulinc.com>

class RSSWriter {

function __construct ($chaninfo, $extraChanInfo, $medium) {
	$chaninfo['link'] = "http://".$chaninfo['link'];
	$this->chaninfo=$chaninfo;
	$this->extraChanInfo=$extraChanInfo;
	$this->website=$chaninfo['link'];
	$this->items=array();
	$this->medium = $medium;
	$this->channelTagList = $this->SetChannelTagList ();
	$this->channelURI=str_replace("&", "&amp;", "http://" . $_SERVER["SERVER_NAME"] . $_SERVER["REQUEST_URI"]);
}

function setImage($title, $URL, $link, $description, $w, $h) {
	$this->image = array(	"URL"	=> $URL,
	"title" => $title,
	"link"	=> $link,
	"description"	=>	$description,
	"width" =>	$w,
	"height"	=>	$h
	);
}

function addItem($item = array('title'=>'missing info')) {
	$this->items[]=$item;
}

// $medium is the namespace, e.g. for pictures, iTunes, whatever
function serialize() {
	$DEBUG = 0;
	
	$output = "";
	$output .= $this->preamble();
	$output .= $this->channelinfo();
	$output .= $this->items();
	$output .= $this->postamble();
	// header("Content-type: text/xml");
	header("Content-type: application/rss+xml");
	// fix copyright symbols
	$output = str_replace  ("\xFFFD", "&#xA9;", $output);
	$output = str_replace  ("ï¿½", "&#xA9;", $output);
	$output = str_replace ("Ã", "", $output);	// common problem
	$output = str_replace ("\xC2", "", $output);	// common problem
	$output = str_replace ("\xA9", "&#xA9;", $output);	// (c) to utf-8
	$output = str_replace ("/\&copy;/", "&#xA9;", $output);

	print utf8_encode ($output);
}

function deTag($in) {
	while(preg_match('/<[^>]+>/', $in)) {
	$in=preg_replace('/<[^>]+>/', '', $in);
	}
	return $in;
}

function preamble() {
	$output = "";
	$output .=	"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?" . ">\n";
	switch ($this->medium) {
	case "itunes" :
		$output .= '<rss xmlns:itunes="http://www.itunes.com/dtds/podcast-1.0.dtd" version="2.0">' . "\n";
		break;
	default :
		$output .= '<rss version="2.0" xmlns:atom="http://www.w3.org/2005/Atom" xmlns:photo="http://www.pheed.com/pheed/" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:media="http://search.yahoo.com/mrss/">' . "\n";
		// Flickr style
		// $output .= '<feed version="0.3" xmlns="http://purl.org/atom/ns#" xmlns:dc="http://purl.org/dc/elements/1.1/" >' . "\n";
		break;
	}
	return $output;
}

function channelinfo() {
	$output = "";
	$output .= "\t<channel>\n";
	foreach ($this->channelTagList as $f) {
		if (isset($this->chaninfo[$f])) {
			//$output .=	"\t<${f}>" . htmlspecialchars($this->chaninfo[$f], ENT_QUOTES) . "</${f}>\n";
			//$output .=	"\t<${f}>" . $this->chaninfo[$f] . "</${f}>\n";
			$output .=	"\t<${f}>" . ConvertTextforRSS($this->chaninfo[$f]) . "</${f}>\n";
		}
	}
	if (isset($this->chaninfo['image'])) {
		$this->image = $this->chaninfo['image'];
		$output .=	$this->image();
	}
	$output .= "\t\t<atom:link href=\"http://". $_SERVER['SERVER_NAME']. $_SERVER['REQUEST_URI']."\" rel=\"self\" type=\"application/rss+xml\" />\n";
	//var_dump ($_SERVER);
	$output .= $this->extraChanInfo;
	return $output;
}

function image() {
	$output = "";
	print __FUNCTION__.__LINE__."hello";
	if (isset($this->image)) {
	$output .=	"\t<image>\n";
	$output .=	"\t\t<title>" . htmlspecialchars($this->image["title"], ENT_QUOTES) . "</title>\n";
	$output .=	"\t\t<url>" . htmlspecialchars($this->image["URL"]) . "</url>\n";
	$output .=	"\t\t<link>" . htmlspecialchars($this->image["link"]) . "</link>\n";
	$output .=	"\t\t<media:thumbnail url=\"" . htmlspecialchars($this->image["URL"]) . "\" />\n";
	$output .=	"\t\t<media:content url=\"" . htmlspecialchars($this->image["link"]) . "\" />\n";
	//		isset($this->image["description"]) && $output .=  "\t\t<description>" . htmlspecialchars($this->image["description"], ENT_QUOTES) . "</description>\n";
	isset($this->image["description"]) && $output .=  "\t\t<description>" . "<![CDATA[". $this->image["description"] . "]]>"."</description>\n";
	isset($this->image["height"]) && $output .=	 "\t\t<height>" . htmlspecialchars($this->image["height"]) . "</height>\n";
	isset($this->image["width"]) && $output .=	"\t\t<width>" . htmlspecialchars($this->image["width"]) . "</width>\n";
	$output .=	"\t</image>\n\n";
	}
	return $output;
}

function postamble() {
	$output = "";
	$output .=	"\t</channel>\n";
	$output .=	"</rss>\n";
	return $output;
}


// If there's no key value, then just use the value (inside an item array)
// That way, we can have single tag values, like enclosure
function items() {
	$output = "";
	foreach ($this->items as $item) {
	$output .=	"\t<item>\n";
	foreach ($item as $key => $value) {
		if (!is_numeric($key)) {
			if (is_array($value)) {
				foreach ($value as $v1) {
					$output .=	"\t\t<${key}>" . htmlspecialchars($v1, ENT_NOQUOTES) . "</${key}>\n";
				}
			} else {
				//$output .=	"\t\t<${key}>" . htmlspecialchars($value, ENT_NOQUOTES) . "</${key}>\n";
				$output .=	"\t\t<${key}>$value</${key}>\n";
			}
		} else {
		$output .= "\t\t$value\n";
		}
	}
	$output .=	"\t</item>\n\n";
	}
	return $output;
}

function SetChannelTagList () {
	switch ($this->medium) {
		case "itunes" :
			$channeltags = array(
			"title",
			"link",
			"copyright",
			"description",
			"language",
			"publisher",
			"creator",
			"rights"
			);
		default :
			$channeltags = array(
			"title",
			"link",
			"copyright",
			"description",
			"language",
			"publisher",
			"creator",
			"rights",
			"pubDate",
			"itunes:author",
			"itunes:block",
			"itunes:category",
			"itunes:image",
			"itunes:explicit",
			"itunes:keywords",
			"itunes:owner",
			"itunes:subtitle",
			"itunes:summary"
			);
			break;
		}
		return $channeltags;
	}
}

function ConvertTextforRSS ($text, $cdata=false) {
	$v = htmlentities($text);
	if ($v != $text) {
		//$text = utf8_encode(htmlentities($text, ENT_NOQUOTES, 'utf-8'));
		$text = htmlentities($text, ENT_NOQUOTES, 'utf-8');
		$text = xml_character_encode($text);
		$cdata && $text = '<![CDATA['.$text.']]>';
	}
	return $text;
}

function xml_character_encode($string, $trans='') { 
	$trans = (is_array($trans)) ? $trans : get_html_translation_table(HTML_ENTITIES, ENT_QUOTES); 
	foreach ($trans as $k=>$v) 
		$trans[$k]= "&#".ord($k).";"; 
	return strtr($string, $trans);
} 


?>
