<?php

// SYSTEM CONFIG FILE FOR FP PICTURE GALLERY SYSTEM
// These settings don't chance across installations
//
// Localizing system settings, e.g. database, are in fpconfig.inc
// Localizing visual/functional settings are in config.inc
//
// The version must be of the form, yyyy.mm.dd-x, where x starts at 01
// Auto-Update Version 2013.07.04-07
//
// System Version 2012.10.24-01

// Report simple running errors
ini_set ("display_errors", "1");

$tz = ini_get('date.timezone');
$tz || date_default_timezone_set('America/Los_Angeles');

// REPORT EVERYTHING
error_reporting(E_ALL);

// REPORT NOTICE, TOO
//error_reporting  (E_ERROR | E_WARNING | E_PARSE | E_NOTICE); 
error_reporting(E_ERROR | E_WARNING | E_PARSE );

// Autoload classes
function __autoload($class_name) {
	$d = dirname(dirname(__FILE__)."../") . "/includes/";
	//echo ("Looking for ".$d . $class_name . '.inc');
	file_exists($d . $class_name . '.inc')
	? require_once ($d . $class_name . '.inc')
	: require_once ($d . $class_name . '.php');
}

// ================================
// Product name
// The name of the gallery system, in general. 
define ("FP_SYSTEM_NAME", "Mimetic Gallery System");


// ================================
// HTTP_HOST works when using an IP address, for example, while SERVER_NAME fails
// ** if called from the command line, there will be no server name or http_host **
if (isset ($_SERVER['HTTP_HOST'])) {
	$SERVERNAME = $_SERVER['HTTP_HOST'];
} else {
	if (isset($_SERVER['SERVER_NAME'])) {
		$SERVERNAME = $_SERVER['SERVER_NAME'];
	} else {
		$SERVERNAME = "";
	}
}
$SYSTEMURL = $SERVERNAME .  dirname ($_SERVER['PHP_SELF']) . "/";
$SYSTEMURL = preg_replace ("|//$|", "/", $SYSTEMURL);

// name of admin php script
$FP_SYSTEM_ADMIN_SCRIPT = "admin.php";
define ("FP_SYSTEM_ADMIN_SCRIPT", $FP_SYSTEM_ADMIN_SCRIPT);

// name of signup php script
$FP_SYSTEM_SIGNUP_SCRIPT = "signup.php";
define ("FP_SYSTEM_SIGNUP_SCRIPT", $FP_SYSTEM_SIGNUP_SCRIPT);

// name of gallery php script
$FP_SYSTEM_GALLERY_SCRIPT = "fp.php";
define ("FP_SYSTEM_GALLERY_SCRIPT", $FP_SYSTEM_GALLERY_SCRIPT);

// base directory of installation
$BASEDIR = "";
isset ($_SERVER['SCRIPT_FILENAME']) && ($BASEDIR = dirname($_SERVER['SCRIPT_FILENAME']));

// If called from command line (e.g. from Maintenance.php) we need
// to get BASEDIR another way. Note that __FILE__ returns the ULR of THIS file!
if (!$BASEDIR || ($BASEDIR == ".")) {
	$BASEDIR = dirname (__FILE__) .".txt";
	$BASEDIR = dirname ($BASEDIR);	// Trick to strip extra ending directory
	//echo "This is a maintenance run.\n".date('l dS \of F Y h:i:s A')."\nBASEDIR = $BASEDIR\n\n";
}
define ("FP_BASEDIR", $BASEDIR);

 /*
  * Home directory of the website relative to the server htdocs
  * Used for Flash, as when we want to load a file.
  * Then, if the website is inside a directory, such as "/fp",
  * we can't just ask for "/_user/_audio/slideshow/audio-190.mp3", 
  * it won't be found! Flash will look 
  * for "http://website.com/_user/_audio/slideshow/audio-190.mp3"
  * instead. By adding the FP_HOMEDIR (which has a slash before it!!!)
  * it will work:
  * $path = FP_HOMEDIR ."_user/_audio/slideshow/audio-190.mp3";
  * We add the following slash, if needed!
 */
$baseURL = dirname($_SERVER['PHP_SELF']);
$baseURL == "/" || $baseURL .= "/";
define ("FP_HOMEDIR", $baseURL);


// system Flags
define ('FP_FEATURED', 'new');
define ('FP_ACTIVE', 'all');
define ('FP_PRIVATE', 'private');


// Max execution time for the Maintenance.php script
// It might be handling big pictures (time in seconds)
$FP_MAINTENANCE_TIMELIMIT = 30;
define ("FP_MAINTENANCE_TIMELIMIT", $FP_MAINTENANCE_TIMELIMIT);

// Time after which maintenance.php unlocks the flag that 
// prevents it from processing. The flag could be locked if it was processing very
// big pictures, and called a second time while it was already at work.
// This time (in seconds) is the amount to give up and ignore the "I'm busy" flag
// which could be left set by a crash, e.g. out of memory
$FP_MAINTENANCE_RESET_TIME = 3*60;	// here 3 minutes
define ("FP_MAINTENANCE_RESET_TIME", $FP_MAINTENANCE_RESET_TIME);	// here 3 minutes

// Images for the site (not for the content of the site)
define ("FP_SYSTEM_IMAGES", "images/");
// the image to use for a missing image. It should be created through the system and belong to sysadmin
// That way, it will be created in various sizes, in each photo directory.
define ("FP_SYSTEM_MISSING_PICTURE", "missing_image.jpg");
define ("FP_SYSTEM_MISSING_PICTURE_PATH", FP_SYSTEM_IMAGES . "missing_image.jpg");

// Configuration files location
$CONFIG_DIR = "_config";
define ("FP_DIR_CONFIG", $CONFIG_DIR);
 
// directory for temp files
$TMPDIR = "tmp";
define ("FP_DIR_TMP", $TMPDIR);

