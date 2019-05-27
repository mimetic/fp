<?php

/*
Must set the following constants in your calling scripts:
	PP_ORDERS_ID : name of id field in the orders database of the calling script
	PP_USERS_ID : name of id field in the users database of the calling script
*/

// Generate a secret code, a kind of primitive public key
// Let's mix together the ID with the secret code in a way that ordinary mortals 
// won't suspect. Secret encodes current date.
function Generate_PP_Secret ($artistID) {
	$secret = SHA1($artistID) . PP_SECRET .	 SHA1(date("dmy"),$t);
	return $secret;
}

function Verify_PP_Secret ($secret, $num) {
	$numcrypt = substr ($secret, 0, strpos ($secret, PP_SECRET));
	$datecrypt = substr ($secret, strlen (PP_SECRET) + strlen ($numcrypt));
	if ($numcrypt == sha1($num) && $datecrypt == sha1(date("dmy")))
		return true;
	return false;
}

// Check the user exists in the calling service's database
// If found, return the record
function VerifyUser ($service, $username, $password, $userID = null) {
	global $msg, $error;
	global $LINK;

	$DEBUG = false;
	
	if (!isset($service)) {
		$error .= "Requested service doesn't exist in the config file!<BR>";
		return false;
	}
	
	$users = $service['users'];
	$usernameField = $service['usernameField'];
	$passField = $service['passField'];
	$usersIDField = $service['usersIDField'];

	$userID ? $checkID = "	AND $usersIDField = '$userID'" : $checkID = "";
	
	$query = "SELECT * FROM $users WHERE $usernameField = '$username' AND $passField = '$password' $checkID LIMIT 1";
	$exists = mysqli_num_rows(mysqli_query ($LINK, $query));
	
	return $exists;
}


// Replace coded system vars. These are so common we have a function for this.
function ReplaceSysVars ($form) {
	global $SYSTEMNAME, $SERVICENAME, $SERVICEURL, $baseURL, $FP_SYSTEM_EMAIL, $SYSTEMURL;
	global $msg, $error;

	$form = preg_replace ("/\{session_id\}/", session_id(), $form);
	$age = time() - $_SESSION['StartTime'];
	$sec = $age % 60;
	($sec < 10) && $sec = "0$sec";
	$timemsg = "<i>You've been on the system for " . floor($age / 60) . " minutes</i>";
	$form = preg_replace ("/\{timemsg\}/", $timemsg, $form);

	$vars = array ("session_id" => session_id(),
					"msg"			=>	$msg,
					"error"			=>	$error,
					"table"			=>	$table,
					"basename"		=>	basename($_SERVER['PHP_SELF']),
					"SYSTEMNAME"	=>	$SYSTEMNAME,
					"SERVICENAME" =>	$SERVICENAME,
					"SERVICEURL"	=>	$SERVICEURL,
					"SystemEmail" =>	$FP_SYSTEM_EMAIL,
					"SYSTEMURL"		=>	$SYSTEMURL,
					"timemsg"		=>	$timemsg
					);
	$vars = array_merge ($vars, get_defined_constants ());
	$form = Substitutions ($form, $vars);
	return $form;
}
	

