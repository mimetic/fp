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

// My error logging...just add a newline after each line. Dammit.
function fp_error_log ($text, $type, $logfile) {
	error_log (date('Y-m-d H:i:s') . ": {$text}\n", $type, $logfile);
}



// SETUP
// Initialize variables and such
function Setup () {
	global $FP_ADMIN_USER, $FP_GALLERY_TYPE, $FP_CONSTANTS;
	global $FP_NAVBAR_ITEM_NAMES;
	global $error, $msg;
	
	$DEBUG = false;

	// Load the Admin user for this session
	$FP_ADMIN_USER = new FPUser (FP_ADMINISTRATOR);
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



//------------------
function StartDatabase($myDB) {
	global $DEPENDENCIES_FILE, $DEPENDENCIES;
	global $LINK, $FP_MYSQL_LINK;
	global $msg, $error;

	$DEBUG = FALSE;
	// Start up the database

	$DEBUG && print "BEGIN StartDatabase<BR>";
	$LINK = new mysqli(MYSQLHOST, MYSQLUSER, MYSQLPASSWORD, $myDB);
//			or die("Could not connect to the site...the server must be very busy.");

	if (mysqli_connect_error()) {
		 die('Connect Error (' . mysqli_connect_errno() . ') '
					. mysqli_connect_error());
	}

	// Select the DATABASE
 	mysql_select_db($myDB)
 			or die("Could not select database $myDB");

	$DEPENDENCIES = GetDependencies($DEPENDENCIES_FILE);
	$DEBUG && print "END StartDatabase<BR>";

	
	// ** Make a new object, too!
	//$FP_MYSQL_LINK = new mysqli(MYSQLHOST, MYSQLUSER, MYSQLPASSWORD, $myDB);
	
	
	if (mysqli_connect_error()) 
		{
		die('Connect Error (' . mysqli_connect_errno() . ') ' . mysqli_connect_error());
		}

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


//---------------
function GetDependencies ($DEPENDENCIES_FILE) {
	global $PICTURE_DIRECTORIES, $DATABASES;	// Arrays of names
	global $BASEDIR;
	global $msg, $error;

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
	$item = str_replace('$','\$',$item);
	$item = '/\\' . FP_FIELDSTART . $item . '\\'.FP_FIELDSUFFIX.'/i';
	return $item;
}



// Replace unmatched or unused snippets in a text
// THIS ONLY WORKS WITH {} coding.
function DeleteUnusedSnippets ($text) {
	global $msg, $error;

	// dump any {} codes
	$text = preg_replace("/(\{\w*?\})/","", $text);


	return $text;
}


// For some reason, the built-in mysql_list_fields is deprecated.
function MysqlListFields ($table) {
	$res = mysqli_query ("SHOW COLUMNS FROM `$table`");
	while ($row = mysqli_fetch_array($res))
		$col_names[]=$row[0];
	return $col_names;
}

// We need to encode and decode the raw params data from the DB
// because otherwise we get big problems with " marks and such.
function EncodeArrayForDB ($params = null) {
	global $msg, $error;

	$params = serialize($params);
	return $params;
}

function DecodeArrayFromDB ($params = null) {
	global $msg, $error;

	//$params = unserialize(base64_decode ($params));
	$params && $params = unserialize($params);
	$params || $params = array ();
	return $params;
}


?>