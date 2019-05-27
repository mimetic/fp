<?php

$error = "";
$msg = "";

require_once "_config/sysconfig.inc";
require_once "_config/fpconfig.inc";
require_once "_config/config.inc";
require_once "includes/functions.inc";
require_once "includes/project_management.inc";
require_once "includes/image_management.inc";
require_once "includes/commerce.inc";


// report all errors except 'Notice', which is for undefined vars, etc. 
// All the isset clauses are to avoid 'notice' errors. 
error_reporting(E_ALL ^ E_NOTICE);

$LINK = StartDatabase(MYSQLDB);
Setup ();

$ADMINFILENAME = basename (__FILE__);

// set the umask, so setting permissions work properly
// umask 022 should give us 644. Extra zero means octal.
$old_umask = umask(0022); 

// Quick maintenance check that all directories, etc. are set up
ConfirmSetup();

// Get form input
$results = GetFormInput();
$actions = $results['actions'];
$action = $results['actions']['action'];
$vars = $results['vars'];

// make $variables from $vars.
// Do this at beginning; anything crucial we check by hand, right?
// DANGEROUS! DO WE NEED THIS?
/*
foreach ($vars as $k => $v) {
	$$k = $v;
}
*/

// Start Session
// Start before getting user, since FPUser looks in the session for the user
session_name("fp_admin");
session_start();

$DEBUG = false;

$user = new FPUser( $LINK );
isset($_SESSION['fp_user']) ? $fp_user = $_SESSION['fp_user'] : $fp_user = null;
$fp_user && $user->loadUser($fp_user);
$user->is_loaded() || $fp_user = null;

// ------------------
if ( $actions['action'] == 'logout') {
	$user->logout();
	//$user = new FPUser ($LINK);
	$fp_user = null;
}


// ------------------
// Must be set right away so page will draw properly (getting snippets from admin)
$_SESSION['theme'] = ADMIN_THEME;
$_SESSION['GroupID'] = FP_ADMINISTRATOR_GROUP;

// Activate?
if ($actions['action'] == "activate") {
	if (isset ($_GET["hash"])) {
		$error .= "Activation attempted, but no activate code was sent!<BR>";
	} else {
		//This is the actual activation. User got the email and clicked on the special link we gave him/her
		$hash = $user->escape($_GET['hash']);
		$activation = $user->activateByHash ($hash);
		!$user->error 
			? $msg .= "You have successfully activated your account.<br>"
			: $error .= "Error: {$user->error}<br>";
	}
}

if ( !$user->is_loaded()) {
	// Lost Password Request?
	if ($actions['action'] == "sendpassword") {
		$res = $user->sendPasswordToUser($_POST['login']);
		$msg .= $res;
		$fp_user = null;
		$user->unload();
	} else {
		//Login stuff:
		if ( isset($_POST['login']) && isset($_POST['passwd'])){
			//Mention that we don't have to use addslashes as the class do the job
			if ( !$user->login($_POST['login'],$_POST['passwd'],$_POST['remember'] )) {
				$error .= "Sorry, I don't know the username and/or password you entered. Perhaps you typed one of them incorrectly?<br>";
				//$user->logout('http://'.$_SERVER['HTTP_HOST'].$_SERVER['PHP_SELF']);
				$actions['action'] = "logout";
				$fp_user = null;
			} else {
				$_SESSION['StartTime'] = time();
				$fp_user = $user->userID;
	
				//$msg .="Successful login for user {$user->userID} : {$_POST['login']}.<BR>";
				//user is now loaded
			}
		} else {
			// no login attempt
		}
	}
} else {
 	//User is loaded
	$fp_user = $user->userID;
}


// ------------------
$DEBUG && $msg .= __LINE__. ": fp_user = $fp_user, Session fp_user: {$_SESSION['fp_user']}<br>";

