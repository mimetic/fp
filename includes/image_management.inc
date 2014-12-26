<?php
/* 
These functions are for working with images (and multimedia and audio)
This version depends on a separate program
to resize the pictures, instead of doing.
This was because I was having problems GD on my server
and I wrote ImageMagick scripts to do the work
instead.

Of course, now that GD 2.x is working, it would be cool to be able to use either....hmmm.

*/

//-------------------------------
// Printing, matting, framing calculations

/*
	GetPrintDimensions
	Returns array: imageDims, paperDims, frameDims, imageArea, paperArea, matteArea
	Usage: 
		list($imageDims, $imageDimsOrdered, $paperDims, $frameDims, $imageArea, $paperArea, $matteArea) =  GetPrintDimensions($image);
*/	
function GetPrintDimensions($width, $height) {
	
	$orientation = ImageOrientation ($width, $height);
	
	$imageDims = array ("width"=>$width, "height"=>$height);

	$artborder = ArtBorderWidthFromSide (ImageGreaterSide($width, $height));
	$paperDims = array ("width"=>$width + (2*$artborder), "height"=>$height + (2*$artborder));
	
	$imageDimsOrdered = ImageGreaterSideFirst($width, $height);
	$frameDims = GetFrameDimensions ($width, $height);

	// Areas
	// While width/height are in inches or cm, areas are in
	// in meters or feet. So, we get the conversion factor
	// between inches and ft2, or cm and meters2
	if (UNIT == "inch") {
		$x = 144;
	} else {
		// UNIT must be CM
		$x = 1000;
	}

	$imageArea = round(($imageDims['width'] * $imageDims['height'])/$x,3);
	$paperArea = round(($paperDims['width'] * $paperDims['height'])/$x,3);
	$matteArea = round(($frameDims['width'] * $frameDims['height'])/$x,3);

	$res = array (
		"orientation"		=> $orientation,
		"imageDims"			=> $imageDims,
		"imageDimsOrdered"	=> $imageDimsOrdered,
		"paperDims"			=> $paperDims,
		"frameDims"			=> $frameDims,
		"imageArea"			=> $imageArea,
		"paperArea"			=> $paperArea,
		"matteArea"			=> $matteArea,
		"frameArea"			=> $matteArea
		);
		//var_dump($res);
	return $res;
}


// GetPrintDimensionsByID
// Given the larger side and the ID, get the smaller side.
// $size: larger side of the image measured in inches/cm (depending on system settings)
// Return list($width,$height) = GetPrintDimensionsByID ($imageID, $size);
function GetPrintDimensionsByID ($ID, $size) {
	list($width,$height) = GetImageOtherSideByID ($ID, $size);
	return GetPrintDimensions($width, $height);
}


// GetImageOtherSide
// Given an image ID and the larger side, get the other side.
// list($width,$height) = GetImageOtherSideByID($ID, $size);

function GetImageOtherSide ($width, $height, $size) {
	if (!($width && $height))
		return null;
		
	$orientation = ImageOrientation ($width, $height);
	if ($orientation == "landscape") {
		$w = $size;
		$h = $size * ($height/$width);
	} else {
		$h = $size;
		$w = $size * ($width/$height);
	}
	$w = round($w,2);
	$h = round($h,2);

	return array($w,$h);
}

// GetImageOtherSideByID
// Given an image ID and the larger side, get the other side.
// list($width,$height) = GetImageOtherSideByID($ID, $size);

function GetImageOtherSideByID ($ID, $size) {
	$imgsize = GetImageSizeByID ($ID);
	$width = $imgsize['width_print'];
	$height = $imgsize['height_print'];
	
	return GetImageOtherSide ($width, $height, $size);
}


// GetMatteDimensions
// Given an image, get matte dimensions. This is for calculating a real matte,
// larger than the image, NOT for display in a given vertical space.
// matting is based on the larger side.
// * Round to nearest 1/4 of a unit
function GetMatteDimensions ($width, $height) {
	$DEBUG = 0;
	
	// Matte based on longer side
// 	($width > $height)
// 	? $s = $width
// 	: $s = $height;
	
	// Matte based on HEIGHT
	$s = $height;
	
	$matteBorder = ($s*MATTE_BORDER_WIDTH);
	$matteBorder = round($matteBorder*4)/4;
	$matteWidthBottom = ($s*MATTEBOTTOM) + $matteBorder;
	$matteWidthBottom = round($matteWidthBottom*4)/4;

	$mattedwidth =	$width + (2 * $matteBorder);
	$mattedheight = $height + $matteBorder + $matteWidthBottom;

	if ($DEBUG) {
		print __FUNCTION__.": matteBorder=$matteBorder, ";
		print __FUNCTION__.": w=$width, h=$height";
		print __FUNCTION__.": mw=$mattedwidth, mh=$mattedheight<BR>";
	}
	return array ("width"=>$mattedwidth, "height"=>$mattedheight);
}

function GetFrameDimensions ($width, $height) {
	return GetMatteDimensions ($width, $height);
}


// AreaWithLargeUnits
// Calculate an area and convert from small units, e.g. cm or in, to large units,
//  e.g. meter or ft
function AreaWithLargeUnits ($x,$y) {


}

//------------------------ 
// CLEANUP
// Find and destroy image entries that have no owner, created by testing & bugs
// Find and destroy parts that have no owner, created by testing & bugs
function KillOrphans () {
	global $PHOTOS_GALLERY, $THUMBNAILS, $MATTED, $FRAMED, $ORIGINALS;
	global $BASEDIR;
	$DEBUG = false;
	
	$query = 'select * from Images';
	$result = mysql_query($query);
	if ($result) {
		while ($image = mysql_fetch_array($result, MYSQL_ASSOC)) {
			$url = $image['URL'];
			$ID = $image['ID'];
			if (!file_exists("$ORIGINALS/$url")) {
				print "DELETE $ID = $url<BR>";
				// delete the IMAGE
				EditTable ("delete", "Images", $ID, array());
			}
		}
	} 


	$query = 'select * from Parts';
	$result = mysql_query($query);
	if ($result) 
	{
		while ($part = mysql_fetch_array($result, MYSQL_ASSOC)) 
		{
			$ID = $part['ID'];
			$projectID = $part['ProjectID'];
			$project = FetchProject($projectID);
			// IMAGE PARTS
			if ($part['PartTable'] == "Images") 
			{
				// check if record exists in Images
				$image = FetchImage($part['PartID']);
				if (empty($image)) 
				{
					DeleteRow ("Parts", "ID = ".$ID);
					print "DELETE Part " . $part['ID'] . " => no image<BR>";
				}
				
			} elseif ($part['PartTable'] == "Artists") {
				// ARTIST PARTS (OF PROJECTS)
				// check if points to non-existant project
				$artist = FetchArtist($part['PartID']);
				if (empty($artist))
				{
					EditTable ("delete", "Parts", $ID, array());
					print "DELETE Part " . $ID . "<BR>";
				}
				$project = FetchProject ($part['ProjectID']);
				print "Part $ID => Project " . $part['ProjectID'] . "<BR>";
				if (empty($project))
				{
					EditTable ("delete", "Parts", $ID, array());
					print "DELETE Part " . $ID . " : no {fp:project}<BR>";
				}
				
			}
		}
	}
}

//------------------------

// HandleImage
// Given image info for an image, insert/delete/update it
// example) $newID = HandleImage ('insert', '', $ArtistID, $projectID, $vars, $filename);

function HandleImage ($action, $ID, $ArtistID=0, $projectID=0, $vars=array(), $filename = "") {
	global $DEPENDENCIES, $_FILES;
	global $MESSAGE;
	global $MAILED_DIR;
	global $PHOTOS_GALLERY, $THUMBNAILS, $MATTED, $FRAMED, $ORIGINALS;
	global $BASEDIR;
	global $msg, $error;

	$DEBUG = 0; 

	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": HandleImage: Starting: action=$action, ID=$ID, ArtistID=$ArtistID, ProjectID=$projectID<BR>\n<BR>\n";
	switch ($action) {
		case "insert" :
		$DEBUG = 0;
			$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": HandleImage: insert:<BR>\n";
			// no filename, don't insert!
			if ($filename) {
				$artistinfo = FetchRowsByValue ('Artists', '*', 'ID', $ArtistID, '', TRUE);
				
				// If the user enters field data, this overrides image data (obviously, it's done after the image creation)
				$vars = StripBlankFields($vars);
				$defaultvars = StripBlankFields (FetchDefaults ($artistinfo));
				//$vars = array_merge ($defaultvars, $imageinfo, $vars);	//the later value for that key will overwrite the previous one
	
				$vars['URL'] = $filename;
				$vars['ArtistID'] = $ArtistID;
				$vars['ProjectID'] = $projectID;
				
				// if no project ID, use the artist's default, which is his gallery
				$projectID || $projectID = $artistinfo['ProjectID'];

				// We don't have any $imageinfo at this point!
				//if (empty($vars['Title']))
				//	$vars['Title'] = $imageinfo['Headline'];
					
				// UNUSED. Now auto-calculated
				//$vars['Active'] = "yes";
				//$vars['Featured'] = "yes";
				
				$newID = AddRecord ("Images", $vars);
				$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": <BR><B>$newID = AddRecord (...);</B><BR>";
				$DEBUG && $msg .= ArrayToTable ($vars);
				
				// If there's a projectID for this image, make a part in the project
				// If there is NO projectID, use the owner's default project ID, i.e. his gallery
				if ($projectID) {
					AddImageToProject ($newID, $projectID);
					$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": Image $newID was added to project $projectID <i>(caption=".$vars['Caption'].")</i><BR>\n";
					//$msg .= "Image $newID was added to project $projectID<BR>\n";
				} else {
					$msg .= "The new picture was not assigned to a project.<BR>";
	//				$msg .= "Couldn't match with a project.<BR>\n";
				}
				
				$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": Finished insert of newID = $newID<BR>\n ";
				return $newID;
			} else {
				$error .= "Tried to insert a picture, but there was no filename for it? Certainly my fault!<BR>";
				return false;
			}
			break;

		case "delete" :
			DeleteImageFiles ($ID);
			// the deleting of the records and dependencies is back in EditTable
			break;
			
		case "update" :
			$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": Update image:";
			$imageinfo = array();
			$filename = "";
			if (isset($_FILES["userfile"]['name'][0]))
				$filename = $_FILES["userfile"]['name'][0];
			
			if ($filename) {
				$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": HandleImage: NEW File name : $filename <BR>\n";
				// delete the existing files if a new file has been introduced
				// then bring in new file
				$artistinfo = FetchRowsByValue ('Artists', '*', 'ID', $ArtistID, '', TRUE);
				
				$watermarktext = $artistinfo['DefaultCopyrightNotice'];

				fp_error_log("Watermark = ".htmlentities($watermarktext), 3, FP_PICTURES_LOG);
				
				$imageinfo = ProcessOneUploadedImage ($watermark);	// Process images and get IPTC data from JPEG file
				if ($imageinfo['error']) {
					$DEBUG && $error .= $imageinfo['error'];
					$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": HandleImage: Error updating image <BR>\n";
				} else {
					DeleteImageFiles ($ID);
					$vars['URL'] = $imageinfo['fp_filename'];
					$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": HandleImage: Deleted image files for image $ID and replaced them<BR>\n";
				}
			}
			
			$vars = array_merge (StripBlankFields($imageinfo), $vars);	// later value for key will overwrite previous
			$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": HandleImage: ($ID) ". ArrayToTable ($vars);
			$success = UpdateRecord ('Images', $ID, $vars);
			if (! $success)
				print "ERROR: Could not update $table<BR>\n";

			break;
	}
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": Finished Handling Images<HR>";
}

// rename all files in a directory so the extension is lower case, i.e. .JPG to .jpg
function RenameFileExtensionsToLC ($path) { 
	global $BASEDIR, $LOGS, $MAILED_DIR;
	global $msg, $error;
	
	$DEBUG = 0;
	$DEBUG && $msg .= __FUNCTION__.": Rename files in $path<BR>";

	$handle=opendir($path);
	while ($file = readdir($handle)) {
		if ((substr($file,0,1) != ".") AND (preg_match("/\.jpg$/i", $file))) { 
			$DEBUG && $msg .= __FUNCTION__.": checking $path/$file<BR>";
			$old = basename ($file);
			rename ($path.$file, $path . preg_replace ("/\.jpg$/i", ".jpg", basename($file)));
		}
	}
	closedir($handle); 
}


function DeleteDupInputImages () {	
	global $BASEDIR, $LOGS, $MAILED_DIR;
	
	$path = $MAILED_DIR;
	$handle=opendir($path);
	while ($file = readdir($handle)) {
		if ((substr($file,0,1) != ".") AND (preg_match("/\.jpg$/i", $file))) { 
			$files[] = $file;
		}
	}
	closedir($handle); 
		
	// DELETE DUPLICATE FILES, CAUSED BY
	// APPLE-DOUBLE MIME ENCODEING
	foreach ($files as $f) {
		$testf = $f;
		$testf = preg_replace ('/\.(jpe?g)$/i', '-1.$1', $testf);
		$k = array_search ($testf, $files);
		if (! ($k === FALSE) ) {
			print "Found index = [$k] ($f, ".$files[$k].")<BR>\n";
			$f = "$BASEDIR/$MAILED_DIR/" . $files[$k];
			print "Delete $BASEDIR/$MAILED_DIR/" . $files[$k]."<BR>\n";
			file_exists($f) && unlinkb ($f);
		}
	}
}

/*
 * Get a list of processed image files.
 * Look in the Processed Originals folder for files.
 */
function GetListProcessedImageFiles () {
	global $PHOTOS_GALLERY, $THUMBNAILS, $MATTED, $FRAMED, $ORIGINALS, $SLIDES;
	global $PROCESSED_PHOTOS, $PROCESSED_THUMBNAILS, $PROCESSED_MATTED, $PROCESSED_FRAMED, $PROCESSED_ORIGINALS, $PROCESSED_SLIDES;
	global $BASEDIR, $MAILED_DIR, $LOGS;
	global $msg, $error;
	
	$DEBUG = 0;
	$files = array();

	$handle=opendir("$BASEDIR/$PROCESSED_ORIGINALS/");
	
	$DEBUG && $msg .= basename(__FILE__) .": ". __FUNCTION__ .":".__LINE__ . ": Looking in " . "$BASEDIR/$PROCESSED_ORIGINALS/<BR>\n";
	foreach (glob ("$BASEDIR/$PROCESSED_ORIGINALS/*.jpg") as $file) {
		$file = basename ($file);
		$DEBUG && $msg .= "Found $file<BR>";
		// If file is locked, then processing has not finished, so skip it
		if (is_processing_image (basename ($file))) {
			fp_error_log(__FUNCTION__." Skip ".basename($file)."... it is still being processed.", 3, FP_PICTURES_LOG);
			continue;
		}
		//if ((substr($file,1,1) != ".") AND (preg_match("/\.jpg$/i", $file))) { 
		$files[] = $file;
		$DEBUG && fp_error_log(__FUNCTION__.__LINE__.":  ".basename($file)." has been processed.", 3, FP_PICTURES_LOG);
		$DEBUG &&	 $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": Found $file<BR>\n";
		//}
	}
	closedir($handle);
	return ($files);
}


