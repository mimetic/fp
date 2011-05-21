<?php

/*
slideshowxml.php

Create XML for PostcardViewer flash 
FRONTLINE-PHOTOS album SYSTEM
version Apr 19, 2006

*** 
It appears that Flash only reads the first parameter after the URL, i.e.
	slideshowxml.php?medium=pictures&action=$action&id=$id&shortname=$shortname
only passes medium=pictures. The rest is lost. So, if I code my own params, split with
a char, we can get it all in.
***

Example:
	http://localhost/fp/slideshowxml.php?params=medium-pictures,action-project,id-13,maxImageWidth-480,maxImageHeight-480,textColor-0xFFFFFF,frameColor-0xffffff,frameWidth-20,stagePadding-40,columns-3,navPosition-left,enableRightClickOpen-true,backgroundImagePath-

notes:
	$action =
		active	:	a list of items from all active projects
		featured:	a list of items from all featured projects
		project	:	a list of items from project ID = $id
		artist	:	a list of featured items for artist, where shortname=$shortname

		groupID	:	restrict to items with this groupID if set

		matted	:	if set with anything, will show matted versions of the pix.


NOTE TO MYSELF: 
$first ? $second : $third
*/

include "_config/sysconfig.inc";
include "_config/fpconfig.inc";
include "_config/config.inc";
include "includes/functions.inc";
include "includes/project_management.inc";
include "includes/image_management.inc";
include "includes/commerce.inc";

error_reporting  (E_ERROR | E_WARNING | E_PARSE); 

$DEBUG = 0;

if ($DEBUG)
	print "DEBUG IS ON<HR>";

// globals
$fp = $_SERVER['HTTP_HOST'] . rtrim( dirname ($_SERVER['REQUEST_URI']), "/") ;

// set the umask, so setting permissions work properly
$old_umask = umask(0); 
$LINK = StartDatabase(MYSQLDB);
Setup ();
$ADMINFILENAME = basename (__FILE__);
$old_umask = umask(0); 

ConfirmSetup();

// *** FOR REASONS I DON'T UNDERSTAND, only the first GET parameter arrives 
// Therefore, we have to code the params into one parameter, and split them
// here. Crappy, but that's how it goes.

$params = DecodeParamsForFlash ($_REQUEST['params']);
count($params) || $params = $_REQUEST;

//fp_error_log(ArrayToTable ($params), 3, $LOGS . "/sv.html");
$DEBUG && print_r ($params);

// The medium indicates which namespace to use.
// Default for pictures so you don't have to specify it
// although, I guess, it would be the right and proper thing to do.
$medium = $params['medium'];		// What to fetch...pictures, video, whatever
$medium || $medium = "pictures";	// default is pictures
//$matted = $params['matted'];		// Whether to use matted or normal pictures
$SlidePath = $SLIDES;		// notice NO "/" after the path name

// Override session setting if groupID parameter is set
$restrictToGroup = isset ($params['groupID']);
$restrictToGroup && $groupID = $params['groupID'];

// Fix this: abs paths use HTTP
$SlideShowAbsPaths = true;
if ($SlideShowAbsPaths ) {
	$imagePath = "";
	$thumbPath = "";
} else {
	$imagePath = "$SLIDES/";
	$SlidePath = $imagePath;
	$thumbPath = "$THUMBNAILS/";
	
	$imagePath = "http://$fp/$imagePath";
	$thumbPath = "http://$fp/$thumbPath" ;

}

$params['imagePath'] = $imagePath;
$params['thumbPath'] = $thumbPath;

$action = $params['action'];

