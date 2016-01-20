<?php

class FPGroup
{
// ---------------------
// Variables
// ---------------------
	/**
	 * The mysqli database connection
	 */
	var $mysqli_connection;


	public $info;
	public $members;
	public $projects;
	public $ID, $title, $description, $statement, $owner, $is_public, $icon, $params, $showingArtistID, $is_solo, $theme, $mastertheme, $variation;

// ---------------------	
// Functions
// ---------------------
function __construct ($mysqli_connection, $ID = null, $fp_user = null) {
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $Themes;
	global $msg, $error;
	
	$DEBUG = 0;
	
	$this->mysqli_connection = $mysqli_connection;
	
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

	if ($this->GroupExists ($ID)) {
		$this->GroupInfo ($ID);
		// fetch existing values for this group
		$this->ID = $ID;
		$this->group = $this->FetchGroup ($ID);
		//$this->title = $this->info['Title'];
		$this->title || $this->title = "Untitled Group";
		//$this->owner = $this->info['ArtistID'];
		//$this->is_public = $this->info['Public'];
		//$this->description = $this->info['Description'];
		//$this->icon = $this->info['Icon'];
		//$this->theme = $this->info['Theme'];
		//$this->params = 	$this->info['Params'];
		$this->IsSolo();

	} else {
		// this is a new, blank group
		$this->ID = null;
		$this->group = array();
		$this->title = "Untitled Group";
		$this->owner = $this->fp_user;
		$this->is_solo = 1;
	}

	list($this->mastertheme,$this->variation) = explode (":", $this->theme);
	if (!$this->ThemeExists($this->theme)) {
		$this->theme = $this->mastertheme;
		$this->variation = null;
		if (!$this->ThemeExists($this->theme)) {
			$this->theme = DEFAULT_THEME;
			$this->variation = null;
		}
	}
}

function members () {
	print "<hr>Members function:";
	print $this->var;
	print "<hr>";
}

function GroupExists ($ID = null) {
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;
	
	// get calling function
	//$msg .= debug_backtrace()[2]['function'] . "<BR>";
	//echo debug_backtrace()[2]['function'] . "<BR>";
	
	
	if (!$ID)
		return false;
	//$ID || $ID = $this->ID;
	$query = "SELECT ID FROM $GROUPS WHERE ID = '$ID'";
	$result = mysqli_query ($this->mysqli_connection, $query);
	if (mysqli_num_rows ($result) > 0)
		return true;
	return false;
}

function iterateVisible() {
	echo "FPGroup::iterateVisible:\n";
	foreach($this as $key => $value) {
		print "$key => $value\n";
	}
}

// Change fields for all groups
// $pairs = array ('fieldname'=>'fieldvalue');
function ModifyGroups ($pairs, $col, $val) {
	global $msg, $error;
	UpdateRow ( DB_GROUPS, $pairs, $col, $val );
}


function FetchGroup ($ID  = null) {
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;

	isset($ID) || $ID = $this->ID;
	$this->members = $this->MemberList ( ($ID != FP_ADMINISTRATOR) );	// do not show member $ID in the list unless he is the Admin
	$this->project = $this->ProjectList ($ID);
	$this->GroupInfo ($ID);
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
function GetTheme() {
	global $THEME_DIRS_ARR;

	$THEME_DIRS_ARR[$this->theme] ? $myTheme = $THEME_DIRS_ARR[$this->theme] : $myTheme = DEFAULT_THEME;
	return ($myTheme);
}

// Set the theme/variation
function SetTheme($themeID = null) {
	global $THEME_DIRS_ARR;
	if ($themeID) {
		$this->theme = $themeID;
		$this->save();
	}
}


// Get info for a group
function GroupInfo ($ID  = null) {
	// Fetch array of all members in the group
	// array [ID] = member record
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;

	isset($ID) || $ID = $this->ID;
	$this->ID = $ID;
	$this->info = FetchRowsByValue ($GROUPS, "*", 'ID', $ID, "", TRUE);
	$this->title = $this->info['Title'];
	$this->description = $this->info['Description'];
	$this->owner = $this->info['ArtistID'];
	$this->is_public = $this->info['Public'];
	$this->icon = $this->info['Icon'];
	$this->theme = $this->info['Theme'];
	$this->params = DecodeArrayFromDB($this->info['Params']);
	$this->logo = GetParam ($this->params, FP_PARAM_GROUP_LOGO);
	$this->icon = GetParam ($this->params, FP_PARAM_GROUP_ICON);

	return $this->info;
}

function OwnerInfo ($ID  = null) {
	$ID || $ID = $this->ID;
	if ($ID) {
		$this->GroupInfo ();
		$record = FetchArtist($this->info['ArtistID']);
		return $record;
	} else {
		return array ();
	}
}

function OwnerID ($ID  = null) {
	isset($ID) || $ID = $this->ID;
	$this->GroupInfo ();
	return $this->info['ArtistID'];
}

function MemberList ( $hideowner = FALSE) {
// Given a group ID, return an array of the artists who are part of the group
// These are chosen by looking at the list of Parts for the group
// If $hideowner is set, remove project owner from the list...it's an "other artists" list, then.
// Sys Admin is not part of any group. Access is given elsewhere.
// Group 1 is the public group and EVERYONE is a member.
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;

	$DEBUG = 0;
	
	$DEBUG && $starttime = microtime(true);


	$members = array();					// empty the member list
	//($this->accesslevel > FP_ADMINISTRATOR) && $hideowner = true;


	$ID = $this->ID;
	$table = "$ARTISTS";
	$set = "DISTINCT $ARTISTS.*, $ARTISTS.ID as ID, CONCAT_WS(', ', $ARTISTS.Lastname, $ARTISTS.Firstname) AS Fullname";
	
	if ($ID == 1) {
		$where = "";
	} else {
		$where = "WHERE $ARTISTS.ID in (select distinct ArtistID from $PARTS where PartTable=\"$GROUPS\" and PartID = '$ID')";
	}
	
	
	$order = "ORDER BY Lastname";
	$query = "select $set from $table $where $order";
	
	$DEBUG && $msg .= "<BR>$query<BR>";

	$result = mysqli_query ($this->mysqli_connection, $query);
	
	$DEBUG && $msg .= __FUNCTION__.__LINE__.": Time elapsed: ". round (microtime(true) - $starttime, 2) . " seconds<BR>";

	// Add in the owner!
	if (!$hideowner)
		$members[$this->owner] = FetchArtist($this->owner);

	while ($record = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
		$members[$record['ID']] = $record;
		$DEBUG && $msg .= "<BR>{$record['Lastname']} in group $ID<BR>";

	}
	if ($this->fp_user != FP_ADMINISTRATOR && $hideowner)
		unset ($members[$this->owner]);

	// System Admin is not part of anything
	if (isset($members[FP_ADMINISTRATOR]))
		unset($members[FP_ADMINISTRATOR]);
	$this->members = $members;
	return $this->members;
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

// Sets $this->projects, an array of all projects in the group
function ProjectList ($ID = null) {
	// Fetch array of all projects of members in the group
	// array [ID] = project record
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;

	$this->projects = array();	// empty the member list

	isset($ID) || $ID = $this->ID;
	$table = "$PROJECTS";
	$set = "*";
	$where = "WHERE $PROJECTS.GroupID = $ID";
	$order = "ORDER BY Title";
	$query = "select $set from $table $where $order";
	$result = mysqli_query ($this->mysqli_connection, $query);
	$this->projects = array();
	if ($result) {
		while ($record = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
			$this->projects[$record['ID']] = $record;
		}
	}
	return $this->projects;
}

// Look for a slideshow project in this group. If found, return the project
function FetchSlideshow ($ID = null) {

	isset($ID) || $ID = $this->ID;

	$table = DB_PROJECTS;
	$query = "select * from $table WHERE (GroupID = $ID AND Slides > 0) LIMIT 1";	
	$result = mysqli_query ($this->mysqli_connection, $query);
	if ($result) {
		$record = mysqli_fetch_array($result, MYSQLI_ASSOC);
	} else {
		$record = null;
	}
	return $record;
}	


// Basic functions for manipulating a gallery

function ExpelMembers ($members = array (), $targetgroupID = PUBLIC_GROUP_ID) {
// Move members from current group to target, default is Public
	$this->MoveMembers ($members, $targetgroupID);
}

function RecruitMembers ($members = array ()) {
// Move members from Public to current group
	$this->MoveMembers ($members, $this->ID, PUBLIC_GROUP_ID);
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

// Add an array of artists to a group
function AddMembers ($members = array ()) {
	return $this->AddMembersToGroup ($members, $this->ID);
}

// Add an artist, or an array of artists to a group
// $members : record ID (integer) or array of ID's
// $newgroupID : empty = $this->ID ; or ID of the group to change
// Don't use this to set the owner!
// NEVER add FP_ADMINISTRATOR to a group. Admin is never part of a group
// but always has access.
function AddMembersToGroup ($members = array (), $newgroupID = null) {
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
									'PartTable'	=>	$GROUPS,
									'PartID'	=>	$newgroupID,
									'ProjectID'	=>	null
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


// IsSolo()
// Return true if this group is a solo group, with only one member. We exclude the owner and FP_Administrator.
// If there are other members, then this is NOT a solo group (unless we're checking only who has pictures)
// This is a common request, and we can do this much faster this way, rather than checking
// for number of members in the group.
// $withPictures : true, then count only artists who have pictures showing; false, count all members of group
//1=FP_SINGLE_GALLERY_SINGLE_USER cannot add galleries or users
//2=FP_SINGLE_GALLERY_MULTI_USER cannot add galleries
//3=FP_MULTI_GALLERY_SINGLE_USER can add new galleries, but not users
//4=FP_MULTI_GALLERY_MULTI_USER can add new galleries, and new users

function IsSolo ($withPictures = false) {
	global $msg, $error;
	
	$DEBUG = 0;
	$DEBUG && $msg .= "Calling function: ".getCallingFunction()."<BR>";
	
	//$artist = new FPUser($this->mysqli_connection, $this->info['ArtistID']);
	//if (($artist->UserType == FP_SINGLE_GALLERY_SINGLE_USER) || ($artist->UserType == FP_MULTI_GALLERY_SINGLE_USER)) {
	// 
	// If this is a single user system, then there's only one user!
	if (FP_GALLERY_TYPE == FP_SINGLE_GALLERY_SINGLE_USER || FP_GALLERY_TYPE == FP_MULTI_GALLERY_SINGLE_USER) {
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
	
//	while ($x = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
//		$xx[] = $x;
//	}

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
//	$x = $this->is_solo;
	
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
	$artist = new FPUser($this->mysqli_connection, $this->info['ArtistID']);
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
			// Get all active projects
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
			$where = "(PartTable = \"".DB_GROUPS."\" AND PartID = {$this->ID} AND $PARTS.ArtistID != {$this->owner} AND $PARTS.ArtistID != ".FP_ADMINISTRATOR.")";
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


// Delete Group
function Delete ($ID = null) {
	global $msg;
	$ID || $ID = $this->ID;
	// Reassign projects which are members of this group (projects) to the public group
	$projects = $this->ProjectList($ID);
	$msg .= "Deleted {fp:group}: {$this->title}<BR>";
	foreach ($this->projects as $project) {
		DeleteProject ($ID, true);
	}
	CascadeDelete (DB_GROUPS, $ID);
}


function DeleteMembers ($members = array ()) {
	$this->DeleteMembersFromGroup ($members);
}

function DeleteMembersFromGroup ($members = array (), $fromgroup = null) {
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
	
	$myGroup = new FPGroup ($this->mysqli_connection,  $this->ID );
	$oldMemberList = array_keys ( $this->MemberList() );
	
	// Get Members in the old list NOT in the new list
	// Expel the unwanted Members to the Public group
	$MembersToDelete = array_diff ($oldMemberList, $newMemberList);
	$DEBUG && $error .= __FUNCTION__ .":".__LINE__ . ": remove Member(s) :" . ArrayToTable ($MembersToDelete) . "<BR>";
	$this->ExpelMembers ($MembersToDelete);
	
	// Now, we remove the owner we'd previously added
	if (array_search($this->owner, $newMemberList))
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
	$this->GroupInfo();	//update info
	return $success;
}

// Save changes 
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
		$success && $this->GroupInfo();	//refresh/update info
	}
	return $success;
}


private function CopyPublicVarsToInfo () {
	$this->info['ID'] = $this->ID;
	$this->info['Title'] = $this->title;
	$this->info['Description'] = $this->description;
	$this->info['ArtistID'] = $this->owner;
	$this->info['Public'] = $this->is_public;
	$this->info['Icon'] = $this->icon;
	$this->info['Theme'] = $this->theme;
	
	SetParam ($this->params, FP_PARAM_GROUP_LOGO, $this->logo);
	SetParam ($this->params, FP_PARAM_GROUP_ICON, $this->icon);
	$this->info['Params'] = EncodeArrayForDB ($this->params);
}

// Logo is either PNG or JPG file
function LogoFilename () {
	global $error, $msg;
	
	$dir = FP_DIR_USER_IMAGES;
	
	$uploadedLogoJPG = "$dir/group_logo_" . $this->ID . ".jpg";
	$uploadedLogoPNG = "$dir/group_logo_" . $this->ID . ".png";
	
	if ($this->logo > 0) {
		$logoImageID = $this->logo;
		$image = FetchImage ($logoImageID);
		$dir = FP_DIR_THUMBNAILS_IMAGES;
		return "$dir/" . $image['URL'];
	} elseif (file_exists($uploadedLogoJPG)) {
		return $uploadedLogoJPG;
	} elseif (file_exists($uploadedLogoPNG)) {
		return $uploadedLogoPNG;
	}
	//return BLANK_IMAGE;
	return null;
}

function LogoHTML ($extrahtml = "") {
	$logo = $this->LogoFilename();
	$logo
		? $logo = URLtoIMG ($logo, $extrahtml)
		: $logo = "<!-- no logo -->";
	return $logo;
}

// Icon is either PNG or JPG file
// Return the relative path from the site home directory to the icon
function IconFilename () {
	global $error, $msg;
	
	$dir = FP_DIR_USER_IMAGES;

	$uploadedIconJPG = "$dir/group_icon_" . $this->ID . ".jpg";
	$uploadedIconPNG = "$dir/group_icon_" . $this->ID . ".png";
	
	if ($this->icon > 0) {
		$iconImageID = $this->icon;
		$image = FetchImage ($iconImageID);
		$dir = FP_DIR_THUMBNAILS_IMAGES;
		return "$dir/" . $image['URL'];
	} elseif (file_exists($uploadedIconJPG)) {
		return $uploadedIconJPG;
	} elseif (file_exists($uploadedIconPNG)) {
		return $uploadedIconPNG;
	}
	//return BLANK_IMAGE;
	return "";
}

function IconHTML ($extrahtml = "") {
	$icon = $this->IconFilename();
	$icon
		? $icon = URLtoIMG ($icon, $extrahtml, FP_GALLERY_ICON_WIDTH, FP_GALLERY_ICON_HEIGHT)
		: $icon = "<!-- no icon -->";
	return $icon;
}

function SetParam ($key, $value) {
	$this->params = SetParam ($this->params, $key, $value);
}

function GetParam ($key) {
	return GetParam ($this->params, $key);
}

function SetIcon ($value) {
	$this->SetParam (FP_PARAM_GROUP_ICON, $value);
	$this->icon = $value;
}

function SetLogo ($value) {
	global $error, $msg;
	
	$this->SetParam (FP_PARAM_GROUP_LOGO, $value);
	$this->logo = $value;
}

function ClearLogo () {
	global $error, $msg;
	
	DeleteUserPicture (basename($this->LogoFilename()));
	$this->SetParam (FP_PARAM_GROUP_LOGO, null);
	$this->logo = $value;
}

function ClearIcon () {
	global $error, $msg;
	
	DeleteUserPicture (basename($this->IconFilename()));
	$this->SetParam (FP_PARAM_GROUP_ICON, null);
	$this->icon = $value;
}

/*
Create a new gallery for a user. Unless the $galleryname is set, use the user's full name.
*/
function MakeGroupForUser ($userID, $groupname = null) {
	$user = new FPUser($this->mysqli_connection, $userID);

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


function ThemeExists ($themeid =  null) {
	global $Themes;

	if (!$themeid) {
		return false;
	}
	/*
	if (!array_key_exists($themeid, $Themes->FP_Themes)) {
		 if (!$Themes->FP_Themes[$themeid]) {
			print __FUNCTION__.": NO MATCH: $themeid<BR>";
			print ArrayToTable ($Themes->FP_Themes);
		}
	}
	*/
	$match = array_key_exists($themeid, $Themes->FP_THEMES_LIST);
	return $match;
}
} // class

?>