// directory for support files, such as profiles, etc.
$FILESDIR = "files";
define ("FP_DIR_FILES", $FILESDIR);

// If this flag is true, then the maintenance.php script will
// sleep for a random time (0-15 seconds) to avoid all users running the heavy-duty processing parts
// of the script at the same time.
define ("FP_RANDOM_DELAY_MAINTENANCE", true);

// This file indicates linked databases, i.e. deleting a record in one indicates deleting
// dependent records in others. Example: delete user -> delete user's pictures
$DEPENDENCIES_FILE = "$BASEDIR/$CONFIG_DIR/dependencies.txt";

// The lifespan of the cookie which we will use if user wants to be remembered by the system (for admin)
// cookie lifetime in seconds (e.g. 30*24*3600 = 2 hours)
$COOKIELIFE = 30*24*3600;	 
define ("FP_COOKIELIFE", $COOKIELIFE);

// Maximum length of a project nickname, which is used to name a symlink (alias).
// It can't be longer than the file system allows!
$MAXNICKLEN 	= 30;

// Maximum length of the username
$MAXUSERNAMELEN	= 30;

// Maximum size in bytes of files to upload. I'm using 10MB
$MAXUPLOADSIZE	=	10000000;
define ("FP_MAXUPLOADSIZE", $MAXUPLOADSIZE);
$FP_MAXSMALLUPLOADSIZE	=	200000;
define ("FP_MAXSMALLUPLOADSIZE", $FP_MAXSMALLUPLOADSIZE);

// Maximum uploaded picture dimensions. 
// If this setting is too high, you might crash your PHP if you exceed the MEMORY_LIMIT when it tries
// to process the picture. I've set my PHP to 128M.

// EOS D5 (4368 x 2912) = 76.3MB
//define ("MAXPIXELS", 12000000);

// Canon 1DS Mark III
// 5,616 x 3,744 = 22 Megapixels = 21026304
// 37.44" x 25"
// define ("MAXPIXELS", 22000000);

//My scans are about 22500000...
define ("MAXPIXELS", 23000000);

// Iris max print size is 46" x 34"
// At 150 dpi, that's 6900 x 5100 pixels
// At 25bit RGB, that's 101MB
// define ("MAXPIXELS", 35190000);

// Whether the system can handle it, "Let us pray..."
// I think a better method would be to force a corresponding lo-res upload for big files.


// Number of emailed pictures to process at once, so we don't overload the server
$PIXTOPROCESS = 3;
define ("FP_PIXTOPROCESS", $PIXTOPROCESS);

/*
ACCESS LEVEL: Each artist has an access level to determine how much access he has within the system.
These determine what a user sees and controls in the admin section (system administrator)

FP_ADMINISTRATOR_LEVEL = Can edit everything allowed by FP_INSTALLATION_TYPE
FP_NORMAL_LEVEL = Can edit galleries and exhibitions (multi-gallery administrator)
FP_PERSONAL_LEVEL = Can edit only exhibitions
FP_SUPPLIER_LEVEL = Can edit only supplier info

*/
// Full administrator
define ("FP_ADMINISTRATOR_LEVEL", 1);
// Can edit galleries and exhibitions (multi-gallery administrator)
define ("FP_NORMAL_LEVEL", 2);
// Can edit only exhibitions and personal gallery
define ("FP_PERSONAL_LEVEL", 3);
// Can edit only supplier info
define ("FP_SUPPLIER_LEVEL", 4);

// Array of access levels
$FP_ACCESS_LEVELS = array (
	FP_ADMINISTRATOR_LEVEL		=>	"{fp:accesslevel1}",
	FP_NORMAL_LEVEL			=>	"{fp:accesslevel2}",
	FP_PERSONAL_LEVEL		=>	"{fp:accesslevel3}",
	FP_SUPPLIER_LEVEL		=>	"{fp:accesslevel4}"
	);

// Admin is always ID=1
define ("FP_ADMINISTRATOR", 1);
define ("FP_ADMINISTRATOR_GROUP", "admin_group");

// The public group, i.e. the group which is used for the gallery listing
// and is the default group, is ID=1
define("PUBLIC_GROUP_ID", 1);


/*
USER TYPE
Each artist has a user level, which determines what kind of account he has.
FP_SINGLE_GALLERY_SINGLE_USER cannot add galleries or users
FP_SINGLE_GALLERY_MULTI_USER cannot add galleries
FP_MULTI_GALLERY_SINGLE_USER can add new galleries, but not users
FP_MULTI_GALLERY_MULTI_USER can add new galleries, and new users
*/
define ("FP_SINGLE_GALLERY_SINGLE_USER", 1);
define ("FP_SINGLE_GALLERY_MULTI_USER", 2);
define ("FP_MULTI_GALLERY_SINGLE_USER", 3);
define ("FP_MULTI_GALLERY_MULTI_USER", 4);

// Array of access levels
$FP_GALLERY_TYPES = array (
	FP_SINGLE_GALLERY_SINGLE_USER		=>	"{fp:userlevel1}",
	FP_SINGLE_GALLERY_MULTI_USER		=>	"{fp:userlevel2}",
	FP_MULTI_GALLERY_SINGLE_USER		=>	"{fp:userlevel3}",
	FP_MULTI_GALLERY_MULTI_USER		=>	"{fp:userlevel4}"
	);

// Used by setup.php 
$FP_GALLERY_CONSTANT_VALUES = array (
	FP_SINGLE_GALLERY_SINGLE_USER	=> "FP_SINGLE_GALLERY_SINGLE_USER",
	FP_SINGLE_GALLERY_MULTI_USER	=> "FP_SINGLE_GALLERY_MULTI_USER",
	FP_MULTI_GALLERY_SINGLE_USER	=> "FP_MULTI_GALLERY_SINGLE_USER",
	FP_MULTI_GALLERY_MULTI_USER	=> "FP_MULTI_GALLERY_MULTI_USER"
	);