//-------------------------------
// FindOneImageAssignment
// Try to assign an image to a project and artist.
// $file : full file path names, e.g. /home/x/y/z/file.jpg
function FindOneImageAssignment ($filename) {
	global $PHOTOS_GALLERY, $THUMBNAILS, $MATTED, $FRAMED, $ORIGINALS, $SLIDES;
	global $PROCESSED_PHOTOS, $PROCESSED_THUMBNAILS, $PROCESSED_MATTED, $PROCESSED_FRAMED, $PROCESSED_ORIGINALS, $PROCESSED_SLIDES;
	global $PIXTOPROCESS;
	global $FP_GALLERY_TYPE;
	global $BASEDIR, $MAILED_DIR, $LOGS;
	global $msg, $error;
	
	$DEBUG = 0;
	$VERBOSE = 1;

fp_error_log("\n--------------------------------", 3, FP_PICTURES_LOG);

	
	// If there's an info file with the picture, get the data from it. It might have the user ID.
	$infofile = str_ireplace(".jpg", ".txt", $filename);
	$infoFromFile = array();
	if (file_exists($infofile))
	    $infoFromFile = unserialize(file_get_contents($infofile));

	// If the picture was uploaded, then the session variables will be set and we
	// can get the artist ID from that, in case the user didn't caption properly!
	
	$fp_user = null;
	//isset ($_SESSION['fp_user']) ? $fp_user = $_SESSION['fp_user'] : $fp_user = NULL;
	$fp_user == FP_ADMINISTRATOR && $fp_user = null;

	// In a single-user system, artist ID is the main user. 
	if (FP_INSTALLATION_TYPE == FP_INSTALLATION_PRIVATE && ($FP_GALLERY_TYPE == FP_MULTI_GALLERY_SINGLE_USER || $FP_GALLERY_TYPE == FP_SINGLE_GALLERY_SINGLE_USER)) {
		$fp_user = GetMainUserID();
		$DEBUG && fp_error_log(__FUNCTION__.__LINE__.":  This is a single user system: Artist ID must be #{$fp_user}", 3, FP_PICTURES_LOG);
	}
	
	$DEBUG && $msg .= __FUNCTION__.__LINE__.": Artist is $fp_user<BR>";
	
	// Get project ID from folder, if possible
	$projectID = trim(array_pop ( explode ("/", dirname($filename)) ));
	is_numeric ($projectID) || $projectID = null;
	$DEBUG &&  $msg .= __FUNCTION__.__LINE__.":  Picture stored in project folder? Found ID='$projectID'<br>";

	$DEBUG &&  $msg .= __FUNCTION__.__LINE__.":  Trying to find artist/project for file: $filename<br>";

	$fileinfo = FetchIPTCInfo ($filename);		
	$projectID != null ? $project = FetchProject ($projectID) : $project = array() ;
	
	// The projectID might not be enough if we have more than one artist in a project
	// so we try to identify the artist, first.

	// If $fp_user set, then attach to the current user.
	$fp_user && $artistID = $fp_user;

// The above methods don't work,
// So let the following methods do the work:
// Check metadata, or use the owner of the gallery we're uploading to.
$artistID = false;


	// Otherwise, figure out the artist by checking the author/byline/credit/source for a known user name
	if ($artistID) {
		$user = new FPUser ($artistID);
		$artistinfo = $user->userData;
		$DEBUG && $msg .= "Artist for this new image is ID $artistID, Firstname is " . $artistinfo['FirstName'] . "<BR>";
	} else {
		$artistinfo = FindArtistByName ($fileinfo['Author']);
		if (empty($artistinfo))
			$artistinfo = FindArtistByName ($fileinfo['Byline']);
		if (empty($artistinfo))
			$artistinfo = FindArtistByName ($fileinfo['Credit']);
		if (empty($artistinfo))
			$artistinfo = FindArtistByName ($fileinfo['Source']);
		
		// If we don't have an artist ID from the IPTC, try the info file
		if (!$artistinfo['ID'] && $infoFromFile['userID']) {
		    $artistinfo = FetchArtist($infoFromFile['userID']);
		}

		// use the project's owner as the artist, if all else fails
		if (empty($artistinfo) && $project) {
			$artistinfo = FetchArtist($project['ArtistID']);
		}
		$user = new FPUser ($artistinfo['ID']);
		$artistID = $artistinfo['ID'];
	}
	
	$artistfullname = $user->Fullname;

	// Get ProjectID
	// If image was in a folder named with a projectID, we'll use that. 
	// If we don't have the project already, try to find it
	// If we found the artist, try to find the project
	if (!$projectID && $artistID) {
		//-------- attach image to a project ---------
		// Figure out the Project for this image
		// Check 'JobID' for the project name or id; if that fails, use
		// 'SpecialInstructions' for the project name or id.
		// if that fails, try to match the name to a catagory or keyword
		// if the project is given by ID number, check it and use it.

		$DEBUG && $msg .= __FUNCTION__ .":".__LINE__ .	": Look for the project ID...<BR>\n";
		
		// -- Look for the project
		
		// Try to match job id with a nickname
		$jobID = trim($fileinfo['JobID']);
		$projectID = FindProjectByName ($jobID);

		// if that fails, try to match job id with project ID. 
		if (!$projectID) {
			ProjectExists($jobID)
			? $projectID = $jobID
			: $projectID = null;
		}
		
		// If that fails, look for instruction, project=nickname
		if (!$projectID) {
			$ismatch = trim(preg_match ("/(project\s*=\s*)?(\S*)/i", $fileinfo['SpecialInstructions'], $x));

			if ($ismatch) {
				$ProjectNickname = $x[2];
			} else {
				$ProjectNickname = '';
			}
	
			//$catagories = explode (",", $fileinfo['Catagory']);
			//$keywords = explode (",", $fileinfo['Keywords']);
			//$arr = array_merge (array ($ProjectNickname), $catagories, $keywords);
			
			$projectID = FindProjectByName ($ProjectNickname);

		}


		// If we didn't find a project, we use the Artist's default project ID
		if (!$projectID)
			$projectID = $user->get_property('ProjectID');
			//$projectID = $artistinfo['ProjectID'];
	}

	// Check the use has storage space free
	// the 'true' forces the system to refresh the calculation of storage used, and not depend on the cached calculation
	if ($user->UserOverStorageLimit ($user->ID, true)) {
		fp_error_log(__FUNCTION__.__LINE__.":  *** User #{$user->ID} is over storage limit ({$user->StorageAllocated})! ***", 3, FP_PICTURES_LOG);
		$e = FetchSnippet ("error_over_storage_limit");
		$error .= "$e";
		return false;
	} else {
		// We got the project and the artist, let's deal with the picture
		if ($artistID && $projectID) {
			$VERBOSE && fp_error_log(__FUNCTION__.__LINE__.": Found User={$user->ID} ({$user->Lastname}), Project ID=$projectID for $filename", 3, FP_PICTURES_LOG);
		} else {
			fp_error_log(__FUNCTION__.__LINE__.":  *** Could not figure out artist/project info ({$user->ID} / $projectID) for $filename, so I rejected it.", 3, FP_PICTURES_LOG);
			$DEBUG || unlinkb ($filename) || $error .= "Cannot delete $filename<BR>";
			return false;
		}
	}

	//fp_error_log(__FUNCTION__.": CONCLUSION: ArtistiD = $artistID, ", 3, FP_PICTURES_LOG);
	fp_error_log("\n--------------------------------", 3, FP_PICTURES_LOG);


	return array ($projectID, $artistID);
}

//-------------------------------
// FindImageAssignments
// Try to assign an image to a project.
// Run this before processing to avoid processing files we can't use.
// $files : array of full file path names, e.g. /home/x/y/z/file.jpg
function FindImageAssignments ($files = array()) {
	global $PHOTOS_GALLERY, $THUMBNAILS, $MATTED, $FRAMED, $ORIGINALS, $SLIDES;
	global $PROCESSED_PHOTOS, $PROCESSED_THUMBNAILS, $PROCESSED_MATTED, $PROCESSED_FRAMED, $PROCESSED_ORIGINALS, $PROCESSED_SLIDES;
	global $PIXTOPROCESS;
	global $FP_GALLERY_TYPE;
	global $BASEDIR, $MAILED_DIR, $LOGS;
	global $msg, $error;
	
	$DEBUG = 0;
	$VERBOSE = 0;

	$goodfiles = array();
	foreach ($files as $filename) {
		$assignment = FindOneImageAssignment ($filename);
		if ($assignment != false) {
			list ($projectID, $artistID) = $assignment;
			$DEBUG && $msg .= __FUNCTION__.__LINE__.": projectid = $projectID, artistID = $artistID<BR>";
			$user = new FPUser ($artistID);
		
			// Check the use has storage space free
			// the 'true' forces the system to refresh the calculation of storage used, and not depend on the cached calculation
			if ($user->UserOverStorageLimit ($user->ID, true)) {
				fp_error_log(__FUNCTION__.__LINE__.":  *** User #{$user->ID} is over storage limit ({$user->StorageAllocated})! ***", 3, FP_PICTURES_LOG);
				$e = FetchSnippet ("error_over_storage_limit");
				$error .= "$e";
				$DEBUG || (unlinkb ($filename) || $error .= __FUNCTION__.": Cannot delete $filename<BR>");
			} else {
				$goodfiles[] = array ($filename, $artistID, $projectID);
			}
		} else {
			fp_error_log(__FUNCTION__.__LINE__.":  *** Could not figure out artist/project info ({$user->ID} / $projectID) for $filename, so I rejected it.", 3, FP_PICTURES_LOG);
			$DEBUG || (unlinkb ($filename) || $error .= __FUNCTION__.": Cannot delete $filename<BR>");
			$DEBUG && $msg .= "Should delete $filename (did not because debugging)<BR>";
		}
	}
	return $goodfiles; //should return array of arrays (filename, ArtistID, ProjectID) or False
}


//-------------------------------
// AddProcessedImagesToDatabase
//
// return an array:
//	$result[] = array($newID, $ArtistID, $projectID);
// This looks for the new pictures appearing in the "processed" folders,
// and adds them to the system. Pictures added by upload will be dumped in the 
// "processed" folders, but the $projectID will be set, so they can be assigned the correct project.
//
// We get the artistID from the image itself. We look for the artist name, and get the ID.
// If that fails, we try last name only.
// We look first in Credit, then Byline, then Source...assuming no one really knows where to put their name.
// If that fails, WE DUMP IT! This means dumping all versions in all the processed directories!
// That gives us all necessary info.
//
// We also FTP the new pictures to the artist's archive/distribution site, if it exists.
// Return array of arrays (newID, ArtistID, ProjectID) or False

function AddProcessedImagesToDatabase ($artistID = null, $projectID = null, $sendEmail = false) {
	global $PHOTOS_GALLERY, $THUMBNAILS, $MATTED, $FRAMED, $ORIGINALS, $SLIDES;
	global $PROCESSED_PHOTOS, $PROCESSED_THUMBNAILS, $PROCESSED_MATTED, $PROCESSED_FRAMED, $PROCESSED_ORIGINALS, $PROCESSED_SLIDES;
	global $PIXTOPROCESS;
	global $BASEDIR, $MAILED_DIR, $LOGS;
	global $msg, $error;
	
	$DEBUG = 0;
	
	$DEBUG && fp_error_log(__FUNCTION__, 3, FP_PICTURES_LOG);
	
	$EMAILMSG = false;

	// Read files in dir
	// If $doOnlyOneFile is set, this array is only one file, not all files.
	$files = GetListProcessedImageFiles ();
	if ($files) {
		// Add each file in the "processed" directory to project $projectID
		$result = false;
		foreach ($files as $filename) {
			$DEBUG && $msg .= __FUNCTION__.__LINE__.": Processing $filename<BR>";
			$DEBUG && $msg .= "projectid = $projectID, artistID = $artistID<BR>";
			$user = new FPUser ($artistID);

			$path = "$BASEDIR/$PROCESSED_ORIGINALS";
			$sourceFile = "$path/$filename";

			$ArtistID = $user->ID;
			$fileinfo = FetchIPTCInfo ($sourceFile);
			// Title should be in "Headline" IPTC field, so we swap the values.
			list($fileinfo['Headline'], $fileinfo['Title']) = array($fileinfo['Title'], $fileinfo['Headline']);

			$fileinfo['Title'] || $fileinfo['Title'] = $fileinfo['Headline'];
			$fileinfo['Title'] || $fileinfo['Title'] = $fileinfo['ObjectName'];
			if (!$fileinfo['Title'] && FP_IMAGES_DEFAULT_TITLE == "filename") {
				$ext = findexts ($filename);
				$fileinfo['Title'] = str_replace("." . $ext, "", $filename);
			}
			$fileinfo['Title'] || $fileinfo['Title'] = "Untitled";

			$fileinfo['ObjectName'] || $fileinfo['ObjectName'] = $filename;
			$fileinfo['Credit'] || $fileinfo['Credit'] = $artistfullname;
			$fileinfo['Byline'] || $fileinfo['Byline'] = $artistfullname;
			$fileinfo['Author'] || $fileinfo['Author'] = $artistfullname;
			//put together vars & info from file, with fileinfo overwriting vars with same name
			$vars = array_merge(StripBlankFields($vars), StripBlankFields($fileinfo));

			// Get any values from an accompanying info file
			// The file will be in the same location as the original file, but will
			// end with ".txt" instead of ".jpg".
			$infofile = str_ireplace(".jpg", ".txt", $sourceFile);
			if (file_exists($infofile))
			{
				$infoFromFile = unserialize(file_get_contents($infofile));
				unlinkb ($infofile);
			} else {
				$infoFromFile = array ();
			}

			// IPTC info overwrites file data; I consider it more likely to be valid.
			if ($infoFromFile)
				$vars = array_merge ($infoFromFile, $vars);

			$DEBUG && $error .= ArrayToTable($vars);

			// Check the user has storage space free
			// the 'true' forces the system to refresh the calculation of storage used, and not depend on the cached calculation
			if ($user->UserOverStorageLimit ($user->ID, true)) {
				fp_error_log(__FUNCTION__.__LINE__.":  *** User #{$user->ID} is over storage limit ({$user->StorageAllocated})! ***", 3, FP_PICTURES_LOG);
				$e = FetchSnippet ("error_over_storage_limit");
				$error .= "$e";
				DeleteAllPicVersions ($filename, true); // true=inside processed folders
				
			} else {
				// We got the project and the artist, let's deal with the picture
				$EMAILMSG && print __FUNCTION__.__LINE__.":  Emailing $filename<BR>\n";
				$successfulmove = MoveProcessedToMain ($filename);
				if ($successfulmove) {
					$newID = HandleImage ('insert', '', $user->ID, $projectID, $vars, $filename);
					$result[] = array($newID, $user->ID, $projectID);
					ClearProjectCache ($projectID);
					$result[] = array ($newID, $artistID, $projectID);
					
					if ($sendEmail) {
						// Email the user that the picture was entered into the gallery
						$artist = FetchArtist($artistID);
						$to = "{$artist['Fullname']} <{$artist['Email']}>";
						$subject = FP_SYSTEM_DISPLAY_NAME . ": Picture added to exhibition.";
					
						$text = FetchSnippet ("alert_artist_image_added_to_project");
						$text = Substitutions ($text, array (
								'Title'		=> $vars['Title'],
								'Filename'	=> $filename,
								'Username'	=> $artist['Username'],
								'Name'		=> $artist['Fullname']
						));
						$text = ReplaceSysVars ($text);
						if (!($result = mail($to, $subject, $text, FP_EMAIL_HEADERS))) {
							fp_error_log(__FUNCTION__.": FAILED TO send email to user #{$artistID} at address $to.", 3, FP_ERROR_LOG);
						}
					}
					// FTP the picture to the artist's archive site. The function will check if there is an FTP.
					$res = FTPImageToArchiveSite ($newID);
					$res && fp_error_log(__FUNCTION__.__LINE__.":  FTP image to archive site if exists ($res)", 3, FP_PICTURES_LOG);
					$res && $msg .= $res;
				}
			}
		}
	}
	//should return array of arrays (newID, ArtistID, ProjectID) or False
	return $result;
}


