<?php
/**
 * TWG Flash uploader 3.0
 *
 * Copyright (c) 2004-2013 TinyWebGallery
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

if (isset($_GET['TFUSESSID'])) { // this is a workaround if you set php_flag session.use_trans_sid=off + a workaround for some servers that don't handle sessions correctly if you open 2 instances of TFU
    session_id($_GET['TFUSESSID']);
}

// DIG: SET THE SESSION TO THE SAME AS THE ONE ADMIN USES.
session_name("fp_admin");


session_cache_limiter("private");
session_cache_limiter("must-revalidate");
session_start();

/* Includes your session file! Then updating is easier because your settings are not overwritten. 
   No my_tfu_session.php is included in the download! */
if (file_exists(dirname(__FILE__) . '/my_' . basename(__FILE__))) {
    include dirname(__FILE__) . '/my_' . basename(__FILE__);
}
?>