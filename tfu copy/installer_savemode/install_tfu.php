<?php
/*************************  
  @HEADER@
  $Date: 2007-02-02 11:11:13 +0100 (Fr, 02 Feb 2007) $
  $Revision: 39 $
**********************************************/

define( '_VALID_TWG', '42' ); 
@session_start();
umask(0000); // otherwise you cannot delete files anymore with ftp if you are no the owner!


if (isset($_GET['remove'])) {
   if (isset($_GET['remove'])) {
     remove("tfu");
     unlink("install_tfu.sm.php");
     echo "TFU removed";   
   } else {
     copy("install_tfu.php", "install_tfu.sm.php");  
     header("Location: install_tfu.sm.php?remove=true&sm=true");
     exit;
   }
   
} else {
  if (!file_exists("install_tfu.sm.php")) {
    copy("install_tfu.php", "install_tfu.sm.php");
    copy("install.lib.php", "install.lib.sm.php");
    copy("tfu.zip", "tfu.sm.zip");
    mkdir("tfu",0777);
    header("Location: install_tfu.sm.php");
    exit;
  } else {
    require_once("install.lib.sm.php");
    $archive_name = realpath("tfu.sm.zip");
    $zip = new PclZip($archive_name);
    $res = $zip->extract(PCLZIP_OPT_PATH, "./tfu");
    unlink("install_tfu.sm.php");
    unlink("install.lib.sm.php");
    unlink("tfu.sm.zip"); 
    echo "TFU installed.";
  }
}

function remove($item) // remove file / dir
{
    $item = realpath($item);
    $ok = true;
    if (is_link($item) || is_file($item))
        $ok = unlink($item);
    elseif (is_dir($item)) {
        $handle = opendir($item);
        while (($file = readdir($handle)) !== false) {
            if (($file == ".." || $file == ".")) continue;

            $new_item = $item . "/" . $file;
            if (is_dir($new_item)) {
                $ok = remove($new_item);
            } else {
                $ok = unlink($new_item);
            } 
        } 
        closedir($handle);
        $ok = @rmdir($item);
    } 
    return $ok;
} 
?>