<?php
include "_config/sysconfig.inc";
include "_config/fpconfig.inc";
include "_config/config.inc";
include "includes/functions.inc";
include "includes/project_management.inc";
include "includes/image_management.inc";
include "includes/commerce.inc";

// If called from command line (e.g. from Maintenance.php) we need
// to get BASEDIR another way. Note that __FILE__ returns the ULR of THIS file!
// $BASEDIR = "";
// if (!$BASEDIR || ($BASEDIR == ".")) {
// 	$BASEDIR = dirname (__FILE__) .".txt";
// 	$BASEDIR = dirname ($BASEDIR);	// Trick to strip extra ending directory
// 	// This message will appear each minute if you're using CRON! 
// 	// $output .= "This is a maintenance run.\n".date('l dS \of F Y h:i:s A')."\nBASEDIR = $BASEDIR\n\n";
// }

// File list to update
if (strtolower(substr($BASEDIR, 0,6)) == "/users" ) {
	$privatedir = "/users/dgross/Sites/fp/fp_private";
} else {
	$privatedir = realpath($BASEDIR."/../fp_private");
}

$syspass = trim(file_get_contents ("$privatedir/pp_secret.txt"));

// Default pix to process at once. Can be set with maxpix=x in URL
$maxpix = 1;
// default refresh of screen
$refresh = 60;

isset($_REQUEST['maxpix']) && $maxpix = $_REQUEST['maxpix'];
isset($_REQUEST['refresh']) && $refresh = $_REQUEST['refresh'];
$refresh > 20 || $refresh = 20;

$syspassEntered = trim($_REQUEST['syspass']);

// ======= Set to true to run from command line: default is show =======
$hideOutput = isset($_REQUEST['hide']);

if ($hideOutput) {
	$hideOutputParam = "&hide=1";
} else {
	$hideOutput = false;
	$hideOutputParam = "";
}

// report all errors except 'Notice', which is for undefined vars, etc. 
// All the isset clauses are to avoid 'notice' errors. 
error_reporting(E_ALL ^ E_NOTICE);

session_name("fp_gallery_session");
session_start();

isset($_REQUEST['GroupID']) && $_SESSION['GroupID'] = $_REQUEST['GroupID'];
isset($_SESSION['GroupID']) || $_SESSION['GroupID'] = PUBLIC_GROUP_ID;

// isset($_REQUEST['theme']) && $_SESSION['theme'] = $_REQUEST['theme'];
isset($_SESSION['theme']) || $_SESSION['theme'] = DEFAULT_THEME;

$groupID = $_SESSION['GroupID'];

$LINK = StartDatabase(MYSQLDB);
Setup ();


$msg = "";

$msg .= '<html lang="en">
<head>
	<meta <meta http-equiv="refresh" content="'.$refresh.'">
</head>
<body>';


$msg .= "<h1>Rebuild Pictures FP</h1>";
$msg .= date("F j, Y, g:i a"). "<BR><BR>";
$msg .= "This program rebuilds all pictures in the FP system. If you change your picture parameters in config.inc, you might wish to rebuild the pictures.<BR>";

$self = $_SERVER['PHP_SELF'];
set_time_limit(5 * 60);

$command = $_REQUEST['q'];

$logfile = "pixrebuild.log";
$logfilesim = "pixrebuild_sim.log";
touch ("$LOGS/$logfile");
touch ("$LOGS/$logfilesim");

if ($command == "go") {
	$simmsg = "";
	$log = $logfile;
} else {
	$simmsg = "<h2>This is a simulation</h2>";
	$log = $logfilesim;
}


$msg .= "The list of pix already built is $LOGS/$log<BR>";
$msg .= "URL parameters: 
<ul>
<li>reset -> q=clear</li>
<li>process -> q=go</li>
<li>process X pictures at once -> maxpix=X</li>
<li>hide output -> hide=1</li>
</ul>";
//$msg .= "Example: php -q util_rebuild_pictures.php q=go maxpix=3 hide=1<BR><BR>";

$msg .= "<form action='$self'>\n";
$msg .= '<button type="submit" name="q">Simulation</button> ';
$msg .= '<button type="submit" name="q" value="clear">Reset List to Process</button> ';
$msg .= '<button type="submit" name="q" value="go">Process Images</button> <br><br>';
$msg .= 'Pictures to process at one time (maxpix): <input name="maxpix" type="text" value="' . $maxpix . '" size="3"><br>';
$msg .= 'Refresh delay: <input name="refresh" type="text" value="' . $refresh . '" size="3"> seconds<br>';
$msg .= 'Updater Password: <input name="syspass" id="syspass" type="password" size="40" value="' . $syspassEntered . '"><br>';


