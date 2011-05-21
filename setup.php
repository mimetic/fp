<?php

/*
	SETUP FOR FP GALLERY
	Requires you know the a password, which is to be found 
	in update/setup_syspass.txt. Primitive protection.

	This script:
	- Creates working folders, e.g. _user, photos, etc.
	- Create System Administrator user
	- Create 'Public' main gallery (depending on installation type)
	- Create user (depending on installation type)
	- Create user gallery (depending on installation type)
	- Create Not For Sale priceset

	- Changes the _config/fpconfig.inc file.

	These must be set for config.inc to avoid errors. It won't be used (I hope).x
	$SYSTEMNAME 	= "frontline_photos";	
	define("SYSTEMNAME", $SYSTEMNAME);
	
	$SYSTEM_DISPLAY_NAME = "FP SETUP";	// text to show
	define("FP_SYSTEM_DISPLAY_NAME", $SYSTEM_DISPLAY_NAME);
	
	$FP_GALLERY_TYPE = FP_SINGLE_GALLERY_SINGLE_USER;
	define ("FP_GALLERY_TYPE", $FP_GALLERY_TYPE);
	
	$CommerceEmail = "sales@mimetic.com";
	define ("FP_COMMERCE_EMAIL", $CommerceEmail);
*/

include "_config/sysconfig.inc";
include "_config/fpconfig.inc";
include "_config/config.inc";
include "includes/functions.inc";
include "includes/project_management.inc";
include "includes/image_management.inc";
include "includes/commerce.inc";

error_reporting(E_ALL ^ E_NOTICE);

$error = "";
$msg = "";
$testing = false;

session_name("fp_admin");
session_start();
$_SESSION['theme'] = ADMIN_THEME;
$_SESSION['GroupID'] = FP_ADMINISTRATOR_GROUP;

// Get page and SQL
$setupdir = FP_DIR_SETUP;

// Some basic security
$syspassEntered = trim($_REQUEST['syspass']);
$syspass = trim(file_get_contents ("$setupdir/setup_syspass.txt"));
$page = file_get_contents ("$setupdir/setup_page.txt");

// Fill in form variables from system constants already declared in config files
$slots = preg_match_all ("/\{(.*?)\}/", $page, $res);

$f = array ();
foreach ($res[1] as $v) {
	$v = ltrim($v, '{');
	$v = rtrim($v, '}');
	if (defined($v) && constant($v)) {
		$f[$v] = constant($v);
		//print "Replace constant $v with {$f[$v]}<br>";
	} else {
		if (isset($v))
			if ($$v) {
				$f[$v] = $$v;
				//print "Replace variable $v with {$f[$v]}<br>";
			}
	}
}

MYSQLUSER && $f['siteusername'] = preg_replace("/_fp/","", MYSQLUSER);

