<?php

/* 	Updater
	Update files on remote sites.
	- list of files to update includes files and directories. Filename is updaterfiles.txt
	File Format:
	- Single filenames and sub-paths on each line
	- filename/directory to copy <tab> files to not copy separated by commas, NO SPACES

	We use the site list which we share with the updater so we ignore the updater's entries.
	The file is updater/updater_site_list.txt
	A line in that file is: (host,username,directory,sitepass, dbname,dbusername,dbpass)
	We use the first four params.
	
	
	To Do:
	- if possible, fix permissions using settings
	- setting to delete extra files at remote site in directories (?)
*/

error_reporting  (E_ERROR | E_WARNING | E_PARSE); 

// Set timeout limit to 10 minutes!
set_time_limit (10 * 60);

// Flush the output buffer...we hope to output continously
ob_start();

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
$updateDir = "$BASEDIR/updater";

define ("ERRORLOG", "$updateDir/updater.log");
define ("OPTION_LIST_IS_POPUP", FALSE); // makes the param easier to read
define ("OPTION_LIST_IS_MULTI", TRUE); // makes the param easier to read

// === END CONFIG ===========================

// Make update directory if it doesn't exist.
if (!file_exists ("$updateDir") ) {
	mkdir ("$updateDir", 0755);
}
// array to hold histories of website file transfer. $history is filled out
// as needed from disk files.
$history = LoadHistory ();
$page = "";
$output = "";
$error = "";

// === BEGIN ===========================

$page .= "<h1>Update Files on Remote Server</h1>";
$page .= "This script copies files from this server to another, using a list of files in updaterfiles.txt<BR>";

// File list to update
if (strtolower(substr($BASEDIR, 0,6)) == "/users" ) {
	$privatedir = "/users/dgross/Sites/fp/fp_private";
	$page .= "(Using laptop configuration: $privatedir)<br><br>";
} else {
	$privatedir = realpath($BASEDIR."/../fp_private");
	$page .= "(Using remote configuration: $privatedir)<br><br>";
}

$filelist = file ("$privatedir/updater_list.txt");
$sitelist = file ("$privatedir/updater_site_list.txt");
$syspass = trim(file_get_contents ("$privatedir/updater_syspass.txt"));
//$page .= "syspass = $syspass in "."$BASEDIR/fp_private/updater_syspass.txt"."<BR>";


// Target
$hostname = $_REQUEST['hostname'];
$username = $_REQUEST['username'];
$password = $_REQUEST['password'];
$source = $_REQUEST['source'];
$directory = $_REQUEST['directory'];
$whichsite = $_REQUEST['site'];
$dofullupdate = $_REQUEST['fullupdate'];
$syspassEntered = trim($_REQUEST['syspass']);

// Get list of source directories
$dirs = GetSubDirectoryTree ($BASEDIR);
$dirs[$BASEDIR] = $BASEDIR;
$sourcePopup = OptionListFromArray ($dirs, "source", $source, true, OPTION_LIST_IS_POPUP, "ID=\"source\" ");

// get list of sites to update into javascript
$x=0;
$allsites = array ();
foreach ($sitelist as $site) {
	list ($name, $value) = explode("\t", trim ($site));
	$jsSiteList .= "\t\tsite[$x] = \"". trim ($value) . "\";\n";
	$popupSiteList .= "\t\t<option value=\"$x\">$name</option>\n";
	$x++;
	$allsites[$name] = explode(",", $value);
}

$js = '
	<script type="text/javascript" language="javascript">
	<!--
	
	function ChooseSite (x) {
	var site = new Array();
	var s = new Array();
	
	if (x) {
	' . $jsSiteList . '
	
	s = site[x].explode(",");
	document.getElementById("hostname").value = s[0];
	document.getElementById("username").value = s[1];
	document.getElementById("directory").value = s[2];
	document.getElementById("password").value = s[3];
	}
	}
	
	//-->
	</script>