// Move complete sets of processed files from the 'processed' folders to the image folders
function MoveProcessedToMain ($filename) {
	global $PHOTOS_GALLERY, $THUMBNAILS, $MATTED, $FRAMED, $ORIGINALS, $SLIDES;
	global $SITEIMAGES, $PROCESSEDDIR, $PROCESSED_PHOTOS, $PROCESSED_THUMBNAILS, $PROCESSED_MATTED, $PROCESSED_FRAMED, $PROCESSED_ORIGINALS, $PROCESSED_SLIDES;
	global $BASEDIR, $LOGS;
	global $msg, $error;

	$DEBUG = 0;
	$DEBUG && $msg .= "<BR>".__FUNCTION__ ." $filename<BR>";
 	
 	$logerror = "";
 	
	// Move processed images to the storage directories
	//$fromDirs = array ($PROCESSED_PHOTOS, $PROCESSED_THUMBNAILS, $PROCESSED_MATTED, $PROCESSED_FRAMED, $PROCESSED_SLIDES);
	//$toDirs = array ($PHOTOS_GALLERY, $THUMBNAILS, $MATTED, $FRAMED, $SLIDES);
	$fromDirs = array ($PROCESSED_ORIGINALS, $PROCESSED_PHOTOS, $PROCESSED_THUMBNAILS, $PROCESSED_SLIDES);
	$toDirs = array ($ORIGINALS, $PHOTOS_GALLERY, $THUMBNAILS, $SLIDES);
	
	// Is the file locked? If so, return
	if (is_processing_image ($filename)) {
		fp_error_log(__FUNCTION__.__LINE__.": Oops, $filename is being processed or moved!", 3, FP_PICTURES_LOG);
		return false;
	}
	
	// Lock the file
	set_lock_for_file ($filename);
	
	
	// Do all versions exist? The system might still be processing even though some files exist
	$missing = 0;
	for ($i=0; $i < count($fromDirs); $i++) {
		$from = "{$BASEDIR}/{$fromDirs[$i]}/{$filename}";
		if (!file_exists ($from)) {
			$missing++;
		}
	}

	if (!$missing) {
		$debug && fp_error_log(__FUNCTION__.__LINE__.": Moving " . basename($filename) . " out of 'processed' folders.", 3, FP_PICTURES_LOG);
		for ($i=0; $i < count($fromDirs); $i++) {
			$from = $BASEDIR."/".$fromDirs[$i] . "/" . $filename;
			$to = $BASEDIR."/".$toDirs[$i] . "/" . $filename;

			$DEBUG && (file_exists ($from) || $logerror .= __FUNCTION__.__LINE__.": missing : $from<BR>");
			
			$DEBUG && $msg .= __FUNCTION__ .":".__LINE__ .": Move $from to $to<BR>\n";
			if (file_exists($from)) {
				copy ($from, $to) || $logerror .= __LINE__.":  Could not copy $from to $to<BR>";
				unlinkb ($from)  || $logerror .= __LINE__.":  Could not delete $from<BR>";
				chmod ($to, 0644)  || $logerror .= __LINE__.":  Could fix permissions (0644) for $to<BR>";
			}
		}
	} else {
		$DEBUG && $logerror .= __LINE__.":  Did not move files...some are missing";
	}
	$logerror && fp_error_log(__FUNCTION__.$logerror, 3, FP_PICTURES_LOG);
	clear_lock_for_file ($filename);
	$result = ($logerror == "");
	return ($result);
}


// =====
// Process files in the photos/input directory:
// Process uploaded image files from Flash Uploader or FTP
function ProcessUploadedImagesFTP ($projectID = null) {
	global $BASEDIR, $MAILED_DIR, $LOGS;
	global $msg, $error;

	$DEBUG = 0;
	
	$error .= __FUNCTION__.": DEPRECATED! USE ProcessLocalImages instead<BR>";
	fp_error_log(__FUNCTION__.": *** DEPRECATED! USE ProcessLocalImages instead", 3, FP_PICTURES_LOG);
	return false;
	
	
	$images = array();

	$files = array();
	$images = false; // no action was performed is default
	// Get files to process
	$files = GetPictureFileTree (100);
	$files && fp_error_log(__FUNCTION__.": -----------".date ("Y-m-d h:m:s")."---------", 3, FP_PICTURES_LOG);

	$files = FindImageAssignments ($files);
	$files = array_slice($files, 0, FP_PIXTOPROCESS);
	
	$result = false;
	foreach ($files as $file) {
		list($filename, $artistID, $projectID) = $file;
		$PID = trim(array_pop ( explode ("/", dirname($filename)) ));
		is_numeric ($PID) && $projectID = $PID;
		$project = FetchProject ($projectID);
		$user = new FPUser ();
		$user->loadUser ($project['ArtistID']);
		$watermark = $user->DefaultCopyrightNotice;
		$result = ProcessOneImage ($filename, "image/jpeg", $filename, $watermark);
		if ($result) {
			$result = AddProcessedImagesToDatabase ($artistID, $projectID, true) ;
		} else {
			fp_error_log(__FUNCTION__.":".__LINE__.": Cannot process $filename...ProcessOneImage failed.<BR>", 3, FP_ERROR_LOG);
		}
	}

	return $result;
}


//-------------------------------
// Process images, but get them from files already in the system
// Look in the input directory, which gets files from emails to the system.
// $allowed_reserved_names allows processing of system reserved names, such as
// 'missing_image.jpg', so we can use this function internally during setup.
function ProcessLocalImages ($overwrite = false, $allowed_reserved_names = false) {
	global $PHOTOS_GALLERY, $SLIDES, $THUMBNAILS, $MATTED, $FRAMED, $ORIGINALS, $MAILED_DIR;
	global $PROCESSED_PHOTOS, $PROCESSED_SLIDES, $PROCESSED_THUMBNAILS, $PROCESSED_MATTED, $PROCESSED_FRAMED, $PROCESSED_ORIGINALS;
	global $PIXTOPROCESS;
	global $BASEDIR, $LOGS;
	global $msg, $error;

	$DEBUG = 0;
	$DEBUG && $msg .= __FUNCTION__. ": " . getCallingFunction() . " called me!<BR>";
	$files = array();
	$images = false; // no action was performed is default
	// Get files to process
	$files = GetPictureFileTree (100);
	$files && fp_error_log(__FUNCTION__.": -----------".date ("Y-m-d h:m:s")."---------", 3, FP_PICTURES_LOG);

	$files = FindImageAssignments ($files);
	$files = array_slice($files, 0, FP_PIXTOPROCESS);
	
	$result = false;
	foreach ($files as $file) {
		list($filename, $artistID, $projectID) = $file;
		$DEBUG && fp_error_log(__FUNCTION__ . ": Process $filename for artist $artistID, project $projectID", 3, FP_PICTURES_LOG);
		$DEBUG && $msg .= __FUNCTION__ . ": Process $filename for artist $artistID, project $projectID<BR>";
		$user = new FPUser (); 
		//$user->loadUser ($project['ArtistID']);
		$user->loadUser ($artistID);
		$watermark = $user->DefaultCopyrightNotice;
		$result = ProcessOneImage ($filename, "image/jpeg", $filename, $watermark, $overwrite, $allowed_reserved_names);
		$DEBUG && $msg .= __FUNCTION__ .":".__LINE__ .	": ran ProcessOneImage on $filename<BR>\n".ArrayToTable ($result);
		if ($result) {
			$result = AddProcessedImagesToDatabase ($artistID, $projectID, false);
			$result
				? fp_error_log(__FUNCTION__.__LINE__. ": Add $filename to system with projectID = '{$projectID}' and artistID = '{$artistID}'", 3, FP_PICTURES_LOG)
				: fp_error_log(__FUNCTION__.__LINE__. ": *** Failed to add $filename to system with projectID = '{$projectID}'", 3, FP_PICTURES_LOG);
		}
	}
	return $result;
}



// Return a list of files in the emailed images directory
// The images are put there by a Perl email parser.
// $count tells how many to return for processing, so we can avoid
// overloading the server doing graphic resizes.
// We only get JPG files.
// Returns an array of file names.
// $count = 0 returns all files.
function PicturesToProcess () {
	global $PHOTOS_GALLERY, $SLIDES, $THUMBNAILS, $MATTED, $FRAMED, $ORIGINALS, $MAILED_DIR;
	global $PROCESSED_PHOTOS, $PROCESSED_SLIDES, $PROCESSED_THUMBNAILS, $PROCESSED_MATTED, $PROCESSED_FRAMED, $PROCESSED_ORIGINALS;
	global $PIXTOPROCESS;
	global $BASEDIR, $LOGS;
	global $msg, $error;

	$DEBUG = 0;

	$directory = "$BASEDIR/$MAILED_DIR/";

	$files = array();
	
	$files = $files + GetPictureFileList (1, $directory);

	foreach ( glob($directory."*",GLOB_ONLYDIR) as $folder) {
		$files = array_merge($files, GetPictureFileList (0, $folder."/"));
		$DEBUG && $msg .= __FUNCTION__.": Files in $folder:".ArrayToTable ($files);
	}
	$count && $files = array_slice ($files, 0, $count);
	return $files;
}


function all_files($dir) {
	$files = Array();
	$file_tmp= glob($dir.'*.{jpg,JPG}',GLOB_MARK | GLOB_NOSORT);

	foreach($file_tmp as $item){
		if(substr($item,-1)!=DIRECTORY_SEPARATOR)
			 $files[] = $item;
		else
			$files = array_merge($files,all_files($item));
		}
	return $files;
}




// Return a list of files in the emailed images directory
// The images are put there by a Perl email parser.
// $count tells how many to return for processing, so we can avoid
// overloading the server doing graphic resizes.
// We only get JPG files.
// Returns an array of file names.
// $count = 0 returns all files.
function GetPictureFileTree ($numPixToProcess = 1) {
	global $MAILED_DIR;
	global $BASEDIR, $LOGS;
	global $msg, $error;

	$DEBUG = 0;
	$directory = "$BASEDIR/$MAILED_DIR/";

	// Get pictures from $mailed_dir (not in a subdirectory)
	$files = GetPictureFileList ($numPixToProcess, $directory);
	
	// stop looking if we have enough pictures to work with
	if ($numPixToProcess == 0 || count ($files) < $numPixToProcess) {
		$DEBUG && print "count=$numPixToProcess<BR>";
		// Check directories
		foreach ( glob($directory."*", GLOB_ONLYDIR | GLOB_MARK) as $folder) {
			$files = array_merge($files, GetPictureFileList (0, $folder));
			if ($numPixToProcess > 0 && count ($files) >= $numPixToProcess)
				break;
			$DEBUG && $msg .= __FUNCTION__.": Files in $folder:".ArrayToTable ($files). "<BR>";
		}
	}
	$numPixToProcess && $files = array_slice ($files, 0, $numPixToProcess);
	return $files;
}



// Return a list of files in the emailed images directory
// The images are put there by a Perl email parser.
// $count tells how many to return for processing, so we can avoid
// overloading the server doing graphic resizes.
// We only get JPG files.
// Returns an array of file names.
// $count = 0 returns all files.
// (does not) Deletes empty directories (not in use anymore) to keep things pretty
function GetPictureFileList ($numPixToProcess = 1, $directory = null) {
	global $MAILED_DIR;
	global $BASEDIR, $LOGS;
	global $msg, $error;

	$DEBUG = 0;
	$directory || $directory = "$BASEDIR/$MAILED_DIR/";
	$files = glob($directory.'*.{jpg,JPG}', GLOB_BRACE);
	$DEBUG && $msg .= __FUNCTION__.": files in $directory = ". join (", ", $files) . "<BR>";
	$numPixToProcess && $files = array_slice ($files, 0, $numPixToProcess);
	return $files;
}


//------------------------
// DeleteImageFiles
// Delete files associated with an image row in the DB (filename is 'URL');
function DeleteImageFiles ($ID) {
	global $PHOTOS_GALLERY, $SLIDES, $THUMBNAILS, $MATTED, $FRAMED, $ORIGINALS, $MAILED_DIR, $BASEDIR, $LOGS;
	global $user;
	
	$info = FetchRowsByValue ('Images', 'URL, ArtistID', 'ID', $ID, '', TRUE);
	$filename = $info['URL'];
	if ($filename != FP_SYSTEM_MISSING_PICTURE) {
		$artistID = $info['ArtistID'];
	//	print "DeleteImageFiles:$ID -> $filename<BR>\n";
		DeleteAllPicVersions ($filename, false);
		isset ($user) && $user->CalculateStorageUsed ($artistID);
		fp_error_log("DeleteImageFiles: deleted ID=$ID, filename = $filename\n------", 3, FP_PICTURES_LOG);
	} else {
		fp_error_log(__FUNCTION__.": ERROR: Attempt to delete FP system file, ".FP_SYSTEM_MISSING_PICTURE.".", 3, FP_PICTURES_LOG);
	}
}

//----
// DumpPicture
// Delete the various resized/matted versions of an image in the PROCESSED folders
function DeleteAllPicVersions ($filename, $inProcessedFolder=false) {
	global $PHOTOS_GALLERY, $SLIDES, $THUMBNAILS, $MATTED, $FRAMED, $ORIGINALS, $MAILED_DIR, $BASEDIR, $LOGS;
	global $PROCESSED_PHOTOS, $PROCESSED_THUMBNAILS, $PROCESSED_MATTED, $PROCESSED_FRAMED, $PROCESSED_ORIGINALS, $PROCESSED_SLIDES;
	global $BASEDIR, $LOGS;
	global $msg, $error;
		
	$DEBUG = false;
	
	if (!$filename)
		return;
	
	if ($inProcessedFolder) {
		if ($DEBUG) {
			print "delete " . "$BASEDIR/$PROCESSED_PHOTOS/" . $filename . "<BR>";
			print "delete " . "$BASEDIR/$PROCESSED_SLIDES/" . $filename . "<BR>";
			print "delete " . "$BASEDIR/$PROCESSED_THUMBNAILS/" . $filename . "<BR>";
			print "delete " . "$BASEDIR/$PROCESSED_MATTED/" . $filename . "<BR>";
			print "delete " . "$BASEDIR/$PROCESSED_FRAMED/" . $filename . "<BR>";
			print "delete " . "$BASEDIR/$PROCESSED_ORIGINALS/" . $filename . "<BR>";		
			print "delete " . "$BASEDIR/$MAILED_DIR/" . $filename . "<BR>";		
		}		
		(file_exists (("$BASEDIR/$MAILED_DIR/$filename"))) && (unlinkb ("$BASEDIR/$MAILED_DIR/$filename"));
		(file_exists (("$BASEDIR/$PROCESSED_PHOTOS/" . $filename))) && (unlinkb ("$BASEDIR/$PROCESSED_PHOTOS/" . $filename));
		(file_exists (("$BASEDIR/$PROCESSED_SLIDES/" . $filename))) && (unlinkb ("$BASEDIR/$PROCESSED_SLIDES/" . $filename));
		(file_exists (("$BASEDIR/$PROCESSED_THUMBNAILS/" . $filename))) && (unlinkb ("$BASEDIR/$PROCESSED_THUMBNAILS/" . $filename));
		(file_exists (("$BASEDIR/$PROCESSED_MATTED/" . $filename))) && (unlinkb ("$BASEDIR/$PROCESSED_MATTED/" . $filename));
		(file_exists (("$BASEDIR/$PROCESSED_FRAMED/" . $filename))) && (unlinkb ("$BASEDIR/$PROCESSED_FRAMED/" . $filename));
		(file_exists (("$BASEDIR/$PROCESSED_ORIGINALS/" . $filename))) && (unlinkb ("$BASEDIR/$PROCESSED_ORIGINALS/" . $filename));
		$DEBUG && fp_error_log("DeleteAllPicVersions: deleted all PROCESSED versions of $filename", 3, FP_MAINTENANCE_LOG);
	} else {
		if ($DEBUG) {
			print "delete $BASEDIR/$PHOTOS_GALLERY/$filename<BR>";
			print "delete $BASEDIR/$SLIDES/$filename<BR>";
			print "delete $BASEDIR/$THUMBNAILS/$filename<BR>";
			//print "delete $BASEDIR/$MATTED/$filename<BR>";
			//print "delete $BASEDIR/$FRAMED/$filename<BR>";
			print "delete $BASEDIR/$ORIGINALS/$filename<BR>";
		}				
		(file_exists (("$BASEDIR/$PHOTOS_GALLERY/$filename"))) && (unlinkb ("$BASEDIR/$PHOTOS_GALLERY/$filename"));
		(file_exists (("$BASEDIR/$SLIDES/$filename"))) && (unlinkb ("$BASEDIR/$SLIDES/$filename"));
		(file_exists (("$BASEDIR/$THUMBNAILS/$filename"))) && (unlinkb ("$BASEDIR/$THUMBNAILS/$filename"));
		//(file_exists (("$BASEDIR/$MATTED/$filename"))) && (unlinkb ("$BASEDIR/$MATTED/$filename"));
		//(file_exists (("$BASEDIR/$FRAMED/$filename"))) && (unlinkb ("$BASEDIR/$FRAMED/$filename"));
		(file_exists (("$BASEDIR/$ORIGINALS/$filename"))) && (unlinkb ("$BASEDIR/$ORIGINALS/$filename"));
		fp_error_log("DeleteAllPicVersions: deleted all IN USE versions of $filename", 3, FP_MAINTENANCE_LOG);
	}
}


