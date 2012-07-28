<?php
/**
 * TWG Flash uploader 2.16.x
 *
 * Copyright (c) 2004-2012 TinyWebGallery
 * written by Michael Dempfle
 *
 *     This file does all file functions of TFU
 *
 *     If an image was detected: jpj, png or gif the images are resized to fit in
 *     the preview box (90 x 55). For all other files no image is returned!
 *
 *     +
 *       - Returns the file list to the flash.
 *       - Create dirs
 *       - Rename dirs
 *       - Delete dirs
 *       - Change dirs
 *       - Check what is possible in the current directory (permissions ...)
 *       - ...
 *
 *     All files from a directory are read and added to the return parameter
 *     &files. The first parameter is the size of the listing! The format is up to you
 *     The current format is e.g. "3 files (234k)" The dirtext parameter is added to
 *     the title bar of the flash
 *
 *     Authentification is done by the session $_SESSION["TFU_LOGIN"]. you can set
 *     this in the tfu_config.php or implement your own way!
 */
define('_VALID_TWG', '42');

if (isset($_GET['TFUSESSID'])) { // this is a workaround if you set php_flag session.use_trans_sid=off + a workaround for some servers that don't handle sessions correctly if you open 2 instances of TFU
    session_id($_GET['TFUSESSID']);
}
session_cache_limiter("private");
session_cache_limiter("must-revalidate");
session_start();

$install_path = ''; // do not change!
include 'tfu_helper.php';

restore_temp_session(); // this restores a lost session if your server handles sessions wrong and increases the session time!

include 'tfu_config.php';

// check if all included files have the same version to avoid problems during update!
if ($tfu_config_version != '2.16' || $tfu_help_version != '2.16') {
  tfu_debug('Not all files belong to this version. Please update all files.');
}