function SaveOrder ($vars = array(), $userID = null) {
	global $msg, $error;
	
	$DEBUG = FALSE;
	
	// $pairs translates subscription var names to IPN var names used in the database
	$pairs = array (
			"txn_type"			=>	'touched',
			"option_selection1"		=>	$vars['service'],
			"coupon_code"			=>	$vars['coupon'], 
			"item_name"			=>	$vars['item_name'], 
			"item_number"			=>	$vars['ID'], 
			"option_selection1"		=>	$vars['service'],
			"option_selection2"		=>	$vars['package'],
			"period1"			=>	$vars['p1'] .strtolower($vars['t1']), 
			"period2"			=>	$vars['p2'] .strtolower($vars['t2']), 
			"period3"			=>	$vars['p3'] .strtolower($vars['t3']), 
			"amount1"			=>	$vars['a1'], 
			"amount2"			=>	$vars['a2'], 
			"amount3"			=>	$vars['a3'],
			"reattempt"			=>	$vars['sra'], 
			"recurring"			=>	$vars['src'], 
			"recur_times"			=>	$vars['srt'],
			// non-paypal vars follow
			"pp_comments"			=>	$vars['pp_comments'],
			"pp_user_id"			=>	$vars['pp_user_id'],	// id of user in the service DB
			"pp_service_id"			=>	$vars['pp_service_id'],
			"pp_package_id"			=>	$vars['pp_package_id'],
			"pp_status"			=>	$vars['pp_status']
			);
	
	// Merge both arrays so all values are available for the update
	$pairs = array_merge ($vars, $pairs);

	// Does the record already exist?
	$userID || $userID = $vars['pp_user_id'];
	if ($ID = OrderExists ($userID)) {
		//$pairs['pp_comments'] = "Package ".$vars['pp_package_id'];
		UpdateOrder ($pairs, $ID);
	} else {
		$ID = AddRecord ( PP_ORDERS_DB , $pairs );
	}
	return $ID;
}

// VerifyCoupon
// Coupons are based on text strings, e.g sign-in names. The coupon must match the provided text string
function VerifyCoupon ($coupon, $text) {
	global $coupon_code_gen_mod, $coupon_codes;
	
	$found = false;
	foreach (array_keys($coupon_codes) as $cc) {
		if ($coupon == hash ("md5", $cc . $text . $coupon_code_gen_mod)) {
			$found = $cc;
		}
		if ($found)
			break;	// leave loop
	}
	return $found;	
}


// GenerateCoupon
// Create a coupon from a text string
// The $code is the actual coupon code that tells us which coupon it is, i.e. the key for the
// $coupon_codes array in config.inc
function CreateCoupon ($text, $code) {
	global $coupon_code_gen_mod;
	
	$code = hash ("md5", $code . $text . $coupon_code_gen_mod);
	return $code;
}


function BuildJSArrayString ($varname, $arr) {
	array_walk ($arr, 'CommasToHTML');
	$x = join (',', $arr);
	$x = "var $varname = \"$x\";\n";
	return $x;
}

function BuildJSArray ($varname, $arr) {
	array_walk ($arr, 'CommasToHTML');
	$x = '"' . join ('", "', $arr) . '"';
	$x = "$varname = new Array ($x);\n";
	return $x;
}

//array_walk function to replace commas
function CommasToHTML(&$item)
{
	$item = str_replace (",", "&#44", $item);
}

function ArrayToTable ($myarray) {
	if (! is_array($myarray))
		return false;
	$table = "<table border=1>\n";
	foreach ($myarray as $k => $v) {
		$table .= "<tr><td><B>$k</B></td><td>$v</td><tr>\n";
/*		if ($DEBUG) */
/*			print "processing $k, $v<BR>"; */
	}
	$table .= "</table>\n";
	return $table;
}

// substitute variables of the form {$myvarname} in $page
// with values in the array, $subs, where
// myvarname => myvarvalue
function Substitutions ($page, $subs) {
	$DEBUG = false;
	if (! is_array($subs))
		$subs = array ($subs);
	foreach ($subs as $k => $v) {
		// Also replace this form : {varname}
		$page = str_replace ('{$'.$k.'}', $v, $page);
		$page = str_replace ('{'.$k.'}', $v, $page);
		$DEBUG && print "REPLACE $k with [ $v ] <BR>\n";
	}
	return $page;
}

function StripEmptyVars ($page) {

	$pattern = '/\{\$.*?\}/'; // the ? makes the .* ungreedy
	$replacement = "";
	$page = preg_replace ($pattern, $replacement, $page);
	return $page;
}