// Used by setup.php 
$FP_GALLERY_NAMES = array (
	"{fp:userlevel1}"	=>	FP_SINGLE_GALLERY_SINGLE_USER,
	"{fp:userlevel2}"	=>	FP_SINGLE_GALLERY_MULTI_USER,
	"{fp:userlevel3}"	=>	FP_MULTI_GALLERY_SINGLE_USER,
	"{fp:userlevel4}"	=>	FP_MULTI_GALLERY_MULTI_USER
	);

// Used by setup.php 
$FP_GALLERY_CONSTANT_NAMES = array (
	"FP_SINGLE_GALLERY_SINGLE_USER"	=>	"{fp:userlevel1}",
	"FP_SINGLE_GALLERY_MULTI_USER"	=>	"{fp:userlevel2}",
	"FP_MULTI_GALLERY_SINGLE_USER"	=>	"{fp:userlevel3}",
	"FP_MULTI_GALLERY_MULTI_USER"	=>	"{fp:userlevel4}"
	);

// INSTALLATION TYPES
// 1 = Public. People can buy subscriptions to accounts on the server. The admin user has unlimited access.
// 2 = Private. There is a single subscription for the entire installation, and admin access has limits, depending on the subscription.
define ("FP_INSTALLATION_PUBLIC", 1);
define ("FP_INSTALLATION_PRIVATE", 2);
// Array of installation types
$FP_INSTALLATION_TYPES = array (
	FP_INSTALLATION_PUBLIC		=>	"{fp:install1}",
	FP_INSTALLATION_PRIVATE		=>	"{fp:install2}"
	);

// Used by setup.php
$FP_INSTALLATION_CONSTANT_VALUES = array (
	FP_INSTALLATION_PUBLIC		=> "FP_INSTALLATION_PUBLIC",
	FP_INSTALLATION_PRIVATE		=> "FP_INSTALLATION_PRIVATE"
	);

// Used by setup.php 
$FP_INSTALLATION_CONSTANT_NAMES = array (
	"FP_INSTALLATION_PUBLIC"		=>	"{fp:install1}",
	"FP_INSTALLATION_PRIVATE"	=>	"{fp:install2}"
	);

// SPECIAL EXHIBITIONS USED FOR OTHER PURPOSES
// SLIDES is a project ("exhibition") to store pictures to be used
// for backgrounds, etc.
define ("FP_SLIDES_PROJECT", "slides");

// Shape of landscape 35mm image is 2:3, or 1.5
// This is used when pulling random images, to be sure they're the right shape
// We round the comparison, so anything between 1.50 and 1.59 should work.
define ("FP_35MM_SHAPE", 1.5);

// codes used to indicate variables in text templates, 
// e.g. "Here is $MyVariable"
define ("FIELDPREFIX", '$');
define ("FP_FIELDSUFFIX", '}');
define ("FP_FIELDSTART", '{');	// Had to pick this name cuz 'prefix' taken by legacy method, above

// prefix to a button in an HTML form to indicate it is a command (I think)
$commandFlag = "action_";

// file to hold system flags, such as last time maintenance was done
$FLAGS_FILE = "fp_flags.txt";
$FLAGS_DELIM = "\t";

// Blank picture when no image should be shown in a list, etc.
define ("BLANK_IMAGE", FP_SYSTEM_IMAGES . "blank.gif");

// File names of ICC/ICM color profiles
//define ("FP_PROFILE_SRGB", FP_DIR_FILES."/sRGB.icm");
define ("FP_PROFILE_SRGB", FP_DIR_FILES."/sRGB_v4_ICC_preference.icc");


// Activity logs
$LOGS	= "$BASEDIR/log";
define ("FP_DIR_LOG", $LOGS);

define ("FP_ERROR_LOG", "$LOGS/error.log");
define ("FP_ORDER_LOG", "$LOGS/sales.log");
define ("FP_PAYMENTS_LOG", "$LOGS/payments.log");
define ("FP_ACTIVITY_LOG", "$LOGS/activity.log");
define ("FP_PICTURES_LOG", "$LOGS/pictures.log");
define ("FP_MAINTENANCE_LOG", "$LOGS/maintenance.log");
define ("FP_UPDATES_LOG", "$LOGS/updates.log");

// MySQL database names
$IMAGES 		= "Images";
$PRICES 		= "Prices";
$PRICESETS 	= "PriceSets";
$ARTISTS 	= "Artists";
$TOPICS 		= "Topics";
$KEYWORDS 	= "Keywords";
$COMMENTS 	= "Comments";
$RATINGS 	= "Ratings";
$SETS 		= "Sets";
$GROUPS 		= "Groups";
$PROJECTS 	= "Projects";
$PARTS 		= "Parts";
$STORIES 	= "Stories";
$SUPPLIERS 	= "Suppliers";
$PAYPAL		= "Paypal";
$SALES		= "Sales";
$PAYMENTS	= "Payments";
$FILES		= "Files";

// This is useful for ReplaceSysVars
$DATABASES = array (
			"IMAGES"			=> $IMAGES,
			"PRICES"			=> $PRICES,
			"PRICESETS"		=> $PRICESETS,
			"ARTISTS"		=> $ARTISTS,
			"TOPICS"			=> $TOPICS,
			"KEYWORDS"		=> $KEYWORDS,
			"COMMENTS"		=> $COMMENTS,
			"SETS"			=> $SETS,
			"RATINGS"		=> $RATINGS,
			"GROUPS"			=> $GROUPS,
			"PROJECTS"		=> $PROJECTS,
			"PARTS"			=> $PARTS,
			"STORIES"		=> $STORIES,
			"SUPPLIERS"		=> $SUPPLIERS,
			"PAYPAL"			=> $PAYPAL,
			"SALES"			=> $SALES,
			"PAYMENTS"		=> $PAYMENTS,
			"FILES"			=> $FILES
			);