//------
// DeleteUserPicture
// Delete a picture from the user directory (inside "_user/_img")
function DeleteUserPicture ($filename) {
	global $BASEDIR;
	
	if ($filename) {
		file_exists ("$BASEDIR/".FP_DIR_USER_IMAGES."/$filename") && unlinkb ("$BASEDIR/".FP_DIR_USER_IMAGES."/$filename");
		$filename = preg_replace ('/\.jpg$/i', '_thumbnail.jpg', $filename);
		file_exists ("$BASEDIR/".FP_DIR_USER_IMAGES."/$filename") && unlinkb ("$BASEDIR/".FP_DIR_USER_IMAGES."/$filename");
	}
}

//------
// Process a single uploaded image NOT for the gallery, but for a user portrait or gallery icon
// The name of the picture will be changed to $filename.
// The picture will be stored in the user's image directory
// The HTLM field is "userspecialfile" not "userfile"!
// We resize to 
function ProcessUploadedSingleImage ($fp_user, $fieldName = "userfile") {
	global $PHOTOS_GALLERY, $SLIDES, $THUMBNAILS, $MATTED, $FRAMED, $ORIGINALS, $MAILED_DIR;
	global $PROCESSED_PHOTOS, $PROCESSED_SLIDES, $PROCESSED_THUMBNAILS, $PROCESSED_MATTED, $PROCESSED_FRAMED, $PROCESSED_ORIGINALS;
	global $BASEDIR, $MAILED_DIR, $LOGS;
	global $default_size, $slide_size, $default_border, $thumb_size, $thumb_border;
	global $msg, $error;

	$DEBUG = 0;
	
	$DEBUG && $msg .= __FUNCTION__.": fieldName = $fieldName<BR>";
	
// Don't bother if nothing submitted!
	if ($_FILES[$fieldName]['tmp_name']) {

		$images = array();
		$fp_user || (isset ($_SESSION['fp_user']) ? $fp_user = $_SESSION['fp_user'] : $fp_user = NULL);
		$artistinfo = FetchArtist ($fp_user);
	
		$filename = $_FILES[$fieldName]['name'];
		$f = $filename; // store for roll/frame use
		$filetype = $_FILES[$fieldName]['type'];
		$filesize = $_FILES[$fieldName]['size'];	//	The size, in bytes, of the uploaded file. 
		$tempname = $_FILES[$fieldName]['tmp_name'];	//	Upload temporary name
	
		$DEBUG && print "<BR>";
		$DEBUG && var_dump ($_FILES[$fieldName]);

		$ext = ".jpg";	// default
		($filetype == "image/png") && $ext = ".png";
	
		if ($filetype != "image/jpeg" && $filetype != "image/png") {
			$error .= "Error: You can only upload JPG or PNG pictures (file was of type $filetype).";
			return false;
		}
	
		
		$DEBUG && $error .= __LINE__.": userfileTag:".$_REQUEST['userfileTag']."<BR>";


		if (isset($_REQUEST['userfileTag'])) {
			$tag = trim($_REQUEST['userfileTag']);
		
			switch ($tag) {
				case "artist_portrait" :
					$newname = $tag."_".$fp_user.$ext;
					$newnameT = $tag."_".$fp_user."_thumbnail".$ext;
				break;
				case "group_icon" :
					// Remove a file with the same name but different extension, 
					$ext == ".jpg" 
						? DeleteAllPicVersions ($_REQUEST['userfileName'].".png")
						: DeleteAllPicVersions ($_REQUEST['userfileName'].".jpg");
					$newname = $_REQUEST['userfileName'].$ext;
					$newnameT = $_REQUEST['userfileName']."_thumbnail".$ext;
				break;
				case "group_logo" :
					// Remove a file with the same name but different extension, 
					$ext == ".jpg" 
						? DeleteAllPicVersions ($_REQUEST['userfileName'].".png")
						: DeleteAllPicVersions ($_REQUEST['userfileName'].".jpg");
					$newname = $_REQUEST['userfileName'].$ext;
					$newnameT = $_REQUEST['userfileName']."_thumbnail".$ext;
				break;
			}
			$newpath = "$BASEDIR/" . FP_DIR_USER_IMAGES . "/$newname";
		} else {
			$newpath = "$BASEDIR/" . FP_DIR_USER_IMAGES . "/$newname";
		}

		if ($filetype != "image/png") {
		// Resize JPG, then keep only the 'photos' and 'thumbnail' versions
		//if (file_exists ($newpath)) {
				$watermarktext = "";
				if (ResizeNewImages ($tempname, $newname, $watermarktext)) {
					copy ("$BASEDIR/".FP_DIR_PROCESSED_PHOTOS."/$newname", "$BASEDIR/".FP_DIR_USER_IMAGES."/$newname");
					copy ("$BASEDIR/".FP_DIR_PROCESSED_THUMBNAILS."/$newname", "$BASEDIR/".FP_DIR_USER_IMAGES."/$newnameT");
				}
				unlinkb ($tempname);
				// remove extra sized pics from processed folders while leaving alone anything
				// else that might be there.
				CleanProcessedDirs ($newname);
			//}
		} else {
		// Don't resize PNG files, just move
			if (copy ($tempname, $newpath)) {
				unlinkb ($tempname);
				chmod ($newpath, 0644);
			}
			$DEBUG && $error .= __FUNCTION__.": file is PNG: Newpath is $newpath, newname=$newname<BR>";
		}
		return $tag;
	} else {
		return false;
	}
}


//------------------------
// Process uploaded image files
// **(called from the main admin script)
// We are using userfile[] in the HTML, so we're going to get an array
// of files, not a single one. This allows for multiple uploads.
// Resize uploaded files, then move them to the storage directories,
// then insert entries in database

function ProcessUploadedImages ($projectID="", $fp_user) {
	global $msg, $error;

	$DEBUG = 0;
	
	$images = array();
	if ($_FILES["userfile"]["name"]) {
		$fp_user || (isset ($_SESSION['fp_user']) ? $fp_user = $_SESSION['fp_user'] : $fp_user = NULL);
		
		$artistID = GetMainUserID ($fp_user, $projectID);
		
		$DEBUG && $msg .= __FUNCTION__ .":".__LINE__ . ": Picture owner should be #{$artistID} into project {$projectID}<BR>";
		$DEBUG && $msg .= __FUNCTION__ .":".__LINE__ . ": Picture count: ".$_FILES["userfile"]."<BR>";
		
		$artistinfo = FetchArtist ($artistID);
		$DEBUG && $msg .= __FUNCTION__ .":".__LINE__ . ":<BR>";
		$watermark = $artistinfo['DefaultCopyrightNotice'];
		$dir = FP_DIR_MAILED_DIR."/$projectID";
		
		for ($i = 0; $i < count ($_FILES["userfile"]['name']); $i++) {
			
			if (preg_match("/(\.jpg|\.png)$/i",$_FILES["userfile"]['name'][$i])) {
			
				// Move to the right folder and let other processing methods take over
				$tempname = $_FILES['userfile']['tmp_name'][$i];
				$newname = UniqueFilename ($dir, $_FILES["userfile"]['name'][$i]);
				$newpath = "$dir/$newname";
		
				$DEBUG && $msg .= __FUNCTION__.__LINE__.": Put $newname into $dir<BR>";
			
				if (file_exists ($tempname) && !empty($newname) && copy ($tempname, $newpath)) {
					unlinkb ($tempname);
					chmod ($newpath, 0644);
					// Lock the file against further processing 
					// I don't think this is necessary...once it's moved here, nothing else will try to touch it.
				} else {
					fp_error_log(__FUNCTION__.":".__LINE__.": Could not copy file from \r$tempname\r to \r$newpath\r (check permissions? All photo directories must be 755!)<BR>", 3, FP_PICTURES_LOG);
				}
				
				// This would process the pic right away...we don't do that anymore.
				//$images[] = ProcessOneUploadedImage($watermark, $i);
			}
		}
	}
	
	//AddProcessedImagesToDatabase ($projectID);
	return $images;
}

// Be sure to addslashes the suffices
// This should work: $goodsearch = "\.jpeg$|\.jpg$|\.txt$";
function DeleteBadUploadedFiles ($goodsearch=null) {
	global $BASEDIR, $MAILED_DIR;
	global $msg, $error;
	
	$DEBUG = 0;
	$goodsearch || $goodsearch = '\.jpeg$|\.jpg$|\.txt$';
	$directory || $directory = "$BASEDIR/$MAILED_DIR/";
	$flags = null;
	$files = rglob('*', $flags, $directory);
	$DEBUG && $msg .= $directory;
	foreach ($files as $file) {
	    if (is_file ($file) && (basename($file) != "index.html") && !preg_match ("/".$goodsearch."/i", $file) && !preg_match ("/^\./", $file)) {
		$DEBUG && $msg .= __FUNCTION__.": Delete $file <BR>";
		fp_error_log(__FUNCTION__.": Delete $file in $directory", 3, FP_MAINTENANCE_LOG);
		unlinkb ($file);
	    } else {
		$DEBUG && $msg .= __FUNCTION__.": $file is OK : $goodsearch<BR>";
	    }
	}
}


//-----------
// Get a web-page uploaded image and process it.
// return image info for insertion into database
// The file name is used for roll-frame information.
// After processing, files are in the Processed directories in the Photos directory
function ProcessOneLocalImage ($path, $filename, $watermark = false, $overwrite = false) {
	global $PHOTOS_GALLERY, $SLIDES, $THUMBNAILS, $MATTED, $FRAMED, $ORIGINALS;
	global $msg, $error;
	
	$DEBUG = false;
	
	$f = $filename; // store this for a moment...for roll/frame use below
	$path = rtrim ($path, "/");
	$path && $filename = $path . "/" . $filename;
	$filesize = filesize($filename);

	$DEBUG && $msg .= __FUNCTION__.": process $filename<BR>";

	$fileinfo = ProcessOneImage ($filename, "image/jpeg", $filename, $watermark, $overwrite);
	// if filename is of the form XXX-YYY or XXX_YYY then use these for a roll/frame value	
	$ismatch = trim(preg_match ("/(.+)[_-](.+)\.jpg/i", $f, $arr));
	if ($ismatch) {
		$fileinfo['RollID'] = $arr[1];
		$fileinfo['FrameID'] = $arr[2];
	}
	return $fileinfo;
}



//-----------
// Get a web-page uploaded image and process it.
// Return image info for insertion into database
function ProcessOneUploadedImage ($watermark = false, $i=0) {
	global $PHOTOS_GALLERY, $SLIDES, $THUMBNAILS, $MATTED, $FRAMED, $ORIGINALS;
	global $BASEDIR, $LOGS;
	global $msg, $error;
	
	$DEBUG = 0;
	
	//ignore passed parameter if called from the web site
	isset($_SESSION['fp_user']) && $fp_user = $_SESSION['fp_user']; 
	
	$fileinfo = array();

	$filename = $_FILES["userfile"]['name'][$i];
	if ($filename) {
		$f = $filename; // store for roll/frame use
		$filetype = $_FILES["userfile"]['type'][$i];
		$filesize = $_FILES['userfile']['size'][$i];	//	The size, in bytes, of the uploaded file. 
		$tempname = $_FILES['userfile']['tmp_name'][$i];	//	Upload temporary name
			
		$DEBUG && $msg .= basename(__FILE__).": ".__FUNCTION__ .":".__LINE__.":<BR>";
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": File name=$filename<BR>\ntype: $filetype<BR>\nsize=$filesize<BR>\ntempname=$tempname<BR>\n";
	
		//	This doesn't seem to be necessary, even though the file names have // in the middle. Hmmmm.
		//	$tempname = preg_replace ("/\/\//","/",$tempname);
	
		$fileinfo = ProcessOneImage ($filename, $filetype, $tempname, $watermark);
		// if filename is of the form XXX-YYY or XXX_YYY then use these for a roll/frame value	
		if ($fileinfo != false) {
			$ismatch = trim(preg_match ("/(.+)[_-](.+)\.jpg/i", $f, $arr));
			if ($ismatch) {
				$fileinfo['RollID'] = $arr[1];
				$fileinfo['FrameID'] = $arr[2];
			}
		}
	}
	return $fileinfo;
}