$DEBUG && print"<h1>Action=$action</h1>\n";
// default is all featured pictures
switch ($action) {
		
	case "active" :
		// example: XML.php?action=active
		$set = "ID";
		$where = "";
		$restrictToGroup && $where .= "$PROJECTS.GroupID = $groupID ";
		$where = GetActiveWhere($where);
		$table = $PROJECTS;
		$order = "RAND()";
		$query = "SELECT $set FROM $table WHERE $where ORDER BY $order";
		
		$DEBUG && print "Query: $query<HR>";
		$result = mysql_query ($query);

		if ($result) {
			$XML = new SlideshowXML($params);
			while ($project = mysql_fetch_array($result, MYSQL_ASSOC)) {
				$XML->addProjects ( $project['ID'] );
			}
			$XML->serialize();
		}
		break;
		
	case "project" :
		// example: XML.php?action=project&ID=x, where x is a project ID number
		// get one user's pictures
		$project = FetchProject ($params['id']);
		$XML = new SlideshowXML($params);
		$XML->addProjects ( $params['id'] );
		$XML->serialize();

		break;

	case "artist" :
		// example: XML.php?action=artist&shortname=x, where x is the ShortName of the artist
		// get pictures from an artist by his LightStalkers shortname
		// We want to get *featured* pictures of this artist
		// IF no match for the shortname, OR no pictures, we drop through to default, below (featured pix)
		$shortname = $params['shortname'];
		if ($shortname) {
			$shortname = $params['shortname'];
			$artist = FetchArtistByShortname ($shortname);
			$artistID = $artist['ID'];
			// get featured projects where the artist is part of the project
			// Get images from the artist, from the Parts table, where the
			// project is featured.
			$where = "$PARTS.ArtistID = '$artistID' AND $PARTS.PartTable = 'Images'";
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
				$XML = new SlideshowXML($params);
				$albumID = "My Album";
				$XML->addAlbum ($albumID, $params, $project, $images);
				while ($parts = mysql_fetch_array($result, MYSQL_ASSOC)) {
					$imageID = $parts['ImageID'];
					$DEBUG && print "-> $imageID<BR>";
					$image = FetchImage($imageID);
					$XML->addImageToAlbum ($albumID, $image);
				}
				$XML->serialize();
				break;
			}
		}
	default :
		// default is featured projects
		// example: XML.php?action=featured
		$set = "ID";
		$restrictToGroup && $where .= "$PROJECTS.GroupID = $groupID ";
		$where = GetActiveWhere($where);
		$table = $PROJECTS;
		$order = "Title";
		$query = "SELECT $set FROM $table WHERE $where ORDER BY $order";
		$result = mysql_query ($query);

		$albumInfo['title'] = "Frontline Photos";
		$albumInfo['link'] = "http://www.frontline-photos.com/";
		$albumInfo['description'] = "Featured Projects";
		$XML = new SlideshowXML($params);	
		if ($result) {
			while ($project = mysql_fetch_array($result, MYSQL_ASSOC)) {
				$projectIDList[] = $project['ID'];
				$XML->addProjects ( $project['ID'] );
			}
			$XML->serialize();
		}
		break;
}

// ### END main section


// -----------------------------------------------------------------------
// Add items from a project to an XML feed
//	<item>
//		<description>This is picture 030528-DG-014.JPG</description>
//		<title>item 030528-DG-014.JPG</title>
//		<link>http://www.mimetic.com/XML/030528-DG-014.JPG</link>
//	</item>



function buildImage ($image) {
	global $IMAGES, $PRICE, $ARTISTS, $TOPICS, $KEYWORDS, $PARTS, $THUMBNAILS, $SLIDES;
	global $COMMENTS, $RATINGS, $SETS, $PROJECTS, $SNIPPETS, $STORIES;
	global $baseURL;
	global $msg, $error;
	global $fp, $imagePath, $thumbPath;
	global $SlidePath;

	// just in case, check URL is set
	if ( $image['URL'] ) {
		$createdate = $image['CreatedDate'];

		$caption = unicode_to_entities(utf8_to_unicode ($image['Caption']));
		$title = unicode_to_entities(utf8_to_unicode ($image['Title']));

		$caption .= " ({fp:photo by} " . $image['Byline'] . ")";
		$caption = ReplaceVocabulary ($caption);
		if (FP_CAPTION_SHOW_LINEBREAKS) {
			//$caption = preg_replace ('/\n/',"<br>\n", $caption);
			// replace the HTML entity code for "/n" ( &#10; ) with <br>
			$caption = preg_replace ("/\&\#10\;/","<br>\n", $caption);
		}

		$oneimage = array (
			"filename"		=>	$image['URL'],
			"caption"	=>	$caption,
			"title"		=>	$title,
			"link"	=>	"http://$fp/$SlidePath/" . $image['URL']
		);
	} else {
		$oneimage = array ();
	}
	return $oneimage;
}