// Database names as constants. Wouldn't this be better than using global declarations
// all over the place?
define ("DB_IMAGES", $IMAGES);
define ("DB_PRICES", $PRICES);
define ("DB_PRICESETS", $PRICESETS);
define ("DB_ARTISTS", $ARTISTS);
define ("DB_TOPICS", $TOPICS);
define ("DB_KEYWORDS", $KEYWORDS);
define ("DB_COMMENTS", $COMMENTS);
define ("DB_RATINGS", $RATINGS);
define ("DB_SETS", $SETS);
define ("DB_GROUPS", $GROUPS);
define ("DB_PROJECTS", $PROJECTS);
define ("DB_PARTS", $PARTS);
define ("DB_STORIES", $STORIES);
define ("DB_SUPPLIERS", $SUPPLIERS);
define ("DB_PAYPAL", $PAYPAL);
define ("DB_SALES", $SALES);
define ("DB_PAYMENTS", $PAYMENTS);

// Special case...we don't need a DB for files (yet), but we can treat 
// requests as if we did use one.
define ("DB_FILES", $FILES);


// Global Directories
// ------------------
$FP_DIR_SETUP = "setup";
define ('FP_DIR_SETUP', $FP_DIR_SETUP);


// ===== USER DIRECTORIES
$FP_DIR_USER		= "_user";
$FP_DIR_USER_IMAGES = "$FP_DIR_USER/_img";
$AUDIODIR			= "_audio";
//$VIDEODIR			= "_video";
$USERFILESDIR		= "_files";
define ('FP_DIR_USER', $FP_DIR_USER);
define ('FP_DIR_USER_IMAGES', $FP_DIR_USER_IMAGES);
define ('FP_SUBDIR_AUDIO', $AUDIODIR);
//define ('FP_DIR_VIDEO', $VIDEODIR);
define ('FP_SUBDIR_USER_FILES', $USERFILESDIR);

// ===== AUDIO STORAGE DIRECTORIES
$SLIDESHOW_AUDIO = $AUDIODIR . "/slideshow";
define ("FP_SUBDIR_SLIDESHOW_AUDIO", $SLIDESHOW_AUDIO);


// ===== THEMES
// theme for the user/site, whatever.
// The theme simply adds a prefix/suffix to the 'snippets' folder name
// in the fetch snippet function. Therefore, if $theme is empty,
// we're using the 'Snippets' folder.
// default is empty

// the "default" here refers to a directory called "default" in themes. Same for 'admin'. Don't change this.
define ("DEFAULT_THEME", "default");
define ("DEFAULT_THEME_VARIATION", null);
define ("ADMIN_THEME", "admin");

// Themes directory
$FP_THEMES_DIR = "_themes";
define ("FP_THEMES_DIR", $FP_THEMES_DIR);

// SUB-FOLDERS IN THEMES
// Snippets are HTML and text used for the interface
$SNIPPETSDIR = "_snippets";
$IMAGESDIR = "_img";
$SWFDIR = "_swf";
$JSDIR = "_js";
$CSSDIR = "_css";
$FP_PREVIEWDIR = "_preview";
// This will exist in both the system themes dir and the user's themes dir
$FP_THEME_VARIATION_DIR = "$CSSDIR/_alt";

define ("FP_SNIPPETSDIR", $SNIPPETSDIR);
define ("FP_IMAGESDIR", $IMAGESDIR);
define ("FP_SWFDIR", $SWFDIR);
define ("FP_JSDIR", $JSDIR);
define ("FP_CSSDIR", $CSSDIR);
define ("FP_PREVIEWDIR", $FP_PREVIEWDIR);
define ("FP_THEME_VARIATION_DIR", $FP_THEME_VARIATION_DIR);

// == set up themes
// THEME SETTINGS
// the default theme for the site (can be changed)
// Lists of available themes for future use, e.g. user chooses his gallery look
//
// Themes (except for 'admin' and 'default') always begin with a number+underscore, e.g. 001_
// This won't appear to the user, but it ensures that when you add a theme, the themes already in use
// won't be screwed up. The issue is that a theme is identified by a number, e.g. theme #2, meaning the
// 2nd theme in the list. Insert something higher in the list, and the projects get assigned the wrong theme.
// So, as long as we don't add new themes with numbers lower than existing ones, we're OK.
// "Themes" class is defined in Themes.inc

$Themes = new Themes ();

// Javascript for all themes has a common name
$THEME_JS_NAME = "fp_javascript.js";
define ("FP_THEME_JS_NAME", $THEME_JS_NAME);
// Javascript for all themes has a common name
$THEME_JS_LIB_NAME = "fp_javascript_lib.js";
define ("FP_THEME_JS_LIB_NAME", $THEME_JS_LIB_NAME);

// CSS for all themes has a common name
$FP_THEME_CSS_NAME = "style.css";
define ("FP_THEME_CSS_NAME", $FP_THEME_CSS_NAME);
$FP_THEME_EXTRA_CSS_NAME = "extra_style.css";
define ("FP_THEME_EXTRA_CSS_NAME", $FP_THEME_EXTRA_CSS_NAME);
$FP_THEME_GALLERY_CSS_NAME = "gallery.css";
define ("FP_THEME_GALLERY_CSS_NAME", $FP_THEME_GALLERY_CSS_NAME);
$FP_THEME_NEWSLETTER_CSS_NAME = "newsletter_style.css";
define ("FP_THEME_NEWSLETTER_CSS_NAME", $FP_THEME_NEWSLETTER_CSS_NAME);

// ======= NEWSLETTER
// Dropbox to drop messages to be sent
// URL of the emailer system PHP script which adds new message to the sending queue.
// I've adapted a small one called PHPEmailList. The script is
// addmessage.php?subject=mysubject&message=mymessage&lists=mylist)
$FP_DIR_NEWSLETTER = "newsletter_dropbox";
define ("FP_DIR_NEWSLETTER", $FP_DIR_NEWSLETTER);