$msg .= "\n</form>\n";

//maintenance lock file
$mlockfile = "$TMPDIR/maintenance-lock-flag.txt";
// write lock
$lockfile = "$BASEDIR/rebuild_pix_lock.txt";
$locked = file_exists ($lockfile);
$locked && $msg .= "<h1>Lock file set!</h1>";

if ($syspassEntered == $syspass && !$locked) {
	
	$handle=opendir("$BASEDIR/$ORIGINALS/");
	while ($file = readdir($handle)) {
		if ((substr($file,1,1) != ".") AND (preg_match("/\.jpg$/i", $file))) { 
			$files[] = $file;
		}
	}
	closedir($handle);
	
	// clear commands starts over
	if ($command == "clear") {
		file_put_contents ("$LOGS/$logfile", "");
		file_put_contents ("$LOGS/$logfilesim", "");
		$command = "";
	} else {
		$pixrebuild = file ("$LOGS/$log", FILE_IGNORE_NEW_LINES | FILE_IGNORE_NEW_LINES);
		$pixrebuild || $pixrebuild = array ();
		
		//$msg .= ArrayToTable ($pixrebuild);
		
		$k=0;
		$i = 0;
		$pixlist = "<table><TR>";
	
		$files = array_diff($files, $pixrebuild);
		$filesremaining = count($files) - $maxpix;
		$filesremaining > 0 || $filesremaining = 0;
		$msg .= "Files remaining to process after this : $filesremaining<BR>";
	
		sort ($files, SORT_STRING);
	
		foreach ($files as $filename) {
			if (($filename != "missing_image.jpg") && !in_array ("$filename\n", $pixrebuild)) {
				$source = "$BASEDIR/$ORIGINALS/$filename";
				$watermarktext = "";
				$newsizes = $default_size;
				if (file_exists ($source)) {
					if ($command == "go") {
						$stime = microtime(true);
						
						// Lock maintenance from running

						// create lock
						file_put_contents ($lockfile, time());
						file_put_contents ($mlockfile, time());
												
						// move original to processed
						$from = "$BASEDIR/$ORIGINALS/$filename";
						$to = "$BASEDIR/$PROCESSED_ORIGINALS/$filename";
						copy ($from, $to) || $msg .= __FUNCTION__.":".__LINE__.": Could not copy $from to $to<BR>";
						chmod ($to, 0777);
	
						$source = $to;
						//DeleteAllPicVersions ($filename);
						ResizeNewImages ($source, $filename, $watermarktext);
						MoveProcessedToMain ($filename);
						
						$proctime = round(microtime(true) - $stime, 2);
						// Remove lockfile
						unlink ($lockfile);
					}
					
					//$msg .= "Rebuild: $filename<BR>";
					$pixlist .= "<TD align=\"center\">
							<img src=\"$PHOTOS_GALLERY/$filename\"><br>$filename<br>{$proctime} sec.
						</TD> \n";
					
					$pixrebuild[] = trim($filename);
					file_put_contents ("$LOGS/$log", join ("\n", $pixrebuild));
				
					$i++;
					if ($i >= $maxpix)
						break 1;
					//$command && sleep (4); // pause to give server breathing time
					$k++;
					if ($k>2) {
						$pixlist .= "</TR><TR>";
						$k = 0;
					}
				} else {
					$msg .= "error: $source does not exist<BR>";
				}
			}
		}
		$pixlist .= "</TR></TABLE>";
		$msg .= 	$simmsg . $pixlist;
		$remaining = join ("<BR>", $files);
		$msg .= "Remaining pictures: <BR>$remaining<BR>";
	}
} else {
	file_exists($lockfile) 
		? $lock = file_get_contents ($lockfile)
		: $lock = 0;
	$delay = time() - $lock;
	if ($delay > (60*3)) {
		file_exists($lockfile) && unlink ($lockfile);
		$msg .= "Oops, locked too long.<BR>";
	}
	$locked
		? $msg .= "Still processing<BR>"
		: $msg .= "Wrong or missing password<BR>";
}

$msg .= "<hr> END";
$msg .= '</body>
</html>';

$hideOutput || print $msg;

mysql_close($LINK);
$FP_MYSQL_LINK->close();

?>