';
$form1 = '

	<form action="updater.php" method="post">
';
$form1a = '
<div style="color:red;">
	<br>
	Updater Password: <input name="syspass" id="syspass" type="password" size="60" value="' . $syspassEntered . '"><br>
	<br>
</div>
';

$form2 = '<select id="site" name="site" onChange="ChooseSite(this.value)">
	<option value="none">Choose a site to update</option>
	<option value="all">UPDATE ALL SITES</option>
	' . $popupSiteList . '
	</select>	
	<br>
	<br>

	hostname: <input id="hostname" name="hostname" type="text" size="60" value="' . $hostname . '"><br>
	username: <input id="username" name="username" type="text" size="60" value="' . $username . '"><br>
	<font color="#FF0000">password: <input id="password" name="password" type="text" size="60" value="' . $password . '"></font><br>
	<input name="fullupdate" type="checkbox" value="1">Update all files (not only changed files)<br>
	<br>
	Copy from: ' . $sourcePopup . 'Use <tt>home/username/public_html</tt> for a full site update.<br>

	To remote directory: <input id="directory" name="directory" type="text" size="60" value="' . $directory . '"> Use <tt>public_html</tt> for a full site update.<br>
	<br>
	<input name="testing" type="checkbox" value="1"> Testing, do not copy.<br>
	<br>
';
$form3 = '	<button type="submit" name="submit">Begin</button>
	</form>';

// Check for password
if ($syspassEntered == $syspass) {
	//$page .= "Updater Password verified<BR>";
	$page .= $js;
	$page .= $form1;
	$page .= $form1a;
	$page .= $form2;
	$page .= $form3;

	if (($whichsite != "all") && ($whichsite == "none" || !$password || !$username || !$hostname || !$directory)) {
		$error .="Missing parameters!";
	} else {
		$testing = isset($_REQUEST['testing']);
		//$testing = true;
		//$page .= __LINE__.":Testing set to true<BR>";
		$testing && $page .= "<hr><h2>TESTING</h2>";
		
		// Send file from source to target
		$ftp_server = $hostname;
		$newfolder = null;
		$ftp_user_name = $username;
		$ftp_user_pass = $password;
		
		if (trim($whichsite) == "all" ) {
			// do all web sites
			$output .= "<h1 style='color:blue;'>Update all sites</h1><hr>";
			foreach ($allsites as $s) {
				if ($s[0] != $_SERVER['SERVER_NAME']) {
					$output .= "<h2 style='color:blue;'>Update {$s[0]}</h2>";
					$files = BuildFileList ($filelist, $source, $s[0]);
					$website = array (	$files,
										$s[0], 
										null,
										$s[1],
										$s[3],
										$source,
										$s[2]
									);
					$output .= FTPFiles ( $website );
				} else {
					$output .= "<h2 style='color:red;'>Skip {$s[0]}</h2>";
					$output .= "WARNING: Don't try to copy from this server to this server ({$s[0]})!<br> The files on {$s[0]} will be destroyed!<br>Fix 'updater_site_list.txt' immediately!<BR>";
				}

			}

		} else {
			// do one site
			if ($ftp_server != $_SERVER['SERVER_NAME']) {
				$output .= "<h1 style='color:blue;'>Update {$ftp_server}</h1><hr>";
				$files = BuildFileList ($filelist, $source, $ftp_server);
				$website = array (	$files,
									$ftp_server, 
									$newfolder,
									$ftp_user_name,
									$ftp_user_pass,
									$source,
									$directory
								);
				$output .= FTPFiles ( $website );
			} else {
				$output .= "<h2 style='color:red;'>Skip {$ftp_server}</h2>";
				$output .= "WARNING: Don't try to copy from this server to this server ({$ftp_server})!<br> The files on {$ftp_server} will be destroyed!<br>Fix 'updater_site_list.txt' immediately!<BR>";
			}
	}
		

		$page .= "<hr>";
	}
} else {
	$error .= "Updater Password FAILED: $syspassEntered<BR>";
	$page .= $form1;
	$page .= $form1a;
	$page .= $form3;
}

