<?PHP
/*
FUNCTIONS
FRONTLINE-PHOTOS GALLERY SYSTEM

VERY IMPORTANT:
PHP is very sensative to permissions. The directories which contain 
pictures must have 0775 permission...or the PHP have some better 
permission power than on my own machine. In any case, it won't
delete what it should, otherwise.

NOTE TO MYSELF:
$first ? $second : $third


*/


error_reporting	(E_ERROR | E_WARNING | E_PARSE );

//------------------
require_once('utf8_to_html.inc');




// SETUP
// Initialize variables and such
function Setup () {
	global $FP_ADMIN_USER, $FP_GALLERY_TYPE, $FP_CONSTANTS;
	global $FP_NAVBAR_ITEM_NAMES;
	global $LINK;
	global $error, $msg;
	
	$DEBUG = false;

	// Load the Admin user for this session
	$FP_ADMIN_USER = new FPUser ($LINK, FP_ADMINISTRATOR);
	$FP_ADMIN_USER->UserType && $FP_GALLERY_TYPE = $FP_ADMIN_USER->UserType;

	FP_USE_PAYPAL_SANDBOX
			? define ("FP_FRAMESHOP_TESTING_MSG", FetchSnippet ("frameshop_testing_message"))
			: define ("FP_FRAMESHOP_TESTING_MSG", "");

	// Get all constants into an array for Substitutions search/replace
	$FP_CONSTANTS = get_defined_constants (true);
	$FP_CONSTANTS = $FP_CONSTANTS['user'];
	$DEBUG &&  $msg .= __FUNCTION__.ArrayToTable ($FP_CONSTANTS);

	$FP_NAVBAR_ITEM_NAMES = FetchNavbarTitles();

}

// ============================


function Maintenance ($DEBUG = false) {
	global $BASEDIR, $LOGS;

	$DEBUG && print __FUNCTION__."<BR>";

//	fp_error_log("Maintenance: BEGIN ----------------", 3, FP_MAINTENANCE_LOG);

	$DEBUG && print "Maintenance function Running<BR>";

	$DEBUG && print "Maintenance: Maintain Projects<BR>";
	MaintainProjects ();

	$DEBUG && print "Maintenance: Maintain Artists<BR>";
	MaintainArtists();

	$DEBUG && print "Maintenance: Maintain Groups<BR>";
	MaintainGroups();

	// Remove Users who are NOT confirmed, and too old
	$DEBUG && print "Maintenance: RemoveOldUsers<BR>";
	RemoveOldUsers ();

	// "ipn_res.inc also calls this function to delete old
	// orders that were never processed.
	$DEBUG && print "Maintenance: DeleteUnusedOrders<BR>";
	DeleteUnusedOrders();

	$DEBUG && print "Maintenance Finished<BR>";
//	fp_error_log("Maintenance: END ----------------\n", 3, FP_MAINTENANCE_LOG);
}


// MaintainGroups();
// If there are no groups, create group ID=1 (public)
// owned by sys admin
function MaintainGroups() {
	global $error, $msg;
	global $LINK;

	$g = new FPGroup ( $LINK );
	if (!$g->GroupExists(FP_ADMINISTRATOR)) {
		AddMainGallery (FP_ADMINISTRATOR);
	}
}


// Create a new gallery. It will be owned by artistID = $id
function AddMainGallery ($id = null, $title = "Main Gallery") {
	global $msg, $error;
	global $LINK;

	$id || $id = FP_ADMINISTRATOR;

	$g = new FPGroup ($LINK);
	$g->ID = PUBLIC_GROUP_ID;
	$g->title = $title;
	$g->owner = $id;
	$g->is_public = 1;
	$result = $g->Save ();
	SetRowID	 (DB_GROUPS, $g->ID, PUBLIC_GROUP_ID);

	$result && $msg .= "Added $title<BR>";
	return $result;
}

//-------------------
// Maintain Users (Artists) database
// - Look for unconfirmed users, delete them if they are too old
function MaintainArtists () {
	global $PHOTOS_GALLERY, $SLIDES, $THUMBNAILS, $MATTED, $ORIGINALS;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;
	global $LINK;

	$DEBUG = 0;
	$DEBUG && $msg .= __FUNCTION__.":MaintainArtists<br>";

	//BE SURE EACH ARTIST HAS A PERSONAL GALLERY
	$myresult = GalleriesForArtists ();
	$DEBUG && print "<HR>$myresult<HR>";
	$DEBUG && print "MaintainArtists: END\n";
}	

//-----
// Checks to be sure each artist has a personal gallery
// and creates one if he doesn't.
function GalleriesForArtists () {
	global $PHOTOS_GALLERY, $SLIDES, $THUMBNAILS, $MATTED, $ORIGINALS;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;
	global $LINK;

	$DEBUG = 0;
	$myresult = "Results of Galleries for Artists:";

	$artists = FetchAllArtistArrays();
	$projects = FetchAllProjectArrays();

	foreach ($artists as $artist) {
		if ($artist['ID'] != FP_ADMINISTRATOR) {
			$projectID = $artist['ProjectID'];
			$DEBUG && $msg .= __FUNCTION__.": Checking for default gallery of " . $artist['Firstname'] . " " . $artist['Lastname'] . " (exists ID $projectID ?)<BR>";
			// does the project exist?
			if (!array_key_exists($projectID, $projects)) {
				$projectID = MakeArtistProject($artist['ID']);
				$DEBUG && print "New Project Created ID=$projectID for artist ID={$artist['ID']}<BR>";
				SetDefaultProject ($artist['ID'], $projectID);

				$q = FetchArtist($artist['ID']);
				$DEBUG && print $q['ID'] . " has project " . $q['ProjectID'] . "<BR>";

				$DEBUG && print "Created gallery for " . $artist['Firstname'] . " " . $artist['Lastname'] . "<BR>";
				$myresult .= "Created gallery for " . $artist['Firstname'] . " " . $artist['Lastname'] . "<BR>";
			} else {
				$DEBUG && $msg .= __FUNCTION__.": Yes, project $projectID exists for artist {$artist['ID']}<BR>";
			}
		}
	}
	return $myresult;
}

//-------------------
// After any modification to a project, we reset a project's count-down timer
// This way, as long as there are modifications to it, it is featured & active
// After the artist stops working on it, and the timer counts down,
// it's moved out of featured, but it stays active for another
// period of time. 
// ACTIVE means it will appear under that artist's name
// but won't be on the Projects page as 'featured.'
// After it is no longer active, it is deleted
function MaintainProjects () {
	global $BASEDIR, $LOGS;

	$DEBUG = 0;
	$DEBUG && print "<BR>MaintainProjects: BEGIN<BR>";

	// Get all projects
	$result = FetchAllProjects('', 'Title');

	while ($project = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
		$projectstatus = GetProjectStatus ($project);
		$DEBUG && print "MaintainProjects: Checking project ".$project['Title']."<BR>\n";
		if ($projectstatus['TimeToDie']) {
			$DEBUG && (print __FUNCTION__.": Do NOT delete project,we are testing: Project ".$project['ID']."<BR>\n");
			$DEBUG || DeleteProject ($project['ID']);
			fp_error_log("Deleted Project ".$project['ID']."\n", 3, FP_MAINTENANCE_LOG);
		}

	}

	$DEBUG && print "<HR>END of Project Maintenance<BR>";
}

//----------
// GetProjectStatus
// Featured/Active
// Calculate these based on date of last change in project,
// rather than marking fields.
// Why? Because it's going to be more accurate, and probably less 
// processing in the end.
// ...to come
// IF $project IS A PROJECT ARRAY, FINE...if not, it is considered a project ID
// and the project is accessed. Weird, but cool.
function GetProjectStatus ($project) {
	global $ARTISTS, $PROJECTS, $IMAGES;
	global $msg, $error;
	global $LINK;

	$DEBUG = false;

	$timetodie = false;
	$active = true;
	$featured = true;
	$DEBUG && $msg .= "<BR>Begin GetProjectStatus<BR>\n";

	if (! is_array($project)) {
		$DEBUG && $msg .= "Parameter is NOT array; fetching project $project<BR>\n";
		$project= FetchProject($project);
	}

	// Is $project a valid array?	
	if (! is_array($project) && $project) {
	
	
		$projectdates = FetchProjectDates ($project);
		$ID = $project['ID'];
	
		// Must do before next block, which changes featured/active
	
		// Get dates, etc., add into the project info array
		$project = array_merge($project, $projectdates);
		$active = $projectdates['Active'];
		$featured = $projectdates['Featured'];
	
		//$DEBUG && print "Project $ID : active=$active | featured=$featured<BR>";
	
		$timetodie = !($active);
		//$DEBUG && $timetodie = FALSE;
	
		// If a project has neither images nor stories, it is neither featured nor active.
		if (ProjectIsEmpty ($ID)) {
			$featured = false;
			$active = false;
			$empty = true;
		} else {
			$empty = false;
		}
	
		$result = array_merge ($projectdates, array (
				'Featured'		=> $featured,
				'Active'		=> $active,
				'Empty'			=> $empty,
				'TimeToDie'		=> $timetodie,
				'Age'			=> $projectdates['Age'],
				'DaysToLive'		=> $projectdates['DaysToLive']
		));
		$DEBUG && $msg .= __FUNCTION__ .":".__LINE__. ": ".$project['Title'];
		$DEBUG && $msg .= ArrayToTable ($result);
	} else {
		$result = array();
	}
	return $result;
}

// --------------
function ProjectIsEmpty ($ID) {
	// If a project has neither images nor stories, it is neither featured nor active.
	$query  = 'SELECT ID from Parts WHERE (PartTable = "' . DB_IMAGES . '" OR PartTable  = "stories") AND ProjectID = ' . $ID;
	$result2 = mysqli_query ($LINK, $query);
	return mysqli_num_rows($result2) == 0;
}




// ============================


/* private: makeCols($arr) makes a comma delimited list of the keys of $arr */
// Ignore fields which begin with "X_", whice aren't in the database
function makeCols($arr) {
	global $Themes;
	$keys = array();
	reset($arr);
	while (list($key, ) = each($arr)) {
		if (! preg_match ('/^(X_)/i', $key))
			$keys[] = $key;
	}
	return (join(",", $keys));
}

/* private: makeVals($arr) makes a comma and single-quote delimited list
* of the values of $arr, with special characters escaped */
// This is used for INSERT of new records. Updating is elswhere.
function makeVals($arr) {
	global $error, $msg;

	$values = array();
	reset($arr);
	while (list( ,$value) = each($arr)) {
		$values[] = "'" . $value . "'";
	}
	return (join(",", $values));
}

// Printing results of select from mysql (not an array!)
function Display ($result = array() ) {
	global $Themes;
	if (! $result)
		return null;
	print "<table border=1 cellpadding=5 bgcolor='gray'>\n";
	$line = mysqli_fetch_array($result, MYSQLI_ASSOC);
	$keys = array_keys ($line);
	print "\t<tr>\n";
	reset ($line);
	while (list ($k, $col_value) = each ($line)) {
		print "\t\t<th><i>$k:</i></th>\n";
	}
	print "\t</tr>\n";
	mysqli_data_seek ($result, 0);
	while ($line = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
		print "\t<tr>\n";
		//		while (list ($k, $col_value) = each ($line)) {
		foreach ($line as $col_value) {
			$col_value = htmlentities($col_value);
			$v = htmlentities($col_value);
			print "\t\t<td>$v&nbsp;</td>\n";
		}
		print "\t</tr>\n";
	}
	print "</table>\n";
	mysqli_data_seek ($result,0);	//reset the pointer
}



/* private: makeUpdate($arr) makes a comma delimited list of the name/value
* pairs of $arr, in the form name='value', with special characters escaped */
function makeUpdate($arr) {
	global $Themes, $msg, $error;
	global $ALLOWEDTAGS;

	$pairs = array();
	reset($arr);
	while (list($key, $value) = each($arr)) {
		if (($key != "ID") AND (array_key_exists ($key, $arr))) {
			$pairs[] =	"$key = ". quote_smart($value);
		}
	}
	return (join(", ", $pairs));
}

// Look for fields named myfield_CHECKBOX and
// if they have a value, set the value of the real field using
// i.e. myfield.
// If add into the list myfield with value = 0
// remove the myfield_CHECKBOX field
function FixCheckBoxFields ($pairs) {
	global $Themes;
	global $msg, $error;
	global $LINK;
	$DEBUG = 0;
	reset ($pairs);
	while (list($key, $value) = each ($pairs)) {
		if (preg_match ('/_CHECKBOX/', $key)) {
			$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": $key : $value<BR>";
			$realkey = str_replace("_CHECKBOX", "", $key);
			// If there a match with another real field name in the pairs provided,
			// then set that field to "1", i.e. true.
			if (array_key_exists ($realkey, $pairs)) {
				$pairs[$realkey] = 1;
				unset ($pairs[$key]);
			}
		}
	}
	return $pairs;
}

// Serialize and encode any arrays in an array
// Used to package up arrays for storage in mysql
function SerializeArraysInPairs ($pairs) {
	global $msg, $error;
	global $LINK;
	$DEBUG = false;
	reset ($pairs);
	while (list($key, $value) = each ($pairs)) {
		if (is_array ($value))
			$pairs[$key] = EncodeArrayForDB ($value);
	}
	return $pairs;
}


// from the manual, how to prepare data for MYSQL
function quote_smart($value) {
	global $Themes;
	global $msg, $error;
	global $LINK;

	// Stripslashes
	if (get_magic_quotes_gpc()) {
		$value = stripslashes($value);
	}

	// The following does an addSlashes, and other goodies
	$value = mysqli_real_escape_string($LINK, $value);
	$value = "'" . $value . "'";

	return $value;
}

// ------------------------------------------
// UpdateImageOrder ($ID, $vars)
// The $vars will include an array of chosen images for a project belonging
// to the owner of the project. The $ArtistID field
// will be of the owner of the project.
// The array is a list of *** Part IDs ***
// However, the IDs are coded with "pic_" before each number, and that
// must be removed.
// The parts are in order; the Parts records must be updated
// to reflect this new order
function UpdateImageOrder ($projectID, $vars) {
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;
	global $LINK;

	$DEBUG = 0;
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": Beginning UpdateImageOrder:<BR>";

	if (! $projectID)
		return FALSE;

	isset($_SESSION['fp_user']) ? $fp_user = $_SESSION['fp_user'] : $fp_user = 1;

	$list = explode(",", $vars['neworder']);

	$offset = $vars['start'];	// force to numeric

	// Set the OrderInProject field in the image's project part
	// Remove the "pic_" prefix of each ID (cannot start a CSS id, e.g. ID="mypic" with a number!)
	$k = $offset;
	foreach ($list as $ID) {
		$ID = trim (str_replace ("pic_","",$ID));
		//$part = GetRecord( $PARTS, $ID );
		$pairs = array ('OrderInProject'	=> $k );
		UpdateRecord (DB_PARTS, $ID, $pairs);
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": $k) PartID=$ID<BR>";
		$k++;
	}
	return TRUE;
}



// Example: UpdateRecord( 'Images', 55, array('ProjectID'=>33 )
// This new version uses "prepared statements" and mysqli for security.
// It is also about 100 times faster!
function UpdateRecord ( $table, $ID, $pairs ) {
	global $BASEDIR, $LOGS;
	global $LINK;
	global $msg, $error;
	global $LINK;

	$DEBUG = 0;

	if (!$ID)
		return false;

	// Look for checkbox fields...if there is a field
	// myfield_CHECKBOX then there is a myfield, but it
	// was unchecked, which means a 0 entry, not NO entry!
	$pairs = FixCheckboxFields ($pairs);
	if ($DEBUG)
		$error .= __FUNCTION__.ArrayToTable($pairs);
	$pairs = StripNonExistantFields ($table, $pairs);
	$pairs = PackageParams ($pairs);
	$pairs = SerializeArraysInPairs ($pairs);

	reset($pairs);
	while (list($k,$v) = each ($pairs)) 
		{
		/* Create a prepared statement */
		$stmt = $LINK->prepare( "UPDATE $table SET $k = ? WHERE ID = ?" );
		if ( $stmt )
			{
			$stmt->bind_param("ss", $v, $ID);
			$result = $stmt->execute();
			if (!$result)
				break;
			} else {
				$result = false;
				break;
			}
		$stmt->close();
		}

	return $result;

}



/* public: updateTopic() updates one single topic */
// Example: UpdateRecord( 'Images', 55, array('ProjectID'=>33 )
function UpdateRecordOLD ( $table, $ID, $pairs ) {
	global $BASEDIR, $LOGS;
	global $msg, $error;
	global $LINK;

	$DEBUG = 0;

	if (!$ID)
		return false;

	// Look for checkbox fields...if there is a field
	// myfield_CHECKBOX then there is a myfield, but it
	// was unchecked, which means a 0 entry, not NO entry!
	$pairs = FixCheckboxFields ($pairs);
	if ($DEBUG)
		$error .= __FUNCTION__.ArrayToTable($pairs);
	$pairs = StripNonExistantFields ($table, $pairs);
	$pairs = PackageParams ($pairs);
	$pairs = SerializeArraysInPairs ($pairs);
	$update = makeUpdate($pairs);
	$query = "UPDATE $table SET $update where ID = '".mysqli_real_escape_string($LINK, $ID)."' ";
	if ($DEBUG)
		$error .= __FUNCTION__.": <BR>$query<BR>";
	$result = mysqli_query ($LINK, $query);
	return $result;

}



// If no $where is set, update the record where the
// id = $arr['id']. 
// $where is of the form, "field='value'"
function UpdateRow ( $table, $pairs, $col="", $val="" ) {
	global $LINK;
	global $error, $msg;
	
	$pairs = StripNonExistantFields ($table, $pairs);
	$pairs = SerializeArraysInPairs ($pairs);
	reset ($pairs);
	if ($col && $val)
		{
		while (list($k,$v) = each ($pairs)) 
			{
			/* Create a prepared statement */
			$stmt = $LINK->prepare( "UPDATE $table SET $k = ? WHERE $col = ?" );
			if ( $stmt )
				{
				$stmt->bind_param("ss", $v, $val);
				$result = $stmt->execute();
				if (!$result)
					break;
				} else {
					$result = false;
					break;
				}
			$stmt->close();
			}
		}
	else
		{
		$result = false;
		}
	return $result;

}


function SetRowID ($table, $oldID=null, $newID = null) {
	global $LINK;
	
	if ($oldID && $newID) {
		$query = "UPDATE $table SET ID = $newID WHERE ID = $oldID";
		mysqli_query ($LINK, $query);
	}
}



// Fetch rows based on a key/value. Return an array of arrays index by $index
function FetchRowsByValueAsArray ($table, $set, $keyfield, $value, $index = 'ID', $order) {
	$result = FetchRowsByValue ($table, $set, $keyfield, $value, $order, false);
	$rows = array();
	if ($result)
		{
		while ($row = mysqli_fetch_array($result, MYSQLI_ASSOC))
			{
			$rows[$row[$index]] = $row;
			}
		}
	return $rows;

}

// return mysql result
// IF $returnarray is set, return the first row as an array (dump the rest)
function FetchRowsByValueMysqli ($table, $set, $keyfield, $value, $order, $returnarray = FALSE) {
	global $Themes;
	global $LINK;
	global $error, $msg;
	$DEBUG = 0;
	
	if (!$value || !$keyfield || !$set || !$table)
		{
	$DEBUG && fp_error_log(__FUNCTION__.": table=$table, set=$set, keyfield=$keyfield, value=$value, order=$order, returnarray=$returnarray", 3, FP_ERROR_LOG);
		return false;
		}
	
	// Sanitize values
	$table = VerifyTableName($table);
	if (!$table)
		return false;
	
	$order && $order = "ORDER BY $order";
	$returnarray ? $limit = "LIMIT 1" : $limit = "";
	
	StartTimer($DEBUG);
	
	//for ($i=1;$i<=1;$i++)
	//	{
		if (false)
			// use mysqli
			{
	
			$query = "select $set from $table where $keyfield = '$value' $order $limit";
			$result = $LINK->query($query);
		
			if ($returnarray && $result)
				{
				$result = $result->fetch_array(MYSQLI_ASSOC);
				$result || $result = array();
				}
			}
			
		else
			// use mysql interface
			{
			$query = "select $set from $table where $keyfield='$value' $order $limit";
			$result = mysqli_query ($LINK, $query);
			if ($returnarray && $result)
				$result = mysqli_fetch_array($result, MYSQLI_ASSOC);
				$result || $result = array();
			}
	//	}
	$DEBUG && fp_error_log(__FUNCTION__.": ".getCallingFunction()." ==> $table, $keyfield = $value : ".CurrentTimer(), 3, FP_ACTIVITY_LOG);
	
	PauseTimer($DEBUG);

	
	return $result;
}


// return mysql result
// IF $returnarray is set, return the first row as an array (dump the rest)
function FetchRowsByValue ($table, $set, $keyfield, $value, $order, $returnarray = FALSE) {
	global $error, $msg;
	global $LINK;
	
	$DEBUG = 0;

	if (!$value || !$keyfield || !$set || !$table)
		{
		$DEBUG && fp_error_log(__FUNCTION__.": table=$table, set=$set, keyfield=$keyfield, value=$value, order=$order, returnarray=$returnarray", 3, FP_ERROR_LOG);
		return false;
		}

	// Sanitize values
	$table = VerifyTableName($table);
	if (!$table)
		return false;
	
	$value = mysqli_real_escape_string($LINK, $value);
	
	$order && $order = "ORDER BY $order";
	$returnarray ? $limit = "LIMIT 1" : $limit = "";

	//StartTimer(true);
//	for ($i=1;$i<=300;$i++)
//		{

		if (! ($keyfield and $value)) {
			$query = "select $set from $table $order";
		} else {
			$query = "select $set from $table where $keyfield='$value' $order $limit";
		}
		if ($DEBUG)
			$error .= __FUNCTION__.": $query<P>";
	
		$result = mysqli_query ($LINK, $query);
		if ($returnarray && $result)
			$result = mysqli_fetch_array($result, MYSQLI_ASSOC);
	
		//$t = EndTimer();
		//$t && $msg .= "Time: $t; ";
//		}

	return $result;
}


function FetchFirstRowAsArray ($table, $set) {
	global $LINK;
	global $error, $msg;

	// Sanitize values
	$table = VerifyTableName($table);
	if (!$table)
		return false;
	
	$query = "select $set from $table LIMIT 1";
	$result = $LINK->query($query);
	$row = $result->fetch_array(MYSQLI_ASSOC);
		
	return $row;
}



function CountRecordsByValue($table, $column, $value) {
	$query = "select $column from $table where $column = `$value`";
	$result = mysqli_query ($LINK, $query);
	$k = 0;
	if ($result)
		$k = mysqli_num_rows($result);
	return $k;
}


function isAction ($fieldname) {
	global $Themes;
	global $commandFlag;
	if (trim($fieldname) == "action")
		return TRUE;
	return FALSE;
}


// Get $_REQUEST array from user, i.e. URL input, and CLEAN IT!
function CleanRequest () {
	$vars = array();
	while( list($var, $value) = each($_REQUEST) ) {
		$vars[CleanUserInput($var)] = CleanUserInput($value);
	}
	return $vars;
}


// Clean user entered item
function CleanUserInput (&$value) {
	// Stripslashes
	if (get_magic_quotes_gpc() && !is_array($value)) {
		$value = stripslashes($value);
	}
	/*
	// Goddamn! If we use mysqli_real_escape_string on everything, we get /" stuff in our text
	$value = mysqli_real_escape_string($LINK, trim($value));
	// Restore newlines and carriage returns
	$value = str_replace ('\n', "\n", $value);
	$value = str_replace ('\r', "\r", $value);
	*/
	return $value;
}

function GetFormInput () {
	global $Themes;
	//global $HTTP_POST_VARS;
	//global $HTTP_GET_VARS;
	global $DATABASES;
	global $msg, $error;
	global $LINK;

	$DEBUG = 0;
	$DEBUG && print __FUNCTION__.":";

	//$vars = count($HTTP_POST_VARS) > 0 ? $HTTP_POST_VARS :
	//		(count($HTTP_GET_VARS) > 0 ? $HTTP_GET_VARS : array() );
	$vars = $_REQUEST;

	$cleanvars = array();
	reset($vars);
	$action = '';
	$table = '';

	$DEBUG && var_dump($_REQUEST);
	
	// basic values used for searches
	// intval() is good for ID's
	$idVars = array ('ArtistID', 'GroupID', 'ID', 'ImageID', 'PriceSetID', 'ProjectID', 'SupplierID');
	foreach ($idVars as $v) 
		{
		if (isset($vars[$v]))
			{
			$vars[$v] = intval(trim($vars[$v]));
			$DEBUG && print __FUNCTION__.": $v = {$vars[$v]}<BR>";
			}
		}
	
	// Which keys do we use? Let's track for a bit... ****
	// TEMP!!!
	//$keys = array_keys($vars);
	//fp_error_log(__FUNCTION__.": Keys: ".join(", ", $keys), 3, FP_ACTIVITY_LOG);
	
	// SANITIZE table names: don't allow table to be anything else
	// Is $table in $DATABASES? If not, error and set to ""
	isset($vars['table']) && $vars['table'] = VerifyTableName($vars['table']);

	while( list($key, $value) = each($vars) ) 
		{
		$DEBUG	&& print "$key = $value<BR>";
		// Stripslashes
		if (get_magic_quotes_gpc() && !is_array($value)) {
			$DEBUG && print __FUNCTION__ .": Strip $value<BR>";
			$value = stripslashes($value);
		}
		
		$key = CleanUserInput($key);
		if (is_array($value))
			{
			array_walk($value,'CleanUserInput');
			$cleanvars[trim($key)] = $value;
			}
		else
			{
			$value = CleanUserInput($value);
			if (isAction($key))
				{
				$action = $value;
				}
			elseif (trim($key) == "table") 
				{
				$table = $value;
				}
			else 
				{
				$cleanvars[trim($key)] = $value;
				}
			}
		}
	$results['vars'] = $cleanvars;
	$results['actions']['action'] = $action;
	$results['actions']['table'] = $table;

	$DEBUG	&& var_dump ($cleanvars);

	return $results;
}


function VerifyTableName ($table = "") {
	global $msg, $error;
	global $LINK;
	global $DATABASES;
	if (!in_array($table, $DATABASES))
		{
		$table = "";
		$e = "Table \"{$table}\" does not exist.";
		$error .= "$e<br>";
		fp_error_log(__FUNCTION__.": Called by: ".getCallingFunction()." $e", 3, FP_ERROR_LOG);
		}
	return $table;
}

// Given commands & inputs from (usually from a web page form),
// do appropriate actions.
// DEPRECATED...IT TURNS OUT, HANDLEACTION DOES ALL THE WORK ANYWAY.
function ProcessInputs ($actions, $vars) {
	global $Themes;
	$DEBUG = false;
	global $msg, $error;
	global $LINK;

	$page = "";
	if ($DEBUG)
		$error .= "<HR>BEGIN ProcessInputs<BR>";
	$action = $actions['action'];
	if (! empty($action))
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": Action = $action<BR>";
	$page = HandleAction ($actions, $vars);
	if ($DEBUG)
		$error .= "<HR>END ProcessInputs<BR>";
	return $page;
}



//------------------
/*
	Given an array of name=>value, replace all coded variables in a string OR an array
	with the values.
	If $arr is a string, a string will be returned.
	If $arr is an array, an array will be returned (keys are kept!)
	Both $varname and {varname} are acceptable. The $ form is legacy, and it's
	better to use the {varname} form.
	$showme : if set, debug message about what was replaced
	$fixslashes: if true, replace " and ' in each text value with html entities
*/
function Substitutions ($arr, $subs, $fixquotes=false) {
	global $msg, $error;
	global $LINK;
	//	global $testcounter;

	$DEBUG = 0;
	$DEBUG2 = 0;
	$TIMER = 0;
	$TIMER && $starttime = microtime(true);
	
	if (!is_array($arr)) {
		$returnarr = false;
		$arr = array($arr);
	} else {
		$returnarr = true;
	}
	
	reset ($arr);
	while (list($key,$text) = each($arr)) {
		if ($text && is_array($subs)) {
			$TIMER && $starttime = microtime(true);
			// Find all constants in the text
			$p ='/(\\' . FP_FIELDSTART . "(\w*?)" . '\\'.FP_FIELDSUFFIX.')/i';
	
			$matches = array();
			// NOTE: join $subs allows us to know if any subs also have {var} in them
			preg_match_all ($p, $text . join('',$subs), $matches);
	
			$replacements = array_intersect_key (array_change_key_case($subs), array_change_key_case(array_flip(array_unique($matches[2]))));
	
			//$replacements = array_map ("trim", $replacements);
	
			$DEBUG && print "THINGS TO REPLACE:".ArrayToTable (array_flip(array_unique($matches[2])));
			$DEBUG && print "replacements:".ArrayToTable ($replacements);
			//$DEBUG && print "replacements:".ArrayToTable ($replacements);
	
			//$DEBUG && $error .=  __FUNCTION__.": PATTERNS IN THE TEXT...<br>";
			//$DEBUG && $error .=  ArrayToTable (array_flip(array_unique($matches[2])));
	
			if ($fixquotes) {
				$replacements = ReplaceQuotesWithHTML ($replacements);
				$DEBUG && print "FIX QUOTES IS ON!<BR>";
			}
	
			if (is_array($replacements)) {
				$keys = array_keys($replacements);
				$patterns = array_map ("make_patterns", $keys);
				$replacements = array_map("fix_dollar_signs", $replacements);
	
				$text = preg_replace ($patterns, $replacements, $text);
				$DEBUG2 && (strstr ($text,'$') &&  print $text);
	
				if ($TIMER) {
					$t = round (microtime(true) - $starttime, 3);
					if ($t>0.01) {
						$error .= __FUNCTION__.__LINE__.": Time elapsed: ".round (microtime(true) - $starttime, 3) . "<BR>";
					}
				}
			}
		}
		$arr[$key] = $text;
	}
	$returnarr
		? $text = $arr
		: $text = join("", $arr);
	return $text;
}


function ReplaceQuotesWithHTML ($arr, $keys = array ()) {
	if (is_array($arr)) {
		$keys || $keys = array_keys ($arr);
		foreach ($keys as $key) {
			if ($key != "Params")
				$arr[$key] = str_replace('"', '&quot;', $arr[$key]);
			$arr[$key] = str_replace("'", '&#039;', $arr[$key]);
		}
	} else {
		$arr = str_replace('"', '&quot;', $arr);
		$arr = str_replace("'", '&#039;', $arr);
	}
	return $arr;
}

function fix_dollar_signs($text) {
	$text = str_replace('$','\$',$text);
	return $text;
}

function make_patterns($item) {
	global $msg, $error;
	global $LINK;
	$item = str_replace('$','\$',$item);
	$item = '/\\' . FP_FIELDSTART . $item . '\\'.FP_FIELDSUFFIX.'/i';
	return $item;
}



// Replace unmatched or unused snippets in a text
// THIS ONLY WORKS WITH {} coding.
function DeleteUnusedSnippets ($text) {
	global $msg, $error;
	global $LINK;

	// dump any {} codes
	$text = preg_replace("/(\{\w*?\})/","", $text);


	return $text;
}


// Scan a text and replace all occurances of {snippetname} with the
// matching snippet.
// Instead of replacing one-by-one, we use the Substitutions function
// to do it all at once.
// Check for FP constants inside the snippets: we can use these to modify snippet names,
// e.g. {MYCONST}_form will get difference forms depending on the system constant.
function ReplaceAllSnippets ($text, $ignore = array()) {
	global $Themes;
	global $SNIPPETS, $SNIPPETSDIR;
	global $BASEDIR, $LOGS;
	global $FP_CONSTANTS;
	global $msg, $error;
	global $LINK;

	$DEBUG = 0;
	$TIMER = 0;

	$TIMER && $starttime = microtime(true);

	if (empty ($text))
		return "";

	$allsnippets = array ();
	$pattern = '/\{(\w*?)\}/';
	$constkeys = array_keys($FP_CONSTANTS);

	// We count to five, allowing five sub-references for unknown snippets.
	// That should really take care of anything!
	$k=0;
	// we build a list of non-snippets, {x} where x isn't a snippet but
	// something else that is replaced. We then ignore these, instead of
	// trying to replace them repeatedly.
	while (($k < 5) AND preg_match_all ($pattern, $text, $matches)) {
		$k++;
		foreach ($matches[1] as $match) {
			if ($ignore[$match])
				continue;

			// Is it a constant?
			if (in_array($match, $constkeys)) {
				// replace with constant
				$allsnippets[$match] = $FP_CONSTANTS[$match];
			} else {
				$value = FetchSnippet ($match);
				$DEBUG && $value && print "$k) found possible snippet: $match = ".htmlentities(substr($value,0,20))." ...<BR>";
				// Don't delete unknown variables; they might be system
				// vars or something else. The $k count prevents
				// endless looping.
				if (! empty ($value)) {
					$allsnippets[$match] = $value;
					$DEBUG && print "Replaced $match<BR>";
				} else {
					$ignore[$match] = $match;
				}
				$DEBUG && $msg .= "Should replace {$match}<BR>";
			}
		}
		$text = Substitutions ($text, $allsnippets);
		// some snippets insert system variables
		// $text = ReplaceSysVars ($text);
	}
	$TIMER && print __FUNCTION__.__LINE__.": Time elapsed: ".round (microtime(true) - $starttime, 3) . "<BR>";
	return $text;
}


/* Get a snippet (of text) */
/* If the $ID is an integer, it will be used as the id */
/* otherwise, $ID is assumed to be the Title of the snippet, i.e. find by name */
/* IF the snippet is not found in the current theme, look in the default theme.
	This makes it very easy to build new themes based on default! */
function FetchSnippet ($snippetID, $myThemeID = null ) {
	global $Themes;
	global $SNIPPETS, $SNIPPETSDIR, $JSDIR;
	global $BASEDIR, $LOGS;
	global $msg, $error;
	global $LINK;

	$DEBUG = 0;

	// we get blank requests sometimes...don't waste time on them

	if ($snippetID == "") {
		//fp_error_log("Error: Request for empty snippet '$snippetID'.", 3, FP_ERROR_LOG );
		return ;
	}

	// We also get requests for large strings, actually tests to see
	// if the string is a snippet. Just skip them.
	if ((strlen ($snippetID) > 64) or (preg_match ("/\s/", $snippetID))) {
		$DEBUG && $error .= __FUNCTION__.": Found a non-snippet ID = ". htmlentities ( substr ($snippetID,0,64) ). "....<BR>";
		return ;
	}

	// Reserved words used by FetchFormatSet
	$reserved = array ('separator_format', 'block_format', 'item_format', '{list}');
	if (in_array($snippetID, $reserved))
		return ;

	isset($_SESSION['theme']) || $_SESSION['theme'] = DEFAULT_THEME;

	$defaultTheme = FP_THEMES_DIR . "/" . DEFAULT_THEME . "/";

	$myThemeID || $myThemeID = $_SESSION['theme'];

	$myTheme = $Themes->FP_Themes[$myThemeID];
	if (isset($myTheme['is_variation']) && $myTheme['is_variation']) {
		$altpath = $myTheme['path'];
		// Get main theme for all snippets, variations are only CSS files
		$myTheme = $Themes->FP_Themes[$myTheme['theme_id']];
	}

	$DEBUG && $msg .= __FUNCTION__.": Theme is really ". $myTheme['name'] ."<br>";


	$snippetID = strtolower($snippetID);
	isset ($myTheme['snippets'][$snippetID]) && $snippet = $myTheme['snippets'][$snippetID];
	// If missing, try the default folder
	if (empty ($snippet)) {
		$snippet = $Themes->FP_Themes[DEFAULT_THEME]['snippets'][$snippetID];
		$DEBUG && $msg .= __FUNCTION__.": Fetch $snippetID from <i>default</i><br>";
	} else {
		$DEBUG && $msg .= __FUNCTION__.": Fetch $snippetID from <i>$myThemeID</i><br>";
	}

	//$DEBUG && ($snippet || print __FUNCTION__.": EEK! $snippetID does not exist in theme $myTheme!<BR>");

	/* if a snippet contains a reference to another snippet, insert it (recursive) */
	// Of the form, <snippet="name"> or <snippet="name" />

	$count = preg_match_all ("/< *snippet.*?= *('|\")(.*?)\\1\ *\/?\>/i", $snippet, $matches);
	for ($i=0; $i< count($matches[0]); $i++) {
		$found_code = $matches[0][$i];
		$snippet_id = $matches[2][$i];
		$insert_snippet = FetchSnippet ($snippet_id);
		$snippet = str_replace ($found_code, $insert_snippet, $snippet);
	}

	// Error if snippet doesn't exist...useful for debugging.
	//$snippet || $error .= __FUNCTION__.": Snippet not found for ID=$snippetID<BR>";

	return $snippet;
}

//Replace each array key with it's value in the supplied string
//If array keys don't have a prefix, use the $prefix

/*
FetchFormatSet();
Fetch a group of snippets, all with the same FormatSetID
return an array of snippets, or text.
If you need a format set for FetchCascade, the array needs sub-arrays.
So, each table is a title, and the FormatSetID is the name of the sub-array
Example:
	An entry in the Snippets table could include these fields:
		title = block_format
		FormatSetID = projects_page
		FormatSetIndex = Artists
	This would create a formatting entry in the array:
		$formatset [$ARTISTS] => array('block_format' => 'snippet text');
	FetchCascade requires each table to have an array containing
	'block_format' and 'item_format' and 'separator_format'
	Then, FetchCascade would use this entry for the $ARTISTS table, assuming
	it calls upon the Artists table.
*/	

function FetchFormatSet ($FormatSetID) {
	global $Themes;
	global $SNIPPETS, $BASEDIR;
	global	$msg, $error;

	$DEBUG = false;

	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": BEGIN<BR>";
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": ID=$FormatSetID<BR>";

	$formatset = array();

	$text = FetchSnippet ("set_".$FormatSetID);

	// NEW version...get a file from snippets directory
	// Get a text file of entries
	// Each line is tab-separated, of format:
	//		0-FormatSetIndex	1-Title 2-Text
	//	the FormatSetIndex is the name of the table searched in FetchCascade
	//	the Title is the name of the name of the formatting

	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": Got format file<BR>";

	$lines = explode("\n", $text);
	reset ($lines);
	while (list($linenum, $rawline)	= each($lines)) {
		$rawline = trim ($rawline);
		if ($rawline) {
			$DEBUG && $msg .= "Rawline: [".str_replace ("{", "&#123;", htmlentities($rawline))."]<BR>";
			$line = explode("\t", trim($rawline));
			$DEBUG && $msg .= __FUNCTION__ .":".__LINE__ . ": ".ArrayToTable($line) . "<BR>";
			$i = null;
			$t = null;
			$s = null;
			isset($line[0]) && $i = $line[0];
			isset($line[1]) && $t = $line[1];
			isset($line[2]) && $s = $line[2];
			// Is [2] a snippet or real text?
			// if the entry is {list} it is NOT a snippet!
			if (preg_match ("/^\{.*\}$/", $s) and !(preg_match ("/\{list\}/i", $s))) {
				$DEBUG = false;
				$DEBUG && $msg .= "<BR>".__FUNCTION__.__LINE__." : replacing snippet--> $s<BR><BR>";
				$s = trim($s, "{}");
				$s = FetchSnippet ($s);
			}
			if ($i.$t) {
				$formatset[$i][$t] = $s;
			} else {
				fp_error_log(__FUNCTION__.": Problem with the formatting set definition 'set_{$FormatSetID}', row contents = [$i][$t][$s].", 3, FP_ERROR_LOG);
			}
		}
	}

	$DEBUG && $msg .= ArrayToTable($formatset)."<BR><BR>";
	return $formatset;
}