//-----------
// function ProcessOneImage
// If it's uploaded file, we make the resized pictures, then move it to the Processed folders,
// where a Perl-script processed picture would be (one emailed in).
// Otherwise, we assume it's already resized.
// From the Processed folders, the picture is moved to the final folders and the file info
// is RETURNED so the picture can be added to the database.
// $allowed_reserved_names allows us to use this function to process
// system pictures, such as the missing image picture during setup.
function ProcessOneImage ($filename, $filetype, $tempname, $watermark = false, $overwrite = false, $allowed_reserved_names = false) {
	global $PHOTOS_GALLERY, $SLIDES, $THUMBNAILS, $MATTED, $FRAMED, $ORIGINALS, $MAILED_DIR;
	global $PROCESSED_PHOTOS, $PROCESSED_SLIDES, $PROCESSED_THUMBNAILS, $PROCESSED_MATTED, $PROCESSED_FRAMED, $PROCESSED_ORIGINALS;
	global $BASEDIR, $LOGS;
	global $msg, $error;
	
	$DEBUG = 0;
	$EMAILMSG = false;
	
	$DEBUG && $msg .= basename(__FILE__) .":".__FUNCTION__ .": BEGIN (called by ".getCallingFunction().")<BR>";
	
	$result = false;

	$DEBUG && fp_error_log(__FUNCTION__.": $filename", 3, FP_PICTURES_LOG);
	if (!$filename) {
		fp_error_log(__FUNCTION__.": *** Oops! No filename given! Called by ".getCallingFunction(), 3, FP_PICTURES_LOG);
		return false;
	}
		
	// If we are already processing this file, don't try to do it again!
	if (is_processing_image ($filename)) {
		fp_error_log(__FUNCTION__.": Oops, attempt to process '$filename' when it is already being processed!", 3, FP_PICTURES_LOG);
		return ;
	} else {	
		set_lock_for_file ($filename);
	}
	
	$starttime = microtime(true);

	$fileinfo = array();
	
	if ($filetype != "image/jpeg") {
		return array ('error' => "Image is not JPEG");
	}

	// Check size of image: keep within config bounds
	
	$imgsize = GetLocalImageSize($tempname);
	$width = $imgsize[0];
	$height = $imgsize[1];
	$DEBUG && $msg .= basename(__FILE__) .":".__FUNCTION__ .":".__LINE__ . ": MAXPIXELS = ". MAXPIXELS."<BR>".ArrayToTable ($imgsize);
	if (($width * $height) > MAXPIXELS) {
		fp_error_log(__FUNCTION__.":".__LINE__.": Cannot process this image: The image size is $width x $height = ". ($width * $height).", but this system can only handle pictures up to ".MAXPIXELS." in size.<BR>", 3, FP_PICTURES_LOG);
		$error .= "Cannot process this image: The image size is $width x $height = ". ($width * $height).", but this system can only handle pictures up to ".MAXPIXELS." in size.<BR>";
		unlinkb ($tempname);
		return array ();
	}
	
	// New resized pix will be in the processed directories, awaiting creation of new database entry.
	// If exists file with same name, rename as filename-1, or -2 if -1 exists, etc.
	
	// clean up evil characters in name
	$newname = basename ($filename);
	$newname = preg_replace ('/(\$|\!|\ |\=|\&|\?|\!|\"|\'|\@|\#|\\|\%|\^|\&|\*)+/', "_", $newname);
	$newname = preg_replace ('/_-/', "-", $newname);
	$DEBUG && $msg .= __FUNCTION__.__LINE__.": Original pic name: $filename<BR>New name: $newname<BR>";
	
	// Make sure name isn't a reserved name, like "missing_image" or "artist_portrait..."
	$allowed_reserved_names || $newname = FixNameIfReserved ($newname);
	$DEBUG && $msg .= __FUNCTION__.__LINE__.": Original pic name: $filename<BR>New name: $newname<BR>";
	
	// Make sure won't overwrite existing IF overwrite flag not set
	// Check existing AND processed pix for existing names
	if (!$overwrite) {
		$newname = UniqueFilename ("$BASEDIR/$PROCESSED_ORIGINALS",$newname);
		$newname = UniqueFilename ("$BASEDIR/$ORIGINALS",$newname);
	}
	$newpath = "$BASEDIR/$PROCESSED_ORIGINALS/$newname";

	$DEBUG && $msg .= __FUNCTION__ .":".__LINE__ . ": tempname = $tempname<BR>";
	$DEBUG && (file_exists ($tempname) && $msg .= __FUNCTION__ .":".__LINE__ . ": tempname exists<BR>");
	$DEBUG && $msg .= __FUNCTION__ .":".__LINE__ . ": copy $tempname --> $newpath<BR>";
	
	if (file_exists ($tempname) && !empty($newname)  && copy ($tempname, $newpath)) {
		unlinkb ($tempname);
		chmod ($newpath, 0644);

		// If there's an accompanying info file, copy it, too
		// JPG ONLY! Generalize for any extension, later...
		$infoTempFilename = str_ireplace(".jpg",".txt",basename($tempname));
		$tempInfoPath = dirname($tempname)."/".$infoTempFilename;
		$newInfoFilename = str_ireplace(".jpg",".txt",$newname);
		$newInfoPath = dirname($newpath)."/".$newInfoFilename;
		if (file_exists ($tempInfoPath) && !empty($infoTempFilename) && copy ($tempInfoPath, $newInfoPath)) {
		    unlinkb ($tempInfoPath);
		}

		// Lock the file against further processing 
		// I don't think this is necessary...once it's moved here, nothing else will try to touch it.
	} else {
		fp_error_log(__FUNCTION__.":".__LINE__.": Could not copy file from \r$tempname\r to \r$newpath\r (check permissions? All photo directories must be 755!)<BR>", 3, FP_PICTURES_LOG);
	}

	$DEBUG && fp_error_log(__FUNCTION__.__LINE__.": Resize $newname", 3, FP_PICTURES_LOG);
	$DEBUG && $msg .= __FUNCTION__.__LINE__.": Original pic name: $filename<BR>New name: $newname<BR>";
	if (file_exists ($newpath)) {
		$result = ResizeNewImages ($newpath, $newname, $watermark);
		// Remember the original name
		$fileinfo['fp_filename'] = basename($newname);
		$result = $fileinfo;
	} else {
		$result = false;
	}

	!$result && fp_error_log(__FUNCTION__.__LINE__.": *** Processing failed.", 3, FP_PICTURES_LOG);

	clear_lock_for_file ($filename);	
	fp_error_log(__FUNCTION__.__LINE__.": Processing $filename took ".round (microtime(true) - $starttime, 2). " seconds.", 3, FP_PICTURES_LOG);
	return $result;
}


// ------
// Check beginning of the file name against reserved words
// Rename with a _ if it's a reserved word name
// missing_image
// artist_portrait
// group_icon
// group_logo
function FixNameIfReserved ($name) {
	
	$reservednames = array (FP_SYSTEM_MISSING_PICTURE, "artist_portrait", "group_icon", "group_logo");
	
	foreach ($reservednames as $rn) {
		if (substr($name, 0, strlen($rn)) == $rn) {
			$name = "userfile_" . $name;
			break ;
		}
		return $name; 
	}
}

// Resize an image to all the useful sizes (slide, galler, thumbnail) 
// Add a border if value is given.
// IF $watermark is set, use the CopyRightText field, or Byline, or Credit (in that order) of the IPTC to mark the image.
// We don't know yet whose picture this is, so we have to use something in the pic itself.
function ResizeImage($newsizes, $source, $dest, $border=0, $watermark = false) {
	global $frameRGB, $jpgquality;
	global $MAILED_DIR, $BASEDIR, $LOGS;
	global $slide_size, $default_size, $thumb_size;
	global $msg, $error;

	
	$DEBUG = 0;
	$VERBOSE = true;
	// Don't check whether we're overwriting an existing file that might be newer,
	// i.e. the modification date might be newer.
	$OVERWRITE_NEWER = true;
	
	$sourcedate = 0;
	$destdate = 0;
	if (file_exists($dest)) {
		clearstatcache();
		$sourceinfo = stat($source);
		$destinfo = stat($dest);
		$sourcedate = $sourceinfo[10];
		$destdate = $destinfo[10];
		$VERBOSE && fp_error_log(__FUNCTION__.__LINE__.": Destination file exists: $dest", 3, FP_PICTURES_LOG);
	}
	
	$DEBUG && ($msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": source = $source<BR>");
	$DEBUG && ($msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": dest = $dest<BR>");
	$DEBUG && (file_exists ($source) && $msg .= __FUNCTION__.": Source exists<BR>");
	
	// If the source exists (system didn't mess up) and it doesn't already exist, 
	// OR the file is newer (an update), then resize it and copy it over.
	if (file_exists("$source") and ((!file_exists("$dest")) || ($OVERWRITE_NEWER or $sourcedate > $destdate))) 
	{
		$DEBUG && ($msg .= __FUNCTION__.": Start resizing.<BR>");
		if ( !($imgsize = GetLocalImageSize($source)) )
			return false;
		$width = $imgsize[0];
		$height = $imgsize[1];
		list ($new_width, $new_height) = CalcResizeImage ($width,$height,$newsizes[0],$newsizes[1]);

		
		($im = @ImageCreateFromJPEG($source)) || fp_error_log(__FUNCTION__.":".__LINE__.": ImageCreateFromJPEG failed", 3, FP_PICTURES_LOG);
		$DEBUG && ($im || $error .= __FUNCTION__.__LINE__.": ImageCreateFromJPEG failed<br>");
		if (!$im) return false;

		($new_im = ImageCreateTrueColor($new_width,$new_height)) || fp_error_log(__FUNCTION__.":".__LINE__.": ImageCreateTrueColor failed", 3, FP_PICTURES_LOG);
		$DEBUG && ($new_im || $error .= __FUNCTION__.__LINE__.": ImageCreateTrueColor failed<br>");
		if (!$new_im) return false;

		($success = ImageCopyResampled($new_im,$im,0,0,0,0,$new_width,$new_height,ImageSX($im),ImageSY($im))) || fp_error_log(__FUNCTION__.":".__LINE__.": ImageCopyResampled failed", 3, FP_PICTURES_LOG);
		$DEBUG && !$success && $error .= __FUNCTION__.__LINE__.": ImageCopyResampled failed<br>";
		if (!$success) return false;
		
		$DEBUG && fp_error_log(__FUNCTION__.__LINE__. ": Watermark = $watermark", 3, FP_PICTURES_LOG);

		if ($watermark) {
			$DEBUG && $msg .= "Watermark with '$watermark'<BR>";
			if ($watermark != true) {
				$iptc = FetchIPTCInfo ($source);
				$watermark = $iptc['CopyrightString'];
				$watermark || $watermark = $iptc['Byline'];
				$watermark || $watermark = $iptc['Credit'];
				$watermark = strip_symbols ($watermark);
			}
			($success = WatermarkImage ($new_im, $watermark)) || fp_error_log(__FUNCTION__.":".__LINE__.": WatermarkImage failed", 3, FP_PICTURES_LOG);
			$DEBUG && !$success && $error .= __FUNCTION__.__LINE__.": WatermarkImage failed<br>";
			if (!$success) return false;
		} else {
			$DEBUG && $error .= __FUNCTION__.__LINE__.": Do not watermark<br>";
		}			

		$framecolor = ImageColorAllocate ($new_im, $frameRGB[0], $frameRGB[1], $frameRGB[2]) ;
		$DEBUG && !$success && $error .= __FUNCTION__.__LINE__.": ImageColorAllocate failed<br>";
		if (!$success) return false;

		while ($border > 0) {
			($success = ImageRectangle ($new_im, $border-1, $border-1, ImageSX($new_im)-$border,	ImageSY($new_im)-$border, $framecolor)) || fp_error_log(__FUNCTION__.":".__LINE__.": ImageRectangle failed", 3, FP_PICTURES_LOG);
			$DEBUG && !$success && $error .= __FUNCTION__.__LINE__.": ImageRectangle failed<br>";
			if (!$success) return false;
			$border = $border - 1;
		}

		($success = ImageJPEG($new_im, $dest, $jpgquality)) || fp_error_log(__FUNCTION__.":".__LINE__.": ImageJPEG failed", 3, FP_PICTURES_LOG);
		$DEBUG && !$success && $error .= __FUNCTION__.__LINE__.": ImageJPEG failed<br>";
		if (!$success) return false;

		if ($result)
			//fp_error_log("Successfully wrote a version of $source as $dest", 3, FP_PICTURES_LOG);
		if ($DEBUG) {
			print "ResizeImage: $dest<BR>\n";
			if ($result)
				print "Yeah, it wrote it...<BR>\n";
		}
	}
	else 
	{
		if (!file_exists("$source")) {
			fp_error_log(__FUNCTION__.": MISSING FILE: $source", 3, FP_PICTURES_LOG);
			$DEBUG && ($error .= __FUNCTION__.": MISSING FILE: $source<br>");
		} else {
			fp_error_log(__FUNCTION__.": source is older than destination file!", 3, FP_PICTURES_LOG);
			$DEBUG && ($error .= __FUNCTION__.": Source is older than destination file<br>");
		}
		return false;
	}
	return true;
}


function ResizeIM () {
	global $MAILED_DIR, $PHOTOS_GALLERY, $SLIDES, $THUMBNAILS, $MATTED, $FRAMED, $ORIGINALS, $BASEDIR, $LOGS;
	global $PROCESSED_PHOTOS, $PROCESSED_SLIDES, $PROCESSED_THUMBNAILS, $PROCESSED_MATTED, $PROCESSED_FRAMED, $PROCESSED_ORIGINALS;
	global $default_size, $slide_size, $default_border, $thumb_size, $thumb_border;

}


// Calculate the new dimensions of an image.
// $width,$height are the width/height.
// The picture will be resized to fit inside $newW, $newH
// RETURN an array of the new (w,h)
function CalcResizeImage ($width,$height,$newW,$newH) {
	global $msg, $error;
	
	$DEBUG = false;

	$DEBUG && $error .= __FUNCTION__.": Resize ($width, $height) to fit inside of ($newW,$newH)<BR>";

	// normalize to correct height
	if ($height) {
		$width = ceil($width * ($newH / $height) );
		$height = $newH;
	
		// if width is too big, normalize down to correct width
		if ($width > $newW) {
			$height = ceil($height * ($newW / $width) );
			$width = $newW;
		}
	
		$DEBUG && $error .= __FUNCTION__.": RESULT: (w,h) $width, $height<BR><BR>";
	} else {
		$width =0;
		$height = 0;
	}
	return array ($width, $height);
}