$error && $error = '<div style="border:1px solid red;padding:10px;">' . $error . '</div>';


$page = str_replace ("{page}", $page, file_get_contents ("$updateDir/updater_page.txt"));
$page = str_replace ("{error}", $error, $page);
$page = str_replace ("{output}", $output, $page);

CleanHistory ();

print $page;
// ENDIT
// ==================================================================



// ================ Functions ================

// ==================================================================
// 	File Format:
// 	- Single filenames and sub-paths on each line
// 	- filename (tab) files to not copy separated by commas, NO SPACES
//	- ">" at the beginning of the line means it's commented out, e.g. > myfile.php

function BuildFileList ($files, $sourcedir, $ftp_server, $nocopylist = array ()) {
	global $history, $dofullupdate;
	global $testing;	

	$DEBUG = 0;
	
	// $history is loaded at beginning with known data
	
	$filelist = array ();
	foreach ($files as $entry) {
		// parse entry
		$f = explode ("\t", trim($entry));
		$file = trim($f[0]);
		
		if ($file[0] == ">")
			continue;

		if (!file_exists ("$sourcedir/$file"))
			continue;
				
		$destination_file = $file;
		
		// if item is a directory
		if (is_dir("$sourcedir/$file")) {
			$DEBUG && print "<hr><i>Send directory: $sourcedir/$file</i><BR>";
			isset($f[1]) ? $ncl = explode (",", $f[1]) : $ncl = array ();
			$nocopylist = array_merge ($nocopylist, $ncl);
			array_walk ($nocopylist, 'trimME');
			$nocopylist && $DEBUG && print "<i>No-copy list: ".join(", ", $nocopylist)."</i><br>";

			$f2 = array ();
			foreach (glob ("$sourcedir/$file/*") as $f) {
				$f = trim ($f);
				$f = str_replace("$sourcedir/", "", $f);
				if (!in_array(basename($f), $nocopylist)) {
					//$f = basename($f);
					$f2[] = $f;
				} else {
					$DEBUG && print "<b>SKIPPING $f (in copy list)</b><BR>";
				}
			}
			$filelist = array_merge ($filelist, BuildFileList ($f2, $sourcedir, $ftp_server, $nocopylist));
			$DEBUG && print "<hr>";
		} else {
			$filename = str_replace($sourcedir, "", trim($file));
			// If mod date on the file is newer than last transfer, then send it again
			$lastTransfer = $history[$ftp_server][$filename] + 0;
			$change = filectime ("$sourcedir/$file") ;
			if ($dofullupdate || $change > $lastTransfer) {
				$DEBUG && print "transfer --> $ftp_server/$file ($change > $lastTransfer)<BR>";
				$filelist[] = $filename;
				!$testing && $history[$ftp_server][$filename] = time();
			} else {
				$DEBUG && print "skip --> $ftp_server/$file<BR>";
			}
		}
	}
	return $filelist;
}




// use for array_walk
function trimMe (&$x) {
	$x = trim ($x);
}


