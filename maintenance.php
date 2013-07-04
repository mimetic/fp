<?php
/*	Maintenance for FP Gallery
	This script is called each minute to check for pictures that need processing.
	It used to do more, but other tasks should be less frequently!

	The picture processing grabs new pix from the incoming folders,
	sends any new submissions by FTP to the artist's site if he has supplied an address,
	adds new images to the site and puts them in the right project.

	The pix might be mailed or uploaded or sent by FTP, because all these ways will be 
	dumped into the same 'input' folders. 
*/

$start = microtime (true);

$error = "";
$msg = "";

include "_config/sysconfig.inc";
include "_config/fpconfig.inc";
include "_config/config.inc";
include "includes/functions.inc";
include "includes/project_management.inc";
include "includes/image_management.inc";
include "includes/commerce.inc";

error_reporting  ( E_ERROR | E_WARNING | E_PARSE ); 

$DEBUG = 0;
$DEBUG && print "DEBUG IS ON<BR>\n";
isset ($_REQUEST['debug']) && $DEBUG = true;

// Delay a random amount of time, if flag is set.
// This should help reduce the loca when all users on a server call this script
// at the same time, which happens with a cron tab.
$DEBUG || (FP_RANDOM_DELAY_MAINTENANCE && sleep (rand(0,15)));



set_time_limit (FP_MAINTENANCE_TIMELIMIT);

// Remove sales/orders that were not processed
$DEBUG && print "Maintenance: DeleteUnusedOrders<BR>";
$LINK = StartDatabase(MYSQLDB);
DeleteUnusedOrders();
mysql_close($LINK);
$FP_MYSQL_LINK->close();


$mlockfile = dirname(__FILE__)."/$TMPDIR/maintenance-lock-flag.txt";
if (file_exists ($mlockfile)) {
	$mlock = trim(ReadTextFile ($mlockfile));
} else {
	$mlock = 0;
	touch ($mlockfile);
}

$DEBUG && $mlock = 0;

// If lock is off...
if ($mlock == 0) {
	$mlock = time();
	$mlockformatted = date('j F, Y g:i:s a', $mlock);
	$DEBUG && print "Start Maintenance at $mlockformatted<BR>\n";

	
	// Check if there are any files to process. Allowed are JPG files.
	DeleteBadUploadedFiles ();
	if ( GetPictureFileTree(1) ) {
		WriteTextFile ($mlockfile, $mlock);
		$LINK = StartDatabase(MYSQLDB);
		if ( ProcessLocalImages () ) {
			$DEBUG && print __FILE__.":".__LINE__. ": Run maintenance()<BR>";
			Maintenance ($DEBUG);	// update everything if new pictures come in
			$time = 	round ((microtime (true) -  $start) * 1000)/1000;
			fp_error_log("Pictures found, ran ProcessLocalImages(), execution time is $time", 3, FP_MAINTENANCE_LOG);
			$DEBUG && print __FILE__.":".__LINE__. ": Processed pictures using ProcessLocalImages.<BR>\n";
		}
		mysql_close($LINK);
		$FP_MYSQL_LINK->close();
		WriteTextFile ($mlockfile, null);
	}
} else {
	// if lock is on...
	$mlockformatted = date('j F, Y g:i:s a', $mlock);
	$mlock && fp_error_log("Maintenance CRON cancelled: Maintenance CRON is already running (started at $mlockformatted)", 3, FP_MAINTENANCE_LOG);
	$DEBUG && print __FILE__.":".__LINE__. ": Maintenance is already running.<BR>\n";
	
	// has there been a crash? too long a delay? Check mlock and after some time, erase and try again
	$DEBUG && print __FILE__.":".__LINE__. ": locktime = $mlock ($mlockformatted), time passed=" . (time() - $mlock) . "<BR>";

	if (time() - $mlock > FP_MAINTENANCE_RESET_TIME ) {
		fp_error_log("Maintenance: I RESET THE MAINTENANCE--IS-BUSY FLAG...PROBABLY A CRASH?", 3, FP_MAINTENANCE_LOG);
		WriteTextFile ($mlockfile, null);
		$DEBUG && print __FILE__.":".__LINE__. ": I RESET THE MAINTENANCE--IS-BUSY FLAG...PROBABLY A CRASH?<BR>";
	}
}


$time = microtime (true) -  $start;

($DEBUG && $msg) && print "MESSAGE: $msg<HR>";
($DEBUG && $error) && print "ERROR: $error<HR>";
$DEBUG && print "End Maintenance at " . date('j F, Y g:i:s a') . "<BR>\n";
$DEBUG && print "This script processing time (in milliseconds) : ". round ($time * 1000)/1000;


?>