// internal use only
define ("OPTION_LIST_IS_POPUP", FALSE); // makes the param easier to read
define ("OPTION_LIST_IS_MULTI", TRUE); // makes the param easier to read


// ===== PHOTO STORAGE DIRECTORIES:

// directory where photographs are stored
$PHOTOSDIR 		= "photos/";

// Directories for photo storage and processing

// Directory where site uploaded pictures are stored.
// NOTE: must end in slash unless blank, e.g. "images/" 
// blank means in same dir as main files
$SITEIMAGES		= "";

$PHOTOS_GALLERY		= $SITEIMAGES . $PHOTOSDIR . "gallery";
$SLIDES			= $SITEIMAGES . $PHOTOSDIR . "slides";
$THUMBNAILS		= $SITEIMAGES . $PHOTOSDIR . "thumbnails";
//$MATTED		= $SITEIMAGES . $PHOTOSDIR . "matted";
//$FRAMED		= $SITEIMAGES . $PHOTOSDIR . "framed";
$ORIGINALS		= $SITEIMAGES . $PHOTOSDIR . "originals";
$AV			= $SITEIMAGES . $PHOTOSDIR . "av";
$MAILED_DIR		= $SITEIMAGES . $PHOTOSDIR . "input";

$PROCESSEDDIR			= $SITEIMAGES . $PHOTOSDIR . "processed/";
$PROCESSED_PHOTOS		= $PROCESSEDDIR . "gallery";
$PROCESSED_SLIDES		= $PROCESSEDDIR . "slides";
$PROCESSED_THUMBNAILS		= $PROCESSEDDIR . "thumbnails";
$PROCESSED_MATTED		= $PROCESSEDDIR . "matted";
$PROCESSED_FRAMED		= $PROCESSEDDIR . "framed";
$PROCESSED_ORIGINALS		= $PROCESSEDDIR . "originals";

$PICTURE_DIRECTORIES = array (
	"PHOTOS"					=> $PHOTOS_GALLERY,
	"SLIDES"					=> $SLIDES,
	"THUMBNAILS"				=> $THUMBNAILS,
//	"MATTED"					=> $MATTED,
//	"FRAMED"					=> $FRAMED,
	"ORIGINALS"				=> $ORIGINALS,
	"AV"						=> $AV,
	"MAILED_DIR"				=> $MAILED_DIR,
	"PROCESSEDDIR"			=> $PROCESSEDDIR,
	"PROCESSED_ORIGINALS"		=> $PROCESSED_ORIGINALS,
	"PROCESSED_PHOTOS"		=> $PROCESSED_PHOTOS,
	"PROCESSED_SLIDES"		=> $PROCESSED_SLIDES,
	"PROCESSED_THUMBNAILS"	=> $PROCESSED_THUMBNAILS
//	"PROCESSED_MATTED"		=> $PROCESSED_MATTED,
//	"PROCESSED_FRAMED"		=> $PROCESSED_FRAMED,
);

define ("FP_DIR_PHOTOS", $PHOTOSDIR);
define ("FP_DIR_GALLERY_IMAGES", $PHOTOS_GALLERY);
define ("FP_DIR_SLIDES_IMAGES", $SLIDES);
define ("FP_DIR_THUMBNAILS_IMAGES", $THUMBNAILS);
//define ("FP_DIR_MATTED", $MATTED);
//define ("FP_DIR_FRAMED", $FRAMED);
define ("FP_DIR_ORIGINAL_IMAGES", $ORIGINALS);
define ("FP_DIR_AV", $AV);
define ("FP_DIR_MAILED_DIR", $MAILED_DIR);
define ("FP_DIR_PROCESSEDDIR", $PROCESSEDDIR);
define ("FP_DIR_PROCESSED_ORIGINALS", $PROCESSED_ORIGINALS);
define ("FP_DIR_PROCESSED_PHOTOS", $PROCESSED_PHOTOS);
define ("FP_DIR_PROCESSED_SLIDES", $PROCESSED_SLIDES);
define ("FP_DIR_PROCESSED_THUMBNAILS", $PROCESSED_THUMBNAILS);
//define ("FP_DIR_PROCESSED_MATTED", $PROCESSED_MATTED);
//define ("FP_DIR_PROCESSED_FRAMED", $PROCESSED_FRAMED);

// Postlet Java uploader variables
// ------------------
//$UPLOADER_ENDPAGE = $SYSTEMURL . "postlet/upload_results.php";
// Note: $SYSTEMURL will be wrong if scripts are called from command line, because there won't be a server to reference from!
// This is OK, since only admin/management scripts are called from command line, e.g. process pictures
$UPLOADER_DEST = $SYSTEMURL . "javaUpload.php";
$UPLOADER_ENDPAGE = $SYSTEMURL . "admin.php?action=update&subset=Pictures&showthumbstatus=Hide&ID={ID}&artistID={artistID}&nextaction=update&table=Projects";


// SIGNUP
// Values to use with the PP subscription website
// These are ID's for different services the PP system is selling
// The services are found in the PP system config file.
// For example, in pp, the service id for a private FP server subscription set is "3".
define ('PP_INSTALLATION_PUBLIC', 2);
define ('PP_INSTALLATION_PRIVATE', 3);

// If pp is outside the fp directory:
//isset ($_SERVER['SERVER_NAME']) ? $PP_URL = "http://{$_SERVER['HTTP_HOST']}/pp/order.php" : $PP_URL = "";
// If pp is inside the fp directory:
$PP_URL = "http://{$SYSTEMURL}pp/order.php";
define ('PP_URL', $PP_URL );

