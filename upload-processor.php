<?php 
	
$start = microtime (true);

$error = "";
$msg = "";

require_once "_config/sysconfig.inc";
require_once "_config/fpconfig.inc";
require_once "_config/config.inc";
require_once "includes/functions.inc";
require_once "includes/project_management.inc";
require_once "includes/image_management.inc";
//require_once "includes/commerce.inc";


// report all errors except 'Notice', which is for undefined vars, etc. 
// All the isset clauses are to avoid 'notice' errors. 
error_reporting(E_ALL ^ E_NOTICE);

$LINK = StartDatabase(MYSQLDB);
Setup ();

$ADMINFILENAME = basename (__FILE__);

// set the umask, so setting permissions work properly
// umask 022 should give us 644. Extra zero means octal.
$old_umask = umask(0022); 

// Get form input
$results = GetFormInput();
$actions = $results['actions'];
$action = $results['actions']['action'];
$vars = $results['vars'];

// Start Session
// Start before getting user, since FPUser looks in the session for the user
session_name("fp_admin");
session_start();




$DEBUG = 0;
$DEBUG && print __FILE__ . ": DEBUG IS ON<BR>\n";

//$DEBUG && fp_error_log(__FUNCTION__.":"."FORM VARS : ".print_r($vars,true), 3, FP_PICTURES_LOG );


$user = new FPUser( $LINK );
isset($_SESSION['fp_user']) ? $fp_user = $_SESSION['fp_user'] : $fp_user = null;
$fp_user && $user->loadUser($fp_user);
$user->is_loaded() || $fp_user = null;

//error_reporting  ( E_ERROR | E_WARNING | E_PARSE ); 

$DEBUG && fp_error_log(__FUNCTION__.":"." USER = $fp_user", 3, FP_PICTURES_LOG );



$ds = DIRECTORY_SEPARATOR;  //1

// Resize $width, $height inside of $w, $h
function fitDimensions ($width, $height, $w,$h) {
	if($width>$height) {
		$fmodwidth=$w;
		$fmodheight=round(($w/$width)*$height);
			if ($fmodheight>$h) {
				$fmodheight=$h;
				$fmodwidth=round(($h/$height)*$width);
			}
	} else {
		$fmodheight=$h;
		$fmodwidth=round(($h/$height)*$width);
	}
	return array ($fmodwidth, $fmodheight);
}

 
if (!empty($_FILES)) {

	//FOR EACH PHOTO UPLOADED, DO THE FOLLOWING STEPS...WE WOULD NEED TO LOOP THIS BIT FOR EACH IMATE IN THE ARRAY

	$tempFile = $_FILES['file']['tmp_name'];          //3             


//ORIGINAL IMAGES GO IN THE LARGESIZE FOLDER:

	//set the date for the filename addition
	//$append = '_' . date("mdyhms");

	//get the original file name from the post
	//$name = basename( $_FILES['filename']['name']);
	$name = basename( $_FILES['file']['name']);

	//get the parts of the file path
	$actual_name = pathinfo($name,PATHINFO_FILENAME);
	$original_name = $actual_name;

	//clean the name of any bad juju
	//Replace all spaces with hyphens
	$clean_name = str_replace(' ', '-', $actual_name);

	// Removes special chars
	$clean_name = preg_replace('/[^A-Za-z0-9\-]/', '', $clean_name);

	//set the extension
	// *** VERY IMPORTANT: THE EXTENSION MUST BE LOWER CASE FOR OTHER CODE TO FIND THE FILE! ***
	$extension = strtolower(pathinfo($name, PATHINFO_EXTENSION));
	
	// Fix jpeg -> jpg
	if ($extension == "jpeg")
		$extension = "jpg";

	//set the final new filename
	$newname = $clean_name.".".$extension;


	//the source file to process
	$tempFile = $_FILES['file']['tmp_name'];


	// Set default Session var for the WTF flash uploader
	// When uploading to projects, the ProjectID will be added as a directory
	$folder = $_SESSION["TFU_DIR"];

	$uploadFolderPath = join( array ( $folder, $newname), $ds);

	$DEBUG && fp_error_log(__FUNCTION__.":"."Output folder is : $uploadFolderPath", 3, FP_PICTURES_LOG );
	
	
	// If the file is not an acceptable file type, dump it.
	
	
	// Move file to "input" folder for this project
	if(move_uploaded_file($tempFile, $uploadFolderPath )) {


	$projectID = "";
	$table == $PROJECTS && $projectID = $vars['ID'];
	$newimages = false;

		// process uploaded HTML images, for artist portrait or logos
		$newimages = ProcessUploadedImages ($projectID, $fp_user);

		// PROCESS IMAGE
		$newimages = ProcessLocalImages ();

	} else {
		echo '<p>There was an error uploading the file or it was too large.  Make sure your photo is smaller than 2 megabytes. Please go <a href="javascript:history.go(-1)">back</a> and try again.</p>';
		exit();
	}
}


?>