if ($syspass == $syspassEntered) {
	
	// Copy htaccess to main directory unless we're working on localhost (testing).
	if ($_SERVER['HTTP_HOST'] != "localhost") {
		$res = copy ("$BASEDIR/$setupdir/files/htaccess.txt", "$BASEDIR/.htaccess");
		$res ? $msg .= "copy $BASEDIR/$setupdir/files/htaccess.txt --> $BASEDIR/.htaccess<br>" : $msg .= "*** Failed to copy $BASEDIR/$setupdir/files/htaccess.txt --> $BASEDIR/.htaccess<br>";
	}
	// Create all photo directories if necessary and copy in protection files
	ConfirmSetup ();
	
	// SQL to create database tables
	$sql = trim(file_get_contents ("$setupdir/fp.sql"));
	

	$vars = array ();
	$vars = StripBlankFields ($_REQUEST);

	// Get database, mysql user name, mysql user password
	if ($_REQUEST["siteusername"]) {
		$username = $_REQUEST["siteusername"];
		$db = $username."_fp";
		$user = $username."_fp";
	} else {
		$username = str_replace ("_fp", "", $f['MYSQLDB']);
		$db = MYSQLDB;
		$user = MYSQLUSER;
	}
	
	$_REQUEST["MYSQLPASSWORD"] ? $password = $_REQUEST["MYSQLPASSWORD"] : $password = MYSQLPASSWORD ;

	$vars['FP_HOME_REALPATH'] = "/home/$username";
	$vars['MYSQLDB'] = $db;
	$vars['MYSQLUSER'] = $user;
	$vars['MYSQLPASSWORD'] = $password;

	// Try to get system values from Admin user in database
	$a = GetAdminUserValues (MYSQLHOST, $db, $user, $password);
	$a || $a = array ();
	
	// Overwrite config file values we got with database values
	while (list ($k,$v) = each ($a)) {
		$v && $f[$k] = $v;
	}
	

	// Merge in user-entered values to overwrite existing form/database values
	$f = StripBlankFields ($f);
	$f = array_merge ($f, StripBlankFields ($vars));
	$f = array_merge ($f, StripBlankFields ($_REQUEST));

	$extrahtml = "";
	$extraline  = '';
	
	FP_USE_PAYPAL_SANDBOX ? $sandbox = "true" : $sandbox = "false";
	
	$f['DEVELOPING'] = OptionListFromArray (
		array ("true"=>"true", "false"=>"false"), 
		"DEVELOPING", 
		$f['DEVELOPING'], 
		TRUE,
		OPTION_LIST_IS_POPUP,
		$extrahtml,
		$extraline
		);
	
	$f['FP_SIGNUP_OPEN'] = OptionListFromArray (
		array ("true"=>"true", "false"=>"false"), 
		"FP_SIGNUP_OPEN", 
		$f['FP_SIGNUP_OPEN'], 
		TRUE,
		OPTION_LIST_IS_POPUP,
		$extrahtml,
		$extraline
		);
	
	$f['FP_USE_PAYPAL_SANDBOX'] = OptionListFromArray (
		array ("true"=>"true", "false"=>"false"), 
		"FP_USE_PAYPAL_SANDBOX", 
		$f['FP_USE_PAYPAL_SANDBOX'], 
		TRUE,
		OPTION_LIST_IS_POPUP,
		$extrahtml,
		$extraline
		);

	$f['FP_GALLERY_TYPE'] = OptionListFromArray (
		$FP_GALLERY_TYPES,
		"FP_GALLERY_TYPE", 
		$f['FP_GALLERY_TYPE'], 
		TRUE,
		OPTION_LIST_IS_POPUP,
		$extrahtml,
		$extraline
		);
	
		$f['FP_INSTALLATION_TYPE'] = OptionListFromArray (
		$FP_INSTALLATION_CONSTANT_NAMES, 
		"FP_INSTALLATION_TYPE", 
		$f['FP_INSTALLATION_TYPE'], 
		TRUE,
		OPTION_LIST_IS_POPUP,
		$extrahtml,
		$extraline
		);

	// Admin user access setting: UserLevel
	$f['USER_GALLERY_TYPE'] = OptionListFromArray (
		$FP_GALLERY_TYPES, 
		"USER_GALLERY_TYPE", 
//		$FP_GALLERY_TYPES[$f['UserLevel']],
		$f['UserLevel'],
		TRUE,
		OPTION_LIST_IS_POPUP,
		$extrahtml,
		$extraline
		);

//print ArrayToTable ($f);

	// Restore user-entered values and database values
	$page = Substitutions ($page, $f);
	$page = ReplaceVocabulary ($page);
	
	// clean it up
	$replace="_";
	$pattern="/([[:alnum:]_\.-]*)/";
	$vars['SYSTEMNAME']=str_replace(str_split(preg_replace($pattern,$replace,$vars['SYSTEMNAME'])),$replace,$vars['SYSTEMNAME']);
	
	$vars['SYSTEM_DISPLAY_NAME'] || $vars['SYSTEM_DISPLAY_NAME'] = mb_convert_case(str_replace ("_", " ", $vars['SYSTEMNAME']), MB_CASE_TITLE);
	if (!$testing) {
		$action = $_REQUEST['action'];
		$action == "makedb" && $msg .= "Database will be modified<br>";
		$msg .= "<h4>Action is '$action'</h4>";
		switch ($action) {
			case 'makedb' :
				if ($username && $password && $sql) {
					define ("MYSQLHOST", "localhost");				
					$result = DBMakeTables (MYSQLHOST, $db, $user, $password, $sql);
					if ($makeDB || !$result) {
						$msg .= "The database was reset and filled in with default values.<BR>";
						DBFillIn (MYSQLHOST, $db, $user, $password, $vars);
	
						if ($db && $username && $password) {
							define ("MYSQLHOST", "localhost");
							UpdateAdminUser ('localhost', $db, $user, $password, $vars);
						} else {
							define ("MYSQLHOST", '');
						}
		
	
					} else {
						$error .= $result;
					}
				} else {
					$error .= "Error: Please enter the domain username(same as FTP username) and MySQL password<BR>";
				}
				break;
		
			case 'updateconfig' :
				ConfigUpdateFile (MYSQLHOST, $db, $user, $password, $vars);
				break;
			
			case 'makeuserdirs' :
				// Create needed user directories, show results
				MakeDirectoriesFromFPConstants (true);
				ProtectPhotoDirectories ($setupdir);
				// install 'missing' pictures into gallery directories
				InstallMissingPicture (FP_SYSTEM_MISSING_PICTURE);
				$msg .= "User directories created/updated.<BR>";
				break;
			default :
				break;
		}
		
	} else {
		$msg .= "TESTING...NOTHING ACTUALLY CHANGED.<BR>";
	}

} else {
	if ($syspassEntered) {
		$error .= "WRONG PASSWORD: Please enter the system password for running Setup.<br>";
	} else {
		$error .= "Please enter the system password for running Setup.<br>";
	}
}

