<?php

// FOLLOWING IS BASED ON:
// $Id: XML10.inc,v 1.3 2001/05/20 17:58:02 edmundd Exp $
// A convenience class to make it easy to write XML classes
// Edd Dumbill <mailto:edd+SlideshowXML@usefulinc.com>

class SlideshowXML {
	
	/**
	 * The mysqli database connection
	 */
	var $mysqli_connection;


	function __construct($mysqli_connection) {
		$this->mysqli_connection = $mysqli_connection;
	}
	
	//FP_DEFAULT_SLIDESHOW_PAUSE
	
	function addAlbum ($albumID, $title, $description, $audio = "", $pause = 0, $images = array ()) {
		global $fp, $lgPath, $tnPath, $DEBUG;

		$DEBUG = 0;
		
		$DEBUG && print __FUNCTION__.__LINE__.": $albumID, $title, $description,<br>audio= $audio,<br>pause=$pause<HR>";

		$this->albums[$albumID] = array (
			'id'		=>	$albumID,
			'title'		=>	$title, 
			'description'	=>	$description, 
			'images'		=>	$images,
			'audio'		=>	$audio,
			'pause'		=>	$pause
		) ;
	}
	
	function addImageToAlbum ($albumID, $image) {
		$this->albums[$albumID]["images"][] = $this->buildImage ($image);
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
		return ( utf8_encode ($output) );
	}
	
	function preamble() {
		$output = "";
		$output .=  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
		$output .= "<gallery>\n";
		return $output;
	}
		
	function postamble() {
		$output = "";
		$output .=  "</gallery>\n";
		return $output;
	}
	
	
	/* 	Create XML for all albums
			We set src, thumbnail, caption, title
			Also, 
	*/
	
