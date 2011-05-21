<?php
/* 
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 * Description of FPMail
 *
 * @author dgross
 */
class FPMail {

// ParseMail MIME object which holds the loaded message
    var $mime = null;

    // ParseMail structure array
    var $structure = array();

    // $pictures: an array of attached JPG filenames
    var $pictures = array();

    /* $info: array of commands and IPTC values, key=value
     *  In particular:
     * - email (source email...where the message came from
     * - subject
     */
    var $info = array();

    // $nickname: nickname of the project into which to put the pix
    var $nickname;
    // $userid: ID of the FP user who sent the mail, the default user for unmarked attachments
    var $userID;
    // $userRealName: name extracted from the "from" header field, if it exists
    var $userRealName;
    // Email of sender, from the header "return-path"
    var $email;
    // Email file to use for extraction
    var $emailFileName;

    // $verbose: for debugging. If on, more logging is done.
    var $verbose = true;

    function FPMail () {

    }


    // ===================
/*
 * Given a full path to a file of a mail message, set the PMail object
 * with values and the attachment file names
 * Extract any attachments into the $outputdir
 */
    function GetPictureFromEmailFile ($filename = null) {
	global $FP_EMAIL_FORCE_LOGIN;
	global $msg, $error;

	// If not filename provided, use $this->emailFileName or get a new file.
	if ($filename) {
	    $this->emailFileName = $filename;
	} else {
	    $this->emailFileName || $this->FindNextEmailFile();
	}

	if (!$this->emailFileName || !file_exists($this->emailFileName))
	    return false;

	fp_error_log(strtoupper(__FUNCTION__).": -----------".date ("Y-m-d h:m:s")."---------", 3, FP_PICTURES_LOG);
	$this->verbose && fp_error_log(__FUNCTION__.__LINE__.":  Process: {$this->emailFileName}", 3, FP_PICTURES_LOG);

	$outputdir = FP_BASEDIR."/".FP_DIR_MAILED_DIR;

	$this->mime = mailparse_msg_parse_file ($this->emailFileName);
	$this->structure = mailparse_msg_get_structure($this->mime);

	$this->pictures = array ();

	/* print a choice of sections */
	foreach($this->structure as $st) {
        /* get a handle on the message resource for a subsection */
	    $section = mailparse_msg_get_part($this->mime, $st);
        /* get content-type, encoding and header information for that section */
	    $data = mailparse_msg_get_part_data($section);
	    //var_dump ($data);
	    list ($type, $subtype) = explode("/", $data["content-type"]);

	    switch ($type) {
		case "image" :
		    $subtype == "jpeg" && $this->pictures[$st] = $data["disposition-filename"];
		    break;
		case "text" :
		// get a handle on the message resource for the desired part
		    $sec = mailparse_msg_get_part($this->mime, $st);
		    ob_start();
		    // extract the part from the message file and dump it to the output buffer
		    mailparse_msg_extract_part_file($sec, $this->emailFileName);
		    $contents = ob_get_contents();
		    ob_end_clean();
		    $this->info = array_merge ($this->info, $this->ExtractFPEmailCommands ($contents));
		    break;
		case "multipart" :
		    $data['headers']['subject'] && $this->nickname = trim($data['headers']['subject']);
		    // Get user name from "from"
		    $matches = array ();
		    $res = preg_match("/(.*)\<(.*)\>/", trim($data['headers']['from']), $matches);
		    $res && $this->userRealName = trim($matches[1]);
		    $res && $this->email = trim($matches[2]);
		    break;
	    }
	}

	$user = $this->FindUser();

	if ($FP_EMAIL_FORCE_LOGIN && !$user->userID) {
	    fp_error_log(__FUNCTION__.": Someone tried to email a picture, but the username/password login failed (FP_EMAIL_FORCE_LOGIN is set).", 3, FP_ERROR_LOG);
	    unlink ($this->emailFileName);
	} else {

	    $subdir = $this->FindSubDirectory($user);
	    if ($subdir)
		$outputdir .= "/$subdir";

	    // If the upload directory is missing, create it.
	    file_exists ($outputdir) || mkdir ($outputdir, 0755);

	    // If we have a user, then get correct input directories for attachments
	    // Non-JPG files go into the user's files directory
	    if ($user->userID) {
		$userfiles = GetUserDirectory ($user->userID) . FP_SUBDIR_USER_FILES;
		$this->userID = $user->userID;
		$this->info['userID'] = $this->userID;
	    }

	    // Write images and supporting data files for each
	    // We will write a serialized array of username, etc.
	    // *** DO THIS ***
	    while (list($st, $fn) = each ($this->pictures)) {
		$fn = UniqueFilename ($outputdir, $fn);
	    /* get a handle on the message resource for the desired part */
		$sec = mailparse_msg_get_part($this->mime, $st);
		ob_start();
		// extract the part from the message file and dump it to the output buffer
		mailparse_msg_extract_part_file($sec, $this->emailFileName);
		$contents = ob_get_contents();
		ob_end_clean();
		file_put_contents ($outputdir."/".$fn, $contents);
		$this->WritePictureInfoFile ($outputdir."/".$fn);
	    }

	    // Write commands list to the log
	    $commandstext = "";
	    while (list($k,$v) = each($this->info)) {
		$commandstext .= "\n\t$k = $v;";
	    }
	    $this->verbose && fp_error_log(__FUNCTION__.__LINE__.":  Commands: $commandstext", 3, FP_PICTURES_LOG);

	    // Delete the mail file
	    unlink ($this->emailFileName);
	    $pixlist=join (", ", $this->pictures);
	    $logmsg = ":  Finished extracting email: User={$user->Fullname}, Email={$this->email}, ProjectID=$subdir, Pictures=$pixlist";
	    $this->verbose && fp_error_log(__FUNCTION__.__LINE__.$logmsg, 3, FP_PICTURES_LOG);
	}
    }