$error && $error = "<div style=\"color:red; background-color:yellow; padding:10px;border:1px solid black;\"><i>{$error}</i></div>";

$page = str_replace ("{error}", $error, $page);
$page = str_replace ("{msg}", $msg, $page);
$page = DeleteUnusedSnippets ($page);

print $page;







// ===================================
// Reads an SQL dump file and executes in MYSQL one line at a time
function DBMakeTables ($host, $db, $user, $password, $sql_query) {
	global $error, $msg;

	$DEBUG = 0;	
	$result = null;
	
	$msg .= __FUNCTION__.": Trying to make tables in $db<BR>";
	$LINK = @DBStart($host, $db, $user, $password);
	if ($LINK) {
		$DEBUG && $msg .= $msg .= __FUNCTION__.": Link established to $db<BR>";
		$delimiter = ';'; 
		$sql_query = trim ($sql_query);
		$sql_query = remove_remarks($sql_query); 
		$sql_query = split_sql_file($sql_query, $delimiter);
		$x = 1;
		foreach($sql_query as $query){
			mysql_query($query);
			$DEBUG && $msg .= $x++.") ". $query."<BR>";
			if (mysql_errno($LINK)) {
				$error .= $x++.") ". substr($query, 0,50)."..."."<BR>" . mysql_errno($LINK) . ": " . mysql_error($LINK). "<br>\n";
				$result = mysql_errno($LINK);
			}
		} 
		$msg .= 'Database Successfully populated<br>';
	} else {
		$error .= "Could not start database<BR>";
		$result = mysql_errno($LINK);
	}
	return $result;
}

