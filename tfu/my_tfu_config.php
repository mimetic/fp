<?php
/**
 * TWG Flash uploader 2.9.x
 User configuration file. Values overwrite those in tfu_config.php
*/

/*

**** WHEN UPDATING TFU, BE SURE TO MODIFY TFU_UPLOAD.PHP!!! ****
Set the session name to "fp_admin" on line 31!
session_name("fp_admin");

*/


session_write_close();

$prevSessionName = session_name("fp_admin") || "";
session_start();
$folder = $_SESSION["tfu_upload_dir"];
$folder || tfu_debug("ERROR! The folder from session is empty!");
session_name($prevSessionName);


#tfu_debug("Upload folder is $folder");

if (isset($_SESSION["tfu_upload_extensions"]) && $_SESSION["tfu_upload_extensions"]) {
	$allowed_file_extensions = $_SESSION["tfu_upload_extensions"];
} else {
	$allowed_file_extensions = 'jpg,jpeg';
}
$show_preview = false; // Show the small preview. Valid is 'true' and 'false' (Strings!) - the function is_gd_version_min_20 checks if the minimum requirements for resizing images are there!
$show_big_preview = 'false'; // Show the big preview - clicking on the preview image shows a bigger preview
$show_delete = 'true'; // Shows the delete button - if download is set to button this moves to the menu!
$enable_folder_browsing = 'false'; // Without browsing creation and deletion is disabled by default!
$enable_folder_creation = 'false'; // Show the menu item to create folders
$enable_folder_deletion = 'false'; // Show the menu item to delete folders - this works recursive!
$enable_folder_rename = 'false'; // Show the menu item to rename folders
$enable_file_rename = 'false'; // Show the menu item to rename files - default is false because this is a security issue
$use_image_magic = false; // You can enable image magick support for the resize of the upload. If you know that you have image magic on your server you can set this to true. image magick uses less memory then gd lib and it does copy exif information!
$normalise_file_names = true;       // new 2.7.5 - This setting convertes all filenames to lowercase and special characters are removed e.g. !"#$%&'()*+,-- []\^_`     are replaces with an _
$directory_file_limit = '200'; // you can specify a maximum number of files someone is allowed to have in a folder to limit the upload someone can make! - only available in the registered version!
$queue_file_limit = '200'; // you can specify a maximum number of files someone can upload at once! - only available in the registered version!
$queue_file_limit_size = '200'; // you can specify the limit of the upload queue in MB! - only available in the registered version!
$hide_help_button = 'false'; // since TFU 2.5 it is possible to turn off the ? (no extra flash like before is needed anymore!) - it is triggered now by the license file! professional licenses and above and old licenses that contain a TWG_NO_ABOUT in the domain (=license for 20 Euro) enable that this switch is read - possible settings are 'true' and 'false'
$enable_file_download = 'true'; // You can enable the download of files! valid entries 'true', false', 'button', 'button1' - 'button' show the dl button insted the menu button if all other elements of the menu are set to false - 'button1' shows the download button instead of the delete button and the delete function is moved to the menu if enabled! - only available in the registered version!
$enable_folder_move = 'false'; // New 2.6 - Show the menu item to move folders - you need a professional license or above to use this
$enable_file_copymove = 'false'; // New 2.6 - Show the menu item to move and copy files - you need a professional license or above to use this
$normalizeSpaces='true';            // new 2.9 - if you enable normalize file names or directory names you can decide here if spaces are replaces with an _ or not.
$exclude_directories = array('.htaccess','index.html', '.DS_Store', 'data.pxp', '_vti_cnf', '.svn', 'CVS', 'thumbs'); // new 2.6 - You can enter directories here that are ignored in TFU. You can enter files as well that should be hidden!
$hide_directory_in_title = 'true';  // You can disable the display of the upload dir in the title bar if you set this to 'true'   

$use_image_magic = true;            // You can enable image magick support for the resize of the upload. If you know that you have image magic on your server you can set this to true. image magick uses less memory then gd lib and it does copy exif information!
$normalise_directory_names = true;   // new 2.8.1 - This setting convertes all directory names that are created or renamed to lowercase and special characters are removed e.g. !"#$%&'()*+,-- []\^_`ˆ‰¸ﬂ are replaces with an _
$hide_hidden_files = true;          // New 2.10.6 - You can hide hidden files and directories in the remote view. All files and folders starting with a . are hidden if you set this to true. 

$resize_data = '10000,1500,1024,800'; // The data for the resize dropdown
    $resize_label = 'Original,1500px,1024px,800px'; // The labels for the resize dropdown

$show_full_url_for_selected_file = 'false'; // 'true' - if you use this parameter the link to the selected file is shown - can be used for direct links - only available in the registered version!

$keep_internal_session_handling = true;  // new 2.7.5 - TFU can detect servers with session problems. And it removes the session_cache folder it it is not needed. If you set this to true the session_Cache folder is not removed automatically. You should set this to true if you have only sometimes problems with the upload!

?>