if (isset($_SESSION['TFU_LOGIN']) && isset($_SESSION['TFU_RN']) && isset($_GET['tfu_rn']) && ($_SESSION['TFU_RN'] ==    parseInputParameter($_GET['tfu_rn']))) {
    $dir = getCurrentDir();
    // if you have more complex filenames you can use the index
    $action = parseInputParameter($_GET['action']);

    if ($enable_enhanced_debug) {
      tfu_debug("Action:" . $action . "; Directory: " . $dir);
    }

    // The extra functionality for twg is on an exern class to make updating much easier
    if (file_exists('twg_plugin.php')) {
      include_once('twg_plugin.php');
      reset_twg_cache($action);
    }
    // end plugin

    if (isset($_GET['index']) && $action != 'dir') {
        // file functions!
        if ((isset($_GET['copyfolder']) && ($_GET['copyfolder'] == "true")) 
          || isset($_GET['createfile']) || isset($_GET['lastuploadinfo']) ) {
            $file = ""; // not needed for this task
        } else {
            $file = getFileName($dir); // returns an array if more than one is selected!
        }

        // plugin check for file operations before they are done!
        $plugins = glob("*_plugin.php");
        if ($plugins) {
          foreach ($plugins as $f) {
              include_once($f);
              if (function_exists(basename ($f,".php"). "_process_file")) {
                call_user_func(basename ($f,".php"). "_process_file" , $action, $file, $dir);
              }
          }
        }

        if ($action == 'rename') { // rename a file
            tfu_rename_file($dir, $file, $enable_file_rename, $keep_file_extension, $fix_utf8);
        } else if ($action == 'delete') { // delete a file
            tfu_delete_file($file, $show_delete);
        } else if ($action == 'xdelete') { // delete several files!
            tfu_delete_files($file, $show_delete);
        } else if ($action == 'copymove') { // copy move files!
            tfu_copy_move($dir, $file, $enable_file_copymove, $enable_folder_move );
        } else if ($action == 'preview') { // preview image
            tfu_preview($file);
        } else if ($action == 'info') { // checks if a preview can be done and the file size is returned.
            tfu_info($file);
        } else if ($action == 'lastuploadinfo') { // gets some infos about the last uploaded file!
            tfu_upload_info($dir);
        } else if ($action == 'text') { // get infos about a file
            tfu_text($file);
        } else if ($action == 'savetext') { // save a textfile
            tfu_savetext($file);
        } else if ($action == 'download') { // download a file - we set the header !
            tfu_download($file, $enable_file_download);
        } else if ($action == 'createThumb') { // create a thumbnail
            tfu_createThumb($file);
        } else if ($action == 'zipdownload') { // download multipe files as zip!
            tfu_zip_download($file, $enable_file_download);
        } else if ($action == 'createfile') { // creates an empty file during upload - if createfile is set an empty file is created + the directory has to be sent.
            $file = $dir . "/" . parseInputParameterFile(trim(my_basename(' ' . $_GET['newfile'])));
            $overwrite= (!isset($_GET['createfile'])); 
            tfu_savetext($file, $overwrite);
            if ($overwrite) {
              $_SESSION["TFU_LAST_UPLOADS"][] = $file;
            } 
        }
         // plugin check for file operations after they are done!
        if ($plugins) {
          foreach ($plugins as $f) {
            include_once($f);
            if (function_exists(basename ($f,".php"). "_after_process_file")) {
              call_user_func(basename ($f,".php"). "_after_process_file" , $action, $file, $dir);
            }
          }
        }

    } else if ($action == 'uploadcheck') {
        echo '&uploadcheck=' . ((isset($_SESSION['TFU_UPLOAD_REMAINING'])) ? $_SESSION['TFU_UPLOAD_REMAINING'] : '0');
    } else if ($action == 'ping') { // The flash sends a ping every 5 minutes to keep the session alive.
            echo 'pong';
    } else if ($action == 'preupload') {
          $_SESSION['TFU_PRE_UPLOAD_DATA'] = urldecode($_POST['data']);
    } else if ($action == 'dir') {
        // directory functions
        $myFiles = array();
        $myDirs = array();
        $status = ""; // this is the status flag I use to check if the actions where sucessful
        if (isset($_GET['getTreeXML'])) {
            echo get_tree_xml();
            return;
        }
        // Plugin check for folder operations: createdir, renamedir, deletedir. Will be refactored in the next version to be more generic!
        if (isset($_GET['createdir']) || isset($_GET['renamedir']) || isset($_GET['deletedir'])) {
          $plugins = glob("*_plugin.php");
          if ($plugins) {
            foreach ($plugins as $f) {
                include_once($f);
                if (function_exists(basename ($f,".php"). "_process_file")) {
                  $action = isset($_GET['createdir']) ? 'createdir' : (isset($_GET['renamedir']) ? 'renamedir' : 'deletedir');
                  call_user_func(basename ($f,".php"). "_process_file" , $action, '', $dir);
                }
            }
          }
        }
        if (isset($_GET['createdir'])) { // creates a directory
            $status = create_dir($dir, $enable_folder_creation, $fix_utf8);
        } else if (isset($_GET['renamedir'])) { // Rename a directory
            $status = rename_dir($dir, $enable_folder_rename, $fix_utf8);
        } else if (isset($_GET['deletedir'])) { // the check if the file can be deleted is done before - if it is not possible we never get here!
            $status = delete_folder($dir, $enable_folder_deletion, $fix_utf8);
        }
        // needed for browsing - we check if a [..] is possible - it is never allowed to go higher as the defined root!
        $show_root = (isset($_SESSION["TFU_ROOT_DIR"])) ? ($dir != $_SESSION["TFU_ROOT_DIR"]) : false;

        if (isset($_GET['changedir'])) { // Change a directory
           $dir = change_folder($dir, $show_root, $enable_folder_browsing, $exclude_directories, $sort_directores_by_date);
        }
        // needed for browsing - we check again because folder could have changed!
        $show_root = (isset($_SESSION["TFU_ROOT_DIR"])) ? ($dir != $_SESSION["TFU_ROOT_DIR"]) : false;

        // I reset the status cache before I read the directory and check the restrictions
        clearstatcache();

        // Read all files and folders
        $size = read_dir($dir, $myFiles, $myDirs, $fix_utf8, $exclude_directories, $sort_files_by_date, $sort_directores_by_date);
        // Sort files and folders
        sort_data($myFiles, $myDirs, $sort_files_by_date, $sort_directores_by_date);

        if ($show_root) {
            array_unshift($myDirs, "..");
        }
        $nrFiles = count($myFiles);
        // check restrictions like if files can be deleted or folders created or of the folder does exists
        $status = check_restrictions($dir, $show_root, $myFiles, $fix_utf8, $status);

        // we check if we have an error in the upload!
        if (isset($_SESSION["upload_memory_limit"]) && isset($_GET['check_upload'])) {
            $mem_errors = "&upload_mem_errors=" . $_SESSION["upload_memory_limit"];
            unset($_SESSION["upload_memory_limit"]);
        } else {
            $mem_errors = "";
        }

        $upload_ok = '&upload_ok=' .  ((isset($_SESSION["TFU_LAST_UPLOADS"])) ?  count($_SESSION['TFU_LAST_UPLOADS']) : '0');
        // This is not optimized! Because if you use sorting by date this is done twice! But this call is cached
        // and the code is cleaner this way. I add the size as the first 10 chars.
        if ($show_server_date_instead_size=='true') {
            $i = 0;
            foreach ($myDirs as $file) {
              $file = urldecode($file);
              if ($fix_utf8 == "") {
                $file = utf8_decode($file);
              }
              $myDirs[$i] = filemtime($dir . '/' . $file) . $myDirs[$i];
              $i++;
            }
        }
        $files = implode('|', $myFiles);
        $dirs = ($enable_folder_browsing == "true") ? implode("|", $myDirs) : "";

        $dirsub = create_directory_title($dir, $hide_directory_in_title, $truncate_dir_in_title , $fix_utf8);
        $currentdir = basename($dir); // currently only the last folder is shown
        $baseurl = "&baseurl=" . getRootUrl() . $dir . "/"; // the baseurl
        if ($fix_utf8 == "") {
            $baseurl = utf8_encode($baseurl); // the baseurl
        }
        if ($directory_file_limit_size != -1) {
           $status .= '&dir_size=' . getFoldersize($dir);
        }
        store_temp_session();
        $size = $nrFiles . " files (" . formatSize($size) . ")"; // formating of the display can be done here!
       echo "&tfufiles=" . $size . "|" . $files . "&tfudirs=" . $dirs . $status . "&currentDir=".$currentdir."&dirtext=" . $dirsub . $mem_errors . $upload_ok . $baseurl . '&last=true';
    } else {
        // shows an error message that the expected index was not send
        echo '&result=index&last=true';
    }
    store_temp_session();
} else if (isset($_GET['tfu_rn']) && isset($_GET['tfu_ut']) && ($_GET['tfu_ut'] == "-1" || $_GET['tfu_ut'] == "5")) {
    checkSessionTempDir($_GET['tfu_ut']);
} else {
    echo 'Not logged in!';
}
?>