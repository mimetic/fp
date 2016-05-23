<?php

class FPProject
{
// ---------------------
// Variables
// ---------------------
	/**
	 * The mysqli database connection
	 */
	public $mysqli_connection;

	public $info;
	public $members;
	public $ID, $title, $description, $owner, $theme, $params, $is_solo, $audio;

// ---------------------	
// Functions
// ---------------------
function __construct ($mysqli_connection, $ID = null) {
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;
	
	$this->mysqli_connection = $mysqli_connection;
	
	$DEBUG = 0;
	
	// Constructor class; if we create a new group object with an
	// ID parameter, what we do is fetch existing data about that
	// group.
	// $fp_user is current system user, whose admin level determines what he sees.
	if (isset ($fp_user)) {
		$this->fp_user = $fp_user;
	} else {
		isset($_SESSION['fp_user']) ? $this->fp_user = $_SESSION['fp_user'] : $this->fp_user = FP_ADMINISTRATOR;
	}
	$this->accesslevel = FetchAccessLevel ($fp_user);

	if ($this->ProjectExists ($ID)) {
		$this->ProjectInfo ($ID);
		$this->ID = $ID;
		$this->title || $this->title = "Untitled";
		$this->owner = $this->info['ArtistID'];
		$this->params = DecodeArrayFromDB($this->info['Params']);
		$this->audio = $this->info['Audio'];
		$this->IsSolo();

	} else {
		// this is a new, blank group
		$this->ID = null;
		$this->group = array();
		$this->title = "Untitled Group";
		$this->owner = $this->fp_user;
		$this->is_solo = 1;
		$this->audio = null;
	}
}

function members () {
	print "<hr>Members function:";
	print $this->var;
	print "<hr>";
}

function ProjectExists ($ID = null) {
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;
	
	if (!$ID)
		return false;
	//$ID || $ID = $this->ID;
	$query = "SELECT ID FROM $PROJECTS WHERE ID = '$ID'";
	$result = mysqli_query ($this->mysqli_connection, $query);
	if ($result && mysqli_num_rows ($result) > 0)
		return true;
	return false;
}

function iterateVisible() {
	echo "FPGroup::iterateVisible:\n";
	foreach($this as $key => $value) {
		print "$key => $value\n";
	}
}

function FetchGroup ($ID  = null) {
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;

	isset($ID) || $ID = $this->ID;
	$this->members = $this->MemberList ( ($ID != FP_ADMINISTRATOR) );	// do not show member $ID in the list unless he is the Admin
	$this->project = $this->ProjectList ($ID);
	$this->ProjectInfo ($ID);
	$this->group = array (	"info"		=>	$this->info,
							"members"	=>	$this->members,
							"projects"	=>	$this->projects
						);
	return $this->group;
}

// Get the theme and variation
// Returned as an array $mytheme = ('theme'=>$theme, 'variation'=>$variation);
// $theme is the CODE of the theme, you must look up the actual directory in $THEMES_DIRS_ARRAY
// (this allows us to have meaningful names for themes)
// $variation is the name of the variation, a hard-coded name, e.g. "light" 
// (this means names for variations can't change, or someone's look&feel will break)
function GetProjectThemeID() {
	global $THEME_DIRS_ARR, $Themes, $error, $msg;

	$themeid = GetParam ($this->params, FP_PARAM_GALLERY_THEME);
	
	if ($themeid === "0") {
		// Get theme from the group
		$group = new FPGroup ($this->mysqli_connection, $this->info['GroupID']);
		$themeid = $group->theme;
	}
	//$THEME_DIRS_ARR[$this->theme] ? $myTheme = $THEME_DIRS_ARR[$this->theme] : $myTheme = DEFAULT_THEME;
	$this->theme = themeid;
	
	return ($themeid);
}

// PictureList
// Get the list of pictures showing in a project
// return pixlist = array (id=>image_record)
function PictureList ($ID=null) {
	global $msg, $error;
		
	$DEBUG = 0;
	$pixlist=array();
	$ID || $ID	= $this->ID;
	// Here we are...get all Image parts for a project!
	$query = "SELECT DISTINCT *, ".DB_IMAGES.".ID AS ID, ".DB_IMAGES.".ProjectID as ProjectID from ".DB_IMAGES.", ".DB_PARTS." WHERE ".DB_PARTS.".ProjectID = '$ID' AND ".DB_PARTS.".PartTable = '".DB_IMAGES."' AND ".DB_IMAGES.".ID = ".DB_PARTS.".PartID ORDER BY ".DB_PARTS.".OrderInProject";
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.":$query<BR>";
	$result = mysqli_query ($this->mysqli_connection, $query);
	while ($image = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
		$pixlist[$image['ID']] = $image;
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": PixList (".__FUNCTION__ .":".__LINE__.") ".$image['ID'] . ":" . $image['Title'] . "<BR>";
	}
	//$pixlist && ksort ($pixlist); // ordered by OrderInProject
	$this->images = $pixlist;
	return $pixlist;
}

// Get info for a project
function ProjectInfo ($ID  = null) {
	// Fetch array of all members in the project
	// array [ID] = member record
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;

	isset($ID) || $ID = $this->ID;
	$this->ID = $ID;
	$this->info = FetchRowsByValue ($PROJECTS, "*", 'ID', $ID, "", TRUE);
	$this->title = $this->info['Title'];
	$this->owner = $this->info['ArtistID'];
	$this->params = DecodeArrayFromDB($this->info['Params']);
	$this->theme = $this->GetProjectThemeID();
	return $this->info;
}

function OwnerInfo ($ID	 = null) {
	$ID || $ID = $this->ID;
	if ($ID) {
		$this->ProjectInfo ();
		$record = FetchArtist($this->info['ArtistID']);
		return $record;
	} else {
		return array ();
	}
}

function OwnerID ($ID  = null) {
	isset($ID) || $ID = $this->ID;
	$this->ProjectInfo ();
	return $this->info['ArtistID'];
}

// Return an array of the artists who are part of the project
// These are chosen by looking at the parts of the project, and adding in the
// owner, even if he has no parts in the project
// If $hideuser is set, remove project owner!
// Array (ID=>artist_record)
function MemberList ( $hideowner = FALSE) {
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;
	

	$DEBUG = 0;
	
	$fp_user = $_SESSION['fp_user'];
	$members = array();
	$projectID = $this->ID;
	$members = array ();
	
	if ($projectID) {
		$project = $this->info;
		$ArtistID = $this->owner;
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": FetchProjectArtists: Current Artist= $ArtistID<BR>";
		
		// Get current artist IDs
		$query = "SELECT DISTINCT $ARTISTS.* FROM $PARTS, $ARTISTS WHERE ($ARTISTS.ID = '$ArtistID') OR ($PARTS.ProjectID = $projectID AND $PARTS.PartTable = 'Artists' AND $PARTS.PartID = $ARTISTS.ID)";
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": $query<BR>";

		$result = mysqli_query ($this->mysqli_connection, $query);
		while ($row = mysqli_fetch_array($result)) {
			$members[$row['ID']] = $row;
			$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": Other artists are " . $row['ArtistID'] . ":" . $row['Fullname'] . "<BR>";
		}
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": FetchProjectArtists: members" . ArrayToTable (array_keys($members)) . "<BR>";
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": *** END FetchProjectArtists<BR>";
	 } else {
		// NO projectID, so it's a blank, new project
		// Use the $fp_user (the creator) as the artist
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": No {fp:project} ID! <BR>";
		$ArtistID = $fp_user;
		$members[$row['ID']] = FetchArtist ($ArtistID);
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": Other artists are " . $row['ID'] . ":" . $row['Fullname'] . "<BR>";
	}

	if ($this->fp_user != FP_ADMINISTRATOR && $hideowner)
		unset ($members[$this->owner]);

	// System Admin is not part of anything
	if (isset($members[FP_ADMINISTRATOR]))
		unset($members[FP_ADMINISTRATOR]);
	$this->members = $members;
	return $this->members;
}

// Return an array of ID's of the artists who are part of the project.
// Uses $this->MemberList to get the list.
function MemberIDList ( $hideowner = FALSE) {
	$memberlist = $this->MemberList($hideowner);
	$idlist = array_keys($memberlist);
	return $idlist;
}

// Create an HTML multi list of all artists, with those in the group checked.
// for the user to create a new record...so an ID must exist
// This gets the HTML for the artist list
function MemberListHTML ($hideuser = FALSE) {
	global $msg, $error;
	
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.":" . ": Preparing artistlist for the form, for fp_user=$userID<BR>";
	$members = $this->MemberList( );
	$checked = array_keys($members);

	$accessLevel == FP_ADMINISTRATOR_LEVEL ? $where = "ID != ".FP_ADMINISTRATOR_LEVEL : $where = "ID != '$this->owner' AND ID != ".FP_ADMINISTRATOR_LEVEL;
	$order = "Lastname";
	$all = FetchAllArtistArrays ($where, $order);
	$values = array();
	foreach ($all as $member) {
		$values[$member['ID']] = $member['Fullname'];
	}
	if ($hideuser)
		unset ($values[$this->owner]);	// don't show owner in the list
	$HTMLlist = OptionListFromArray ($values, "artistlist[]", $checked);
	return $HTMLlist;
	}


// Given a project ID, return an array of the artists who are part of the project
// These are chosen by looking at the parts of the project, and adding in the
// owner, even if he has no parts in the project
// If $hideuser is set, remove project owner!
// Array (ID=>fullname)
function MemberNames ( $hideowner = FALSE) {
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;


	$DEBUG = 0;
	
	$fp_user = $_SESSION['fp_user'];
	$members = array();
	$projectID = $this->ID;
	$memberNames = array ();
	
	if ($projectID) {
		$project = $this->info;
		$ArtistID = $this->owner;
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": FetchProjectArtists: Current Artist= $ArtistID<BR>";
		
		// Get current artist IDs
		$query = "SELECT DISTINCT $ARTISTS.ID AS ArtistID, CONCAT_WS(' ', Firstname, Lastname) AS Fullname FROM $PARTS, $ARTISTS WHERE ($ARTISTS.ID = '$ArtistID') OR ($PARTS.ProjectID = $projectID AND $PARTS.PartTable = 'Artists' AND $PARTS.PartID = $ARTISTS.ID)";
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": $query<BR>";

		$result = mysqli_query ($this->mysqli_connection, $query);
		while ($row = mysqli_fetch_array($result)) {
			$ID = $row['ArtistID'];
			$memberNames[$ID] = $row['Fullname'];
			$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": Other artists are " . $row['ArtistID'] . ":" . $row['Fullname'] . "<BR>";
		}
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": FetchProjectArtists: members" . ArrayToTable ($memberNames) . "<BR>";
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": *** END FetchProjectArtists<BR>";
	 } else {
		// NO projectID, so it's a blank, new project
		// Use the $fp_user (the creator) as the artist
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": No {fp:project} ID! <BR>";
		$ArtistID = $fp_user;
		$artist = FetchArtist ($ArtistID);
		$memberNames[$row['ArtistID']] = $artist['Fullname'];
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": Other artists are " . $row['ArtistID'] . ":" . $row['Fullname'] . "<BR>";
	}
	
	// Remove the owner if flag set
	if ($hideowner)
		unset($memberNames[$this->owner]);

	// System Admin is not part of anything
	if (isset($memberNames[FP_ADMINISTRATOR]))
		unset($memberNames[FP_ADMINISTRATOR]);
	$this->memberNames = $memberNames;
	return $this->memberNames;
}


// ====================================
// Basic functions for manipulating a project

function ExpelMembers ($members = array (), $targetgroupID = PUBLIC_GROUP_ID) {
// Move members from current group to target, default is Public
	$this->MoveMembers ($members, $targetgroupID);
}

function MoveMembers ($members = array (), $targetgroupID = null, $sourcegroupID = null) {
	// Move members from current group to another
	// The default target group is the Public group
	// Moving an artist out also means removing him from any projects in the group
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;
	
	isset($targetgroupID) || $targetgroupID = $this->ID;
	isset($sourcegroupID) || $sourcegroupID = $this->ID;
	
	$projects = $this->ProjectList();

	if ($targetgroupID != $sourcegroupID) { 
		if (! is_array($members)) { $members = array ($members); }
		foreach ($members as $artistID) {
			// remove artist from all projects in the group
			foreach ($projects as $project) {
				RemoveArtistFromProject ($project['ID'], $artistID);
			}
			
			// don't mess with the member's default group.
			// UpdateRecord( $ARTISTS, $artistID, array ('GroupID' => $targetgroupID ));
			// Can't add if other group won't accept
			if ($this->AddMembersToGroup ($artistID, $targetgroupID))			
				$this->DeleteMembersFromGroup ($artistID, $sourcegroupID);
		}
	}
}

// Add an artist, or an array of artists to a project
// $members : record ID (integer) or array of ID's
// $newgroupID : empty = $this->ID ; or ID of the project to change
// Don't use this to set the owner!
// NEVER add FP_ADMINISTRATOR to a project. Admin is never part of a project but always has access.
function AddMembersToProject ($members = array (), $newgroupID = null) {
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;
	
	$DEBUG = 0;
	isset($newgroupID) || $newgroupID = $this->ID;
	
		
	if (! is_array($members)) { $members = array ($members); }
	foreach ($members as $artistID) {
		if ($artistID != FP_ADMINISTRATOR) {
			//MemberCount(false) checks for all members, not just showing members
			if ($this->MemberCount(false) < MAX_GROUP_SIZE) {
				// check to see if artist is already in this group
				// This version checks if the artist is the owner of the group, too.
				$query = "select distinct $PARTS.ID from $PARTS, $GROUPS where ($PARTS.ArtistID = '$artistID' AND $PARTS.PartID = '$newgroupID' AND $PARTS.PartTable = '$GROUPS') OR ($GROUPS.ID = '$newgroupID' AND $GROUPS.ArtistID = '$artistID')";
				$DEBUG && $error .= __FUNCTION__.__LINE__.": $query<BR>";
				$result = mysqli_query ($this->mysqli_connection, $query);
				if (mysqli_num_rows ($result) == 0) {
					$pairs = array ('ArtistID'	=>	$artistID,
									'PartTable' =>	$GROUPS,
									'PartID'	=>	$newgroupID,
									'ProjectID' =>	null
									);
					EditTable ('insert', $PARTS, '', $pairs);
					$DEBUG && $error .= __FUNCTION__.__LINE__.": Added $artistID to the group<BR>";
				} else {
					$DEBUG && $error .= __FUNCTION__.__LINE__.": Artist $artistID is already in the group.<BR>";
					//$error .= "AddMembersToGroup: $artistID is already part of group $newgroupID<BR>";
				}
			} else {
				$error .= "Error: This group is full. Only ".MAX_GROUP_SIZE." members are allowed.<br>";
				return FALSE;
			}
		}
	}
	return TRUE;
}


// ------------------------------------------
// UpdateImageParts ($ID, $vars)
// The $vars will include an array of chosen images for a project belonging
// to the owner of the project. The $ArtistID field
// will be of the owner of the project.
// The array is an ordered list of image IDs.
// It also includes the Artists for the project!!!
function SetProjectImages ($imageList) {
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;
	
	$DEBUG = 0;
	$DEBUG && $msg .= __FUNCTION__ .":".__LINE__.": Beginning UpdateImageParts:<BR>";
	
	$projectID = $this->ID;
	
	if (!is_array($imageList) && count($imageList))
		return FALSE;

	if (! $projectID)
		return FALSE;

	isset($_SESSION['fp_user']) ? $fp_user = $_SESSION['fp_user'] : $fp_user = 1;
	
	//$artistIDs = array_keys($this->MemberList());
	
	// Remove all current images
	$this->RemoveImages();
	
	// Add new images
	$this->AddImages($imageList);
	
	return TRUE;
}

// ------------------------------------------
// UpdateImageParts ($ID, $vars)
// The $vars will include an array of chosen images for a project belonging
// to the owner of the project. The $ArtistID field
// will be of the owner of the project.
// The array is a list of IDs.
// The images listed in $vars come in special variables, not just an array! 
// It also includes the Artists for the project!!!
function UpdateImageParts ($vars, $projectID=null) {
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;
	
	$DEBUG = 0;
	$DEBUG && $msg .= __FUNCTION__ .":".__LINE__.": Beginning UpdateImageParts:<BR>";
	
	$projectID || $projectID = $this->ID;
	
	// Just in case, to avoid errors, check if there's an imagelist. If NOT, don't do anything!
	// This might be called from a form that has no imagelist, e.g. the upload form might end up here
	// because it came through the generic handler 
	// I think an empty list will still be set.
	if (!isset($vars['updateimagelist']))
		return FALSE;

	if (! $projectID)
		return FALSE;

	isset($_SESSION['fp_user']) ? $fp_user = $_SESSION['fp_user'] : $fp_user = 1;
	
	$artistIDs = array_keys($this->MemberList());
	
	$DEBUG && $msg .= __FUNCTION__ .":".__LINE__.": VARS".ArrayToTable ($vars);
	$DEBUG && $msg .= __FUNCTION__ .":".__LINE__.": ARTISTS in project".ArrayToTable ($artistIDs);

	// imagelist comes as a series of lists, newright1, newright2, etc.
	// newleftx is a list of pictures NOT in a project
	// The numbering of the newleft/newright vars, e.g. newleft2, is arbitrary. Each left/right
	// set is for an artist in the project.
	// We can't just delete and add parts, since they probably have ordering in them which 
	// will be lost if we delete the parts.
	//
	// The values of the images include "pic_" because ID's in CSS cannot be numbers.
	// So, we use id="pic_123" and now we have to strip "pic_".
	
	// Build the list of images to add to the project
	$x = 1;
	$images_to_add = array ();
	while (isset($vars["addedRight".$x])) {
		if ($vars["addedRight$x"])
			$images_to_add = array_merge ($images_to_add, explode ("|", str_replace ("pic_", "", $vars["addedRight$x"])));
		$DEBUG && $msg .= __FUNCTION__.": images_to_add : ". ArrayToTable($images_to_add);
		$x++;
	}

	// Build the list of all images to remove from the project
	$x = 1;
	$images_to_remove = array ();
	while (isset($vars["removedRight".$x])) {
		if ($vars["removedRight$x"])
			$images_to_remove = array_merge ($images_to_remove, explode ("|", str_replace ("pic_", "", $vars["removedRight$x"])));
		$DEBUG && $msg .= __FUNCTION__.": images_to_remove : ". ArrayToTable($images_to_remove);
		$x++;
	}

	// string list of all images in project for the mysql update
	$images_to_remove = join (",", $images_to_remove);
	
	// delete part records for rejected images
	$query = "DELETE from Parts where ProjectID = $projectID AND PartTable = '".DB_IMAGES."' and PartID in ($images_to_remove)";
	$DEBUG && $msg .= __FUNCTION__.": all images in project = $images_in_project<BR>";
	$result = mysqli_query ($this->mysqli_connection, $query);

	// insert new images not already in the project
	foreach ($images_to_add as $ID) {
		$DEBUG && $msg .= __FUNCTION__ .":".__LINE__.": adding image ID=$ID, artistID = $ArtistID, Project=$projectID<BR>";
		// Need the artist ID for ease of sort, less searching later. We assume this is set.
		$image = FetchImage ($ID);
		$ArtistID = $image['ArtistID'];
		// be sure the artist for this image is actually in the project
		// it could be the image was passed on, but the artist unselected.
		if (in_array($ArtistID, $artistIDs)) {
			$pairs = array (	'ProjectID'		=> $projectID,
							'ArtistID'		=> $ArtistID,
							'PartTable'		=> $IMAGES,
							'PartID'			=> $ID,
							'OrderInProject'	=> 0
							);
			$newID = EditTable ('insert', $PARTS, '', $pairs);
			$DEBUG && $msg .= __FUNCTION__ .":".__LINE__.": UpdateImageParts: adding image ID=$ID, artistID = $ArtistID, Project=$projectID<BR>";
		} else {
			$DEBUG && $msg .= __FUNCTION__ .":".__LINE__.": HEY! the image=$ID is here but not the artist=$ArtistID<BR>";
		}
	}
	
	return TRUE;
}


// IsSolo()
// Return true if this project is a solo group, with only one member. We exclude the owner and FP_Administrator.
// If there are other members, then this is NOT a solo group (unless we're checking only who has pictures)
// This is a common request, and we can do this much faster this way, rather than checking
// for number of members in the group.
// $withPictures : true, then count only artists who have pictures showing; false, count all members of the project
function IsSolo ($withPictures = false) {
	global $msg, $error;
	
	$DEBUG = 0;

	// If this is a single user system, then there's only one user!
	
	$artist = new FPUser ($this->mysqli_connection, $this->info['ArtistID']);
	if (($artist->UserType == FP_SINGLE_GALLERY_SINGLE_USER) || ($artist->UserType == FP_MULTI_GALLERY_SINGLE_USER)) {
		$this->showingArtistID = $this->OwnerID ();
		$this->is_solo = 1;
		$DEBUG && print __FUNCTION__.__LINE__.": This is a single user system, owner is {$this->OwnerID}<BR>";
		return (1);
	}
	
	$DEBUG && $msg .= __FUNCTION__.__LINE__.": This is a multi-user system<BR>";

	if ($withPictures) {
		// Want all members of a group who have pictures showing in that group
		// The group is solo if the result is 1
		$set = "distinct ID";
		$table = "Artists";
		$where = "ID in (select distinct ArtistID from ".DB_PARTS." where PartTable = '".DB_IMAGES."' and ProjectID in (select distinct ID from ".DB_PROJECTS." where GroupID = '{$this->ID}'))";
	} else {
		// Otherwise, look for artists in the group not including owner and admin
		// The group is solo if the result is zero
		$set = "distinct ".DB_PARTS.".ArtistID as ID";
		$table = DB_PARTS;
		$where = "(PartTable = \"".DB_GROUPS."\" AND PartID = {$this->ID} AND ArtistID != {$this->owner} AND ArtistID != ".FP_ADMINISTRATOR.") LIMIT 1";
	}
	
	$query = "select $set FROM $table WHERE $where";
	$result = mysqli_query ($this->mysqli_connection, $query);
	
	$DEBUG && $msg .= "$query<BR>";
	
	if ($result) {
		$count = mysqli_num_rows ($result);
		
		if ($withPictures) {
			$DEBUG && $msg .= "* Count group members with pictures.<BR>";
			$this->is_solo = ($count == 1);
			if ($count > 0) {
				$x = mysqli_fetch_array($result, MYSQLI_ASSOC);
				$DEBUG && $msg .= __FUNCTION__. __LINE__.ArrayToTable ($x);
				$this->showingArtistID = $x['ID'];
			} else {
				$this->showingArtistID = $this->owner;
			}
		} else {
			$this->is_solo = ($count == 0);
			$this->showingArtistID = $this->owner;
		}
	} else {
		$this->is_solo = true;
		$this->showingArtistID = $this->owner;
	}
		
	
	$DEBUG && $msg .= "with_pictures = $withPictures<BR>";
	$DEBUG && $msg .= "Count = $count<BR>";
	$DEBUG && $msg .= "Solo = {$this->is_solo}<BR>";
	$DEBUG && $msg .= "Showing Artist is {$this->showingArtistID}<BR><hr>";
	//$DEBUG && print __FUNCTION__.":<div style='color:black;'>$msg</div>";
	
	return $this->is_solo;
}

// Return the number of members in a group NOT including the System Administrator
// unless $rejectSysAdmin is false.
// If $withPictures = all, then we check for artists with any project
function MemberCount ( $withPictures = false, $rejectSysAdmin = true ) {
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;
	
	$DEBUG = 0;

	
	// If this is a single user system, then there's only one user!
	$artist = new FPUser ($this->mysqli_connection, $this->info['ArtistID']);
	if ($artist->UserType == FP_SINGLE_GALLERY_SINGLE_USER || $artist->UserType == FP_MULTI_GALLERY_SINGLE_USER) {
		$this->showingArtistID = $this->OwnerID ();
		return (1);
	}
	
	// Otherwise...
	$ID = $this->ID;
	$table = "$IMAGES, $PARTS";
	$set = "DISTINCT $PARTS.ArtistID as ID";
	
	
	if ($rejectSysAdmin) {
		$rejectSysAdmin = "($PARTS.ArtistID != 1) AND ";
	} else {
		$rejectSysAdmin = "";
	}
	
	
	switch ($withPictures) {
		case FP_ACTIVE :
			// Get all projects
			$projects = "NOT ($PROJECTS.Slides <=> 1) AND $PROJECTS.ArtistID != " . FP_ADMINISTRATOR . " AND $PROJECTS.GroupID = {$this->ID} AND $PROJECTS.ID = $PARTS.ProjectID AND $PARTS.PartTable = 'Images'";
	
			$where = "select ID from Artists where ID in (select ArtistID from Parts where PartTable = \"{$IMAGES}\" and ProjectID in (select ID from Projects where GroupID = '{$this->ID}')";
			break;
			
		case FP_FEATURED :
			// Get featured projects
			$projects = "NOT ($PROJECTS.Slides <=> 1) AND $PROJECTS.ArtistID != " . FP_ADMINISTRATOR . " AND $PROJECTS.GroupID = {$this->ID} AND $PROJECTS.ID = $PARTS.ProjectID AND $PARTS.PartTable = 'Images'";
			$projects = GetFeaturedWhere ($projects);
	
			$where = "($PARTS.PartTable = '$IMAGES') AND $PARTS.ProjectID IN (SELECT DISTINCT $PROJECTS.ID FROM $PROJECTS, $PARTS WHERE ( $projects ))";
			break;
		
		// This covers $withPictures == false
		default :
			$where = "(PartTable = \"".DB_GROUPS."\" AND $PARTS.PartID = {$this->ID} AND $PARTS.ArtistID != {$this->owner} AND $PARTS.ArtistID != ".FP_ADMINISTRATOR.")";
			// of course, every group has an owner
			$count += 1;
			$DEBUG && $msg .= _FUNCTION__.": checking for all members<BR>";
	}
	

	$where = "WHERE $rejectSysAdmin $where";
	$query = "select $set FROM $table \n$where";
	$result = mysqli_query ($this->mysqli_connection, $query);
	$result ? $count = mysqli_num_rows ($result) : $count = 0;

	!$result && fp_error_log(__FUNCTION__.": *** Error: Bad sql query: $query", 3, FP_ERROR_LOG );

	$DEBUG &&  $msg .= __FUNCTION__.": This group {$this->ID} has $count members<BR>";

	if ($count == 1) {
		//If there's only one artist showing, (solo gallery), we can't assume the gallery owner is that artist. It could be a Public gallery, owned by admin, but only one person is showing at this time.
		$showinfo = mysqli_fetch_array ($result);
		$artistID = $showinfo['ID'];
		$this->showingArtistID = $artistID;
	} else {
		$this->showingArtistID = $this->OwnerID ();
	}

	$DEBUG && $msg .= "<BR>active only = $activeonly; count = $count<BR>";
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.":" . $query;
	$DEBUG && $error .= __FUNCTION__.__LINE__.": showingArtistID = {$this->showingArtistID}<BR>";
	
	return ( $count );
}

function DeleteMembersFromProject ($members = array (), $fromgroup = null) {
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;
	
	$DEBUG = false;
	$DEBUG && $error .= __FUNCTION__.": BEGIN<BR>";
	
	isset($fromgroup) || $fromgroup = $this->ID;
	if (! is_array($members)) { $members = array ($members); }
	$DEBUG && $error .= __FUNCTION__.": ".ArrayToTable ($members);
	foreach ($members as $artistID) {
		$DEBUG && $error .= __FUNCTION__.": "."ArtistID = '$artistID' AND PartID = '$fromgroup' AND PartTable = '$GROUPS'<BR>";
		DeleteRow( $PARTS, "ArtistID = '$artistID' AND PartID = '$fromgroup' AND PartTable = '$GROUPS'" );
	}
}

// Change the owner of a group
// If the owner changes, the ArtistID of the group is set to owner,
// the current owner becomes a member,
// and we remove the Parts record which assigns the user to the group.
function SetOwner ($newOwnerID) {
	global $msg, $error;
	
	$DEBUG = 0;
	
	if (!$newOwnerID)
		return false;
	if ($newOwnerID == $this->owner)
		return true;
	$DEBUG && $msg .= __FUNCTION__.": Set $newOwnerID to replace {$this->owner} as owner.<br>";
	// Must change owner first, AddMembers won't add the current owner (as set in the database)
	$oldOwner = $this->owner;
	$success = UpdateRecord (DB_GROUPS, $this->ID, array ('ArtistID' => $newOwnerID));					
	if ($success) {
		$this->DeleteMembersFromGroup ($newOwnerID, $this->ID);
		$this->AddMembers($this->owner);
		$this->owner = $newOwnerID;
	}
	return $success;
}

// Add any users in the $vars['artistlist'] to the group.
function UpdateMembers ($vars) {
	global $msg, $error;
	
	$DEBUG = 0;
	
	isset($_SESSION['fp_user']) ? $this->fp_user = $_SESSION['fp_user'] : $this->fp_user = FP_ADMINISTRATOR;
	
	// no param passed, do nothing.
	if (! isset ($this->ID))
		return FALSE;

	if (! isset($vars['artistlist']))
		return FALSE;

	// fetch the updated Member list
	$newMemberList = $vars['artistlist'];
	$DEBUG && $error .= __FUNCTION__ .":".__LINE__ . ": Updated ArtistList :" . ArrayToTable ($newMemberList) . "<BR>";

	$this->SetOwner ($vars['ArtistID']);
	
	// Be sure owner is not removed
	in_array($this->owner, $newMemberList) || $newMemberList[] = $this->owner;
	
	$newArtistList[] = $this->owner;
	// if no Members (shouldn't happen) then assign the current user.
	if (! count($newMemberList)) {
		$newMemberList = array ($this->fp_user);
	}
	
	$myGroup = new FPGroup ($this->mysqli_connection, $this->ID );
	$oldMemberList = array_keys ( $this->MemberList() );
	
	// Get Members in the old list NOT in the new list
	// Expel the unwanted Members to the Public group
	$MembersToDelete = array_diff ($oldMemberList, $newMemberList);
	$DEBUG && $error .= __FUNCTION__ .":".__LINE__ . ": remove Member(s) :" . ArrayToTable ($MembersToDelete) . "<BR>";
	$this->ExpelMembers ($MembersToDelete);
	
	// Now, we remove the owner we'd previously added
	if ($k = array_search($this->owner, $newMemberList))
		unset ($newMemberList[$k]);
	
	// Add those in the new list but not in the old list
	$MembersToAdd = array_diff ($newMemberList, $oldMemberList);
	$DEBUG && $error .= __FUNCTION__ .":".__LINE__ . ": add new Member(s) :" . ArrayToTable ($MembersToAdd) . "<BR>";
	$this->AddMembers ($MembersToAdd);
}

// Write Group to database . Create a new group if $this->ID is empty.
// Use the ID in the $vars array if it exists.
function UpdateInfo ($vars = array ()) {
	// Package up any params, store into the existing param field
	$vars || $vars = $this->info;
	$vars['Params'] = MergeParams ($this->params, $vars);
	$vars['ID'] && $this->ID = $vars['ID'];
	if ($this->ID) {
		$success = UpdateRecord (DB_GROUPS, $this->ID, $vars);
	} else {
		$this->ID = EditTable ('insert', DB_GROUPS, null, $vars);
		$success = $this->ID;
	}
	$this->ProjectInfo();	//update info
	return $success;
}

function Save () {
	$success = false;
	$this->CopyPublicVarsToInfo ();
	if ($this->ID) {
		if (FetchGroup($this->ID) && UpdateRecord (DB_GROUPS, $this->ID, $this->info)) {
			$success = true;
		} else {
			$this->ID = EditTable ('insert', DB_GROUPS, null, $this->info);
			$success = $this->ID;
		}
		$success && $this->ProjectInfo();	//refresh/update info
	}
	return $success;
}

private function CopyPublicVarsToInfo () {
	$this->info['ID'] = $this->ID;
	$this->info['Title'] = $this->title;
	$this->info['Description'] = $this->description;
	$this->info['ArtistID'] = $this->owner;
	$this->info['Icon'] = $this->icon;
	$this->info['Theme'] = $this->theme;
	
	SetParam ($this->params, FP_PARAM_GROUP_LOGO, $this->logo);
	SetParam ($this->params, FP_PARAM_GROUP_ICON, $this->icon);
	$this->info['Params'] = EncodeArrayForDB ($this->params);
}

function SetParam ($key, $value) {
	$this->params = SetParam ($this->params, $key, $value);
}

// Create a new project (exhibition) for a user. If the name is not given, call it "untitled"
function MakeProjectForUser ($userID, $groupname = null) {
	$user = new FPUser ($this->mysqli_connection, $userID);

	if (!$groupname) {
		$groupname = $user->Fullname;
	}

	$vars['Title'] = $groupname;
	$vars['ArtistID'] = $userID;
	$this->UpdateInfo ($vars);
	
	// Make User the owner of the group
	$user->setval('GroupID', $this->ID);
	$user->updateUser ();
	
	return $this->ID;
}

// Remove all images from a project
function RemoveImages ($imageList = null)
	{
	$imageList
		? $list = " and PartID in ($imageList)"
		: $list = "";	
	$query = "DELETE from ".DB_PARTS." where ProjectID = {$this->ID} AND PartTable = '".DB_IMAGES."'$list";
	$result = mysqli_query ($this->mysqli_connection, $query);
	}

// Add an ordered list of images to the project.
// If there are existing images, these pictures are added, in order, after those
function AddImages ($imageList = array() )
	{
	$projectID = $this->ID;
	$artistIDs = $this->MemberIDList();
	$nextIndex = $this->NextImageIndex();
	
	foreach ($imageList as $imageID) 
		{
		$image = FetchImage ($imageID);
		$artistID = $image['ArtistID'];

		// be sure the artist for this image is actually in the project
		// it could be the image was passed on, but the artist unselected.
		if (in_array($artistID, $artistIDs)) 
			{
			$pairs = array (	'ProjectID'		=> $projectID,
							'ArtistID'			=> $artistID,
							'PartTable'			=> DB_IMAGES,
							'PartID'			=> $imageID,
							'OrderInProject'	=> $nextIndex
							);
			$nextIndex++;
			$newID = EditTable ('insert', DB_PARTS, '', $pairs);
			}
		else 
			{
			$DEBUG && $msg .= __FUNCTION__ .":".__LINE__.": HEY! the image=$imageID is here but not the artist=$artistID<BR>";
			}
		}
	}

// Get the highest OrderInProject number + 1
// Used to add new images to a project, in order.
function NextImageIndex ()
	{
	$query = "SELECT  MAX(OrderInProject)+1 as NextIndex from ".DB_PARTS." where ProjectID = {$this->ID} AND PartTable = '".DB_IMAGES;
	$result = mysqli_query ($this->mysqli_connection, $query);
	if ($result) {
		$result = mysqli_fetch_array($result, MYSQLI_ASSOC);
	} else {
		return 1;
	}
	$next = $result['NextIndex'];
	$next || $next = 1;
	return $next;
	}
	
	
	
/*
 * SaveProjectPictureSettings
 * Save settings for individual pictures in a project. The settings could include
 * position, size, etc. 
 * Settings are an array of arrays, packed into a string.
 */
function SaveProjectPictureSettings ($settings = array(), $imageID = null) {
	if (!$settings || !is_array($settings)) {
			return false;
	}
	
	$imageID &&	$settings['id'] = $imageID;

	if (!$settings['id']) {
			return false;
	}
	 
	$ss = serialize ($settings);
	$params = FetchParams (DB_PROJECTS, $this->ID);
	$allSettings = unserialize(GetParam ($params, FP_PARAM_GALLERY_ITEM_SETTINGS ));
	$allSettings[$settings['id']] = $ss;
	$params = SetParam ($params, FP_PARAM_GALLERY_ITEM_SETTINGS, serialize($allSettings));
	SaveParams (DB_PROJECTS, $this->ID, $params);
	return true;
}

/*
 * FetchAllProjectPictureSettings()
 * Get the array of all picture settings for a project.
 * Each element in the array is an array.
 */
function FetchAllProjectPictureSettings () {
	$a = unserialize(GetParam ($this->params, FP_PARAM_GALLERY_ITEM_SETTINGS ));
	if (is_array($a)) {
		while (list($k,$v) = each ($a)) {
			$aa[$k] = unserialize($v);
		}
		return $aa;
	} else {
		return null;
	}
}



/*
 * FetchProjectPictureSettings
 * Fetch settings for individual pictures in a project.
 * Settings are an array of arrays, packed into a string.
 */
function FetchProjectPictureSettings ($imageID = null) {
	if (!$imageID) {
			return false;
	}
	$params = FetchParams (DB_PROJECTS, $this->ID);
	$allSettings = unserialize(GetParam ($params, FP_PARAM_GALLERY_ITEM_SETTINGS ));
	if ($allSettings[$imageID]) {
		$setting = unserialize($allSettings[$imageID]);
	} else {
		$setting = null;
	}
	return $setting;
}
	

// =====================================================================================
} // class

?>