// Given a string with variables in it, of the form {$myvar}
// get the list of all vars in that string that will need replacing
// Useful for code snippets with vars embedded. Does NOT work with
// array variables, e.g. $myarr[1] 
function GetVarList ($text) {
	$s = '/\{\$(.*?)\}/';
	preg_match_all ($s, $text, $vars);

	return ($vars[1]);
}

// Create an HTML drop down menu from an array of arrays
// where the key is the returned value
// and $item['name'] is the displayed value
function BuildDropDown ($options, $checked = "") {
	$dropdown = "";
	reset($options);
	foreach ($options as $key => $value) {
		if (!$value['disabled']) {
			if ( $key == $checked ) {
				$dropdown .= "<option value=\"$key\" selected>" . $value['name'] . "</option>\n";
			} else {
				$dropdown .= "<option value=\"$key\">" . $value['name'] . "</option>\n";
			}
		}
	}
	return $dropdown;
}

/* Get a snippet (of text) */
/* If the $id is an integer, it will be used as the id */
/* otherwise, $id is assumed to be the Title of the snippet, i.e. find by name */
function FetchSnippet ($id) {
	global $SNIPPETS, $msg, $error;

	$DEBUG = false;
	$DEBUG && print "<B>SNIPPETS = $SNIPPETS</B><BR>";
	$DEBUG && print "<B>Seeking snippet id = [$id]</B><BR>";
	$snippet = "";
	
	// Check if there's a text file in the snippets
	// Try .txt and .html endings, just in case
	
	file_exists ("$SNIPPETS/".$id) && $snippet = file_get_contents ("$SNIPPETS/".$id);
	if (empty($snippet)) {
		file_exists("$SNIPPETS/".$id.".txt") && $snippet = file_get_contents ("$SNIPPETS/".$id.".txt");
	}
	if (empty($snippet)) {
		file_exists("$SNIPPETS/".$id.".html") && $snippet = file_get_contents ("$SNIPPETS/".$id.".html");
	}

	/* if a snippet contains a reference to another snippet, insert it (recursive) */
	$count = preg_match_all ("/< *snippet.*?= *('|\")(.*?)\\1>/", $snippet, $matches);
	for ($i=0; $i< count($matches[0]); $i++) {
		$found_code = $matches[0][$i];
		$snippet_id = $matches[2][$i];
		$insert_snippet = FetchSnippet ($snippet_id);
		if ($DEBUG)
			$error .= "DEBUG: found inserted snippet id $snippet_id<BR>";
		$snippet = str_replace ($found_code, $insert_snippet, $snippet);
	}
	return $snippet;
}

//----------
// WriteLog (entry)
// Write a log entry
//	if $prepend, than add to beginning of log
// 1-Open new file, write entry.
// 2-Write old file to new file
// 3-delete old file, rename new file
// 12/28/04: Add comments to beginning of log
function WriteLog ($entry, $showdate = false, $prepend = false) {
	$tempfile = "temp";
	$d = date('j.F.Y|g:i a');
	$filename = LOG;	// system defined name
	$tempfile = "temp" . LOG;
	$showdate && $entry = "$d:" . $entry;

	if ($prepend) {
		if (! ($newfd = fopen ("$tempfile", "w")))
			print "ERROR WriteLog: Cannot open temp file ($tempfile)<BR>\n";		
		if (fwrite ($newfd, $entry."\n") === FALSE)
			print "ERROR WriteLog: Cannot write to temp file ($tempfile)<BR>\n";
		if (fwrite ($newfd, file_get_contents($filename)) === FALSE)
			print "ERROR WriteLog: Cannot read/write old log to temp file ($tempfile)<BR>\n";
		fclose ($newfd);
		file_exists ($filename) && unlink ($filename);
		rename ($tempfile, $filename);
	} else {
		file_put_contents ($filename, "{$entry}\n", FILE_APPEND);
	}
}


// --------------- DATABASE FUNCTIONS
/*
PP DATABASE FUNCTIONS

*/