   /* We will create a serialized file with the same name
    * for FindOneImageAssignment will look for, with the same name as the file but with
    * a .txt extension.
    */
    function WritePictureInfoFile ($filename) {
	$info = $this->info;
	$output = serialize($this->info);
	$filename = str_ireplace (".jpg",".txt",$filename);
	file_put_contents($filename, $output);
    }

   /*
    *
    * Find the user for this email.
    * Return an FPUser object (a user).
    * If $FP_EMAIL_FORCE_LOGIN is true, only accept username/password as identification.
    * Get the artist's record.
    * 1. If username/password is set, use that.
    * 2. If the email matches a user's email, use that.
    * 3. If the name in the 'from' matches, uses that.
    * If we don't have a user name, keep going and hope the file has the user info
    * embedded in the IPTC fields.
    */
    function FindUser () {
	global $FP_EMAIL_FORCE_LOGIN;

	$user = new FPUser();
	if ($this->username && $this->password) {
	    $found = $user->login($this->username, $this->password, false, true);
	}

	if (!$found && !$FP_EMAIL_FORCE_LOGIN) {
	    if (!$user->userID && $this->email) {
		$user->fetchByEmail($this->email);
	    }
	    if (!$user->userID && $this->userRealName) {
		$artist = FindArtistByName ($this->userRealName);
		$artist && $user->loadUser($artist['ID']);
	    }
	    $user->userID && ($this->verbose && fp_error_log(__FUNCTION__.__LINE__.":  Found user without Login command", 3, FP_PICTURES_LOG));
	}

	$user->userID || $user=null;
	if ($this->verbose) {
	    $user
		? $logmsg = "Found user {$user->Fullname} ({$user->userID})."
		: $logmsg = "No user found for {$user->Fullname} : {$this->username}/{$this->password}.";
	    fp_error_log(__FUNCTION__.__LINE__.":  $logmsg", 3, FP_PICTURES_LOG);
	}
	$user->userID && $user->loadUser();
	return $user;
    }