//COMMERCE AND FRAME SHOP
// This is the url for the PayPal IPN processing script. Note, it must be urlencoded.
$PAYPAL_NOTIFY_URL = "http://".$SYSTEMURL."ipn_res.php";

// Do not encode the "http://" part of the URL!!!
// $PAYPAL_NOTIFY_URL = "http://" . urlencode($SYSTEMURL."ipn_res.php");
define ("PAYPAL_NOTIFY_URL", $PAYPAL_NOTIFY_URL );

// Number of entries hard-wired into the database for ltd. editions
// This number tells us how many sql fields have been created for amount, etc., info about
// sales of a picture.

$MAX_LIMITED_EDITION_SETS = 6;
define ("FP_MAX_LIMITED_EDITION_SETS",  $MAX_LIMITED_EDITION_SETS);

// Delay before an unconfirmed order is deleted.
// This means you have this amount of time to complete a Paypal payment before FP system dumps the order.
$FP_CLEAR_UNCONFIRMED_ORDERS_DELAY = 5 * 60;	// five minutes
//$FP_CLEAR_UNCONFIRMED_ORDERS_DELAY = 5;	// five seconds TESTING
define ("FP_CLEAR_UNCONFIRMED_ORDERS_DELAY", $FP_CLEAR_UNCONFIRMED_ORDERS_DELAY);

// CODES for the order specification URL-encoded order, for AJAX or passing data through PayPal
// These are codes a printing service will recognized. 
// e.g. myprinter.com?ro=1&pa=3

define("FP_QUANTITY", "qy");
define("FP_ORDER_ROW", "ro");
define("FP_ORDER_SIZE", "sz"); //don't know what this is for (not used anywhere)
define("FP_ORDER_ARTISTID", "ar");
define("FP_ORDER_FRAMECODE", "fr");
define("FP_ORDER_MATTECODE", "mt");
define("FP_ORDER_MATTEWIDTH", "mw");
define("FP_ORDER_SYSTEM_UNITS", "un");
define("FP_ORDER_PAPERCODE", "pa");
define("FP_ORDER_INKSETCODE", "is");
define("FP_ORDER_GLAZINGCODE", "gl");
define("FP_ORDER_FILENAME", "fn");
define("FP_ORDER_IMAGEID", "id");
define("FP_ORDER_IMAGEWIDTH", "iw");
define("FP_ORDER_IMAGEHEIGHT", "ih");
define("FP_ORDER_PRINTWIDTH", "pw");
define("FP_ORDER_PRINTHEIGHT", "ph");
define("FP_ORDER_MATCHPRINT", "mp");
define("FP_ORDER_SUPPLIER_ID", "su");
define("FP_ORDER_EXTRA_SHIPPING", "sh");
define("FP_ORDER_CAT_NUM", "cn");

$FP_ORDER = array (
			"FP_QUANTITY"				=> "qy",
			"FP_ORDER_ROW"				=> "ro",
			"FP_ORDER_SIZE"				=> "sz",	//largest side of image
			"FP_ORDER_PSIZE"				=> "pz",	//largest side of paper 
			"FP_ORDER_ARTISTID"			=> "ar",
			"FP_ORDER_FRAMECODE"			=> "fr",
			"FP_ORDER_MATTECODE"			=> "mt",
			"FP_ORDER_MATTEWIDTH"			=> "mw",
			"FP_ORDER_SYSTEM_UNITS"		=> "un",
			"FP_ORDER_PAPERCODE"			=> "pa",
			"FP_ORDER_INKSETCODE"			=> "is",
			"FP_ORDER_GLAZINGCODE"		=> "gl",
			"FP_ORDER_FILENAME"			=> "fn",
			"FP_ORDER_IMAGEID"			=> "id",
			"FP_ORDER_PRINTWIDTH"			=> "pw",
			"FP_ORDER_PRINTHEIGHT"		=> "ph",
			"FP_ORDER_MATCHPRINT"			=> "mp",
			"FP_ORDER_SUPPLIER_ID"		=> "su",
			"FP_ORDER_EXTRA_SHIPPING"		=> "sh"
			);

DEFINE ("FP_ORDER_COOKIE", "fp_order");


//============= File Sender
// queue directory
define ("AS_SENDER_QUEUE_DIR", $BASEDIR . "/autosender_queue");
// queue file name
define ("AS_SENDER_QUEUEFILE", "autosender_queue.txt");
// queue is dir/filename
define ("AS_SENDER_QUEUE", AS_SENDER_QUEUE_DIR."/".AS_SENDER_QUEUEFILE);
file_exists (AS_SENDER_QUEUE_DIR) || mkdir (AS_SENDER_QUEUE_DIR, 0755);
file_exists (AS_SENDER_QUEUE) || touch (AS_SENDER_QUEUE);


//============= PARAMS
// These constants let you reference params without worrying about the reference code (01, 02, etc.)
//----
// GALLERY (PROJECT) PARAMS
define ("FP_PARAM_GALLERY_GROUPING",	 	'01');
define ("FP_PARAM_GALLERY_SPACING", 		'02');
define ("FP_PARAM_GALLERY_SHOWCAPTIONS", 	'03');
define ("FP_PARAM_GALLERY_THEME", 		'04');
define ("FP_PARAM_GALLERY_GROUP_SPACING", 	'05');
define ("FP_PARAM_SLIDES_PROJECT", 		'06');
define ("FP_PARAM_GALLERY_SCALING", 		'07');
define ("FP_PARAM_GALLERY_COMMENTS", 		'08');
define ("FP_PARAM_GALLERY_ITEM_SETTINGS", 	'09');
//define ("FP_PARAM_GALLERY_CLIENTLIST", 	'10');

//----
// SUPPLIER PARAMS
define ("FP_PARAM_SUPPLIER_FRAME_COST_METHOD",	'01');
define ("FP_PARAM_SUPPLIER_MATTE_COST_METHOD",	'02');

