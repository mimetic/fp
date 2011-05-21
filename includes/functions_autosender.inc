<?PHP

// AUTOSENDER FUNCTIONS

/*
This needs to replace FTPFiles and MailFiles. It should take the same input and instead of actually sending those files, it should put them into a sending queue.

	MailFiles ($projectID, $picturelist, $projectdir, $From, $FromName, $To, $ToName , $projectdirHIRES, $userID, $message);

	FTPFiles ($files, $sourceDir, $ftp_server, $directory, $newfolder, $ftp_user_name, $ftp_user_pass , $overwritefiles = false) {

Here's how we use add to the queue, where $image has image info and $order has sending info (ftp or email):
	$unique_filename = CopyImageToSendQueue ($image);
	WriteSendOrder ($unique_filename, $order, $queuefile, $logfile);


*/

function MailFilesToQueue ($projectID, $picturelist, $projectdir, $From, $FromName, $To, $ToName, $projectdirHIRES, $userID, $message, $logfile = '') {
	$letters = LettersToSend ($projectID, $picturelist, $projectdir, $From, $FromName, $To, $ToName , $projectdirHIRES, $userID, $message);
	$result = AddLettersToQueue ($letters, $logfile);
	return $result;
}



// Send files by Mail by dropping them into the queue.
// This will replace MailFiles, which sends them directly.

// LettersToSend
// Create an array of letters to send, given a project ID
// The project ID let's us find the pictures directory
function LettersToSend ($projectID, $picturelist, $projectdir, $From, $FromName, $To, $ToName , $projectdirHIRES, $userID, $message = "") {
	global $PHOTOS;
	global $msg, $error;
 	global $SYSTEMNAME, $FP_SYSTEM_EMAIL;

	$DEBUG = 0;
	$DEBUG && $msg .= __FUNCTION__."<BR>";
	
	// Array of messages to send
	$letters = array ();
	
	$result = "<HR width=\"150\" align=\"left\">\n\n<B>Email results:</B><BR>\n<small>";
	$project = GetRecord (PROJECTS_DB, $projectID);
	$subjectline = "";
	!empty ($project['Subject_line']) && ($subjectline = $project['Subject_line']);

	$k = 1;	//file counter
	if ($picturelist) {
		sort ($picturelist);
		$DEBUG && $msg .= "Picturelist: ".ArrayToTable ($picturelist);
		
		foreach ($picturelist as $picturename) {
			$Text = "";
			$picture = "$projectdir/$picturename";

			// If we're sending lores pictures, it's possible there's no picture
			// because the thumbnail wasn't created yet...perhaps user uploaded
			// with FTP.
			if (file_exists ($picture)) {
				$Subject = $subjectline;
				($Subject == "") && ($Subject = "$subjectline ($picturename from $FromName)");
				$Subject = str_replace("[name]", $picturename, $Subject);
				$Subject = str_replace("[from]", $FromName, $Subject);
				// don't try to send IPTC info about a contact sheet.
				if (preg_match ("/CONTACT/",$projectdirHIRES)) {
					$IPTC = "";
					$sheetname = str_replace (".jpg", "" , $picturename);
					$Text .= $sheetname;
				} else {
					$IPTC = FetchIPTCInfo ("$projectdirHIRES/$picturename");
					$Text .= "";
					while( list($key, $value) = each($IPTC)) {
						$value && $Text .= "$key : $value\n";
					}
					$Text && $Text = "IPTC PICTURE INFORMATION for $picturename:\n\n$Text";
					$Text = "Attached picture: $picturename\n-----------------------------------\n$Text";
				}
				$Text = "MESSAGE: $message\n$Text";
				$Html = "";
				// This adds only one file at a time.
				// I guess, we could do a loop to add multiple pictures. 
				// Use constants in config (PPF_EMAIL_LORES_MAXPIX and PPF_EMAIL_HIRES_MAXPIX)
				// to do this...later.
				$AttmFiles = array ($picturename);
				
				$DEBUG && $msg .= "IPTC: $Text<BR>\n";
				$DEBUG && $msg .= "pretending to SendMail($From, $FromName, $To, $ToName, $From, $FromName, $Subject, ..., $Html, $AttmFiles) <BR>\n";
				
				
				//$DEBUG || SendMail($From, $FromName, $To, $ToName, $Subject, $Text, $Html, $AttmFiles);
				
				$letters[] = array (
					'email_from'			=> $From,
					'email_from_name' 	=> $FromName,
					'email_to'			=> $To,
					'email_to_name'		=> $ToName,
					'email_subject'		=> $Subject,
					'email_text'			=> $Text,
					'email_html'			=> $Html,
					'project_dir'		=> $projectdir,
					'attm_files'			=> $AttmFiles
					);

				$result .= "$k) Sending <B>$picturename</B> to $ToName ($To)<BR>\n";
			} else {
				$result .= "$k) * $picturename: <i>Thumbnail/Lores version of not yet created, so nothing was sent.</i><BR>\n";
			}
		$k++;
		}
	}
	return $letters;
}