// Send a picture, or set, to another FTP server
// FTP (filename|array, ftp_server, ftp_user_name, ftp_user_pass
// NOTE: We're sending from the 'originals' folder in photos. 
// 		 Just list the file name, no directory info.
//		 This function provides it.
function FTPFiles ($website ) {
	global $BASEDIR, $testing;
	global $error;
	
	$DEBUG = 0;
	$success = true;
	$output = "";

	list ($files, $ftp_server, $newfolder, $ftp_user_name, $ftp_user_pass , $sourcedir, $directory) = $website;

	
	print  "<hr><h2>Copy from <font color='blue'>$sourcedir</font> to <font color='blue'>$ftp_server/$directory</font></h2>";
	//print  "<h2></h2>ftp_server = $ftp_server | directory = $directory | newfolder = $newfolder | ftp_user_name = $ftp_user_name | ftp_user_pass  = $ftp_user_pass  | sourcedir = $sourcedir<br>";
	
	//$DEBUG && print __LINE__.": FTPFiles ($files, $ftp_server, $directory, $newfolder, $ftp_user_name, $ftp_user_pass , $sourcedir );<br>";
	
	
	// default source directory is queue path, SENDER_QUEUEPATH
	$sourcedir || $sourcedir = SENDER_QUEUEPATH;

	//convert $files to array of one element if it isn't one
	if (!is_array($files))
		$files = array ($files);
	

	// set up basic connection
	$conn_id = ftp_connect("$ftp_server"); 
	
	// login with username and password
	$conn_id && $historyin_result = ftp_login($conn_id, "$ftp_user_name", "$ftp_user_pass"); 
		
	// check connection
	if ((!$conn_id) || (!$historyin_result)) { 
		fp_error_log(__FUNCTION__.":"."Failed to connect to $ftp_server for user $ftp_user_name", 3, ERRORLOG );
		$error .=  __FUNCTION__.":"."Failed to connect to $ftp_server for user $ftp_user_name";
		$success = false;
		return $success;
	} else {
		print  "<i>Connected to $ftp_server</i><BR>";
	}
	
	// switch to passive PASV mode
	if (!ftp_pasv ($conn_id, TRUE)) {
		fp_error_log(__FUNCTION__.":"."Could not turn on PASSIVE mode", 3, ERRORLOG );
		$success = false;
		return $success;
	} else {
		print  "<i>Switched to passive mode</i><BR>";
	}

	$ftpbasedir = ftp_pwd($conn_id);
	$ftpbasedir == "/" && $ftpbasedir = "";
	$DEBUG && print  __LINE__.": FTP base directory: $ftpbasedir<P>";
	
	// change to directory
	if ($directory) {
		if (!ftp_chdir ($conn_id, $directory)) {
			fp_error_log(__FUNCTION__.__LINE__.": "."Ftp could not change to $directory", 3, ERRORLOG );
			$success = false;
			return $success;
		} else {
			print  "<i>Changed directory to $directory</i><BR>";
		}
	}
	
	
	// create a new folder
	if ($newfolder) {
		NewRemoteFolder ($conn_id, $newfolder);
	}


	$currentdir = ftp_pwd($conn_id);
	$DEBUG && print  "<b>Current Directory: $currentdir</b><BR>";
	
	// Send files, ignore files/dirs indicated
	foreach ($files as $source_file) {
		$DEBUG && print  "<hr>Transfer: $ftpbasedir/$directory/$source_file<BR>";
		
		//if ($currentdir != $ftpbasedir)
		//	ftp_chdir ($conn_id, "$ftpbasedir/$directory");
		
		$destination_file = basename ($source_file);
		
		// Check if filename has directories, e.g. /hello/there/file.txt
		// If so, create directories to match then go there
		
		$dirs = GetDirectories ($source_file);
		
		// Go to correct directory. Create it if need be
		// $currentdir = ftp_pwd($conn_id);
		$fullpath = "$ftpbasedir/$directory";
		//$sourcefilepath = dirname($source_file);
		//$sourcefilepath == "." && $sourcefilepath = "";
		if ("$currentdir/." != "$fullpath/".dirname($source_file)) {
			$DEBUG && print  "&nbsp;&nbsp;&nbsp;<i>Current path: $currentdir<br>
				&nbsp;&nbsp;&nbsp; target path = $fullpath/".dirname($source_file)."<br>";
			foreach ($dirs as $dir) {
				$fullpath .= "/$dir";
				$DEBUG && print  __LINE__.": Switch directory to $fullpath<BR>";
				NewRemoteFolder ($conn_id, $fullpath);
			}
			$DEBUG && print  "&nbsp;&nbsp;&nbsp;Go to "."$directory/".dirname ($source_file)."<br>";
			ftp_chdir ($conn_id, "/$directory/".dirname ($source_file));
			$currentdir = ftp_pwd($conn_id);
		}
		
		if (!$testing) {
			$upload = ftp_put($conn_id, "$destination_file", "$sourcedir/$source_file", FTP_BINARY);
		} else {
			$upload = true;
		}
				
		// check upload status
		if (!$upload) { 
				print  "<font color='red'>".__FUNCTION__.":"."$source_file -> $ftp_server/$directory as $destination_file FAILED</font><br><br>";
				$error .=  "<font color='red'>".__FUNCTION__.":"."$source_file -> $ftp_server/$directory as $destination_file FAILED</font><br><br>";
				$success = false;
				return $success;
		} else {
			$testing && print  "(test) ";
			print  "upload --> $ftp_server/$directory/<b>$source_file</b><BR>";
			//print  __FUNCTION__.":"."Uploaded $source_file to $ftp_server/$directory as $destination_file<br><br>";
			// Record in log that we sent this file
			WriteEventToHistory ($ftp_server, $source_file);
		}
		ob_flush ();
		flush();
	}
	
	// close the FTP stream 
	ftp_quit($conn_id); 
	
	return $output;
}