//----
// IMAGES PARAMS
define ("FP_PARAM_IMAGE_OFFSITE", 		'01');
define ("FP_PARAM_IMAGE_WIDTH", 		'02');
define ("FP_PARAM_IMAGE_HEIGHT",		'03');
define ("FP_PARAM_IMAGE_OLD_PID",		'04');
define ("FP_PARAM_IMAGE_EDITION_SIZE",		'05');
define ("FP_PARAM_IMAGE_IS_ORIGINAL_ART",	'06');
define ("FP_PARAM_IMAGE_IS_FIXED_SIZE",		'07');
define ("FP_PARAM_IMAGE_FIXED_WIDTH",		'08');
define ("FP_PARAM_IMAGE_FIXED_HEIGHT",		'09');
define ("FP_PARAM_IMAGE_IS_VIDEO",		'10');
define ("FP_PARAM_IMAGE_VIDEO_EMBED_CODE",	'11');
define ("FP_PARAM_IMAGE_VIDEO_WIDTH",		'12');
define ("FP_PARAM_IMAGE_VIDEO_HEIGHT",		'13');
define ("FP_PARAM_IMAGE_ARTWORK_NAME",		'14');
define ("FP_PARAM_IMAGE_NO_SELL_MATTE",		'15');
define ("FP_PARAM_IMAGE_NO_SELL_FRAME",		'16');
define ("FP_PARAM_IMAGE_FIXED_PRICE",		'17');
define ("FP_PARAM_AMOUNT_SOLD_OUTSIDE",		'18');

//----
// ARTIST PARAMS
define ("FP_PARAM_ARTIST_BLOG", 		'01');
define ("FP_PARAM_ARTIST_LINKS", 		'02');
define ("FP_PARAM_ARTIST_LINKNAMES", 		'03');
define ("FP_PARAM_ARTIST_GALLERY_LINK", 	'04');
define ("FP_PARAM_ARTIST_COUPONS", 		'05');

//----
// GROUPS PARAMS
// 1 & 2 indicate whether the top (1) and bottom (2) menus are replaced with custom menus
define ("FP_PARAM_GROUP_MENU_1", 			'01');
define ("FP_PARAM_GROUP_MENU_2", 			'02');
// Menu items in custom menus
define ("FP_PARAM_GROUP_MENU_1_1", 		'03');
define ("FP_PARAM_GROUP_MENU_1_2", 		'04');
define ("FP_PARAM_GROUP_MENU_1_3", 		'05');
define ("FP_PARAM_GROUP_MENU_1_4", 		'06');
define ("FP_PARAM_GROUP_MENU_1_5", 		'07');
define ("FP_PARAM_GROUP_MENU_1_6", 		'08');
define ("FP_PARAM_GROUP_MENU_1_7", 		'09');
define ("FP_PARAM_GROUP_MENU_1_8", 		'10');

define ("FP_PARAM_GROUP_MENU_2_1", 		'11');
define ("FP_PARAM_GROUP_MENU_2_2", 		'12');
define ("FP_PARAM_GROUP_MENU_2_3", 		'13');
define ("FP_PARAM_GROUP_MENU_2_4", 		'14');
define ("FP_PARAM_GROUP_MENU_2_5", 		'15');
define ("FP_PARAM_GROUP_MENU_2_6", 		'16');
define ("FP_PARAM_GROUP_MENU_2_7", 		'17');
define ("FP_PARAM_GROUP_MENU_2_8", 		'18');

// checkbox: using custom or built-in menus?
define ("FP_PARAM_GROUP_CUSTOM_MENU", 	'19');

// HTML code for custom menu items in custom menus
define ("FP_PARAM_GROUP_MENU_CUSTOM_1", 	'20');
define ("FP_PARAM_GROUP_MENU_CUSTOM_2", 	'21');
define ("FP_PARAM_GROUP_MENU_CUSTOM_3", 	'22');
define ("FP_PARAM_GROUP_MENU_CUSTOM_4", 	'23');
define ("FP_PARAM_GROUP_MENU_CUSTOM_5", 	'24');


// serialized array of function=>text, using keys from $FP_NAVBAR_ITEMS, below
define ("FP_PARAM_GROUP_ICON", 	'25');
define ("FP_PARAM_GROUP_LOGO", 	'26');

// Hide listing of exhibitions on the page? useful to use only drop-down menus
define ("FP_PARAM_GROUP_HIDE_LISTING",		'27');

// Hide listing of exhibitions on the page? useful to use only drop-down menus
define ("FP_PARAM_GROUP_WEB_FONTS_HEAD",		'28');
define ("FP_PARAM_GROUP_WEB_FONTS_CSS",		'29');
define ("FP_PARAM_GROUP_WEB_FONTS_NAMES",		'30');


//----
// PRICESET PARAMS
define ("FP_PARAM_PRICESET_TIER", 		'01');


//============== CSS CODE 
// Useful code we can insert into style params in HTML for showing/hiding
define ("FP_CSS_HIDE", 'display:none;');
define ("FP_CSS_SHOW", '');

// Navbar (menu bar) elements
// These can be chosen in the admin system

// This array holds the function ("new") and the code snippet ID ("navbar_new").
// The code snippets are changeable, per system.
// Also, we could have a field (in groups) so the user can enter a new name, to override
// the code snippet.

$FP_NAVBAR_HOME_CODE = "home";
define ("FP_NAVBAR_HOME_CODE", $FP_NAVBAR_HOME_CODE);

