<?php
/**
 * TWG Flash uploader 3.2
 *
 * Copyright (c) 2004-2014 TinyWebGallery
 * written by Michael Dempfle
 *
 *
 *        This file start the session and handles 
 *        session name and external optional session
 *        implementations.   
 */
/**
 * ensure this file is being included by a parent file
 */
defined('_VALID_TWG') or die('Direct Access to this location is not allowed.');

// if you want to use a different session name use the next line:
session_name("fp_admin");
// If you use a different session name you should also add &session_name=<your session name>
// and change TFUSESSID below to your session name 

if (isset($_GET['TFUSESSID'])) { // this is a workaround if you set php_flag session.use_trans_sid=off + a workaround for some servers that don't handle sessions correctly if you open 2 instances of TFU
    session_id($_GET['TFUSESSID']);
}

// *DIG*
// this is a workaround if you set php_flag session.use_trans_sid=off + a workaround for some servers that don't handle sessions correctly if you open 2 instances of TFU
if (isset($_GET['fp_admin'])) {
    session_id($_GET['fp_admin']);
}

session_cache_limiter("private");
session_cache_limiter("must-revalidate");
session_start();

/* Includes your session file! Then updating is easier because your settings are not overwritten. 
   No my_tfu_session.php is included in the download! */
if (file_exists(dirname(__FILE__) . '/my_' . basename(__FILE__))) {
    include dirname(__FILE__) . '/my_' . basename(__FILE__);
}

include 'tfu_helper.php';

restore_temp_session(); // this restores a lost session if your server handles sessions wrong and increases the session time!
?>