// given a path, e.g my/full/path/name/file.txt
// return the directories in an array, e.g. (my, full, path, name)
function GetDirectories ($path) {
	$path = dirname ($path);
	$x = 1;
	$dir = array ();
	while ($path != ".") {
		$path_parts = pathinfo($path);
		array_unshift($dir, $path_parts['basename']);
		$path = $path_parts['dirname'];
		if ($x > 100)
			break;
		$x++;
	}
	return $dir;
}


function NewRemoteFolder ($conn_id, $newfolder) {
	global $testing;
	
	$DEBUG = 0;

	$newfolder = str_replace(" ", "_", $newfolder);
	if (!$newfolder)
		exit;
		
	// see if the folder already exists
	$DEBUG && $output .= __FUNCTION__.": Check for $newfolder<BR>";
	if (!@ftp_chdir ($conn_id, $newfolder)) {
	// create the new folder because it doesn't already exist
		if (!$testing) {
			if (!ftp_mkdir ($conn_id, $newfolder)) {
				fp_error_log(__FUNCTION__.":"."Ftp could not create a new folder called $newfolder", 3, ERRORLOG );
				$success = false;
				return $success;
			} else {
				if (!ftp_chdir ($conn_id, $newfolder)) {
					fp_error_log(__FUNCTION__.":"."Ftp could not change to $newfolder", 3, ERRORLOG );
					$success = false;
					return $success;
				} else {
					$output .= __FUNCTION__.": Created $newfolder<BR>";
				}
			}
		}
	}
	return true;
}



//----------
// Record date/time of last update of a file in the history of updates
// This way we don't update all files, only changed ones
// We'll have to go back and clean up the log, later.
function WriteEventToHistory ($website, $filename) {
	global $history, $updateDir;

	$f = "$updateDir/updater_history.txt";
	if (!($handle = fopen($f, 'a'))) {
		echo __FUNCTION__.": Cannot open file ($f)";
		exit;
	}
	
	$time = time();
	$x = "$website\t$filename\t$time\n";
	
	// Write $somecontent to our opened file.
	if (fwrite($handle, $x) === FALSE) {
		echo __FUNCTION__.": Cannot write to file ($f)";
		exit;
	} else {
		//print __FUNCTION__.": Wrote $x<BR> to $f<br>";
	}
	
	fclose($handle);
}

function CleanHistory () {
	global $history, $updateDir;
	$history = LoadHistory ();
	SaveHistory ($history);
}