// Fetch a record as an array
function GetOrder($orderID) {
	$record = FetchRowsByValue (PP_ORDERS_DB, "*", PP_ORDERS_ID, $orderID, "", true);
	return $record;
}

// If order exists, return the ID of the order record
function OrderExists ($userID) {
	$q = "SELECT " . PP_ORDERS_ID . " FROM " . PP_ORDERS_DB . " WHERE pp_user_id = $userID LIMIT 1";
	$res = mysqli_query ($q);
	mysqli_num_rows ($res) 
		? list($orderID) = mysqli_fetch_array(mysqli_query ($q))
		: $orderID = false;
	return $orderID;
}

// Fetch a record as an array
function GetUser($ID) {
	$record = FetchRowsByValue (PP_DB_USERS, "*", PP_USERS_ID, $ID, "", true);
	return $record;
}


// $fields = GetFieldList ($table);
// Get list of mysql column names, i.e. field names
function GetFieldList ($table) {	
	$res = mysqli_query ("SHOW COLUMNS FROM $table");
	while ($row = mysqli_fetch_array($res))
		$col_names[]=$row[0];
	return $col_names;
}


//--------------------------
// Insert a record, return the ID of the added record
function AddRecord( $table, $pairs ) {
	global $msg, $error;
	global $LINK;
	
	$DEBUG = false;
	$pairs = StripNonExistantFields ($table, $pairs);
	$cols = makeCols($pairs);
	$vals = makeVals($pairs);
	$query = "insert into $table ($cols) values ($vals)";
	$result =	 mysqli_query ($LINK, $query);
	$ID = mysqli_insert_id();
	return $ID;
}


// return mysql result
// IF $returnarray is set, return the first row as an array (dump the rest)
function FetchRowsByValue ($table, $set, $keyfield, $value, $order = "", $returnarray = TRUE) {
	global $error, $msg;
	global $LINK;
	
	$DEBUG = 0;
	
	$returnarray ? $limit = " LIMIT 1" : $limit = "";
	
	if ($order) 
		$order = "ORDER BY $order";
	if (! ($keyfield and $value)) {
		$query = "select $set from $table $order";
	} else {
		$query = "select $set from $table where $keyfield='$value' $order $limit";
	}
	if ($DEBUG) 
		print __FUNCTION__. ": $query<P>";
	$result = mysqli_query ($LINK, $query);
	if ($returnarray && $result)
		$result = mysqli_fetch_array($result, MYSQLI_ASSOC);
	return $result;
}

function UpdateOrder ($pairs, $orderID) {
	global $msg, $error;
	global $LINK;

	$pairs = FixCheckboxFields ($pairs);
	$pairs = StripNonExistantFields (PP_ORDERS_DB, $pairs);
	$update = makeUpdate($pairs, PP_ORDERS_ID);
	$query = "UPDATE " . PP_ORDERS_DB . " SET $update where " . PP_ORDERS_ID ." = '$orderID' ";
	$result = mysqli_query ($LINK, $query);
	$error .= mysqli_error ();
	return $result;
}

/*
A package is an array of database changes, each of which is an array containing the table to be modified, the command (add, modify, delete), and columns and values if adding/modifying.

Example:
$pkg = 	array ( 
		array (	"command"		=> "modify",
				"table"			=> 'Artists',
				"UserLevel"		=> 2,
				"AccessLevel"		=> 3,
				"SubscriptionID"		=> 0,
				"SubscriptionDesc"	=> "Trial account",
				"Storage"		=> 5)),
		array (	"command"		=> "add",
				"table"			=> 'Groups',
				"UserLevel"		=> 2,
				"AccessLevel"		=> 3,
				"SubscriptionID"		=> 0,
				"SubscriptionDesc"	=> "Trial account",
				"Storage"		=> 5))
);
*/