	function outputalbums() {
		global $fp, $lgPath, $tnPath;
		global $IMAGES, $PRICE, $ARTISTS, $TOPICS, $KEYWORDS, $PARTS, $THUMBNAILS, $SLIDES;
		global $SLIDESHOW_AUDIO;
		global $EndWidthBlackSlide, $SlideShowAbsPaths, $firstproject;
		global $SlidePath, $albumTNPath;
		global $DEBUG;
		
		$output = "";
		
		is_array($this->albums) || $this->albums = array();
		//randomize the order
		//shuffle ($this->albums);
		
		// Set the first album to $firstproject
		foreach ($this->albums as $album) {
			//$album['audio'] ? $audio = "audio=\"" . "https://$fp/" . FP_SUBDIR_SLIDESHOW_AUDIO . "/". $album['audio'] .'"'   : $audio = "";
			$album['audio'] ? $audio = "audio=\"" . $this->audiopath . $album['audio'] .'" audioCaption=""'   : $audio = "";

			// random image from album as preview
			$img = $album['images'][array_rand ($album['images'])]['src'];
			//print_r ($img);
			$preview = "tn=\"$albumTNPath". $img . "\"";
			
			
			$output .= "\t<album id=\"ssp".$album['id']."\" title=\"" . $album['title'] . "\" description=\"" . $album['description'] . "\" lgPath=\"$lgPath\" tnPath=\"$tnPath\" $audio $preview>\n";
			
			$pause = $album['pause'];
			
			foreach ($album['images'] as $image) {
				//$DEBUG && print ArrayToTable ($image);

				if ($SlideShowAbsPaths) {
					$src = "https://$fp/$SlidePath/" . $image["src"] ;
					$tn = "https://$fp/$THUMBNAILS/" . $image["src"] ;
				} else {
					$src = $image["src"];
					$tn = $image["src"];
				}
			
				$output .=  "\t\t<img";
				$output .=  " src=\"" . htmlspecialchars($src) . "\"";
				$output .=  " tn=\"" . htmlspecialchars($tn) . "\"";
				//$output .=  " link=\"" . htmlspecialchars($image["link"]) . "\"";
				$output .=  " caption=\"" . htmlspecialchars($image["caption"], ENT_QUOTES) . "\"";
				$output .=  " title=\"" . htmlspecialchars($image["title"], ENT_QUOTES) . "\"";
				isset($pause) && $output .=  " pause=\"$pause\"";
				$output .=  " />\n";
			}
			
			
			if ($EndWidthBlackSlide) {
				$output .= "\t\t<img src=\"https://$fp/images/blackslide.jpg\" tn=\"https://$fp/images/blackslidethumb.jpg\" />\n";
			}
			
			$output .= "\t</album>\n";
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
		global $msg, $error, $DEBUG;
		global $fp, $lgPath, $tnPath;
		global $SlidePath, $audioParam, $durationParam;
				
		// if we send a single project ID, turn it into an array of one element
		if (! is_array($projectIDList))	
			$projectIDList = array ($projectIDList);
			
		if (count($projectIDList)) {
			foreach ($projectIDList as $projectID) {
				$project = FetchProject ($projectID);
				// Get user data directory
				$userDir = GetUserDirectory ($project['ArtistID'],true);	// true means get relative path
				$this->audiopath = FP_HOMEDIR.$userDir.FP_SUBDIR_USER_FILES."/";

				//$DEBUG && print ArrayToTable ($project);
// 				$albumInfo = array();
// 				$albumInfo["title"] = $project['Title'];
// 				$albumInfo['link'] = "https://$fp/";
// 				$albumInfo["description"] = $description;
				
				// get pictures for this project
				$pixlist = array();
				$images = array ();
				$record = FetchProject ($projectID);
				$query = "select * from $PARTS where ProjectID = '$projectID' and PartTable = '$IMAGES' ORDER BY $PARTS.OrderInProject";
				$result = mysqli_query ($this->mysqli_connection, $query);
				while ($part = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
					$image = FetchImage ($part['PartID']);
					//$images[$image['CreatedDate']] = $this->buildImage ($image);
					$images[] = $this->buildImage ($image);
				}

				$DEBUG && print __FUNCTION__.__LINE__.": durationparam=$durationParam<BR>";
				
				// Duration param is for whole show. We must convert to pause time
				// for each slide. We must also add in the transition pause for each pic
				
				if ($durationParam) {
					$pause = $durationParam;
				} else if (count($images) && $project['SlideShowDuration']) {
					$pause = $project['SlideShowDuration'] / count ($images);
				} else {
					// use default FP setting (not the value in the SWF itself)
					$pause = FP_DEFAULT_SLIDESHOW_PAUSE + FP_SSP_TRANSITION_LENGTH;
				}				
				
				$DEBUG && print __FUNCTION__.__LINE__.": pause=$pause<BR>";

				// Subtract the transition time so the show matches 
				// the user entered show length.
				$pause > FP_SSP_TRANSITION_LENGTH
					? $pause = $pause - FP_SSP_TRANSITION_LENGTH
					: $pause = 0;		// this will give us a trivially short pause without reverting to default

				$DEBUG && print __FUNCTION__.__LINE__.": pause=$pause<BR>";

				
				//name of audio file for slideshow
				// default is to play audio
				if ($audioParam == "off") {
					$audio = "";
				} else if (!$audioParam) {
					$audio = trim($project['Audio']);
					//trim($project['Audio']) ? $audio = GetSlideshowAudioFilename ($project['ID']) : $audio = null;
				} else {
					$audio = $audioParam;
				}
				
				// We don't need the description, and it could contain chars to mess this up.
				$description = "";
				$description = strip_tags($project['Description']);
				$description = preg_replace("/(\r|\n|\t)*/s","",$description); // remove multiple new lines
				$description = unicode_to_entities(utf8_to_unicode ($description));
				$title = strip_tags($project['Title']);
				$title = unicode_to_entities(utf8_to_unicode ($title));
				if ($images) {
					$this->addAlbum ($projectID, $title, $description, $audio, $pause, $images);
				}
			}
		}
	}
	
	
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
		global $FP_CAPTION_SHOW_LINEBREAKS;
		global $baseURL;
		global $msg, $error;
		global $fp, $lgPath, $tnPath;
		global $SlidePath;
	
		// just in case, check URL is set
		if ( $image['URL'] ) {
			$createdate = $image['CreatedDate'];
			$caption = unicode_to_entities(utf8_to_unicode ($image['Caption']));
			$title = unicode_to_entities(utf8_to_unicode ($image['Title']));
	
			$caption .= " ({fp:photo by} " . $image['Byline'] . ")";
			$caption = ReplaceVocabulary ($caption);
			if ($FP_CAPTION_SHOW_LINEBREAKS) {
				//$caption = preg_replace ('/\n/',"<br>\n", $caption);
				// replace the HTML entity code for "/n" ( &#10; ) with <br>
				$caption = preg_replace ("/\&\#10\;/","<br>\n", $caption);
			}
	
			$oneimage = array (
				"src"		=>	$image['URL'],
				"caption"	=>	$caption,
				"title"		=>	$title,
				"link"		=>	"https://$fp/$SlidePath/" . $image['URL']
			);
		} else {
			$oneimage = array ();
		}
		return $oneimage;
	}

}

?>