// FOLLOWING IS BASED ON:
// $Id: XML10.inc,v 1.3 2001/05/20 17:58:02 edmundd Exp $
// A convenience class to make it easy to write XML classes
// Edd Dumbill <mailto:edd+SlideshowXML@usefulinc.com>

class SlideshowXML {
	
	var $params;

	function SlideshowXML($params) {
		$this->params = $params;
	}
	
		
	function addAlbum ($albumID, $params, $project, $images = array ()) {

		$album = array ();
		$album['cellDimension'] =$this->params['cellDimension'];
		$album['columns'] =$this->params['columns'];
		$album['zoomOutPerc'] =$this->params['zoomOutPerc'];
		$album['zoomInPerc'] =$this->params['zoomInPerc'];
		$album['frameWidth'] =$this->params['frameWidth'];
		$album['frameColor'] =$this->params['frameColor'];
		$album['textColor'] =$this->params['textColor'];
		$album['enableRightClickOpen'] =$this->params['enableRightClickOpen'];
 		$album['images'] = $images;
// 		$album['backgroundImagePath'] =$this->params['backgroundImagePath'];
// 		$album['imagePath'] = $project['imagePath'];
//		$album['thumbPath'] = $project['thumbPath'];
// 		$album['title'] = $project['Title'];
// 		$album['description'] = $project['Description'];
		
		$this->albums[$albumID] = $album;
	}
	
	function addImageToAlbum ($albumID, $image) {
		$this->albums[$albumID]["images"][] = buildImage ($image);
	}
	
	function setAlbumImages ($albumID, $images) {
		$this->albums[$albumID]["images"] = $images;
	}
	
	function addImage($image = array() ) {
		$this->images[] = $image;
	}
	
	// $medium is the namespace, e.g. for pictures, iTunes, whatever
	function serialize() {
	
		$output = "";
		$output .= $this->preamble();
		$output .= $this->outputalbums();
		$output .= $this->postamble();
		// header("Content-type: text/xml");
	//	header("Content-type: application/XML+xml");
		// fix copyright symbols
		preg_replace ("/\©/", "&#xA9", $output);
		preg_replace ("/\&copy;/", "&#xA9", $output);
		print utf8_encode ($output);
		$DEBUG = false;
			
		//global $LOGS;
		//fp_error_log($output, 3, $LOGS . "/sv.html");

	}
	
	function preamble() {
		$output = "";
		$output .=  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
		return $output;
	}
		
	function postamble() {
		$output = "";
		return $output;
	}
		