function ConfigUpdateFile ($host, $db, $user, $password, $vars) {
	global $msg, $error, $function;
	
	$DEBUG = 0;	

	$configfile = trim (file_get_contents ("_config/fpconfig.inc"));
	$unchanged = $configfile;
	
	
	// set the $FP_ACCOUNT_NAME;
	if ($vars['siteusername']) {
		$search[]  = '/(\$FP_ACCOUNT_NAME\s*=\s*").*?("\s*;)/i';
		$replace[] = '${1}'.$vars['siteusername'].'${2}';
		$config_msg .= "Updated <i>\$FP_ACCOUNT_NAME</i> with {$vars['siteusername']}<BR>";
	}

	// set the $FP_DOMAIN_NAME;
	if ($vars['FP_DOMAIN_NAME']) {
		$search[]  = '/(\$FP_DOMAIN_NAME\s*=\s*").*?("\s*;)/i';
		$replace[] = '${1}'.$vars['FP_DOMAIN_NAME'].'${2}';
		$config_msg .= "Updated <i>\$FP_DOMAIN_NAME</i> with {$vars['FP_DOMAIN_NAME']}<BR>";
	}

	// set the host
	if ($host) {
		$search[]  = '/(define\s*\("MYSQLHOST",\s*")\w+("\)\s*;)/i';
		$replace[] = '${1}'.$host.'${2}';
		$config_msg .= "Updated <i>define(MYSQLHOST)</i> with $host<BR>";
	}

	// set the db
	if ($db) {
		$search[]  = '/(define\s*\(\"MYSQLDB",\s*")\w+("\);)/i';
		$replace[] = '${1}'.$db.'${2}';
		$config_msg .= "Updated <i>define(MYSQLDB)</i> with $db<BR>";
	}

	// set the user
	if ($user) {
		$search[]  = '/(define\s*\(\"MYSQLUSER",\s*")\w+("\);)/i';
		$replace[] = '${1}'.$user.'${2}';
		$config_msg .= "Updated <i>define(MYSQLUSER)</i> with $user<BR>";
	}

	// set the password
	if ($password) {
		$search[]  = '/(define\s*\(\"MYSQLPASSWORD",\s*")\S+?("\);)/i';
		$replace[] = '${1}'.$password.'${2}';
		$config_msg .= "Updated <i>define(MYSQLPASSWORD)</i> with $password<BR>";
	}

	// set the $FP_SYSTEM_EMAIL;
	if ($vars['FP_SYSTEM_EMAIL']) {
		$search[]  = '/(\$FP_SYSTEM_EMAIL\s*=\s*").*?("\s*;)/i';
		$replace[] = '${1}'.$vars['FP_SYSTEM_EMAIL'].'${2}';
		$config_msg .= "Updated <i>\$FP_SYSTEM_EMAIL</i> with {$vars['FP_SYSTEM_EMAIL']}<BR>";
	}

	// set the $FP_INFO_EMAIL;
	if ($vars['FP_INFO_EMAIL']) {
		$search[]  = '/(\$FP_INFO_EMAIL\s*=\s*").*?("\s*;)/i';
		$replace[] = '${1}'.$vars['FP_INFO_EMAIL'].'${2}';
		$config_msg .= "Updated <i>\$FP_INFO_EMAIL</i> with {$vars['FP_INFO_EMAIL']}<BR>";
	}

	// set the $FP_INFO_EMAIL;
	if ($vars['FP_COMMERCE_EMAIL']) {
		$search[]  = '/(\$FP_COMMERCE_EMAIL\s*=\s*").*?("\s*;)/i';
		$replace[] = '${1}'.$vars['FP_COMMERCE_EMAIL'].'${2}';
		$config_msg .= "Updated <i>\$FP_COMMERCE_EMAIL</i> with {$vars['FP_COMMERCE_EMAIL']}<BR>";
	}

	// set the $FP_HOME_REALPATH;
	if ($vars['FP_HOME_REALPATH']) {
		$search[]  = '/(\$FP_HOME_REALPATH\s*=\s*").*?("\s*;)/i';
		$replace[] = '${1}'.$vars['FP_HOME_REALPATH'].'${2}';
		$config_msg .= "Updated <i>\$FP_HOME_REALPATH</i> with {$vars['FP_HOME_REALPATH']}<BR>";
	}

	// set the $SYSTEMNAME;
	if ($vars['SYSTEMNAME']) {
		$search[]  = '/(\$SYSTEMNAME\s*=\s*").*?("\s*;)/i';
		$replace[] = '${1}'.$vars['SYSTEMNAME'].'${2}';
		$config_msg .= "Updated <i>\$SYSTEMNAME</i> with {$vars['SYSTEMNAME']}<BR>";
	}

	// set the $SYSTEM_DISPLAY_NAME;
	if ($vars['SYSTEM_DISPLAY_NAME']) {
		$search[]  = '/(\$SYSTEM_DISPLAY_NAME\s*=\s*").*?("\s*;)/i';
		$replace[] = '${1}'.$vars['SYSTEM_DISPLAY_NAME'].'${2}';
		$config_msg .= "Updated <i>\$SYSTEM_DISPLAY_NAME</i> with {$vars['SYSTEM_DISPLAY_NAME']}<BR>";
	}

	// set the $FP_GOOGLE_ANALYTICS_CODE;
	if ($vars['FP_GOOGLE_ANALYTICS_CODE']) {
		$search[]  = '/(define\s*\(\"FP_GOOGLE_ANALYTICS_CODE",\s*").*?("\);)/i';
		$replace[] = '${1}'.$vars['FP_GOOGLE_ANALYTICS_CODE'].'${2}';
		$config_msg .= "Updated <i>define(FP_GOOGLE_ANALYTICS_CODE)</i> with {$vars['FP_GOOGLE_ANALYTICS_CODE']}<BR>";
	}

	// set the $FP_USE_PAYPAL_SANDBOX;
	if ($vars['DEVELOPING']) {
		$search[]  = '/(\$DEVELOPING\s*=\s*)(true|false|1|0)(\s*;)/i';
		$replace[] = '${1}'.$vars['DEVELOPING'].'$3';
		$config_msg .= "Updated <i>\$DEVELOPING</i> with {$vars['DEVELOPING']}<BR>";
	}

	// set the $FP_SIGNUP_OPEN;
	if ($vars['FP_SIGNUP_OPEN']) {
		$search[]  = '/(\$FP_SIGNUP_OPEN\s*=\s*)(true|false|1|0)(\s*;)/i';
		$replace[] = '${1}'.$vars['FP_SIGNUP_OPEN'].'$3';
		$config_msg .= "Updated <i>\$FP_SIGNUP_OPEN</i> with {$vars['FP_SIGNUP_OPEN']}<BR>";
	}

	// set the $FP_SIGNUP_CODE;
	if ($vars['FP_SIGNUP_CODE']) {
		$search[]  = '/(define\s*\(\"FP_SIGNUP_CODE",\s*").*?("\);)/i';
		$replace[] = '${1}'.$vars['FP_SIGNUP_CODE'].'${2}';
		$config_msg .= "Updated <i>define(FP_SIGNUP_CODE)</i> with {$vars['FP_SIGNUP_CODE']}<BR>";
	}

	// set the $FP_USE_PAYPAL_SANDBOX;
	if ($vars['FP_USE_PAYPAL_SANDBOX']) {
		$search[]  = '/(\$FP_USE_PAYPAL_SANDBOX\s*=\s*)(true|false|1|0)(\s*;)/i';
		$replace[] = '${1}'.$vars['FP_USE_PAYPAL_SANDBOX'].'$3';
		$config_msg .= "Updated <i>\$FP_USE_PAYPAL_SANDBOX</i> with {$vars['FP_USE_PAYPAL_SANDBOX']}<BR>";
	}

	// set the $FP_GALLERY_TYPE;
	if ($vars['FP_GALLERY_TYPE']) {
		$search[]  = '/(\$FP_GALLERY_TYPE\s*=\s*).*?(\s*;)/i';
		$replace[] = '${1}'.$vars['FP_GALLERY_TYPE'].'${2}';
		$config_msg .= "Updated <i>\$FP_GALLERY_TYPE</i> with {$vars['FP_GALLERY_TYPE']}<BR>";
	}

	// set the $FP_INSTALLATION_TYPE;
	if ($vars['FP_INSTALLATION_TYPE']) {
		$search[]  = '/(\$FP_INSTALLATION_TYPE\s*=\s*).*?(\s*;)/i';
		$replace[] = '${1}'.$vars['FP_INSTALLATION_TYPE'].'${2}';
		$config_msg .= "Updated <i>\$FP_INSTALLATION_TYPE</i> with {$vars['FP_INSTALLATION_TYPE']}<BR>";
	}

	$configfile = preg_replace ($search, $replace, $configfile);
	
	if ($DEBUG) {
		print "<h2>Config File</h2>";
		print str_replace ("\n", "<br>", htmlentities ($configfile));
		print "<hr>";
	} else if ($configfile != $unchanged) {
		// if we made a change, write the file
		if (file_put_contents ("_config/fpconfig.inc", $configfile)) {
			$msg .= $config_msg;
		} else {
			$error .= __FUNCTION__.": Could not write $configfile<br>";
		}
	} else {
		$msg .= "Nothing changed, so no changes were made to the config file.<br>";
	}
}

