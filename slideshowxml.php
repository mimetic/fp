<?PHP

/*
slideshowxml.php

Create XML for SlideShowPro
FP album SYSTEM
version Jan 24, 2009

*** 
It appears that Flash only reads the first parameter after the URL, i.e.
	slideshowxml.php?medium=pictures&action=$action&id=$id&shortname=$shortname
only passes medium=pictures. The rest is lost. So, if I code my own params, split with
a char, we can get it all in.
***

notes:
$action =
	active		: a list of items from all active projects
	featured		: a list of items from all featured projects (default, used if no action is set)
	project		: a list of items from project ID = $id
	artist		: a list of featured items for artist, where shortname=$shortname
	id			: a number, the id of the project to show
	GroupID		: restrict to items with this group ID if set
	albumtnsize	: album thumbnail file size = 'gallery', 'slide', or 'thumbnail' (default is 'gallery')
	size		: slide file size = 'gallery', 'slide', or 'thumbnail' (default is 'slide')

*/

$NOCACHE = 0;

// =========== BEGIN CACHING CODE ================
// Include the package
require_once('Cache/Lite.php');

// Get the id for this cache
preg_match("|^/\w+/(.+?)/|",__FILE__, $m);
$username = $m[1];
$cachegroup = "project{$username}";
$cacheid = preg_replace ("/clearcache=\w+/i", "", $_SERVER['REQUEST_URI']);
$cacheid = $cachegroup . preg_replace ("/\W/","",$cacheid);