// Login returns a user or false (for failure)
if ($fp_user) {

	// ========= clear the website page caches ==================
	// Assume that any change in Admin could change a page. Don't risk it,
	// Clear all the caches for this website. The cache is now in tmp, locally,
	// so we're not trashing cache for all users on the server
	// However, we still have a problem with a multi-user system:
	// we're clearing all cache... very inefficient!!!!
	require_once('Cache/Lite.php');
	
	// Set a few options
	$options = array(
		'cacheDir' => 'tmp/cache/'
	);
	
	$Cache_Lite = new Cache_Lite($options);
	$Cache_Lite->clean();
	unset ($Cache_Lite);
	
	// ===============================================================

	$_SESSION['fp_user'] = $fp_user;
	//$msg .= "Username and password accepted.<BR>";
	
	$accessLevel = FetchAccessLevel ($fp_user);		

	isset($vars['PageID']) ? $pageid = $vars['PageID'] : $pageid = "tab12";

	$actions['table'] || $actions['table'] = DB_PROJECTS;
	$table = $actions['table'];

	// Reload the Themes and variations to get the correct variations from the user files
	$Themes->userID = $fp_user;
	$Themes->LoadAllThemes(true);

	// =========== NO PRINTING BEFORE HERE...OR COOKIES FAIL HORRIBLY ===============
	// Get User Admin Level
	// Restrict access to users list if not level 1
	//$user = GetRecord("Artists", $fp_user);

	$user->loadUser($fp_user);
	
	$DEBUG && $msg .= __LINE__. ": fp_user = $fp_user, Session fp_user: {$_SESSION['fp_user']}<br>";


	$UserFullname = $user->Fullname;
	$accessLevel = null;
	$user->getval("AccessLevel") && $accessLevel = $user->getval("AccessLevel");
	
	// =============== MAINTENANCE =====================

	Maintenance ();
	
	// No need to do this each time if the setup is any good at all.
	// BUT, it's handy if we want to keep making changes...
	MakeDirectoriesFromFPConstants (true);	//true=show changes
	
	// =============== IMAGE PROCESSING =====================
	
	// This is a snippet of code to show if there are pictures still awaiting processing
	// It lets user click to process some more pictures.
	$pixwaiting = "<!-- no pictures waiting to process -->";	// this is a 'click to process' code
	// this is REFRESH META code, so the page automatically reloads, thereby processing pictures
	$refresh = "<!--no refresh set-->";		
	
	$projectID = "";
	$table == $PROJECTS && $projectID = $vars['ID'];
	$newimages = false;

	// process uploaded HTML images, for artist portrait or logos
	$newimages = ProcessUploadedImages ($projectID, $fp_user);

	// DON'T PROCESS EACH TIME WE CALL ADMIN. TOO SLOW. LET MAINTENANCE DO THIS, OR USER CAN DO IT FROM THE PICTURES UPLOAD PAGE
	// Only process upon request. Maintenance will do it for you over time.
	// If there are pictures to put in the system from the processed folders, e.g. from a file upload just performed, process them.
	if ($_REQUEST['processuploadedpix'])
		$newimages = ProcessLocalImages ();
	
	(!$pageid && ($_SESSION['PageID'])) && ($pageid = $_SESSION['PageID']);

} else {
	// If we don't get a userID, log out for security!
	$actions['action'] = "logout";
	$pageid = "nomenu";
}

// =============== ACTION PROCESSING =====================

$picturemgmt = "";
$noaccess = "";

$text = FetchSnippet ("main", ADMIN_THEME);
$form = HandleAction ($actions, $vars, $fp_user);

// $vars may have been updated by HandleAction
// To be conservative, only get $ID if reset by HandleAction, as when a new record is created
$ID = $vars['ID'];

$pageid != "nomenu" 
	? $menu = FetchSnippet ("menu" . $accessLevel, ADMIN_THEME)
	: $menu = "";
//$pageid || $pageid = "tab1";


// if user has free account, show link to update the account
if ($user->is_loaded()) {
	$subscribe_menu_item = GetSubscribeMenuItem ($user);
	$menu = str_replace("{SUBSCRIBE}", $subscribe_menu_item, $menu);
}

// Replace codes in text
$f = array ();