// Send files by FTP by dropping them into the queue.
// This will replace FTPFiles, which sends them directly.
function FTPFilesToQueue ($files, $sourceDir, $ftp_server, $directory, $newfolder, $ftp_user_name, $ftp_user_pass , $overwritefiles = false, $logfile = '') {
	global $msg, $error;
	
	$DEBUG = false;
	$result = '';

	$DEBUG && $error .= __FUNCTION__."<br>";
	
	$order = array ();
	$order['method'] = "ftp";
	$order['ftp_server'] = $ftp_server;
	$order['ftp_directory'] = $directory;
	$order['ftp_user_name'] = $ftp_user_name;
	$order['ftp_user_pass'] = $ftp_user_pass;
	
	sort ($files);
	
	foreach ($files as $filename) {
		$unique_filename = CopyImageToSendQueue ($filename, $sourceDir);
		if ($unique_filename) {
			$result .= WriteSendOrder ($unique_filename, $order, AS_SENDER_QUEUE, $logfile);
		} else {
			$error .= "Unable to copy $filename to the queue<br>";
		}
		if ($DEBUG) {
			$error .= __FUNCTION__.": Add $filename to queue for FTP: ftp=$ftp_server<BR>";
		}	

	}
	return $result;
}

/*
function SendMail ($From,$FromName,$To,$ToName,$Subject,$Text,$Html,$AttmFiles = array()) {
*/
function AddLettersToQueue ($letters = array (), $logfile = '' ) {
	global $msg, $error;
	
	$DEBUG = false;
	$DEBUG && $error .= __FUNCTION__;
	
	$result = '';

	foreach ($letters as $order) {		
		$order['method'] = "email";
		
		/*
		$order['email_from'] = $letter['email_from'];
		$order['email_from_name'] = $letter['email_from_name'];
		$order['email_to'] = $letter['email_to'];
		$order['email_to_name'] = $letter['email_to_name'];
		$order['email_subject'] = $letter['email_subject'];
		$order['email_text'] = $letter['email_text'];
		$order['email_html'] = $letter['email_html'];
		*/
		
		$order['logfile'] = $logfile;
		
		$DEBUG && $error .= "Email to {$order['email_to']}<BR>";
		$DEBUG && $error .= ArrayToTable ($order['attm_files']);

		foreach ($order['attm_files'] as $filename) {
			$unique_filename = CopyImageToSendQueue ($filename, $order['project_dir']);
			$unique_filename 
				? $result .= WriteSendOrder ($unique_filename, $order, AS_SENDER_QUEUE, $logfile)
				: $error .= "Unable to copy $filename to the queue<br>";
			if ($DEBUG) {
				$error .= __FUNCTION__.": Add $filename to queue for Email: to {$order['email_to']}....<BR>";
			}
		}
	}

	//	SendMail($From, $FromName, $To, $ToName, $Subject, $Text, $Html, $AttmFiles);
	return $result;
	
}