// ----------------
// Change values of Admin user.
// 
function UpdateAdminUser ($host, $db, $user, $password, $vars) {
	global $error, $msg;
	global $FP_MYSQL_LINK;

	$LINK = @DBStart($host, $db, $user, $password);
	if ($LINK) {
		// Create Admin user
		$pairs = array (	
				'Username'			=> "Administrator",
				'AccessLevel'		=> FP_ADMINISTRATOR_LEVEL,
				'UserLevel'			=> $vars['USER_GALLERY_TYPE'],
				'Firstname'			=> "System",
				'Lastname'			=> "Administrator",
				'Password'			=> $vars['Password'],
				'Password_Reminder'	=> $vars['Password_Reminder'],
				'Confirmed'			=> '1',
				'Storage'			=> $vars['Storage'],
				'Email'				=> $vars['FP_SYSTEM_EMAIL'],
				'Commission'		=> 0
				);
								
		$pairs = StripBlankFields ($pairs);
		$myUser = new FPUser(FP_ADMINISTRATOR);
		$myUser->UpdateUser ($pairs);
		mysql_close($LINK);
		// $FP_MYSQL_LINK->close();
	}
}


// ----------------
// Get values of Admin user.
// 
function GetAdminUserValues ($host, $db, $user, $password) {
	global $error, $msg;
	
	$record = array ();
	
	$LINK = @DBStart($host, $db, $user, $password);
	if ($LINK) {
		$record = FetchArtist( FP_ADMINISTRATOR );
		mysql_close($LINK);
		// $FP_MYSQL_LINK->close();
	} else {
		$msg .= "$host/$db/$user/$password don't work to open a database.<BR>";
	}
	return $record;
}