/*
FetchCascade();

Notes:
	The items in '$tables' are the names of tables, as used in a SELECT query.
	Therefore, you can also set an item to 'table1, table2' for a joint select.

	Multiple calls to table: If a table item ends with # and a number, e.g. mytable#2, 
	then the #x is stripped, but differentiates between an earlier entry
	of the same table. That is, this allows you to call the same table 
	twice, but still have formatting/where/order different for each call.

	Snippet Calls: if any field access is called "SnippetID", then the corresponding
	snippet text is also access and tossed into the results as "Snippet".
	
	$variation is a flag used to choose different kinds of data elements, e.g. different versions of images that are stored as one item in the databases. It was designed to allow us to ask for a framed, normal, or matted photo.
	
	$rowlength allows you to insert a text snippet between rows. For example,
	instead of one long row of pictures, you could set $rowlength to 3, and 
	use line break between the rows. "0" means no row break.
	
	
	Format Sets contain tab separated lines of text for different kinds of formatting. 
	See line 775 for where these are used.
	
	
An example of FetchCascade:
	
	$tables = array ('Projects', 'Parts', $ARTISTS, 'Parts#2');
	
	$sets	= array ("Projects"		=>	"*",
					"Parts"	=>	"*",
					$ARTISTS	=>	"*",
					"Parts#2"	=>	"*, PartID AS SnippetID",
					);
	$projects_where =	"Active = 'yes'";
	$parts_where	=	"ProjectID = '\$Projects_ID' 
						AND PartTable = $ARTISTS";
					
	$artists_where	=	"Artists.ID = '\$Parts_ArtistID' AND Active = 'yes'";
	$parts_where2	=	"ProjectID = '\$Projects_ID' 
						AND Parts.ArtistID = '\$Artists_ID'
						AND PartTable = 'Snippets'";
	
	$wheres = array ("Projects"		=>	$projects_where,
					"Parts"	=>	$parts_where,
					$ARTISTS	=>	$artists_where,
					"Parts#2"	=>	$parts_where2,
					);
	
	$orders = array ("Projects"		=>	"Title",
					"Parts"	=>	"",
					$ARTISTS	=>	"Lastname, Firstname",
					"Parts#2"	=>	""
					);

	// $params is an array of parameters (not the serialized value stored in the database!)
	$params = array ("Projects" =>	$params,
					"Parts"		=>	null,
					$ARTISTS	=>	null,
					"Parts#2"	=>	null,
					);				
	$formats = FetchFormatSet ('stories_page');
	
	$skip_empty_rows = false;
	
	$variation = "framed";
	
	$output = FetchCascade ($tables, $sets, $wheres, $orders, $formats, $limits, $fields, $skip_empty_rows, $variation);
	echo $output;

*/