    /*
     * FindSubDirectory
     * Find the project subdirectory, inside the upload folder, in which to
     * put the attached picture files.
     */
    function FindSubDirectory ($user = null) {
    // Get Project upload directory. Default is the general input dir.

	$projectID = null;

	// Do we have a project nickname?
	if ($this->nickname) {
	    $this->verbose && fp_error_log(__FUNCTION__.__LINE__.":  Nickname: {$this->nickname}", 3, FP_PICTURES_LOG);
	    $projectID = FindProjectByName ($this->nickname);
	}
	// If no project ID, then get the user's home project
	if (!$projectID && $user->userID)
	    $projectID = $user->get_property('ProjectID');

	$this->verbose && fp_error_log(__FUNCTION__.__LINE__.":  Project ID: $projectID", 3, FP_PICTURES_LOG);
	return $projectID;
    }

/*
 * Extract commands and values from a text
 * Key/Values are in the format, key[=:]value, on its own line.
 * A special case is username/password, which is of the form username/password
 * We capitalize each key, e.g. "date" becomes "Date" because that's how IPTC fields
 * are name.
 */
    function ExtractFPEmailCommands ($text) {
	$matches = array();
	$commands = array();

	// strip HTML codes
	$text = preg_replace ("/<br\s*\/?>/i","\n",$text);
	$text = preg_replace ("/(<\/?.*?>\s*)/s","",$text);
	$text = preg_replace ("/(&nbsp;)+/i"," ",$text);


	$s = "/(\w*?) *[\:\=\|] *(.*)[\n\r]+/";
	if (preg_match_all ($s, $text, $matches)) {
	    for ($i=0;$i<count($matches[1]);$i++) {
		$k = trim(ucfirst(strtolower($matches[1][$i])));
		if ($k && !is_numeric($k)) {
		    $commands[$k] = trim($matches[2][$i]);
		    //$this->verbose && fp_error_log(__FUNCTION__.__LINE__.":  Found $k = ".$commands[$k], 3, FP_PICTURES_LOG);
		}
	    }
	}

	// only get it once...second time seems to screw up.
	if (!$this->username && $commands['Login']) {
	    list ($this->username, $this->password) = explode("/", $commands['Login']);
	    fp_error_log(__FUNCTION__.__LINE__.":  Found username/password: {$this->username}/{$this->password}", 3, FP_PICTURES_LOG);
	}

	// IPTC uses first letter capitalized or camel-case.
	// We've simplified, and we only look at single word names, e.g. Date, so we're OK if we capitalize.

	// Fill in my standard name with the alternative names, whatever the user used
	// Credit, Author, Byline => Credit
	$names = array("Credit", "Author", "Byline");
	foreach ($names as $k) {
	    if ($commands[$k]) {
		foreach ($names as $i) {
		    !$commands[$i] && $commands[$i]=$commands[$k];
		}
	    }
	}

	// Caption, Description => Caption
	$commands['Description'] && $commands['Caption'] = $commands['Description'];

	// Caption, Description => Caption
	$commands['Date'] && $commands['CreatedDate'] = $commands['Date'];

	return $commands;
    }

    /*
     * Look in the email inbox and find the next email message
     * Set $this->emailFileName to the next file to work on
     */
    function FindNextEmailFile () {
	global $FP_EMAIL_MAILDIR;

	$mailpath = $FP_EMAIL_MAILDIR;

	if (is_dir ($mailpath)) {
	    if ($dh = opendir($mailpath)) {
		while (($file = readdir($dh)) != false) {
		    if ($file[0] != "." && is_file("$mailpath/$file"))
			break;
		}
		closedir ($dh);
	    }
	    $file && $this->emailFileName = "$FP_EMAIL_MAILDIR/$file";
	    if ($file) {
		//$this->verbose && fp_error_log(__FUNCTION__.__LINE__.":  Looking for mail in $mailpath, found $file", 3, FP_PICTURES_LOG);
	    }
	} else {
	    $this->emailFileName = null;
	    return false;
	}
    }
}