// If private account, no signups
FP_INSTALLATION_TYPE != FP_INSTALLATION_PRIVATE
	? $f['SIGNUP_TRIAL_ACCOUNT'] = FetchSnippet ('signup_trial_account')
	: $f['SIGNUP_TRIAL_ACCOUNT'] = '<!-- no signups allowed -->';

$f['META_INDEX'] = FetchSnippet ('meta_robots_noindex_nofollow');

				
$text = str_replace ("{pixwaiting}", $pixwaiting, $text);
$text = str_replace ("{refresh}", $refresh, $text);
$user->is_loaded() ? $userinfo = FetchSnippet ("userinfo", ADMIN_THEME) : $userinfo = "";
if ($error) {
	$error = str_replace ("{error}", $error, FetchSnippet ("errormsg", ADMIN_THEME));
}

$user->CheckStorageSpace();

$text = ReplaceSysVars ($text, $table, $fp_user, $ID, $nextaction);

// Color Error RED
$error = FormatErrorForOutput ($error);

($fp_user == FP_ADMINISTRATOR)
	? $adminwarning = FetchSnippet ("admin_user_warning")
	: $adminwarning = "";

$overstoragelimit = $user->UserOverStorageLimit ($ID);
// The order of these two substitutions actually matters,
// because the first set are blocks of text which 
// probably contain entries from the second set

// show/hide feedback code
if (!FP_DISABLE_USERVOICE_FEEDBACK) {
	$userfeedbacklink = FetchSnippet("uservoice_feedback");
} else {
	$userfeedbacklink = "";
}


// SUBSTITUTIONS SET 1
$text = Substitutions ($text,	array (	"form"		=>	$form));
										
// SUBSTITUTIONS SET 2
$text = Substitutions ($text,	array (	
	"noaccess"	=>	$noaccess,
	"error"		=>	FormatErrorForOutput($error),
	"msg"		=>	FormatMessageForOutput($msg),
	"login"		=>	$vars['login'],
	"picturemgmt"	=> 	$picturemgmt,
	"Menu"		=>	$menu
	));
	
// SUBSTITUTIONS SET 3
$sysvars = array(		
	"title"				=>	"Administration",
	"userinfo"			=>	$userinfo,
	"showthumbstatus"		=>	$showthumbstatus,
	"fp_user"			=>	$fp_user,
	"UserFullname"			=>	$UserFullname,
	"accesslevel"			=>	$accessLevel,
	"accessleveldesc"		=>	FetchAccesslevelDesc ($accessLevel),
	"session_id"			=>	SID,
	"basename"			=>	basename($PHP_SELF),
	"userUsage"			=>	$user->StorageUsed,
	"userSpace"			=>	$user->StorageAllocatedText,
	"freespace"			=>	$user->StorageFreeText,
	"overstoragelimit"		=>	$overstoragelimit,
	"javascript"			=>	FetchSnippet ("javascript", ADMIN_THEME),
	// OTCODE is only used where the OptionTransfer javascript is,
	// as on the Project Picture Management page,
	// To use it, we need to drop this code into the 'body'
	// I set the value of $OTCODE in BuildOptionTransfer
	// in functions.inc
	// otherwise, it will be empty.
	"OTCODE"				=> 	$OTCODE,
	"DONOTHING"			=> 	FetchSnippet("do_nothing"),
	"PAGEID"				=>	$pageid,
	"ADMIN_USER_WARNING"		=>	$adminwarning,
	"table"				=>	$table,
	"USER_FEEDBACK"			=>	$userfeedbacklink
	);
	
$text = Substitutions ($text, $f);
$text = Substitutions ($text, $sysvars);
$text = ReplaceAllSnippets ($text);
$text = ReplaceSysVars ($text, $table, $fp_user, $ID, $nextaction);

$text = insertBGANCost ($text);
$text = DeleteUnusedSnippets ($text);

// Unnecessary: file sizes are small as it is.
//$text = compress_html($text);

// TFU flash uploader says:
// It's best to close the session before you include the flash because depending on the page caching it is possible the the flash is loaded before the session is written to the disk (which is done at the end of the php file normally)
session_write_close();

// OUTPUT PAGE
print $text;

mysqli_close($LINK);
//$FP_MYSQL_LINK->close();

?>