// Add to the "send file" file
// The file sending daemon will pick up this file and
// ftp/email files away to suppliers.
// This is modified from the FP version:
// - $image is the image info as an array
// - $order is the sending info
function WriteSendOrder ($filename, $order, $queuefile, $logfile) {
	global $msg, $error;
	
	$DEBUG = 0;
	
	$result = '';
	
	$DEBUG && $error .= __FUNCTION__.": queue file = $queuefile<BR>";
	
	if (!touch ($queuefile)) {
		$result .= __FUNCTION__.": Cannot write FTP package becase FTP information missing: server={$package['ftp_server']}; username={$package['ftp_user_pass']};password={$package['ftp_user_pass']};directory={$package['ftp_directory']}";
		return false;
	}
	
	$package = $order;
	$package['logfile'] = $logfile;
	$package['filename'] = $filename;
	$package['datetime'] = date ("Y-m-d h:m:s");

	// SEND BY EMAIL OR FTP
	if ($order['method'] == 'email') {
		// SEND BY EMAIL
		extract ($package);
		
		if ($order['email_from'] && $order['email_from_name'] && $order['email_to'] && $order['email_to_name'] && $order['email_subject'] && $order['attm_files']) {
			$packagetext = http_build_query ($package). "\n";
			if (file_put_contents (AS_SENDER_QUEUE, $packagetext, FILE_APPEND)) {
				$result .= "<div id=\"" . basename($filename) . "\"><i>Sending " . RealName($filename) . " by {$order['method']}.</i></div>\n";
				//fp_error_log("<i>Added " . RealName($source_file) . " to the queue.</i><BR>\n", false, true, 3, $logfile);
			} else {
				$result .= __FUNCTION__.": The file ".AS_SENDER_QUEUE." is missing or not writable";
			}
		} else {
			$result .= __FUNCTION__.": Cannot write Email package because Email information missing: email_from = {$order['email_from']} && email_from_name = {$order['email_from_name']} && email_to = {$order['email_to']} && email_to_name = {$order['email_to_name']} && email_subject = {$order['email_subject']} && image has image info = {$order['image has image info']} && attm_files = {$order['attm_files']}";
			return false;
		}
	} else {
		// SEND BY FTP
		if ($order['ftp_server'] && $order['ftp_user_name']) {
			$package['method'] = 'ftp';	// we fill this in, just in case
			$packagetext = http_build_query ($package). "\n";
			if (file_put_contents (AS_SENDER_QUEUE, $packagetext, FILE_APPEND)) {
				$result .= "<div id=\"" . basename($filename) . "\"><i>Sending " . RealName($filename) . " by {$order['method']}.</i></div>\n";
			} else {
				$result .= __FUNCTION__.": The file ".AS_SENDER_QUEUE." is missing or not writable";
			}
			
		} else {
			$result .= __FUNCTION__.": Cannot write FTP package because FTP information missing: server={$package['ftp_server']}; username={$package['ftp_user_pass']};password={$package['ftp_user_pass']};directory={$package['ftp_directory']}";
			return false;
		}
	}
	return $result;
}

// Adapted from the FP version
// $image is an array of the image info
function CopyImageToSendQueue ($filename, $sourceDir) {
	global $error, $msg;

	$DEBUG = 0;

	// prefix insures unique file names
	$uniquename = uniqid() . "___" . $filename;
	if ($filename) {
		$src = "$sourceDir/".$filename;
		if (file_exists($src) && !file_exists($dest)) {
			$dest = AS_SENDER_QUEUE_DIR."/".$uniquename;
			copy ($src, $dest) || $error .=  __FUNCTION__.": Failed to copy $src to $dest<br>";
			if ($DEBUG) {
				$msg .= __FUNCTION__.": $src -> $dest in queue<BR>";
			}	
			return $dest;
		} else {
			!file_exists($src) && $error .= "$src does not exist!<br>";
			file_exists($dest) && $error .= "$dest exists in the queue!<br>";
			return false;
		}
	} else {
		return false;
	}
}


// Return the original file name of a file to be sent. 
// We strip off the uniqid part of the name.
function RealName ($filename) {
	return preg_replace ("/^.*?___/", "", $filename);
}

?>