//----
// Resize an original sized picture (or any picture) to the various different
// sizes: default, slide, thumbnail, and also make a matted version of the slide
// Put the results in the 'processed' directories.
// BE VERY SURE you've set permissions!
// IF $watermark is set, tell the resizer to watermark the image while it resizes.
function ResizeNewImages ($filename, $name, $watermark =	false) {
	global $MAILED_DIR, $PHOTOS_GALLERY, $SLIDES, $THUMBNAILS, $MATTED, $FRAMED, $ORIGINALS, $BASEDIR, $LOGS, $DEVELOPING;
	global $PROCESSED_PHOTOS, $PROCESSED_SLIDES, $PROCESSED_THUMBNAILS, $PROCESSED_MATTED, $PROCESSED_FRAMED, $PROCESSED_ORIGINALS;
	global $default_size, $slide_size, $default_border, $thumb_size, $thumb_border;
	global $msg, $error;
	
	$DEBUG = 0;
	$VERBOSE = 0;
	// RESIZE TO SLIDE SIZE
	$VERBOSE && fp_error_log(__FUNCTION__.": ($filename, $name, $watermark)", 3, FP_PICTURES_LOG);

	$DEBUG && $msg .= __FUNCTION__.":".__LINE__.": RESIZE $filename -> $name<BR>";
	$DEBUG && (file_exists ("$filename") || $error .= __LINE__.": Missing file: $filename<BR>");

	if (!file_exists($filename)) {
		fp_error_log("File Missing: $filename", 3, FP_PICTURES_LOG);
		return false;
	}
	
	$output = array();
	$response = "";
		
	if (FP_IMAGE_TOOL == "gd") {
		$VERBOSE && fp_error_log("Resizing $filename using GD", 3, FP_PICTURES_LOG);
		// SLIDES: RESIZE TO LARGE VIEWING SIZE (RESIZE ORIGINAL -> SLIDE)
$VERBOSE && fp_error_log("START!", 3, FP_PICTURES_LOG);
		$success = ResizeImage($slide_size, $filename, "$BASEDIR/$PROCESSED_SLIDES/$name", $default_border, $watermark);
$VERBOSE && fp_error_log("END!", 3, FP_PICTURES_LOG);
		$success || fp_error_log(__LINE__.": FAILED to resize to {$slide_size[0]}, $filename -> $name", 3, FP_PICTURES_LOG);
		if (!$success) return false;
		//chmod ("$BASEDIR/$PROCESSED_SLIDES/$name", 0644);
		$VERBOSE && fp_error_log("Finished resize to SLIDE size of $filename", 3, FP_PICTURES_LOG);
		
		// Do the next resizes based on the slide image...it's already much smaller, so this will save a lot of time!
		// GALLERY: RESIZE TO DEFAULT SIZE FOR GALLERY (RESIZE ORIGINAL -> GALLERY)
		FP_WATERMARK_GALLERY_PIX ? $w = $watermark : $w = "";
		$success = ResizeImage($default_size, "$filename", "$BASEDIR/$PROCESSED_PHOTOS/$name", $default_border, $w) || fp_error_log(__LINE__.": FAILED to resize to {$default_size[0]}, $filename -> $name", 3, FP_PICTURES_LOG);
		if (!$success) return false;
		//chmod ("$BASEDIR/$PROCESSED_PHOTOS/$name", 0644);
		$VERBOSE && fp_error_log("Finished resize to PHOTO size of $filename -> $name", 3, FP_PICTURES_LOG);
	
		// THUMBNAIL: RESIZE TO THUMBNAIL SIZE
		$success = ResizeImage($thumb_size, "$BASEDIR/$PROCESSED_SLIDES/$name", "$BASEDIR/$PROCESSED_THUMBNAILS/$name", $thumb_border, '') || fp_error_log(__LINE__.": FAILED to resize to {$thumb_size[0]}, $filename -> $name", 3, FP_PICTURES_LOG);;
		if (!$success) return false;
		//chmod ("$BASEDIR/$PROCESSED_THUMBNAILS/$name", 0644);
		$VERBOSE && fp_error_log("Finished resize to THUMBNAIL size of $filename -> $name", 3, FP_PICTURES_LOG);
	
		$VERBOSE && fp_error_log("Finished resizing $filename using GD!", 3, FP_PICTURES_LOG);
	}
	else	// using ImageMagick
	{
		// The "-size" value I use is double that of the final thumbnail so that resize will still generate an reasonable result.
		// I like the results from sharpen: 1.5x1+1.0+0.0
		// And, yes, starting with 2x file before sharpening and resizing
		// down does look noticeably better. Really.
		// Or not... It seems a bad ICC profile may have been the culprit!
		$mult = 1;
		$quality = " -quality " . FP_IMAGEMAGICK_QUALITY;
		// unshapr mask radius=.5, sigma=.5, amount=1.2, threshhold=0.05
		$sharpen = " -unsharp .5x.5+1.2+0.05";
		$basics = " -units 'pixelsperinch' -density '72x72'";
		$profile = " -intent Perceptual -profile '$BASEDIR/".FP_PROFILE_SRGB."'";
		// We can choose a different filter here.
		$filter = "";
		$loadsize = ceil($slide_size[0]*$mult) . "x" . ceil($slide_size[1]*$mult);
		
		if ($DEVELOPING) {
			fp_error_log("[ImageMagick] WARNING! Because 'developing' is set, we are using ".FP_IM_CONVERT." for ImageMagick! This may be wrong for non-local testing!", 3, FP_PICTURES_LOG);
		}
		
		$cmd = FP_IM_CONVERT . " '{$filename}[{$loadsize}>]' $basics $quality $profile $sharpen $filter ";

		//Write slide pic
		$mult == 1 
			? $cmd .= " -write '$BASEDIR/$PROCESSED_SLIDES/$name'"
			: $cmd .= " -resize '{$slide_size[0]}x{$slide_size[1]}>' -write '$BASEDIR/$PROCESSED_SLIDES/$name'";

		// Write gallery pic. Sharpen a litle bit first
		$sharpen = " -unsharp .5x.4+1.0+0.05";
		$cmd .= " -resize '{$default_size[0]}x{$default_size[1]}>' $sharpen -write '$BASEDIR/$PROCESSED_PHOTOS/$name'";
		
		// Write thumbnail. Use IM Thumbnail command to strip profile, for size saving
		$cmd .= " -thumbnail '{$thumb_size[0]}x{$thumb_size[1]}>' '$BASEDIR/$PROCESSED_THUMBNAILS/$name'";
		
		exec ($cmd, $output, $response);
		$VERBOSE && fp_error_log("[ImageMagick] Response: $response\n".join("\n",$output), 3, FP_PICTURES_LOG);
		$response && $im_error += 1;
		$response && fp_error_log("[ImageMagick] Error #{$response} while processing slides/gallery/thumbs for $name:\n$cmd", 3, FP_PICTURES_LOG);
		
		
		// Watermark the Slide
		$fontfile = $BASEDIR . "/" . FP_WATERMARK_FONT_DIR. "/".FP_WATERMARK_FONT_NAME;
		
		if ($watermark) {
			$cmd = FP_IM_CONVERT." -size 400x400 xc:none -fill grey \
			-font '$fontfile' -pointsize ".FP_WATERMARK_FONT_SIZE." -gravity center \
			-annotate 315x315+20+20 '$watermark' \
			miff:- |\
			".FP_IM_COMPOSITE." -tile -dissolve ".floor(FP_WATERMARK_OPACITY/1.27)."% - '$BASEDIR/$PROCESSED_SLIDES/$name'  '$BASEDIR/$PROCESSED_SLIDES/$name'";
			exec ($cmd, $output, $response);
			$response && $im_error += 1;
			$response && fp_error_log("[ImageMagick] Error #{$response} while watermarking slides for $name", 3, FP_PICTURES_LOG);
			$VERBOSE && fp_error_log("[ImageMagick] Response: $response\n".join("\n",$output), 3, FP_PICTURES_LOG);
		}
		

		// Watermark the Gallery pic
		if ($watermark && FP_WATERMARK_GALLERY_PIX) {
			$cmd = FP_IM_CONVERT." -size 150x150 xc:none -fill grey \
			-font '$fontfile' -pointsize 10 -gravity center \
			-annotate 315x315+20+20 '$watermark' \
			miff:- |\
			".FP_IM_COMPOSITE." -tile -dissolve 25% - '$BASEDIR/$PROCESSED_PHOTOS/$name'  '$BASEDIR/$PROCESSED_PHOTOS/$name'";
			exec ($cmd, $output, $response);
			$response && $im_error += 1;
			$response && fp_error_log("[ImageMagick] Error #{$response} while watermarking gallery pix $name", 3, FP_PICTURES_LOG);
			$VERBOSE && fp_error_log("[ImageMagick] Response: $response\n".join("\n",$output), 3, FP_PICTURES_LOG);
		}
		
		//print __FUNCTION__.__FILE__.": $cmd<BR>";
		$VERBOSE && fp_error_log("[ImageMagick] Created slide/gallery/thumbnail pictures of $filename -> $name", 3, FP_PICTURES_LOG);
	}

	if ($im_error) {
		return false;
	} else {
		return true;
	}
}


//----
// Sharpen Image
function SharpenImage ($image) {
	$sharpenMatrix = array(array(-1,-1,-1), array(-1,16,-1), array(-1,-1,-1));
	$divisor = 8;
	$offset = 0;
	imageconvolution($image, $sharpenMatrix, $divisor, $offset);
	return $image;
}

// ------
// Mark an image resource with text (x,y) is for text location.
function WatermarkImage (&$im, $text = "", $xoffset = 0, $yoffset = 0) {
	global $BASEDIR;
	global $error, $msg;
	
	$DEBUG = 0;
	
	$result = true;
	imagealphablending($im,true); 
	$border = 20;
	
	$DEBUG && ($error .= __FUNCTION__.": Watermark start<BR>");
	$DEBUG && fp_error_log(__FUNCTION__.__LINE__. ": Watermark a file with $text", 3, FP_PICTURES_LOG);

	if ($text) {

		// Convert UTF to Latin2, which is needed for the TT fonts

		$text = html_entity_decode( htmlentities ($text, ENT_NOQUOTES, 'UTF-8'), ENT_NOQUOTES, 'ISO-8859-1');

		$DEBUG && ($error .= __FUNCTION__.": Watermark image with $text<BR>");

		$color = imagecolorallocatealpha ($im, 250, 250, 250, FP_WATERMARK_OPACITY);

		$width = ImageSX ($im);
		$height = ImageSY ($im);
		
		// Font size with built-in fonts
		//$font = FP_WATERMARK_FONT_SIZE;
		
		$textwidth = imagefontwidth($font) * strlen ($text);
		$textheight = imagefontheight($font);
		
		$xcount = ceil(($width / $textwidth)/2);
		$DEBUG && ($error .= __FUNCTION__.": xcount = $xcount<BR>");
		$ycount = 3;
		
		$xstep = floor( ( ( ($width-(2*$border)) / $xcount) - $textwidth)/2);
		$ystep = floor( ( ( ($height-(2*$border)) / $ycount) - $textheight)/2);
		
		$xoffset || $xoffset = ceil(($width-(2*$border)) / $xcount);
		$yoffset || $yoffset = ceil(($height-(2*$border)) / $ycount);

		$DEBUG && ($error .= __FUNCTION__.": height=$height, width=$width, textwidth = $textwidth<BR>");
		for ($y=$border;$y<$height-$border;$y=$y+$yoffset) {
			for ($x=$border;$x<$width-$textwidth-$border;$x=$x+$xoffset) {
				// light text, but transparent
				//$result = imagestring ($im, $font, $x + $xstep, $y + $ystep, $text, $color);
				
				
				// Font size using TrueType fonts
				$size = FP_WATERMARK_FONT_SIZE;
				$fontfile = $BASEDIR . "/" . FP_WATERMARK_FONT_DIR. "/".FP_WATERMARK_FONT_NAME;
				$angle = 45;
				
				$result = imagettftext ( $im , $size , $angle , $x + $xstep,  $y + $ystep , $color , $fontfile , $text );	

				
				$DEBUG && ($error .= __FUNCTION__.": Watermarked image : $x, $y, $text<BR>");
			}
		}
		
		$result ||	fp_error_log(__FUNCTION__.": Failed to watermark image", 3, FP_PICTURES_LOG);
	}
	return $result;
}


// ------
// Imagemagick version: Mark an image resource with text (x,y) is for text location.
function WatermarkImageIM ($text = "", $xoffset = 0, $yoffset = 0) {
	global $BASEDIR;
	global $error, $msg;
	
	$DEBUG = false;
	
	$result = true;
	$border = 20;
	
	$DEBUG && ($error .= __FUNCTION__.": Watermark start<BR>");
	
	if ($text) {

		// Convert UTF to Latin2, which is needed for the TT fonts
		$text = html_entity_decode( htmlentities ($text, ENT_NOQUOTES, 'UTF-8'), ENT_NOQUOTES, 'ISO-8859-1');

		$cmd = "-gravity northwest -annotate 315x315 '$text'  \\";
		$cmd .= "-gravity north -annotate 315x315 '$text'  \\";
		$cmd .= "-gravity northeast -annotate 315x315 '$text'  \\";
		$cmd .= "-gravity west -annotate 315x315 '$text'  \\";
		$cmd .= "-gravity center -annotate 315x315 '$text'  \\";
		$cmd .= "-gravity east -annotate 315x315 '$text'  \\";
		$cmd .= "-gravity southwest -annotate 315x315 '$text'  \\";
		$cmd .= "-gravity south -annotate 315x315 '$text'  \\";
		$cmd .= "-gravity southeast -annotate 315x315 '$text'  \\";
	}
	return $result;
}


//------
// Get IPTC Info from a JPEG for entry in the database
// return an array with data of most common IPTC values...more than I store in the DB!
// Unless the character set is specified, we assume the IPTC info is Latin1 (CP1252)
// UTF=-8 = "ESCï¿½%ï¿½G",  and Latin1/ISO-8859-1 ("Latin" or "ESCï¿½.ï¿½A") where
// the escape code is "\x1B"

