<?php

/* 	
	Updater for FP for the databases
	Update MySQL.


	We use the site list which we share with the updater so we ignore the updater's entries.
	The file is updater/updater_site_list.txt
	A line in that file is: (host,username,directory,sitepass, dbname,dbusername,dbpass)
	We use the last three params.

*/


error_reporting  (E_ERROR | E_WARNING | E_PARSE); 

// If called from command line (e.g. from Maintenance.php) we need
// to get BASEDIR another way. Note that __FILE__ returns the ULR of THIS file!
$BASEDIR = "";
if (!$BASEDIR || ($BASEDIR == ".")) {
	$BASEDIR = dirname (__FILE__) .".txt";
	$BASEDIR = dirname ($BASEDIR);	// Trick to strip extra ending directory
	// This message will appear each minute if you're using CRON! 
	// $output .= "This is a maintenance run.\n".date('l dS \of F Y h:i:s A')."\nBASEDIR = $BASEDIR\n\n";
}

// === CONFIG ===========================
// base directory of installation
$LAPTOP = false;	// RUN FROM LAPTOP...different location for private files
isset ($_SERVER['SCRIPT_FILENAME']) && ($BASEDIR = dirname($_SERVER['SCRIPT_FILENAME']));

// directory to store files for the updater
$updateDir = "updater";

define ("ERRORLOG", "$BASEDIR/$updateDir/updater.log");

// === END CONFIG ===========================

if (!file_exists ("$BASEDIR/$updateDir") ) {
	mkdir ("$BASEDIR/$updateDir", 0755);
}

$page = "";
$output = "";
$error = "";
$msg = "";
$syspassEntered = "";
$query = "";

$page .= "<h1>Update MySQL</h1>";
$page .= "This script runs SQL to update the FP system.<BR>";

$testing = $_REQUEST['testing'];


// directory to store files for the updater
$updateDir = "updater";
$dblistfile = "updater_site_list.txt";

// === END CONFIG ===========================

if (strtolower(substr($BASEDIR, 0,6)) == "/users" ) {
	$privatedir = "/users/dgross/Sites/fp/fp_private";
	$page .= "(Using laptop configuration: $privatedir)<br><br>";
} else {
	$privatedir = realpath($BASEDIR."/../fp_private");
	$page .= "(Using remote configuration: $privatedir)<br><br>";
}

if (!file_exists ("$BASEDIR/$updateDir") ) {
	mkdir ("$BASEDIR/$updateDir", 0755);
}

$lastQueryFile = "updater_mysql_previous.sql";
$queriesFile = "updater_mysql_history.sql";

$syspass = trim(file_get_contents ("$privatedir/updater_syspass.txt"));
file_exists ("$BASEDIR/$updateDir/$lastQueryFile") && $query = trim(file_get_contents ("$BASEDIR/$updateDir/$lastQueryFile"));
if ($query) {
	$msg .= "Using query update file : $BASEDIR/$updateDir/$lastQueryFile<br>";
} else {
	$msg .= "Using query from website form.<br>";
	empty($_REQUEST['query']) || $query = stripslashes(trim($_REQUEST['query']));
}
$syspassEntered = trim($_REQUEST['syspass']);
$testing && $syspassEntered = $syspass;

// Get db to update (if set)
$mydb = $_REQUEST['mydb'];

// list of databases to update
$dblistarr = file ("$privatedir/$dblistfile");

// build list of databases to be updated ($dblist is array of dbname, username, password)
// build optionlist for HTML form to choose a db to update
$dbOptionList = "";
$dblist = array ();
foreach ($dblistarr as $option) {
	list ($label, $option) = explode("\t", $option);
	$dbOptionList .= "<option label=\"$label\" value=\"" . trim($option) . "\"></option>\n";
	$dblist[] = $option;
}
$dbOptionList = "<option label=\"All Sites\" value=\"all\"></option>\n" . $dbOptionList;
$dbOptionList = "<option label=\"Choose a site to update\" value=\"none\"></option>\n" . $dbOptionList;