// ----------------
/*
	Fill important DB entries
	 - create admin user
	
	Depending on installation type:
		Public Installation
			- create 'public' main gallery, owned by sys admin
		Private
			- create main user
			- create gallery for main user
			

	- run Maintenance (creates admin gallery and other useful things)
	- create a "Not for sale" price set
*/
function DBFillIn ($host, $db, $user, $password, $vars) {
	global $error, $msg;

	$LINK = @DBStart($host, $db, $user, $password);
	
	CreateAdminUser ($vars);
	CreateNotForSalePrice ();
	
	if ($vars['FP_INSTALLATION_TYPE'] == FP_INSTALLATION_PUBLIC) {
		// PUBLIC INSTALL
		AddMainGallery(FP_ADMINISTRATOR);
	
	} else {
		// PRIVATE INSTALL
		$id = AddMainUser ($vars);
		$title = trim ($vars['Firstname'] . " " . $vars['Lastname']);
		AddMainGallery($id, $title);
		
	}

	Maintenance ();
	
	mysql_close($LINK);
	// $FP_MYSQL_LINK->close();
}

// Create 'Not for Sale' price set
function CreateNotForSalePrice () {
	global $msg, $error;
	
	$pairs = array (	
			'ID'			=> 1,
			'Title'		=> "(Not For Sale)"
			);
	$newID = AddRecord( DB_PRICESETS, $pairs );
	$newID ? $msg .= "Added price set (not for sale)<br>" : $error .= "Could not add price set (not for sale)<br>";
}

function CreateAdminUser ($vars) {
	global $msg, $error;
	
	// Create Admin user
	$pairs = array (	
			'ID'				=> FP_ADMINISTRATOR,
			'Username'			=> "Administrator",
			'AccessLevel'		=> FP_ADMINISTRATOR_LEVEL,
			'UserLevel'		=> FP_SINGLE_GALLERY_SINGLE_USER,
			'Firstname'		=> "System",
			'Lastname'			=> "Administrator",
			'Password'			=> $vars['Password'],
			'Password_Reminder'	=> $vars['Password_Reminder'],
			'Confirmed'		=> '1',
			'Storage'			=> 	$vars['Storage'],
			'Email'			=>  $vars['FP_SYSTEM_EMAIL'],
			'Commission'		=> 0

			);
	
	$newID = AddRecord( DB_ARTISTS, $pairs );
	if ($newID) {
		$q = "UPDATE ".DB_ARTISTS." SET `ID` = " . FP_ADMINISTRATOR . " WHERE `ID` = $newID";
		mysql_query($q);
		if (($newID != 1) && mysql_affected_rows () == 0 )
			$msg .= __FUNCTION__.": *** Failed to set Administrator to ID=".FP_ADMINISTRATOR."<br>";
	}
	$newID ? $msg .= "Created System Administrator<br>" : $error .= "Could not add System Administrator to ".DB_ARTISTS."<br>";
}