function SaveHistory ($history) {
	global $updateDir;
	
	$history || $history = array ();
	$f = "$updateDir/updater_history.txt";
	
	$rows = "";
	while (list ($website, $files) = each ($history)) {
		//print "Write log of $website:<hr>";
		while (list ($filename, $time) = each ($files)) {
			$row ="$website\t$filename\t$time\n";
			$rows .= $row;
			//print "Write row : $row<BR>";
		}
	}
	file_put_contents ($f, $rows);
}

// Loads and cleans up the file transfer event log.
// Loading deals with duplicate entries, keeping the most recent,
// then writes the cleaned up log back out.
function LoadHistory () {
	global $updateDir, $output;
	
	$DEBUG = 0;
	
	$f = "$updateDir/updater_history.txt";
	file_exists ($f) 
		? $historyfile = file ($f)
		: $historyfile = array ();
		
	$history = array ();
	foreach ($historyfile as $row) {
		//print __FUNCTION__.": $row<BR>";
		$row = trim ($row);
		list ($website, $filename, $time) = explode("\t", $row);
		if (!isset($history[$website][$filename])) {
			$history[$website][$filename] = $time;
		} else {
			if ($time > $history[$website][$filename]) {
				$history[$website][$filename] = $time;
			}
		}
		$DEBUG && print __FUNCTION__.": history[$website][$filename] = $time<BR>";
	}
		
	return $history;
}


function SaveHistoryOLD ($website) {
	global $history, $updateDir;
	
	$history || LoadHistory ($website);
	$f = "$updateDir/updater_history.txt";
	file_put_contents ($f, serialize ($history));
}

function LoadHistoryOLD () {
	global $history, $updateDir;

	$f = "$updateDir/updater_history.txt";
	file_exists ($f) 
		? $history = unserialize(file_get_contents ($f))
		: $history = array ();
	return $history;
}

//----------
// WriteLog (filename, entry)
// Write a log entry
function WriteLog ($filename = "updater.log", $entry="") {
	error_log (date('Y-m-d H:i:s') . ": {$entry}\n", 3, $filename);
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
	global $THEME;
	global $msg, $error;
	$DEBUG = 0;
	
	is_array($values) || $values = array();
	
	if ($sort) 
		asort ($values);

	if (!is_array($checked))
		$checked = array ($checked);
	
	$optionlist = "";
	
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": values:" . ArrayToTable ($values);
	
	//if ($values) {
		$extraline && $optionlist .= "<OPTION VALUE=\"" . $extraline['value'] . "\" " . $extraline['checked'] . ">" . $extraline['label'] ."</OPTION>\n";		
		while (list($ID, $name) = each ($values)) { 
			$ID = trim($ID);
			$name = trim($name);
			in_array($ID, $checked) ? $check = " selected" : $check = "";
			$optionlist .= "<OPTION VALUE=\"$ID\" $check>$name</OPTION>\n";
			$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": OPTION VALUE=\"$ID\" $check $name<BR>\n";
		}
		if ($size === OPTION_LIST_IS_POPUP) {
			$size = "";
		} elseif (!$size) {
			count ($values) > 10 ? $size = OPTION_LIST_MAXSIZE : $size = count($values);
			$size = 'SIZE="' . $size . '" MULTIPLE';
		} else {
			$size = 'SIZE="' . $size . '" MULTIPLE';
		}
		$block = "\n<SELECT NAME=\"$listname\" $size $extrahtml>\n$optionlist</SELECT>\n";
// 	} else {
// 		$block = "";
// 	}
	
	return $block;

}

// Get array of subdirectories under $d, e.g. $d="/Users/dgross/Sites/fp"
function GetSubDirectoryTree ($d) {
	global $BASEDIR;

	$dirs = array();
	foreach ( glob("$d/*",GLOB_ONLYDIR) as $dir) {
		$dirs[$dir] = $dir;
		$dirs = array_merge ($dirs, GetSubDirectoryTree ($dir));
	}
	return $dirs;
}


?>