function AddPackageToUser ($packages, $ID) {
	global $error, $msg;
	
	foreach ($packages as $pkg) {
		$command = $pkg['command'];
		$table = $pkg['command'];
		$table || $table = PP_DB_USERS;
		$pairs = $pkg;
		$pairs['ID'] = $ID;
		unset ($pairs['table']);
		unset ($pairs['command']);
		switch ($command) {
			case 'add' :
				$res = AddRecord ($table, $pairs);
				break;
			case 'delete' :
				$where = "ID = $ID";
				$res = DeleteRow ($table, $where);
				break;
			default :
				// default is modify
				$res = UpdateRecord ($table, $pairs, $ID, PP_USERS_ID );
				break;
		}
		$ps = print_r($pairs,true);
		WriteLog (__FUNCTION__.":".__LINE__ . " :: Command=$command :: $ps");
	}
	return $res;
}

// Example: UpdateRecord( 'Images', 55, array('ProjectID'=>33, "id" )
function UpdateRecord ($table, $pairs, $ID, $IDname = "ID") {
	global $msg, $error;
	global $LINK;
	
	$DEBUG = false;
	
	$pairs = FixCheckboxFields ($pairs);
	$pairs = StripNonExistantFields ($table, $pairs);
	$update = makeUpdate($pairs, $IDname);
	$query = "UPDATE $table SET $update where $IDname = '$ID' ";
	mysqli_query ($LINK, $query);
	$error .= mysqli_error ();
	return $result;
}


// --------------------
// param1 = table
// param2 = mysql WHERE clause, without the 'WHERE'
function DeleteRow( $table, $where ) {
	$DEBUG = false;
	if ($DEBUG) {
		$error .= "DeleteRow: delete from $table where $where<BR>";
		$error .= "select * from $table where $where<BR>";
		$error .= ResultToTable (mysqli_query ("select * from $table where $where"));
	}
	return mysqli_query ("delete from $table where $where");
}



function DeleteOrder( $orderID ) { 
	return mysqli_query ("delete from " . PP_ORDERS_DB . " where " . PP_ORDERS_ID . " = '$orderID'");
}


// Look for fields named myfield_CHECKBOX and
// if they have a value, set the value of the real field using
// i.e. myfield.
// If add into the list myfield with value = 0
// remove the myfield_CHECKBOX field
function FixCheckBoxFields ($pairs) {
	global $msg, $error;
	$DEBUG = false;
	foreach ($pairs as $key => $value) {
		if (preg_match ('/_CHECKBOX/', $key)) {
			$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": $key : $value<BR>";
			$realkey = str_replace("_CHECKBOX", "", $key);
			// If there a match with another real field name in the pairs provided,
			// then set that field to "1", i.e. true.
			if (array_key_exists ($realkey, $pairs)) {
				$pairs[$realkey] = 1;
			}
		}
	}
	return $pairs;
}

//-------------------------------------------------
// Choose the valid field names from an array, which may contain extra values
// for other purposes than updating a database row
// $validfields is an array of valid field names
// $arr is an array of key, value pairs.
function StripNonExistantFields ($table, $arr) {
	$validfields = GetFieldList ($table);
	$arrkeys = array_keys ($arr);
	$arr2 = array();
	foreach ($arrkeys as $key) {
		if (in_array($key, $validfields)) {
			$arr2[$key] = $arr[$key];
		}
	}
	return $arr2;
}