	function outputalbums() {
		global $fp, $imagePath, $thumbPath;
		global $IMAGES, $PRICE, $ARTISTS, $TOPICS, $KEYWORDS, $PARTS, $THUMBNAILS, $SLIDES;
		global $EndWidthBlackSlide, $SlideShowAbsPaths;
		global $SlidePath;
		global $DEBUG;
		
		$output = "";
		
		//randomize the order
		is_array($this->albums) || $this->albums = array();
		shuffle ($this->albums);
		
		foreach ($this->albums as $album) {
		
			$DEBUG && print "<h1>Album: Title=".$album['title']."</h1>\n";
			$output .= "\t<gallery "  
			. " cellDimension=\"" . $album['cellDimension'].'"' 
			. " captionColor=\"" . $album['textColor'].'"' 
			. " frameColor=\"" . $album['frameColor'].'"' 
			. " frameWidth=\"" . $album['frameWidth'].'"' 
			. " columns=\"" . $album['columns'].'"' 
			. " zoomOutPerc=\"" . $album['zoomOutPerc'].'"' 
			. " zoomInPerc=\"" . $album['zoomInPerc'].'"' 
			. " enableRightClickOpen = \"" . $album['true'].'"'
//			. " backgroundImagePath = \"" . $album['backgroundImagePath'] . '"' 
//			. " imagePath = \"" . $album['imagePath'].'"' 
//			. " thumbPath = \"" . $album['thumbPath'].'"'
			;

/*
<gallery cellDimension="800" columns="4" zoomOutPerc="15" zoomInPerc="100" frameWidth="20" frameColor="0xFFFFFF" captionColor="0xFFFFFF" enableRightClickOpen="true" >
*/
			if ($this->params['showTitle'] != "off" ) {
				$output .= " title=\"" . $album['title'] . '"';
			}
			$output .= ">\n";


			foreach ($album['images'] as $image) {
			
				$DEBUG && print ArrayToTable ($image);

				if ($SlideShowAbsPaths) {
					$filename = "http://$fp/$SlidePath/" . $image["filename"] ;
				} else {
					$filename = $image["filename"];
				}
			
					$output .=  "<image>\n";
					$output .=  "\t<url>" . htmlspecialchars($filename) . "</url>\n";
					if ($this->params['showCaption'] == "title") {
						$output .=  "\t<caption>" . htmlspecialchars($image["title"], ENT_QUOTES) . "</caption>\n";
					} elseif ($this->params['showCaption'] != "off")  {
						$output .=  "\t<caption>" . htmlspecialchars($image["caption"], ENT_QUOTES) . "</caption>\n";
					}
					$output .=  "</image>\n";
			}
			
			
			if ($EndWidthBlackSlide) {
				$output .= "<image>\n\t<filename>http://$fp/images/blackslide.jpg</filename>\n</image>";
			}
			
			$output .= "</simpleviewerGallery>\n";
		}			
		return $output;
	}
	
	function deTag($in) {
	  while(ereg('<[^>]+>', $in)) {
		$in=ereg_replace('<[^>]+>', '', $in);
	  }
	  return $in;
	}
	
	
	// Add all pictures in a project to the show as an album. Parameter is the ID of the project
	// You can use either an integer or an array, the function will handle it.
	function addProjects ($projectIDList) {
		global $IMAGES, $PRICE, $ARTISTS, $TOPICS, $KEYWORDS, $PARTS, $THUMBNAILS, $SLIDES;
		global $COMMENTS, $RATINGS, $SETS, $PROJECTS, $SNIPPETS, $STORIES;
		global $baseURL;
		global $msg, $error;
		global $fp, $imagePath, $thumbPath;
		global $SlidePath;
		
		$DEBUG = false;
		
		// if we send a single project ID, turn it into an array of one element
		if (! is_array($projectIDList))	
			$projectIDList = array ($projectIDList);
			
		if (count($projectIDList)) {
			foreach ($projectIDList as $projectID) {
				$project = FetchProject ($projectID);
				$project['imagePath'] = $imagePath;
				$project['thumbPath'] = $thumbPath;
				// get pictures for this project
				$pixlist = array();
				$images = array ();
				// get all images in this project
				$query = "select * from $PARTS where ProjectID = '$projectID' and PartTable = '$IMAGES' ORDER BY $PARTS.OrderInProject";
				$result = mysql_query($query);
				while ($part = mysql_fetch_array($result, MYSQL_ASSOC)) {
					$image = FetchImage ($part['PartID']);
					$images[] = buildImage ($image);
				}
				
				if ($images) {
					$this->addAlbum ($projectID, $params, $project, $images);
				}
			}
		}
	}

	
}

?>