/*
0	ApplicationRecordVersion	int16u:	 
3	ObjectTypeReference	string[3,67]	 
4	ObjectAttributeReference	string[4,68]+	 
5	ObjectName	string[0,64]	 
7	EditStatus	string[0,64]	 
8	EditorialUpdate	digits[2]	01 = Additional language
10	Urgency	digits[1]	0 = 0 (reserved) 
1 = 1 (most urgent) 
2 = 2 
3 = 3 
4 = 4 
5 = 5 (normal urgency) 
6 = 6 
7 = 7 
8 = 8 (least urgent) 
9 = 9 (user-defined priority)
12	SubjectReference	string[13,236]+	 
15	Category	string[0,3]	 
20	SupplementalCategories	string[0,32]+	 
22	FixtureIdentifier	string[0,32]	 
25	Keywords	string[0,64]+	 
26	ContentLocationCode	string[3]+	 
27	ContentLocationName	string[0,64]+	 
30	ReleaseDate	digits[8]	 
35	ReleaseTime	string[11]	 
37	ExpirationDate	digits[8]	 
38	ExpirationTime	string[11]	 
40	SpecialInstructions	string[0,256]	 
42	ActionAdvised	digits[2]	01 = Object Kill 
02 = Object Replace 
03 = Ojbect Append 
04 = Object Reference
45	ReferenceService	string[0,10]+	 
47	ReferenceDate	digits[8]+	 
50	ReferenceNumber	digits[8]+	 
55	DateCreated	digits[8]	 
60	TimeCreated	string[11]	 
62	DigitalCreationDate	digits[8]	 
63	DigitalCreationTime	string[11]	 
65	OriginatingProgram	string[0,32]	 
70	ProgramVersion	string[0,10]	 
75	ObjectCycle	string[1]	'a' = Morning 
'b' = Both Morning and Evening 
'p' = Evening
80	By-line	string[0,32]+	 
85	By-lineTitle	string[0,32]+	 
90	City	string[0,32]	 
92	Sub-location	string[0,32]	 
95	Province-State	string[0,32]	 
100	Country-PrimaryLocationCode	string[3]	 
101	Country-PrimaryLocationName	string[0,64]	 
103	OriginalTransmissionReference	string[0,32]	 
105	Headline	string[0,256]	 
110	Credit	string[0,32]	 
115	Source	string[0,32]	 
116	CopyrightNotice	string[0,128]	 
118	Contact	string[0,128]+	 
120	Caption-Abstract	string[0,2000]	 
121	LocalCaption	string[0,256]	(I haven't found a reference for the format of tags 121, 184-188 and 225-232, so I have just make them writable as strings with reasonable length. Beware that if this is wrong, other utilities won't be able to read these tags as written by ExifTool)
122	Writer-Editor	string[0,32]+	 
125	RasterizedCaption	string[7360]	 
130	ImageType	string[2]	 
131	ImageOrientation	string[1]	'L' = Landscape 
'P' = Portrait 
'S' = Square
135	LanguageIdentifier	string[2,3]	 
150	AudioType	string[2]	'0T' = Text Only 
'1A' = Mono Actuality 
'1C' = Mono Question and Answer Session 
'1M' = Mono Music 
'1Q' = Mono Response to a Question 
'1R' = Mono Raw Sound 
'1S' = Mono Scener 
'1V' = Mono Voicer 
'1W' = Mono Wrap 
'2A' = Stereo Actuality 
'2C' = Stereo Question and Answer Session 
'2M' = Stereo Music 
'2Q' = Stereo Response to a Question 
'2R' = Stereo Raw Sound 
'2S' = Stereo Scener 
'2V' = Stereo Voicer 
'2W' = Stereo Wrap
151	AudioSamplingRate	digits[6]	 
152	AudioSamplingResolution	digits[2]	 
153	AudioDuration	digits[6]	 
154	AudioOutcue	string[0,64]	 
184	JobID	string[0,64]	 
185	MasterDocumentID	string[0,256]	 
186	ShortDocumentID	string[0,64]	 
187	UniqueDocumentID	string[0,128]	 
188	OwnerID	string[0,128]	 
200	ObjectPreviewFileFormat	int16u	0 = No ObjectData 
1 = IPTC-NAA Digital Newsphoto Parameter Record 
2 = IPTC7901 Recommended Message Format 
3 = Tagged Image File Format (Adobe/Aldus Image data) 
4 = Illustrator (Adobe Graphics data) 
5 = AppleSingle (Apple Computer Inc) 
6 = NAA 89-3 (ANPA 1312) 
7 = MacBinary II 
8 = IPTC Unstructured Character Oriented File Format (UCOFF) 
9 = United Press International ANPA 1312 variant 
10 = United Press International Down-Load Message 
11 = JPEG File Interchange (JFIF) 
12 = Photo-CD Image-Pac (Eastman Kodak) 
13 = Bit Mapped Graphics File [.BMP] (Microsoft) 
14 = Digital Audio File [.WAV] (Microsoft & Creative Labs) 
15 = Audio plus Moving Video [.AVI] (Microsoft) 
16 = PC DOS/Windows Executable Files [.COM][.EXE] 
17 = Compressed Binary File [.ZIP] (PKWare Inc) 
18 = Audio Interchange File Format AIFF (Apple Computer Inc) 
19 = RIFF Wave (Microsoft Corporation) 
20 = Freehand (Macromedia/Aldus) 
21 = Hypertext Markup Language [.HTML] (The Internet Society) 
22 = MPEG 2 Audio Layer 2 (Musicom), ISO/IEC 
23 = MPEG 2 Audio Layer 3, ISO/IEC 
24 = Portable Document File [.PDF] Adobe 
25 = News Industry Text Format (NITF) 
26 = Tape Archive [.TAR] 
27 = Tidningarnas Telegrambyra NITF version (TTNITF DTD) 
28 = Ritzaus Bureau NITF version (RBNITF DTD) 
29 = Corel Draw [.CDR]
201	ObjectPreviewFileVersion	int16u	 
202	ObjectPreviewData	string[0,256000]	 
221	Prefs	string[0,64]	(PhotoMechanic preferences)
225	ClassifyState	string[0,64]	 
228	SimilarityIndex	string[0,32]	 
230	DocumentNotes	string[0,1024]	 
231	DocumentHistory	string[0,256]	 
232	ExifCameraInfo	string[0,4096]	 
255	CatalogSets	string[0,256]+	(written by iView MediaPro)


*/
function FetchIPTCInfo ($filename) {
	global $BASEDIR, $LOGS;
	global $msg, $error;
	
	$DEBUG = 0;
	$DEBUG && fp_error_log("FetchIPTCInfo: $fk = ".$fileinfo[$fk], 3, FP_PICTURES_LOG);

	//error_reporting	 (E_PARSE); // New syntax for PHP 3/4
	if (!file_exists($filename)) {
		$DEBUG && $msg .= basename(__FILE__) .":".__FUNCTION__ .":".__LINE__ .": "."File $filename not found<BR>\n";
		return array();
	}
	
	$info = array ();
	$fileinfo = array();

	$size = GetLocalImageSize ($filename, $info); // get the iptc block into $info

	if (isset($info["APP13"])) {
		$iptc = iptcparse($info["APP13"]);
		if ($DEBUG) {
			print __FUNCTION__ .":".__LINE__ .":<BR>";
			var_dump ($iptc);
		}
		if (is_array($iptc)) {
			$fileinfo['CodedCharacterSet'] = $iptc["1#090"][0];
			$fileinfo['ObjectName'] = $iptc["2#005"][0];
			$fileinfo['EditStatus'] = $iptc["2#007"][0];
			$fileinfo['Priority'] = $iptc["2#010"][0];
			$fileinfo['Category'] = $iptc["2#015"][0];
			$fileinfo['SupplementalCategory'] = $iptc["2#020"][0];
			$fileinfo['FixtureIdentifier'] = $iptc["2#022"][0];
			$fileinfo['Keywords'] = $iptc["2#025"][0];
			$fileinfo['ReleaseDate'] = $iptc["2#030"][0];
			$fileinfo['ReleaseTime'] = $iptc["2#035"][0];
			$fileinfo['SpecialInstructions'] = $iptc["2#040"][0];
			$fileinfo['ReferenceService'] = $iptc["2#045"][0];
			$fileinfo['ReferenceDate'] = $iptc["2#047"][0];
			$fileinfo['ReferenceNumber'] = $iptc["2#050"][0];
			$fileinfo['CreatedDate'] = $iptc["2#055"][0];
			$fileinfo['CreatedTime'] = $iptc["2#060"][0];
			$fileinfo['OriginatingProgram'] = $iptc["2#065"][0];
			$fileinfo['ProgramVersion'] = $iptc["2#070"][0];
			$fileinfo['ObjectCycle'] = $iptc["2#075"][0];
			$fileinfo['Byline'] = $iptc["2#080"][0];
			$fileinfo['BylineTitle'] = $iptc["2#085"][0];
			$fileinfo['City'] = $iptc["2#090"][0];
			$fileinfo['ProvinceState'] = $iptc["2#095"][0];
				$fileinfo['State'] = $fileinfo['ProvinceState'];
			$fileinfo['CountryCode'] = $iptc["2#100"][0];
			$fileinfo['Country'] = $iptc["2#101"][0];
			$fileinfo['OriginalTransmissionReference'] = $iptc["2#103"][0];
			$fileinfo['Headline'] = $iptc["2#105"][0];
			$fileinfo['Title'] = $fileinfo['Title'];
			$fileinfo['Credit'] = $iptc["2#110"][0];
			$fileinfo['Source'] = $iptc["2#115"][0];
			$fileinfo['CopyrightString'] = $iptc["2#116"][0];
				$fileinfo['CopyrightNotice'] = $iptc["2#116"][0];
			$fileinfo['Caption'] = $iptc["2#120"][0];
			$fileinfo['JobID'] = $iptc["2#184"][0];
			$fileinfo['LocalCaption'] = $iptc["2#121"][0];
		} else {
		$DEBUG && $msg .= basename(__FILE__) .":".__FUNCTION__ .":".__LINE__ .": "."FetchIPTCInfo: the APP13 info isn't an array.<BR>\n";
		}
	} else {
		$DEBUG && $msg .= basename(__FILE__) .":".__FUNCTION__ .":".__LINE__ .": "."FetchIPTCInfo: Error: APP13 is not set for file = $filename<BR>\n";
	}
	$DEBUG && $msg .= basename(__FILE__) .":".__FUNCTION__ .":".__LINE__ .": "."File info:<BR>\n".ArrayToTable($fileinfo);
	
	if ($DEBUG) {
		foreach (array_keys($fileinfo) as $fk) {
			$fk && fp_error_log("FetchIPTCInfo: $fk = ".$fileinfo[$fk], 3, FP_PICTURES_LOG);
		}
	}
	
	$utf8 = "\1B%G";
	$latin1 = "\1B%A";
	
	// Character sets:
	// Default if nothing is indicated is Mac OS Roman, which doesn't destroy UTF-8.
	// Windows users might be in trouble, but we can't reliably detect windows char sets.
	
	switch ($fileinfo['CodedCharacterSet']) {
		case $latin1 :
			$fileinfo = array_map("utf8_encode", $fileinfo);
			break;
		case $utf8 :
			break;
		default :
			$fileinfo = array_map("macToUTF8", $fileinfo);
			break;
	}
	
	array_walk ($fileinfo, 'trimME');
	$DEBUG && $error .= __FUNCTION__.__LINE__.ArrayToTable ($fileinfo);
	return $fileinfo;
}


// convert Mac Roman to UTF-8 character set
function macToUTF8 ($x) {
	return iconv("macintosh", "UTF-8", $x);
}

// CalcPrintSizes ($width, $height)
// $width & $height are in pixels
//
// Given the image size (read using GetLocalImageSize)
// we determine the sizes we allow for printing
// the image. This way, we don't sell lousy quality prints.
//
// We return an array of acceptable print sizes.
// The keys are the size to show, width x height
// The values are only the larger size, which is the index in the pricing database
// for each entry.
//
// If empty array, the image is to small to print well.
//
// If the image is marked as "fixed size", we use that as the only print size.
//
// * $makesummary is true/false. If true, return a string of all available sizes.
// * $maxonly means return ONLY the largest size. Pictopia needs this,
// because it only allows us to offer the largest, and smaller, sizes,
// NOT an order for a given size.
// * $myprintsizes is an array of sizes, e.g. 10,20,30. If this is not supplied,
// we use the default sizes set up in the config file.

function CalcPrintSizes ($width, $height, $makesummary=false, $maxonly=false, $myprintsizes=null, $fixedSizeImage = false) {
	global $PRINTSIZES, $PRINTDPI, $BASEDIR;
	global $PRINTDPI;
	global $msg, $error;

	$DEBUG = false;
	
	
	$orientation = ImageOrientation ($width, $height);
	if ($orientation == "landscape") {
		$bigside = $width;
		$smallside = $height;
	} else {
		$bigside = $height;
		$smallside = $width;
	}
	
	// get ratio of greater size to less, to calculate lesser later
	// so if we have 10", and the ration of 8/10, we know to offer an 8x10.
	$ratio = $smallside/$bigside;
	
	// max picture size allowed, based on pixel count at $PRINTDPI per inch.
	$maxsize = ($bigside / $PRINTDPI);
	$sizes = array ();
	
	sort ($myprintsizes);

	foreach ($myprintsizes as $size) {
		if ($size <= $maxsize) {
			$otherside = round($size * $ratio);
			if ($orientation == "landscape") {
				$sizes[$size."x".$otherside] = $size;
			} else {
				$sizes[$otherside."x".$size] = $size;
			}
		}
	}

	if ($maxonly && $sizes)
		$sizes = array(array_pop($sizes));
	
	if ($makesummary) { 
		//$sizes = array_merge ($goodsizes, $sizes);
		$sizes = join (", ", array_keys($sizes));
		//$goodsizes || $goodsizes = array ("None");
	}
	
	$DEBUG && print __FUNCTION__.": ".print_r ($sizes);
	return $sizes;
}



// Calculate orientation of an image: landscape or portrait.
function ImageOrientation ($width, $height) {
	// get the greater of width or height
	if ($width > $height) {
		$orientation = "landscape";
	} else {
		$orientation = "portrait";
	}
	return $orientation;
}

// Calculate orientation of an image: landscape or portrait.
function ImageGreaterSide ($width, $height) {
	// get the greater of width or height
	if ($width > $height) {
		return $width;
	}
	return $height;
}


// Return height,width but with the larger side first.
// This allows for calculations which depend on the greater side.
function ImageGreaterSideFirst ($width, $height) {
	// get the greater of width or height
	if ($width > $height) {
		return array ("bigside"=>$width, "smallside"=>$height);
	}
	return array ("bigside"=>$height, "smallside"=>$width);
}

// $maxonly is useful for finding if an image is big enough to sell.
function CalcPrintSizesFromImageID ($ID, $printsizes, $maxonly = false) {
	global $PHOTOS_GALLERY, $THUMBNAILS, $MATTED, $ORIGINALS, $BASEDIR;
	global $PRINTDPI;
	global $msg, $error;

	$sizes = array ();
	if ($printsizes) {
		$image = FetchImage ($ID);
		if ($image) {
			$params = DecodeArrayFromDB ($image['Params']);
			$fWidth = GetParam ($params, FP_PARAM_IMAGE_FIXED_WIDTH);
			$fHeight = GetParam ($params, FP_PARAM_IMAGE_FIXED_HEIGHT);
			if (GetParam ($params, FP_PARAM_IMAGE_IS_FIXED_SIZE) && $fWidth && $fHeight) {
				$orientation = "landscape";
				($fWidth > $fHeight)
				? $s = $fWidth
				: $s = $fHeight;

				// If the fixed size is bigger than any of the priceset print sizes, then we don't have
				// a size for the print! It requires a larger size to get its pricing!
				if ($s <= max($printsizes)) {
					$printsizes = array ( $s => $s );
					$width =  ceil($fWidth * $PRINTDPI);
					$height = ceil($fHeight * $PRINTDPI);
				} else {
					return array("error"=>"nosize");
				}
				// Since it is a fixed sized pic, we must have a for-sale size larger to get pricing!
				// Normally, it's the opposite — we need the image size > the for-sale size
			} else {
				$imgsize = GetImageSizeByID($ID);
				$width = $imgsize['width_print'];
				$height = $imgsize['height_print'];
			}

			$sizes = CalcPrintSizes ($width, $height, false, $maxonly, $printsizes);
		}
	}
	return $sizes;
}


// function GetPaperSizes ($width, $height)
// Calculate the art border around the printed image for an array of image sizes
// $printsizes is an array of largest-side sizes, e.g. 10,14,20 means max side, 10", 14", and 20"
// This can be used to calculate paper size, once the actual image size is known
function GetArtBorders ($printsizes) {
	global $msg, $error;
	$DEBUG = 0;
	
	$borders = array();
	foreach ($printsizes as $size) {
		$border = ArtBorderWidthFromSide ($size);
		$borders[$size] = $border;
	}
	return $borders;
}


function ArtBorderWidthFromSize ($width, $height) {
	global $FP_FRAMESHOP_ART_BORDER;
	global $msg, $error;

	if ($width > $height) {
		$artborder_width = $FP_FRAMESHOP_ART_BORDER * $width;
	} else {
		$artborder_width = $FP_FRAMESHOP_ART_BORDER * $height;
	}
	
	$artborder_width = Math.ceil ($artborder_width);
	return $artborder_width;
}


// Round up to nearest integer
function ArtBorderWidthFromSide ($side) {
	global $FP_FRAMESHOP_ART_BORDER;

	$w = ceil($FP_FRAMESHOP_ART_BORDER * $side);
	return $w;
}


function ImageFilesExist ($url) {
	$filename = FP_DIR_ORIGINAL_IMAGES . "/" . $url;
	$r = file_exists ($filename);
// 	if (!$r) {
// 		echo __FUNCTION__.": $url DOES NOT EXIST, called by ".getCallingFunction()."<BR>";
// 	}
	return $r;
}


// Get the dimensions of an image.
// The image might be offsite: the original file might be stored
// elsewhere, accessible or not. If so, the 'offsite' flag will
// be set, and we get the size from the database record.
//
// Return array (width, height)

function GetImageSizeByID ($ID) {
	global $PHOTOS_GALLERY, $THUMBNAILS, $MATTED, $ORIGINALS, $BASEDIR;
	global $BASEDIR, $LOGS;
	global $PRINTDPI;
	global $msg, $error;
	
	$d = array ();
	$image = FetchImage ($ID);
	$params = DecodeArrayFromDB ($image['Params']);
	
	if ($image) {
		// Get size from local file
		$filename = $ORIGINALS . "/" . $image['URL'];
		if (ImageFilesExist ($image['URL'])) {
			$imgsize = GetLocalImageSize($filename);
			$d = array ("width"=>$imgsize[0], "height"=>$imgsize[1]);
		} else {
			$error .= __FUNCTION__.": Image record #{$ID} is missing its image files! ({$image['URL']}). Called by ".getCallingFunction()."<br>";
			fp_error_log(__FUNCTION__.": Image record #{$ID} is missing its image files ({$image['URL']})! Called by ".getCallingFunction()."<BR>", 3, FP_ERROR_LOG );
			// Cannot called delete! This will recurse, since delete ends up calling GetImageBySize!
			//EditTable ('delete', DB_IMAGES, $ID, null, true); // force delete
			return array();
		}
		
		// If there is a fized size entered for the image (in UNITS), use that as the size
		if (GetParam ($params, FP_PARAM_IMAGE_IS_FIXED_SIZE) && GetParam ($params, FP_PARAM_IMAGE_FIXED_WIDTH) && GetParam ($params, FP_PARAM_IMAGE_FIXED_HEIGHT)) {
			// Get size from database (height/width are stored in Params)
			$d['width_print'] = ceil(GetParam ($params, FP_PARAM_IMAGE_FIXED_WIDTH) * $PRINTDPI);
			$d['height_print'] = ceil(GetParam ($params, FP_PARAM_IMAGE_FIXED_HEIGHT) * $PRINTDPI);
			$d['width_fixed'] =  GetParam ($params, FP_PARAM_IMAGE_FIXED_WIDTH);
			$d['height_fixed'] =  GetParam ($params, FP_PARAM_IMAGE_FIXED_HEIGHT);
		} else {
			$d['width_fixed'] =  null;
			$d['height_fixed'] =  null;
			// If there is an offsite image, get that size, too.
			// Let's be sure it isn't zero x zero, by accident
			if (GetParam ($params, FP_PARAM_IMAGE_OFFSITE) && GetParam ($params, FP_PARAM_IMAGE_WIDTH) && GetParam ($params, FP_PARAM_IMAGE_HEIGHT)) {
				// Get size from database (height/width are stored in Params)
				$d['width_print'] = GetParam ($params, FP_PARAM_IMAGE_WIDTH);
				$d['height_print'] = GetParam ($params, FP_PARAM_IMAGE_HEIGHT);
			} else {
				$d['width_print'] = $d['width'];
				$d['height_print'] = $d['height'];
			}
		}
	}
	return $d;
}