// list of databases to update
// Each record is a string, "username,password" for the db.
if ($mydb != "none" && $mydb != "all") {
	$dblist = array ($mydb);
}

$comment = "";	// Comment is entered into the history file
if ($syspassEntered == $syspass) {
	if ($mydb != "none") {
		// make sure there's a query to run!
		if ($query) {
			// update all chosen sites (could be one)
			$comment .= "# -------------------------\n# " . date('l dS \of F Y h:i:s A') . "\n";
			foreach ($dblist as $db) {
				// note, we use the site list which we share with the updater
				// so we ignore the updater's entries 
				// a line is: (host,username,directory,sitepass, dbname,dbusername,dbpass)
				$dbinfo = explode(",", trim($db));
				$myDB = trim($dbinfo[4]);
				$user = trim($dbinfo[5]);
				$password = trim($dbinfo[6]);
				//list ($myDB, $user, $password) = explode(",", trim($db));
				$msg .= "* Update <b>$myDB</b> : ";
				
				// connect to database
				$LINK = StartDatabase("localhost", $user, $password, $myDB);
				
				if (!$LINK) {
					$error .= "<div style='border:1px solid red;padding:10px;'>";
					$error .= mysqli_error($LINK) . "<br>Query:<br>$queryline";
					$error .= "</div>";
				} else {
					// split query into lines (mysqli_query does only one line)
					$queries = explode("\r", $query);
					foreach ($queries as $queryline) {
						$queryline = trim ($queryline);
						// run the query
						!$testing && $result = mysqli_query ($queryline);
						!$testing ? $e = mysqli_error($LINK) : $e = false;
						if ($e) {
							$error .= "Error updating <b>$myDB</b>: <i>" . $e . "</i><br>Query: <i>$queryline</i><br>";
						} else {
							!$testing ? $msg .= " <i>$queryline</i><br>" : $msg = "TESTING<br>";
							// delete updater file if it exists
						}
					}
				}
				// Disconnect from database
				mysqli_close($LINK);
				$FP_MYSQL_LINK->close();
				$msg .= "<br>";
				$comment .= "# Modify database: $myDB\n";
			}
			
			// Write query to query log. This is a mysql document, actually,
			// so we can use it to modify existing databases.
			if (!$testing)
				// Add command to history
				file_put_contents ("$BASEDIR/$updateDir/$queriesFile", $comment, FILE_APPEND);
				file_put_contents ("$BASEDIR/$updateDir/$queriesFile", $query . "\n", FILE_APPEND);
				// Write query to previous query file
				file_put_contents ("$BASEDIR/$updateDir/$lastQueryFile", $query);
				
		} else {
			$error .= "Please enter a query<BR>";
		}	
	} else {
		$error .= "Choose a database to update.<BR>";
	}
} else {
	$error .= "Sorry, wrong password.<BR>";
}

$error && $error = '<div style="border:1px solid red;padding:10px;margin:10px;">' . $error . '</div>';
$msg && $msg = '<div style="border:1px solid blue;padding:10px;margin:10px;">' . $msg . '</div>';
$page = str_replace ("{page}", $page, file_get_contents ("$updateDir/updater_mysql_page.txt"));
$page = str_replace ("{msg}", $msg, $page);
$page = str_replace ("{syspassEntered}", $syspassEntered, $page);
$page = str_replace ("{query}", stripslashes($query), $page);
$page = str_replace ("{dbOptionList}", $dbOptionList, $page);
$page = str_replace ("{error}", $error, $page);
$page = str_replace ("{output}", $output, $page);
print $page;
// ENDIT
// ==================================================

function StartDatabase($host, $user, $password, $myDB) {
	global $error;
	
	$LINK = new mysqli($host, $user, $password);
	if (!$LINK) {
		$error .= "Could not connect to $host:$myDB as $user/$password.";
		return;
	}

	if ($DEBUG)
		print "Connected successfully<BR>";
	
	// Select the DATABASE
	if (!mysqli_select_db($LINK, "$myDB")) {
		$error .= "Could not select database $myDB";
	}
	
	return $LINK;
}


?>