// Create the first user (not admin user)
function AddMainUser ($vars) {
	global $msg, $error;

	// Create user
	//$username = strtolower ($vars['Firstname'].$vars['Lastname']);
	$username = $vars['Email'];
	$pairs = array (	
			'ID'				=> 2,
			'Username'			=> $username,
			'AccessLevel'		=> FP_NORMAL_LEVEL,
			'UserLevel'		=> $vars['USER_GALLERY_TYPE'],
			'Firstname'		=> $vars['Firstname'],
			'Lastname'			=> $vars['Lastname'],
			'Password'			=> $vars['Password'],
			'Password_Reminder'	=> 'System password',
			'Confirmed'		=> '1',
			'Email'			=> $vars['Email'],
			'Storage'			=> $vars['Storage'],
			'Commission'		=> 0
			
			);
	// Param 04 = Link to galleries. Depends on single/multi gallery
	
	$params = array ();
	$params = SetParam ($params, FP_PARAM_ARTIST_GALLERY_LINK, 0);
	$pairs['Params'] = $params;
	
	$myUser = new FPUser();
	$newID = $myUser->newUser($pairs, FP_NEW_ARTIST_GALLERY);

	// $newID = AddRecord( DB_ARTISTS, $pairs );
	$newID ? $msg .= "Created user '$username'<br>" : $error .= "Could not add System Administrator to ".DB_ARTISTS."<br>";
	return $newID;
}


//------------------
// Install "missing picture"
function InstallMissingPicture ($fn) {
	global $error, $msg;
	global $setupdir, $BASEDIR;

	copy ("$BASEDIR/$setupdir/$fn", "$BASEDIR/".FP_SYSTEM_IMAGES. "$fn") || $error .= "Failed to copy from $BASEDIR/$setupdir/$fn to $BASEDIR/".FP_SYSTEM_IMAGES. "$fn<br>";
	$sysprojdir = "$BASEDIR/" . FP_DIR_MAILED_DIR . "/1";
	file_exists ($sysprojdir) || mkdir ($sysprojdir, 0777) || $error .= "Failed to make directory $sysprojdir<BR>";

	copy ("$BASEDIR/$setupdir/$fn", $sysprojdir . "/$fn");

	copy ("$BASEDIR/$setupdir/$fn", $BASEDIR.'/'.FP_DIR_TMP . "/$fn");

	$msg .= "Importing 'missing picture' graphic as system admin picture (file name is $fn)<BR>";
	
	// This will resize the 'missing picture' image to the appropriate sizes, then move it into the 
	// picture directories.
	ProcessOneImage ( $BASEDIR.'/'.FP_DIR_TMP . "/$fn", "image/jpeg",  FP_DIR_TMP."/$fn", false, true, true);
	MoveProcessedToMain (FP_SYSTEM_MISSING_PICTURE);
	$msg .= "Added 'missing picture'.<br>";

	unlink ($sysprojdir . "/$fn");
	$msg .= "Removed 'missing picture' from the system admin picture list.<br>";
}


//------------------
function DBStart($host, $db, $user, $password) {
	global $error;
	$DEBUG = TRUE;

	$DEBUG && $msg .= "Connect to $user@$db:$password on $host<BR>";
	$LINK = mysql_connect($host, $user, $password)
		or $error .= "Could not connect to the site ($host, $user, $password)...the server must be very busy.";

	if ($DEBUG)
		$msg .= "Connected successfully to $db<BR>";
	
	// Select the DATABASE
	mysql_select_db("$db")
		or $error .= "Could not select database $db";
	
	return $LINK;
}




// remove_remarks will strip the sql comment lines out of an uploaded sql file  
function remove_remarks($sql){ 
	$sql = preg_replace('/\n{2,}/', "\n", preg_replace('/^[-].*$/m', "\n", $sql)); 
	$sql = preg_replace('/\n{2,}/', "\n", preg_replace('/^#.*$/m', "\n", $sql)); 
	return $sql; 
} 

// split_sql_file will split an uploaded sql file into single sql statements. 
// Note: expects trim() to have already been run on $sql. 
function split_sql_file($sql, $delimiter){ 
	$sql = str_replace("\r" , '', $sql); 
	$data = preg_split('/' . preg_quote($delimiter, '/') . '$/m', $sql); 
	$data = array_map('trim', $data); 
	// The empty case 
	$end_data = end($data); 
	if (empty($end_data)) 
	{ 
		unset($data[key($data)]); 
	} 
	return $data; 
}  
?>