// Get image size from a file
// I'm using this instead of just calling GetLocalImageSize
// so we have more control over where "local" files are stored in the future.
function GetLocalImageSize ($path, &$info=null) {
	if (file_exists($path)) {
		return GetImageSize ($path, $info);
	} else {
		return null;
	}
}



// =========== AUDIO =============
//------
// Process a single uploaded image NOT for the gallery, but for a user portrait or gallery icon
// The name of the picture will be changed to $filename.
// The picture will be stored in the user's image directory
// The HTLM field is "userspecialfile" not "userfile"!
// We resize to 
function ProcessUploadedSlideshowAudio ($fp_user, $projectID, $fieldName = "userfile") {
	global $SLIDESHOW_AUDIO;
	global $BASEDIR, $MAILED_DIR, $LOGS;
	global $default_size, $slide_size, $default_border, $thumb_size, $thumb_border;
	global $msg, $error;

	$DEBUG = 0;

// FP_SUBDIR_SLIDESHOW_AUDIO

// Don't bother if nothing submitted!
	if ($_FILES[$fieldName]['tmp_name']) {

		$images = array();
		$fp_user || (isset ($_SESSION['fp_user']) ? $fp_user = $_SESSION['fp_user'] : $fp_user = NULL);
		$artistinfo = FetchArtist ($fp_user);
	
		$filename = $_FILES[$fieldName]['name'];
		$extension = findexts ($filename);
		$f = $filename; // store for roll/frame use
		$filetype = $_FILES[$fieldName]['type'];
		$filesize = $_FILES[$fieldName]['size'];	//	The size, in bytes, of the uploaded file. 
		$tempname = $_FILES[$fieldName]['tmp_name'];	//	Upload temporary name
	
		$DEBUG && print "fieldname=$fieldName<BR>";
		$DEBUG && var_dump ($_FILES[$fieldName]);
	
		if ($filetype != "audio/mpeg") {
			$error .= "Error: You can only upload MPEG audio.";
			return false;
		}
	
		if (isset($_REQUEST['userfileTag'])) {
			$tag = $_REQUEST['userfileTag'];
		
			switch ($tag) {
				case "project_audio" :
					$newname = strtolower(GetSlideshowAudioFilename ($projectID) );
				break;
				default :
					$newname = $filename;
				break;
			}
		}
		$userDir = GetUserDirectory ($fp_user);
		$newpath = $userDir.$SLIDESHOW_AUDIO;
		file_exists($newpath) || MakeDirectoryTree ($newpath);
		$newpath .= "/$newname";

		// make sure the directories exist
		// MakeDirectoryTree (dirname($newpath));
		
		if (copy ($tempname, $newpath)) {
			unlinkb ($tempname);
			chmod ($newpath, 0644);
			return $filename;
		}
	}
	return null;	
}

// LEGACY- FROM WHEN AUDIO FOR SLIDESHOWS WAS A SPECIAL UPLOAD
function DeleteSlideshowAudioFile ($projectID) {
	global $SLIDESHOW_AUDIO;
	global $BASEDIR, $MAILED_DIR, $LOGS;

	if (!$projectID)
		return false;
	$filename = GetSlideshowAudioFilename ($projectID);
	file_exists ("$BASEDIR/$SLIDESHOW_AUDIO/$filename") && unlinkb ("$BASEDIR/$SLIDESHOW_AUDIO/$filename");
}

// Get the filename for an audio clip --- it's based on the project ID
function GetSlideshowAudioPath ($projectID) {
	$project = new FPProject($projectID);
	$userDir = GetUserDirectory($project->owner);
	$path = $userDir.FP_SUBDIR_USER_FILES;
	return $path;

	$filename = "audio-" . $projectID . ".mp3";
	return $filename;
}

// ====================================================================================


// GetProjectSlideshowAudio ($projectID)
// Given the project ID, look in the audio folder to see if there's a file
// Return the filename.
function GetProjectSlideshowAudio ($projectID) {
	global $SLIDESHOW_AUDIO;
	global $BASEDIR;
	global $msg, $error;

	$project = new FPProject($projectID);
	return $project->audio;
}


// Set the slideshow duration based on the length of the audio
// VERSION: mp3_id3_utils
function GetAudioFileDuration ($filename) {
	global $msg, $error;

// Initialize getID3 engine
	$getID3 = new getID3;
	// Analyze file and store returned data in $ThisFileInfo
	$ThisFileInfo = $getID3->analyze($filename);
	$length = ceil(@$ThisFileInfo['playtime_seconds']);
	return $length;
}



// Set the slideshow duration based on the length of the audio
// VERSION: mp3_id3_utils, does NOT work well.
function GetSlideShowDurationMP3UTILS ($projectID) {
	global $SLIDESHOW_AUDIO;
	global $BASEDIR;
	global $msg, $error;

	$DEBUG = 0;
	
	// Do we have audio?
	$audio = GetProjectSlideshowAudio ($projectID);
	$DEBUG && $msg .= __FUNCTION__.": projectID = $projectID, audio=$audio<BR>";
	if (!$audio)
		return 0;
	$id3 = mp3_id( "$BASEDIR/$SLIDESHOW_AUDIO/$audio" );
	$length = $id3['length_sec'];
	
	$DEBUG && $msg .= __FUNCTION__.": ". ArrayToTable ($id3);
	return $length;
}

// PROCESSING LOCK FUNCTIONS
// Set a lock, clear it, check if it's set.

function is_processing_image ($filename) {
	global $TMPDIR, $BASEDIR, $LOGS;
	global $PROCESSED_ORIGINALS;
	global $msg, $error;
	
	$VERBOSE = false;
	$toolong = 3 * 60 * 60; // three minutes
	
	$filename = basename ($filename);
	
	$lockfile = "$TMPDIR/insert_local_images_lock.txt";
	file_exists ($lockfile) ? $locks = unserialize (file_get_contents($lockfile)) : $locks = array ();
	
	// Recover from crashes which set lock
	// If file is missing, remove lock. If more than 5 minutes have passed, removed lock.
	if (isset($locks[$filename]) && ( (time() - $locks['time'] > $toolong) or (!file_exists ("$BASEDIR/$PROCESSED_ORIGINALS/$filename")) ) ) {
		if (!file_exists ("$BASEDIR/$PROCESSED_ORIGINALS/$filename")) {
			fp_error_log(__FUNCTION__.": $filename does not exist, remove lock.", 3, FP_PICTURES_LOG);
		} else {
			fp_error_log(__FUNCTION__.": $filename was locked too long.", 3, FP_PICTURES_LOG);
		}
		clear_lock_for_file ($filename);	
	}

	if (isset($locks[$filename])) {
		$VERBOSE && fp_error_log(__FUNCTION__.": $filename is locked", 3, FP_PICTURES_LOG);
		return true;
	} else {
		$VERBOSE && fp_error_log(__FUNCTION__.": $filename is unlocked", 3, FP_PICTURES_LOG);	
		return false;
	}
}

function set_lock_for_file ($filename) {
	global $TMPDIR, $BASEDIR, $LOGS;
	
	$VERBOSE = false;

	$filename = basename ($filename);
	$VERBOSE && fp_error_log(__FUNCTION__.": Lock $filename", 3, FP_PICTURES_LOG);

	$lockfile = "$TMPDIR/insert_local_images_lock.txt";
	file_exists ($lockfile) ? $locks = unserialize (file_get_contents($lockfile)) : $locks = array ();
	
	$locks[$filename] = $filename;
	$locks['time'] = time ();
	
	file_put_contents ($lockfile, serialize($locks)) || fp_error_log(__FUNCTION__.": The lock-file $lockfile is not writable.", 3, FP_ERROR_LOG );
}

function clear_lock_for_file ($filename) {
	global $TMPDIR, $BASEDIR, $LOGS;

	$VERBOSE = false;

	$filename = basename ($filename);

	$VERBOSE && fp_error_log(__FUNCTION__.": Unlock $filename", 3, FP_PICTURES_LOG);

	$lockfile = "$TMPDIR/insert_local_images_lock.txt";
	file_exists ($lockfile) ? $locks = unserialize (file_get_contents($lockfile)) : $locks = array ();
	
	if (isset($locks[$filename]))
		unset($locks[$filename]);
	$locks['time'] = time ();	// time of unlocking

	file_put_contents ($lockfile, serialize($locks)) || fp_error_log(__FUNCTION__.": The lock-file $lockfile is not writable.", 3, FP_ERROR_LOG );
}

// How much storage does an image take, including original and previews?
// If it's not stored on site, it will take less, of course!
// The size is returned in MB, rounded up
function image_storage_size ($ID) {
	global $msg, $error;
	global $PHOTOS_GALLERY, $THUMBNAILS, $ORIGINALS, $SLIDES;
	global $PICTURE_DIRECTORIES;

	$DEBUG = 0;
	
	$image = FetchImage ($ID);
	$storage = 0;
	foreach ($PICTURE_DIRECTORIES as $dir) {
		if (file_exists ("$dir/{$image['URL']}")) {
			$storage += filesize("$dir/{$image['URL']}");
			$DEBUG && $msg .= __FUNCTION__.": $dir/{$image['URL']} = ". filesize("$dir/{$image['URL']}") . "<br>";
		}
	}
	$DEBUG && $msg .= __FUNCTION__. ": Storage used in bytes: $storage<BR>";
	//$storage = ceil ($storage/1000000) / 10;
	return $storage;
}



/*
Clean out Processed dir of any files:
$PROCESSEDDIR
$PROCESSED_PHOTOS
$PROCESSED_SLIDES
$PROCESSED_THUMBNAILS
$PROCESSED_MATTED
$PROCESSED_FRAMED
$PROCESSED_ORIGINALS
*/
function CleanProcessedDirs ($filename = null) {
	global $BASEDIR;
	global $error, $msg;
	
	
	$dirs = array (FP_DIR_PROCESSED_PHOTOS, FP_DIR_PROCESSED_SLIDES, FP_DIR_PROCESSED_THUMBNAILS, FP_DIR_PROCESSED_MATTED, FP_DIR_PROCESSED_FRAMED, FP_DIR_PROCESSED_ORIGINALS);
	$filename2 = "";
	
	if (!$filename) {
		$filename = "*.jpg";
		$filename2 = "*.JPG";
	}
	
	foreach ($dirs as $dir) {
		$filename && rfr ("$BASEDIR/$dir/",$filename);
		$filename2 && rfr ("$BASEDIR/$dir/",$filename2);
	}
	
}


// ------------------------------

function rfr($path,$match){
	$report = "";
	static $deld = 0, $dsize = 0;
	$dirs = glob($path."*");
	$files = glob($path.$match);
	foreach($files as $file){
		$report .= "Deleting $file<BR>";
		if(is_file($file)){
			$dsize += filesize($file);
		unlinkb($file);
		$deld++;
		}
	}
	foreach($dirs as $dir){
		if(is_dir($dir)){
		$dir = basename($dir) . "/";
		rfr($path.$dir,$match);
		}
	}
	return "$report<br>\n$deld files deleted with a total size of $dsize bytes";
}



function DeleteUnreferencedImages($delete=false) {
	global $msg, $error;
	
	$query = "SELECT * FROM ".DB_IMAGES." ORDER BY ID";
	$result = mysql_query($query);
	
	while ($record = mysql_fetch_array($result, MYSQL_ASSOC)) {
		$ID = $record['ID'];
		$pix[$record['URL']] = $record['URL'];
	}
	
	// Get list of all images
	$images = array ();
	foreach (glob (FP_DIR_ORIGINAL_IMAGES . "/" . "*.jpg") as $filename) {
		$p = substr(basename($filename), 0,6);
		if ($p != "artist" && $p != "group" && $p != "missin" ) {
			$images[] = basename($filename);
		}
	}
	
	// compare lists
	foreach ($images as $image) {
		if (!isset($pix[$image])) {
			$list .= "Image $image is not in use<BR>";
			if ($delete)
				DeleteAllPicVersions ($image);
		} else {
			//$list .= "<B>*Image $image is in use</b><BR>";
		}
	}
	return $list;
}


// DeleteMissingPix
// Delete any image entries in the database which don't have picture files in the photos directory
function DeleteMissingPix ($delete = false) {
	global $msg, $error;
	
	$output = "";
	$query = "SELECT * FROM ".DB_IMAGES." ORDER BY ID";
	$result = mysql_query($query);
	
	while ($record = mysql_fetch_array($result, MYSQL_ASSOC)) {
	
		$ID = $record['ID'];
	
		$url = $record['URL'];
		$missing =0;
		
		file_exists("$BASEDIR/$PHOTOS_GALLERY/$url") || $missing++;
		$missing && $line .= "MISSING $BASEDIR/$PHOTOS_GALLERY$url <BR>" ;
	 
		file_exists("$BASEDIR/$SLIDES/$url") || $missing++;
		$missing && $line .= "MISSING $BASEDIR/$SLIDES$url <BR>" ;
	 
		file_exists("$BASEDIR/$THUMBNAILS/$url") || $missing++;
		$missing && $line .= "MISSING $BASEDIR/$THUMBNAILS$url <BR>" ;
	 
	// 	file_exists("$BASEDIR/$MATTED/$url") || $missing++;
	// 	$missing && $line .= "MISSING $BASEDIR/$MATTED$url <BR>" ;
	//  
	// 	file_exists("$BASEDIR/$FRAMED/$url") || $missing++;
	// 	$missing && $line .= "MISSING $BASEDIR/$FRAMED$url <BR>" ;
	 
		file_exists("$BASEDIR/$ORIGINALS/$url") || $missing++;
		$missing && $line .= "MISSING $BASEDIR/$ORIGINALS$url <BR>" ;

		$missing && $output .= "Picture #{$ID} ({$record['Title']}) : $line<BR>\n";
		if ($missing) {

			print "OH SHIT";
		}
		if ($missing) {
			if ($delete) {
				DeleteRowByID( DB_IMAGES, $ID);
				// parts?
				$a = array ();
				
				$query = "SELECT * FROM ".DB_PARTS." WHERE ( PartTable = \"".DB_IMAGES."\" AND PartID = \"$ID\" ) ORDER BY ID";
				$parts = mysql_query($query);
				while ($part = mysql_fetch_array($parts, MYSQL_ASSOC)) {
					$a[] =$part['ID'];
					DeleteRowByID( DB_PARTS, $part['ID'] );
				}
				$output .= "<i>Parts referring to this image</i> : ";
				$output = '<font color="#CC0000">' . join (",", $a) . '</font>';
			}
		} else {
			//$output = "<font color=\"#999999\">{$output}All files accounted for.</font>";
		}
		$msg .= $output;
	}
}

?>