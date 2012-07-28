<?php
/**
 * TWG Flash uploader 2.12.x
 *
 * Copyright (c) 2004-2010 TinyWebGallery
 * written by Michael Dempfle
 *
 *    This file is the login and stetup file of the flash.
 *
 *    Have fun using TWG Flash Uploader
 */
define('_VALID_TWG', '42');
if (isset($_GET['TFUSESSID'])) { // this is a workaround if you set php_flag session.use_trans_sid=off + a workaround for some servers that don't handle sessions correctly if you open 2 instances of TFU
    session_id($_GET['TFUSESSID']);
}
session_start();

$install_path = '';      // Please read the howto 8 of the TFU FAQ what you have to do with this parameter! You need a / at the end if you set it + you have to 
include $install_path . "tfu_helper.php";

restore_temp_session(true); // this restores a lost session if your server handles sessions wrong - only important for joomla because for TFU standalone nothing is in the session yet.

if (isset($_POST['twg_user']) && isset($_POST['twg_pass'])) { // twg_user and twg_pass are always sent by the flash! - never remove this part! otherwise everyone can call tfu_config directly
    /**
     * ----------------------------
     * Important!
     * ----------
     * You should add your authentification here if you don't use the internal one because everyone can send a
     * post request with twg_user - this is NOT a security check - it only checks if this parameter is set!
     * You should always protect your data as good as possible. If login = "true" everyone can upload even without
     * the flash by sending a request. Therefore if you work in a CMS or on your own webpage after a login
     * you have to add this check here as well!
     * If you use $login="auth" the check if it is a correct user is done below! You can add your user authentification
     * there too!
     * The simplest way is:
     * 1. Set a session variable after you have sucessfully logged in into your main website
     * (This is normally done anyway).
     * 2. Get this variable in tfu_config.php with $_SESSION['<your variable>'] and add this check to the line
     * where you find isset($_POST["twg_user"]).
     * By default this line looks like:
     * if ( isset($_POST["twg_user"]) && isset($_POST["twg_pass"])){
     * Afterwards something like:
     * if ( isset($_POST["twg_user"]) && isset($_POST["twg_pass"]) && isset($_SESSION['<your variable>'])){
     * You can of course do more than simply checking if the variable exists. You can e.g. get this variable and
     * check in your db is it is o.k. It's up to you and your existing system how you solve it!
     * Only be aware that you have to do something!
     * ----------------------------
     */

    /**
     * Start parameters - don't remove the parameters part - 
     * The paramters are needed even if you implement your own 
     * authentification It makes sure that the flash is the client     
     */
    $user = parseInputParameter($_POST['twg_user']);
    $pass = parseInputParameter($_POST['twg_pass']);
    $rn = parseInputParameter($_POST['twg_rn']);
    $rn = substr(session_id(), 0, 5) . $rn . session_id();

    include $install_path . "tfu_config.php";
    /**
     * end parameters - now you can implement your own authentification and autorisation
    */
     
    /**
     * AUTHENTIFICATION
     *
     * This part is interesting if you want to use the login!
     */
    /**
     * TFU has a very simply user managment included -
     * add users/folders/paths at .htusers.php.
     * The password is encrypted - please use the password generator that is included.
     * Read the "Important" part on top!
     */ 
    if (($login == "auth" || $login == "reauth") && $user != "__empty__" && $user != "") {
        include ($install_path . ".htusers.php");
        foreach ($GLOBALS["users"] as $userarray) {

            // you have to use sha1 encrypted passwords if you want to use the
            // included login mechanism - see the provided password generator.
            if ($user == $userarray[0] && $pass == $userarray[1]) {
                $login = "true";
                $folder = $userarray[2];
                if ($userarray[3] != "") {
                    $show_delete = $userarray[3];
                }
                if ($userarray[4] != "") {
                    $enable_folder_browsing = $enable_folder_creation = $userarray[4];
                    $enable_folder_deletion = $enable_folder_rename = $userarray[4];
                }
                break;
            } else {
                $login = "reauth";
            }
        }
    }

    /*
    Here the $login variable has to be finally set if you do your own authentification
    */
    
    // some dynamic settings need to be stored in the session
    setSessionVariables();
    // Sending and checking the registration infos - check is done in the flash therefore
    // we have to send part of the registration infos to the flash!
    $license_file = $install_path . "twg.lic.php";
    if (file_exists($license_file)) {
        ob_start();
        include $license_file;
        ob_end_clean();
        // we encrypt the license data since 1.7 to enhance security!
        $d = tfu_enc($d, $rn);
        $l = tfu_enc($l, $rn);
        $m = tfu_enc($m, $rn);
        $s = tfu_enc($s, $rn, 50);
        $reg_infos = "&d=" . $d . "&s=" . $s . "&m=" . $m . "&l=" . $l;
    } else {
        $reg_infos = ""; // means freeware version!
    }
    store_temp_session();
    // send the config data to the flash
    sendConfigData();
} else {
    include $install_path . "tfu_config.php";
    printServerInfo();
}
?>