function FetchCascade ($tables, $sets = array(), $wheres = array(), 
		$orders = array(), $formats = array(), $limits = array(), 
		$fields = array(), $skip_empty_rows = FALSE, $variation = null, 
		$params = array (), $artist_info = array ()) 
				{
	global $Themes;
	global $BASEDIR;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	global $msg, $error;
	global $LINK;
	global $default_size, $slide_size, $default_matte_size, $slide_matte_size;	// array (x,y) of size of display picture in a gallery
	global $MAILED_DIR, $PHOTOS_GALLERY, $SLIDES, $THUMBNAILS, $MATTED, $FRAMED, $ORIGINALS, $BASEDIR, $LOGS;

	// This is set by gallery.php at the top
	global $myGroup;

	// Timing for testing
	$TIMER = 0;

	$TIMER && $starttime = microtime(true);

	$DEBUG = 0; //***
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": BEGIN<BR>";
	$showFields = false; // debugging, shows replacement fields for snippets


	$GALLERY_VIDEO_CODE = FetchSnippet ("gallery_video");
	$GALLERY_IMAGE_CODE = FetchSnippet ("gallery_img");
	$GALLERY_VIDEO_POPUP = FetchSnippet ("gallery_video_popup_link");


	// GET PARAMS VALUES
	// These can reset the theme and other settings.
	//$rowlengths[$tablekey] > 0 ? $rowlength = $rowlengths[$tablekey] : $rowlength = 0;
	if ($params) {
		$rowlength = $params[FP_PARAM_GALLERY_GROUPING];
		$fields['rowlength'] = $rowlength;
		if ($params[FP_PARAM_GALLERY_SPACING] > 0) {
			$fields['gallery_spacing'] = "width:".$params[FP_PARAM_GALLERY_SPACING]."px;";
		} else {
			$fields['gallery_spacing'] = "display:none;";
		}
		$params[FP_PARAM_GALLERY_GROUP_SPACING] += 0;
		if ($params[FP_PARAM_GALLERY_GROUP_SPACING] > 0) {
			$fields['gallery_group_spacing'] = "width:".$params[FP_PARAM_GALLERY_GROUP_SPACING]."px;height:".$params[FP_PARAM_GALLERY_GROUP_SPACING]."px;";
		} else {
			$fields['gallery_group_spacing'] = "display:none;";
		}

		$showcaptions = $params[FP_PARAM_GALLERY_SHOWCAPTIONS];
		$scale = $params[FP_PARAM_GALLERY_SCALING];
		$scale || $scale = 1;	// don't allow zero!
	} else {
		$rowlength = 0;
		$fields['rowlength'] = 1;
		//$picturespacing = 0;
		$fields['gallery_spacing'] = "";
		$fields['gallery_group_spacing'] = "";
		$showcaptions = 0;	// 0 = popup captions
		$scale = 1.00;	// keep images at 100% of size
	}
	$default_size_scaled = $default_size;
	$default_size_scaled[0] = $default_size_scaled[0] * $scale;
	$default_size_scaled[1] = $default_size_scaled[1] * $scale;

	// Show/Hide link to commenting system
	if ($params[FP_PARAM_GALLERY_COMMENTS]) {
		$fields['COMMENTLINK'] = FetchSnippet('commentlink');
		//$fields['COMMENTLINK'] = "";
	} else {
		$fields['COMMENTLINK'] = "";
	}


	// If $showcaptions is set, then captions are shown by default
	// 'FP_DISPLAY_CAPTION_SHOW_HIDE' is a style setting for 'display:DISPLAY_CAPTION'
	// 'FP_DISPLAY_CAPTION_FLAG' is part a javascript call, e.g. myjs('DISPLAY_CAPTION_FLAG', param1)
	// to tell the the javascript whether or not to hide/show the caption
	// If 'false' then captions are hidden by default
	switch ($showcaptions) {
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

	// Used to make javascript for ordering prints
	// We load it here so it isn't repeatedly loaded,
	// since it doesn't change.
	// It's the default ordering code, in case the artist doesn't have custom code
	$defaultShoppingCartCode = FetchSnippet ("shoppingcart_wrapper");
	$defaultPrintSaleCode = FetchSnippet ("javalink_order_wrapper");
	$defaultBuyPrintButton = FetchSnippet ("DefaultBuyPrintButton");
	$popupBuyPrintButton = FetchSnippet ("PopupBuyPrintButton");
	$defaultCreditLine = FetchSnippet ("gallery_picture_credit");
	$defaultCaptionFormat = FetchSnippet ("gallery_picture_caption");


	// Get print sales info for each artist:
	// Get HTML code for all artists, get PayPal data for each artist, get default item price for PayPal
	// If there's no PayPal business set, don't show a PayPal button
	// PrintSalesID is the artist's ID for the print sales vendor, e.g. the ProviderID for Pictopia
	$PrintSalesID = array ();
	$DEBUG && $msg .= "***** artist_info: ".count($artist_info) . "<BR>";
	if (!$artist_info)
		$artist_info = FetchAllArtistArrays () ;

	foreach ($artist_info as $artist) {
		$artistID = $artist['ID'];
		$artist ['PrintSaleCode'] ? $printSaleCode[$artistID] = $artist ['PrintSaleCode'] : $printSaleCode[$artistID] = $defaultPrintSaleCode;
		$artist ['PayPalBusiness'] ? $artistPayPalBusiness[$artistID] = $artist ['PayPalBusiness'] : $artistPayPalBusiness[$artistID] = "";
		$artist['PrintSalesID'] ? $PrintSalesID[$artistID] = $artist ['PrintSalesID'] : $PrintSalesID[$artistID] = "";
		$DEBUG && ($artist['PrintSalesID'] && $msg .= "YES, artist $artistID has a PrintSalesID<BR>");
		$artist['Vendor'] ? $Vendor[$artistID] = $artist ['Vendor'] : $Vendor[$artistID] = 0;
		$artist['Ecommerce'] ? $Ecommerce[$artistID] = $artist ['Ecommerce'] : $Ecommerce[$artistID] = 0;
		// Get default price set for this artist
		// If not set, use default = 1
		$artist['DefaultPriceSetID'] || $artist['DefaultPriceSetID'] = 0;
		if ($artist ['DefaultPriceID']) {
			$price = FetchRowsByValue (DB_PRICES, "Price", "ID", $artist ['DefaultPriceID'], '', true);
			$itemAmount[$artistID] = $price['Price'];
		}
	}

	$output = "";
	$DEBUG && $msg .= "***** PrintSalesID: ".count($PrintSalesID) . "<BR>";

	// Get the table, and a handy key (the table value can be unwieldy)
	// If there is no handy key, just use the table name.
	reset ($tables);
	list ($tablekey, $table) = each($tables);
	$table = array_shift($tables);

	$tablename = preg_replace("/(.*)\#\d+/", "\\1", $table);
	$table = Substitutions ($table, $fields);
	//$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": Table = ".htmlentities($table)."<BR>";
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": <HR><h3>Fetching Table $table</h3>\n";
	//$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": Fields : ".ArrayToTable($fields)."<BR>\n";

	if (empty($tablekey))
		$tablekey = $table;

	$set = "*";
	if (! empty ($sets[$tablekey]))
		$set = $sets[$tablekey];

	$where = "";
	if (! empty($wheres[$tablekey]))
		$where = " WHERE ".$wheres[$tablekey];

	$order = "";
	if (! empty ($orders[$tablekey]))
		if (preg_match("/^group( +)by/i", $orders[$tablekey], $o)) {
			$order = $orders[$tablekey];
		} else {
			$order = " ORDER BY ".$orders[$tablekey];
		}

	$limit = "";
	if (!empty($limits) && !empty($limits[$tablekey]))
		$limit = " LIMIT ".$limits[$tablekey];
	$format = $formats[$tablekey];
	//$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": Format = $tablekey" . ArrayToTable($formats[$tablekey]) . "<BR>";

	$query = "SELECT $set FROM $tablename $where $order $limit";
	$query = Substitutions ($query, $fields);


	$TIMER && $error .= __FUNCTION__.__LINE__.": Time elapsed: ".round (microtime(true) - $starttime, 3) . "<BR>";
	(round (microtime(true) - $starttime, 3) > .1) && $starttime = microtime(true);
	$DEBUG && $msg .= __FUNCTION__ .":".__LINE__ . ": QUERY<DIV style='border:2px solid red;'>$query</div><BR><BR>\n";
	$result = mysqli_query ($LINK, $query);
	$TIMER && $error .= __FUNCTION__.__LINE__.": Time elapsed: ".round (microtime(true) - $starttime, 3) . "<BR>";
	(round (microtime(true) - $starttime, 3) > .1) && $starttime = microtime(true);


	// Get field types and names
	$fieldtype = array ();
	$fieldnames = array();
	if ($result) {
		while ($finfo = $result->fetch_field()) {
			$fieldnames[$finfo->name] = $finfo->type;
		}
	}

// 	
// 	$result
// 			? $mysqlnumfields = $result->field_count
// 			: $mysqlnumfields = 0;
// 	$fieldtype = array ();
// 	
// 	
// 	
// 	for($i=0; $i<$mysqlnumfields; $i++) {
// 		$res = mysqli_fetch_field_direct($result,$i);
// 		$mysqlfieldname = $res->name;
// 		$fieldtype[$mysqlfieldname] = $res->type;
// 	}



	//	$got_rows = mysqli_fetch_array($result, MYSQLI_ASSOC);
	//	$result && mysqli_data_seek ($result, 0);	// important to reset this to 0 for the while loop, below!
	$got_rows = false;
	$result && $got_rows = mysqli_num_rows($result);

	if (! $got_rows) {
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": NO RESULT FOR $query<BR>";
	}

	if ((! $skip_empty_rows) OR $got_rows) {
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": $tablekey: OK, we have data or skip_empty_rows is false<BR>";
		if (trim($formats[$tablekey]['block_format'])=='') {
			$blockformat = FetchSnippet ('block_format');
		} else {
			$blockformat = FetchSnippet ($formats[$tablekey]['block_format'])	// first, assume we've got a snippet entry
					OR $blockformat = $formats[$tablekey]['block_format'];			// if that fails, use the text as is.
		}

		if (empty($formats[$tablekey]['item_format'])) {
			$itemformat = FetchSnippet ('item_format');
		} else {
			$itemformat = FetchSnippet ($formats[$tablekey]['item_format']) // first, assume we've got a snippet entry
					OR $itemformat = $formats[$tablekey]['item_format'];			// if that fails, use the text as is.
		}

		$itemseparator = "";
		if (trim($formats[$tablekey]['separator_format']) == '') {
			$itemseparator = FetchSnippet ('separator_format');
		} else {
			$itemseparator = FetchSnippet ($formats[$tablekey]['separator_format']) // first, assume we've got a snippet entry
					OR $itemseparator = $formats[$tablekey]['separator_format'];			// if that fails, use the text as is.
		}

		if (trim($formats[$tablekey]['row_format']) == '') {
			$rowformat = FetchSnippet ('row_format');
		} else {
			$rowformat = FetchSnippet ($formats[$tablekey]['row_format'])	// first, assume we've got a snippet entry
					OR $rowformat = $formats[$tablekey]['row_format'];			// if that fails, use the text as is.
		}

		$list = "";
		$itemcount = 0;

		$DEBUG = 0;
		$DEBUG && $msg .= __FUNCTION__.":".__LINE__."DEBUG ON<BR>";

		$prev = null;
		$next = null;

		$lines = array ();
		$k = 1;

		// Get all records
		// This allows us to look ahead (for prev/next)
		while ($line = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
			$line && $lines[$k++] = $line;
		}

		$DEBUG && $msg .= __FUNCTION__.__LINE__.": Found $got_rows records<BR>";

		// max lines to avoid infinite loops is 500
		$column = 0;

		/*
		 * PROJECTS table (Exhibitions)
		 */
		// Get individual image settings for a project, e.g. size/position
		if ($table == DB_PROJECTS) {
			$project = new FPProject ($LINK, $lines[1]['ID']);
			//$allSettings = unserialize($project->params[FP_PARAM_GALLERY_ITEM_SETTINGS]);
			$params['ProjectPictureSettings'] = $project->FetchAllProjectPictureSettings();
		}
				

		
		


		reset ($lines);
		while ( ($column < 500) && list($column, $line)	= each($lines) ) {

			
			// Values for Substitutions replacement to be done BEFORE $fields are put in, because
			// they use $fields, e.g. gallery_img or gallery_video snippets.
			$prefields = array();

			$TIMER && $error .= __FUNCTION__.__LINE__.": Time elapsed; column = $column: ".round (microtime(true) - $starttime, 3) . "<BR>";

			if ($column == 499) {
				fp_error_log(__FUNCTION__.": Infinite loop! Table=$tablekey.", 3, FP_ERROR_LOG);
				$error .= "Warning: infinite loop in FetchCascade with table $table<BR>";
			}
			$rowtext = "";
			$prefix = $tablekey."_";

			$DEBUG && $msg .= __FUNCTION__.__LINE__.": Column = $column<BR>";
			
			
			/* ==== GET $FIELDS === */
			/* Fetch values for this row */
			$k=0;
			reset ($line);
			while (list($key, $var)	= each($line)) {
				$keyname = "{$prefix}$key";
				// If we detect htmlentities, then convert UTF-8 string fields to html entities
				// The string MIGHT be a code or something, so we must be careful about this
				if ($fieldtype[$key] == "string" && $key != "Params") {
					if (htmlentities ($var) != $var) {
						$var = utf8_to_html($var);
						$line[$key] = $var;
						$DEBUG && $msg .= __FUNCTION__.__LINE__.": Convert from UTF to HTML Entities".$fieldtype[$key] . " $tablekey : $key : $var<BR>";
					}
				}
				$fields[$keyname] = $var;
				
				$fieldnames[$finfo->name] = $finfo->type;

				$res = mysqli_fetch_field_direct($result, $k);
				$fieldtable = $res->table;

				$k++;
				//print __FUNCTION__.__LINE__.":$k) fieldtable=$fieldtable, key=$key, tablekey=$tablekey, table=$table<BR>";
				$fields[$tablekey."_".$key] = $var;
			}
			
			
			$TIMER && $error .= __FUNCTION__.__LINE__.": Time elapsed; column = $column: ".round (microtime(true) - $starttime, 3) . "<BR>";


			// DEBUGGING
			if ($showFields) {
				is_array ($fields) || var_dump ($fields);
				$msg .= "<hr>".__FUNCTION__.__LINE__."<BR>Fields for replacement:<BR>".implode(" , ", $fields)."<HR>";
			}

			// Variations for solo vs. group galleries
			// Credit disappears when it's a solo gallery
			// Check all members of group
			if ( $myGroup->is_solo ) {
				$fields["Credit"] = "<!--solo show-->";
				$fields["SOLO_SUFFIX"] = "_solo";
			} else {
				$fields["SOLO_SUFFIX"] = "";
				$f = array (
						"credit"	=>$line['Fullname']
				);
				$fields["Credit"] = Substitutions ($defaultCreditLine, $f);
			}

			// In Statement and Description fields, convert returns to <br>
			if (isset ($fields[$PROJECTS.'_Statement']))
				$fields[$PROJECTS.'_Statement'] = str_replace ("\n", "<br>\n", $fields[$PROJECTS.'_Statement']);
			if (isset ($fields[$PROJECTS.'_Description']))
				$fields[$PROJECTS.'_Description'] = str_replace ("\n", "<br>\n", $fields[$PROJECTS.'_Description']);

			
			// If this is a single-user gallery, then we don't need to show the artist's
			// name in a gallery..
			if ( $myGroup->is_solo && FP_HIDE_NAME_SOLO_GALLERY) {
				$fields[$ARTISTS.'_Fullname'] = "";
			} else {
				// Get project owner's information into Artists_fieldname, e.g. Artists_Lastname
				if (isset ($fields[$PROJECTS.'_ArtistID'])) {
					$artist = FetchArtist ($fields[$PROJECTS.'_ArtistID']);
					while (list($k,$v) = each ($artist)) {
						$fields[$ARTISTS.'_'.$k] = $v;
					}
				}
			}

			// Previous/Next Buttons
			if ($column == 0) {
				$prev_link_ID = "";
				$next_link = $lines[$column+1];
				$next_link_ID = $lines[$column + 1]['PartID'];
			} elseif ($column >= count($lines)) {
				$prev_link = $lines[$column-1];
				$prev_link_ID = $lines[$column - 1]['PartID'];
				$next_link_ID = "";
			} else {
				$prev_link = $lines[$column-1];
				$prev_link_ID = $lines[$column - 1]['PartID'];
				$next_link = $lines[$column+1];
				$next_link_ID = $lines[$column + 1]['PartID'];
			}
			$prevnext = array ("prevID"=>$prev_link_ID,"nextID"=>$next_link_ID);

			//				$error .= __FUNCTION__.__LINE__.": ";
			//				var_dump ($prevnext);
			//				var_dump ($line[$column+1]);


			$fields['PrevLink'] = Substitutions (FetchSnippet("gallery_set_image_item_prev"), $prevnext);
			$fields['NextLink'] = Substitutions (FetchSnippet("gallery_set_image_item_next"), $prevnext);

			$fields['PrevLinkID'] = $prev_link_ID;
			$fields['NextLinkID'] = $next_link_ID;

			// Make Year and Month available
			$fields['YEAR'] = YearOf($line['CreatedDate']);
			$fields['MONTH'] = MonthOf($line['CreatedDate']);
			$fields['YEAR_DIVIDER'] = $fields['YEAR'] ? ", " : "";
			$fields['MONTH_DIVIDER'] = $fields['MONTH'] ? ", " : "";
			// Two ways of handling 'untitled' pix
			// Titling Option: Year set, but no title -> "Untitled"
			//if (YearOf($line['CreatedDate']) && !$fields[$prefix."Title"])
			//	$fields[$prefix."Title"] = "{FP:Untitled}";

			// Title Option 2: Year set, no title, no Year Divider
			if (YearOf($line['CreatedDate']) && !$fields[$prefix."Title"])
				$fields['YEAR_DIVIDER'] = "";

			$fields[$prefix.'YEAR'] = YearOf($line['CreatedDate']);
			$fields[$prefix.'MONTH'] = MonthOf($line['CreatedDate']);
			$fields[$prefix.'YEAR_DIVIDER'] = $fields['YEAR'] ? ", " : "";
			$fields[$prefix.'MONTH_DIVIDER'] = $fields['MONTH'] ? ", " : "";


			$TIMER && $error .= __FUNCTION__.__LINE__.": Time elapsed; column = $column: ".round (microtime(true) - $starttime, 3) . "<BR>";

			
			
			

			$DEBUG && $msg .= __FUNCTION__.__LINE__.": table=$table<br>";
			$DEBUG && $msg .= __FUNCTION__.__LINE__."PrintSalesID:" . ArrayToTable($PrintSalesID);

			/* 
			 * ==================================================
			 * IMAGES
			 * ==================================================
			 */
			if (preg_match ("/$IMAGES/", $table)) {
				// fields to be inserted into code with {} BEFORE the $fields is Substituted.
				
				isset ($fields[$prefix."Params"]) ? $imageParams = DecodeArrayFromDB($fields[$prefix."Params"]) : $imageParams = array ();

				$title = $fields[$prefix."Title"];
				// convert quotes
				$title = ReplaceQuotesWithHTML($title);

				$fields["{$prefix}Title"] = $title;
				$item_name = $title . " (".$fields[$prefix."RollID"]."-".$fields[$prefix."FrameID"] . ")";
				// If the field's table is Images, we get the ID
				// else we get ImageID
				if ($fieldtable == $IMAGES) {
					$item_number = $fields[$prefix."ID"];
				} else {
					$item_number = $fields[$prefix."ImageID"];
				}

				$artistID = $fields[$prefix."ArtistID"];

				//print __FUNCTION__.__LINE__.': '.$fields['Images_Fullname']."<BR>";
				//print __FUNCTION__.__LINE__.': '.$fields['Images_GroupID']."<BR>";

				/* 
				 * ==================================================
				 * Image settings stored in a Project, e.g. per image settings
				 * for size, position, etc. in a gallery
				 * ==================================================
				 */
				if ($DEBUG) {
					print __FUNCTION__.":".__LINE__;
					print "Item: $item_number<Br>";
					print var_dump($params['ProjectPictureSettings']);
				}
				// Is there a setting for this image?
				if (isset($params['ProjectPictureSettings'][$item_number])) {
					$imageExtraStyle = "";
					while(list($k,$v) = each ($params['ProjectPictureSettings'][$item_number])) {
						$fields[$prefix.$k] = $v;
						//print __FUNCTION__.__LINE__.": $prefix$k = $v<br>";
					
						/*
						Built a style element for the picture on the page, to apply these settings
						These aren't stored as style elements, because they could be anything      'id' => string '1415' (length=4)
						'originalTop' => int 0
						'originalLeft' => int 150
						'originalSizeWidth' => int 347
						'originalSizeHeight' => int 300
						'positionTop' => int 0
						'positionLeft' => int 150
						'width' => int 318
						'height' => int 275

						 */

						switch ($k) {
							case "positionTop":
								$imageExtraStyle .= " top: {$v}px;";
								break;
							case "positionLeft":
								$imageExtraStyle .= " left: {$v}px;";
								break;
							case "width":
								$imageExtraStyle .= " width: {$v}px;";
								break;
							case "height":
								$imageExtraStyle .= " height: {$v}px;";
								break;

							case "originalTop":
								$imageExtraAttrs .= " originalTop=\"$v\"";
								break;
							case "originalLeft":
								$imageExtraAttrs .= " originalLeft=\"$v\"";
								break;
							case "originalSizeWidth":
								$imageExtraAttrs .= " originalSizeWidth=\"$v\"";
								break;
							case "originalSizeHeight":
								$imageExtraAttrs .= " originalSizeHeight=\"$v\"";
								break;

							}

					}
					$fields[$prefix.'ImageExtraStyle'] = "{$imageExtraStyle} ";
					$fields[$prefix.'ImageExtraAttrs'] = "{$imageExtraAttrs} ";
				}
				
				
				
				
				// ---CAPTION---
				// Wrap a format around the Caption
				// Also, convert from UTF-8 to HTML Entities
				$caption = utf8_to_html(trim($fields[$prefix."Caption"]));

				if ($caption) {
					if (FP_CAPTION_SHOW_LINEBREAKS) {
						//$caption = preg_replace ('/\n/',"<br>\n", $caption);
						// replace the HTML entity code for "/n" ( &#10; ) with <br>
						$caption = preg_replace ("/\&\#10\;/","<br>\n", $caption);
					}
					$f = array("{$prefix}Caption" => $caption);
					$fields[$prefix."Caption"] = Substitutions ($defaultCaptionFormat, $f);
					// Display a show caption button?
					$fields['SHOW_CAPTION_BUTTON'] = FetchSnippet ("gallery_show_caption");
				} else {
					$fields["{$prefix}Caption"] = "";
					$fields['SHOW_CAPTION_BUTTON'] = "";
				}

				// Popup message below a picture
				// Don't do the unicode_to_entities, it's been done above for $fields['caption'] and $fields['Title'];
				$fields['PopupMsg'] = preg_replace("/\r|\n|\t/", "", Substitutions (FetchSnippet ("gallery_popupmsg"), $fields));

				//$fields["{$prefix}Caption_Encoded"] = utf8_to_html (preg_replace("/\r|\n|\t/", "", $caption));
				$fields["{$prefix}Caption_Encoded"] = utf8_to_html (preg_replace("/\r|\n|\t/", "", $caption));

				// -------
				// GALLERY WALL SIZE: Calculate visual matting settings

				if (isset ($fields[$prefix."URL"])) {

					$URL = $fields[$prefix."URL"];

					$isVideo = GetParam ($imageParams, FP_PARAM_IMAGE_IS_VIDEO);

					// replace {IMAGE_IMG_CODE}
					// If this is an image, then use the image code snippet
					// If it is video, use that.
					if ($isVideo) {
						$prefields['IMAGE_IMG_CODE'] = $GALLERY_VIDEO_CODE;
						$prefields['VIDEO_POPUP_LINK'] = $GALLERY_VIDEO_POPUP;

						$fields[$prefix."VideoCode"] = GetParam($imageParams,FP_PARAM_IMAGE_VIDEO_EMBED_CODE);
						$missing_picture = false;
						$imgsize = array (GetParam($imageParams,FP_PARAM_IMAGE_VIDEO_WIDTH), GetParam($imageParams,FP_PARAM_IMAGE_VIDEO_HEIGHT));
						$fields[$prefix."IsVideo"] = "IsVideo";
						$fields[$prefix."LightboxGroup"] = "video-player";
						$imgIsVideo = true;
						
					} else {
						$fields[$prefix."IsVideo"] = "";
						$prefields['IMAGE_IMG_CODE'] = $GALLERY_IMAGE_CODE;

						$fields[$prefix."LightboxGroup"] = "lightbox-mygroup";

						// Check if picture is missing (due to system error!)
						// If so, replace with a placeholder, and make sure it's not for sale!
						if (!file_exists ("$BASEDIR/$PHOTOS_GALLERY/$URL")) {
							$missing_picture = true;
							$URL = FP_SYSTEM_MISSING_PICTURE;
							$fields[$prefix."URL"] = $URL;
						} else {
							$missing_picture = false;
							// Size the pictures in the gallery spread
						}
	
						$imgsize = GetLocalImageSize("$BASEDIR/$PHOTOS_GALLERY/$URL");
						$imgIsVideo = false;
					}

					$fields['OriginalWidth'] = $imgsize[0];
					$fields['OriginalHeight'] = $imgsize[1];

					// Framing: If no frame, set width to zero (THIS IS DONE IN THE GALLERY.PHP SCRIPT)
					$framestyle = $fields[DB_PROJECTS . "_Framestyle"];
					$framewidth = $fields[DB_PROJECTS . "_Framewidth"];
					$framewidth>0 || $framestyle = 0;

					if ($framestyle) {
						$projectframewidth = $framewidth;	// not the same as the config.sys "framewidth", which has a setting for graphically drawing frames
					} else {
						$projectframewidth = 0;		// no frame style, then width = 0
					}

					// Resize image to fit into preferred gallery sizes, e.g. 300 high
					list ($new_width, $new_height) = CalcResizeImage ($imgsize[0],$imgsize[1],$default_size_scaled[0],$default_size_scaled[1]);

					// TESTING: Resize verticals to be same size as horizontals, just for showing. Enlargement looks bad!
					if (false) {
						if ($imgsize[0] < $imgsize[1]) {
							list ($new_width, $new_height) = CalcResizeImage ($imgsize[0],$imgsize[1],$default_size_scaled[1],$default_size_scaled[0]);
						} else {
							list ($new_width, $new_height) = CalcResizeImage ($imgsize[0],$imgsize[1],$default_size_scaled[0],$default_size_scaled[1]);

						}
					}

					$DEBUG = 0;
					$DEBUG && $msg .= __FUNCTION__.":".__LINE__;
					
					// $variation now indicates matting and nothing else.
					// IMG_Width / height are the size the picture WITH the matte around it
					// MattedWidth / height are the size of the picture INSIDE the matte
					if ($variation) {
						// matted
						list ($w, $h, $mattedW, $mattedH, $matteWidth, $matteWidthBottom) = GetDisplayMatteDimensions ($new_width, $new_height);
						$fields['IMG_Width'] = $mattedW;
						$fields['IMG_Height'] = $mattedH;
						$fields['MattedWidth'] = $w;
						$fields['MattedHeight'] = $h;
						// Matte Width so we can use a "border" style
						$fields['MatteWidth'] = $matteWidth;
						$fields['MatteWidthBottom'] = $matteWidthBottom;

						if ($DEBUG) {
							$r1 = $new_width / $new_height;
							$r2 = $w / $h;
							$msg .= __FUNCTION__.__LINE__.": [yes variation] variation = $variation : original ($new_width, $new_height) ($r1), image ($w,$h) ($r2), matted ($mattedW, $mattedH)<BR>";
						}
					} else {
						// not matted
						$fields['IMG_Width'] = $new_width;
						$fields['IMG_Height'] = $new_height;
						$fields['MattedWidth'] = $new_width;
						$fields['MattedHeight'] = $new_height;
						$fields['MatteWidth'] = 0;
						$fields['MatteWidthBottom'] = 0;
						$DEBUG && $msg .= __FUNCTION__.__LINE__.": [no variation] variation = $variation : original ($new_width, $new_height), image ($w,$h)<BR>";
					}

					$DEBUG = 0;

					// Framed width
					$fields['FramedWidth'] = $fields['IMG_Width'] + (2*$projectframewidth);
					$fields['FramedHeight'] = $fields['IMG_Height'] + (2*$projectframewidth);

					// Other measurements for pictures
					// Thumbnails
					// Don't scale these.
					$imgsize = GetLocalImageSize("$BASEDIR/$THUMBNAILS/$URL");
					$fields['ThumbnailWidth'] = $imgsize[0];
					$fields['ThumbnailHeight'] = $imgsize[1];



					// -------
					// SLIDE SIZES: Calculate Slide sizes for display (popup slides)
					// Final size (framed, matted) is controlled in config by $slide_size

					$imgsize = GetLocalImageSize("$BASEDIR/$SLIDES/$URL");
					list ($new_width, $new_height) = CalcResizeImage ($imgsize[0],$imgsize[1],$slide_size[0],$slide_size[1]);

					$DEBUG = 0;
					if ($variation) {
						list ($w, $h, $mattedW, $mattedH, $matteWidth, $matteWidthBottom) = GetDisplayMatteDimensions ($new_width, $new_height, FP_SLIDE_ADJUSTMENT);
						$fields['IMG_Slide_Width'] = $mattedW;
						$fields['IMG_Slide_Height'] = $mattedH;
						$fields['MattedSlideWidth'] = $w;
						$fields['MattedSlideHeight'] = $h;
						$fields['MatteWidthSlide'] = $matteWidth;
						$fields['MatteWidthSlideBottom'] = $matteWidthBottom;
						if ($DEBUG) {
							$r1 = $new_width / $new_height;
							$r2 = $w / $h;
							$msg .= __FUNCTION__.__LINE__.": variation = $variation : original ($new_width, $new_height) ($r1), image ($w,$h) ($r2), matted ($mattedW, $mattedH), matteWidth = {$matteWidth}<BR>";
						}
					} else {
						$fields['IMG_Slide_Width'] = $new_width;
						$fields['IMG_Slide_Height'] = $new_height;
						$fields['MattedSlideWidth'] = $new_width;
						$fields['MattedSlideHeight'] = $new_height;
						$fields['MatteWidthSlide'] = $matteWidth;
						$fields['MatteWidthSlideBottom'] = $matteWidthBottom;
						$fields['MatteWidthSlide'] = 0;
						$fields['MatteWidthSlideBottom'] = 0;
						$DEBUG && $msg .= __FUNCTION__.__LINE__.": variation = $variation : original ($new_width, $new_height), image ($w,$h)<BR>";
					}
					$DEBUG = 0;

					// Framed width
					$fields['FramedSlideWidth'] = $fields['IMG_Slide_Width'] + (2*$projectframewidth);

				}

				$fields['Picture_Popup_Width'] = $fields['SlideWidth'] + $projectframewidth;
				$fields['Picture_Popup_Height'] = $fields['SlideHeight'] + $projectframewidth;

				//---- Tear Sheet link

				$fields['TearSheet'] = Substitutions (FetchSnippet ("TearsheetLink"), array (
						"ProjectID"		=> $projectID,
						"ImageID"		=> $item_number
				));

				//---- SHOPPING CARTS, PICTOPIA LINKS, PAYPAL LINKS...
				// Artist must have a sales ID to be able to sell.
				$DEBUG && $msg .= __LINE__."<hr>Working on artist ID $artistID (" . $Ecommerce[$artistID].")<BR>";
				if (!$imgIsVideo && !$missing_picture && $PrintSalesID[$artistID] && $Ecommerce[$artistID]) {
					$projectID = $fields[DB_PROJECTS."_ID"];
			$TIMER && $error .= __FUNCTION__.__LINE__.": *Time elapsed; column = $column : ".round (microtime(true) - $starttime, 3) . "<BR>";

					$imageSalesInfo = ImageSalesInfo ($item_number, $projectID);
					$pricesetID = $imageSalesInfo['pricesetID'];
					$sizes = $imageSalesInfo['size'];
					$printsizes = $imageSalesInfo['printsizes'];
					$prices = $imageSalesInfo['PrintPrices'];
					$priceframed = $imageSalesInfo['priceframed'];
					$editionType = $imageSalesInfo['edition_type'];
			$TIMER && $error .= __FUNCTION__.__LINE__.": *Time elapsed; column = $column : ".round (microtime(true) - $starttime, 3) . "<BR>";

					

					//PRICERANGE
					// low and high prices to show a price range
					$low = null;
					$high = null;
					$prices && $low = formatPrice ($prices[0], 0);
					$prices && $high = formatPrice ($prices[count($prices)-1] );
					//$priceframed && $high = formatPrice ($priceframed[count($priceframed)-1], 0);
					$fields['lowprice'] = $low ;
					$fields['highprice'] = $high ;


					if ($low and $high) {
						if ($low == $high) {
							$pricerange = Substitutions (FetchSnippet ("gallery_set_images_item_oneprice"), array(
									"LOWPRICE"	=>	$low
							));
						} else {
							$pricerange = Substitutions (FetchSnippet ("gallery_set_images_item_pricerange"), array(
									"LOWPRICE"	=>	$low,
									"HIGHPRICE" =>	$high
							));
						}
					}

					// Available sizes
					if ($editionType == "original") {
						$availableSizes = Substitutions (FetchSnippet ("gallery_set_images_item_sizes_original"), array(
								"SIZELIST"	=>	implode (FetchSnippet ("gallery_set_images_item_sizelist_divider"), $printsizes)
						));
					} else {
						$availableSizes = Substitutions (FetchSnippet ("gallery_set_images_item_sizes"), array(
								"SIZELIST"	=>	implode (FetchSnippet ("gallery_set_images_item_sizelist_divider"), $printsizes)
						));
					}
					// LIMITED_EDITION
					// if $imageSalesInfo['totaledition'] == 0, then this is an open edition. Otherwise, it
					// contains the total number of prints in the edition, for all sizes available
					if ($imageSalesInfo['totaledition']) {
						// If this is original artwork, it is an edition of 1
						// but we must show different text
						if ($editionType == "original") {
							$limitedEdition = FetchSnippet("edition_original");
							$limitedEditionPopup = FetchSnippet("edition_original_popup");
						} else {
							$limitedEdition = Substitutions (FetchSnippet ("edition_limited"), array(
									"EDITIONSIZE"	=>	$imageSalesInfo['totaledition']
							));
							$limitedEditionPopup = Substitutions (FetchSnippet ("edition_limited_popup"), array(
									"EDITIONSIZE"	=>	$imageSalesInfo['totaledition']
							));
						}


					} else {
						$limitedEdition = FetchSnippet("edition_unlimited");
						$limitedEditionPopup = FetchSnippet("edition_unlimited_popup");
					}



					// Get the name of this artwork. Default is {fp:image}, meaning the word
					// 'image' is looked up in the vocabulary, but if it's an
					// original artwork, the artist might call it something else.
					$imageParams[FP_PARAM_IMAGE_ARTWORK_NAME]
					? $artworkname = $imageParams[FP_PARAM_IMAGE_ARTWORK_NAME]
					: $artworkname = '{fp:Image}';


					switch ($Ecommerce[$artistID]) {

						case 1 :
						// -------
						// SHOPPING CART Code for purchasing
						//
						// Get available sizes based on the image size, i.e. pixel width determines print size
							$wrapper = FetchSnippet ("shoppingcart_size_wrapper");
							$formattedsizes = ClickablePrintSizes($item_number, $pricesetID, $wrapper, false);

							if ($formattedsizes) {
								$fields['ShoppingCart'] = Substitutions ($defaultShoppingCartCode, array (
										"text"		=> $formattedsizes,																											"URL"			=>	$URL,																											"ID"			=>	$item_number,																											"PrintSalesID"	=>	$PrintSalesID[$artistID]																											));
							} else {
								$fields['ShoppingCart'] ="<!--NOT FOR SALE: too small or no priceset -->";
							}
							break;

						case 2 :
						// -------
						// PICTOPIA Javascript code
						// clickable sizes is javascript to order by size
						// $printSaleCode[$artistID] is a snippet, loaded at the beginning of this function
						// First, we check the artist to see if he's got his own coding

						// If the PPS field for this project is empty,
						// we do not show any Pictopia links.

						// DOES NOT WORK...TOO OLD...NEEDS UPDATING

							if (! empty($fields["$PROJECTS.pps"])) {
								if ($PrintSalesID[$artistID]) {
									$URL = $fields[$prefix."URL"];
									$sizes = ClickablePrintSizes($item_number, 0, $printSaleCode[$artistID], true);
									$fields[$IMAGES."_PrintSizes"] = $sizes;
									$fields["PrintSizes"] = $sizes;
									$fields["Pictopia"] = $sizes;
									$fields['PopupBuyButton'] = $sizes;
									//$fields['PopupBuyButtonEncoded'] = urlencode($fields['PopupBuyButton']);
									$fields['PopupBuyButtonEncoded'] = separator ($fields['PopupBuyButton']);

								} else {
									$fields[$IMAGES."_PrintSizes"] = "<!-- No Pictopia account -->";
									$fields["PrintSizes"] = "<!-- No Pictopia account -->";
									$fields["Pictopia"] = "<!-- No Pictopia account -->";
								}

							} else {
								$fields["Pictopia"] = "<!--No PPS set...Do not sell prints-->";
							}
							break;
						case 3 :
						// -------
						// PAYPAL SHOPPING CART buy button
							$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": PayPal info : $item_name <BR>";
							if ($itemAmount[$artistID] and (! empty($artistPayPalBusiness[$artistID]))) {
								$fields[$IMAGES."_PPbutton"] = CreateOnePPButton ($item_name, $item_number, $itemAmount[$artistID], $artistPayPalBusiness[$artistID]);
								$fields["PayPal"] = CreateOnePPButton ($item_name, $item_number, $itemAmount[$artistID], $artistPayPalBusiness[$artistID]);
							} else {
								$fields[$IMAGES."_PPbutton"] = "<!-- Ecommerce Method 3, missing data, cannot make PayPal button -->";
								$fields["PayPal"] = "<!-- Ecommerce Method 3, missing data, cannot make PayPal button -->";
							}
							break;

						case 4 :
						// -------
						// E-JUNKIE SHOPPING CART add to cart button
							$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": PayPal info : $item_name <BR>";
							if ($itemAmount[$artistID] and (! empty($artistPayPalBusiness[$artistID]))) {
								$fields[$IMAGES."_PPbutton"] = CreateOnePPButton ($item_name, $item_number, $itemAmount[$artistID], $artistPayPalBusiness[$artistID]);
								$fields["PayPal"] = CreateOnePPButton ($item_name, $item_number, $itemAmount[$artistID], $artistPayPalBusiness[$artistID]);
							} else {
								$fields[$IMAGES."_PPbutton"] = "<!-- missing data, cannot make button -->";
								$fields["PayPal"] = "<!-- missing data, cannot make button -->";
							}
							break;

						case 5 :
						// -------
						// Go to frameshop link
						// if image is for sale, and if it's big enough to sell
						// Artist must have chosen a supplier
							$pricesetID = FetchPriceSetID ($item_number, $projectID);
							$imageForSale = imageIsForSale ($item_number, $pricesetID);
							if ($imageSalesInfo["supplierID"] && $pricesetID && (count($sizes) > 0) && $imageForSale ) {
								$currentmatte = ($variation ? "1" : "0" );
								$fields['ShoppingCart'] = Substitutions (FetchSnippet ("frameshop_link"), array (
										"PRICERANGE"		=> $pricerange,
										"CURRENTFRAME"		=> $fields[DB_PROJECTS."_Framestyle"],
										"CURRENTSIZE"		=> count ($sizes)-1,
										"CURRENTMATTE"		=> $currentmatte,
										"LIMITED_EDITION"	=> $limitedEdition,
										"ID"			=> $item_number,
										"PROJECTID"		=> $projectID,
										"AVAILABLESIZES"	=> $availableSizes
								));


								$fields['PopupBuyButton'] = Substitutions (FetchSnippet ("frameshop_link_popup"), array (
										"PRICERANGE"		=> $pricerange,
										"CURRENTFRAME"		=> $fields[DB_PROJECTS."_Framestyle"],
										"CURRENTSIZE"		=> count ($sizes)-1,
										"LIMITED_EDITION"	=> $limitedEdition,
										"ID"				=> $item_number,
										"PROJECTID"			=> $projectID,
										"AVAILABLESIZES"		=> $availableSizes,
										"CURRENTMATTE"		=> $currentmatte
								));
								//PRINT __FUNCTION__.__LINE__.":";

								// this allows us to store this HTML code inside a field
								// and use JS to dynamically write it!
								//$fields['PopupBuyButtonEncoded'] = urlencode($fields['PopupBuyButton']);
								$fields['PopupBuyButtonEncoded'] = utf8_to_html ($fields['PopupBuyButton']);
							}
							else {
								if (!$imageSalesInfo["supplierID"])
									$e = "No supplier.";
								if (!$pricesetID)
									$e .= " No Priceset.";
								if (!(count($sizes) > 0))
									$e .= " No sizes offered.";
								if (!$imageForSale)
									$e .= " Not for sale.";

								$fields['ShoppingCart'] ="<!--NOT FOR SALE: $e -->";
								$fields['PopupBuyButtonEncoded'] = "";
								$fields['PopupBuyButton'] = "";
							}
							break;

						default :
							break;
					}
					// Fill in the built-in "Buy Print" snippet
					$fields['BuyPrint'] = Substitutions ($defaultBuyPrintButton, array (
							"PayPal"		=> $fields["PayPal"],
							"Pictopia"		=> $fields["PrintSizes"],
							"PrintSizes"	=> $fields["PrintSizes"],
							"ShoppingCart"	=> $fields["ShoppingCart"],
							"ItemName"		=> $item_name,
							"ItemNumber"	=> $item_number,
							"ItemAmount"	=> $itemAmount[$artistID],
							"BusinessEmail" => $artistPayPalBusiness[$artistID],
							"PrintSalesID"	=> $PrintSalesID[$artistID],
							"PRICERANGE"	=> $pricerange,
							"LIMITED_EDITION"	=> $limitedEdition,
							"ARTWORK_NAME"		=> $artworkname



					));

				} else {
					$fields['BuyPrint'] = "<!-- eCommerce turned off OR artist does not have a print service ID-->";
				}


			}
			$TIMER && $error .= __FUNCTION__.__LINE__.": Time elapsed; column = $column : ".round (microtime(true) - $starttime, 3) . "<BR>";

			/* Format this row */
			//$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": Fields : ".ArrayToTable($fields)."<BR>\n";
			$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": ItemFormat:<HR> ";
			//$DEBUG && var_dump ($itemformat);
			$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": <HR>";
			
			// ------------------
			// true = fixquotes, false=don't. Well, true messes up stuff. The question is, why
			// did I have it on? What was I fixing?
			$thisrow = Substitutions ($itemformat, $prefields, false);
						
			$thisrow = Substitutions ($thisrow, $fields, false);
			// ------------------


			/* sub-lists are also items, but formatted with their own formatting.
				But, like a list item, they add onto the list, too. */
			$subrow = "";
			if (! empty($tables)) {
				$DEBUG && $msg .= __FUNCTION__ .":".__LINE__ . ":";
				$DEBUG && $msg .= "***********Working on :".($artist_info['28']['Firstname']."<br>");
				$subrow = FetchCascade ($tables, $sets, $wheres, $orders, $formats, $limits, $fields, $skip_empty_rows, $variation, $params, $artist_info);
				$DEBUG && $msg .= "***********Finished on :".($artist_info['28']['Firstname']."<br>");
				//$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": FetchCascade returned $subrow<BR>";
				//$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": </blockquote>\n";
				($DEBUG AND !$subrow) && print "FetchCascade returned nothing<BR>";
			}

			// Now at bottom of list of tables
			// If no data was generated, dump the row from the top
			// IF $skip_empty_tables is TRUE
			if (empty($tables)) {
				$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": BOTTOM OF TABLE LIST!<BR>";
			}


			// IF this is an end of row add in $rowformat, as determined by $rowlength
			if ($rowlength && $itemcount > 0) {
				if (fmod ($itemcount, $rowlength) == 0) {
					$list .= Substitutions($rowformat,$fields);
				}
			}

			if (empty($tables) OR (!empty($subrow)) OR (!$skip_empty_rows)) {
				// Separator counter...add a separator if more than one item, i.e. k>1
				$itemcount++;
				$itemcount > 1 && $list .= Substitutions ($itemseparator, $fields);
				$list .= $thisrow;
				$list .= $subrow;
			} else {
				$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": NOT Added info the to the list ($list)<BR>";
			}
			$column++;
		}

		$TIMER && $error .= __FUNCTION__.__LINE__.": Time elapsed: ".round (microtime(true) - $starttime, 3) . "<BR>";
		// Put the list built above into the formatting block of text
		// If the list is empty...no bottom level elements...dump it! Probably won't happen, unless something
		// weird is going on, like a project with pix by an artist not actually listed in the project.
		if ($list) {
			$fields ['list'] = $list;
			//$output = Substitutions ($blockformat, array ('list' => $list));
			$output = Substitutions ($blockformat, $fields);
			// I think we don't need to do this here. They will be replaced later, and we can hope
			// that no sys vars will be built upon until the end
			// $output = ReplaceSysVars ($output);
			//$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ":: output = ".htmlentities($output)."<BR>";
		} else {
			$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": LIST EMPTY: output = ".htmlentities($output)."<BR>";
		}
	}


	($DEBUG AND ! $output) && print "This row ($tablekey) produced NO text for the page.<BR>";
	//$DEBUG &&	print "4 $output<BR>";

	$TIMER && $error .= __FUNCTION__.__LINE__.": Time elapsed: ".round (microtime(true) - $starttime, 3) . "<BR>";

	return $output;
}	/* end FetchCascade */




// GetDisplayMatteDimensions ($w, $h, $mattescale, $framewidth)
// Fit the matted image into the current width/height!!!
//
// Use the Matte settings in the config.sys
// to figure out the size of a picture, and the shape, if it
// has a matte.
// We want the height to stay the same, while the width can change.
// $mattescale is the percentage to reduce the picture to fit; the rest is the matte
// so .8 means reduce picture by 80%, leaving a 20% matte around.
function GetDisplayMatteDimensions ($width, $height, $mattescale = null) {
	global $msg, $error;
	global $LINK;

	$mattescale || $mattescale = MATTESCALE;
	$new_interior_width =  ceil ($width * $mattescale) ;
	$new_interior_height = ceil ($height * $mattescale);

	$matteWidth = ceil ( ($height*(1-$mattescale))/2 );
	$matteWidthBottom = floor ( ($height*MATTEBOTTOM)/2 ) + $matteWidth;

	$mattedwidth =	$new_interior_width + (2 * $matteWidth);
	$mattedheight = $new_interior_height + $matteWidth + $matteWidthBottom;

	return array ($new_interior_width, $new_interior_height, $mattedwidth, $mattedheight, $matteWidth, $matteWidthBottom);
}

// Get the size of an image, but scaled
function GetImageSizeScaled ($imagepath, $scale=1) {
	global $msg, $error;
	global $LINK;
	$scale || $scale = 1;
	$imgsize = GetLocalImageSize($imagepath);
	$imgsize[0] = $imgsize[0] * $scale;
	$imgsize[1] = $imgsize[1] * $scale;
	$imgsize[2] = "width=\"" . $imgsize[0] ."\" height=\"" . $imgsize[1] . "\"";

	$msg .= __FUNCTION__.": scale=$scale, url=$imagepath<BR>";

	return $imgsize;
}

// ==========================================
// URLtoIMG ($URL, $other = "")
// $URL is a string, $other is other HTML to go in the IMG code
function URLtoIMG ($URL, $other = "", $maxWidth = 0, $maxHeight = 0, $alt="", $scale = 1, $urlprefix = "") {
	global $Themes;
	global $BASEDIR;
	global $error, $msg;

	if ($URL && is_file ("$BASEDIR/$URL")) {
		$size = GetLocalImageSize ("$BASEDIR/$URL");

		if ($size) {
			$width = $size[0] * $scale;
			$height = $size[1]* $scale;
			$sizetext = "width=\"$width\" height=\"$height\"";

			if ($maxWidth+$maxHeight > 0) {
				list ($width, $height) = CalcResizeImage ($width,$height,$maxWidth,$maxHeight);
				$width = $width * $scale;
				$height = $height * $scale;
				$sizetext = "width=\"$width\" height=\"$height\"";
			}

			$border = "";

			$alt || $alt = "picture";

			$IMG = Substitutions (FetchSnippet ('IMG'), array ( 'URL' => $urlprefix.$URL,
					'size'	=> $sizetext,
					'alt'	=> $alt,
					'other' => $other,
					'border'	=> $border
					)
			);
			return $IMG;
		} else {
			// Failed to size picture, assume error.
			return "";
		}
	} else {
		return "";
	}
}


/*
RandomRow
Get a random row from a table. This is used to fetch 'matted' pictures
for pages, but could also fetch random stories, or whatever.
Returns a mysql $result.

$table is the table
$set contains the fields to get
$where is the WHERE
$count is the number of rows to return
*/
function RandomRow ($table, $set = '*', $where = '', $limit = 1) {
	global $error, $msg, $Themes;
	global $LINK;

	$DEBUG = 0;
	if ($where)
		$where = "WHERE $where ";
	if ($limit)
		$limit = "LIMIT $limit";
	$query = "SELECT $set FROM $table $where ORDER BY RAND() $limit";
	$DEBUG && $msg .= __FUNCTION__ .":".__LINE__ . ": RandomRow: $query <BR><BR>";
	//print "randomrow: query: $query<BR>";
	$result = mysqli_query ($LINK, $query);
	$num_rows = 0;
	$result && ($num_rows = mysqli_num_rows($result)); // rows returned
	if ($num_rows) {
		$DEBUG && $result && $msg .= ResultToTable ($result);
		return mysqli_fetch_array($result, MYSQLI_ASSOC);
	} else {
		return array ();
	}
}

// ------------------------------------
// Grab the images from a random Project. Why not a Part? Well, if we start with
// a Project, we can specify that it is Active and Featured, if we wish.
// If that fails, grab anything so you don't return nothing!
//
// $shape is the w/h ratio. 3:2 is standard 35mm. If you give a shape of 3/2 = 1.5, then
// only pictures approximately 3:2 in shape will be used.
function FetchRandomImage ($where="", $format = 'sample_picture', $shape=null, $projectID = null,$groupID=null) {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	global $MAILED_DIR, $PHOTOS_GALLERY, $SLIDES, $THUMBNAILS, $MATTED, $FRAMED, $ORIGINALS, $BASEDIR, $LOGS;

	global $BASEDIR;
	global $msg, $error;
	global $LINK;

	$DEBUG = 0;
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ ."<BR>";

	$imageinfo = array('Images_URL'=>'');

	// If no $projectID is specified, AND there exists a FP_SLIDES_PROJECT project,
	// get pictures from that project.
	// Try 5 times; if the system is so messed up it screws up 5 times, just give up
	$alarm = 1;
	$arr = array();
	while ($alarm < 5 && empty($arr['URL'])) {

		// Fetch a picture from project FP_SLIDES_PROJECT if no projectID is given
		if (!$where && !$projectID) {
			$where2 = "Slides > 0";
			$arr = RandomRow ("Projects", 'ID', $where2, 1);
			$arr && $projectID = $arr['ID'];
		}

		// get image ID from parts in projects in the group
		$w = array ();
		$groupID || $groupID = PUBLIC_GROUP_ID;
		$projectID
				? $pid = " = $projectID"
				: $pid = "IN (SELECT ID from ".DB_PROJECTS." WHERE GroupID = $groupID)";

		$query = "SELECT PartID from ".DB_PARTS." WHERE PartTable = '".DB_IMAGES."' AND ProjectID $pid ORDER BY RAND() LIMIT 1";
		$DEBUG && $msg .= __FUNCTION__ .":".__LINE__ . ": query=<BR>$query<BR>";

		$result = mysqli_query ($LINK, $query);
		$result && ($num_rows = mysqli_num_rows($result)); // rows returned
		if ($num_rows) {
			$arr = mysqli_fetch_array($result, MYSQLI_ASSOC);
		} else {
			$arr = null;
		}

		$DEBUG && $msg .= "Are there pictures in $projectID? <BR>";

		if (! empty($arr) && $arr['PartID']) {
			// Get the image info
			//$arr = FetchRowsByValue ($IMAGES, "*", 'ID', $arr['PartID'], '', TRUE);
			$arr = FetchImage($arr['PartID']);
			$DEBUG && $msg .= __FUNCTION__ .":".__LINE__ . ": Found image " . $arr['ID'] .", URL = ".$arr['URL']."<BR>";
		}

		$DEBUG && $msg .= __FUNCTION__ .":".__LINE__ . ": Try #$alarm: " . $projectID . " - ";

		// dump the image if it's the wrong shape
		// example: Desired shape is 1.5 (that is, 3:2) (35mm film)
		// If shape is 3:2.2, so ratio is 1.5
		//
		if ($shape && $arr['URL']) {
			$imgsize = GetLocalImageSize("$BASEDIR/" . FP_DIR_SLIDES_IMAGES . "/" . $arr['URL']);
			$width = $imgsize[0];
			$height = $imgsize[1];
			$imgShape = round ($width / $height , 1);
			$DEBUG && $msg .= __FUNCTION__.": ". "$BASEDIR/" . FP_DIR_SLIDES_IMAGES . "/" . $arr['URL'] . "<BR>shape=$shape, imageshape=$imgShape, tolerance=".FP_RANDOM_SHAPE_TOLERANCE."<br>";
			if ( abs($shape - $imgShape) >= FP_RANDOM_SHAPE_TOLERANCE )
				$arr = array ();
			$DEBUG && ($arr && $msg .= "Yes, shape matches $shape<BR>");
			$DEBUG && (!$arr && $msg .= "No, shape does not match $shape<BR>");
			$DEBUG && (!$arr && $msg .= "$alarm) Looking again...<BR><BR>");
		}
		$alarm++;
	}

	$DEBUG && ($msg .= __FUNCTION__.__LINE__.": Counter = $alarm<BR><BR>");


	// if none found, grab anything
	if (empty($arr)) {
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": Found none, taking anything.<BR>";
		$where = "PartTable = 'Images'";
		$arr = RandomRow ("Parts", 'PartID', $where, 1);
		if (! empty ($arr) && $arr['PartID']) {
			//$arr = FetchRowsByValue ($IMAGES, "*", 'ID', $arr['PartID'], '', TRUE);
			$arr = FetchImage($arr['PartID']);
			file_exists ("$BASEDIR/" . FP_DIR_SLIDES_IMAGES . "/" . $arr['URL'])
					? $imgsize = GetLocalImageSize("$BASEDIR/" . FP_DIR_SLIDES_IMAGES . "/" . $arr['URL'])
					: $arr = array ();
		}
	}
	
	// We should now have an image to show.
	if (! empty($arr)) {
		reset ($arr);
		while (list ($key, $value) = each ($arr)) {
			$imageinfo['Images_'.$key] = $value;
		}
		
		$projectID || $projectID = $imageinfo['Images_ProjectID'];
		//$imageinfo['Images_ProjectID'] = $projectID;
		//print "FetchRandomImage: " . $projectID . "<BR>";

		$IMG = URLtoIMG ($PHOTOS_GALLERY . "/" . $imageinfo['Images_URL']);
		preg_match ("/{/", $format) || $format = FetchSnippet ($format);
		$samplepicture = Substitutions ($format, $imageinfo);
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": $samplepicture<BR>";

		// Get caption, surround block of text, add in artist info.
		//$row = FetchRowsByValue($ARTISTS, "*", 'ID', $imageinfo['Images_ArtistID'], '', TRUE);
		$row = FetchArtist($imageinfo['Images_ArtistID']);
		$artistinfo = array();
		while (list ($key, $value) = each ($row)) {
			$artistinfo['Artists_'.$key] = $value;
		}
		//print ArrayToTable ($artistinfo);

		//Get Project info for the project with owns this image
		//$row = FetchRowsByValue("Projects", "*", 'ID', $projectID,'',TRUE);
		$row = FetchProject($projectID);
		$projectinfo = array();
		while (list ($key, $value) = each ($row)) {
			$projectinfo['Projects_'.$key] = $value;
		}
		//print ArrayToTable ($projectinfo);

		$samplepicture = Substitutions ($samplepicture, $projectinfo);
		//print htmlentities ($samplepicture)."<BR>";
		$samplepicture = Substitutions ($samplepicture, $artistinfo);
		//print htmlentities ($samplepicture)."<BR>";

		$f = array ();
		$f["Images_IMG"] = $IMG;
		$f["size"] = $imgsize[3];
		$samplepicture = Substitutions ($samplepicture, $f);
		//print htmlentities ($samplepicture)."<BR>";


		return $samplepicture;
	} else {
		// NO pictures anywhere in the system
		return "";
	}
}


/*
 * FetchRandomImageList
 * Fetches a list of URLs for images for background pictures on the website, to be used with Javascript.
 * Allows us to prefetch the list once, then let javascript pic the image
 * so we can cache the whole page.
 *
 * Fetch the images from the specified project. If no project spec'd, get them from the "Carousel".
 * Failing that, anything.
*/
function FetchRandomImageList ($groupID=null, $projectID = null) {

	/*
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	global $MAILED_DIR, $PHOTOS_GALLERY, $SLIDES, $THUMBNAILS, $MATTED, $FRAMED, $ORIGINALS, $BASEDIR, $LOGS;
	*/

	global $BASEDIR;
	global $msg, $error;
	global $LINK;

	$groupID || $groupID = PUBLIC_GROUP_ID;

	// Fetch a picture from project FP_SLIDES_PROJECT if no projectID is given
	if (!$projectID) {
		$where2 = "Slides > 0";
		$arr = RandomRow ("Projects", 'ID', $where2, 1);
		$arr && $projectID = $arr['ID'];
	}

	// Look for a "slideshow" project in this group
	$myGroup = new FPGroup ($LINK, $groupID);
	$projects = $myGroup->FetchSlideshow();
	if ($projects) {
		$projects = array($projects);
	} else {
		$projects = $myGroup->ProjectList();
	}

	$list = array();
	foreach ($projects as $project) {
		$pid = $project['ID'];
		$list = array_merge($list, FetchProjectImageURLs ($pid, null));
	}

	foreach ($list as &$v) {
		$v = FP_DIR_SLIDES_IMAGES . "/" . urlencode($v);
	}

	// make comma-separated list
	if ($list) {
		$listtext = '"' . join('","', $list) . '"';
	}
	return $listtext;

}


// --------------------
// param1 = table
// param2 = mysql WHERE clause, without the 'WHERE'
function DeleteRow( $table, $where ) {
	global $Themes;
	global $LINK;
	
	$DEBUG = false;
	if ($DEBUG) {
		$error .= "DeleteRow: delete from $table where $where<BR>";
		$error .= "select * from $table where $where<BR>";
		$error .= ResultToTable (mysqli_query ($LINK, "select * from $table where $where"));
	}
	mysqli_query ($LINK, "delete from $table where $where");
	return mysqli_error ($LINK);
}

function DeleteRowByID( $table, $ID ) {
	global $Themes;
	global $LINK;
	
	$DEBUG = FALSE;
	if ($DEBUG) {
		$error .= "DeleteRowByID: delete from $table where ID = '$ID'<BR>";
		$error .= "select * from $table where ID = '$ID'";
		$error .= ResultToTable (mysqli_query ($LINK, "select * from $table where ID = '$ID'"));
	}
	return mysqli_query ($LINK, "delete from $table where ID = '$ID'");
}

//--------------------------
// Insert a record, return the ID of the added record
function AddRecord( $table, $pairs ) {
	global $Themes;
	global $error, $msg;
	global $LINK;

	$DEBUG = 0;
	$DEBUG && $msg .=  "<h3>AddRecord to $table</h3>";
	$DEBUG && $msg .=  "AddRecord: I got these vars for table=$table:<BR>".ArrayToTable($pairs);

	$pairs = StripNonExistantFields ($table, $pairs);
	$pairs = SerializeArraysInPairs ($pairs);

	$DEBUG && $msg .=  "AddRecord: I got these vars after StripNonExistantFields:<BR>".ArrayToTable($pairs);

	//	$cols = makeCols($pairs);
	//	$vals = makeVals($pairs);

	$DEBUG && $msg .=  "AddRecord: After using makeVals:<BR>".ArrayToTable($pairs);
	// make sure vals don't contain chars which will screw up the Mysql query
	// in particular, a ' will destroy it

	// $query = "insert into $table ($cols) values ($vals)";
	//	if ($DEBUG)
	//		$error .= "AddRecord: QUERY =  <BR><TT>$query</TT><BR>";
	//	$result =  mysqli_query ($LINK, $query);
	//	$ID = mysqli_insert_id();

	$pairs = FixCheckboxFields ($pairs);
	$pairs = StripNonExistantFields ($table, $pairs);
	$pairs = PackageParams ($pairs);
	$pairs = SerializeArraysInPairs ($pairs);
	$update = makeUpdate($pairs);

	$DEBUG && $msg .=  "AddRecord: After using makeUpdate:<BR>".ArrayToTable(explode(",", $update));

	mysqli_query ($LINK, "insert into $table SET $update");

	$ID = mysqli_insert_id($LINK);
	$e = mysqli_error ($LINK);
	$e && $error .= __FUNCTION__.": $e<br>";

	if ($DEBUG)
		$error .= __FUNCTION__.": Insert into $table SET $update FAILED<BR>";

	// This doesn't work, I don't know why
	if ($table == DB_ARTISTS && $ID) {
		$q = "UPDATE `$table` SET `CreationDate` = NOW() where ID = '$ID'";
		mysqli_query ($LINK, $q);
		$e = mysqli_error ($LINK);
		$e && $error .= __FUNCTION__.": $e<br>";
	}

	return $ID;
}

//---------------
function GetDependencies ($DEPENDENCIES_FILE) {
	global $PICTURE_DIRECTORIES, $DATABASES;	// Arrays of names
	global $BASEDIR;
	global $msg, $error;
	global $LINK;

	$DEBUG = false;

	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": BEGIN<BR>";
	$DEPENDENCIES = array();
	$arr = file ($DEPENDENCIES_FILE);
	while (list ($line_num, $raw) = each ($arr)) {
		trim($raw);
		if (! preg_match("#(^//)|(^\W)#", $raw)) {
			$raw = Substitutions ($raw, $DATABASES);
			$DEBUG && $msg .= "--> $raw<BR>";
			$row = explode ("\t", $raw);
			$command = array_shift($row);
			$table = array_shift($row);
			$DEPENDENCIES[$command][$table][] = $row;
		}
	}
	if ($DEBUG) {
		while (list($c, $command) = each ($DEPENDENCIES)) {
			$error .= "<h1>Command $c</H1>";
			while (list($t, $table) = each ($command)) {
				$error .= "<h2>Table $t</h2>";
				foreach ($table as $row) {
					$error .= "row:";
					$error .= ArrayToTable ($row);
				}
			}
		}
	}
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": END GetDependencies<BR>";
	return $DEPENDENCIES;
}

// ------------------------------
// CascadeDelete ($table, $ID)
// Given a row to delete, delete the rows in other tables which are related
// The mysql commands are in the text file $DEPENDENCIES
// If there is no entry in the DEPENDENCIES, delete normally.
function CascadeDelete ($table, $ID) {
	global $Themes;
	global $DEPENDENCIES, $BASEDIR, $LOGS;
	global $error, $msg;

	$DEBUG = 0;
	$VERBOSE = false;

	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": Dependencies: ". ArrayToTable($DEPENDENCIES['delete']) . "<BR>";
	$action = 'delete';
	$row = FetchRowsByValue ($table, "*", "ID", $ID, '', TRUE);
	if ($row and isset($DEPENDENCIES[$action][$table])) {
		foreach ($DEPENDENCIES[$action][$table] as $arr) {
			$table2 = array_shift($arr);
			$where = array_shift($arr);
			$row['ID'] = $ID;
			$row['table'] = $table;
			$where = Substitutions ($where, $row);
			// $where = preg_replace('/\$ID/i', $ID, $where);
			// $where = preg_replace('/\$table/i', $table, $where);
			$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": CascadeDelete: where=[$where]<BR>";
			$failure = DeleteRow ($table2, $where);
			$failure || ($VERBOSE && $msg .= "Deleted entry from $table $where<BR>");
			$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": CascadeDelete : $table2 $where<BR>";
			// Note, it's common to fail with keywords and comments tables
			$failure && fp_error_log(__FUNCTION__.": CascadeDelete: Could not delete $table2 WHERE $where : $failure", 3, FP_ERROR_LOG );
			if ($DEBUG)
				$msg .= "CascadeDelete: delete row $table2 WHERE $where<BR>";
		}
	} else {
		$result = DeleteRowByID ($table, $ID);
		$result || $error .= __FUNCTION__.__LINE__.": Failed to delete $ID in $table<BR>";
	}
}

// Return an array whose KEYS match a search string
function PregMatchKeys ($searchpattern, $arr) {
	global $Themes;
	$arr2 = array ();
	while (list ($k, $v) = each ($arr)) {
		if (preg_match ($searchpattern, $k)) {
			$arr2[$k] = $v;
		}
	}
	return $arr2;
}


// --------------------
// Update multiple rows
// The rows come from an HTML form
// We're getting variables of the form XXX_ID, where XXX is some
// flag to indicate what to do, and ID is an ID of the item, e.g. and image.
// $vars comes from the HTML form (array of variables)
// $table tells us which table to act upon (name of table)
function MultiEdit ($table, $vars) {
	global $Themes, $LINK;
	$DEBUG = false;
	$page = ""; //default is don't return a page. Do it for FTP.
	if ($DEBUG) {
		print "Multiedit $table<BR>";
		print ArrayToTable ($vars);
	}
	$IDlist = array ();

	//MultiSetFlag ($table, "Featured", $vars);
	//MultiSetFlag ($table, "Active", $vars);

	//Edit marked rows, with multieditfield = multieditvalue
	$flag = "multiedit";
	$IDlist = FetchListFromMultiForm ($flag, $vars);
	$idset = ArrayToString ($IDlist);
	$field = $vars['multieditfieldname'];
	$value = $vars['multieditdata'];
	$value = quote_smart($LINK, $value);
	if ($idset) {
		$query = "UPDATE $table SET $field=\"$value\" WHERE ID IN ( $idset )";
		$DEBUG && $msg .= $query . "<BR>";
		$result = mysqli_query ($LINK, $query);
		if (! $result)
			print mysqli_error($LINK) . "Failed to make $field=$value in $idset<BR>";
	}

	// FTP the marked rows
	$flag = "FTP";
	$IDlist = FetchListFromMultiForm ($flag, $vars);
	if (count($IDlist))
		$page = BuildFTPForm ($IDlist, 'ftp.somewhere.com','picturefolder','username','password');

	//Delete marked rows
	$flag = "delete";
	$IDlist = FetchListFromMultiForm ($flag, $vars);

	foreach ($IDlist as $ID) {
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": Deleting ID=$ID from $table<BR>";
		EditTable ('delete', $table, $ID, '');
	}

	return $page;
}

// Convert an array into a quoted, comma separated string for
// use with SQL queries.
// (a,b,c) becomes "a","b","c"
function ArrayToString ($a) {
	global $Themes;
	array_walk ($a, 'Enquote');
	$s = implode ("," , $a);
	return $s;
}

function Enquote (&$t) {
	global $Themes;
	$t = '"'.$t.'"';
}

// FetchListFromMultiForm ($flag)
// Return a list of ID's from a form which has a list of items
// $flag is the name of the flag for the fields in the form
// The fields are of the form, $flag_$ID
// So, $flag="delete" would get all fields which are "delete_1" and "delete_2" and
// so on. These are usually checkboxes.
function FetchListFromMultiForm ($flag, $vars) {
	global $Themes;
	$IDlist = array();
	$rows = PregMatchKeys("/^".$flag."_/i", $vars);
	foreach (array_keys ($rows) as $ID) {
		// form is "active_X"
		$IDlist[] = substr($ID, strlen($flag)+1);
	}
	return $IDlist;
}



// MultiSetFlag
// Change some field in multiple rows
// $flag is the name of the field to set in the database, e.g. "Featured"
// $vars contains the variables from the HTML form

function MultiSetFlag ($table, $flag, $vars) {
	global $Themes;
	global $LINK;
	
	$DEBUG = false;
	//	$idset = implode (", " , $IDlist);
	$allrows = PregMatchKeys("/^record_/i", $vars);
	$all = implode (", " , $allrows);

	$flaggedrows = PregMatchKeys("/^" . $flag . "_/i", $vars);
	$flaggedrows = array_keys ($flaggedrows);
	$idset = implode (", " , $flaggedrows);
	$idset = preg_replace("/" . $flag . "_/i", "", $idset);
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": <HR>$flag<BR>$idset<HR>";
	// Set all to no in this set of records
	$query = "UPDATE $table SET $flag=\"no\" WHERE ID IN ( $all )";
	$DEBUG && $msg .= $query . "<BR>";
	$result = mysqli_query ($LINK, $query);
	if (! $result)
		print mysqli_error($LINK) . "Failed to make $flag=no $all<BR>";

	// Now set marked ones to yes
	if (empty($idset))
		$idset = '""';
	$query = "UPDATE $table SET $flag=\"yes\" WHERE ID IN ( $idset )";
	$DEBUG && $msg .= $query . "<BR>";
	$result = mysqli_query ($LINK, $query);
	if (! $result)
		print mysqli_error($LINK) . "Failed to make $flag=yes $idset<BR>";


}


// ----------------
// For making checkmarks on forms...
function CheckedIfTrue ($value) {
	global $Themes;
	if ($value) {
		return "CHECKED";
	} else {
		return "";
	}
}

// ----------------
// Given true/false, show appropriate text/picture
function ShowIfTrue ($value) {
	global $Themes;
	if ($value) {
		return FetchSnippet('ShowTrue');
	} else {
		return FetchSnippet('ShowFalse');
	}
}



// Uses an array, $grouplist[$groupID] = $title
// to build a popup. This is the kind of array 
// you get from FetchActiveGroupsID.
function GroupsPopupFromList ($grouplist) {
	global $Themes;
	global $msg, $error;
	global $LINK;

	is_array ($grouplist) || $grouplist = array ($grouplist);
	if (!count ($grouplist))
		return "";

	$extrahtml = "";
	$extraline	= array ('label'=>"Go to group...",'value'=>"", 'checked'=>'CHECKED');
	$values = array ();
	foreach ($grouplist as $id => $title) {
		$v = "projects.php?GroupID=" . $id;
		$values[$v] = $title;
	}
	$extrahtml = "OnChange=\"javascript:changeLocation(this)\"";
	$groupPopup = OptionListFromArray ($values, "GroupList", array(), TRUE, FALSE, $extrahtml, $extraline);
	$groupPopup || $groupPopup = "No Groups";	// this won't happen
	return $groupPopup;
}



// Display groups on a page.
// This is based on the slide viewer for the image management form
// We count from 1, not 0, as the first item, for the $start parameter.
// IF a group has no projects, don't show it by default
function DisplayGroupsList ($start = 1, $showEmptyGroups = FALSE) {
	global $Themes;
	global $IMAGESDIR;
	global $PHOTOS_GALLERY, $SLIDES, $THUMBNAILS, $MATTED, $ORIGINALS, $SITEIMAGES;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	global $msg, $error;
	global $LINK;
	global $groups_display_cols, $groups_items_per_page, $groups_desc_len;

	$DEBUG = false;
	isset($_SESSION['fp_user']) ? $fp_user = $_SESSION['fp_user'] : $fp_user = 1;

	isset($_SESSION['theme']) || $_SESSION['theme'] = DEFAULT_THEME;

	if (isset ($myTheme)) {
		$myTheme = FP_THEMES_DIR . "/$myTheme/";
	} elseif (isset ($THEME)) {
		$myTheme = FP_THEMES_DIR . "/$THEME/";
	} else {
		$myTheme = FP_THEMES_DIR . "/" . $_SESSION['theme'] . "/";
	}
	$myTheme = CurrentThemeID ();
	$myThemeDir = CurrentThemeDir ();


	$blankgalleryicon = "$myTheme$IMAGESDIR/blankgalleryicon.png";

	// Get only groups with active projects
	$grouplist = FetchActiveGroupsID ();

	$groupPopup = "<!-- not enough groups for popup -->";
	if (count ($grouplist) > $groups_items_per_page) {
		$groupPopup = GroupsPopupFromList ($grouplist);
		$groupnav = FetchSnippet ("group_table_nav");
	} else {
		$groupnav = "<!-- no need for navigation -->";
	}

	if (count($grouplist)) {
		// Show which items on the page?
		// show how many items per page?
		$start ? $from = $start - 1 : $from = 0;
		$to = min (count ($grouplist), $from + $groups_items_per_page);

		// prev/next code snippets
		// set current info
		$current = FetchSnippet ("group_table_current");
		$current = str_replace ("{from}", $from+1, $current);
		$current = str_replace ("{to}", $to, $current);
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__. "current:".$current."<BR>";

		// set prev link
		$prev = "";
		$from > 0 && $prev = FetchSnippet ("group_table_prev");
		$prev = str_replace("{start}", max($from - $groups_items_per_page-1, 0), $prev);
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__. "prev:".$prev."<BR>";

		// set next link
		$next = "";
		$to < count ($grouplist) && $next = FetchSnippet ("group_table_next");
		$next = str_replace("{start}", min($to+1, count ($grouplist)), $next);
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__. "next:".$next."<BR>";

		// Fetch the according snippet for the item template
		$group_table = FetchSnippet ("group_table_start");
		$k = $from;
		$c = 1;
		$numcols = $groups_display_cols;
		$colwidth = ceil (100 / $numcols);
		$grouplist = TitleSort ($grouplist);	// my special sort..ignores "the" in a title, keeps keys
		//asort ($grouplist);	// sort on Title, preserve the keys!!!!

		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__. ArrayToTable ($grouplist);
		$grouplist = array_slice ($grouplist, $from, $to - $from, TRUE);	// PRESERVE KEYS (PHP 5+)
		$DEBUG && $msg .= "from $from to $to, $groups_items_per_page items per page<BR>";
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__. ArrayToTable ($grouplist);

		$collen = ceil(count($grouplist) / $numcols);

		if (!empty($grouplist)) {
			$group_table_item = FetchSnippet ("group_table_item");
			$group_table_end = FetchSnippet ("group_table_end");
			$group_table_item_start = FetchSnippet ("group_table_item_start");
			$group_table_item_end = FetchSnippet ("group_table_item_end");
			$group_table_blank_item = FetchSnippet ("group_table_blank_item");
			$group_table_row_end = FetchSnippet ("group_table_row_end");
			foreach (array_keys ($grouplist) as $groupID) {
				$myGroup = new FPGroup ($LINK,  $groupID );
				//$DEBUG && $msg .= __FUNCTION__ .":".__LINE__ ." Group $groupID ****" . ArrayToTable ($myGroup->info) . "<BR>";
				$oneGroupItem = $group_table_item_start;
				$oneGroupItem .= $group_table_item;

				if (FP_SHOW_GALLERY_ICON) {
					$iconURL = $myGroup->IconFilename();
					if (!FP_SHOW_BLANK_GALLERY_ICON) {
						$showHideIcon = CSSShowHide ( $iconURL	);	// show if not blank
					}
					$icon = URLtoIMG ($iconURL,"style='border:0px;'",FP_GALLERY_ICON_WIDTH, FP_GALLERY_ICON_HEIGHT);
				} else {
					$icon = "<!--icons hidden (FP_SHOW_GALLERY_ICON)-->";
					$showHideIcon = CSSShowHide (false);
					// don't show gallery icons at all
				}

				$desc = substr ($myGroup->description, 0, $groups_desc_len);
				strlen ($myGroup->description) > $groups_desc_len && $desc .= "....";

				$oneGroupItem = str_replace("{icon}", $icon, $oneGroupItem);
				$oneGroupItem = str_replace("{SHOWHIDEICON}", $showHideIcon, $oneGroupItem);
				$oneGroupItem = str_replace("{ID}", $myGroup->ID, $oneGroupItem);
				$oneGroupItem = str_replace("{title}", $myGroup->title, $oneGroupItem);
				$oneGroupItem = str_replace("{description}", $desc, $oneGroupItem);
				$oneGroupItem = str_replace("{k}", $k+1, $oneGroupItem);
				$oneGroupItem = str_replace("{colwidth}", $colwidth, $oneGroupItem);

				$oneGroupItem .= $group_table_item_end;

				$group_table .=	"\t\t\t" . $oneGroupItem;

				$k++;	// comes here because we definitely add an item in line above...don't move
				$c++;
				if ($c > $numcols and $k < $to) {
					$group_table .= $group_table_row_end ;
					$c = 1;
				}
			}
		}
		$collen || $collen = 1;
		(($k % $collen) != 0) && ($group_table .= str_repeat($group_table_blank_item, ($k % $collen) ) . $group_table_row_end);
		$group_table .= $group_table_end;
	} else {
		$group_table = "";
	}

	// SUBSTITUTIONS
	$form = FetchSnippet ("group_table_wrapper");
	$form = Substitutions ($form, array (	"grouplist"			=>	$group_table,
			"groupnav"			=>	$groupnav,
			"current"			=>	$current,
			"next"				=>	$next,
			"prev"				=>	$prev,
			"start"				=>	$from+1,
			"GroupPopup"		=>	$groupPopup
	));
	return $form;
}


//--------------------------------------------------
function AddNewRecordLink ($table) {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;

	switch ($table) {
		case $IMAGES	:
			$addnewrecord = FetchSnippet ("addnewrecordimageslink");
			break;
		case $SALES :
			$addnewrecord = "";
			break;
		default :
			$addnewrecord = FetchSnippet ("addnewrecordlink");
			break;
	}

	return $addnewrecord;
}


//--------------------------------------------------
// From a list of Artists, return an array ID=>name
function FetchArtistNames ($IDlist) {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	is_array($IDlist) || $IDlist = array ($IDlist);
	$artistlist = implode (", " , $IDlist);
	$query = "SELECT DISTINCT ID, CONCAT_WS(', ', Lastname, Firstname) AS Fullname FROM $ARTISTS WHERE ID IN ( $artistlist )";
	$result = mysqli_query ($LINK, $query);
	while (list ($ID, $name) = mysqli_fetch_array($result)) {
		$artistlist[$ID] = $name;
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": Other artists are " . $row['Fullname'] . "<BR>";
	}
	return $artistlist;
}



// ---------------------------------------
// If a project exists, return 1, else return 0
function ProjectExists ($ID) {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	$fields = "ID";
	$result = FetchRowsByValue ($PROJECTS, $fields, 'ID', $ID, '', TRUE);
	return count($result);
}


// ---------------------------------------
// Fetch a project as an array
// 10.10.03: we calculate age to find out if still featured, active
function FetchProject ($ID) {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	$fields = ProjectsCalcFields ("*");
	$result = FetchRowsByValue ($PROJECTS, $fields, 'ID', $ID, 'Title', TRUE);
	return $result;
}

// ---------------------------------------
// Fetch a project as an array
function FetchFeaturedProjects ($groupID) {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	global $LINK;

	$groupID || $groupID = $_SESSION['GroupID'];
	$where = "($PROJECTS.Public = 0) AND NOT ($PROJECTS.Slides <=> 1) AND $PROJECTS.ArtistID != " . FP_ADMINISTRATOR . " AND $PROJECTS.GroupID = $groupID AND $PROJECTS.ID = $PARTS.ProjectID AND $PARTS.PartTable = 'Images'";
	$where = GetFeaturedWhere ($where);
	$fields = ProjectsCalcFields ("$PROJECTS.*");
	$order = "ORDER BY $PROJECTS.Title";
	$query = "SELECT DISTINCT $fields from $PROJECTS,$PARTS WHERE $where $order";
	$result = mysqli_query ($LINK, $query);
	return $result;
}

// ---------------------------------------
// Fetch a project as an array
function FetchFeaturedActiveProjects ($groupID) {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	global $LINK;

	$groupID || $groupID = $_SESSION['GroupID'];
	$where = "($PROJECTS.Public = 0) AND NOT ($PROJECTS.Slides <=> 1) AND $PROJECTS.ArtistID != " . FP_ADMINISTRATOR . " AND $PROJECTS.GroupID = $groupID AND $PROJECTS.ID = $PARTS.ProjectID AND $PARTS.PartTable = 'Images'";
	$where = GetActiveWhere ($where);
	$fields = ProjectsCalcFields ("$PROJECTS.*");
	$order = "ORDER BY $PROJECTS.Title";
	$query = "SELECT DISTINCT $fields from $PROJECTS,$PARTS WHERE $where $order";
	$result = mysqli_query ($LINK, $query);
	return $result;
}

// Fetch a group as an array
function FetchGroup ($ID) {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	$result = FetchRowsByValue ($GROUPS, "*", 'ID', $ID, 'Title', TRUE);
	return $result;
}

// Fetch all groups
// To get all, leave $where empty.
function FetchAllGroups ($where = null, $order='SortTitle', $showempty = false) {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	global $msg, $error;
	global $LINK;
	$fields = "*";

	$where && $where = "WHERE $where";
	$order && $order = "ORDER by $order";
	$query = "SELECT $fields, TRIM(LEADING 'The ' from Title) as SortTitle FROM $GROUPS $where $order";
	$result = mysqli_query ($LINK, $query);
	return $result;
}

// Fetch list of IDs for all active groups, meaning w/projects which have images
// If $ShowEmpty is false, don't select groups without active projects
// Returns array ID=>Title
// Do not select groups which are "hidden"
function FetchActiveGroupsID () {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	global $msg, $error;
	global $LINK;

	$DEBUG = false;

	// subquery: all projects with images. These are the active projects
	$subquery = "SELECT DISTINCT $PARTS.projectID from $PROJECTS, $PARTS WHERE $PROJECTS.ID = $PARTS.ProjectID AND $PARTS.PartTable = 'Images' AND $PROJECTS.Public < 1 AND $PROJECTS.Slides < 1";

	$fields = "$GROUPS.ID, $GROUPS.Title, TRIM(LEADING 'The ' from $GROUPS.Title) as SortTitle";
	$where = "WHERE $PROJECTS.GroupID = $GROUPS.ID AND $PROJECTS.ID IN ($subquery)";
	$order = "ORDER by SortTitle";
	$query = "SELECT DISTINCT $fields FROM $GROUPS, $PROJECTS $where $order";

	$result = mysqli_query ($LINK, $query);
	$DEBUG && print basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": query = $query<BR>";
	while ($group = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
		$groups[$group['ID']] = $group['Title'];
	}
	return $groups;
}


function FetchAllGroupsArrays ($where = null, $order='SortTitle') {
	global $Themes;
	$result = FetchAllGroups ($where, $order);
	while ($group = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
		$groups[$group['ID']] = $group;
	}
	return $groups;
}

function FetchAllGroupsID ($where = null, $order='SortTitle') {
	global $Themes;
	$result = FetchAllGroups ($where, $order);
	while ($group = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
		$groups[$group['ID']] = $group['Title'];
	}
	return $groups;
}


// ---------------------------------------
// Fetch ALL projects as a standard MYSQL value
// 10.10.03: we calculate age to find out if still featured, active
function FetchAllProjects ($where=null, $order='SortTitle') {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	global $msg, $error;
	global $LINK;
	global $LINK;
	$DEBUG = false;

	$fields = ProjectsCalcFields("*, TRIM(LEADING 'The ' from Title) as SortTitle");
	$where && $where = "WHERE $where";
	$order && $order = "ORDER by $order";
	$query = "SELECT $fields FROM $PROJECTS $where $order";
	$result = mysqli_query ($LINK, $query);
	if ($DEBUG)
		$msg .= "FetchAllProjects: $query<P>";
	return $result;
}

// return mysql result
function FetchAllArtists ($where, $order = 'Lastname') {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	global $LINK;
	$fullname = "CONCAT_WS(' ', Firstname, Lastname) AS Fullname";
	$fields = "*, $fullname";
	$where && $where = "WHERE $where";
	$order && $order = "ORDER by $order";
	$query = "SELECT $fields FROM $ARTISTS $where $order";
	$result = mysqli_query ($LINK, $query);
	return $result;
}

// Fetch all artists, return an array of artist arrays
function FetchAllArtistArrays ($where = "", $order='Lastname') {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	$artists = array();
	$result = FetchAllArtists ($where, $order) ;
	while ($artist = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
		$artists[$artist['ID']] = $artist;
	}
	return $artists;
}

// Fetch all projects, return an array of project arrays
function FetchAllProjectArrays ($where = "", $order='Title') {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	$projects = array();
	$result = FetchAllProjects ($where, $order) ;
	while ($project = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
		$projects[$project['ID']] = $project;
	}
	return $projects;
}

// ---------------------------------------
// Fetch an artist as an array by ID
function FetchArtist ($ID) {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	//$fullname = "CONCAT_WS(', ', Lastname, Firstname) AS Fullname";
	$fullname = "CONCAT_WS(' ', Firstname, Lastname) AS Fullname";
	$fulllongname = "CONCAT_WS(' ', Firstname, Middlename, Lastname) AS FullLongName";
	$creationdate = "UNIX_TIMESTAMP(Timestamp) AS CreationDate";
	$record = FetchRowsByValue ($ARTISTS, "*, $fullname, $fulllongname, $creationdate", 'ID', $ID, 'Lastname', TRUE);
	return $record;
}

// ---------------------------------------
// Fetch an artist as an array by ID
function FetchArtistByUsername ($Username) {
	$fullname = "CONCAT_WS(' ', Firstname, Lastname) AS Fullname";
	$fulllongname = "CONCAT_WS(' ', Firstname, Middlename, Lastname) AS FullLongName";
	$creationdate = "UNIX_TIMESTAMP(Timestamp) AS CreationDate";
	$record = FetchRowsByValue (DB_ARTISTS, "*, $fullname, $fulllongname, $creationdate", 'Username', $Username, 'Lastname', TRUE);
	return $record;
}

// ---------------------------------------
// Fetch an artist as an array by LightStalkers shortname
function FetchArtistByShortname ($shortname) {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	return FetchRowsByValue ($ARTISTS, '*', 'ShortName', $shortname, 'Lastname', TRUE);
}

// ---------------------------------------
// Fetch an image as an array
// Merge the params field values into the array as though they were real record values
function FetchImage ($ID) {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	$image = FetchRowsByValue ($IMAGES, '*', 'ID', $ID, '', TRUE);
	return $image;
}

// ---------------------------------------
// Fetch an order as an array
function FetchOrder ($ID) {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	return FetchRowsByValue (DB_SALES, '*', 'ID', $ID, '', TRUE);
}

// ---------------------------------------
// Fetch an paypal record as an array
function FetchPayPal($ID) {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	return FetchRowsByValue (DB_PAYPAL, '*', 'ID', $ID, '', TRUE);
}


// ---------------------------------------
// Fetch an paypal record as an array
function FetchPayPalByTxnID ($txnid) {
	return FetchRowsByValue (DB_PAYPAL, '*', 'txn_id', $txnid, '', TRUE);
}


// ---------------------------------------
// Fetch sales by txn_id as an array of arrays, indexed by sale record ID
function FetchSalesByTxnID ($txnid) {
	$result = FetchRowsByValue (DB_SALES, '*', 'txn_id', $txnid, '', FALSE);
	$sales = array();
	while ($sale = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
		$sales[$sale['id']] = $sale;
	}
	return $sales;
}

// ---------------------------------------
// Fetch sales by txn_id as an array of arrays, indexed by sale record ID
function FetchSalesByInvoice ($invoice) {
	$result = FetchRowsByValue (DB_SALES, '*', 'invoice', $invoice, '', FALSE);
	$sales = array();
	while ($sale = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
		$sales[$sale['id']] = $sale;
	}
	return $sales;
}

// ---------------------------------------
// Fetch sales by ID as an array of arrays, indexed by sale record ID
function FetchSale ($ID) {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	return FetchRowsByValue (DB_SALES, "*", 'ID', $ID, '', TRUE);
}


// ---------------------------------------
// Fetch a supplier record.
// We need to clean up the entry a little:
// - frame, paper, etc., info might have empty lines indicating no frame style available
// Must be sure no commas (,) in the supplier info lists, since JS uses commas to split them.
function FetchSupplier ($ID) {
	$supplier = GetRecord( DB_SUPPLIERS, $ID);

	// Could use preg_split ("/n+") to make arrays...but how does it match with frame preview?
	// Index by $i+1 because we will add "no frame" as choice 0.
	$fnames = explode("\n", $supplier['Frames']);
	$fcodes = explode("\n", $supplier['FrameCodes']);
	$frameNames = array(0=>"No Frame");
	$frameCodes = array(0=>"");
	for ($i=0;$i<count($fnames);$i++) {
		$fn = trim($fnames[$i]);
		if ($fn) {
			$frameNames[$i+1] = $fn;
			$frameCodes[$i+1] = trim($fcodes[$i]);
		}
	}
	
	// Strip commas from lines
	$supplier['FrameNamesList'] = str_replace(",", " ", $supplier['FrameNamesList']);$frameNames;
	$supplier['FrameCodesList'] = str_replace(",", " ", $supplier['FrameCodesList']);$frameCodes;
	$supplier['Frames'] = str_replace(",", " ", $supplier['Frames']);str_replace("\r", "", $supplier['Frames']);
	$supplier['FrameCodes'] = str_replace(",", " ", $supplier['FrameCodes']);str_replace("\r", "", $supplier['FrameCodes']);
	$supplier['Mattes'] = str_replace(",", " ", $supplier['Mattes']);preg_replace("/[\r|\n]+/", "\n", $supplier['Mattes']);
	$supplier['MatteColors'] = str_replace(",", " ", $supplier['MatteColors']);preg_replace("/[\r|\n]+/", "\n", $supplier['MatteColors']);
	$supplier['MatteCodes'] = str_replace(",", " ", $supplier['MatteCodes']);preg_replace("/[\r|\n]+/", "\n", $supplier['MatteCodes']);
	$supplier['Papers'] = str_replace(",", " ", $supplier['Papers']);preg_replace("/[\r|\n]+/", "\n", $supplier['Papers']);
	$supplier['PaperCodes'] = str_replace(",", " ", $supplier['PaperCodes']);preg_replace("/[\r|\n]+/", "\n", $supplier['PaperCodes']);
	$supplier['Inksets'] = str_replace(",", " ", $supplier['Inksets']);preg_replace("/[\r|\n]+/", "\n", $supplier['Inksets']);
	$supplier['InksetCodes'] = str_replace(",", " ", $supplier['InksetCodes']);preg_replace("/[\r|\n]+/", "\n", $supplier['InksetCodes']);
	$supplier['Glazing'] = str_replace(",", " ", $supplier['Glazing']);preg_replace("/[\r|\n]+/", "\n", $supplier['Glazing']);
	$supplier['GlazingCodes'] = str_replace(",", " ", $supplier['GlazingCodes']);preg_replace("/[\r|\n]+/", "\n", $supplier['GlazingCodes']);

	$supplier['FrameNamesList'] = $frameNames;
	$supplier['FrameCodesList'] = $frameCodes;
	// Strip carriage-return (\r) chars
	// keep blank lines
	$supplier['Frames'] = str_replace("\r", "", $supplier['Frames']);
	$supplier['FrameCodes'] = str_replace("\r", "", $supplier['FrameCodes']);
	// Strip blank lines
	$supplier['Mattes'] = preg_replace("/[\r|\n]+/", "\n", $supplier['Mattes']);
	$supplier['MatteColors'] = preg_replace("/[\r|\n]+/", "\n", $supplier['MatteColors']);
	$supplier['MatteCodes'] = preg_replace("/[\r|\n]+/", "\n", $supplier['MatteCodes']);
	$supplier['Papers'] = preg_replace("/[\r|\n]+/", "\n", $supplier['Papers']);
	$supplier['PaperCodes'] = preg_replace("/[\r|\n]+/", "\n", $supplier['PaperCodes']);
	$supplier['Inksets'] = preg_replace("/[\r|\n]+/", "\n", $supplier['Inksets']);
	$supplier['InksetCodes'] = preg_replace("/[\r|\n]+/", "\n", $supplier['InksetCodes']);
	$supplier['Glazing'] = preg_replace("/[\r|\n]+/", "\n", $supplier['Glazing']);
	$supplier['GlazingCodes'] = preg_replace("/[\r|\n]+/", "\n", $supplier['GlazingCodes']);

	$supplier['Handling'] = array(
		'PrintHandling'		=> $supplier['PrintHandling'],
		'PrintHandlingIntl'	=> $supplier['PrintHandlingIntl'],
		'MatteHandling'		=> $supplier['MatteHandling'],
		'MatteHandlingIntl'	=> $supplier['MatteHandling'],
		'FrameHandling'		=> $supplier['FrameHandling'],
		'FrameHandlingIntl'	=> $supplier['FrameHandlingIntl']
		);

	return $supplier;

}


// ---------------------------------------
// Fetch a story as an array
function FetchStory ($ID) {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	//	$lead = "concat(substring(Story,1,60),'...') AS Lead";
	$lead = "substring(Story,1,225) AS Lead";
	return FetchRowsByValue ($STORIES, "*, $lead", 'ID', $ID, '', TRUE);
}

function FetchPriceSet ($ID) {
	return FetchPriceSetByID ($ID);
}

function FetchPriceSetNames () {
	global $LINK;
	
	$list = array();
	$query = "SELECT ID, Title FROM ".DB_PRICESETS;
	$result = mysqli_query ($LINK, $query);

	//	get rows into one array and avoid duplicates
	if ($result) {
		while ($row = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
			$list[$row['ID']] = $row['Title'];
		}
	}
	return $list;
}


function FetchPriceSetByID ($ID) {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	return FetchRowsByValue (DB_PRICESETS, "*", 'ID', $ID, '', TRUE);
}


// Get current story IDs for a project
function FetchProjectStories ($projectID) {
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	global $msg, $error;
	global $LINK;

	$query = "SELECT DISTINCT PartID FROM $PARTS WHERE $PARTS.ProjectID = $projectID AND PartTable = '$STORIES'";
	$result = mysqli_query ($LINK, $query);
	$IDlist = array();

	//	get rows into one array and avoid duplicates
	if ($result) {
		while ($row = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
			if (!in_array($row, $IDlist))
				$IDlist[] = $row['PartID'];
		}
	}
	return $IDlist;
}


// Get current story IDs for a project
function FetchProjectStoriesByArtist ($projectID, $artistID) {
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	global $msg, $error;
	global $LINK;

	$query = "SELECT $STORIES.* FROM $PARTS, $STORIES WHERE $PARTS.ProjectID = $projectID AND PartTable = '$STORIES' AND $STORIES.ArtistID = $artistID AND $STORIES.ID = $PARTS.PartID";

	$result = mysqli_query ($LINK, $query);
	$IDlist = array();

	//	get rows into one array and avoid duplicates
	if ($result) {
		while ($row = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
			if (!in_array($row, $IDlist))
				$IDlist[$row['ID']] = $row['Title'];
		}
	}
	return $IDlist;
}

// Get all story IDs for all artists in a project
function FetchAllArtistsStories ($projectID) {
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	global $msg, $error;
	global $LINK;

	$query = "SELECT DISTINCT Stories.*, Firstname, Lastname, CONCAT_WS(' ', Firstname, Lastname) AS Fullname
		from Stories 
		LEFT JOIN Artists ON (Stories.ArtistID = Artists.ID)
		LEFT JOIN Parts ON (Parts.PartID = Artists.ID)
		WHERE Parts.PartTable = \"$ARTISTS\" AND Parts.ProjectID = $projectID;";

	$result = mysqli_query ($LINK, $query);
	$IDlist = array();

	//	get rows into one array and avoid duplicates
	if ($result) {
		while ($row = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
			$IDlist[] = $row['ID'];
		}
	}
	return $IDlist;
}


// Get all story IDs for an artist
function FetchArtistStories ($artistID) {
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	global $msg, $error;
	global $LINK;

	$result = FetchRowsByValue (DB_STORIES, "*", "ArtistID", $artistID, "Title", FALSE);

	$IDlist = array();
	if ($result) {
		while ($row = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
			$IDlist[$row['ID']] = $row['Title'];
		}
	}
	return $IDlist;
}


// =============================================================================
// Given a project ID, return an array of the artists who are part of the project
// These are chosen by looking at the parts of the project, and adding in the
// owner, even if he has no parts in the project
// If $hideuser is set, remove project owner!
// Array (ID=>fullname)
function FetchProjectArtists ($projectID, $fp_user, $hideuser = FALSE, $table = null) {
	global $msg, $error;
	global $LINK;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;

	$DEBUG = 0;
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": *** START FetchProjectArtists<BR>";
	$DEBUG && $msg .= basename(__FILE__) .":". __FUNCTION__ .":".__LINE__ . ": hideuser? $hideuser<BR>";
	// If this is a blank project, there's no ProjectID yet!
	// Can't even get an owner from it!
	$artistlist = array ();

	$table || $table = $PROJECTS;


	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": ProjectID = $projectID<BR>";
	if ($projectID) {
		$project = FetchProject ($projectID);
		$ArtistID = $project['ArtistID'];
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": FetchProjectArtists: Current Artist= $ArtistID<BR>";


		// Get current artist IDs
		$query = "SELECT DISTINCT $ARTISTS.ID AS ArtistID, CONCAT_WS(' ', Firstname, Lastname) AS Fullname FROM $PARTS, $ARTISTS WHERE ($PARTS.ProjectID = $projectID AND PartTable = 'Artists') AND ($ARTISTS.ID = PartID OR $ARTISTS.ID = '$ArtistID')";
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": $query<BR>";

		$result = mysqli_query ($LINK, $query);
		while ($row = mysqli_fetch_array($result)) {
			$ID = $row['ArtistID'];
			$artistlist[$ID] = $row['Fullname'];
			$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": Other artists are " . $row['ArtistID'] . ":" . $row['Fullname'] . "<BR>";
		}
		// Add project owner to the list
		$ownerinfo = FetchArtist($ArtistID);
		$artistlist[$ArtistID] = $ownerinfo['Fullname'];

		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": FetchProjectArtists: artistlist" . ArrayToTable ($artistlist) . "<BR>";
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": *** END FetchProjectArtists<BR>";
	} else {
		// NO projectID, so it's a blank, new project
		// Use the $fp_user (the creator) as the artist
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": No Project ID! <BR>";
		$ArtistID = $fp_user;
		$query = "SELECT ID AS ArtistID, CONCAT_WS(', ', Lastname, Firstname) AS Fullname FROM Artists WHERE (ID = '$ArtistID')";
		$result = mysqli_query ($LINK, $query);
		$row = mysqli_fetch_array($result);
		$artistlist[$row['ArtistID']] = $row['Fullname'];
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": Other artists are " . $row['ArtistID'] . ":" . $row['Fullname'] . "<BR>";
	}
	// Remove current user if HideUser set
	if ($hideuser) {
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.  "Do not show owner, artist $ArtistID<BR>";
		unset ($artistlist[$ArtistID]);
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.  ArrayToTable ($artistlist);
	}
	return $artistlist;
}


// ---------------------------------------
// Fetch all images belonging to a particular artist, return array of ID=>Title
// Used for menus
function FetchArtistImageTitles ($artistID, $showFilename = true) {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	global $LINK;
	
	$where = "WHERE ArtistID = '$artistID'";
	$order = "ORDER by Title";
	$query = "SELECT * FROM $IMAGES $where $order";
	$result = mysqli_query ($LINK, $query);
	$images = array ();
	while ($image = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
		$showFilename
				? $images[$image['ID']] = $image['Title']. " [" . $image['URL'] . "] "
				: $images[$image['ID']] = $image['Title'];
		//$images[$image['ID']] = $image['Title'];
	}

	return $images;
}	

// ---------------------------------------
// Fetch all images belonging to a particular artist, return array of ID=>Title
// Used for menus
function FetchArtistImages ($artistID) {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	global $LINK;
	
	$where = "WHERE ArtistID = '$artistID'";
	$order = "ORDER by ID";
	$query = "SELECT * FROM $IMAGES $where $order";
	$result = mysqli_query ($LINK, $query);
	$pixlist = array ();
	while ($image = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
		$pixlist[$image['ID']] = $image;
	}

	return $pixlist;
}	


// =============================================================================
// FetchArtistProjects ($artistID)
// Given an artist ID, get an array of all projects belonging to that artist
// If $getall is true, get all. Default is only get one project, as an array.
function FetchArtistProjects ($artistID, $getall = false) {
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;
	global $LINK;

	if ($getall) {
		$rows = FetchRowsByValueAsArray ($PROJECTS, "*", "ArtistID", $artistID, "ID", "Title");
		return $rows;
	} else {
		return FetchRowsByValue ($PROJECTS, "*", "ArtistID", $artistID, "Title", true);
	}
}


function FetchArtistPriceSets ($artistID) {
	global $IMAGES, $PRICES, $PRICESETS, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;
	global $LINK;

	return FetchRowsByValue ($PRICESETS, "*", "ID", $artistID, "Title", TRUE);
}


// ---------------------------------------
// Fetch all images belonging to a particular artist, return array of ID=>Title
// Used for menus
function FetchArtistImageIDs ($artistID) {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	global $LINK;
	
	$where = "WHERE ArtistID = '$artistID'";
	$order = "ORDER by ID";
	$query = "SELECT * FROM $IMAGES $where $order";
	$result = mysqli_query ($LINK, $query);
	$idlist = array ();
	while ($image = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
		$idlist[$image['ID']] = $image['ID'];
	}

	return $idlist;
}	


// ---------------------------------------
// Fetch all images belonging to a particular artist, return array of ID=>URL
function FetchArtistImageURLs ($artistID) {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	global $LINK;
	
	$where = "WHERE ArtistID = '$artistID'";
	$order = "ORDER by Title";
	$query = "SELECT ID, URL FROM $IMAGES $where $order";
	$result = mysqli_query ($LINK, $query);
	$images = array ();
	while ($image = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
		$images[$image['ID']] = $image['URL'];
	}

	return $images;
}	


// ====================================================================================
// FetchArtistPortrait ($artistID)
// Given the artist ID, get the artist's portrait as HTML suitable for output
// $size is text added to the file name. The only variation we have right now is "thumbnail".
function FetchArtistPortrait ($artistID, $size = '', $style = null) {
	global $msg, $error;
	global $LINK;
	global $PHOTOS_GALLERY, $THUMBNAILS, $MATTED, $ORIGINALS;

	$DEBUG = 0;

	$artist = FetchArtist ($artistID);

	if ($artist['PortraitImageID'] < 0)
		return false;

	if ($artist['PortraitImageID'] > 0) {
		// use a picture from the galleries
		$DEBUG && $error .= __FUNCTION__.": pictureID = {$artist['PortraitImageID']}<BR>";
		$imageinfo = FetchRowsByValue(DB_IMAGES, "URL", "ID", $artist['PortraitImageID'], '', TRUE);
		switch ($size) {
			case 'thumbnail' : $dir = FP_DIR_THUMBNAILS_IMAGES;
				break;
			default : $dir =  FP_DIR_GALLERY_IMAGES;
		}
		$IMG = URLtoIMG ($dir ."/". $imageinfo['URL'], $style);
		$DEBUG && $error .= __FUNCTION__.": ".FP_DIR_GALLERY_IMAGES ."/". $imageinfo['URL'].", picture = ".htmlentities($IMG)."<BR>";
	} else {
		$size && $size = "_".$size;
		$portraitURL = "artist_portrait_".$artistID."$size.jpg";
		if ( file_exists (FP_DIR_USER_IMAGES . "/" . $portraitURL ) ) {
			// use an uploaded portrait picture
			$DEBUG && $error .= __FUNCTION__.": $portraitURL<BR>";
			$IMG = URLtoIMG (FP_DIR_USER_IMAGES . "/" . $portraitURL, $style);
		} else {
			return false;
		}
	}
	return $IMG;
}


// Get all project image IDs, return array $id=>$name
// Useful for creating option lists of images to choose for a project
// If $artistID is set, limit to images by that artist (necessary for
// the project pictures input form)
function FetchProjectImageIDs ($projectID, $artistID = null, $showFilename = true) {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	global $msg, $error;
	global $LINK;

	$DEBUG = 0;

	$artistID ? $a = "$PARTS.ArtistID = '$artistID' AND" : $a = "";

	$query = "SELECT PartID, Title, URL FROM $PARTS, $IMAGES WHERE $a $PARTS.ProjectID = $projectID AND $PARTS.PartTable = '$IMAGES' and $PARTS.PartID = $IMAGES.ID ORDER BY $IMAGES.Title";
	$result = mysqli_query ($LINK, $query);
	$DEBUG && $msg .= __FUNCTION__.": ".$query."<BR>";
	$IDlist = array();

	//	get rows into one array and avoid duplicates
	if ($result) {
		while ($row = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
			if (!in_array($row, $IDlist))
				$showFilename
						? $IDlist[$row['PartID']] = $row['Title']. " [" . $row['URL'] . "] "
						: $IDlist[$row['PartID']] = $row['Title'];
		}
	}
	return $IDlist;
}	


// Get all project image IDs, return array $id=>$name
// Useful for creating option lists of images to choose for a project
// If $artistID is set, limit to images by that artist (necessary for
// the project pictures input form)
function FetchProjectImageURLs ($projectID, $artistID = null) {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	global $msg, $error;
	global $LINK;

	$DEBUG = 0;

	$artistID ? $a = "$PARTS.ArtistID = '$artistID' AND" : $a = "";

	$query = "SELECT PartID, URL FROM $PARTS, $IMAGES WHERE $a $PARTS.ProjectID = $projectID AND $PARTS.PartTable = '$IMAGES' and $PARTS.PartID = $IMAGES.ID ORDER BY $IMAGES.Title";
	$result = mysqli_query ($LINK, $query);
	$DEBUG && $msg .= __FUNCTION__.": ".$query."<BR>";
	$IDlist = array();

	//	get rows into one array and avoid duplicates
	if ($result) {
		while ($row = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
			if (!in_array($row, $IDlist))
				$IDlist[$row['PartID']] = $row['URL'];
		}
	}
	return $IDlist;
}	


function FetchSupplierIDs () {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES, $SUPPLIERS;
	global $LINK;

	$order = "ORDER by Name";
	$query = "SELECT ID,Name FROM $SUPPLIERS $order";
	$result = mysqli_query ($LINK, $query);
	$vendors = array ();
	while ($vendor = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
		$vendors[$vendor['ID']] = $vendor['Name'];
	}

	return $vendors;
}	

/*
 * Given a name string, e.g. David I. Gross, return the artist's record if it exists
*/
function FindArtistByName ($fullname) {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	global $msg, $error;
	global $LINK;

	$DEBUG = false;
	$VERBOSE = false;

	if ($fullname) {

		preg_match_all ("|(\w+)|", $fullname, $names);
		$DEBUG && $msg .= __FUNCTION__.": Fullname=$fullname, ".ArrayToTable ($names[0]);

		$names = $names[0];

		$lastname = array_pop ($names);
		$firstname = array_shift ($names);
		$middlename = array_pop ($names);

		if ($firstname)
			$w1 .= "Firstname = '$firstname' AND ";
		if ($middlename)
			$w2 .= "Middlename = '$middlename' AND ";

		$where = $w1 . $w2 . "Lastname = '$lastname'";

		$fn = "CONCAT_WS(' ', Firstname, Lastname) AS Fullname";
		$query = "SELECT *, $fn FROM $ARTISTS WHERE $where LIMIT 1";

		$DEBUG && $msg .= __FUNCTION__.": Query: $query<BR>";

		$artistinfo = mysqli_fetch_array(mysqli_query ($LINK, $query), MYSQLI_ASSOC);

		// if that fails, try again without middle initial
		if (!$artistinfo) {
			$where = $w1 . "Lastname = '$lastname'";
			$query = "SELECT *, $fn FROM $ARTISTS WHERE $where LIMIT 1";
			$artistinfo = mysqli_fetch_array(mysqli_query ($LINK, $query), MYSQLI_ASSOC);
		}

		$VERBOSE && fp_error_log(__FUNCTION__.": Attempting to find artist named $firstname $lastname (from $fullname)", 3, FP_PICTURES_LOG);
		return $artistinfo;
	}
	return array();
}

/* 
USER LEVEL SETTINGS FROM SYSCONFIG

FP_ADMINISTRATOR_LEVEL = 1
FP_NORMAL_LEVEL = 2
FP_PERSONAL_LEVEL = 3
FP_SUPPLIER_LEVEL = 4
*/

// ---------------------------------------
// Get the user level for a given artist (i.e. user)
// I'd really prefer to use a number. The field is "AccessLevel" in the database
// This should replace the AccessLevel (above).
// 1 = admin, >1 = user
function FetchAccessLevel ($ArtistID) {
	global $Themes;
	global $PHOTOS_GALLERY, $SLIDES, $THUMBNAILS, $MATTED, $ORIGINALS;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	global $msg, $error;
	global $LINK;

	if ($ArtistID) {
		$artist = GetRecord ($ARTISTS, $ArtistID);
		$accesslevel = $artist['AccessLevel'];
	} else {
		$accesslevel = FP_NORMAL_LEVEL;
	}
	$accesslevel || $accesslevel = FP_NORMAL_LEVEL;
	return $accesslevel;
}


//----------
// Access Level Popup
function BuildAccessLevelMenu ($ArtistID) {
	global $Themes;
	$l = FetchAccessLevel ($ArtistID);
	$text = FetchSnippet ('AccessLevelMenu');
	switch ($l) {
		case "user" :
			$s = "level2";
			break;
		case "admin"	:
			$s = "level1";
			break;
		default :
			$s = "level2";
	}
	$text = Substitutions ($text, array ($s => "SELECTED"));
	return $text;
}


//-------------------------------------------------
// Choose the valid field names from an array, which may contain extra values
// for other purposes than updating a database row
// $validfields is an array of valid field names
// $arr is an array of key, value pairs.
function StripNonExistantFields ($table, $arr) {
	global $Themes;
	$validfields = FetchFieldNames ($table);
	$arrkeys = array_keys ($arr);
	$arr2 = array();
	foreach ($arrkeys as $key) {
		if (in_array($key, $validfields)) {
			$arr2[$key] = $arr[$key];
		}
	}
	return $arr2;
}


// Array of field names from a table.
function FetchFieldNames ($table) {
	global $Themes;
	global $msg, $error;
	global $LINK;
	
	$fieldnames = array();
	$result = mysqli_query ($LINK, "SELECT * from $table");
	($result === false) && $error .= "FetchFieldNames got nothing from table=$table<BR>";
	if ($result) {
		while ($finfo = $result->fetch_field()) {
			$fieldnames[$finfo->name] = $finfo->name;
		}
// 		$k = 0;
// 		$numfields = $result->field_count;
// 		while ($k < $numfields) {
// 			$key = mysqli_fetch_field_direct($result, $k);
// 			$fieldnames[$key] = $key;
// 			$k++;
// 		}
	}
	return $fieldnames;
}



// Field name pop-up menu:
// Build a <select> pop-up selector in HTML of field names of a table
// First use is for editing multiple records...choose the field, enter value, & update.
// Get the list of fields from "formlets/".$table."_fields.txt"
// If not there, build it
//----------------------
function FieldOptionList ($table, $listname, $exceptions = array('ID', 'Active', 'Featured')) {
	global $Themes;
	global $msg, $error;
	global $LINK;
	
	$fieldnames = array();
	if (file_exists ("formlets/".$table."_fields.txt"))
		$fieldnames = file ("formlets/".$table."_fields.txt");

	if (! count($fieldnames)) {
		$result = mysqli_query ($LINK, "SELECT * from $table");
		$k=0;
		if ($result) {
			while ($finfo = $result->fetch_field()) {
				$fieldnames[] = $finfo->name;
			}

// 			while ($k < $result->field_count) {
// 				$res = mysqli_fetch_field_direct($result, $k++);
// 				$fieldnames[] = array ('', $res->name);
// 			}
		}
	}
	$optionlist = "";
	foreach ($fieldnames as $fn) {
		list ($line_num, $fname) = $fn;
		$fname = trim($fname);
		if (! in_array($fname, $exceptions))
			$optionlist .= "<OPTION VALUE = \"$fname\">$fname\n";
	}
	$block = "<SELECT NAME=$listname>\n$optionlist\n</SELECT>";
	return $block;
}

// Field value popup menu:
// Build a <select> pop-up selector in HTML from a field in a table
// $listname is the name for the selection in HTML
// $set = mysql query set of field name, or a calculation like CONCAT_WS(FirstName, LastName)
// $fieldlabel = what to call the field in $set, e.g. LastName or FullName or whatever
// Two fields are retrieved: $set and $fieldlabel
// example: $ArtistIDList = OptionList ($ARTISTS, "ArtistID", "ID", "Lastname", "ID=3", "45", "Fullname", "");
//----------------------
function OptionList ($table, $listname, $set, $fieldlabel, $where, $checked, $order, $size = OPTION_LIST_IS_POPUP, $extrahtml="", $extraline = array()) {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	global $msg, $error;
	global $LINK;
	$DEBUG = FALSE;

	if ($order)
		$order = "ORDER BY $order";
	if ($where)
		$where = "WHERE $where";
	if (!is_array($checked))
		$checked = array ($checked);
	$query = "SELECT $set, $fieldlabel from $table $where $order";
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": $query<BR>";

	// If the field is an AS clause, get the label
	if (preg_match ("/\bAS\b +(.*)/i", $set, $match))
		$set = $match[1];
	$fieldlabelname = $fieldlabel;
	// If the fieldlabel is an AS clause, get the label
	if (preg_match ("/\bAS\b +(.*)/i", $fieldlabel, $match))
		$fieldlabelname = $match[1];
	$optionlist = "";
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": Field label name is $fieldlabelname<BR>";

	$result = mysqli_query ($LINK, $query);
	if ($result) {
		$k = 0;
		$extraline && $optionlist .= "<OPTION VALUE=\"" . $extraline['value'] . "\" " . $extraline['checked'] . ">" . $extraline['label'] ."</OPTION>\n";
		while ($row = mysqli_fetch_array ($result)) {
			$k++;
			$v = trim($row[$set]);
			$l = trim($row[$fieldlabelname]);
			$check = "";
			if (in_array($row[$set], $checked))
				$check = " selected";
			$optionlist .= "<OPTION VALUE=\"$v\" $check>$l</OPTION>\n";
			$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": OPTION VALUE=\"$v\" $check $l<BR>\n";
		}
		if ($size) {
			$k > 10 ? $size = OPTION_LIST_MAXSIZE : $size = $k;
			$size = 'SIZE="' . $size . '" MULTIPLE';
		}
		else {
			$size = "";
		}
		$block = "\n<SELECT NAME='{$listname}' $size $extrahtml>\n$optionlist</SELECT>";
	}
	else {
		$block = "";
	}
	return $block;
}

// Field value popup menu:
// Build a <select> pop-up selector in HTML from an array
// $values is the array of ($value, $name) used in <OPTION VALUE=$value>$NAME</OPTION>
// $listname is the name for the selection in HTML
// $checked is array of checked values (value matches value in $values)
// $sort = true, sort $values by value
// $extraline = array ('value'=>$value, 'checked'=>$checked, 'label'=>$label) where $checked should be text "CHECKED" or ""
// Two fields are retrieved: $set and $fieldlabel
// example: $ArtistIDList = OptionListFromArray ($values, "ID", array("1"), true, true, "", array("0"=>"empty"));

function OptionListFromArray ($values, $listname, $checked = array(), $sort = TRUE, $size = OPTION_LIST_IS_MULTI, $extrahtml="", $extraline = array()) {
	//global $Themes;
	global $msg, $error;
	global $LINK;
	$DEBUG = 0;

	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__.": BEGIN<BR>";

	is_array($values) || $values = array();

	if ($sort)
		asort ($values);

	if (!is_array($checked))
		$checked = array ($checked);

	$DEBUG && $msg .= __FUNCTION__.":".__LINE__.": Checked = ".ArrayToTable($checked);

	$optionlist = "";

	//$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": values:" . ArrayToTable ($values);

	//if ($values) {
	$extraline && $optionlist .= "<OPTION VALUE=\"" . $extraline['value'] . "\" " . $extraline['checked'] . ">" . $extraline['label'] ."</OPTION>\n";

	reset($values);
	$k = 1;
	while (list($ID, $name) = each ($values)) {
		$ID = trim($ID);
		$name = trim($name);
		if ($name[0] != "/") {
			in_array($ID, $checked) ? $check = " selected" : $check = "";
			$optionlist .= "<OPTION VALUE=\"$ID\" $check>$name</OPTION>\n";
			$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": OPTION VALUE=\"$ID\" $check $name<BR>\n";
			$k++;
		}
	}
	if ($size === OPTION_LIST_IS_POPUP) {
		$size = "";
	} elseif (!$size) {
		$k > 10 ? $size = OPTION_LIST_MAXSIZE : $size = $k;
		$size = 'SIZE="' . $size . '" MULTIPLE';
	} else {
		$size = 'SIZE="' . $size . '" MULTIPLE';
	}
	$block = "\n<SELECT NAME=\"$listname\" $size $extrahtml>\n$optionlist</SELECT>\n";

	return $block;

}

// Given a list of values (name, value), create a set of radio buttons or checkboxes
// in HTML code.
function MultipleChoice ($values, $fieldname, $checked = array (), $sort = TRUE, $size = OPTION_LIST_IS_MULTI, $sep = " ") {
	global $Themes;
	global $msg, $error;
	global $LINK;
	$DEBUG = false;

	is_array($values) || $values = array();

	if ($sort)
		asort ($values);

	if (!is_array($checked))
		$checked = array ($checked);

	$optionlist = "";

	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": values:" . ArrayToTable ($values);

	$size == OPTION_LIST_IS_MULTI ? $type = "RADIO" : $type = "CHECKBOX";
	if ($values) {
		while (list($ID, $name) = each ($values)) {
			$ID = trim($ID);
			$name = trim($name);
			in_array($ID, $checked) ? $check = "CHECKED" : $check = "";
			$optionlist .= "<INPUT TYPE=$type NAME=\"$fieldname\" VALUE=\"$ID\" $check>$name$sep\n";
			$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": OPTION VALUE=\"$ID\" $check $name<BR>\n";
		}
	} else {
		$block = "";
	}

	return $block;

}

// Make a checkbox for $fieldname using its current value
// The form update function checks for myfield_CHECKBOX fields and uses them
// to fill in 'myfield', and the hidden 'myfield' makes sure that if the user
// unchecks the box, it will be recorded unchecked, since HTML doesn't report
// unchecked values.
function YesNo ($fieldname, $value, $extrahtml = "") {
	$value ? $checked = "CHECKED" : $checked = "";
	$text = "<input type=CHECKBOX name=\"$fieldname"."_CHECKBOX\" value=\"1\" $extrahtml $checked>\n";
	// hidden field forces value to be recorded even if checkbox is unchecked
	$text .= "<INPUT TYPE=hidden NAME=\"$fieldname\" VALUE=\"0\">";
	return $text;
}



//------------------------
// Pass username/password, return fp_user which is the artist's id.
// On failure, return null, and kill the session.
// 
function VerifyLogin ($username, $password) {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	global $COOKIELIFE, $SYSTEMNAME, $msg, $error;
	global $LINK;
	
	$DEBUG = false;
	if (!(empty($username) OR empty($password))) {
		$fields = "*";
		$where = "WHERE Username = '" . mysqli_real_escape_string($LINK, $username) . "' AND Password = '" . mysqli_real_escape_string($LINK, $password) . "'";
		$query = "SELECT $fields from $ARTISTS $where";
		$result = mysqli_query ($LINK, $query);

		if (mysqli_num_rows($result) == 0) {
			$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": Incorrect password or username: $query";
			return FALSE;
		} else {
			$record = mysqli_fetch_array ($result, MYSQLI_ASSOC);
			$one_userID = $record['ID'];
			$login_ok = $record['ID'];

			$_SESSION['fp_user'] = $record['ID'];
			$_SESSION['StartTime'] = time();
			$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": Matched " . $record['Username'] . "/" . $record['password'] . "<BR>";
			// if user is not confirmed, then confirm the user
			if (! $record['Confirmed']) {
				ConfirmUser ($one_userID);
			}
			return $login_ok;
		}
	}
}

// UNUSED!
// Return the admin menu page, with ArtistID filled in
// If no Artist ID, then return FALSE;
//------------------------
// function GetAdminMenu ($fp_user) {
//	global $Themes;
//	if ($fp_user) {
//		$artistinfo = FetchRowsByValue ($ARTISTS, "*", "ID", $fp_user, '', TRUE);
//		$AccessLevel = $artistinfo['AccessLevel'];
//		if ($AccessLevel == 'admin') {
//			$page = FetchSnippet ("admin_menu_admin");
//		} else {
//			$page = FetchSnippet ("admin_menu_user");
//		}
//		$artistinfo['fp_user'] = $fp_user;
//		$page = Substitutions ($page, $artistinfo);
//		return $page;
//	} else {
//		return FALSE;
//	}
// }

//------------------------
// Uses any fields in an array beginning with 'default' and returns an array
// with their values
function FetchDefaults ($arr) {
	global $Themes, $error, $msg;
	
	!$arr && $arr = array();
	
	$vars = array();
	while (list($name, $value) = each ($arr)) {
		if (preg_match("/^default/i", $name)) {
			$name = substr($name, 7);
			$vars[$name] = $value;
		}
	}
	return $vars;
}


//------------------------
// Remove fields with no value from the array
// This isn't the same as a field with a value of ""!
function StripBlankFields ($arr) {
	global $Themes, $error, $msg;

	$vars = array();
	if ($arr)
	{
		reset($arr);
		while (list ($k, $v) = each($arr)) {				
			if (!empty($v)) {
				$vars[$k] = $v;				
			}
		}
	}
	return $vars;
}

//------------------------------
// Find the ID of a project
// Search an array to find a match for a project name
function FindProjectByName ($arr) {
	global $Themes;
	$DEBUG = false;
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": FindProjectByName START<BR>";
	$articles = array ('the', 'a', 'an', 'in', 'of', 'to');

	// if parameter is not an array, e.g. a string, make it an array
	if (!is_array($arr)) {
		//fp_error_log(__FUNCTION__.": Looking for project '$arr'", 3, FP_PICTURES_LOG);
		$arr = array ($arr);
	}

	// Try to match the nickname
	foreach ($arr as $name) {
		// Check if numeric values match IDs
		if (is_numeric(trim($name))) {
			$projectID = FetchRowsByValue ("Projects", "ID", "ID", $name, '', TRUE);
			if (isset($projectID['ID'])) {
				$projectID = $projectID['ID'];
				$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": Project id = $projectID<BR><BR>";
				return $projectID;
			}
		}

		// Try to match nickname
		$words = explode (" ", $name);
		$name = strtolower($words[0]);
		if (in_array($name, $articles))
			$name = strtolower($words[1]);
		//$query = "select ID from Projects where soundex(Nickname) = soundex('$name')";
		$query = "select ID from Projects where Lower(Nickname) = '$name'";
		$projectID = mysqli_fetch_array(mysqli_query ($LINK, $query), MYSQLI_ASSOC);
		$DEBUG && $msg .= $query . "<BR>";
		if ($projectID) {
			$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": Yep, $name worked.<BR>";
			return $projectID['ID'];
		}
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": Nope, $name failed<BR>";
	}

	// Try to match the Title. We use the full title because
	// it's too hard to get the first word of the title from the DB for the WHERE
	foreach ($arr as $name) {
		$query = "select ID from Projects where soundex(Title) = soundex('$name')";
		$projectID = mysqli_fetch_array(mysqli_query ($LINK, $query), MYSQLI_ASSOC);
		$DEBUG && $msg .= $query . "<BR>";
		if ($projectID) {
			$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": Yep, $name worked.<BR>";
			return $projectID['ID'];
		}
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": Nope, $name failed<BR>";
	}

	return $projectID['ID'];
}


// This creates a javascript for the main HTML page,
// which opens up a message window
// $text is text with $fields in it.  See function FillInInfo for special
global $Themes;
// formatting note on $fields in the text!!!!
function OpenMessageWindow ($text, $title) {
	global $Themes;
	if (!(preg_match("/\<\!\-\-closewindow\-\-\>/", $text))) {
		$text .= '<!--closewindow--><CENTER><a href="Javascript:window.close()">Close this window</a></CENTER>';
	}
	$text = preg_replace ("/\"/", "\\\"", $text);
	$title = preg_replace ("/\"/", "\\\"", $title);

	$p = '
<script type="text/javascript" language="Javascript">

msgWindow=window.open("","displayWindow","menubar=no,scrollbars=yes,status=yes,width=200,height=300");		
msgWindow.document.write ("<HEAD><TITLE>' . $title . '</TITLE></HEAD>");
msgWindow.document.write ("' . $text . '");

</script>
';

	return $p;
}


// FillInInfo
// Given project, artist, image info and a text string,
// fill in the information.
// NOTE: fields for Projects are of the form,
//	"I like $ProjectsTitle" to use Projects.Title
//	"My name is $ArtistsLastname" to use Artists.Lastname
//	"The image is titled, $ImagesTitle" to use Images.Title
function FillInInfo ($text,$ImageID, $ArtistID, $projectID) {
	global $Themes;
	global $PHOTOS_GALLERY, $THUMBNAILS, $MATTED, $ORIGINALS;

	$fields = array();
	$project = FetchProject($projectID);
	reset ($project);
	while (list($key,$value) = each($project)) {
		$fields["$" . "Projects$key"] = $value;
	}
	$artist = FetchArtist($ArtistID);
	reset ($artist);
	while (list($key,$value) = each($artist)) {
		$fields["$" . "Artists$key"] = $value;
	}
	$image = FetchImage($ImageID);
	reset ($image);
	while (list($key,$value) = each($image)) {
		$fields["$" . "Images$key"] = $value;
	}
	if ($fields['ImagesTitle'] == "")
		$fields['ImagesTitle'] = "Untitled";

	//For images
	if (! empty($image['URL'])) {
		$URL = $image['URL'];
		$b = basename($URL);
		$image['URL'] = $b;
		$image['IMG'] = URLtoIMG ($THUMBNAILS . "/" . $b);
		$image['IMG_THUMBNAIL'] = URLtoIMG ($THUMBNAILS . "/" . $b);
		$image['IMG_NORMAL'] = URLtoIMG ($PHOTOS_GALLERY . "/" . $b);

		$image['PHOTOS'] = $PHOTOS_GALLERY;
		$image['THUMBNAILS'] = $THUMBNAILS;
		$image['ORIGINALS'] = $ORIGINALS;
	}

	$text = Substitutions ($text, $image);
	$text = Substitutions ($text, $fields);
	return $text;
}



// ------------------------------------------------------------
// Check if necessary folders exist; create them if necessary
// *** TO DO: Be sure an admin user exists
// *** TO DO: Be sure a public group exists
function ConfirmSetup ( $TableDefinitions = "Tables") {
	global $Themes;
	global $INCOMING, $PHOTOSDIR, $PROCESSEDDIR, $PICTURE_DIRECTORIES;
	global $PHOTOS_GALLERY, $SLIDES, $THUMBNAILS, $MATTED, $FRAMED, $ORIGINALS, $MAILED_DIR;
	global $PROCESSED_PHOTOS, $PROCESSED_SLIDES, $PROCESSED_THUMBNAILS, $PROCESSED_MATTED, $PROCESSED_FRAMED, $PROCESSED_ORIGINALS;
	global $FP_DIR_USER, $AUDIODIR, $SLIDESHOW_AUDIO, $AV;
	global $BASEDIR, $LOGS;
	global $msg, $error;
	global $LINK;

	// tables
	// *** IMPLEMENT THIS FUNCTION...OR FIX...$tables = GetRecords( $TableDefinitions ) ;
	//	ksort ($tables);
	//	$tabledefs = array();
	//	while (list($key, $value) = each ($tables)) {
	//		$tabledefs [$value['Tablename']][] = $value['Fieldname'];
	//	}
	//
	//	while (list($key, $value) = each ($tabledefs)) {
	//		if (!file_exists($key)) {
	//			print "Creating table : $key<BR>";
	//			$fielddefs = join (", ", $value) . "\r";
	//			WriteTextFile($key, $fielddefs);
	//		}
	//	}


	$setupdir = FP_DIR_SETUP;

	// Directories
	// create $PHOTOSDIR, $PHOTOS_GALLERY, $ORIGINALS, $SLIDES, $THUMBNAILS;
	// create same inside $PROCESSED directory
	$old_umask = umask(0);
	$indexfile = "$BASEDIR/$setupdir/files/index.html";

	file_exists("$BASEDIR/$PHOTOSDIR") || mkdir ("$BASEDIR/$PHOTOSDIR", 0755);
	//file_exists("$BASEDIR/$PHOTOSDIR/index.html") || copy ($indexfile, "$BASEDIR/$PHOTOSDIR/index.html");

	// copy htaccess to originals photos directory
	if (!file_exists("$BASEDIR/.htaccess")) {
		if (copy ("$BASEDIR/$setupdir/files/htaccess-photos-originals.txt", "$BASEDIR/.htaccess")) {
			$msg .= "Copied htaccess protection to photos main directory<br>";
			chmod("$BASEDIR/.htaccess", 0644);
		} else {
			$msg .= "*** Failed to copy $BASEDIR/$setupdir/files/htaccess-photos-originals.txt --> $BASEDIR/.htaccess<br>";
		}
	}

	file_exists("$BASEDIR/$PROCESSEDDIR") || mkdir ("$BASEDIR/$PROCESSEDDIR", 0755);
	//file_exists("$BASEDIR/$PROCESSEDDIR/index.html") || copy ($indexfile, "$BASEDIR/$PROCESSEDDIR/index.html");

	foreach ($PICTURE_DIRECTORIES as $dir) {
		$dir= rtrim ($dir, "/");
		file_exists("$BASEDIR/$dir") || mkdir ("$BASEDIR/$dir", 0755);
		//file_exists("$BASEDIR/$dir/index.html") || copy ($indexfile, "$BASEDIR/$dir/index.html");
	}
	// _user directory
	file_exists("$BASEDIR/$FP_DIR_USER") || mkdir ("$BASEDIR/$FP_DIR_USER", 0755);

	// copy htaccess to mailed (input from user) photos directory
	if (!file_exists("$BASEDIR/$FP_DIR_USER/.htaccess")) {
		if (copy ("$BASEDIR/$setupdir/files/htaccess-users.txt", "$BASEDIR/$FP_DIR_USER/.htaccess")) {
			$msg .= "Copied htaccess protection to _user directory<br>";
			chmod("$BASEDIR/$FP_DIR_USER/.htaccess", 0644);
		} else {
			$msg .= "*** Failed to copy $BASEDIR/$setupdir/files/htaccess-users.txt --> $BASEDIR/$FP_DIR_USER/.htaccess<br>";
		}
	}

	// copy htaccess to mailed (input from user) photos directory
	if (!file_exists("$BASEDIR/$MAILED_DIR/.htaccess")) {
		if (copy ("$BASEDIR/$setupdir/files/htaccess-photos-originals.txt", "$BASEDIR/$MAILED_DIR/.htaccess")) {
			$msg .= "Copied htaccess protection to photos upload directory<br>";
			chmod("$BASEDIR/$MAILED_DIR/.htaccess", 0644);
		} else {
			$msg .= "*** Failed to copy $BASEDIR/$setupdir/files/htaccess-photos-originals.txt --> $BASEDIR/$MAILED_DIR/.htaccess<br>";
		}
	}

	// _user/_audio, user slideshow audio
	file_exists("$BASEDIR/$FP_DIR_USER/$AUDIODIR") || mkdir ("$BASEDIR/$FP_DIR_USER/$AUDIODIR", 0755);
	//file_exists("$BASEDIR/$FP_DIR_USER/$AUDIODIR/index.html") || copy ($indexfile, "$BASEDIR/$FP_DIR_USER/$AUDIODIR/index.html");
	file_exists("$BASEDIR/$FP_DIR_USER/$SLIDESHOW_AUDIO") || mkdir ("$BASEDIR/$FP_DIR_USER/$SLIDESHOW_AUDIO", 0755);
	//file_exists("$BASEDIR/$FP_DIR_USER/$SLIDESHOW_AUDIO/index.html") || copy ($indexfile, "$BASEDIR/$FP_DIR_USER/$SLIDESHOW_AUDIO/index.html");

	// _user/video
	//file_exists("$BASEDIR/$FP_DIR_USER/".FP_DIR_VIDEO) || mkdir ("$BASEDIR/$FP_DIR_USER/".FP_DIR_VIDEO, 0755);
	//file_exists("$BASEDIR/$FP_DIR_USER/".FP_DIR_VIDEO."/index.html") || copy ($indexfile, "$BASEDIR/$FP_DIR_USER/".FP_DIR_VIDEO."/index.html");

	// _user/files
	file_exists("$BASEDIR/$FP_DIR_USER/".FP_SUBDIR_USER_FILES) || mkdir ("$BASEDIR/$FP_DIR_USER/".FP_SUBDIR_USER_FILES, 0755);
	//file_exists("$BASEDIR/$FP_DIR_USER/".FP_SUBDIR_USER_FILES."/index.html") || copy ($indexfile, "$BASEDIR/$FP_DIR_USER/".FP_SUBDIR_USER_FILES."/index.html");

	// Logs
	file_exists(FP_DIR_LOG) || mkdir (FP_DIR_LOG, 0755);
	file_exists("$BASEDIR/tmp") || mkdir ("$BASEDIR/tmp", 0755);
	file_exists("$BASEDIR/tmp/cache") || mkdir ("$BASEDIR/tmp/cache", 0777);


}

// Confirm a user
function ConfirmUser ($ArtistID) {
	global $Themes;
	global $msg, $error;
	global $LINK;
	global $ARTISTS, $PROJECTS, $IMAGES, $BASEDIR;

	$artist = FetchArtist ($ArtistID);
	$artist['Confirmed'] = true;
	EditTable ("update", $ARTISTS, $ArtistID, $artist);

}

// Remove any users who aren't confirmed, and who have
// waited too long.
function RemoveOldUsers () {
	global $Themes;
	global $ARTISTS, $PROJECTS, $IMAGES, $BASEDIR;
	global $MAXCONFIRMTIME;
	global $msg, $error;
	global $LINK;

	$DEBUG = 0;

	// Turn this OFF (0) to allow deletion of records. Meanwhile...be sure!
	$doNotDelete = 0;

	$DEBUG && $msg .= __FUNCTION__."<BR>";

	$artists = FetchAllArtistArrays();
	foreach ($artists as $artist) {
		$DEBUG && $msg .= __FUNCTION__.__LINE__.": Testing artist {$artist['ID']}<BR>";
		if (($artist['ID'] != FP_ADMINISTRATOR) && !$artist['Confirmed'] ) {
			$timeStamp = $artist['Timestamp'];
			$delay = time() - strtotime($timeStamp);
			$DEBUG && $msg .= __FUNCTION__.": timeStamp = ($timeStamp)".strtotime($timeStamp)." delay=$delay<BR>";

			if ($delay > FP_MAXCONFIRMTIME) {
				// delete the artist!
				$doNotDelete && $msg .= "Testing is on (nothing deleted)--> ";
				$msg .= "<span class='important'>Deleted Artist " . $artist['Firstname']. " " . $artist['Lastname'] . " for not confirming membership.</span><BR>";
				$doNotDelete || EditTable ("delete", $ARTISTS, $artist['ID'], array());
				$DEBUG && $msg .= __FUNCTION__.__LINE__.": $msg<BR>";
				
				fp_error_log(__FUNCTION__.": Deleted Artist " . $artist['Firstname']. " " . $artist['Lastname'] . " for not confirming membership.", 3, FP_MAINTENANCE_LOG);
			}
		}
	}
}

function dateDiff($dformat, $endDate, $beginDate) {
	$date_parts1=explode($dformat, $beginDate);
	$date_parts2=explode($dformat, $endDate);
	$start_date=gregoriantojd($date_parts1[0], $date_parts1[1], $date_parts1[2]);
	$end_date=gregoriantojd($date_parts2[0], $date_parts2[1], $date_parts2[2]);
	return $end_date - $start_date;
}

function MySQLDateToPHP ($datetime) {
	global $Themes;
	$datetime = date ("Y-m-d h:m:s", $datetime);
	return $datetime;
}

// Replace coded system vars. These are so common we have a function for this.
// The param, $ID, is whatever you want it to be, but it replaces {ID}
function ReplaceSysVars ($form, $table="", $UserID="", $ID="", $nextaction="") {
	global $Themes, $DEVELOPING;
	global $msg, $error;
	global $LINK;
	global $PHOTOS_GALLERY, $SLIDES, $THUMBNAILS, $MATTED, $ORIGINALS, $SITEIMAGES;
	global $SYSTEMNAME, $SERVERNAME, $PHOTOS_GALLERY, $THUMBNAILS, $HIRES, $MAXUPLOADSIZE;
	global $PICTURE_DIRECTORIES, $DATABASES;	// Arrays of names
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	global $SYSTEMURL;
	global $FP_GALLERY_TYPE, $FP_GALLERY_TYPES;
	global $BASEDIR, $ADMINFILENAME, $FP_ORDER, $TABLE_ALIASES, $TABLE_OBJECTS;
	global $FP_CONSTANTS;
	global $groupID;
	global $framewidth;
	global $default_size, $slide_size;	// default picture sizes in gallery
	global $FP_VOCABULARY;
	global $msg, $error;
	global $LINK;


	$DEBUG = 0;
	$TIMER = 0;
	$TIMER && $starttime = microtime(true);

	$age = time() - $_SESSION['StartTime'];
	$sec = $age % 60;
	($sec < 10) && $sec = "0$sec";
	$timemsg = "<i>You've been on the system for " . floor($age / 60) . " minutes</i>";

	// -- Try to get Group ID
	$groupID = $_SESSION['GroupID'];
	if ($groupID) {
		$myGroup = new FPGroup ($LINK, $groupID);
		$myGroup->IsSolo(true)
				? $solo = "_solo"
				: $solo = "";
	}
	
	// $UserID
	// If this is NOT administration usage (i.e., showing the admin theme):
	if (($_SESSION['theme'] != ADMIN_THEME) &&  !$UserID) {
		// Set $UserID if not already set, based on group/project
		// If all else fails, use admin. 
		if ($_SESSION['projectid']) {
			$project = FetchProject($_SESSION['projectid']);
			$UserID = $project['ArtistID'];
		} else {
			if ($groupID) {
				$UserID = $myGroup->owner;
			} else {
				$UserID = FP_ADMINISTRATOR;
			}
		}
	} else {
		$UserID || $UserID = ($_SESSION['fp_user']);

		//$UserID || $error .= "Error: I am confused about who is using the system!!!";

		// This seems like a BAD idea! If there's no fp_user and
		// we're showing admin theme, shouldn't we just log out?!??!?
		$UserID || $UserID = FP_ADMINISTRATOR;
	}

	// If userID is not the admin user, reload the themes. NOT EFFICIENT, BUT WORKS FOR NOW, I HOPE!
	if ($UserID != FP_ADMINISTRATOR && $Themes->userID != $UserID) {
		$Themes->userID = $UserID;
		$Themes->LoadAllThemes(true);
	}

	$user = GetRecord(DB_ARTISTS, $UserID);
	$accessLevel = FetchAccessLevel ($UserID);
	$UserFullName = join (" ", array($user['Firstname'], $user['Lastname']));

	// User Subscription Info
	// Private installation gets all subscription info from Admin user
	if ($UserID != FP_ADMINISTRATOR && FP_INSTALLATION_TYPE == FP_INSTALLATION_PRIVATE) {
		$SubscriptionDesc = FetchSnippet ("guest_account_desc");
	} else {
		$user['SubscriptionDesc'] ? $SubscriptionDesc = $user['SubscriptionDesc'] : $SubscriptionDesc = "{fp:trial_account}";
		$SubscriptionDesc = str_replace ("{SUBSCRIPTION_DESC}", $SubscriptionDesc, FetchSnippet ("subscription_desc_wrapper"));
	}

	$SubscriptionDesc = convertDollarToHTML(htmlentities($SubscriptionDesc));

	// -- Try to get Group ID
	$groupID = $_SESSION['GroupID'];
	$groupID || (isset ($user['GroupID']) && $groupID = $user['GroupID']);



	// ----- GET THEME DIRECTORY NAME
	// ** WTF? ***
	if ($UserID)
		$Themes->userID = $UserID;
	$myThemeID = CurrentThemeID ();
	if ($myThemeID != ADMIN_THEME && (!$myThemeID || !isset($Themes->FP_THEMES_LIST[$myThemeID])))
		$myThemeID = DEFAULT_THEME;
	$Themes->themeID = $myThemeID;
	$myTheme = $Themes->FP_Themes[$myThemeID];

	if ($DEBUG) {
		print __FUNCTION__.": myThemeID = $myThemeID<br>";
		print "<BR>";
	}

	// Now we have the theme, get the theme directory
	$res = $Themes->FetchCurrentThemeDirs($myThemeID);
	$myThemePath = $Themes->theme;	// name of theme javascript file
	$jslibfile = $Themes->jslibfile;		// name of theme javascript library file
	$jsfile = $Themes->jsfile;	// name of theme javascript file
	$jspath = $Themes->jspath;	// full path to theme javascript
	$css = $Themes->css;	// full path to theme style css
	$altCSS = $Themes->altCSS;	// full path to theme style css
	$extraCSS = $Themes->extraCSS;	// full path to theme extra styling
	$userVariationCSS = $Themes->userVariationCSS;		// full path to USER variation css
	$adminVariationCSS = $Themes->adminVariationCSS;	// full path to ADMIN variation css
	$systemVariationCSS = $Themes->systemVariationCSS;	// full path to SYSTEM variation css
	$newsletter = $Themes->newsletterCSS;	// full path to newsletter stylesheet


	// Web Fonts info for this group (gallery)
	// If missing, get it from the main gallery
	$webfontsCSS = $myGroup->params[FP_PARAM_GROUP_WEB_FONTS_CSS];
	
	if ($webfontsCSS) {
		$webFontStylesheet = Substitutions (FetchSnippet ("webfont_css_wrapper"), array("webfontcss"=>$webfontsCSS) );
		$webFontList = $myGroup->params[FP_PARAM_GROUP_WEB_FONTS_NAMES];
		$webFontHead = $myGroup->params[FP_PARAM_GROUP_WEB_FONTS_HEAD];
	} elseif ($groupID != PUBLIC_GROUP_ID) {
		$temp = new FPGroup ($LINK, PUBLIC_GROUP_ID);
		$webfontsCSS = $temp->params[FP_PARAM_GROUP_WEB_FONTS_CSS];
		if ($webfontsCSS) {
			$webFontStylesheet = Substitutions (FetchSnippet ("webfont_css_wrapper"), array("webfontcss"=>$webfontsCSS) );
			$webFontList = $temp->params[FP_PARAM_GROUP_WEB_FONTS_NAMES];
			$webFontHead = $temp->params[FP_PARAM_GROUP_WEB_FONTS_HEAD];
		} else {
			$webFontStylesheet = "";
			$webFontList = "";
			$webFontHead = "<!-- NO WEB FONT HEAD (A) -->";
		}
	} else {
		$webFontStylesheet = "";
		$webFontList = "";
		$webFontHead = "<!-- NO WEB FONT HEAD (B) -->";
	}

	// Theme directories as a Javascript array.
	// Used to make a list of theme dirs, so javascript can find previews of themes
	$themePreviewPaths = join(', ', $Themes->FP_THEMES_PREVIEWS_FOR_JS);
	$themePreviewPaths = "{ ".$themePreviewPaths." }";

	// This is really a stand-alone style sheet.
	//$newsletter = $css4;

	// get the extra CSS path/name
	$sheetnames = array ();
	$css && $sheetnames[] = $css;
	$extraCSS && $sheetnames[] = $extraCSS;
	$systemVariationCSS && $sheetnames[] = $systemVariationCSS;
	$adminVariationCSS && $sheetnames[] = $adminVariationCSS;
	$userVariationCSS && $sheetnames[] = $userVariationCSS;
	//$newsletter && $sheetnames[] = $newsletter;

	// If this is admin, then get the first project of the user to put into the "Images" menu item
	// This lets us choose to show pix from the first project, instead of all pix, which can be very
	// very slow!
	if ($_SESSION['theme'] == ADMIN_THEME) {
		if ($UserID != FP_ADMINISTRATOR) {
			$projectlist = FetchArtistProjects ($UserID);
		} else {
			$projectlist = FetchFirstRowAsArray ($PROJECTS, "*");
		}
		$FIRST_PROJECT_ID = $projectlist['ID'];
	} else {
		 $FIRST_PROJECT_ID;
	}
	
	$PACKAGING_METHOD_LABEL = FetchSnippet("packaging_label_".FP_PACKAGING_METHOD);
	$PACKAGING_METHOD_TIP = FetchSnippet("packaging_tip_".FP_PACKAGING_METHOD);
	
	
	
	$table ? $tablealias = $TABLE_ALIASES[$table] : $tablealias = null;
	$table ? $tableobject = $TABLE_OBJECTS[$table] : $tableobject = null;

	$DEBUG && $msg .= __FUNCTION__.": TableAlias = $tablealias<BR>";

	$vars = array ( "SubscriptionDesc"				=>	$SubscriptionDesc,
			"message"					=>	$msg,
			"error"						=>	$error,
			'grouplogo'					=>	$myGroup->icon,
			//'grouplogo'					=>	$group['Icon'],
			// used to set CSS class of title box on page; default is blank here
			'sectionclass'				=>	"",
			"slogan"						=>	FetchSnippet("slogan"),
			"timemsg"					=>	$timemsg,
			"GALLERYFADEDARKNESS"			=>	GALLERYFADEDARKNESS,
			"JAVASCRIPT_FP_LIB"			=>	$jslibfile,
			"JAVASCRIPT"					=>	$jsfile,
			"JAVASCRIPT_PATH"			=>	$jspath,
			"STYLESHEET"					=>	$css,
			"EXTRA_STYLESHEET"			=>	$extraCSS,
			"USER_VARIATION_STYLESHEET"		=>	$userVariationCSS,
			"ADMIN_VARIATION_STYLESHEET"		=>	$adminVariationCSS,
			"SYSTEM_VARIATION_STYLESHEET"		=>	$systemVariationCSS,
			"GALLERY_STYLESHEET"			=>	$gallerycss,		// NOT IN USE
			"STYLESHEETS"				=>	StyleSheetHTML ($sheetnames),
			"FP_GALLERY_TYPE"			=>	FP_GALLERY_TYPE,

			// WEB FONTS
			"MASTER_PAGE_HEAD_WEBFONTS"	=>  $webFontHead,
			"WEBFONT_STYLESHEET"		=>  $webFontStylesheet,
			"FP_WEB_FONT_LIST"			=>	$webFontList,


			// SYSTEM VARIABLES
			"FP_SINGLE_GALLERY_SINGLE_USER" =>	FP_SINGLE_GALLERY_SINGLE_USER,
			"FP_SINGLE_GALLERY_MULTI_USER"	=>	FP_SINGLE_GALLERY_MULTI_USER,
			"FP_MULTI_GALLERY_SINGLE_USER"	=>	FP_MULTI_GALLERY_SINGLE_USER,
			"FP_MULTI_GALLERY_MULTI_USER"	=>	FP_MULTI_GALLERY_MULTI_USER,
			"FP_INSTALLATION_SHOW_HIDE" =>	CSSShowHide (FP_INSTALLATION_SHOW_HIDE),

			// SIGNUP WITH PP SYSTEM
			"PP_URL"						=>	PP_URL,
			"PP_SERVICE_ID"				=>	PP_SERVICE_ID,

			// COMMERCE and FRAMESHOP
			"PACKAGING_METHOD_LABEL"			=> $PACKAGING_METHOD_LABEL,
			"PACKAGING_METHOD_TIP"				=> $PACKAGING_METHOD_TIP,
			"CURRENCY"					=> CURRENCY,
			"CURRENCY_AFTER"				=> CURRENCY_AFTER,
			"UNIT"						=> UNIT,
			"UNITS"						=> UNITS,
			"WEIGHT_UNIT"					=> WEIGHT_UNIT,
			"CURRENCY"					=>	CURRENCY,
			"CURRENCY_POSITION"			=>	CURRENCY_POSITION,
			"FRAMESHOP_REF_SIZE"			=>	FRAMESHOP_REF_SIZE,
			"PAYPAL_NOTIFY_URL"			=>	PAYPAL_NOTIFY_URL,
			"PAYPAL_BUSINESS_EMAIL"		=>	PAYPAL_BUSINESS_EMAIL,
			"PAYPAL_CURRENCY_CODE"		=>	PAYPAL_CURRENCY_CODE,
			"PAYPAL_POST_TO_URL"			=>	PAYPAL_POST_TO_URL,
			"MATTE_BORDER_WIDTH"			=>	MATTE_BORDER_WIDTH,
			"FRAMESHOP_ART_BORDER"		=>	FRAMESHOP_ART_BORDER,
			"FRAMESHOP_ART_BORDER_COLOR"	=>	FRAMESHOP_ART_BORDER_COLOR,
			"FRAMESHOP_ART_BORDER_NOFRAME"	=>	FRAMESHOP_ART_BORDER_NOFRAME,
			"SHOPPING_CART"				=>	FetchSnippet (FP_SHOW_CART_NAME),
			"NAVBAR_SHOPPING_CART"		=>	FetchSnippet (FP_NAVBAR_SHOW_CART_NAME),
			"FRAMESHOP_PICTURE_SCALING" =>	FRAMESHOP_PICTURE_SCALING,

			// Theme
			"THEMESDIR"					=> FP_THEMES_DIR,
			"THEME"						=> $myThemePath,
			"THEME_NAME"					=> $myTheme['name'],
			"THEME_ID"					=> $myThemeID,
			"DEFAULT_THEME"					=> DEFAULT_THEME,
			"THEMEPREVIEWPATHS"				=> $themePreviewPaths,

			// DISPLAY
			"FP_CURRENT_IMAGE_INDEX"		=>	$_REQUEST['FP_CURRENT_IMAGE_INDEX'] + 0,
			"FP_PREVIOUS_URL"			=>	$_REQUEST['FP_PREVIOUS_URL'],
			"SOLIDIFY"				=> (FP_SOLIDIFY ? "solid" : ""),

			// Basic values that might appear in more complex values, above
			"table"						=>	$table,
			"FP_USER_TYPE_NAME"			=>	$FP_GALLERY_TYPES[$FP_GALLERY_TYPE],
			"TABLE_ALIAS"				=>	$tablealias,
			"TABLE_OBJECT"				=>	$tableobject,
			//"UserID"					=>	$_SESSION['UserID'],
			"ACCESSLEVEL"				=>	$accessLevel,
			"UserFullName"				=>	$UserFullName,
			"UserFirstname"				=>	$user['Firstname'],
			"UserLastname"				=>	$user['Lastname'],
			"ID"						=>	$ID,
			"basename"					=>	basename($_SERVER['SCRIPT_FILENAME']),
			"groupID"					=>	$groupID,
			'grouptitle'					=>	$myGroup->title,
			//'grouptitle'					=>	$group['Title'],
			"FP_USER_ID"				=>	$UserID,
			"DefaultXSize"				=>	$default_size[0],
			"DefaultYSize"				=>	$default_size[1],
			"SlideXSize"					=>	$slide_size[0],
			"SlideYSize"					=>	$slide_size[1],
			"SLIDE_SHOW_WIDTH"			=>	$slide_size[0]+20,
			"SLIDE_SHOW_HEIGHT"			=>	$slide_size[1]+50,
			"UserFullName"				=>	$UserFullName,
			"nextaction"					=>	$nextaction,
			"SYSTEMNAME"					=>	$SYSTEMNAME,
			"SERVERNAME"					=>	$SERVERNAME,
			"BASEDIR"					=>	$BASEDIR,
			"ADMINFILENAME"				=>	$ADMINFILENAME,
			"FRAMEWIDTH"					=>	$framewidth,
			"MATTESCALE"					=>	MATTESCALE,
			"MATTECOLOR"					=>	MATTECOLOR,
			"MATTEBOTTOM"				=>	MATTEBOTTOM,
			"SYSTEMURL"					=>	$SYSTEMURL,
			"session_id"					=> session_id(),
			"GROUP_IS_SOLO"				=> $solo,
			"DATETIME"				=> date('l jS \of F Y h:i:s A'),
			"DATETIME_COMPACT"			=> date("D M j G:i:s T Y", time()),


			// MENU BAR
			"FIRST_PROJECT_ID"			=> $FIRST_PROJECT_ID
	);


	// If user ID passed to the function, substitute for 'UserID'
	$UserID && $vars['UserID'] = $UserID;

	// Replace user codes to insert or link to files, e.g. [[myfile:My File]]
	($myThemeID != ADMIN_THEME) && $form = ReplaceUserFileCodes ($form, $UserID);

	// Add in Database names (from sysconfig)
	$vars = array_merge($vars, $DATABASES);

	// Add in Database names (from sysconfig)
	$vars = array_merge($vars, $FP_ORDER);

	// Add in Directories (from sysconfig)
	$vars = array_merge($vars, $PICTURE_DIRECTORIES);

	$form = Substitutions ($form, $vars);

	// Add in ALL user-defined (by FP) constants
	$form = Substitutions ($form, $FP_CONSTANTS);

	// Replace vocabulary from the "vocabulary" snippet
	// Key words are marked {FP:word}
	$form = ReplaceVocabulary ($form);
	$TIMER && $error .= __FUNCTION__.__LINE__.": Time elapsed: ".round (microtime(true) - $starttime, 3) . "<BR>";

	return $form;
}


// Format for a use code: [[filename.ext:description:x-size:y-size]]
// The x/y sizes are used for SWF and Movie files, not JPG.
// The $artistID determines which files directory the files are taken from
function ReplaceUserFileCodes ($text, $artistID) {
	global $msg, $error;
	global $LINK;
	global $SYSTEMURL, $BASEDIR, $USER_IMAGE_EXTENSIONS, $USER_VIDEO_EXTENSIONS, $USER_FLASH_EXTENSIONS;

	if (!$text)
		return false;

	$userDir = GetUserDirectory ($artistID, true);	// true means get a relative path, not the absolute, starting in website root dir

	$matches = array();
	$count = preg_match_all("/".preg_quote(FP_INSERT_USERFILE_CODE_OPEN)."(.*?)".preg_quote(FP_INSERT_USERFILE_CODE_CLOSE)."/i", $text, $matches, PREG_PATTERN_ORDER);
	if ($count) {
		$search = array();
		$replace = array();
		$embedV = FetchSnippet ("user_file_embed_video");
		$embedI = FetchSnippet ("user_file_embed_img");
		$link = FetchSnippet ("user_file_link");
		for ($i=0;$i<count($matches[0]);$i++) {
			list ($filename, $desc) = explode(":", $matches[1][$i]);
			// does the user file exists?
			$path = FP_BASEDIR."/".$userDir.FP_SUBDIR_USER_FILES."/".$filename;
			$adminpath = FP_BASEDIR."/".FP_DIR_USER."/".FP_SUBDIR_USER_FILES."/".$filename;
			$photopath = FP_BASEDIR."/".FP_DIR_GALLERY_IMAGES."/".$filename;
			$webpath = $userDir.FP_SUBDIR_USER_FILES."/".$filename;

			$found = false;
			if (file_exists($path)) {
				$subdir = FP_HOMEDIR.$userDir.FP_SUBDIR_USER_FILES."/".$filename;
				$found = true;
			} else if (file_exists($adminpath)) {
				$subdir =  FP_HOMEDIR.FP_DIR_USER."/".FP_SUBDIR_USER_FILES."/".$filename;
				$found = true;
			} else if (file_exists($photopath)) {
				$path = FP_DIR_GALLERY_IMAGES."/".$filename;
				$found = true;
			}


			if ($found) {
				$ext = pathinfo($path, PATHINFO_EXTENSION);
				// Get template for link/embed
				if (in_array($ext, $USER_IMAGE_EXTENSIONS)) {
					$t = $embedI;
				} else if (in_array($ext, $USER_VIDEO_EXTENSIONS)) {
					$t = $embedV;
				} else {
					$t = $link;
				}
				//$t = str_replace ("{PATH}", FP_HOMEDIR.$subdir.FP_SUBDIR_USER_FILES."/".$filename, $t);
				$t = str_replace ("{PATH}", $webpath, $t);
				$t = str_replace ("{TEXT}", $desc, $t);
				$search[] = "/".preg_quote($matches[0][$i])."/";
				$replace[] = $t;
			}
		}
		$text = preg_replace ($search, $replace, $text);
	}
	return $text;
}

// quickie function to return the CSS show or hide code
// true = show, false = hide
function CSSShowHide ($x) {
	if ($x) {
		return FP_CSS_SHOW;
	} else {
		return FP_CSS_HIDE;
	}
}

function FetchAccesslevelDesc ($accessLevel) {
	global $Themes;
	$desc = FetchSnippet ("accesslevel" . $accessLevel);
	return $desc;
}

function FetchUserLevelDesc ($UserType) {
	global $Themes;
	$desc = FetchSnippet ("userlevel" . $UserType);
	return $desc;
}


// List of pictures belonging to all artists in project $projectID
// returns Array
function ProjectArtistsPixlist ($projectID) {
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	global $msg, $error;
	global $LINK;

	$project = new FPProject ($LINK, $projectID);
	$artistlist = array_keys($project->MemberNames());
	$allpix = array();

	foreach ($artistlist as $artistID) {
		$artistPix = FetchArtistImages ($artistID);
		$allpix = $allpix + $artistPix;
	}
	return $allpix;
}



// List of pictures in project $ID
// returns Array
function Pixlist ($projectID) {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	global $msg, $error;
	global $LINK;

	$DEBUG = 0;
	$pixlist = array();
	$record = FetchProject ($projectID);
	$project = new FPProject ($LINK, $projectID);
	$pixlist = $project->PictureList();
	return $pixlist;
}

// This gets an array useful for reordering image parts
// It returns $myarray[$partID] = $Title
// The $template allows me to specify exactly what the value is
// *** BE CAREFUL ABOUT FIELD NAMES. There are dup names (ID) from the SQL search ***
function PixlistForOrdering ($ID, $template = "{title}") {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	global $msg, $error;
	global $LINK;

	$DEBUG = false;
	$pixlist = array();
	$record = GetRecord ("Projects", $ID);
	// Wrong! This is only pix originally given the projectID, not the pix in the project!
	//$query = "select * from $IMAGES where ProjectID = '$ID'";
	// Here we are...get all Image parts for a project!
	$query = "SELECT DISTINCT *, $PARTS.ID as PID, $IMAGES.ID AS ImageID, $IMAGES.ProjectID as ProjectID from $IMAGES, $PARTS WHERE Parts.ProjectID = '$ID' AND Parts.PartTable = '$IMAGES' AND Images.ID = Parts.PartID ORDER BY $PARTS.OrderInProject";
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.":$query<BR>";
	$result = mysqli_query ($LINK, $query);
	while ($part = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
		$text = Substitutions ($template, $part);
		$pixlist[$part['PID']] = $text;
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__	. ": " . $part['OrderInProject'] . ":" . $part['Title'] . "<BR>";
	}
	return $pixlist;
}

// This gets an array useful for reordering image parts
// It returns $myarray[$imageID] = $partID
function PartsForImages ($ID) {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	global $msg, $error;
	global $LINK;

	$DEBUG = false;
	$pixlist = array();
	$record = GetRecord ("Projects", $ID);
	$query = "SELECT DISTINCT *, $PARTS.ID as PID, $IMAGES.ID AS ImageID, $IMAGES.ProjectID as ProjectID from $IMAGES, $PARTS WHERE Parts.ProjectID = '$ID' AND Parts.PartTable = '$IMAGES' AND Images.ID = Parts.PartID ORDER BY $PARTS.OrderInProject";
	$result = mysqli_query ($LINK, $query);
	while ($part = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
		$pixlist[$part['ImageID']] = $part['PID'];
	}
	return $pixlist;
}



// Replace all coded words in a string with text files in the snippet directory
// Coding is {name_of_file}, where the filename to use is in snippets folder,
// and the name is name_of_file.txt
function ReplaceSnippets ($form) {
	global $Themes;
	global $SNIPPETS, $SNIPPETSDIR;

	isset($_SESSION['theme']) || $_SESSION['theme'] = DEFAULT_THEME;
	$myTheme = $_SESSION['theme'];
	$pattern = "/(\{.*?\})/";	// the ? makes the .* ungreedy
	$matches = array();
	preg_match_all ($pattern, $form, $matches);
	foreach ($matches[0] as $keyword) {
		$filename = trim ($keyword, "{}");
		if (file_exists("$myTheme$SNIPPETSDIR/".$filename.".txt")) {
			$keywordtext = FetchSnippet($filename);
			$form = preg_replace ("/$keyword/i", $keywordtext, $form);
		}
	}
	return $form;
}


// Get list of images and their order in the project
function FetchProjectImagesByAge ($projectID) {
	global $LINK;
	
//$query = "SELECT PartID, OrderInProject FROM Parts WHERE Parts.ProjectID = $projectID AND PartTable = 'Images'";
	$query = "SELECT DISTINCT Images.ID, TimeStamp FROM Parts, Images WHERE Parts.ProjectID = 1 and Parts.PartTable = 'Images' and Images.ID = Parts.PartID ORDER BY TimeStamp";
	$result = mysqli_query ($LINK, $query);
	$IDlist = array();

	if ($result) {
		while ($row = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
			$IDlist[ $row['ID'] ] = $row['Timestamp'];
		}
	}
	return $IDlist;
}


// Get list of images and their order in the project
// $array[id] = ordernumber
// It's a strange array, but actually most useful for building lists with while()
function FetchProjectImageOrder ($projectID) {
	global $LINK;
	
	$query = "SELECT DISTINCT PartID, OrderInProject FROM Parts WHERE Parts.ProjectID = $projectID AND PartTable = 'Images' ORDER BY PartID, OrderInProject";
	$IDlist = array();

	$result = mysqli_query ($LINK, $query);
	$k = 1;
	if ($result) {
		$numpix = mysqli_num_rows ($result);
		while ($row = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
			if ($row['OrderInProject']) {
				$order = $k;
				$k++;
			} else {
				$order = $numpix;
				$numpix--;
			}
			$IDlist[ $row['PartID'] ] = $order;
		}
	}
	return $IDlist;
}



// Get current image IDs for a project
function FetchProjectImageList ($projectID) {
	global $LINK;
	
	$query = "SELECT PartID FROM Parts WHERE Parts.ProjectID = $projectID AND PartTable = 'Images'";
	$result = mysqli_query ($LINK, $query);

	$IDlist = array();

	//	get rows into one array and avoid duplicates
	if ($result) {
		while ($row = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
			if (!in_array($row, $IDlist))
				$IDlist[] = $row['PartID'];
		}
	}
	return $IDlist;
}


function FetchProjectImageCount ($projectID) {
	global $LINK;
	
	$query = "SELECT DISTINCT PartID FROM Parts WHERE Parts.ProjectID = $projectID AND PartTable = 'Images'";
	$result = mysqli_query ($LINK, $query);
	$num_rows = mysqli_num_rows ($result);
	return $num_rows;
}

// --------------------
// a call to add prefix text to each item in an array
// for ArrayPrefix function
function PrefixItem (&$item, $key, $prefix) {
	$item = $prefix . $item;
}

// --------------------
// a call to add prefix text to each item in an array
// for ArrayPrefix function
function SuffixItem (&$item, $key, $suffix) {
	$item = $item . $suffix;
}

// Prefix VALUES in array (not indeces)!
function ArrayPrefix ($arr, $prefix) {
	array_walk ($arr, 'PrefixItem', $prefix);
	return $arr;
}

// Wrap VALUES in array (not indeces) with text.
function ArrayWrap ($arr, $prefix, $suffix) {
	array_walk ($arr, 'PrefixItem', $prefix);
	array_walk ($arr, 'SuffixItem', $suffix);
	return $arr;
}


// --------------------
function GetThumbnailURL ($URL, $kind) {
	global $PHOTOS_GALLERY, $THUMBNAILS, $MATTED, $ORIGINALS;
	$d = dirname($URL);
	$b = basename($URL);
	$f = $$kind . "/" . $b;
	return $f;
}


//------------------
function StartDatabase($myDB) {
	global $DEPENDENCIES_FILE, $DEPENDENCIES;
	global $LINK, $LINK;
	global $msg, $error;

	$DEBUG = FALSE;
	// Start up the database

	$DEBUG && print "BEGIN StartDatabase<BR>";
	$LINK = mysqli_connect(MYSQLHOST, MYSQLUSER, MYSQLPASSWORD, $myDB);
//			or die("Could not connect to the site...the server must be very busy.");

	if (!$LINK) {
		die('Connect Error (' . mysqli_connect_errno() . ') '. mysqli_connect_error() );
	}

	if ($DEBUG)
		print "Connected successfully:" . mysqli_get_host_info($link) . "\n" . "<BR>";

	// Select the DATABASE
// 	mysqli_select_db($LINK, $myDB)
// 			or die("Could not select database $myDB");

	$DEPENDENCIES = GetDependencies($DEPENDENCIES_FILE);
	$DEBUG && print "END StartDatabase<BR>";

	
	// ** Make a new object, too!
	//$LINK = new mysqli(MYSQLHOST, MYSQLUSER, MYSQLPASSWORD, $myDB);
	//$LINK = $LINK;

// 	if (mysqli_connect_error()) 
// 		{
// 		die('Connect Error (' . mysqli_connect_errno() . ') ' . mysqli_connect_error());
// 		}

	return $LINK;
}


//------------------
// get contents of a file into a string
function ReadTextFile ($filename) {
	if (file_exists($filename)) {
		return file_get_contents ($filename);
	} else {
		return "file $filename not found";
	}
}

function WriteTextFile ($filename, $str) {
	$fp = fopen($filename, "w+");
	if(!$fp) {
		return FALSE;
	}
	fputs($fp, $str);
	fclose ($fp);
	return TRUE;
}


/* Show an array */
function PrintArray ($myarray) {
	print "<table border=1>\n";
	reset ($myarray);
	while (list($k, $v) =  each ($myarray)) {
		print "<tr><td>$k</td><td>$v</td><tr>\n";
	}
	print "</table>\n";
}

/* Make a table from an array */
// Make an HTML table from an array (make it vertical)
function ArrayToTable ($myarray) {
	if ($myarray && is_array ($myarray)) {
		//$table = "<table border=\"1\" width=\"100%\" >\n";
		$table = "<table style='border:1px solid gray;'>\n";
		reset ($myarray);
		while (list($k, $v) =  each ($myarray)) {
			if (! $v)
				$v = " ";
			is_array ($v) && $v = ArrayToTable ($v);
			$v = htmlentities ($v);
			$v = preg_replace ("/\{/", "&#123;", $v);
			$table .= "<tr ><td style='border:1px solid black;padding:2px;'>$k</td><td style='border:1px solid black;padding:2px;'>$v</td></tr>\n";
			/*		if ($DEBUG) */
			/*			print "processing $k, $v<BR>"; */
		}
		$table .= "</table>\n";
	} else {
		$table = "<BR>(empty array)<BR>";
	}
	return $table;
}

// Make an HTML table from an array (make it horizontal, just VALUES)
function ArrayToTableH ($myarray) {
	$table = "<table border=1 width=100%><tr>\n";
	reset ($myarray);
	while (list($k, $v) =  each ($myarray)) {
		if (! $v)
			$v = " ";
		$v = htmlentities ($v);
		$v = preg_replace ("/\{/", "&#123;", $v);
		$table .= "<td>$v</td>\n";
		/*		if ($DEBUG) */
		/*			print "processing $k, $v<BR>"; */
	}
	$table .= "</TR></table>\n";
	return $table;
}

/* Create a table from a SELECT result */
function ResultToTable ($result) {
	$table = "<table border=1 cellpadding=5 width=100%>\n";
	while ($line = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
		$table .= "\t<tr>\n";
		foreach ($line as $col_value) {
			$table .= "\t\t<td>$col_value</td>\n";
		}
		$table .= "\t</tr>\n";
	}
	$table .= "</table>\n";
	mysqli_data_seek ($result,0);	//reset the pointer
	return $table;
}


/* Build a list from a mysql result */
/* Codes used: $lineformat is a line; each occurance of "$field" will be replaced with a field */
/*			$mainformat contains $list, which is replaced with the list of formatted lines */
function ResultToList ($result, $mainformat, $lineformat) {
	$block = $mainformat;
	$list = "";
	while ($line = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
		$output = $lineformat;
		foreach ($line as $field) {
			$output = preg_replace ('/\{field\}/', $field, $output,1);
		}
		$list .= $output;
	}
	$block = str_replace ('{list}', $list, $block);
	mysqli_data_seek ($result,0);	//reset the pointer
	return $block;
}


// Fetch XML data on an Artist from remote address
// This is for use with LightStalkers
// Prefix allows you to identify the var names in the template
// for use with Substitution(). For LightStalkers, these vars are LS_name, etc.
function FetchRemoteLSData ($URL, $prefix = "") {
	global $Themes;
	global $msg, $error;
	global $LINK;

	$DEBUG = false;
	if (file_exists ($URL)) {
		$xmlfile = file_get_contents ($URL);
	} else {
		$xmlfile = "";
	}
	$xmlObj = new SimpleXMLParser;
	$xml = $xmlObj->Parse($xmlfile, "person");

	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": From $URL<HR>";
	$record = array();
	//$record["Name"] = array ("Name", $xmlObj->Parse($xml, "name", 0));
	$record["Profession"] = array ("Profession", $xmlObj->Parse($xml, "profession", 0));
	$record["Organization"] = array ("Organization", $xmlObj->Parse($xml, "agencyrep", 0));
	$record["Location"] = array ("Current Location", $xmlObj->Parse($xml, "location", 0));
	//$record["Latitude"] = array ("Latitude", $xmlObj->Parse($xml, "latitude", 0));
	//$record["Longitude"] = array ("Longitude", $xmlObj->Parse($xml, "longitude", 0));
	$record["URL1"] = array ("Web Site", $xmlObj->Parse($xml, "url1", 0));
	$record["URL2"] = array ("Web Site", $xmlObj->Parse($xml, "url2", 0));
	$record["Blog"] = array ("Blog", $xmlObj->Parse($xml, "blogurl", 0));
	$record["Biography"] = array ("Biography", $xmlObj->Parse($xml, "bio", 0));
	$record["Phonemobile"] = array ("Phone (mobile)", $xmlObj->Parse($xml, "phonemobile", 0));
	$record["Phonehome)"] = array ("Phone (home)", $xmlObj->Parse($xml, "phonehome", 0));
	$record["Phonework)"] = array ("Phone (work)", $xmlObj->Parse($xml, "phonework", 0));
	$record["PhoneThuraya)"] = array ("Phone (Thuraya)", $xmlObj->Parse($xml, "phonethuraya", 0));
	$record["PhoneInmarsat)"] = array ("Phone (Inmarsat)", $xmlObj->Parse($xml, "phoneinmarsat", 0));
	$record["PhoneIridium)"] = array ("Phone (Iridium)", $xmlObj->Parse($xml, "phoneiridium", 0));
	$record["AIM"] = array ("AIM/iChat", $xmlObj->Parse($xml, "im_aim", 0));
	$record["MSN"] = array ("MSN Messenger", $xmlObj->Parse($xml, "im_msn", 0));
	$record["Yahoo"] = array ("Yahoo! Messenger", $xmlObj->Parse($xml, "im_yahoo", 0));
	$record["Skype"] = array ("Skype", $xmlObj->Parse($xml, "im_skype", 0));
	$record["Gizmo"] = array ("Gizmo Project", $xmlObj->Parse($xml, "im_gizmo", 0));
	//$record["BloodType"] = array ("Blood Type", $xmlObj->Parse($xml, "bloodtype", 0));
	//$record["EmergencyContact"] = array ("Emergency Contact", $xmlObj->Parse($xml, "emergency", 0));
	$record["ContactLanguage"] = array ("Contact Language", $xmlObj->Parse($xml, "contactin", 0));
	$record["DRFeed"] = array ("Digital Railroad Feed", $xmlObj->Parse($xml, "drfeed1", 0));
	$record["FlickrFeed"] = array ("Flickr Feed", $xmlObj->Parse($xml, "flickrnsid", 0));
	$record["updated"] = array ("Information Last Updated", $xmlObj->Parse($xml, "updated", 0));

	$DEBUG && $msg .= ArrayToTable ($record);

	$format = FetchSnippet ("LSInfoLineItem");
	$list = "<!-- LS values -->\n";
	reset ($record);
	while (list($key, $value) = each($record)) {
		if ( $record[$key][1] ) {
			// make a formatted block line for output

			// if it's a hyperlink, wrap it with an anchor
			$v = $record[$key][1];
			if (strpos ($v, "http:") === "0") {
				$v = "<a class=\"data\" href=\"$v\" target=\"_blank\">$v</a>";
			}
			$text = str_replace('{key}', $record[$key][0], $format);
			$text = str_replace('{value}', $v, $text);
		} else {
			$text = "<!-- ".$record[$key][0]." is missing -->\n";
		}
		$list .= $text;
	}
	$list = str_replace ('{list}', $list, FetchSnippet ("LSInfoBlock"));
	return $list;
}


// GetRecord
// Taken from an earlier system, this function is just another way to
global $Themes;
// get a record, similar to the Fetch functions.
// By implementing it here, I can port code over easier from that other system.

function GetRecord( $table, $ID , $prefixtable = FALSE) {
	global $Themes;
	global $PROJECTS;
	$table || $table = $PROJECTS;
	// FetchRowsByValue ($table, $set, $keyfield, $value, $order, $returnarray = FALSE)
	$record = FetchRowsByValue ($table, '*', 'ID', $ID, "", TRUE);
	if ($record) {
		if (isset ($record['Params'])) {
			$params = DecodeArrayFromDB($record['Params']);
		} else {
			$params = array ();
			$record['Params'] = "";
		}
		$record['ParamsUnpacked'] = $params;
	}
	return $record;
}


// Strip most HTML tags and CSS tags from text.
function LimitHTLM ($value) {
	global $Themes;
	global $msg, $error;
	global $LINK;

	//$value = decode_entities(trim($value));
	$value = strip_tags($value, '<a> <b> <br> <dd> <dl> <dt> <em> <i> <li> <ol> <p> <strong> <u> <ul>');
	$value = preg_replace('/\Wstyle\s*=[^>]+?>/i', '>', $value);
	$value = preg_replace('/\Won[a-z]+\s*=[^>]+?>/i', '>', $value);
	$value = trim($value);
	//$value = htmlentities($value);
	return $value;
}


// Stuff taken from Drupal code
/**
 * Decode all HTML entities (including numerical ones) to regular UTF-8 bytes.
 * Double-escaped entities will only be decoded once ("&amp;lt;" becomes "&lt;", not "<").
 *
 * @param $text
 *	The text to decode entities in.
 * @param $exclude
 *	An array of characters which should not be decoded. For example,
 *	array('<', '&', '"'). This affects both named and numerical entities.
 */
function decode_entities($text, $exclude = array()) {
	global $Themes;
	static $table;
	// We store named entities in a table for quick processing.
	if (!isset($table)) {
		// Get all named HTML entities.
		$table = array_flip(get_html_translation_table(HTML_ENTITIES));
		// PHP gives us ISO-8859-1 data, we need UTF-8.
		$table = array_map('utf8_encode', $table);
		// Add apostrophe (XML)
		$table['&apos;'] = "'";
	}
	$newtable = array_diff($table, $exclude);

	// Use a regexp to select all entities in one pass, to avoid decoding double-escaped entities twice.
	return preg_replace('/&(#x?)?([A-Za-z0-9]+);/e', '_decode_entities("$1", "$2", "$0", $newtable, $exclude)', $text);
}

/**
 * Helper function for decode_entities
 global $Themes;
 */
function _decode_entities($prefix, $codepoint, $original, &$table, &$exclude) {
	global $Themes;
	// Named entity
	if (!$prefix) {
		if (isset($table[$original])) {
			return $table[$original];
		}
		else {
			return $original;
		}
	}
	// Hexadecimal numerical entity
	if ($prefix == '#x') {
		$codepoint = base_convert($codepoint, 16, 10);
	}
	// Encode codepoint as UTF-8 bytes
	if ($codepoint < 0x80) {
		$str = chr($codepoint);
	}
	else if ($codepoint < 0x800) {
		$str = chr(0xC0 | ($codepoint >> 6))
				. chr(0x80 | ($codepoint & 0x3F));
	}
	else if ($codepoint < 0x10000) {
		$str = chr(0xE0 | ( $codepoint >> 12))
				. chr(0x80 | (($codepoint >> 6) & 0x3F))
				. chr(0x80 | ( $codepoint		& 0x3F));
	}
	else if ($codepoint < 0x200000) {
		$str = chr(0xF0 | ( $codepoint >> 18))
				. chr(0x80 | (($codepoint >> 12) & 0x3F))
				. chr(0x80 | (($codepoint >> 6)	& 0x3F))
				. chr(0x80 | ( $codepoint		& 0x3F));
	}
	// Check for excluded characters
	if (in_array($str, $exclude)) {
		return $original;
	}
	else {
		return $str;
	}
}

/**
 * Count the amount of characters in a UTF-8 string. This is less than or
 * equal to the byte count.
 */
function string_length(&$text) {
	global $Themes;
	return strlen(preg_replace("/[\x80-\xBF]/", '', $text));
}

// Convert MYSQL date to a UNIX timestamp
function mysqldate_to_timestamp($date) {
	global $Themes;
	$hms_arr = explode("-", $date);
	$Y = $hms_arr[0] + 0;
	$M = $hms_arr[1]+ 0;
	$D = $hms_arr[2]+ 0;
	$ts = mktime(0,0,0, $M, $D, $Y);
	return $ts;
}

function timestamp_to_mysqldatetime ($datetime = null) {
	$datetime || $datetime = time ();
	return date ("Y-m-d H:i:s", $datetime);
}

// Encode GET params for use with Flash slide show. 
// It seems the Flash show won't pass more than the first param to the XML 
// generator, so I have to encode them myself all into one param. Weird, huh?
// Give this function a normal GET params, e.g. cows=nice&id=100
// It will return params=cows-nice,id-100
function EncodeParamsForFlash ($param) {
	$param = str_replace ("=","-", $param);
	$param = str_replace ("&",",", $param);
	$param = "params=$param";
	return $param;
}

function DecodeParamsForFlash ($params) {
	$params = explode(",", $params);
	foreach ($params as $p) {
		list ($k, $v) = explode("-", $p);
		$k && $record[$k] = $v;
	}
	return $record;
}

// Format text for output
// Used to show story text. Converts returns to <BR>, etc.
function FormatText ($text, $p="") {

	$text = strip_tags_except ($text, array ("b", "i", "u"));

	// fix lines & paragraphs
	$p || $p = "<p>";
	if ($p == "<p>") {
		$s = '/(.*)\n/';
		$r = $p.'$1 &nbsp;</p>'."\n";
	} else {
		$s = '/(.*)\n/';
		$r = '$1'."$p\n";
	}
	$text = preg_replace ($s,$r,$text);



	//$text = str_replace ("\r", "<BR>\r", $text);
	return $text;
}

function strip_tags_except($text, $allowed_tags, $strip=TRUE) {

	if (! is_array($allowed_tags))
		return $text;

	if (! count($allowed_tags))
		return $text;

	$open = $strip ? '' : '&lt;';
	$close = $strip ? '' : '&gt;';

	preg_match_all('!<\s*(/)?\s*([a-zA-Z]+)[^>]*>!i', $text, $all_tags);
	array_shift($all_tags);
	$slashes = $all_tags[0];
	$all_tags = $all_tags[1];
	foreach ($all_tags as $i => $tag) {
		if (in_array($tag, $allowed_tags))
			continue;
		//		$text = preg_replace('!<(\s*' . $slashes[$i] . '\s*' . $tag . '[^>]*)>!', $open . '$1' . $close, $text);
		$text = preg_replace('!<(\s*' . $slashes[$i] . '\s*' . $tag . '[^>]*)>!', "", $text);
	}

	return $text;
}

// use for array_walk
function trimMe (&$x) {
	$x = trim ($x);
}


function FormatAsCurrency (&$x) {
	if (is_float($x))
		$x = number_format ($x, 2, FP_DECIMAL, FP_THOUSANDS);
}

function AddCurrencySymbol (&$x, $showafter = true) {
	if (CURRENCY_POSITION != "after") {
		$x = CURRENCY . $x;
		$showafter && $x .= ' '.CURRENCY_AFTER;
	} else {
		$x = $x.' '.CURRENCY;
	}
	return $x;
}

/*
	NavBar
	User Level can determine which navbar the viewer sees (not access level!)
	This is not for admin menus, by the way, but gallery menus
	This way, in an FP installation we can have members who think they're alone!
	
	Admin:
	1: fp administrator as access to everything
	2: gallery/coop member has access to his galleries only
	3: solo artist does not see or access gallery - only his personal gallery
	
	Choose the right Navigation bar (menu bar) to show to viewers (not admin)
	This varies depending on the group and artist.
	- Access Level = 3 means viewers see you as in your gallery
	$currentchoice lets you highlight or dim the current menu choice (indicating where the user is)
	$whichnavbar defaults to "", which is the main navbar. It could also be "secondary" or any other suffix
	to add to the snippet name when fetching
	$solidify: if the navbar has content, then make the background of the navbar solid. Generally true on the bottom,
	but not the top.
*/
function NavBar ($groupID, $currentchoice = "", $whichnavbar = "") {
	global $error, $msg;
	global $LINK;

	$DEBUG = 0;
	$DEBUG && print __FUNCTION__.": DEBUG<BR>";

	$myGroup = new FPGroup ($LINK, $groupID );
	$currentchoice ? $ccID = "navbar_".$currentchoice : $currentchoice = "";

	//true = only find members with pictures showing
	//$solo = $myGroup->is_solo;
	$solo = $myGroup->IsSolo(true);
	$f = array ();

	$DEBUG && $msg .= __FUNCTION__.": Solo = $solo, owner={$f['artistID']}<BR>";
	$solo
		? $showingartistID = $myGroup->showingArtistID
		: $showingartistID = $myGroup->OwnerID ();

	$artist = FetchArtist ($showingartistID);

	// Show link to gallery listings?
	$artistParams = DecodeArrayFromDB ($artist['Params']);
	$linkToGalleries = GetParam ($artistParams, FP_PARAM_ARTIST_GALLERY_LINK);

	$f['NAVBAR_FAQ_DIVIDER'] = "";

	if (GetParam ($myGroup->params, FP_PARAM_GROUP_CUSTOM_MENU)) {
		// Custom Navbar (created in the admin)
		$navbar = BuildCustomNavbar($whichnavbar, $solo);
		$menuHasItems = ($navbar != "");
	} else {
		$DEBUG && $msg .= "Using built-in navbar<BR>";
		// Built-in navbar from snippets
		$divider = FetchSnippet ("navbar_divider");

		// show By Location in the menu bar?
		if (FP_NAVBAR_SHOW_LOCATION) {
			$place = $divider . FetchSnippet ("navbar_item_code_place");
		}
		else {
			$place= "";
		}
		$f['NAVBAR_BY_LOCATION'] = $place;

		// show FAQ in menubar?
		(FP_NAVBAR_FAQ == $whichnavbar) ? $faq = FetchSnippet ("navbar_item_code_faq") : $faq = "";
		$faq && $f['NAVBAR_FAQ_DIVIDER'] = $divider;
		$f['NAVBAR_CODE_FAQ'] = $faq;
		$whichnavbar && $whichnavbarChooser = "_".$whichnavbar;

		if ( $solo ) {
			$formats = FetchFormatSet ('projects_page_solo');
			$navbar = FetchSnippet ("navbar_solo$whichnavbarChooser");
			$navbarlist = FetchSnippet ("navbar_solo_list$whichnavbarChooser");
			$DEBUG && $msg .= __FUNCTION__.": Solo!<BR>";
		}
		else {
			$formats = FetchFormatSet ('projects_page');
			$navbar = FetchSnippet ("navbar$whichnavbarChooser");
			$navbarlist = FetchSnippet ("navbar_list$whichnavbarChooser");
			$DEBUG && $msg .= __FUNCTION__.": Not solo!<BR>";
		}

		$menuitems = explode(",", $navbarlist);
		$menuitemsClean = array();
		//for ($i=0;$i<count($menuitems);$i++) {
		foreach ($menuitems as $item) {
			//$item = trim($menuitems[$i]);
			$item = trim($item);
			switch ($item) {
				case "FAQ" :
					FP_NAVBAR_FAQ == $whichnavbar
							? $item = FetchSnippet ("navbar_item_code_faq")
							: $item = "";
					break;
				case "PLACE" :
					FP_NAVBAR_SHOW_LOCATION
							? $item = FetchSnippet ("navbar_item_code_place")
							: $item = "";
					break;
				case "ABOUT" :
					$solo
							? $item = FetchSnippet ("navbar_item_code_solo_about")
							: $item = FetchSnippet ("navbar_item_code_about");
					break;
				case "GALLERY_LINK" :
					$linkToGalleries
							? $item = FetchSnippet ("navbar_gallery_list_link")
							: $item = "";
					break;
				default:
					$item = FetchSnippet ($item);
					break;
			}

			$item && $menuitemsClean[] = $item;
		}
		$f['menuitems'] = join ($divider, $menuitemsClean);
		$menuHasItems = (count($menuitems) > 0);
	}
	$groupID && $f['GroupID'] = $groupID;

	// things to add into the menubar...add to array after items which might use them so they show up
	// Get artist info, esp. accesslevel
	// We called IsSolo, so now we can get the showingArtist ID.
	$solo
			? $f["artistID"] = $myGroup->showingArtistID
			: $f["artistID"] = $myGroup->OwnerID ();
	$f["ArtistFullname"] = $artist["Fullname"];
	
	// problems with the default...
	($linkToGalleries != null) || $linkToGalleries = 1;

	if ($linkToGalleries+0) {
		$f['GALLERY_LIST_LINK'] = FetchSnippet ("navbar_gallery_list_link");
	}
	else {
		$f['GALLERY_LIST_LINK'] = "";
	}

	// If the menubar is NOT empty then add background style "solid".
	/*
	if (FP_SOLIDIFY && $menuHasItems)
		{
		$f['SOLIDIFY'] = "solid";
		}
	else
		{
		$f['SOLIDIFY'] = "";
		}
	*/
	
	$f["BLOGURL"] = GetParam (unserialize($artist["Params"]), FP_PARAM_ARTIST_BLOG);
	$navbar = Substitutions ($navbar, $f);
	$navbar = str_replace ("{".$ccID."}", "selected", $navbar);
	$navbar = ReplaceAllSnippets($navbar);
	$navbar = preg_replace ("/\{navbar_selector_\w+\}/i", "unselected", $navbar);
	return $navbar;
}

// Get an array html code for each menu item.
// e.g. "new" gets an html link to projects.php
function FetchNavbarItems () {
	global $FP_NAVBAR_ITEMS;

	$items = array ();
	reset ($FP_NAVBAR_ITEMS_CODE);
	while (list($k, $v) = each ($FP_NAVBAR_ITEMS_CODE)) {
		$items[$k] = FetchSnippet ($v);
	}
	return $items;

}


// Custom Menu Bar
// IF the group's custom menu bar param is set to true, we build the navbar
// as specified. $which indicates whether we want navbar 1 or navbar 2 (top or bottom).
// If the project is solo, then the menu item, "about the artist",
// is a link to the photographer's page.
// If the project is NOT solo, this link goes to the artists' page,
// a list of all artists in the group/exhibition
// If the config parameter FP_SHOW_RETURN_TO_GROUP is set, then we force the
// 'home' entry into the menu, if it is a top or main menu.
function BuildCustomNavbar ($which = "1", $solo = false) {
	global $msg, $error;
	global $LINK;
	global $FP_NAVBAR_ITEMS_CODE, $FP_NAVBAR_ITEMS_CODE_SOLO, $FP_NAVBAR_ITEM_NAMES;

	$DEBUG = 0;
	$DEBUG && $msg .= __FUNCTION__."<BR>";

	$which || $which = 1;
	$customwrapper = FetchSnippet ("navbar_item_custom_wrapper");

	$groupID || $groupID = $_SESSION['GroupID'];
	$myGroup = new FPGroup ($LINK,  $groupID );

	// choose source array for code snippet ID references
	if ($solo) {
		$source = $FP_NAVBAR_ITEMS_CODE_SOLO;
		$titles = $FP_NAVBAR_ITEM_NAMES[1];
	} else {
		$source = $FP_NAVBAR_ITEMS_CODE;
		$titles = $FP_NAVBAR_ITEM_NAMES[0];
	}
	
	// build array of menu items
	$items = array();
	for ($i=1;$i<=8;$i++) {
		$k = GetParam ($myGroup->params, constant("FP_PARAM_GROUP_MENU_".$which."_".$i));
		if ($k)
			// CLIENT WELCOME code should not show if no user is logged in
			if ($k == "clientwelcome") {
				(isset($_SESSION['clientid']) && $_SESSION['clientid'] != "") && $items[] = $k;
			} else {
				$items[] = $k;
			}
	}

	
	// Make sure 'home' link is there if it should be
	if (FP_SHOW_RETURN_TO_GROUP && ($which == 1) && !(array_search(FP_NAVBAR_HOME_CODE, $items)))
		{
		$items[] = FP_NAVBAR_HOME_CODE;
		}


	$n =  array ();
	$i = 1;
	foreach ($items as $item) {
		$DEBUG && $msg .= __FUNCTION__.__LINE__.": Navbar $which : item $i = $item<BR>";
		// fetch custom entries
		if (strpos ($item, "custom") !== false) {
			// user-entered menu item entries
			$f = array();
			$k = str_ireplace ("custom", "", $item);
			$f["title"] = $item;
			$code = GetParam ($myGroup->params, constant ("FP_PARAM_GROUP_MENU_CUSTOM_".$k) );
			$f["code"] = $code;
			$code = Substitutions ($customwrapper, $f);
			$DEBUG && $msg .= __FUNCTION__.__LINE__.": Fetched custom menu item $item: ".htmlentities ($code)."<BR>";
		} else {
			// built-in menu item entries
			$title = $titles[$item];
			$code = FetchSnippet ($source[$item]);
			$menu = FetchNavbarMenus ($groupID, $item);
			$code = preg_replace ("/\{menu\}/i", $menu, $code);
			$code = preg_replace ("/\{menu_id\}/i", $i, $code);
			$DEBUG && $msg .= __FUNCTION__.__LINE__.": item=$item, GroupID = $groupID, Navbar title = $title, code=".htmlentities($code)."<hr>";
		}
		$code && $n[] = $code;
		$i++;
	}
	$divider = FetchSnippet ("navbar_divider");
	$f["NAVBAR"] = join ($divider, $n);
	$f["NAVBAR"]
			? $navbar = Substitutions (FetchSnippet ("navbar_custom_wrapper_$which"), $f)
			: $navbar = "";

	return $navbar;
}

// Get menus for menu items. This applies to drop-down menus, submenus.
function FetchNavbarMenus ($groupID, $itemcode) {
	global $error, $msg;
	global $PROJECTS, $PARTS;
	global $FP_NAVBAR_ITEMS_CODE;

	$groupID || $groupID = $_SESSION['GroupID'];

	switch ($itemcode) {
		case "new menu" :
		// new projects
			$result = FetchFeaturedProjects ($groupID);
			$format = FetchSnippet ('navbar_menu_project_item');
			break;
		case "all menu" :
		// new projects
			$result = FetchFeaturedActiveProjects ($groupID);
			$format = FetchSnippet ('navbar_menu_project_item');
			break;
	}

	$menu = array();
	if ($result) {
		while ($row = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
			$menu[] = Substitutions ($format, $row);
		}
	}
	$divider = FetchSnippet("navbar_menu_divider");
	$menu = implode($divider, $menu);
	$menu || $menu = "(empty)";
	return $menu;

}

// Fill out navbar names
function FetchNavbarTitles ($groupID = null) {
	global $error, $msg;
	global $FP_NAVBAR_ITEMS, $FP_NAVBAR_ITEMS_SOLO;

	$DEBUG = 0;

	$FP_NAVBAR_ITEM_NAMES = array ();
	reset ($FP_NAVBAR_ITEMS);
	while (list($k, $v) = each ($FP_NAVBAR_ITEMS)) {
		//$FP_NAVBAR_ITEM_NAMES[0][$k] = FetchSnippet ($v, $myTheme);
		$FP_NAVBAR_ITEM_NAMES[0][$k] = FetchSnippet ($v);
		$FP_NAVBAR_ITEM_NAMES[1][$k] = FetchSnippet ($FP_NAVBAR_ITEMS_SOLO[$k]);
	}
	$DEBUG && $msg .= __FUNCTION__.": MULTI:".ArrayToTable($FP_NAVBAR_ITEM_NAMES[0])."<BR>";
	$DEBUG && $msg .= __FUNCTION__.": SOLO: ".ArrayToTable($FP_NAVBAR_ITEM_NAMES[1])."<BR>";
	return $FP_NAVBAR_ITEM_NAMES;
}


// Add to the "send file" file
// The file sending daemon will pick up this file and
// FTP files away to suppliers.
function WriteImageSendOrder ($imageID, $supplierID) {
	global $msg, $error;
	global $LINK;

	$image = FetchImage ($imageID);
	$order = FetchSupplier($supplierID);
	$order['method'] = "ftp";
	WriteSendOrder ($image['URL'], $order, AS_SENDER_QUEUE, FP_ACTIVITY_LOG);
}

// Write a "mail me" in the HTML code that a spam sweeper 
// won't find. This is supposed to protect email addresses.
// "mom" is a javascript function which pieces back together the email address

function mailMe($saddress,$scaption,$stitle) {
//variables
	$eaddress= "";
	$sdomain= "";
	$aextra = "";

	//begin parsing
	list($eaddress, $sdomain)= explode('@', $saddress);
	list($sdomain, $aextra) = explode('\?', $sdomain);
	$sdomain = ereg_replace('\.', '#', $sdomain);

	//create the js address
	$smailme = "mom('".urlencode( $sdomain );
	if($aextra != "" ) {
		$smailme .= "?" . $aextra;
	}
	$smailme .= "','" . urlencode( $eaddress ) . "')";

	//build the js events
	$sbuild =" onmouseover=\"javascript:this.href=$smailme;\"";
	$sbuild.=" onfocus=\"javascript:this.href=$smailme;\"";

	//return
	return "<a href=\"/contact/\"$sbuild title=\"$stitle\">$scaption</a>";
}


// Get a user-entered paper name given the stored, internal paper code
function PaperName ($supplierID, $papercode) {
	$supplier = FetchSupplier ($supplierID);
	$papers = explode ("\n", $supplier['Papers']);
	$papercodes = explode ("\n", $supplier['PaperCodes']);
	array_walk ($papers, 'trimME');
	isset($papers[$papercode]) ? $paper = $papers[$papercode] : $paper = "PAPER NOT FOUND";
	return $paper;
}

// Get an user-entered inkset name given the stored, internal inkset code
function InkName ($supplierID, $inkcode) {
	$supplier = FetchSupplier ($supplierID);
	$inksets = explode ("\n", $supplier['Inksets']);
	$inksetcodes = explode ("\n", $supplier['InksetCodes']);
	array_walk ($inksets, 'trimME');
	isset($inksets[$inkcode]) ? $ink = $inksets[$inkcode] : $ink = "INKSET NOT FOUND";
	return $ink;
}

// Get a user-entered paper name given a user-entered paper code
function PaperNameFromCode ($supplierID, $papercode) {
	$supplier = FetchSupplier ($supplierID);
	$papers = explode ("\n", $supplier['Papers']);
	$papercodes = explode ("\n", $supplier['PaperCodes']);
	array_walk ($papers, 'trimME');
	array_walk ($papercodes, 'trimME');
	$key = array_search	($papercode, $papercodes);
	$key === false ? $paper = "PAPER NOT FOUND" : $paper = $papers[$key];
	return $paper;
}

// Get an user-entered inkset name given a user-entered inkset code
function InkNameFromCode ($supplierID, $inkcode) {
	$supplier = FetchSupplier ($supplierID);
	$inksets = explode ("\n", $supplier['Inksets']);
	$inksetcodes = explode ("\n", $supplier['InksetCodes']);
	array_walk ($inksets, 'trimME');
	array_walk ($inksetcodes, 'trimME');
	$key = array_search	($inkcode, $inksetcodes);
	$key === false ? $ink = "INKSET NOT FOUND" : $ink = $inksets[$key];
	return $ink;
}

// For optimizing pages
// Seems to work...

function css_strip_whitespace($css) {
	$replace = array(
			"#/\*(.*?)\*/#ms" => "", // Strip C style comments.
			"#\s{2,}#"	=> " " // Strip excess whitespace.
	);
	$search = array_keys($replace);
	$css = preg_replace($search, $replace, $css);

	$replace = array(
			"#: #" => ":",
			"#; #" => ";",
			"# \{#" => "{",
			"# \}#" => "}",
			"#, #" => ",",
			"#\{ #" => "{",
			"#\;\r*\}#" => "}", // Strip optional semicolons.
			"#\,\r#" => ",", // Don't wrap multiple selectors.
			"#\r\}#" => "}", // Don't wrap closing braces.
			"#\}#" => "x", // Put each rule on it's own line.
	);
	$search = array_keys($replace);
	$css = str_replace($search, $replace, $css);

	return trim($css);
}

function compress_html ($data) {

	$data=preg_replace("/ {2,}/s"," ",$data); // remove multiple spaces
	$data=preg_replace("/\t*/s","",$data); // remove tabs
	$data=preg_replace("/(\r|\n){2,}/s","\n",$data); // remove multiple new lines
	//$data=preg_replace("/\<\!\-\-(.*?)\-\-\>/s","",$data); // remove all comments <!-- -->
	$data=preg_replace('/> +</', '> <',$data);
	return $data;
}

// Given HTML, figure out which styles (class or id) are in use and
// extract them from the full style sheet
// Return a condense style sheet
// This seems to be CSS safe

function ExtractStylesInUse ($text, $fullsheet) {
	$p = preg_match_all ("/(class|id)\s*=\s*\"(.*?)\"/mis", $text, $styles);
	$sheet = "";

	foreach ($styles[2] as $name) {
		preg_match ("/(.*[\#\.]$name.*?)\s*\{/i", $fullsheet, $match);
		$names[] = $match[1];
		//print "<HR>".$match[1]."<br>\n";
	}

	$inuse = array ();
	foreach ($names as $name) {
		if (!in_array ($name, $inuse)) {
			$inuse[] = $name;
			//	print "<HR>$name<br>\n";
			$style = preg_match ("/($name.*?{.*?\})/mis", $fullsheet, $match);
			$lines[] =	$match[1];
		}
	}
	sort($lines);
	$sheet = implode ("\n", $lines );

	return $sheet;
}


function do_post_request($url, $data, $optional_headers = null) {
	if (substr($url, 0, 7) != "http://")
		$url = "http://$url";

	$params = array('http' => array(
					'method' => 'POST',
					'content' => $data
	));

	if ($optional_headers !== null) {
		$params['http']['header'] = $optional_headers;
	}
	$ctx = stream_context_create($params);
	$fp = @fopen($url, 'rb', false, $ctx);
	if (!$fp) {
		throw new Exception("Problem with $url, $php_errormsg");
	}
	$response = @stream_get_contents($fp);
	if ($response === false) {
		throw new Exception("Problem reading data from $url, $php_errormsg");
	}
	return $response;
}


// get a file name extension
function findexts ($filename) {
	$ext = pathinfo($filename, PATHINFO_EXTENSION);
	return $ext;
	/*
	$filename = strtolower($filename) ;
	$exts = explode("[/\\.]", $filename) ;
	$n = count($exts)-1;
	$exts = $exts[$n];
	return $exts;
	*/
} 


// Params functions
// I'm switching to params in a field, instead of adding new fields
// because it's easier to upate the system
// $params is the raw param field, e.g. var1=1;var2=2;var3=3


// We need to encode and decode the raw params data from the DB
// because otherwise we get big problems with " marks and such.
function EncodeArrayForDB ($params = null) {
	global $msg, $error;
	global $LINK;

	$params = serialize($params);
	return $params;
}

function DecodeArrayFromDB ($params = null) {
	global $msg, $error;
	global $LINK;

	//$params = unserialize(base64_decode ($params));
	$params && $params = unserialize($params);
	$params || $params = array ();
	return $params;
}

function SaveParams ($table, $ID, $params) {
	$pairs = array ("Params" => EncodeArrayForDB($params));
	UpdateRecord( $table, $ID, $pairs );
}

function FetchParams ($table, $ID) {
	global $msg, $error;
	global $LINK;

	$r = FetchRowsByValue ($table, "Params", "ID", $ID, "", TRUE);
	$r['Params'] ? $params = DecodeArrayFromDB($r['Params']) : $params = "";

	return $params;
}

function GetParam ($params, $key ) {
	$p = $params;
	isset($p[$key]) ? $v = $p[$key] : $v = null;
	return $v;
}

function SetParam ($params, $key, $value) {
	is_array ($params) || $params = array ();
	$params[$key] = trim($value);
	return $params;
}

// Any vars whose name starts "param_" are put into a serialized params array
// If there is a "Params" entry and it is an array, then we use it and ignore any other params values
// which are stored in $vars, e.g. as $vars[param_1]. 
// Call MergeParams first to get all such $vars[param_x] into the $params array.
// If there are param variables, then the variable "Params" is created (any existing one is overwritten)
// If there are no param variables, nothing is done (existing params variable is kept)
function PackageParams ($vars) {
	global $msg, $error;
	global $LINK;

	if (isset ($vars['Params']) && is_array($vars['Params'])) {
		$params = $vars['Params'];
	} else {
		$params = array ();
	}

	reset ($vars);
	while (list ($k, $v) = each ($vars)) {
		if (substr($k,0,6) == "param_" && strlen($k) == 8) {
			$pk = substr($k, 6,2);
			$params[$pk] = $v;
			unset ($vars[$k]);
		}
	}
	$params && $vars['Params'] = $params;
	return $vars;
}

// Store any "param_" variables in $vars into the $params array, 
// delete the "param_" variable from $vars. 
// Overwrite existing params values from $vars.
function MergeParams ($params, $vars) {
	global $msg, $error;
	global $LINK;
	$DEBUG = 0;
	is_array($params) || $params = array ();
	$DEBUG && $error .= __FUNCTION__.__LINE__;
	reset ($vars);
	while (list ($k, $v) = each ($vars)) {
		$DEBUG && $error .= __FUNCTION__.__LINE__.": $k, $v<BR>";
		if (substr($k,0,6) == "param_" && strlen($k) == 8) {
			$pk = substr($k, 6,2);
			$params = SetParam ($params, $pk, $v);
			$DEBUG && $error .= __FUNCTION__.__LINE__.": $pk : $v<BR>";
		}
	}
	return $params;
}

function FixCheckBoxParams ($params, $vars) {
	global $Themes;
	global $msg, $error;
	global $LINK;

	//$plist = array_keys ($params);
	reset ($vars);
	while (list ($k, $v) = each ($vars)) {
		if (substr($k,0,6) == "param_" && preg_match ('/_CHECKBOX^/', $k)) {
			$pk = substr($k, 6,2);
			if (array_key_exists ($pk, $params)) {
				$params[$pk] = $v;
				unset ($vars[$k]);
			}
		}
	}
	return $pairs;
}

// Get default settings from config.inc
// Used to fill out params for a new record
function GetDefaultParams ($table) {
	global $FP_PARAMS_DEFAULTS;
	global $msg, $error;
	global $LINK;

	$params = null;
	if (isset($FP_PARAMS_DEFAULTS[$table])) {
		$params = array ();
		reset ($FP_PARAMS_DEFAULTS[$table]);
		while (list ($k,$v) = each ($FP_PARAMS_DEFAULTS[$table])) {
			$v && $params[$k] = $v;
		}
	}
	return $params;
}


// =============================================================================

/*	Get the current theme.
	Set the session theme (and return the theme id) based on the current GroupID.	
*/ 
function CurrentThemeID () {
	global $msg, $error;
	global $LINK;
	global $Themes;

	$DEBUG = 0;
	$DEBUG && $error = __FUNCTION__.__LINE__.": <BR>";

	if (isset($_SESSION['projectid'])) {
		$project = new FPProject ($LINK, $_SESSION['projectid']);
		$theme = $project->GetProjectThemeID();
		$_SESSION['theme'] = $theme;
		$DEBUG && $error .= __FUNCTION__.__LINE__."project={$_SESSION['projectid']}, theme={$_SESSION['theme']}<BR>";
	} elseif (isset($_SESSION['GroupID'])) {
		if ($_SESSION['GroupID'] == FP_ADMINISTRATOR_GROUP) {
			$_SESSION['theme'] = ADMIN_THEME;
			$DEBUG && $error .= __FUNCTION__.__LINE__."group={$_SESSION['GroupID']}, theme={$_SESSION['theme']}<BR>";
		} else {
			$myGroup = new FPGroup ($LINK, $_SESSION['GroupID']);
			$_SESSION['theme'] = $myGroup->theme;
			$DEBUG && $error .= __FUNCTION__.__LINE__."group={$_SESSION['GroupID']}, theme={$_SESSION['theme']}<BR>";
		}
	} else {
		$_SESSION['GroupID'] = PUBLIC_GROUP_ID;
		$myGroup = new FPGroup ($LINK, $_SESSION['GroupID']);
		$_SESSION['theme'] = $myGroup->theme;
		$DEBUG && $error .= __FUNCTION__.__LINE__."theme={$_SESSION['theme']}<BR>";
	}
	return	$_SESSION['theme'];
}

// Return paths to theme directories inside the theme directories
// array = ("theme"=>themepath, "css"=>csspath, "js"=>jspath)
// If the theme doesn't have a directory for something, we use the default directories
// If the theme is a 'variation', then CSS will point to that variation css path.
function CurrentThemeDir ($themeID = null) {
	global $Themes;
	global $error, $msg;


	$DEBUG = 0;

	$themeID || $themeID = CurrentThemeID ();
	$myTheme = $Themes->FP_Themes[$themeID];
	$DEBUG && $error .= __FUNCTION__.": theme id=" . $themeID ."<BR>";

	// *** Here's where variations come into play ***
	// If this theme is, in fact, a variation, then we get a third CSS from the variation
	// After that, we switch over to the main theme
	if ($myTheme['is_variation']) {
		$altpath = $myTheme['path'];
		// Get main theme
		$myTheme = $Themes->FP_Themes[$myTheme['theme_id']];

		$myPath = $myTheme['path'];
		file_exists (FP_THEMES_DIR."/".$myPath) || $myPath = DEFAULT_THEME;

		if (file_exists (FP_THEMES_DIR."/".$myPath."/".FP_THEME_VARIATION_DIR."/".$altpath)) {
			$csspath3 = FP_THEMES_DIR."/".$myPath."/".FP_THEME_VARIATION_DIR."/".$altpath;
		} else {
			$csspath3 = null;
		}

	} else {
		if ($myTheme['path'] && file_exists (FP_THEMES_DIR."/".$myTheme['path'])) {
			$myPath = $myTheme['path'];
		} else {
			$myPath = DEFAULT_THEME;
		}
	}

	// get the javascript path
	// The theme may borrow the default one
	// (For variations, this comes from the master theme)
	if (file_exists (FP_THEMES_DIR."/".$myPath."/".FP_JSDIR)) {
		$jspath = FP_THEMES_DIR."/".$myPath."/".FP_JSDIR;
	} else {
		$jspath = FP_THEMES_DIR."/".DEFAULT_THEME."/".FP_JSDIR;
	}

	// get the main javascript path/name (the fp_javascript.js file)
	// (For variations, this comes from the master theme)
	if (file_exists ($jspath ."/".FP_THEME_JS_NAME)) {
		$jsfile = $jspath ."/".FP_THEME_JS_NAME;
	} else {
		$jsfile = $jspath ."/".FP_THEME_JS_NAME;
	}



	// Get the CSS path/name
	// Main CSS comes from the theme (also for variations)
	if (file_exists (FP_THEMES_DIR."/".$myPath."/".FP_CSSDIR."/".FP_THEME_CSS_NAME)) {
		$csspath = FP_THEMES_DIR."/".$myPath."/".FP_CSSDIR."/".FP_THEME_CSS_NAME;
	} else {
		$csspath = FP_THEMES_DIR."/".DEFAULT_THEME."/".FP_CSSDIR."/".FP_THEME_CSS_NAME;
	}

	// Extra CSS path
	if (file_exists (FP_THEMES_DIR."/".$myPath."/".FP_CSSDIR."/".FP_THEME_EXTRA_CSS_NAME)) {
		$csspath2 = FP_THEMES_DIR."/".$myPath."/".FP_CSSDIR."/".FP_THEME_EXTRA_CSS_NAME;
	} else {
		//$csspath2 = FP_THEMES_DIR."/".DEFAULT_THEME."/".FP_CSSDIR."/".FP_THEME_EXTRA_CSS_NAME;
		$csspath2 = "";
	}

	// NOT IN USE! BETTER TO FLAG DIFFERENCES WITH CSS STYLING
	// Gallery additional modification CSS path
	if (file_exists (FP_THEMES_DIR."/".$myPath."/".FP_CSSDIR."/".FP_THEME_GALLERY_CSS_NAME)) {
		$csspath4 = FP_THEMES_DIR."/".$myPath."/".FP_CSSDIR."/".FP_THEME_GALLERY_CSS_NAME;
	} else {
		//$csspath4 = FP_THEMES_DIR."/".DEFAULT_THEME."/".FP_CSSDIR."/".FP_THEME_GALLERY_CSS_NAME;
		$csspath4 = "";
	}

	// Newsletter additional modification CSS path
	//	if (file_exists (FP_THEMES_DIR."/".$myPath."/".FP_CSSDIR."/".FP_THEME_NEWSLETTER_CSS_NAME)) {
	//		$csspath5 = FP_THEMES_DIR."/".$myPath."/".FP_CSSDIR."/".FP_THEME_NEWSLETTER_CSS_NAME;
	//	} else {
	//		$csspath5 = FP_THEMES_DIR."/".DEFAULT_THEME."/".FP_CSSDIR."/".FP_THEME_NEWSLETTER_CSS_NAME;
	//	}

	$DEBUG && $msg .= __FUNCTION__.": Theme Style: CSS=$csspath<BR>";
	$DEBUG && $msg .= __FUNCTION__.": Extra Styling: extraCSS=$csspath2<BR>";
	$DEBUG && $msg .= __FUNCTION__.": Variation: variationCSS=$csspath3<BR>";
	$DEBUG && $msg .= __FUNCTION__.": Gallery Style: CSS4=$csspath4<BR>";
	//	$DEBUG && $msg .= __FUNCTION__.": CSS5=$csspath5<BR>";
	$DEBUG && $msg .= __FUNCTION__.": JS=$jspath<BR>";
	$DEBUG && $msg .= __FUNCTION__.": theme=$myPath<hr><BR>";


	$result = array (
			"css"			=> $csspath,
			"extraCSS"		=> $csspath2,
			"variationCSS"		=> $csspath3,
			"gallery"		=> $csspath4,
//			"newsletter"		=> $csspath5,
			"alt"			=> $variationpath,
			"jspath"		=> $jspath,
			"jsfile"		=> $jsfile,
			"theme"			=> $myPath);
	return $result;
}


// Given stylesheet name(s), return HTML to call it
function StyleSheetHTML ($sheetnames = "") {
	global $SERVERNAME, $SYSTEMURL;
	if (!$sheetnames)
		return "";

	//$p = dirname ($_SERVER['PHP_SELF']);

	is_array ($sheetnames) || $sheetnames = array ($sheetnames);
	$output = "";
	foreach ($sheetnames as $s) {
		$output .= "<link rel=\"Stylesheet\" rev=\"Stylesheet\" href=\"$s\" type=\"text/css\">\n";
	}
	return $output;
}

// For some reason, the built-in mysql_list_fields is deprecated.
function MysqlListFields ($table) {
	global $LINK;
	$res = mysqli_query ($LINK, "SHOW COLUMNS FROM `$table`");
	while ($row = mysqli_fetch_array($res))
		$col_names[]=$row[0];
	return $col_names;
}

function FormatErrorForOutput ($myerror = null) {
	global $error;

	$myerror || $myerror = $error;
	$myerror && $myerror = Substitutions (FetchSnippet ("error_wrapper"), array ("error"=>$myerror));
	return $myerror;
}

function FormatMessageForOutput ($mymsg = null) {
	global $msg;

	$mymsg || $mymsg = $msg;
	$mymsg && $mymsg = Substitutions (FetchSnippet ("msg_wrapper"), array ("msg"=>$mymsg));
	return $mymsg;
}

function insertBGANCost ($text = "") {
	$pagecount = round (strlen ($text) / 1000);
	$bgancost = sprintf("%01.2f", round (12 * ($pagecount / 1000), 2));
	$_SESSION['RunningCostBGAN'] += $bgancost;
	$bgantotal = sprintf("%01.2f", round (12 * ($_SESSION['RunningCostBGAN'] / 1000), 2));
	$text = str_replace ('{PageCharacterCount}', $pagecount, $text);
	$text = str_replace ('{BGANCost}', $bgancost, $text);
	$text = str_replace ('{BGANTotal}', $_SESSION['RunningCostBGAN'], $text);
	return $text;
}

// function SnippetToArray
// Ignore lines beginning with // or #
// Each line is split on the first comma
// IF there's only the key on the line, i.e., no comma followed by another value
// then the value is set to the key. So, in a vocab list, list listing "myword" means
// myword is replaced with myword, so no damage done.
function SnippetToArray ($snippet, $sep = ",", $themeID = null) {
	global $error, $msg;
	$farr = explode("\n", FetchSnippet ($snippet, $themeID));
	$arr = array ();
	foreach ($farr as $row) {
		$row = trim ($row);
		if ($row && substr($row,0,2) != "//" && substr($row,0,2) != "#" ) {
			preg_match ("/(.*?),(.*)/", $row, $matches);
			$k = trim($matches[1]);
			$v = trim($matches[2]);
			$v || $v = $k;
			$arr[$k] = $v;
		}
	}
	return $arr;
}

// Read a text file into an array
// File consists of lines with two items divided by something
function FileToArray ($f, $sep = ",") {
	$farr = File ($f, FILE_IGNORE_NEW_LINES|FILE_SKIP_EMPTY_LINES);
	$arr = array ();
	foreach ($arr as $row) {
		list ($k,$v) = explode($sep, $row);
		$arr[$k] = $v;
	}
	return $arr;
}

// Replace marked words in a text with their replacements
// from the "vocabulary" file in snippets. Start with current
// theme snippets, then do the default theme snippets last.
// Non-English language vocabularies begin with a code and underscore, e.g. "vocabulary_fr" for French
// The User Vocabulary file, "vocabulary.txt" in the _user directory, is applied before any other
function ReplaceVocabulary ($text) {
	global $msg, $error;
	global $LINK;
	// Replace vocabulary from the "vocabulary" snippet
	// Key words are marked {FP:word}

	$DEBUG = 0;
	$DEBUG && print __FUNCTION__.": TEXT = $text<BR>";

	isset($_SESSION['theme']) || $_SESSION['theme'] = DEFAULT_THEME;
	$myThemeID = $_SESSION['theme'];
	$myThemeID || $myThemeID = DEFAULT_THEME;

	$lists = array ();
	$lists[] = SnippetToArray ("vocabulary_user", ",", DEFAULT_THEME);

	$myThemeID != DEFAULT_THEME && $lists[] = SnippetToArray ("vocabulary", ",", $myThemeID);

	(FP_LANGUAGE == "en" or FP_LANGUAGE == "") ? $lang = "" : $lang = "_" .FP_LANGUAGE ;
	$lists[] = SnippetToArray ("vocabulary{$lang}", ",", DEFAULT_THEME);

	$DEBUG && $error .= __FUNCTION__.":theme=$myThemeID<BR>";

	foreach ($lists as $vocab) {
		$DEBUG && $error .= __FUNCTION__.": ".ArrayToTable ($vocab);
		reset ($vocab);
		while (list($k,$v) = each ($vocab)) {
			// if foreign language, use unicode, so no recursive {fp:xx} allowed!
			$lang && $v = separator ($v);
			// replace case-sensitive first
			$text = preg_replace ("/\{(?:(FP|fp|Fp|fP)):\s*$k\s*\}/", $v, $text);
			// then, replace case-insensitive to catch possible mistakes
			$text = preg_replace ("/\{fp:\s*$k\s*\}/i", $v, $text);
			$DEBUG && PRINT __FUNCTION__.": Replaced $k with $v<BR>";
		}
		// anything left over, leave as is but w/o the markers
	}
	$text = preg_replace ("/\{FP:\s*(.*?)\s*\}/i", '\1', $text);
	$DEBUG && $msg .= __FUNCTION__.": <hr><BR>";
	$DEBUG && print "$text<hr><BR>";

	return $text;
}


function DirectorySize ($dir) {
	$bytes = array_sum(array_map('filesize',glob("$dir/*")));
	return $bytes;
}

// given an integer of minutes,
// return a string hh:mm
// (int) is type-casting
function MinutesToTime ($min) {
	$min >= 0 ? $sign = "" : $sign = "-";
	$days = $min;
	$min = $min % (24 * 60);
	$hours = $sign . sprintf("%02d:%02d:%02d", abs((int)$days/1440),  abs((int)($min%1440)/60), abs((int)$min%60));
	return $hours;
}

// END OF FILE


// Strip symbols from text.
function strip_symbols( $text ) {
	$plus	= '\+\x{FE62}\x{FF0B}\x{208A}\x{207A}';
	$minus	= '\x{2012}\x{208B}\x{207B}';

	$units	= '\\x{00B0}\x{2103}\x{2109}\\x{23CD}';
	$units .= '\\x{32CC}-\\x{32CE}';
	$units .= '\\x{3300}-\\x{3357}';
	$units .= '\\x{3371}-\\x{33DF}';
	$units .= '\\x{33FF}';

	$ideo	= '\\x{2E80}-\\x{2EF3}';
	$ideo  .= '\\x{2F00}-\\x{2FD5}';
	$ideo  .= '\\x{2FF0}-\\x{2FFB}';
	$ideo  .= '\\x{3037}-\\x{303F}';
	$ideo  .= '\\x{3190}-\\x{319F}';
	$ideo  .= '\\x{31C0}-\\x{31CF}';
	$ideo  .= '\\x{32C0}-\\x{32CB}';
	$ideo  .= '\\x{3358}-\\x{3370}';
	$ideo  .= '\\x{33E0}-\\x{33FE}';
	$ideo  .= '\\x{A490}-\\x{A4C6}';

	return preg_replace(
			array(
			// Remove modifier and private use symbols.
			'/[\p{Sk}\p{Co}]/u',
			// Remove mathematics symbols except + - = ~ and fraction slash
			'/\p{Sm}(?<![' . $plus . $minus . '=~\x{2044}])/u',
			// Remove + - if space before, no number or currency after
			'/((?<= )|^)[' . $plus . $minus . ']+((?![\p{N}\p{Sc}])|$)/u',
			// Remove = if space before
			'/((?<= )|^)=+/u',
			// Remove + - = ~ if space after
			'/[' . $plus . $minus . '=~]+((?= )|$)/u',
			// Remove other symbols except units and ideograph parts
			'/\p{So}(?<![' . $units . $ideo . '])/u',
			// Remove consecutive white space
			'/ +/',
			),
			' ',
			$text );
}

// convert an $ in a string to html dollar signs (&#36;), to prevent variable substitions in PHP
function convertDollarToHTML ($text) {
	$text = str_replace('$', '&#36;', $text);
	return $text;
}

// Generate a secret code, a kind of primitive public key
// Let's mix together the ID with the secret code in a way that ordinary mortals 
// won't suspect. Secret encodes current date.
function Generate_PP_Secret ($artistID) {
	$secret = SHA1($artistID) . PP_SECRET .	SHA1(date("dmy"),$t);
	//	print "secret=$secret<br>";
	return $secret;
}

function Verify_PP_Secret ($secret, $num) {
	$numcrypt = substr ($secret, 0, strpos ($secret, PP_SECRET));
	$datecrypt = substr ($secret, strlen (PP_SECRET) + strlen ($numcrypt));
	if ($numcrypt == sha1($num) && $datecrypt == sha1(date("dmy")))
		return true;
	return false;
}


// Thanks to Scott Reynen
// http://randomchaos.com/documents/?source=php_and_unicode
function utf8_to_unicode( $str ) {
	$unicode = array();
	$values = array();
	$lookingFor = 1;
	for ($i = 0; $i < strlen( $str ); $i++ ) {
		$thisValue = ord( $str[ $i ] );
		if ( $thisValue < 128 ) $unicode[] = $thisValue;
		else {
			if ( count( $values ) == 0 ) $lookingFor = ( $thisValue < 224 ) ? 2 : 3;
			$values[] = $thisValue;
			if ( count( $values ) == $lookingFor ) {
				$number = ( $lookingFor == 3 ) ?
						( ( $values[0] % 16 ) * 4096 ) + ( ( $values[1] % 64 ) * 64 ) + ( $values[2] % 64 ):
						( ( $values[0] % 32 ) * 64 ) + ( $values[1] % 64 );
				$unicode[] = $number;
				$values = array();
				$lookingFor = 1;
			} // if
		} // if
	} // for
	return $unicode;
} // utf8_to_unicode

function unicode_to_entities( $unicode ) {
	$entities = '';
	foreach( $unicode as $value )
		$entities .= '&#' . $value . ';';
	return $entities;
} // unicode_to_entities

function unicode_to_utf8( $str ) {
	$utf8 = '';

	//foreach( $str as $unicode ) {
	for ($i = 0; $i < strlen( $str ); $i++ ) {
		$unicode = ord( $str[ $i ] );
		if ( $unicode < 128 ) {
			$utf8.= chr( $unicode );
		} elseif ( $unicode < 2048 ) {
			$utf8.= chr( 192 +	( ( $unicode - ( $unicode % 64 ) ) / 64 ) );
			$utf8.= chr( 128 + ( $unicode % 64 ) );
		} else {
			$utf8.= chr( 224 + ( ( $unicode - ( $unicode % 4096 ) ) / 4096 ) );
			$utf8.= chr( 128 + ( ( ( $unicode % 4096 ) - ( $unicode % 64 ) ) / 64 ) );
			$utf8.= chr( 128 + ( $unicode % 64 ) );
		} // if
	} // foreach
	return $utf8;
} // unicode_to_utf8

function _utf8_to_html ($data) {
	$ret = 0;
if (gettype($data) != "string") {
	die (__FUNCTION__.":".__LINE__.": Data is not string!" . print_r($data,true));
}

	foreach((str_split(strrev(chr((ord($data{0}) % 252 % 248 % 240 % 224 % 192) + 128) . substr($data, 1)))) as $k => $v)
		$ret += (ord($v) % 128) * pow(64, $k);
	return "&#$ret;";
}

// =====
// MakeDirectoryTree ($directory)
// If directories for a file don't exist already, create them
// so we can write the file
function MakeDirectoryTree ($path) {
	global $BASEDIR;
	global $error, $msg;

	$DEBUG = 1;

	rtrim ($path, '/');
	$paths = explode("/", $path);
	$myPath = "";
	foreach ($paths as $p) {
		$myPath .= "$p/";
		if (!file_exists ($myPath)) {
			mkdir ($myPath, 0755);
			//mkdir ("$BASEDIR/$myPath", 0755);
		}
	}
}


/*
Generate a META noindex code, or META search terms, for Robot searches.
Param1 = table
Param2 = record id (optional)
Default is NOINDEX!
*/
function MetaIndexCode ($table, $id = null) {
	global $LINK;
	
	switch ($table) {
		case DB_GROUPS :
			if ($id) {
				$myGroup = new FPGroup ($LINK, $id );
				$desc = $myGroup->description;
			}
			if (!$id || !$desc) {
				$desc = "{$myGroup->title} is an online gallery of photography and art.";
			}
			$desc = preg_replace ("/<.*?>/","",$desc);
			$metacode = "<META NAME=\"Description\" CONTENT=\"{$desc}\">";
			break;
		case DB_PROJECTS :
			if ($id) {
				$myProject = FetchProject ( $id );
				$desc = $myProject['Description'];
				$desc .= $myProject['Statement'];
			}
			if (!$id || !$desc) {
				$desc = "{$myProject['Title']} is an online exhibition of photography and art.";
			}
			$desc = preg_replace ("/<.*?>/","",$desc);
			$metacode = "<META NAME=\"Description\" CONTENT=\"{$desc}\">";
			break;
		case DB_ARTISTS :
			if ($id) {
				$myArtist = FetchArtist ( $id );
				$desc = $myArtist['Biography'];
			}
			if (!$id || !$desc) {
				$desc = "{$myArtist['Fullname']} is a photographer and artist exhibiting on the gallery website " . FP_SYSTEM_DISPLAY_NAME . ".";
			}
			$desc = preg_replace ("/<.*?>/","",$desc);
			$metacode = "<META NAME=\"Description\" CONTENT=\"{$desc}\">";
			break;
		default:
			$metacode = FetchSnippet ('meta_robots_noindex');
	}

	// Strip out any user codes!
	$metacode = preg_replace("/".preg_quote(FP_INSERT_USERFILE_CODE_OPEN)."(.*?)".preg_quote(FP_INSERT_USERFILE_CODE_CLOSE)."/", "", $metacode);

	return $metacode;
}


// Get the year from an Image date, which is in mysql date format
function YearOf ($date) {
	$hms_arr = explode("-", $date);
	$Y = $hms_arr[0] + 0;
	$Y || $Y = "";
	return $Y;
}

// Get the year from an Image date, which is in mysql date format
function MonthOf ($date) {
	$hms_arr = explode("-", $date);
	$M = $hms_arr[1]+ 0;
	$M || $M = "";
	return $M;
}


/*
	Choose which is the user who accepts anything that is uploaded by the Admin user,
	or whose ownership cannot be determined. For a single-user system, this the other user
	besides the admin user. In a multiuser system, this is the first user besides the admin user.
*/
function GetMainUserID ($artistID = null, $projectID = null) {
	global $LINK;
	
	if (!$artistID || $artistID == FP_ADMINISTRATOR) {
		if ($projectID) {
			$project = FetchProject ($projectID);
			$artistID = $project['ArtistID'];
		} else {
			$query = "SELECT MIN(ID) AS ID from " . DB_ARTISTS . " WHERE ID != " . FP_ADMINISTRATOR . " LIMIT 1";
			$result = mysqli_fetch_array(mysqli_query ($LINK, $query), MYSQLI_ASSOC);
			$artistID = $result['ID'];
		}
	}
	return $artistID;
}



// Use this to ALSO get the calculated fields from Projects
// This way, you don't have to write this crap every time
// you need it. Many functions are generalized, so 
// they don't add this is in for Projects.
// Normally, you'd send this '*' for all fields, as the $field parameter.
function ProjectsCalcFields ($fields) {
	$fields .= ', (TO_DAYS(NOW()) - TO_DAYS(LastUpdate)) AS Age';
	$fields .= ', if (ActiveLifeSpan = 0, 1, ((TO_DAYS(NOW()) - TO_DAYS(IF(LastUpdate, LastUpdate, ProjectDate))) <= IF(ActiveLifeSpan, ActiveLifeSpan + LifeSpan, LifeSpan))) AS Active';
	$fields .= ', if (LifeSpan = 0, 1, ((TO_DAYS(NOW()) - TO_DAYS(LastUpdate)) <= Lifespan)) AS Featured';
	return $fields;
}

// Use this to set WHERE to choose Active AND Featured projects, based on project age
function GetActiveWhere ($where = '') {
	$where && ($where .= " AND");
	$where .= ' (((TO_DAYS(NOW()) - TO_DAYS(IF(Projects.LastUpdate, Projects.LastUpdate, Projects.ProjectDate))) <= IF(Projects.ActiveLifeSpan, Projects.ActiveLifeSpan + Projects.Lifespan, Projects.LifeSpan))';
	$where .= ' OR (Projects.ActiveLifeSpan = 0))';
	return $where;
}

// Use this to set	WHERE to choose Featured projects but not Active projects, based on project age
function GetFeaturedWhere ($where = '') {
	$where && ($where .= " AND");
	$where .= ' (((TO_DAYS(NOW()) - TO_DAYS(Projects.LastUpdate)) <= Projects.Lifespan)';
	$where .= ' OR (Projects.Lifespan = 0)) ';
	return $where;
}

// Return Projects which are changed (Featured), calculated based on last Timestamp, not "LastUpdate" field.
// The difference is, last update only measures days. So, a change this morning will be unnoticed if the RSS
// system marks today as unchanged.
function GetFeaturedPreciseWhere ($where = '') {
	$where && ($where .= " AND ");
	$where .= '(((TO_DAYS(NOW()) - TO_DAYS(Projects.LastUpdate)) <= Projects.Lifespan) ';
	$where .= 'OR (Projects.Lifespan = 0)) ';
	$where .= 'OR (Projects.Timestamp > (NOW()+'.FP_PROJECT_UPDATED_INTERVAL.') ) ';
	return $where;
}



// Clear the server cache files for a project.
// If we're certain only the project pictures have changed
// this is useful.
function ClearProjectCache ($projectID = null) {
	if ($projectID) {
		require_once('Cache/Lite.php');
		// Get the id for this cache
		preg_match("|^/\w+/(.+?)/|",__FILE__, $m);
		$username = $m[1];
		$cachegroup = "project{$username}";
		$cacheid = "project".$projectID;

		// Set a few options
		$options = array(
				'cacheDir' => 'tmp/cache/'
		);
		$Cache_Lite = new Cache_Lite($options);
		$Cache_Lite->remove($cacheid, $cachegroup);
		// fp_error_log("Cleared the cache file for project #{$projectID} (called by ".getCallingFunction().").", 3, FP_PICTURES_LOG);
	}
}

// Clear the server cache files for a story.
// If we're certain only the project pictures have changed
// this is useful.
function ClearStoryCache ($storyID = null) {
	if ($storyID) {
		require_once('Cache/Lite.php');
		// Get the id for this cache
		preg_match("|^/\w+/(.+?)/|",__FILE__, $m);
		$username = $m[1];
		$cachegroup = "story{$username}";
		$cacheid = "story".$storyID;

		// Set a few options
		$options = array(
				'cacheDir' => 'tmp/cache/'
		);
		$Cache_Lite = new Cache_Lite($options);
		$Cache_Lite->remove($cacheid, $cachegroup);
		fp_error_log("Cleared the cache file for story #{$storyID} (called by ".getCallingFunction().").", 3, FP_PICTURES_LOG);
	}
}

// Clear all server cache files: we don't know which projects or pages might be affected,
// so clear all!
function ClearAllCache () {
	if ($projectID) {
		require_once('Cache/Lite.php');
		// Get the id for this cache
		preg_match("|^/\w+/(.+?)/|",__FILE__, $m);
		$username = $m[1];
		$cachegroup = "{$username}";
		$cacheid = "project".$projectID;
		// Set a few options
		$options = array(
				'cacheDir' => 'tmp/cache/'
		);
		$Cache_Lite = new Cache_Lite($options);
		$Cache_Lite->remove($cacheid, $cachegroup);
		fp_error_log("Cleared the cache file for project #{$projectID} (called by ".getCallingFunction().").", 3, FP_ACTIVITY_LOG);
	}
}

// Return name of calling function.
// Use this to find out which function invoked the current function
function getCallingFunction() {
	$backtrace = debug_backtrace();
	return $backtrace[2]['function'];
}

// My error logging...just add a newline after each line. Dammit.
function fp_error_log ($text, $type, $logfile) {
	error_log (date('Y-m-d H:i:s') . ": {$text}\n", $type, $logfile);
}

// Get a user directory for a given user inside the users directory (_user).
// $relative flag means don't add the root directory, so we can use the user dir for an HTTP reference.
function GetUserDirectory ($artistID = null, $relative = false) {
	if ($artistID && $artistID != FP_ADMINISTRATOR) {
		$user_sub_path = "/user_". $artistID ;
	} else {
		$user_sub_path = "";
	}
	$userDir = FP_DIR_USER.$user_sub_path."/";
	$relative || $userDir = FP_BASEDIR."/".$userDir;
	file_exists($userDir) || MakeDirectoryTree ($userDir);
	return $userDir;
}

// Get a list of audio files from the user's upload directory
// $type: array of extensions, w/o ".", e.g. ['mp3', 'wav']
// $type defaults to all files
function FetchUserFileList ($type = array(), $artistID = null) {
	global $msg, $error;
	global $LINK;
	global $USER_AUDIO_EXTENSIONS, $USER_VIDEO_EXTENSIONS, $USER_IMAGE_EXTENSIONS;

	$artistID || $artistID = FP_ADMINISTRATOR;
	$userDir = GetUserDirectory ($artistID);
	$list = array();
	$type || $type = array ("");
	$type = "*.{" . join(",", $type) . "}";
	$list = $list + glob ($userDir.FP_SUBDIR_USER_FILES."/{$type}", GLOB_BRACE);
	if ($list) {
		$list = array_map ("basename", $list);
		$list = array_combine ($list, $list);
	} else {
		$list = array();
	}
	return $list;
}


function rglob($pattern, $flags = 0, $path = '') {
	if (!$path && ($dir = dirname($pattern)) != '.') {
		if ($dir == '\\' || $dir == '/') $dir = '';
		return rglob(basename($pattern), $flags, $dir . '/');
	}
	$paths = glob($path . '*', GLOB_ONLYDIR | GLOB_NOSORT);
	$files = glob($path . $pattern, $flags);
	foreach ($paths as $p) $files = array_merge($files, rglob($pattern, $flags, $p . '/'));
	return $files;
}

function UniqueFilename ($dir, $filename) {
	global $msg, $error;
	global $LINK;

	$dir = rtrim ($dir, "/");
	$newfilename = $filename;
	$k = 1;
	while ($newfilename != "" and file_exists ("$dir/$newfilename")) {
		$newfilename = preg_replace ("/(\..*?)$/", "-$k$1", $filename);
		$k++;
		$k > 999 && exit; // just a fail-safe... good assumption won't be 999 dups
	}
	if ($filename != $newfilename) {
		$msg .= __FUNCTION__.__LINE__.": Note: Renamed $filename to $newfilename<BR>";
	}
	return $newfilename;
}

/*
	ArrayToJavascriptVars
	Write an array of values as Javascript vars.
*/
function ArrayToJavascriptVars($arr, $prefix = "FPVARS_") {
	$script = "<script type=\"text/javascript\" language=\"javascript\">\n";
	$row = "";
	reset ($vars);
	while (list($k,$v) = each ($vars)) {
		$row .= "$k = $v;\n";
	}
	$script .= $row . "\n</script>\n";
	return $script;
}


// Unpack text fields into arrays for a price set record.
// Look for columns prefixed with "a_". These are arrays.
// If the index $i is set, then get the values only for that index
function UnpackPriceSetRow($record, $i=null) {
	if ($record) {
		reset ($record);
		while (list($k,$v) = each ($record)) {
			if (strtolower(substr($k,0,2)) == "a_") {
				$newK = substr($k,2,strlen($k)-2);
				//print __FUNCTION__." : $k => $newK<BR>";
				$record[$newK] = explode(',', $record[$k]);
				isset($i) && $record[$newK]=$record[$newK][$i];
			}
		}
	}

	// Convert strings to arrays
	/*
	$record['Size'] = explode(',', $record['Size']);
	isset($i) && $record['Size']=$record['Size'][$i];
	$record['EditionSize'] = explode(',', $record['EditionSize']);
	isset($i) && $record['EditionSize']=$record['EditionSize'][$i];
	$record['PrintCost'] = explode(',', $record['PrintCost']);
	isset($i) && $record['PrintCost']=$record['PrintCost'][$i];
	$record['MatteCost'] = explode(',', $record['MatteCost']);
	isset($i) && $record['MatteCost']=$record['MatteCost'][$i];
	$record['FrameToPrintCost'] = explode(',', $record['FrameToPrintCost']);
	isset($i) && $record['FrameToPrintCost']=$record['FrameToPrintCost'][$i];
	$record['FrameMatteCost'] = explode(',', $record['FrameMatteCost']);
	isset($i) && $record['FrameMatteCost']=$record['FrameMatteCost'][$i];
	$record['PrintPrice'] = explode(',', $record['PrintPrice']);
	isset($i) && $record['PrintPrice']=$record['PrintPrice'][$i];
	$record['Markup'] = explode(',', $record['Markup']);
	isset($i) && $record['Markup']=$record['Markup'][$i];
	$record['MattePrice'] = explode(',', $record['MattePrice']);
	isset($i) && $record['MattePrice']=$record['MattePrice'][$i];
	$record['FrameToPrintPrice'] = explode(',', $record['FrameToPrintPrice']);
	isset($i) && $record['FrameToPrintPrice']=$record['FrameToPrintPrice'][$i];
	$record['FrameMattePrice'] = explode(',', $record['FrameMattePrice']);
	isset($i) && $record['FrameMattePrice']=$record['FrameMattePrice'][$i];

	$record['PrintShipWeight'] = explode(',', $record['PrintShipWeight']);
	isset($i) && $record['PrintShipWeight']=$record['PrintShipWeight'][$i];
	$record['MatteShipWeight'] = explode(',', $record['MatteShipWeight']);
	isset($i) && $record['MatteShipWeight']=$record['MatteShipWeight'][$i];
	$record['FrameToPrintShipWeight'] = explode(',', $record['FrameToPrintShipWeight']);
	isset($i) && $record['FrameToPrintShipWeight']=$record['FrameToPrintShipWeight'][$i];
	$record['FrameMatteShipWeight'] = explode(',', $record['FrameMatteShipWeight']);
	isset($i) && $record['FrameMatteShipWeight']=$record['FrameMatteShipWeight'][$i];

	$record['Inactive'] = explode(',', $record['Inactive']);
	isset($i) && $record['Inactive']=$record['Inactive'][$i];
	$record['Amount'] = explode(',', $record['Amount']);
	isset($i) && $record['Amount']=$record['Amount'][$i];
	*/

	return $record;
}

// Convert an array of numbers to cm.
// Return array (xcm, ycm).
// Round to $places decimal places.
function InToCm (&$item, $key, $places = 1) {
	$item = number_format(round($item * 2.54,$places), $places);
}

// --------------------
// Edit a table. The dependencies file has 'commands' on how to
// edit/delete/update a table (esp. how to delete related records).
// I guess a proper cascading delete would take care of this.
// $force bypasses protections, esp. for delete, e.g. with deleting pix which have price sets.
function EditTable ($action, $table, $ID, $vars = array ()) {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	global $DEPENDENCIES;
	global $msg, $error;
	global $LINK;
	global $BASEDIR, $LOGS;

	$DEBUG = 0;

	$action = strtolower(trim($action));	//just in case
	$table = trim($table);	//just in case
	$ID = trim($ID);			//just in case
	$result = "";

	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": EditTable : action=$action : table=$table<BR> vars:<BR>" .  ArrayToTable ($vars) .  "<BR><BR>";

	// don't need $vars for everything
	//if (!is_array($vars) || !$vars)
	//	return false;

	// had to do this...got warnings otherwise! Couldn't use if/then/else
	isset($_SESSION['fp_user']) && $fp_user = $_SESSION['fp_user'];
	
	switch ($action) {
		case "insert":
			isset($vars['ArtistID'])  || $vars['ArtistID'] = $fp_user;

			// Package up any params, store into the existing param field
			$params = "";
			$params = MergeParams ($params, $vars);
			$vars['Params'] = $params;

			if ($DEBUG)
				$error .= "Command is Insert<BR>";
			// Special treatments
			switch ($table) {
				case $PROJECTS :
					$DEBUG && $msg .= __FUNCTION__ .":".__LINE__ . ": added a record to $table for $fp_user<BR>";
					$vars['MaxPix'] > MAX_PROJECT_PICTURES && $vars['MaxPix'] = MAX_PROJECT_PICTURES;
					$vars['GroupID'] || $vars['GroupID'] = PUBLIC_GROUP_ID;
					$newID = AddRecord ($table, $vars);
					break;
				case $IMAGES :
					isset($vars['ProjectID']) ? $projectID = $vars['ProjectID'] : $projectID = null;
					$newID = HandleImage ("insert", '', $vars['ArtistID'], $projectID, $vars, "");
					//$msg .= "Picture added.<BR>";
					break;
				case $ARTISTS :
				// Add the portrait after.
					$myUser = new FPUser($LINK);
					$newID = $myUser->newUser($vars);
					if ($newID != FP_ADMINISTRATOR) {
						// Add artist to current group
						$groupID = $vars['GroupID'];
						$myGroup = new FPGroup ($LINK, $groupID);
						$myGroup->AddMembersToGroup ($newID, $groupID);
					}
					break;
				default :
					$newID = AddRecord ($table, $vars);
			}
			if (! $newID)
				$error .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": Internal error: I couldn't insert into $table<BR>";

			return $newID;
			break;
		case "delete":
			$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": EditTable:delete: ID = $ID<BR>";
			if ($ID) {
				//for images, we have to delete the image files, too
				switch ($table) {
					case DB_PROJECTS :
					// this deletes the image files, but not the records in the database
						DeleteProject ($ID);	//this calls CascadeDelete
						break;

					case DB_IMAGES :
						// Very important! Don't delete an image if it has been sold.
						$image = FetchImage ($ID);
						$salesinfo = ImageSalesInfo ($ID);
						if ($fp_user != FP_ADMINISTRATOR && $image["PriceSetID"] && ($salesinfo['totalsold'] > 0)) {
							fp_error_log(__FUNCTION__.": ERROR: Tried to delete an image (ID=$ID) with a price set (".$image["PriceSetID"]."). You are not allowed to delete an image if it has a Price Set.", 3, FP_ERROR_LOG);
							$error .= FetchSnippet ("error_CantDeleteLtdEdPicture");
							$result = false;
						} else {
							// this deletes the image files, but not the records in the database
							HandleImage ("delete", $ID);
							// this deletes the database entries
							CascadeDelete ($table, $ID);
							$image['Title'] ? $t = "<em>".$image['Title']."</em>" : $t ="";
							$msg .= "Picture $t (" . $image['URL'] . ") deleted.<BR>";
							$DEBUG &&  $msg .= "DeleteRow: success deleting image ".$ID."<BR>";
							$result = "Image deleted.";
						}
						break;

					case DB_ARTISTS :
					// don't delete the admin user!
						if ($ID != FP_ADMINISTRATOR) {
							// Also delete related Gallery of that user
							$artist = FetchArtist ($ID);
							$pid = $artist['ProjectID'];
							EditTable ("delete", DB_PROJECTS, $pid, array());
							CascadeDelete ($table, $ID);
							$msg .= "Artist deleted.<BR>";
						}
						break;
					case DB_GROUPS :
						if ($ID == PUBLIC_GROUP_ID) {
							$error .= "You cannot delete the default {fp:group}.<BR>";
						} else {
							$myGroup = new FPGroup ($LINK, $ID);
							$myGroup->Delete();
						}
						break;

					case DB_PRICESETS :
					// if the priceset has an imageID, clear the reference in the image record
						$priceset = GetPriceset ($ID);
						if ($priceset['ImageID']) {
							// restore PriceSetID for this image from saved "OldPID"
							$image = FetchImage($priceset['ImageID']);
							$params = DecodeArrayFromDB ($image['Params']);
							$sourceID = GetParam ($params, FP_PARAM_IMAGE_OLD_PID);
							$arr = array("PriceSetID"=>$sourceID,"EditionsLocked"=>0);
							EditTable ("update", DB_IMAGES, $priceset['imageID'], $arr);
						}
						CascadeDelete ($table, $ID);
						fp_error_log(__FUNCTION__.": Cleared the priceset ID in image #".$priceset['imageID']." because the price set #{$ID} was deleted.", 3, FP_MAINTENANCE_LOG);
						$msg .= "Deleted an entry in the $table list.<BR>";
						break;

					default:
						$msg .= "Deleted an entry in the $table list.<BR>";
						CascadeDelete ($table, $ID);
				}

			}
			break;
		case "update":
			$DEBUG = false;
			$DEBUG && $error .= __FUNCTION__.__LINE__."<BR>";
			$DEBUG && $error .= ArrayToTable ($vars['artistlist']);
			if ($ID) {
				// Package up any params, store into the existing param field
				$params = FetchParams ($table, $ID);
				$vars = FixCheckBoxFields ($vars);	// some params might be checkboxes!
				$params = MergeParams ($params, $vars);

				$vars['Params'] = $params;

				//$error .= __FUNCTION__.__LINE__.": ". ArrayToTable ($params);
				$DEBUG = 0;
				$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": DEBUG TURNED ON HERE<BR>";
				$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": EDITTABLE: case update<BR>";

				switch ($table) {
					case $IMAGES :
						isset($vars['ProjectID']) ? $projectID = $vars['ProjectID'] : $projectID = null;
						HandleImage ("update", $ID, $ID, $projectID, $vars);
						break;
					case $ARTISTS :
						$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": EditTable : Update Artist ID = $ID<BR>";
						$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": EditTable : Update PortraitImageID = {$vars['PortraitImageID']}<BR>";
						$success = UpdateRecord ($table, $ID, $vars);
						if (! $success)
							$error .= "ERROR: Could not update $table<BR>";
						break;
					case $PROJECTS :
						$vars['MaxPix'] > MAX_PROJECT_PICTURES && $vars['MaxPix'] = MAX_PROJECT_PICTURES;	// applies to Projects only
						$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": EditTable $table: " . ArrayToTable ($vars);
						// These params must be numbers
						$params[FP_PARAM_GALLERY_GROUPING] += 0;
						$params[FP_PARAM_GALLERY_SPACING] += 0;
						$params[FP_PARAM_GALLERY_GROUP_SPACING] += 0;

						// set 'Params' again to capture these changes
						$vars['Params'] = $params;
						$success = UpdateRecord ($table, $ID, $vars);
						if (! $success)
							$error .= "ERROR: Could not update $table<BR>";
						break;

					case $SUPPLIERS :
					// clean up the tube size entry
					// NO BREAK...use default to write record

					case DB_GROUPS :
						$success = UpdateRecord ($table, $ID, $vars);
						break;

					default :
						$vars['MaxPix'] > MAX_PROJECT_PICTURES && $vars['MaxPix'] = MAX_PROJECT_PICTURES;	// applies to Projects only
						$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": EditTable $table: " . ArrayToTable ($vars);
						// Params will be encoded by UpdateRecord
						$success = UpdateRecord ($table, $ID, $vars);
						if (! $success)
							$error .= "ERROR: Could not update $table<BR>";
						break;
				}
			}
			break;
		case "duplicate" :
			$record = FetchRowsByValue ($table, "*", 'ID', $ID, '', TRUE);
			$t = $record['Title'];
			$matches = array();
			preg_match ('/ (\d+)$/', $t, $matches);
			$i = $matches[1];
			$i || $i = 0;
			$i++;
			$t = preg_replace('/ ('.$matches[1].')$/', " $i", $t);

			//isset($record['Name']) && $record['Name'] = $record['Name'] . " $i";
			$record['Title'] = $t;
			$newID = EditTable ("insert", $table, $ID, $record);
			$msg .= "Duplicated record #{$ID}.";
			break;
		default:
			$error .= "Unknown command: $action<BR>";
	}
	return $result;
}

// Shortcut for deleting image records & files
function DeleteImage ($ID) {
	$ID && EditTable ('delete', DB_IMAGES, $ID);
}


/*
--------------------
FetchCoupons ($artistID)
Get coupon values for an artist.
*** A coupon is automatically added which is the "At Cost" coupon, and the code is the artist's FP password ***
Return an array of arrays:
	codes[code] = (description, code, discount)
*/
function FetchCoupons ($artistID) {
	$DEBUG = false;

	$artist = FetchArtist($artistID);
	$coupons = GetParam (unserialize($artist["Params"]), FP_PARAM_ARTIST_COUPONS);

	$DEBUG && fp_error_log(__FUNCTION__. ": coupon raw param: $coupons", 3, FP_ERROR_LOG);

	$coupons = explode("\r", $coupons);

	$arr = array();
	foreach ($coupons as $coupon) {
		$DEBUG && fp_error_log(__FUNCTION__. ": artistid=$artistID, $coupon", 3, FP_ERROR_LOG);
		$row = split (',', $coupon);
		$desc = trim($row[0]);
		$code = trim($row[1]);
		$discount = trim($row[2]);
		$arr[$code] = array ('description'=>$desc, 'code'=>$code, 'discount'=>$discount);
	}

	// Add "At Cost" code
	$arr[$artist['Password']] = array ('description'=>'At Cost', 'code'=>$artist['Password'], 'discount'=>100);

	$DEBUG && fp_error_log(__FUNCTION__. ":('description'=>$desc, 'code'=>$code, 'discount'=>$discount);", 3, FP_ERROR_LOG);

	return $arr;
}


/*
GeneratePassword
Generate a random, hard to duplicate password or coupon code.
*/
function GeneratePassword ($password_length = 12) {
	srand(make_seed());
	$alfa = "1234567890qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM";
	$token = "";
	for($i = 0; $i < $password_length; $i ++) {
		$token .= $alfa[rand(0, strlen($alfa))];
	}
	return $token;
}

function make_seed() {
	list($usec, $sec) = explode(' ', microtime());
	return (float) $sec + ((float) $usec * 100000);
}


/*
SendPasswordByEmail ($userID)
Send the password by email to a user.
*/

function SendPasswordByEmail ($artistID) {
	global $msg, $error;
	global $LINK;
	$DEBUG = false;

	$artist = FetchArtist($artistID);

	$to = "{$artist['Fullname']} <{$artist['Email']}>";
	$subject = FP_SYSTEM_DISPLAY_NAME . ": Lost Password";

	$text = FetchSnippet ("SendPasswordByEmail");
	$text = Substitutions ($text, array (
			'Username'	=> $artist['Username'],
			'Password'	=> $artist['Password']
	));
	$text = ReplaceSysVars ($text);


	if ($result = mail($to, $subject, $text, FP_EMAIL_HEADERS)) {
		fp_error_log(__FUNCTION__.": Sent password to user #{$artistID} ({$artist['Fullname']}) at $to.", 3, FP_MAINTENANCE_LOG);
		$DEBUG && $msg .= __FUNCTION__."::".__LINE__.":: $to, $toName, $subject<BR>";
		return true;
	} else {
		fp_error_log(__FUNCTION__.": FAILED TO send password to user #{$artistID} ({$artist['Fullname']}) at $to.", 3, FP_MAINTENANCE_LOG);
		$DEBUG && $msg .= __FUNCTION__."::".__LINE__.":: FAILED $to, $toName, $subject<BR>";
		$error .= "Could not send email! ($result)<BR>";
		return false;
	}
}




function TestPME () {
	$table = DB_IMAGES;
	$rows = array (1398, 1400);
	$actions = array (array('fit_image_to_size', 35), array ('fit_image_to_priceset') );
	$res = ProcessMultiEdit ($table, $rows, $actions);
	return $res;
}

// ProcessMultiEdit
// A new multieditor, using more sophisticated Javascript to get rows, params
// $table is a table, e.g Images
// $rows is an array of ID's of records
// $actions is an array : actions = array ( array (command, param));

function ProcessMultiEdit ($table, $rows, $actions) {
	global $msg, $error;
	global $LINK;
	global $PRINTDPI;

	$DEBUG = 1;

	$res = "";
	// result if the page should be reloaded in browser to reflect changes
	$reload = false;

	//Edit marked rows, with multieditfield = multieditvalue

	foreach ($rows as $ID) {
		// Get the record
		$row = GetRecord( $table, $ID);
		isset($row['Params']) ? $params = DecodeArrayFromDB ($row['Params']) : $params = null ;

		// Useful vars to get before looping
		switch ($table) {
			case DB_IMAGES :
				$d = GetImageSizeByID ($ID);
				$w = $d['width'];
				$h = $d['height'];
				$greater = ImageGreaterSide ($w, $h);

				$projectID = $row['ProjectID'];
				$pricesetID = FetchPriceSetID ($ID, $projectID);
				$priceset = GetPriceSet ($pricesetID);
				($priceset['printsizes'])
						? $maxsize = max ($priceset['printsizes'])
						: $maxsize = 0;
		}

		// Apply actions to record
		foreach ($actions as $action) {
			$command = $action[0];
			$val = $action[1];
			if ($command == "fit_image_to_size" ) {
				// IMAGES COMMAND
				// Modify the image size so it's large enough to print at the request size
				// This is the "file at printer" sizing param!
				// $val is size of larger side

				// current side in pixels
				$r = ($val * $PRINTDPI) / $greater;
				$newWidth = ceil($r * $w);
				$newHeight = ceil($r * $h);
				// Set "Print has file" flag
				$params = SetParam ($params, FP_PARAM_IMAGE_OFFSITE, 1);
				$params = SetParam ($params, FP_PARAM_IMAGE_WIDTH, $newWidth);
				$params = SetParam ($params, FP_PARAM_IMAGE_HEIGHT, $newHeight);
				SaveParams ($table, $ID, $params);
				$reload = true;

				if ($DEBUG) {
					$logmsg = __FUNCTION__."1) $table : id: $ID, $command ($val) : Set w=$newWidth, h=$newHeight<BR>";
					fp_error_log(__FUNCTION__.": $logmsg", 3, FP_MAINTENANCE_LOG);
				}


			}
			if ($command == "fit_image_to_priceset" ) {
				// IMAGES COMMAND
				// Modify the image size so it's large enough to print at the max size of the
				// parent project's print set.
				// This is the "file at printer" sizing param!
				// $val is size of larger side

				$val = $maxsize;
				// current side in pixels
				$r = ($val * $PRINTDPI) / $greater;
				$newWidth = ceil($r * $w);
				$newHeight = ceil($r * $h);
				// Set "Print has file" flag
				$params = SetParam ($params, FP_PARAM_IMAGE_OFFSITE, 1);
				$params = SetParam ($params, FP_PARAM_IMAGE_WIDTH, $newWidth);
				$params = SetParam ($params, FP_PARAM_IMAGE_HEIGHT, $newHeight);
				SaveParams ($table, $ID, $params);
				$reload = true;


				// modify the image size so it's large enough to print at the largest size of the image's priceset
				if ($DEBUG) {
					$logmsg = __FUNCTION__."2) id: $ID, $command ( $val )<BR>";
					fp_error_log(__FUNCTION__.": $logmsg", 3, FP_MAINTENANCE_LOG);
				}


			}
			if ($command == "set_fixed_size" ) {
				// IMAGES COMMAND
				// Modify the image so it is fixed.
				// Set the width/height.
				list($w,$h) = split(",", $val);
				$w = trim($w);
				$h = trim($h);

				if ($w>0 && $h > 0) {
					$params = SetParam ($params, FP_PARAM_IMAGE_IS_FIXED_SIZE, 1);
					$params = SetParam ($params, FP_PARAM_IMAGE_FIXED_WIDTH, $w);
					$params = SetParam ($params, FP_PARAM_IMAGE_FIXED_HEIGHT, $h);
					SaveParams ($table, $ID, $params);
					$reload = true;
					if ($DEBUG) {
						$logmsg = __FUNCTION__."3) $table : id: $ID, $command ($val ) : Set w=$w, h=$h";
						fp_error_log(__FUNCTION__.": $logmsg", 3, FP_MAINTENANCE_LOG);
					}
				}


			}
			if ($command == "change_parent_project" ) {
				// IMAGES COMMAND
				// Change the parent project to the requested project
				if ($DEBUG) {
					$logmsg = __FUNCTION__."4) id: $ID, $command ( $val )";
					fp_error_log(__FUNCTION__.": $logmsg", 3, FP_MAINTENANCE_LOG);
				}
			}
			if ($command == "PriceSetID" ) {
				// IMAGES COMMAND
				// Set the priceset
				if ($val != "") {
					UpdateRecord( $table, $ID, array("PriceSetID" => $val) );
					$reload = true;
				}

				if ($DEBUG) {
					$logmsg = __FUNCTION__."5) id: $ID, $command ( $val )";
					fp_error_log(__FUNCTION__.": $logmsg", 3, FP_MAINTENANCE_LOG);
				}
			}
			if ($command == "clear_fixed_size" ) {
				// IMAGES COMMAND
				// Modify the image so it is not fixed.
				$params = SetParam ($params, FP_PARAM_IMAGE_IS_FIXED_SIZE, 0);
				SaveParams ($table, $ID, $params);
				$reload = true;

				if ($DEBUG) {
					$logmsg = __FUNCTION__."6) $table : id: $ID, $command ($val = $w,$h) : Set w=$newWidth, h=$newHeight";
					fp_error_log(__FUNCTION__.": $logmsg", 3, FP_MAINTENANCE_LOG);
				}
			}
			if ($command == "set_edition_size" ) {
				// IMAGES COMMAND
				// Set the custom edition size for the image
				if ($val > 0 && !$row["EditionsLocked"]) {
					$params = SetParam ($params, FP_PARAM_IMAGE_EDITION_SIZE, $val);
					SaveParams ($table, $ID, $params);
					$reload = true;
					if ($DEBUG) {
						$logmsg = __FUNCTION__."6) $table : id: $ID, $command ($val = $w,$h) : Set w=$newWidth, h=$newHeight";
						fp_error_log(__FUNCTION__.": $logmsg", 3, FP_MAINTENANCE_LOG);
					}
				}

			}
			if ($command == "Projects" ) {
				// IMAGES COMMAND
				// SClearet the custom edition size for the image
				if (!$row["EditionsLocked"]) {
					$params = SetParam ($params, FP_PARAM_IMAGE_EDITION_SIZE, '');
					SaveParams ($table, $ID, $params);
					$reload = true;
					if ($DEBUG) {
						$logmsg = __FUNCTION__."6) $table : id: $ID, $command ($val = $w,$h) : Set w=$newWidth, h=$newHeight";
						fp_error_log(__FUNCTION__.": $logmsg", 3, FP_MAINTENANCE_LOG);
					}
				}
			}
			if ($command == "no_sell_matte" ) {
				// IMAGES COMMAND
				// Modify the image so it can/cannot be sold with matte
				$params = SetParam ($params, FP_PARAM_IMAGE_NO_SELL_MATTE, $val);
				SaveParams ($table, $ID, $params);
				$reload = true;
			}
			if ($command == "no_sell_frame" ) {
				// IMAGES COMMAND
				// Modify the image so it can/cannot be sold with frame
				$params = SetParam ($params, FP_PARAM_IMAGE_NO_SELL_FRAME, $val);
				SaveParams ($table, $ID, $params);
				$reload = true;
			}
			
			if ($command == "delete" ) {
				// Delete the image!
				EditTable ("delete", DB_IMAGES, $ID, array());
				$msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": Deleted Image ID=$ID via multi-editor<BR>";
				$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": Deleted Image ID=$ID via multi-editor<BR>";
				$reload = true;
			}
			
			
			
		}
	}
	if ($reload)
		$res = "reload";

	fp_error_log(__FUNCTION__.": RESULT: $res\n", 3, FP_MAINTENANCE_LOG);
	return $res;

	/*
	if ($idset) {
		$query = "UPDATE $table SET $field=\"$value\" WHERE ID IN ( $idset )";
		$DEBUG && $msg .= $query . "<BR>";
		$result = mysqli_query ($LINK, $query);
		if (! $result)
		print mysqli_error($LINK) . "Failed to make $field=$value in $idset<BR>";
	}

	// FTP the marked rows
	$flag = "FTP";
	$IDlist = FetchListFromMultiForm ($flag, $vars);
	if (count($IDlist))
		$page = BuildFTPForm ($IDlist, 'ftp.somewhere.com','picturefolder','username','password');

	//Delete marked rows
	$flag = "delete";
	$IDlist = FetchListFromMultiForm ($flag, $vars);

	foreach ($IDlist as $ID) {
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": Deleting ID=$ID from $table<BR>";
		EditTable ('delete', $table, $ID, '');
	}
	
	*/
}



/*
====================================================================================================
====================================================================================================
                               THE FOLLOWING FUNCTIONS ARE NOT IN USE!
====================================================================================================
====================================================================================================
*/


//----------------------
// NOT IN USE!!!
function EditRowForm ($table, $vars) {
	global $Themes;
	return BuildInputRecordForm ($table, $vars, FALSE);
}

//----------------------
// NOT IN USE!!!
function NewRowForm ($table, $vars) {
	global $Themes;
	print __FUNCTION__." : ++++++++ is this in use? ++++++++++++";
	return BuildInputRecordForm ($table, $vars, TRUE);
}


//----------------------
// NOT IN USE!!!
// BuildInputRecordForm: Build a form for input for a database record (i.e. row)
// if $blank is TRUE, then show a blank form to enter a new record, and $action = insert
// otherwise, fill in the form.

function BuildInputRecordForm ($table, $vars, $blank = FALSE) {
	global $Themes;
	global $PHOTOS_GALLERY, $THUMBNAILS, $MATTED, $ORIGINALS;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;

	$DEBUG = 1;

	// define vars used later
	$priceOptions = "";
	$ProjectIDList = "";
	$projectInfo = "";
	$artistinfo = "";
	$projectvals = array();
	$currvals = array();

	// If blank then we're creating a new record, and the following action upon
	// 'submit' is to insert, not update in the database.
	$blank
			? $action = "insert"
			: $action = "update";

	if (empty($vars['ID']) OR $blank) {
		$ID = "";
	} else {
		$ID = trim($vars['ID']);
	}

	empty($vars['fp_user']) ? $fp_user = '' : $fp_user = $vars['fp_user'];
	empty($vars['ArtistID']) ? $ArtistID = '' : $ArtistID = $vars['ArtistID'];
	empty($vars['ProjectID']) ? $projectID = '' : $projectID = $vars['ProjectID'];

	$AccessLevel = FetchAccessLevel ($fp_user);
	$form =	BuildDefaultInputForm ($action, $table, "", $ID, $AccessLevel);

	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": ID = $ID<BR>";
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": fp_user = $fp_user<BR>";
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": AccessLevel = $AccessLevel<BR>";
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": Form name should be : ".strtolower("entry_form_" . $table . "_" . $AccessLevel) . "<BR>";

	// Get existing data (if there is any)
	// If this is new, i.e. blank, record, there won't be data
	if ($ID) {
		$currvals = FetchRowsByValue ($table, "*", 'ID', $ID, '', TRUE);
		//$DEBUG && $msg .= ArrayToTable ($currvals);
		$checked = array ('yes' => "CHECKED", 'no' => '');
		$checkedbinary = array ('1' => "CHECKED", '0' => '');
		if (! empty ($currvals['Matted']))
			$currvals["matted_checked"] = $checkedbinary[$currvals['Matted']];
		if (isset($currvals['URL'])) {
			$currvals['IMGPhoto'] = URLtoIMG ("$PHOTOS_GALLERY/" .	$currvals['URL']);
			$currvals['IMGThumbnail'] = URLtoIMG ("$THUMBNAILS/" .	$currvals['URL']);

			if (! empty ($currvals['Featured']))
				$currvals["featured_checked"] = $checked[$currvals['Featured']];
			// Put in requested images
			if (! empty ($currvals['Active']))
				$currvals["active_checked"] = $checked[$currvals['Active']];
		}

		if (isset($currvals['ArtistID']))
			$ArtistID = $currvals['ArtistID'];

		// Set $projectID, which can come from different sources....
		switch ($table) {
			case "Projects" :
			//If this is working with table=Projects, the ID is the ProjectID.
				$projectID = $ID;
				$projectvals = FetchRowsByValue ('Projects', "*", "ID", $projectID, '', TRUE);
				$projectdates = FetchProjectDates ($projectvals);
				// Get dates, etc., add into the project info array
				$projectvals = array_merge($projectvals, $projectdates);
				if (!isset($projectvals['ArtistID']))
					$projectvals['ArtistID'] = $fp_user;
				$ArtistID = $projectvals['ArtistID'];
				// Now, if PPS is empty, leave it that way. Empty means no sales allowed
				//if (!isset($projectvals['pps']) || $projectvals['pps'] == "")
				//	$projectvals['pps'] = "default";
				break;
			case $ARTISTS :
			//ProjectID in Artists is an unfortunate name...should be DefaultProjectID
				$projectID = $currvals['ProjectID'];
				break;
		}

	}


	// If we do this, we replace owners of projects, etc., with the current admin level user. Not good.
	$ArtistID || $ArtistID = $fp_user;
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": BuildInputRecordForm: ARTIST ID = $ArtistID<BR>";


	if ($ArtistID)
		$artistinfo = FetchRowsByValue ($ARTISTS, '*', 'ID', $ArtistID, 'Lastname', TRUE);

	switch ($table) {
		case $PROJECTS :
			$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": Preparing artistlist, storylist, imagelist for the form<BR>";
			$currvals['artistlist'] = FetchArtistList ($projectID, $fp_user);
			$currvals['storylist'] = BuildProjectStories ($projectID, $fp_user);
			$currvals['imagelist'] = BuildProjectImagesMulti ($projectID, $fp_user);
			break;
		case $ARTISTS :
			if (isset($currvals['PortraitImageID'])) {
				$imageinfo = FetchRowsByValue($IMAGES, "URL", "ID", $currvals['PortraitImageID'], '', TRUE);
				$currvals['PortraitIMG'] = URLtoIMG ("$THUMBNAILS/" .  $imageinfo['URL']);
			}
			//print "BuildInputRecordForm: Artists " . $artistinfo['PortraitImageID'] . "<BR>";
			break;
	}

	// access level popup
	if (preg_match('/\{AccessLevel\}/i', $form)) {
		$currvals['AccessLevel'] = BuildAccessLevelMenu ($ArtistID);
	}

	// projects popup
	if (preg_match('/\{ProjectIDList\}/i', $form)) {
		$ProjectIDList = OptionList ("Projects", "ProjectID", "ID", "Title", "Active = 'yes'", $projectID, "Title", OPTION_LIST_IS_POPUP);
	}

	// artists popup
	if (preg_match('/\{ArtistIDList\}/i', $form)) {
		$fieldlabel = "CONCAT_WS(', ', Lastname, Firstname) AS Fullname";
		$ArtistIDList = OptionList ($ARTISTS, "ArtistID", "ID", $fieldlabel, "", $ArtistID, "Fullname", OPTION_LIST_IS_POPUP);
	} else {
		$ArtistIDList = '';
	}

	if (preg_match("/\{priceOptions\}/i", $form)) {
		if (isset($currvals['PriceID'])) {
			$currprice = trim($currvals['PriceID']);
		} else {
			$currprice = trim($artistinfo['DefaultPriceID']);
		}
		$currvals['priceOptions'] = OptionList ("Prices", "ID", "ID", "Price", "ArtistID = $ArtistID", $currprice, "Price", OPTION_LIST_IS_POPUP);
	}

	// set all CHECKBOXES appropriately
	$form = SetCheckBoxesInForm($form, $currvals);

	$formvars = array ( '$table' => $table,
			'$ID' => $ID,
			'$action' => $action,
			'$ArtistID' => $ArtistID,
			'$projectID' => $projectID,
			'$ArtistIDList' => $ArtistIDList,
			'$ProjectIDList' => $ProjectIDList,
			'$fp_user' => $fp_user,
	);

	// Fetch and use defaults if this is a blank form.
	// We grab all defaults from the artist; not all will be put into the form
	// since each form is different. some will just be ignored.
	if ($blank) {
		$artistinfo = FetchArtist ($ArtistID);
		$defaults = FetchDefaults ($artistinfo);
		$formvars = array_merge($defaults, $formvars);	//second param overwrites the first if keys are same
	}

	$DEBUG && $msg .= ArrayToTable ($formvars);


	$form = Substitutions ($form, $formvars);
	$form = Substitutions ($form, $projectvals);
	$form = Substitutions ($form, $vars);

	// OK, We always stick in the $currvals, because these are set even for a blank, new item.
	//	if (! $blank) {
	$form = Substitutions ($form, $currvals);
	//	}
	//dump unused $variables
	$form = preg_replace ('/(\\' . FIELDPREFIX . '\w+\b)/','',$form);
	//	$form = preg_replace ('/(\\$\w+\b)/','',$form);
	return $form;
}

/*
	function countAvailableItems ($codes)
	How many items in an array of items, e.g. frames, are actually available.
	When NOT available, the name begins with "/", e.g. "/Gold Frame"
	$codes is TEXT, a list of items separated by returns.
*/
function countAvailableItems ($codes) {
	$items = explode ("\n", $codes);
	$k = 0;
	foreach ($items as $item) {
		$item = trim($item);
		if ($item != "" && $item[0] != "/")
			$k++;
	}
	return $k;
}


/*
	unlinkb
	This does an unlink, but if there's an error, logs it.
*/
function unlinkb ($path) {
	if (file_exists($path))
	{
		$res = unlink ($path);
		if (!$res) {
			fp_error_log(__FUNCTION__.": (called by ".getCallingFunction().")*** Error : unlink $path", 3, FP_MAINTENANCE_LOG);
		}
	} else {
		fp_error_log(__FUNCTION__.": (called by ".getCallingFunction().")*** Error : unknown path :  $path", 3, FP_MAINTENANCE_LOG);
		$res = null;
	}
	return $res;
}


// Start/End debugging timer
// param $active allows us to use $DEBUG as a flag
// The EndTimer doesn't stop timing, just returns current timer 
// To reset, use StartTimer again.
function StartTimer ($active = false) {
	global $timer, $timerPauseTime;
	if ($active)
		{
		if ($timerPauseTime)
			{
			$timer = microtime(true) - ($timerPauseTime - $timer);
			$timerPauseTime = 0;
			}
		else
			{
			$timer = microtime(true);
			}
		}
}

// Pause timer by resetting 
function PauseTimer ($active = false) {
	global $timer, $timerPauseTime;
	$active && $timerPauseTime = microtime(true);
}

function CurrentTimer ($threshold = 0) {
	global $timer, $timerPauseTime;
	
	if ($timerPauseTime)
		{
		return null;
		}
	else
		{
		$t = round (microtime(true) - $timer, 2);
		if ($t>$threshold) 
			{
			return $t;
			}
		else
			{
			return null;
			}
		}
}


function EndTimer ($threshold = 0.01) {
	global $timer, $timerPauseTime;
	
	if ($timerPauseTime)
		{
		$timer = microtime(true) - ($timerPauseTime - $timer);
		$timerPauseTime = 0;
		}
	else
		{
		$timer = microtime(true);
		}

	$t = round (microtime(true) - $timer, 2);
	if ($t>$threshold) 
		{
		return $t;
		}
	else
		{
		return null;
		}
}


/*
function CompactArraysToStrings ($vars)
Process the input from an HTML form for a price set. Warning, this will convert
all arrays, even something that snuck into your $vars but wasn't on the HTML
form. Whatever that might be...
This means converting arrays to strings to put into the DB.
*/
function CompactArraysToStrings ($vars) {
	global $msg, $error;
	global $LINK;

	// Convert arrays to strings
	reset ($vars);
	while (list($k,$v) = each ($vars)) {
		if (is_array($v)) {
			array_walk ($v, 'trimME');
			$v = implode (",", $v);
			$vars['a_'.$k] = $v;
			unset($vars[$k]);
		}
	}
	return $vars;

}







/*
------------------------------------------------------------------------
-- CLIENT SIGN IN

Very simple. We just get the client id. Other functions use this to determine whether
they want to show data based on it. For example, the projects list should check
whether a project is visible by a particular client.
------------------------------------------------------------------------
*/
function clientLogin($username) {

	$DEBUG = false;

	$DEBUG  && fp_error_log(__FUNCTION__ . ": BEGIN", 3, FP_ACTIVITY_LOG);
	$DEBUG  && fp_error_log(__FUNCTION__ . ": Username = $username", 3, FP_ACTIVITY_LOG);

	$_SESSION['clientid'] = $username;
	fp_error_log(__FUNCTION__ . ": Someone signed in as $username", 3, FP_ACTIVITY_LOG);
	return true;

}





?>