// Force a clear of cache
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
if ($NOCACHE or !($output = $Cache_Lite->get($cacheid, $cachegroup))) {
	// =========== NO CACHE, BUILD THE PAGE ================

	include "_config/sysconfig.inc";
	include "_config/fpconfig.inc";
	include "_config/config.inc";
	include "includes/functions.inc";
	include "includes/project_management.inc";
	include "includes/image_management.inc";
	include "includes/commerce.inc";
	
	error_reporting  (E_ERROR | E_WARNING | E_PARSE); 
	
	$DEBUG = 0;
	if (isset($_REQUEST['debug'])) {
		$DEBUG = true;
	}
	
	$sspversion = "1.8x";
	
	if ($DEBUG)
		print "DEBUG IS TRUE<HR>";
	
	// globals
	$scriptdir = rtrim( dirname ($_SERVER['REQUEST_URI']), "/") ;
	$fp = $_SERVER['HTTP_HOST'] . $scriptdir ;
	// set the umask, so setting permissions work properly
	$old_umask = umask(0); 
	$LINK = StartDatabase(MYSQLDB);
	Setup ();
	$ADMINFILENAME = basename (__FILE__);
	$old_umask = umask(0); 
	
	ConfirmSetup();
	// $vars = GetFormInput();
	// $error = "";
	// $msg = "";
	// $actions = $vars['actions'];
	// $action = $actions['action'];
	// $record = $vars['vars'];
	
	// *** FOR REASONS I DON'T UNDERSTAND, only the first GET parameter arrives 
	// Therefore, we have to code the params into one parameter, and split them
	// here. Crappy, but that's how it goes.
	
	if (isset($_REQUEST['params'])) {
		$record = DecodeParamsForFlash ($_REQUEST['params']);
	} else {
		$record = $_REQUEST;
	}
	
	// The medium indicates which namespace to use.
	// Default for pictures so you don't have to specify it
	// although, I guess, it would be the right and proper thing to do.
	$medium = $record['medium'];		// What to fetch...pictures, video, whatever
	$medium || $medium = "pictures";	// default is pictures
	
	$size = $record['size'];
	$size || $size = 'slides';
	
	$albumtnsize = $record['albumtnsize'];
	$albumtnsize || $albumtnsize = 'thumbnails';
	
	$firstproject = $record['firstproject'];
	$firstproject || $firstproject = null;
	
	// notice NO "/" after the path name
	switch ($size) {
	case 'gallery' : {
		$SlidePath = FP_DIR_GALLERY_IMAGES;
		break;
		}
	case 'thumbnails' : {
		$SlidePath = FP_DIR_THUMBNAILS_IMAGES;
		break;
		}
	default : {
		$SlidePath = FP_DIR_SLIDES_IMAGES;
		}
	}
	
	// Album thumbnails come from the thumbnails directory unless we override here:
	switch ($albumtnsize) {
	case 'gallery' : {
		$albumTNPath = FP_DIR_GALLERY_IMAGES;
		break;
		}
	case 'slides' : {
		$albumTNPath = FP_DIR_SLIDES_IMAGES;
		break;
		}
	default : {
		$albumTNPath = FP_DIR_THUMBNAILS_IMAGES;
		}
	}
	
	
	
	// Override session setting if groupID parameter is set
	$restrictToGroup = isset ($record['GroupID']);
	$restrictToGroup && $groupID = $record['GroupID'];
	
	// Fix this: abs paths use HTTP
	if ($SlideShowAbsPaths ) {
		$lgPath = "";
		$tnPath = "";
	} else {
		$lgPath = "$scriptdir/$SlidePath/";
		$SlidePath = $lgPath;
		$albumTNPath = "$scriptdir/$albumTNPath/";
		$tnPath = "$scriptdir/$THUMBNAILS/";
		
	}	
	
	$action = trim($record['action']);
	// "off" means no sound, a value is used as a sound file, or no value means play project's audio (if exists)
	$audioParam = trim($record['audio']);
	// "auto" or no value means use FP system setting, otherwise use value
	$durationParam = trim($record['duration']) + 0;	// force numeric
	$DEBUG && print __FUNCTION__.__LINE__.": durationParam: {$durationParam}<BR>";
	
	$DEBUG && print "Action: $action<HR>";
	// default is all featured pictures
	switch ($action) {
			
		case "active" :
			// Active means active projects in the system.
			// example: XML.php?action=active
			$set = "ID";
			$where = "($PROJECTS.Public = 0) AND ($PROJECTS.client_list = '' )";
			$restrictToGroup && $where .= "AND ($PROJECTS.GroupID = $groupID) ";
			$where = GetActiveWhere($where);
			$table = $PROJECTS;
			$order = "$PROJECTS.Title";
			$query = "SELECT $set FROM $table WHERE $where ORDER BY $order";
			
			$DEBUG && print "Query: $query<HR>";
			$result = mysql_query ($query);
	
			if ($result) {
	// 			$albumInfo['title'] = "Frontline Photos";
	// 			$albumInfo['link'] = "http://www.frontline-photos.com/";
	// 			$albumInfo['description'] = "Photographs from Featured Projects";
				$XML = new SlideshowXML();	
				while ($project = mysql_fetch_array($result, MYSQL_ASSOC)) {
					$XML->addProjects ( $project['ID'] );
				}
				$output = $XML->serialize();
			}
			break;
			
		case "artist" :
			// example: XML.php?action=artist&shortname=x, where x is the ShortName of the artist
			// get pictures from an artist by his LightStalkers shortname
			// We want to get *featured* pictures of this artist
			// IF no match for the shortname, OR no pictures, we drop through to default, below (featured pix)
			$shortname = $record['shortname'];
			if ($shortname) {
				$shortname = $record['shortname'];
				$artist = FetchArtistByShortname ($shortname);
				$artistID = $artist['ID'];
				// get featured projects where the artist is part of the project
				// Get images from the artist, from the Parts table, where the
				// project is featured.
				$where = "($PROJECTS.Public = 0) AND $PARTS.ArtistID = '$artistID' AND $PARTS.PartTable = 'Images' AND ($PROJECTS.client_list = '' )";
				$restrictToGroup && $where .= " AND $PROJECTS.GroupID = $groupID AND $PROJECTS.ID IN (SELECT ID FROM $PROJECTS WHERE GroupID = '$groupID')";
				//$where = GetFeaturedWhere ($where);
				$where .= " AND (((TO_DAYS(NOW()) - TO_DAYS($PROJECTS.LastUpdate)) <= $PROJECTS.Lifespan) OR ($PROJECTS.Lifespan = 0))";
				
				$query = "SELECT DISTINCT $PARTS.PartID AS ImageID FROM $PARTS, $PROJECTS WHERE $where ORDER BY RAND()";			
				$DEBUG && print "Artist $shortname<BR>$query<HR>";
	
				$result = mysql_query ($query);
	
				if ($result) {	
					$artistname = $artist['Firstname'] . " " . $artist['Lastname'];
					$title = "Photographs by $artistname";
					$description = "Featured projects by $artistname";
					$XML = new SlideshowXML();
					$albumID = "My Album";
					$XML->addAlbum ($albumID, $title, $description);
					while ($parts = mysql_fetch_array($result, MYSQL_ASSOC)) {
						$imageID = $parts['ImageID'];
						$DEBUG && print "-> $imageID<BR>";
						$image = FetchImage($imageID);
						$XML->addImageToAlbum ($albumID, $image);
					}
					$output = $XML->serialize();
					break;
				}
			}
	
		case "project" :
			// example: XML.php?action=project&ID=x, where x is a project ID number
			// get one user's pictures
	// 		$albumInfo['title'] = FP_SYSTEM_DISPLAY_NAME;
	// 		$albumInfo['link'] = "http://".$SYSTEMURL."/";
	//		$project = FetchProject ($record['id']);
	//		$albumInfo['description'] = $project['Title'];
	//		$duration = $project['SlideShowDuration'];		//total length of the slide show in seconds

			$DEBUG && print "ProjectID: {$record['id']}<br>";
	
			$XML = new SlideshowXML();	
			$XML->addProjects ( $record['id'] );
			$output = $XML->serialize();
			break;
	
		default :
			// default is featured projects
			// example: XML.php?action=featured
			$DEBUG && print "GroupID: {$record['GroupID']}<br>";

			$set = "ID";
			$where = "($PROJECTS.Public = 0)  AND ($PROJECTS.client_list = '' )";
			$restrictToGroup && $where .= "AND ($PROJECTS.GroupID = $groupID) ";
			$where = GetActiveWhere($where);
			$table = $PROJECTS;
			$order = "Title";
			$query = "SELECT $set FROM $table WHERE $where ORDER BY $order";
			$result = mysql_query ($query);
	
	// 		$albumInfo['title'] = "Frontline Photos";
	// 		$albumInfo['link'] = "http://".$SYSTEMURL."/";
	// 		$albumInfo['description'] = "Featured Projects";
			$XML = new SlideshowXML();	
			if ($result) {
				while ($project = mysql_fetch_array($result, MYSQL_ASSOC)) {
					$projectIDList[] = $project['ID'];
					$XML->addProjects ( $project['ID'] );
				}
				$output = $XML->serialize();
			}
			break;
	}
	
	$output = compress_html($output);
	$DEVELOPING || $Cache_Lite->save($output, $cacheid, $cachegroup);
}	
print $output;
$DEBUG && file_put_contents ("images.xml", $output);

?>