// from the manual, how to prepare data for MYSQL
function quote_smart($value, $mysqli_link) {
	global $LINK;
	
		if (!$mysqli_link)
			$mysqli_link = $LINK;
	
		// Stripslashes
		if (get_magic_quotes_gpc()) {
				$value = stripslashes($value);
		}
		// Quote if not integer
		if (!is_numeric($value)) {
				$value = "'" . mysqli_real_escape_string($mysqli_link, $value) . "'";
		}
		return $value;
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


// private: makeUpdate($arr) makes a comma delimited list of the name/value
// pairs of $arr, in the form name='value', with special characters escaped
function makeUpdate($arr, $IDname = "ID") {
	global $ALLOWEDTAGS;
	$pairs = array();
	foreach ($arr as $key => $value) {
		if (($key != $IDname) AND (array_key_exists ($key, $arr))) {
			$pairs[] =	"$key = ". quote_smart(LimitHTLM ($value));
		}
	}
	return (join(", ", $pairs));
}


/* private: makeCols($arr) makes a comma delimited list of the keys of $arr */
// Ignore fields which begin with "X_", whice aren't in the database
function makeCols($arr) {
	$keys = array();
	reset($arr);
	foreach ($arr as $key => $_ ) {
		if (! preg_match ('/^(X_)/i', $key))
			$keys[] = $key;
	}
	return (join(",", $keys));
}

/* private: makeVals($arr) makes a comma and single-quote delimited list
* of the values of $arr, with special characters escaped */
function makeVals($arr) {
	$values = array();
	reset($arr);
	foreach ($arr as $_ => $value) {
		// We need the addslashes, or queries are fucked. 
		// I took it out once...why?
//		$values[] = "'" . $value . "'";
		$values[] = "'" . addslashes($value) . "'";
	 }
	return (join(",", $values));
}


//------------------
function StartDatabase($mysqlhost, $myDB, $myUser = MYSQLUSER, $myPassword = MYSQLPASSWORD) {
	global $msg, $error;
	
	//$myUser || $myUser = MYSQLUSER;
	//$myPassword || $myPassword = MYSQLPASSWORD;
	
	$DEBUG = 0;
	// Start up the database


	$DEBUG && print "BEGIN StartDatabase<BR>new mysqli(".MYSQLHOST.", $myUser, $myPassword)<br>";
	if ( $LINK = new mysqli($mysqlhost, $myUser, $myPassword) ) {
		// Select the DATABASE
		if ( mysqli_select_db($LINK, $myDB) ) {
			return $LINK;
		} else {
			$error .= "Could not select database $myDB<BR>";
			die("Could not select database $myDB");
		}		
	} else {
		$error .= __FUNCTION__.":Could not connect to the site...the server must be very busy.<BR>";
		die(__FUNCTION__.":Could not connect to the site...the server must be very busy.");
	}
	return false;
}


// Strip most HTML tags and CSS tags from text.
function LimitHTLM ($value) {
	global $THEME;
	global $msg, $error;

	$value = decode_entities(trim($value));
	$value = strip_tags($value, '<a> <b> <br> <dd> <dl> <dt> <em> <i> <li> <ol> <p> <strong> <u> <ul>');
	$value = preg_replace('/\Wstyle\s*=[^>]+?>/i', '>', $value);
	$value = preg_replace('/\Won[a-z]+\s*=[^>]+?>/i', '>', $value);
	return $value;
}

// Stuff taken from Drupal code
/**
 * Decode all HTML entities (including numerical ones) to regular UTF-8 bytes.
 * Double-escaped entities will only be decoded once ("&amp;lt;" becomes "&lt;", not "<").
 *
 * @param $text
 *	 The text to decode entities in.
 * @param $exclude
 *	 An array of characters which should not be decoded. For example,
 *	 array('<', '&', '"'). This affects both named and numerical entities.
 */
function decode_entities($text, $exclude = array()) {
	global $THEME;
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
 */
function _decode_entities($prefix, $codepoint, $original, &$table, &$exclude) {
	global $THEME;
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
				 . chr(0x80 | ( $codepoint			 & 0x3F));
	}
	else if ($codepoint < 0x200000) {
		$str = chr(0xF0 | ( $codepoint >> 18))
				 . chr(0x80 | (($codepoint >> 12) & 0x3F))
				 . chr(0x80 | (($codepoint >> 6)	& 0x3F))
				 . chr(0x80 | ( $codepoint				& 0x3F));
	}
	// Check for excluded characters
	if (in_array($str, $exclude)) {
		return $original;
	}
	else {
		return $str;
	}
}


?>