// Snippet file names for the Title text (the name) for each menu
$FP_NAVBAR_ITEMS = array (
		"new"				=>	"navbar_item_title_new",
		"new menu"			=>	"navbar_item_title_new_menu",
		"all"				=>	"navbar_item_title_all",
		"all menu"			=>	"navbar_item_title_all_menu",
		"artist"			=>	"navbar_item_title_artist",
		"place"				=>	"navbar_item_title_place",
		"blog"				=>	"navbar_item_title_blog",
		"about"				=>	"navbar_item_title_about",
		"faq"				=>	"navbar_item_title_faq",
		"terms"				=>	"navbar_item_title_terms",
		"privacy"			=>	"navbar_item_title_privacy",
		"contact"			=>	"navbar_item_title_contact",
		"admin"				=>	"navbar_item_title_admin",
		$FP_NAVBAR_HOME_CODE	=>	"navbar_item_title_home",
		"catalog"			=>	"navbar_item_title_catalog",
		"shows"				=>	"navbar_item_title_shows",
		"share"				=>	"navbar_item_title_share",
		"fp"				=>	"navbar_item_title_fp",
		"client"			=>	"navbar_item_title_client_login",
		"clientwelcome"		=>	"navbar_item_title_client_welcome"
		);
		
$FP_NAVBAR_ITEMS_SOLO  = array (
		"about"			=>	"navbar_item_title_about_solo",
	);
$FP_NAVBAR_ITEMS_SOLO = array_merge ($FP_NAVBAR_ITEMS, $FP_NAVBAR_ITEMS_SOLO);

// Snippet file names for actual HTML code for each menu
$FP_NAVBAR_ITEMS_CODE = array (
		"new"			=>	"navbar_item_code_new",
		"new menu"		=>	"navbar_item_code_new_menu",
		"all"			=>	"navbar_item_code_all",
		"all menu"		=>	"navbar_item_code_all_menu",
		"artist"			=>	"navbar_item_code_artist",
		"place"			=>	"navbar_item_code_place",
		"blog"			=>	"navbar_item_code_blog",
		"about"			=>	"navbar_item_code_about",
		"faq"			=>	"navbar_item_code_faq",
		"terms"			=>	"navbar_item_code_terms",
		"privacy"		=>	"navbar_item_code_privacy",
		"contact"		=>	"navbar_item_code_contact",
		"admin"			=>	"navbar_item_code_admin",
		"home"			=>	"navbar_item_code_home",
		"catalog"		=>	"navbar_item_code_catalog",
		"shows"			=>	"navbar_item_code_shows",
		"share"			=>	"navbar_item_code_share",
		"fp"			=>	"navbar_item_code_fp",
		"client"		=>	"navbar_item_code_client_login",
		"clientwelcome"		=>	"navbar_item_code_client_welcome"
		);

$FP_NAVBAR_ITEMS_CODE_SOLO = array (
		"about"			=>	"navbar_item_code_solo_about"
		);
// Merge with non-solo items, but solo overwrites, e.g "all"
$FP_NAVBAR_ITEMS_CODE_SOLO = array_merge ($FP_NAVBAR_ITEMS_CODE, $FP_NAVBAR_ITEMS_CODE_SOLO);

// Names in the popup menu in Admin for each item
$FP_NAVBAR_POPUP_MENU = array (
		"new"			=>	"new exhibitions",
		"new menu"		=>	"new exhibitions menu",
		"all"			=>	"all exhibitions",
		"all menu"		=>	"all exhibitions menu",
		"artist"			=>	"by artist",
		"place"			=>	"by location",
		"blog"			=>	"blog",
		"about"			=>	"about the artist",
		"faq"			=>	"FAQ",
		"terms"			=>	"terms",
		"privacy"		=>	"privacy",
		"contact"		=>	"contact the artist",
		"admin"			=>	"artist admin area",
		"home"			=>	"gallery listing",
		"catalog"		=>	"catalog",
		"shows"			=>	"all shows",
		"share"			=>	"share",
		"fp"			=>	"powered by FP",
		"client"		=>	"client access",
		"clientwelcome"	=>	"client welcome",
		"custom1"		=>	"custom 1",
		"custom2"		=>	"custom 2",
		"custom3"		=>	"custom 3",
		"custom4"		=>	"custom 4",
		"custom5"		=>	"custom 5",
		);


// Admin page CSS Id's
// The BODY tag gets this ID in the admin pages, allowing us to do the menus right.
// 'none' is the tag so menu tabs w/o pages can get id's
define ("FP_PAGEID_NONE", "tab");
define ("FP_PAGEID_HELP", "tab1");
define ("FP_PAGEID_ARTISTS", "tab2");
define ("FP_PAGEID_PROJECTS", "tab3");
define ("FP_PAGEID_GROUPS", "tab4");
define ("FP_PAGEID_IMAGES", "tab5");
define ("FP_PAGEID_STORIES", "tab6");
define ("FP_PAGEID_SALES", "tab7");
define ("FP_PAGEID_SUPPLIERS", "tab8");
define ("FP_PAGEID_NEWSLETTER", "tab9");
define ("FP_PAGEID_SNIPPETS", "tab10");
define ("FP_PAGEID_THEMES", "tab11");
define ("FP_PAGEID_UPGRADE", "tab12");


// Text codes a user can wrap around a filename or code, to indicate it, OR a link to it,
// should be inserted in some text.
define ("FP_INSERT_USERFILE_CODE_OPEN", "[[");
define ("FP_INSERT_USERFILE_CODE_CLOSE", "]]");
$USER_IMAGE_EXTENSIONS = array("jpg","png","gif");
$USER_VIDEO_EXTENSIONS = array("swf","mov","mp4","mv4");
$USER_AUDIO_EXTENSIONS = array("mp3");

// Emailed Picture Processing:
// email box where pictures are received, e.g. pix@mydomain.com
// We use variables instead of constants because this could be changed in FP_CONFIG for weirdo systems
$FP_EMAIL_ACCOUNT = "pix";

// Security issue: Require username/password to send pix by email?
// Or, allow email pictures to be used only based on "from" field, which is easily spoofed?
// true = require username/password, false = use "from" field
$FP_EMAIL_FORCE_LOGIN = false;

?>