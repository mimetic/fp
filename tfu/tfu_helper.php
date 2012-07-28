<?php
/**
 * TWG Flash uploader 2.16.x
 *
 * Copyright (c) 2004-2012 TinyWebGallery
 * written by Michael Dempfle
 *
 *
 *        This file has all the helper functions.
 *        Normally you don't have to modify anything here.
 *        Only the timezone can be interesting for you: $timezone
 */
/**
 * * ensure this file is being included by a parent file
 */
defined('_VALID_TWG') or die('Direct Access to this location is not allowed.');
$tfu_help_version = '2.16';
// some globals you can change
$check_safemode = true;              // New 2.12.x - By default TFU checks if you have a safe mode problem. On some server this test does not work. There you can try to turn it off and test if you can e.g. create directories, upload files to new created directories.
$session_double_fix = false; // this is only needed if you get errors because of corrupt sessions. If you turn this on a backup is made and checked if the first one is corrupt
$timezone = ''; // Please set your timezone here if you have problems with timezones - if you need exact times - enter your timezone - see http://www.dynamicwebpages.de/php/timezones.php
if (function_exists('date_default_timezone_set')) { // php 5.1.x
        if ($timezone != '') {
          @date_default_timezone_set($timezone);
        } else if (function_exists('date_default_timezone_get')) {
          set_error_handler('on_error_no_output');
          @date_default_timezone_set(@date_default_timezone_get());
          set_error_handler('on_error');
        } else {
          @date_default_timezone_set('Europe/Berlin');
        }
}
// default settings you should normally not change.
$bg_color_preview_R = 255;
$bg_color_preview_G = 255;
$bg_color_preview_B = 255;
$input_invalid = false;
$old_error_handler = false;
$master_profile = false;
$debug_file = dirname(__FILE__) . "/tfu.log"; // can be overwritten in the config!

tfu_setHeader();

@ob_start();

include dirname(__FILE__) . '/tfu_zip.class.php';

// check if all included files have the same version to avoid problems during update!
if ($tfu_zip_version != '2.16') {
  tfu_debug('Not all files belong to this version. Please update all files.');
}

/**
 * * Needed for Https and IE!
 */
function tfu_setHeader()
{
    // header("Pragma: public");
    // header("Expires: 0");
    // header("Cache-Control: must-revalidate, post-check=0, pre-check=0");
    // header('Pragma: I-hate-internet-explorer');
    // header('Cache-Control:no-store');
     if (strstr($_SERVER['HTTP_USER_AGENT'], "MSIE"))
    {
      header('Pragma: private');
      header('Cache-Control: private');
    }
    else
    {
      header('Pragma: public');
      header('Cache-Control: no-store, no-cache, must-revalidate' );
      header('Cache-Control: post-check=0, pre-check=0', false );
    }

    header('Expires: Sat, 26 Jul 1997 05:00:00 GMT' );
    header('Last-Modified: ' . gmdate( 'D, d M Y H:i:s' ) . ' GMT' );
    header('Vary: User-Agent');

}
/**
 * function:tfu_debug()
 */
function tfu_debug($data)
{
    global $debug_file; // set in the tfu_config.php or is overwritten by the twg config
    global $enable_enhanced_debug;
    	$data = replaceInput($data); // we check output data too - you never know!
	    $input_invalid = false;

      if(stristr($data, 'called statically') === false && stristr($data, 'deprecated') === false) { // This error can happen in Joomla and can be ignored
        $debug_string = date('m.d.Y G:i:s') . ' - ' . $data . "\n";
	      if ($enable_enhanced_debug) {
          $debug_string .= '    Request: ' . $_SERVER['PHP_SELF'] . '?' . $_SERVER['QUERY_STRING'] . "\n";
          foreach (debug_backtrace() as $element) {
            $debug_string .= '      Stack: ' . basename($element['file']) . ":" . $element['line'] . ":" . $element['function'];
            foreach ($element['args'] as $par) {
              if (is_array($par)) {
                $par = str_replace("\n", "", print_r($par, true));
              }
              $debug_string .= ":" . substr($par, 0, 100); // max 100 chars
            }
            $debug_string .= "\n";
          }
          if (function_exists("memory_get_usage")) {
            $debug_string .= "    Current memory usage: ". floor(memory_get_usage() / 1024)." KB\n";
          } else {
          $debug_string .=   "    Current memory usage: memory_get_usage not available.\n";
          }
        }
        if ($debug_file == '') {
		      @ob_start();
              @error_log($debug_string, 0);
		      @ob_end_clean();
		      return;
	      }

        if (file_exists($debug_file)) {
            if (filesize($debug_file) > 2000000) { // debug file max = 2MB !
                // we move the old one and start a new one - but only once!
                rename (dirname(__FILE__) . '/tfu.log', dirname(__FILE__) . '/tfu.log.bak');
                $debug_file_local = fopen($debug_file, 'w');
            } else {
                $debug_file_local = fopen($debug_file, 'a');
            }
            fputs($debug_file_local, $debug_string);
            fclose($debug_file_local);
        } else {
            if (is_writeable(dirname(__FILE__))) {
                if (!isset($debug_file)) { // if helper is included somewhere else!
                  $debug_file = dirname(__FILE__) . "/tfu.log";
                }
                $debug_file_local = @fopen($debug_file, 'a');
                @fputs($debug_file_local, $debug_string);
                @fclose($debug_file_local);
                clearstatcache();
            } else {
                @ob_start();
                @error_log($debug_string, 0);
		        @ob_end_clean();
            }
        }
    }
}

function on_error($num, $str, $file, $line)
{
    if ((strpos ($file, 'email.inc.php') === false) && (strpos ($str, 'fopen') === false) && (strpos ($str, 'Deprecated') === false)) {
        tfu_debug("ERROR $num in " . substr($file, -40) . ", line $line: $str");
    }
}

function on_error_no_output($num, $str, $file, $line)
{
}

if (!isset($skip_error_handling)) {
  @ini_set('display_errors','On');
  $old_error_handler = set_error_handler("on_error");
}


/**
 * Resizes a jpg/png/gif file if needed and stores it back to the original location
 * Needs gdlib > 2.0!
 * All other files are untouched
 * 1 = ok
 * 0 = failed
 * 2 = unknown - we retry after the save later.
 *
 */
function resize_file($image, $size, $compression, $image_name, $dest_image = false)
{
    global $use_image_magic, $image_magic_path, $enable_upload_debug, $store;

    if (!isset($store)) {
      $store = 0;
    }
    set_error_handler('on_error_no_output');
    ini_set('gd.jpeg_ignore_warning', 1); // since php 5.1.3 this leads that corrupt jpgs are read much better!
    set_error_handler('on_error');
    // we can do some caching here! - nice for 2.6 ;).
    if ($size == 'undefined') {
      tfu_debug('Resize: ERROR - No size is sent from the flash. Make sure that you have at least one value entered in the config. The image is NOT resized.');
      return 1;
    }


    $srcx = 0;
    $srcy = 0;
    if ($enable_upload_debug) { tfu_debug('Resize: Preparing to resize "' . $image . ' with size: '.$size.'"'); }
    if (file_exists($image)) {
        $oldsize = getimagesize($image);
        if ($oldsize[0] == 0) {
            // for broken images we try to read the exif data!
            $oldsize = get_exif_size($image, $image_name);
        }
        $oldsizex = $oldsize[0];
        $oldsizey = $oldsize[1];

        if (strpos($size, "x") !== false) {
           $s = explode("x", $size);
           $width =  $s[0];
           $height = $s[1] ;
        } else {
             if (($oldsizex < $size) && ($oldsizey < $size)) {
                 if ($enable_upload_debug) { tfu_debug('Resize: Image ('. $oldsizex .'x' .$oldsizey. ') is not resized with setting "' . $size . '"'); }
                 return 1;
             }
             if ($oldsizex > $oldsizey) { // querformat - this keeps the dimension between horzonal and vertical
                 $width = $size;
                 $height = ($width / $oldsizex) * $oldsizey;
             } else { // hochformat - this keeps the dimension between horzonal an vertical
                 $height = $size;
                 $width = ($height / $oldsizey) * $oldsizex;
             }
             $width =  round($width);
             $height = round($height);
        }
        if ($use_image_magic) {
            if ($enable_upload_debug) {
              tfu_debug("Resize: Image magick is used");
            }
            $ima = $loc_ima = realpath($image);
            $resize = $width . 'x' . $height;
            if ($dest_image) {
              $store = 1;
              $loc_ima = $dest_image;
            } else {
              if ($store == 2) { // 1st attempt was not o.k. - we try it with a backup name.
                $loc_ima = $ima . '.bak';
              }
            }
            $command = $image_magic_path . ' "' . $ima . '" -quality ' . $compression . ' -resize ' . $resize . ' "' . $loc_ima . '"';
            if ($enable_upload_debug) {
              tfu_debug("Resize: Image magick command: " . $command);
            }
            execute_command ($command);
             // we check if the resize was o.k.
              if ($store == 0) { // we do the resize to the same image again
                 // we check if the resize was o.k.
                $newsize = getimagesize($ima);
                if ($width != $newsize[0]) { // resize failed for unknown reasons.
                   if ($enable_upload_debug) { tfu_debug("Resize: Image could not be resized."); }
                   return 2; // we try to resize again later!
                } else {
                   return 1;
                }
              } else { // we resize to a temp file
                if (file_exists($ima . '.bak')) {
                $newsize = getimagesize($ima . '.bak');
                 if ($width != $newsize[0]) { // resize failed for unknown reasons.
                   if ($enable_upload_debug) { tfu_debug("Resize: Image could not be resized - wrong size"); }
                   unlink($ima . '.bak');
                   return 2; // we try to resize again later!
                 } else { // resize is o.k.
                   unlink($ima);
                   rename($ima . '.bak',$ima);
                   return 1;
                }
                } else {
                  if ($enable_upload_debug) { tfu_debug("Resize: Image could not be resized in temp upload dir. Retry after move to final destination."); }
                  return 2;
                }
              }
        } else {
            if ($enable_upload_debug) { tfu_debug("Resize: gd-lib is used."); }
            if (!isMemoryOk($oldsize, $size, $image_name, true)) {
                return 0;
            }
            if ($enable_upload_debug) { tfu_debug("Resize: memory seems o.k."); }

            $src = get_image_src($image, $oldsize[2]);
            if (!$src) {
                tfu_debug('File ' . $image_name . ' cannot be resized!');
                return false;
            }
            $dst = ImageCreateTrueColor($width, $height);
            imagecopyresampled($dst, $src, 0, 0, $srcx, $srcy , $width, $height, $oldsizex, $oldsizey);
            @imagedestroy($src);

            if ($dest_image) {
                $image = $dest_image;
            }
            if ($enable_upload_debug) { tfu_debug("Resize: image was resampled."); }

            if ($oldsize[2] == 1) { // gif
                $res = imagegif($dst, $image);
            } else if ($oldsize[2] == 2) { // jpg
                $res = imagejpeg($dst, $image, $compression);
            } else if ($oldsize[2] == 3) { // png
                $res = imagepng($dst, $image);
            } else {
                $res = imagejpeg($dst, $image, $compression);
            }
            if ($res) {
                // we check if the resize was o.k.
                $newsize = getimagesize($image);
                if ($width != $newsize[0]) { // resize failed for unknown reasons.
                     if ($enable_upload_debug) { tfu_debug("Resize: Image could not be resized."); }
                     return 2;
                }
                if ($enable_upload_debug) { tfu_debug("Resize: Image was saved and resized."); }
                @imagedestroy($dst);
                return 1;
            } else {
                tfu_debug('cannot save: ' . $image);
                return 0;
            }
        }
    } else
        return 2;
}

/**
 * resizes a file and writes it back to the user! - can do jpg, png and gif if the support is there !
 * renamed png's (that that are actually jpg's are handled as well!)
 * Needs gdlib > 2.0!
 */
function send_thumb($image, $compression, $sizex, $sizey, $generateOnly = false)
{
    global $bg_color_preview_R, $bg_color_preview_G, $bg_color_preview_B;
    global $info_text, $info_textcolor_R, $info_textcolor_G, $info_textcolor_B, $info_font, $info_fontsize;

    set_error_handler('on_error_no_output');
    ini_set('gd.jpeg_ignore_warning', 1); // since php 5.1.3 this leads that corrupt jpgs are read much better!
    set_error_handler('on_error');
    $srcx = 0;
    $srcy = 0;
    $dimx = $sizex;
    $dimy = $sizey;
    $usethumbs = false;

    if (file_exists(dirname(__FILE__) . '/thumbs') && is_writable(dirname(__FILE__) . '/thumbs')) { // is a caching dir available and writeable?
        $cachename = dirname(__FILE__) . '/thumbs/' . sha1($image . $sizex) . '.jpg';
        $usethumbs = true;
    }

    if ($usethumbs && file_exists($cachename)) {
        // we return the jpg!
        header('Content-type: image/jpg');
        header('Content-Length: ' . filesize($cachename));
        $fp = fopen($cachename, 'rb');
        while ($content = fread($fp, 8192)) {
            print $content;
        }
        fclose($fp);
        return true;
    } else if (file_exists($image)) {
        if (filesize($image) == 0) {
          return false;
        }
        $oldsize = getimagesize($image);
        // for broken images we try to read the exif data!
        if ($oldsize[0] == 0) {
            $oldsize = get_exif_size($image, $image);
        }
        $oldsizex = $oldsize[0];
        $oldsizey = $oldsize[1];

        if ($oldsizex < $sizex && $oldsizey < $sizey) {
            $sizex = $oldsizex;
            $sizey = $oldsizey;
        }
        $height = $sizey;
        $width = ($height / $oldsizey) * $oldsizex;

        if ($width > $sizex) {
            $width = $sizex;
            $height = ($width / $oldsizex) * $oldsizey;
        }

        if (isMemoryOk($oldsize, $sizex, '')) {
            $src = get_image_src($image, $oldsize[2]);
            if (!$src) { // error in image!
                if ($sizex < 100) {
                    // we return an empty white one ;).
                    $src = ImageCreateTrueColor($oldsizex, $oldsizey);
                    $back = imagecolorallocate($src, 255, 255, 255);
                    imagefilledrectangle($src, 0, 0, $oldsizex, $oldsizex, $back);
                }
                tfu_debug($image . ' is not a valid image - please check the file.');
                return false;
            }
            // $dst = ImageCreateTrueColor($width, $height);
            $dst = ImageCreateTrueColor($dimx, $dimy);
            if ($dimx < 100) { // white bg for small preview
                $back = imagecolorallocate($dst, $bg_color_preview_R, $bg_color_preview_G, $bg_color_preview_B);
            } else { // gray bg for big preview
                $back = imagecolorallocate($dst, 245, 245, 245);
            }
            imagefilledrectangle($dst, 0, 0, $dimx, $dimy, $back);
            if ($dimx > 100) { // border
                imagerectangle ($dst, 0, 0, $dimx-1, $dimy-1, imagecolorallocate($dst, 160, 160, 160));
            }

            $offsetx = 0;
            $offsetx_b = 0;
            if ($dimx > $width) { // we have to center!
                $offsetx = floor(($dimx - $width) / 2);
            } else if ($dimx > 100) {
                $offsetx = 4;
                $offsetx_b = 8;
            }

            $offsety = 0;
            $offsety_b = 0;
            if ($dimy > $height) { // we have to center!
                $offsety = floor(($dimy - $height) / 2);
            } else if ($dimx > 100) {
                $offsety = 4;
                $offsety_b = 8;
            }

            $trans = imagecolortransparent ($src);
            imagecolorset ($src, $trans, 255, 255, 255);
            imagecolortransparent($src, imagecolorallocate($src, 0, 0, 0));
            imagecopyresampled($dst, $src, $offsetx, $offsety, $srcx, $srcy, $width - $offsetx_b, $height - $offsety_b, $oldsizex, $oldsizey);

            if (function_exists("imagettftext") && $dimx > 100 && $info_text != '' ) {
               // some extra info at the bottom of the image. Available parameters: {date} {size} {dimension}
                  $text = str_replace('{dimension}', $oldsizex."x".$oldsizey, $info_text);
                  $text = str_replace('{size}', formatSize(filesize($image)), $text);
                  $text = str_replace('{date}', date("d.m.Y",filemtime($image)), $text);
			   $color = imagecolorclosest ($dst, $info_textcolor_R, $info_textcolor_G, $info_textcolor_B);
			   imagettftext($dst, $info_fontsize, 0, 8, $dimy-8, $color, $info_font, $text);
            }

            header('Content-type: image/jpg');
            if ($usethumbs) { // we save the thumb
                imagejpeg($dst, $cachename, $compression);
            }
            if (!$generateOnly) {
                ob_start();
                if (imagejpeg($dst, '', $compression)) {
                    $buffer = ob_get_contents();
                    header('Content-Length: ' . strlen($buffer));
                    ob_end_clean();
                    echo $buffer;
                    @imagedestroy($dst);
                    return true;
                } else {
                    ob_end_flush();
                    tfu_debug('cannot save: ' . $image);
                    @imagedestroy($src);
                }
            }
        }
    }
    return false;
}
// we check if we can get a memory problem!
function isMemoryOk($oldsize, $newsize, $image_name, $debug = true)
{
    $memory_read = (($oldsize[0] * $oldsize[1] * 6) + 2048576) * 1.1; // mem and we add 2 MB + 10% for safty
    $memory_orig = ($newsize * $newsize * 6) * 1.1; // 10% overhead.
    $memory = $memory_read + $memory_orig;

    // I try to increase the memory if more is needed and if it is possible.
    if (function_exists("memory_get_usage")) {
      $InUse=memory_get_usage();
      if ($memory > return_kbytes(ini_get('memory_limit')*1024) - $InUse)
      {
        @ini_set('memory_limit',$memory + $InUse + 5000000); // 5 MB for processing extra!
      }
    }
    $memory_limit = return_kbytes(ini_get('memory_limit')) * 1024;
    if ($memory > $memory_limit && $memory_limit > 0) { // we store the number of images that have a size problem in the session and output this in the readDir file
        $mem_errors = 0;
        if (isset($_SESSION['upload_memory_limit'])) {
            $mem_errors = $_SESSION['upload_memory_limit'];
        }
        $_SESSION['upload_memory_limit'] = ($mem_errors + 1);
        if ($debug) {
            tfu_debug('File ' . $image_name . ' cannot be processed because not enough memory is available! Needed: ~' . $memory . '. Available: ' . $memory_limit);
        }
        return false;
    } else {
        return true;
    }
}
$sn = get_server_name();

function get_image_src($image, $type)
{
    set_error_handler('on_error_no_output'); // No error shown because we handle this error!
    if ($type == 1) { // gif
        $src = imagecreatefromgif($image);
    } else if ($type == 2) { // jpg
        $src = imagecreatefromjpeg($image);
    } else if ($type == 3) { // png
        $src = @imagecreatefrompng($image);
    } else {
        $src = imagecreatefromjpeg($image); // if error we try read an jpg!
    }
    set_error_handler('on_error');
    return $src;
}
/**
 * A small helper function !
 */
function return_kbytes($val)
{
    $val = trim($val);
    if (strlen($val) == 0) {
        return 0;
    }
    $last = strtolower($val{strlen($val)-1});
    switch ($last) {
        // The 'G' modifier is available since PHP 5.1.0
        case 'g':
            $val *= 1024;
        case 'm':
            $val *= 1024;
        case 'k':
            $val *= 1;
    }
    return $val;
}
$m = is_renameable();

/**
 * get maximum upload size
 */
function getMaximumUploadSize()
{
    $upload_max = return_kbytes(ini_get('upload_max_filesize'));
    $post_max = return_kbytes(ini_get('post_max_size'));
    return $upload_max < $post_max ? $upload_max : $post_max;
}

/**
 * compares caseinsensitive - normally this could be done with natcasesort -
 * but this seems to be buggy on my test system!
 */
function mycmp ($a, $b)
{
    return strnatcasecmp ($a, $b);
}

/**
 * compares caseinsensitive - ascending for date
 */
function mycmp_date ($a, $b)
{
    return strnatcasecmp ($b, $a);
}

function cmp_dec ($a, $b)
{
    return mycmp(urldecode($a), urldecode($b));
}

function cmp_dir_dec ($a, $b)
{
    $a = substr($a, 0);
    $b = substr($b, 0);
    return mycmp(urldecode($a), urldecode($b));
}

function cmp_date_dec ($a, $b)
{
    return mycmp_date(urldecode($a), urldecode($b));
}

/**
 * deletes everything from the starting dir on! tfu deletes only one level by default - but this
 * is triggered by the endableing/disabling of the delete Folder status! not by this function!
 */
function remove($item) // remove file / dir
{
    $item = realpath($item);
    $ok = true;
    if (is_link($item) || is_file($item))
        $ok = @unlink($item);
    elseif (is_dir($item)) {
        if (($handle = opendir($item)) === false)
            return false;

        while (($file = readdir($handle)) !== false) {
            if (($file == '..' || $file == '.')) continue;

            $new_item = $item . '/' . $file;
            if (!file_exists($new_item))
                return false;
            if (is_dir($new_item)) {
                $ok = remove($new_item);
            } else {
                $ok = @unlink($new_item);
            }
        }
        closedir($handle);
        $ok = @rmdir($item);
    }
    return $ok;
}

function is_tfu_deletable($file)
{
    $isWindows = substr(PHP_OS, 0, 3) == 'WIN';

    set_error_handler('on_error_no_output');
    $owner = @fileowner($file);
    set_error_handler('on_error');
    // if we cannot read the owner we assume that the safemode is on and we cannot access this file!
    if ($owner === false) {
        return false;
    }
    // if dir owner not same as effective uid of this process, then perms must be full 777.
    // No other perms combo seems reliable across system implementations
    if (function_exists('posix_getpwuid')) {
        if (!$isWindows && posix_geteuid() !== $owner) {
            return (substr(decoct(@fileperms($file)), -3) == '777' || @is_writable(dirname($file)));
        }
    }
    if ($isWindows && getmyuid() != $owner) {
        return (substr(decoct(fileperms($file)), -3) == '777');
    }
    // otherwise if this process owns the directory, we can chmod it ourselves to delete it
    return is_writable(dirname($file));
}

function replaceInput($input)
{
    global $input_invalid;

    $output = str_replace('<', '_', $input);
    $output = str_replace('>', '_', $output);
    $output = str_replace('?', '_Q_', $input);
    // we check some other settings too :)
    if (strpos($output, 'cookie(') || strpos($output, 'popup(') || strpos($output, 'open(') || strpos($output, 'alert(') || strpos($output, 'reload(') || strpos($output, 'refresh(')) {
        $output = 'XSS';
    }
    // we check for security if a .. is in the path we remove this!	and .// like in http:// is invalid too!
    $output = str_replace("..", "__", $output);
    $output = str_replace("//", "__", $output);
    return $output;
}

function getCurrentDir()
{
    // we read the dir - first session, then parameter, then default!
    if (isset($_SESSION['TFU_DIR'])) {
        $dir = $_SESSION['TFU_DIR'];
    } else {
        $dir = 'upload';
    }
    return $dir;
}

function getFileName($dir, $find_index_file = false) {
    global $fix_utf8, $exclude_directories, $sort_files_by_date, $hide_hidden_files, $enable_enhanced_debug, $use_index_for_files;

    if (!$use_index_for_files && !$find_index_file) {
      // used for position critical stuff like delete, rename... 
      if (isset($_POST['tfu_file_name'])) {     
        $filename_post = $_POST['tfu_file_name'];
        $filename = $dir . '/' . fix_decoding($filename_post, $fix_utf8);
          if (file_exists($filename)) {
            return $filename;
          } else {
            tfu_debug("Check the encoding settings. Files canot be found when sent from the flash. If you get this error and you cannot fix this with the encoding please use the old index way and set \$use_index_for_files=false. See TFU FAQ 21.");
            return "_FILE_NOT_FOUND"; 
          } 
      }      
      // xdelete, copymove !!!  Because there can be many files! 
      if (isset($_POST['tfu_file_names'])) { 
        $filenames = array();
        $filenames_post_array = explode('||', $_POST['tfu_file_names']);
        foreach ($filenames_post_array as $filename_post) {
            $filename = $dir . '/' . fix_decoding($filename_post, $fix_utf8);
            if (file_exists($filename)) {
              $filenames[] = $filename;
            } else {
              tfu_debug("Check the encoding settings. Files canot be found when sent from the flash. If you get this error and you cannot fix this with the encoding please use the old index way and set \$use_index_for_files=false. See TFU FAQ 21.");
              return "_FILE_NOT_FOUND"; 
            } 
        }
        return $filenames;
      }
    }
    
    if (!$find_index_file) {  
      if (!isset($_GET['index']) || $_GET['index'] == '') {
          return '';
      }
      $index = parseInputParameter($_GET['index']);
      }
      // All files are sorted in the array myFiles
      $dirhandle = opendir($dir);
      $myFiles = array();
      while (($file = readdir($dirhandle)) !== false) {
          if ($file != '.' && $file != '..' && !in_array($file, $exclude_directories)&& (!($hide_hidden_files && (strpos($file, '.') === 0)))) {
              if (!is_dir($dir . '/' . $file) && check_view_extension($file)) {
                  if ($sort_files_by_date) {
                      $file = filemtime(($dir . '/' . $file)) . $file;
                  }
                  array_push($myFiles, fix_encoding($file, $fix_utf8));
              }
          }
      }
      closedir ($dirhandle);
      if ($sort_files_by_date) {
          usort ($myFiles, 'mycmp_date');
      } else {
          usort ($myFiles, 'mycmp');
      }
      // now we have the same order as in the listing and check if we have one or multiple indexes !
      if (!$find_index_file) {
      if (strpos($index, ',') === false) { // only 1 selection
          if (isset($myFiles[$index])) {
            return get_decoded_string($dir, $myFiles[$index]);
          } else {
            if ($enable_enhanced_debug) {
              tfu_debug("File index not found.");
            }
            return "_FILE_NOT_FOUND";
          }
      } else { // we return an array !
          // we need the offset
          $offset = parseInputParameter($_GET['offset']);
          $filenames = array();
          $index = trim($index, ',');
          $indices = explode(',', $index);
          foreach ($indices as $ind) {
              $filenames[] = get_decoded_string($dir, $myFiles[$ind - $offset]);
          }
          return $filenames;
      }
    } else {
        // reverse search
        $transMyFiles = array_flip($myFiles);
        return $transMyFiles[$find_index_file]; 
    }
}

function get_decoded_string($dir, $string)
{
    global $fix_utf8;
    if ($fix_utf8 == 'none') {
        return $dir . '/' . $string;
    } else if ($fix_utf8 == '') {
        return $dir . '/' . utf8_decode(remove_sort_prefix($string));
    } else {
        return $dir . '/' . iconv('UTF-8', $fix_utf8, remove_sort_prefix($string));
    }
}

function remove_sort_prefix($string) {
    global $sort_files_by_date;
    if ($sort_files_by_date) {
        return substr($string, 10);
    } else {
        return $string;
    }
}

function getRootUrl() {
    if (isset($_SERVER)) {
        $GLOBALS['__SERVER'] = &$_SERVER;
    } elseif (isset($HTTP_SERVER_VARS)) {
        $GLOBALS['__SERVER'] = &$HTTP_SERVER_VARS;
    }
    $dirn = dirname ($GLOBALS['__SERVER']['PHP_SELF']);
    if ($dirn == '\\' || $dirn == '/') $dirn = '';
    // fix for IIS7 - check has to be for on or 1, not for existence only!
    $isHttps = isset($GLOBALS['__SERVER']['HTTPS']) 
               && ( (strtolower($GLOBALS['__SERVER']['HTTPS']) == 'on') 
               || ($GLOBALS['__SERVER']['HTTPS'] == '1'));
    return 'http' . (($isHttps) ?  's' : '') . '://' . $GLOBALS['__SERVER']['HTTP_HOST'] . $dirn . '/';
}

function tfu_checkSession()
{
}
if (isset($_SESSION['TF' . 'U_RN'])) {
    $s = $_SESSION['TF' . 'U_RN'];
    $t = substr($s, 0, 3) . substr($s, 21, 3) . substr($s, 10, 4);
    if (time() > ($t + (6 * 12 * 2 * 1000))) $_SESSION['TF' . 'U_RN'] = '0';
}

/**
 * * removes ../ in a pathname!
 */
function fixUrl($url)
{
    $pos = strpos ($url, '../');
   	while ($pos !== false && $pos != 0) {
        $before = substr($url, 0, $pos-1);
        $after = substr($url, $pos + 3);
        $before = substr($before, 0, strrpos($before, '/') + 1);
        $url = $before . $after;
        $pos = strpos ($url, '../');
    }
    return $url;
}

function runsNotAsCgi()
{
    $no_cgi = true;
    if (isset($_SERVER['SERVER_SOFTWARE'])) {
        $mystring = $_SERVER['SERVER_SOFTWARE'];
        $pos = strpos ($mystring, 'CGI');
        if ($pos === false) {
            // nicht gefunden...
        } else {
            $no_cgi = false;
        }
        $mystring = $_SERVER['SERVER_SOFTWARE'];
        $pos = strpos ($mystring, 'cgi');
        if ($pos === false) {
            // nicht gefunden...
        } else {
            $no_cgi = false;
        }
    }
    return $no_cgi;
}

function has_safemode_problem_global()
{
    $isWindows = substr(PHP_OS, 0, 3) == 'WIN';
    $no_cgi = runsNotAsCgi();

    if (function_exists('posix_getpwuid') && function_exists('posix_getpwuid')) {
        if (!isset($_SESSION['tfu_posix_geteuid_works'])) {
          $_SESSION['tfu_posix_geteuid_works'] = 'check';
          $userid = @posix_geteuid();
          $userinfo = @posix_getpwuid($userid);
          $def_user = array ('apache', 'nobody', 'www');
          if (in_array ($userinfo['name'], $def_user)) {
            $no_cgi = true;
          }
          unset($_SESSION['tfu_posix_geteuid_works']);
        }
    }
    if (ini_get('safe_mode') == 1 && $no_cgi && !$isWindows) {
        return true;
    }
    return false;
}
// set a umask that makes the files deletable again!
if ($check_safemode && (has_safemode_problem_global() || runsNotAsCgi())) {
    umask(0000); // otherwise you cannot delete files anymore with ftp if you are no the owner!
} else {
    umask(0022); // Added to make created files/dirs group writable
}

function gd_version()
{
    static $gd_version_number = null;
    if ($gd_version_number === null) {
        if (function_exists('gd_info')) {
            $info = gd_info();
            $module_info = $info['GD Version'];
            if (preg_match("/[^\d\n\r]*?([\d\.]+)/i",
                    $module_info, $matches)) {
                $gd_version_number = $matches[1];
            } else {
                $gd_version_number = 0;
            }
        } else { // needed before 4.3 !
            ob_start();
            phpinfo(8);
            $module_info = ob_get_contents();
            @ob_end_clean();
            if (preg_match("/\bgd\s+version\b[^\d\n\r]+?([\d\.]+)/i",
                    $module_info, $matches)) {
                $gd_version_number = $matches[1];
            } else {
                $gd_version_number = 0;
            }
        }
    }
    return $gd_version_number;
}

function is_gd_version_min_20()
{
    if (gd_version() >= 2) {
        return 'true';
    } else {
        return 'false';
    }
}

function restore_split_files($items)
{
    $split_array = array();
    // first we check if files are split and group he splited files
    foreach ($items as $filename) {
        if (is_part($filename)) {
            $split_array[removeExtension($filename)][] = $filename;
        }
    }

    foreach ($split_array as $restore => $parts) {
        $totsize = 0;
        // sorting of parts is important!
        usort($parts, 'mycmp');
        // we open the destination
        $dest_file = fopen($restore, 'wb');
        foreach ($parts as $parts_name) {
            $totsize += filesize($parts_name);
            $fp = fopen($parts_name, 'rb');
            while ($content = fread($fp, 8192)) {
                fputs($dest_file, $content);
                flush();
                ob_flush();
            }
            fclose($fp);
        }
        fclose($dest_file);
        // if o.k. we delete the .part files! - check the filesize maybe?
        if (filesize($restore) == $totsize) {
            array_map('unlink', $parts);
        }
    }
}

function resize_merged_files($items, $size)
{
    global $compression;
    $split_array = array();
    // first we check if files are split and group the splited files
    foreach ($items as $filename) {
        if (is_part($filename)) {
            $split_array[removeExtension($filename)][] = $filename;
        }
    }
    foreach ($split_array as $restore => $parts) {
        if (is_supported_tfu_image(my_basename($restore), $restore) && $size < 100000) {
          resize_file($restore, $size, $compression, my_basename($restore));
        }
    }
}

function is_part($str)
{
    global $split_extension;
    $ex = substr (strrchr ($str, '.'), 1);
    $pos = strpos ($ex, $split_extension);
    if ($pos === false) {
        return false;
    } else if ($pos == 0) {
        return true;
    } else {
        return false;
    }
}

function is_supported_tfu_image($image,$current)
{
    global $scan_images;
    $image = strtolower ($image);
    $isimage = preg_match('/.*\.(jp)(e){0,1}(g)$/', $image) ||
    preg_match('/.*\.(gif)$/', $image) ||
    preg_match('/.*\.(png)$/', $image) ;
    if ($isimage) {
      // we check if this is really an image - if we cannot read the size we assume it could be a php fake
      set_error_handler('on_error_no_output');
      if (file_exists($current)) {
        $size = @getimagesize ($current);
        if ($size === false || $scan_images) {
          // seems not to be an image - now we we replace the <?php with <_php
          $data = file_get_contents($current);
          $data2 = str_replace("<?php","<_php",$data);
          if ($data2 != $data) {
            file_put_contents($current, $data2);
            tfu_debug("SECURITY WARNING: Please check the file ".$image.". It was uploaded with an image extensions but included php code. The php start of this file was changed because of security issues!" );
          }
        }
      }
      set_error_handler('on_error');
    }
    return $isimage;
}

if (!isset($skip_error_handling)) {
  set_error_handler("on_error_no_output"); // 4.x gives depreciated errors here but if I change it it does only work with 5.x - therefore I don't show any errors here !
}
if (file_exists('tfu_exifReader.php')) {
  include 'tfu_exifReader.php';
}
if (!isset($skip_error_handling)) {
  set_error_handler("on_error");
}



function get_exif_size($filename, $image_name)
{
    set_error_handler('on_error_no_output'); // is needed because error are most likly but we don't care about fields we don't even know
    $er = new phpExifReader($filename);
    $er->processFile();
    $exif_info = $er->getImageInfo();
    set_error_handler('on_error');

    $size_array = array();
    $size_array[2] = 2;
    if (isset($er->ImageInfo[TAG_EXIF_IMAGEWIDTH])) {
        $size_array[0] = $er->ImageInfo[TAG_EXIF_IMAGEWIDTH];
    } else {
        $size_array[0] = 1024;
        tfu_debug('Size of image ' . $image_name . ' cannot be detected using 1024x768.');
    }

    if (isset($er->ImageInfo[TAG_EXIF_IMAGELENGTH])) {
        $size_array[1] = $er->ImageInfo[TAG_EXIF_IMAGELENGTH];
    } else {
        $size_array[1] = 768;
    }
    return $size_array;
}

function removeCacheThumb($filename)
{
    $thumbsdir = dirname(__FILE__) . '/thumbs';
    if (file_exists($thumbsdir) && is_writable($thumbsdir)) { // is a caching dir available and writeable?
        $cachename = $thumbsdir . '/' . sha1($filename . '160') . '.jpg'; // small
        if (file_exists($cachename)) {
            @unlink($cachename);
        }
        $cachename = $thumbsdir . '/' . sha1($filename . '400') . '.jpg'; // big
        if (file_exists($cachename)) {
            @unlink($cachename);
        }
    }
    cleanup_thumbs_cache();
}

function cleanup_thumbs_cache()
{
    if (isset($_SESSION['checkcache'])) { // we only check once per session!
        return;
    }
    $_SESSION['checkcache'] = 'TRUE';

    $cache_time = 10; // in days !!
    $thumbsdir = dirname(__FILE__) . '/thumbs';

    $cache_time = $cache_time * 86400;
    $del_time = time() - $cache_time;
    if (file_exists($thumbsdir) && is_writable($thumbsdir)) {
        $d = opendir($thumbsdir);
        $i = 0;
        while (false !== ($entry = readdir($d))) {
            if ($entry != '.' && $entry != '..') {
                $atime = fileatime($thumbsdir . '/' . $entry);
                if ($atime < $del_time) {
                    @unlink($thumbsdir . '/' . $entry);
                }
            }
        }
        closedir($d);
    }
}

function removeExtension($name)
{
    return substr($name, 0, strrpos ($name, '.'));
}

/**
 * * create a unique directory - 1 st is year, 2 and 3 rd is month - rest is unique up to length
 */
function createUniqueDir($basedir, $length = 10)
{
    $dir = '';
    $prefix = substr(date('Ym'), 3);
    while ($dir == '') {
        $start = pow (10, $length-3);
        $stop = pow (10, $length-2)-1;
        $value = rand($start, $stop);

        $tempdir = $basedir . $prefix . $value;
        if (!file_exists($tempdir)) {
            mkdir($tempdir);
            $dir = $tempdir;
            break;
        }
    }
    return $dir;
}

/**
 * Finds the destination folder depending on the id - the id has the format 1,2,0
 * means folder 2 in level 1, 3 rd folder in level 2, 1st folder in level 3.....
 * empty means root!
 */
function getDestinationFolder($id_list)
{
    global $exclude_directories, $hide_hidden_files;
    $base_dir = $_SESSION['TFU_ROOT_DIR'];
    if ($id_list == '') return $base_dir;
    $ids = explode(',', $id_list);
    $dir = $base_dir;
    foreach ($ids as $id) {
        // read the dir - get the directory and set the base to the new level.
        $dirhandle = opendir($dir);
        $myDirs = array();
        while (false !== ($filed = readdir($dirhandle))) {
            if ($filed != '.' && $filed != '..' && !in_array($filed, $exclude_directories) && (!($hide_hidden_files && (strpos($filed, '.') === 0)))) {
                if (is_dir($dir . '/' . $filed)) {
                    array_push($myDirs, $filed);
                }
            }
        }
        usort ($myDirs, 'mycmp');
        $dir = $dir . '/' . $myDirs[$id];
    }
    return $dir;
}

function get_tree_xml()
{
    if (isset($_SESSION["TREE_" . $_SESSION['TFU_ROOT_DIR']])) {
      return $_SESSION["TREE_" . $_SESSION['TFU_ROOT_DIR']];
    } else {
      $tree = '<node><node label="/"  id="">' . show_dir_xml($_SESSION['TFU_ROOT_DIR']) . '</node></node>';
      $_SESSION["TREE_" . $_SESSION['TFU_ROOT_DIR']] = $tree;
      return $tree;
    }
}

function show_dir_xml($myDir = '.', $indent = 0, $levelStr = '')
{
    global $exclude_directories, $hide_hidden_files;
    $dir = opendir($myDir);
    $einrueckung = str_repeat(' ', $indent * 4);
    if ($levelStr != '') {
        $levelStr .= ',';
    }
    $foo = '';
    $counter = 0;
    $dirlist = array();

    while ($file = readdir($dir)) {
        $dirlist[] = $file;
    }
    usort ($dirlist, 'mycmp');
    foreach ($dirlist as $file) {
        $newDir = $myDir . '/' . $file;

        if ($file == '.' || $file == '..' || in_array($file, $exclude_directories)&& (!($hide_hidden_files && (strpos($file, '.') === 0))))
            continue;

        if (is_dir($newDir)) {
            $curLevelStr = $levelStr . '' . $counter++;
            $foo .= '<node id="' . $curLevelStr . '" label="' . $file . '">' . "\n" . show_dir_xml($newDir . '/', 1, $curLevelStr) . "</node>\n";
        }
    }
    return $foo;
}

function get_unique_filename($dir, $image)
{
    $i = 1;
    $probeer = $image;
    while (file_exists($dir . $probeer)) {
        $punt = strrpos($image, '.');
        if (substr($image, ($punt-3), 1) !== ('(') && substr($image, ($punt-1), 1) !== (')')) {
            $probeer = substr($image, 0, $punt) . '(' . $i . ')' .
            substr($image, ($punt), strlen($image) - $punt);
        } else {
            $probeer = substr($image, 0, ($punt-3)) . '(' . $i . ')' .
            substr($image, ($punt), strlen($image) - $punt);
        }
        $i++;
    }
    return $probeer;
}

/**
 * Needed for loading saving text files
 */
$cp1252_map = array(
		      '\xc2\x80' => '\xe2\x82\xac', /* EURO SIGN */
		      '\xc2\x82' => '\xe2\x80\x9a', /* SINGLE LOW-9 QUOTATION MARK */
		      '\xc2\x83' => '\xc6\x92',     /* LATIN SMALL LETTER F WITH HOOK */
		      '\xc2\x84' => '\xe2\x80\x9e', /* DOUBLE LOW-9 QUOTATION MARK */
		      '\xc2\x85' => '\xe2\x80\xa6', /* HORIZONTAL ELLIPSIS */
		      '\xc2\x86' => '\xe2\x80\xa0', /* DAGGER */
		      '\xc2\x87' => '\xe2\x80\xa1', /* DOUBLE DAGGER */
		      '\xc2\x88' => '\xcb\x86',     /* MODIFIER LETTER CIRCUMFLEX ACCENT */
		      '\xc2\x89' => '\xe2\x80\xb0', /* PER MILLE SIGN */
		      '\xc2\x8a' => '\xc5\xa0',     /* LATIN CAPITAL LETTER S WITH CARON */
		      '\xc2\x8b' => '\xe2\x80\xb9', /* SINGLE LEFT-POINTING ANGLE QUOTATION */
		      '\xc2\x8c' => '\xc5\x92',     /* LATIN CAPITAL LIGATURE OE */
		      '\xc2\x8e' => '\xc5\xbd',     /* LATIN CAPITAL LETTER Z WITH CARON */
		      '\xc2\x91' => '\xe2\x80\x98', /* LEFT SINGLE QUOTATION MARK */
		      '\xc2\x92' => '\xe2\x80\x99', /* RIGHT SINGLE QUOTATION MARK */
		      '\xc2\x93' => '\xe2\x80\x9c', /* LEFT DOUBLE QUOTATION MARK */
		      '\xc2\x94' => '\xe2\x80\x9d', /* RIGHT DOUBLE QUOTATION MARK */
		      '\xc2\x95' => '\xe2\x80\xa2', /* BULLET */
		      '\xc2\x96' => '\xe2\x80\x93', /* EN DASH */
		      '\xc2\x97' => '\xe2\x80\x94', /* EM DASH */

		      '\xc2\x98' => '\xcb\x9c',     /* SMALL TILDE */
		      '\xc2\x99' => '\xe2\x84\xa2', /* TRADE MARK SIGN */
		      '\xc2\x9a' => '\xc5\xa1',     /* LATIN SMALL LETTER S WITH CARON */
		      '\xc2\x9b' => '\xe2\x80\xba', /* SINGLE RIGHT-POINTING ANGLE QUOTATION*/
		      '\xc2\x9c' => '\xc5\x93',     /* LATIN SMALL LIGATURE OE */
		      '\xc2\x9e' => '\xc5\xbe',     /* LATIN SMALL LETTER Z WITH CARON */
		      '\xc2\x9f' => '\xc5\xb8'      /* LATIN CAPITAL LETTER Y WITH DIAERESIS*/
    );

function tfu_seems_utf8($Str)
{
    for ($i = 0; $i < strlen($Str); $i++) {
        if (ord($Str[$i]) < 0x80) $n = 0; # 0bbbbbbb
        elseif ((ord($Str[$i]) &0xE0) == 0xC0) $n = 1; # 110bbbbb
        elseif ((ord($Str[$i]) &0xF0) == 0xE0) $n = 2; # 1110bbbb
        elseif ((ord($Str[$i]) &0xF0) == 0xF0) $n = 3; # 1111bbbb
        else return false; # Does not match any model
        for ($j = 0; $j < $n; $j++) { // n octets that match 10bbbbbb follow ?
            if ((++$i == strlen($Str)) || ((ord($Str[$i]) &0xC0) != 0x80)) return false;
        }
    }
    return true;
}

function cp1252_to_utf8($str)
{
    global $cp1252_map;
    return strtr(utf8_encode($str), $cp1252_map);
}

function utf8_to_cp1252($str)
{
    global $cp1252_map;
    return utf8_decode(strtr($str, array_flip($cp1252_map)));
}

function getExtension($name)
{
    $name = rtrim($name, ".,; \t\n\r\0\x0B");
    return substr (strrchr ($name, '.'), 1);
}


function space_enc($string) {
  global $description_mode;

  if ($description_mode == 'true') { // - description is added at the end - we don't encode ' : ' and the rest.
    $teile = explode(" : ", $string,2);
    $teile[0] =  str_replace(" ", "%20",  $teile[0]);
    $string = implode(" : ", $teile);
  } else {
    $string =  str_replace(" ", "%20", $string);
  }
  return $string;

}
/**
 * This does a nice character exchange with a random crypt key!
 * If you need a 100% secure connection please use https!
 */
function tfu_enc($str, $id, $length = false)
{
    if ($length) {
        $str = substr($str, 0, $length);
    }
    for ($i = 0; $i < strlen($id); $i++) {
        if (ord($id{$i}) > 127) {
            tfu_debug('The crypt key at position ' . $i . ' is not valid - please change the implementation.');
            return $str;
        }
    }
    $code = '';
    $keylen = strlen($id);
    for ($i = 0; $i < strlen($str); $i++) {
        $code .= chr(ord($str{$i}) + ord($id{$i%$keylen}));
    }
    return utf8_encode($code);
}

function setSessionVariables()
{
    global $folder, $user, $login;
    // this settings are needed in the other php files too!
    if ($login == 'true') {
        $_SESSION['TFU_LOGIN'] = 'true';
        if (!isset($_SESSION['TFU_USER'])) { // can be set by the Joomla wrapper and we don't overwrite it with a dummy value!
          $_SESSION['TFU_USER'] = ($user != '' && $user != '__empty__') ? $user : $_SERVER['REMOTE_ADDR'];
        }
    } else {
       unset($_SESSION['TFU_USER']);
    }
    $_SESSION['TFU_RN'] = parseInputParameter($_POST['twg_rn']);
    $_SESSION['TFU_ROOT_DIR'] = $_SESSION['TFU_DIR'] = $folder;
    store_temp_session();
}

/**
 * All parameters a sent to the flash
 * First I wanted to introduce a Config class but this is the only place where they
 * have to be passed globaly - therefore no class is used now.
 */
function sendConfigData()
{
    global $login, $rn, $maxfilesize, $resize_show, $resize_data, $resize_label, $resize_default, $allowed_file_extensions;
    global $forbidden_file_extensions, $show_delete, $enable_folder_browsing, $enable_folder_creation;
    global $enable_folder_deletion, $enable_file_download, $keep_file_extension, $show_preview, $show_big_preview;
    global $enable_file_rename, $enable_folder_rename, $enable_folder_move, $enable_file_copymove, $language_dropdown;
    global $preview_textfile_extensions, $edit_textfile_extensions;
    //, $maxfilesize_split;
    // optional settings
    global $reg_infos, $login_text, $relogin_text, $upload_file, $base_dir, $titel;
    global $warning_setting, $hide_remote_view, $directory_file_limit, $remote_label;
    global $preview_label, $show_full_url_for_selected_file, $upload_finished_js_url;
    global $preview_select_js_url, $delete_js_url, $js_change_folder, $js_create_folder;
    global $js_rename_folder, $js_delete_folder, $js_copymove, $queue_file_limit, $show_size;
    global $queue_file_limit_size, $split_extension, $hide_help_button, $direct_download;
    global $description_mode_show_default, $description_mode, $download_multiple_files_as_zip;
    global $overwrite_files, $description_mode_mandatory, $post_upload_panel, $form_fields;
    global $big_progressbar,$img_progressbar,$img_progressbar_back,$img_progressbar_anim, $big_server_view;
    global $zip_file_pattern, $is_jfu_plugin, $has_post_processing, $directory_file_limit_size;
    global $show_server_date_instead_size, $enable_file_creation, $enable_file_creation_extensions;
    global $switch_sides, $date_format;

    // the sessionid is mandatory because upload in flash and Firefox would create a new session otherwise - sessionhandled login would fail then!
    $output = '&login=' . $login .  '&maxfilesize=' . '' . $maxfilesize;
    // $output .= '&maxfilesize_split=' . tfu_enc('' . $maxfilesize_split, $rn);
    // $output .= '&maxfilesize_php=' . getMaximumUploadSize();
    $output .= '&resize_show=' . $resize_show . '&resize_data=' . $resize_data;
    $output .= '&resize_label=' . urlencode($resize_label) . '&resize_default=' . $resize_default;
    $output .= '&allowed_file_extensions=' . $allowed_file_extensions . '&forbidden_file_extensions=' . $forbidden_file_extensions;
    $output .= '&show_delete=' . $show_delete . '&enable_folder_browsing=' . $enable_folder_browsing;
    $output .= '&enable_folder_creation=' . $enable_folder_creation . '&enable_folder_deletion=' . $enable_folder_deletion ;
    $output .= '&enable_file_download=' . $enable_file_download . '&keep_file_extension=' . $keep_file_extension;
    $output .= '&show_preview=' . $show_preview . '&show_big_preview=' . $show_big_preview ;
    $output .= '&enable_file_rename=' . $enable_file_rename . '&enable_folder_rename=' . $enable_folder_rename;
    $output .= '&enable_folder_copy=' . $enable_folder_move . '&enable_file_copy=' . $enable_file_copymove;
    $output .= '&language_dropdown=' . $language_dropdown;
    $output .= '&preview_textfile_extensions=' . $preview_textfile_extensions . '&edit_textfile_extensions=' . $edit_textfile_extensions;
    // optional settings
    $output .= $reg_infos . '&login_text=' . $login_text;
    $output .= '&relogin_text=' . $relogin_text . '&upload_file=' . $upload_file;
    $output .= '&base_dir=' . $base_dir . '&titel=' . urlencode($titel);
    $output .= '&warning_setting=' . $warning_setting . '&hide_remote_view=' . $hide_remote_view;
    $output .= '&directory_file_limit=' . $directory_file_limit;
    $output .= '&remote_label=' . urlencode($remote_label) . '&preview_label=' . $preview_label;
    $output .= '&show_full_url_for_selected_file=' . $show_full_url_for_selected_file;
    $output .= '&upload_finished_js_url=' . urlencode($upload_finished_js_url) . '&preview_select_js_url=' . urlencode($preview_select_js_url);
    $output .= '&delete_js_url=' . urlencode($delete_js_url) . '&js_change_folder=' . urlencode($js_change_folder);
    $output .= '&js_create_folder=' . urlencode($js_create_folder) . '&js_rename_folder=' . urlencode($js_rename_folder);
    $output .= '&js_delete_folder=' . urlencode($js_delete_folder) . '&js_copymove=' . urlencode($js_copymove);
    $output .= '&queue_file_limit=' . $queue_file_limit . '&queue_file_limit_size=' . $queue_file_limit_size;
    $output .= '&split_extension=' . $split_extension . '&hide_help_button=' . $hide_help_button;
    $output .= '&direct_download=' . $direct_download . '&show_size=' . $show_size;
    $output .= '&description_mode=' . $description_mode . '&description_mode_show_default=' . $description_mode_show_default;
    $output .= '&multiple_zip_download=' . $download_multiple_files_as_zip;
    $output .= '&overwrite_files=' . $overwrite_files . '&description_mode_mandatory=' . $description_mode_mandatory;
    $output .= '&post_upload_panel=' . $post_upload_panel . '&form_fields=' .$form_fields;
    $output .= '&big_progressbar=' . $big_progressbar . '&img_progressbar=' .$img_progressbar;
    $output .= '&img_progressbar_back=' . $img_progressbar_back . '&img_progressbar_anim=' .$img_progressbar_anim;
    $output .= '&big_server_view=' . $big_server_view . '&zip_file_pattern=' . $zip_file_pattern;
    $output .= '&is_jfu_plugin=' . $is_jfu_plugin . '&has_post_processing=' . $has_post_processing;
    $output .= '&directory_file_limit_size=' . $directory_file_limit_size . '&show_server_date_instead_size=' . $show_server_date_instead_size;
    $output .= '&enable_file_creation=' . $enable_file_creation . '&enable_file_creation_extensions=' . $enable_file_creation_extensions;
    $output .= '&switch_sides=' . $switch_sides . '&date_format=' . $date_format;
     

    // all parameters are sent encrypted to the client.
    $parameters = "&parameters=" . urlencode(tfu_enc($output, $rn));

    // we generate a nonce for this request
    // last=true is added for such websites who add their own code to each page!
    
    echo '&tfu_nonce=' . create_tfu_nonce() . $parameters . "&last=true";
}

/**
 * This stores all data in a session in a temporary folder as well if it does exist.
 * This is a workaround if a session is lost and empty in the tfu_upload.php and restored there!
 */
function store_temp_session()
{
    global $session_double_fix;
    clearstatcache();
    if (file_exists(dirname(__FILE__) . '/session_cache') && session_id() != "") { // we do your own small session handling
        $cachename = dirname(__FILE__) . '/session_cache/' . session_id();
        $ser_file = fopen($cachename, 'w');
        fwrite($ser_file, serialize($_SESSION));
        fclose($ser_file);
        if ($session_double_fix) {
            $ser_file = fopen($cachename . '2', 'w');
            fwrite($ser_file, serialize($_SESSION));
            fclose($ser_file);
        }
    }
}

function checkSessionTempDir($type = 0)
{
    global $keep_internal_session_handling;
    if (isset($_FILES) && (count ($_FILES) > 0)) {
        $filen = array_shift($_FILES);
        $filename = $filen['name'];
        tfu_debug('It can be possible that someone tried to upload something without permissions! If you think this is the case the IP of this user is logged: ' . $_SERVER['REMOTE_ADDR'] . '. He tried to upload the following file: ' . $filename);
    }
    if (!file_exists(dirname(__FILE__) . '/session_cache')) {
        tfu_debug('Or it is possible that the session handling of the server is not o.k. Therefore TFU simulates a basic session handling and uses the session_cache folder for that.');
        if (!mkdir(dirname(__FILE__) . '/session_cache')) {
            tfu_debug('Directory session_cache could not be created! Please create the sub directoy session_cache and set the permissions of it to 777.');
        } else {
            tfu_debug('Directory session_cache could be created! TFU does now an internal session handling. Delete the directory session_cache to turn the internal handling off.');
            @chmod(dirname(__FILE__) . '/session_cache', 0777);
            // we create an index.htm to prevent listings!
            $datei = fopen(dirname(__FILE__) . '/session_cache/index.htm', 'w');
            fclose($datei);
        }

    } else if ($type == 5) { // the upload check has to fail because the whole session data is gone.
      echo 'int_session_handling=true';
    } else {
        tfu_debug('It seems that the session handling of the server is not o.k. TFU already tried a workaround that does not seem to work. TFU deleted the session_cache folder. Maybe it was created because of a wrong request! Please go to http://www.tinywebgallery.com/de/tfu/tfu_faq_12.php. If this does not help please report this in the forum to find a solution!');
        if (!$keep_internal_session_handling) {
            remove(dirname(__FILE__) . '/session_cache');
        }
      echo 'int_session_handling=false';
    }
}

$check_server_file_extensions = $m;

function restore_temp_session($checkrn = false)
{
    global $session_double_fix;
    clearstatcache();
    if (file_exists(dirname(__FILE__) . '/session_cache')) { // we do your own small session handling
        $cachename = dirname(__FILE__) . '/session_cache/' . session_id();
        if (file_exists($cachename)) {
            $data = file_get_contents($cachename);
            set_error_handler('on_error_no_output'); // is needed because error are most likly but we don't care about fields we don't even know
            $sdata = unserialize($data);
            set_error_handler('on_error');
            if (isset($sdata) && (isset($sdata['TFU_RN']) || $checkrn)) {
                $_SESSION = $sdata;
            } else { // we try again
                sleep(1);
                if ($session_double_fix) {
                    $cachename .= '2';
                }
                $data = file_get_contents ($cachename);
                set_error_handler('on_error_no_output'); // is needed because error are most likly but we don't care about fields we don't even know
                $sdata = unserialize($data);
                set_error_handler('on_error');
                if (isset($sdata) && (isset($sdata['TFU_RN'])|| $checkrn)) {
                    $_SESSION = $sdata;
                } else {
                    tfu_debug('Session data could no be restored :' . $data);
                }
            }
        }
        // check the protection of the folder
        $index_htm = dirname(__FILE__) . '/session_cache/index.htm';
        if (!file_exists($index_htm)) {
             $fh = fopen($index_htm, 'w');
             fclose($fh);
        }
        $htaccess = dirname(__FILE__) . '/session_cache/.htaccess';
        if (!file_exists($htaccess)) {
             $fh = fopen($htaccess, 'w');
             fwrite($fh, 'deny from all');
             fclose($fh);
        }
        // now we have to clean old temp sessions! - we do this once a day only!
        // first we check if we have done this already!
        $today = dirname(__FILE__) . '/session_cache/_cache_day_' . date('Y_m_d') . '.tmp';
        if (file_exists($today)) {
            return;
        }
        // not done - we delete all files on this folder older than 1 day + the _cache_day_*.tmp files
        $d = opendir(dirname(__FILE__) . '/session_cache');
        $i = 0;
        $del_time = time() - 86000; // we delete file older then 24 hours
        while (false !== ($entry = readdir($d))) {
            if ($entry != '.' && $entry != '..') {
                $atime = fileatime(dirname(__FILE__) . '/session_cache/' . $entry);
                if ($atime < $del_time) {
                    @unlink(dirname(__FILE__) . '/session_cache/' . $entry);
                }
            }
        }
        $tmp_files = glob(dirname(__FILE__) . '/session_cache/*.tmp');
        if ($tmp_files) {
            foreach($tmp_files as $fn) {
                if ($fn != '/session_cache/.' && $fn != '/session_cache/..') {
                  @unlink($fn);
                }
            }
        }
        // now we write the flag
        $fh = fopen($today, 'w');
        fclose($fh);
    }

    // Joomla session update!
    if (isset($_SESSION['__default']['session.counter'])) {
      $_SESSION['__default']['session.counter'] = $_SESSION['__default']['session.counter'] + 1;
      $_SESSION['__default']['session.timer.now'] = time();
      $_SESSION['__default']['session.timer.last'] = $_SESSION['__default']['session.timer.now'];
    }
}

// creates a nonce see http://en.wikipedia.org/wiki/Cryptographic_nonce
// just some random parts and then a hash of it.
function create_tfu_nonce() {
    return nhash(date("is") . session_id() . rand());
}

// checks if the extension is allowed to be viewed!
function check_view_extension($name)
{
    global $check_server_file_extensions, $allowed_view_file_extensions, $forbidden_view_file_extensions, $forbidden_view_file_filter;
    $allowed_view_file_extensions = str_replace(' ', '', strtolower($allowed_view_file_extensions));
    $forbidden_view_file_extensions = str_replace(' ', '', strtolower($forbidden_view_file_extensions));
    $forbidden_view_file_filter = str_replace(' ', '', strtolower($forbidden_view_file_filter));

    $isAllowed = true;

    if ($check_server_file_extensions == 'v') {
         if ($allowed_view_file_extensions != 'all' || $forbidden_view_file_extensions != '') {
             if ($allowed_view_file_extensions != 'all' && strpos($name, '.') === false) {
                 $isAllowed = false;
             } else {
                 $ext = strtolower(getExtension($name));
                 if ($allowed_view_file_extensions == 'all') { // we check the not allowed extensions
                     $isAllowed = !in_array($ext, explode(',', $forbidden_view_file_extensions));
                 } else { // we only allow the allowed extension
                     $isAllowed = in_array($ext, explode(',', $allowed_view_file_extensions));
                 }
             }
         } else {
             $isAllowed = true;
         }
         // now we check the filter on non windows systems!
         if (function_exists('fnmatch')) {
              if ($forbidden_view_file_filter != '') {
                   $filters =explode(',', $forbidden_view_file_filter);
                   foreach ($filters as $filter) {
                     if (fnmatch ($filter, $name)) {
                       $isAllowed = false;
                       break;
                     }
                   }
              }
         }
    }

    return $isAllowed;

}

function t($l, $s)
{
    $n = '';
    $m = explode(';', $l);
    foreach($m as $v) {
      $nrp = substr_count($v, '.');
      if ($nrp == 1 && (strpos($v,'*')=== false)) { $nrp++; $v.="*."; }
        $el = explode('.', $s);
        if ($el !== false) {
          $r = array_slice($el, 0, -$nrp);
          $n .= ';' . str_replace('*', 'ww'.'w'. ((count($r) >0) ? ('.'.array_pop($r)) : ''), $v);
        }
      }
    return $n;
}
// Checks if the uploaded extension is o.k. on the server side!
// Trailing . are removed because of security issues.
function check_valid_extension($name, $die_if_invalid = true)
{
    global $forbidden_file_extensions, $allowed_file_extensions;

    $afe = str_replace(' ', '', strtolower($allowed_file_extensions));
    $name = rtrim($name, ".,; \t\n\r\0\x0B");
    $path_info = pathinfo($name);
    if (isset($path_info['extension'])) {
      $file_extension = strtolower($path_info['extension']);
    } else {
      $file_extension = '';
    }
    
    if ($afe == '') { // this can be useful if you don't allow to upload - only to download!
        return false;
    }
    if ($afe != 'all') {
        $extension_whitelist = explode(',', $afe); // Allowed file extensions
        // $valid_chars_regex = '.A-Z0-9_ !@#$%^&()+={}\[\]\',~`-';				// Characters allowed in the file name (in a Regular Expression format)
        $is_valid_extension = false;
        foreach ($extension_whitelist as $extension) {
            if ($file_extension == $extension) {
                $is_valid_extension = true;
                break;
            }
        }
    } else { // not allowed extensions
        $nafe = str_replace(' ', '', strtolower($forbidden_file_extensions));
        $is_valid_extension = true;
        if ($nafe != '') {
            $extension_blacklist = explode(',', $nafe);
            foreach ($extension_blacklist as $extension) {
                if (strpos($file_extension, $extension) !== false) {
                    $is_valid_extension = false;
                    break;
                }
            }
        }
    }
    if (!$is_valid_extension && $die_if_invalid) {
        header('HTTP/1.1 500 Internal Server Error');
        echo 'No valid extension - upload not permitted.';
        exit(0);
    }
    return $is_valid_extension;
}
// Validate the file size (Warning the largest files supported by this code is 2GB)
function check_valid_filesize($name)
{
    global $maxfilesize;
    $file_size = sprintf("%u", @filesize($name));
    if (!$file_size || $file_size > ($maxfilesize * 1024)) {
        @unlink($name);
        header('HTTP/1.1 500 Internal Server Error');
        echo 'File size too big - upload not permitted.';
        exit(0);
    }
}

/**
 * Right now the parameters are only checked very basic because they are not
 * returned back to a frontend or passed somewhere else.
 * Therefore only a basic regular expression is used!
 * You can pass a different regex if you want to restrict something more (e.g a file name)
 * The are 2 functions
 * - parseInputParameter check for valid caracters because there I know the proper values
 * - parseInputParameterFile is an exclude - there I only check for chars that are not allowed in file names!
 */
function parseInputParameter($value, $def = '', $valid_chars_regex = '.\w_,-')
{
    return (isset($value)) ? preg_replace('/[^' . $valid_chars_regex . ']|\.+$/i', '_', $value) : $def;
}

function parseInputParameterFile($value, $def = '')
{
    if (isset($value)) {
        $reserved = preg_quote("\/:*?'<>", "/");
        return preg_replace("/([\\x00-\\x1f{$reserved}])/", '_', $value);
    } else {
        return $def;
    }
}

function printServerInfo()
{
   global $check_image_magic;                                                                                                                                                                                   global $m;
    echo '
  <style type="text/css">
  body { 	font-family : Arial, Helvetica, sans-serif; font-size: 12px; background-color:#ffffff; }
  td { vertical-align: top; font-size: 12px; }
  .install {  margin-left: auto;  margin-right: auto;  margin-top: 3em;  margin-bottom: 3em; padding: 10px; border: 1px solid #cccccc;  width: 650px; background: #F1F1F1; }
  </style>
';
    $limit = return_kbytes(ini_get('memory_limit'));

    echo '<br><p><center>Some info\'s about your server. This limits are not TFU limits. You have to change this in the php.ini.</center></p>';
    echo '<div class="install">';
    echo '<table><tr><td>';
    echo '<tr><td width="400">TFU version:</td><td width="250">2.15.1&nbsp;';
    // simply output the license type by checking the strings in the license. No real check like in the flash is done here.
    
    if ($m != "" && $m != "s" && $m !="w" ) {
        ob_start();
        $ff = dirname(__FILE__) . "/twg.lic.php";
        if (!file_exists($ff)) { // we are in TWG
          $ff = dirname(__FILE__) . "/../../twg.lic.php";
        }
        include  $ff;
        ob_end_clean();
        if ($l == $d) {
          echo " (Enterprise Edition License)";
        } else if (strpos($d, "TWG_PROFESSIONAL") !== false) {
          echo " (Professional Edition License)";
        } else if (strpos($d, "TWG_SOURCE") !== false) {
          echo " (Source code Edition License)";
        } else {
          echo " (Standart Edition License)";
        }
    } else {
      echo " (Freeware Edition)";
    }
    echo  '</td></tr>';

    echo '<tr><td width="400">Server name:</td><td width="250">' . get_server_name() . '</td></tr>';
    echo '<tr><td>PHP upload limit (in KB): </td><td>' . getMaximumUploadSize() . '</td></tr>';
    echo '<tr><td>PHP memory limit (in KB):&nbsp;&nbsp;&nbsp;</td><td>' . $limit . '</td></tr>';
    echo '<tr><td>Safe mode:</td><td>';
    echo (ini_get('safe_mode') == 1) ? 'ON<br>You maybe have some limitations creating folders or uploading<br>if the permissions are not set properly.<br>Please check the TWG FAQ 30 if you want to know more about<br>safe mode and the problems that comes with this setting.' : 'OFF';
    echo '</td></tr><tr><td>GD lib:</td><td>';
    echo (!function_exists('imagecreatetruecolor')) ? '<font color="red">GDlib is not installed properly.<br>TFU Preview does not work!</font>' : 'Available';
    echo '</td></tr>';
    echo '<tr><td>Max resize resolution (GDlib):</td><td>';
    if (!$limit) {
        echo '<font color="green">No limit</font>';
    } else {
        $xy = $limit * 1024 / 6.6;  // 10 % overhead. 
        $x = floor(sqrt ($xy / 0.75));
        $y = floor(sqrt($xy / 1.33));
        if ($x > 4000) {
            echo '<font color="green">~ ' . $x . ' x ' . $y . '</font>';
        } else if ($x > 2000) {
            echo '<font color="orange">~ ' . $x . ' x ' . $y . '</font>';
        } else {
            echo '<font color="red">~ ' . $x . ' x ' . $y . '</font>';
        }
    }
    echo '</td></tr>';
    $test = check_image_magic("",$check_image_magic);
    echo '<tr><td>Image magick support:&nbsp;&nbsp;&nbsp;</td><td>' . (($test == '1') ? '<font color="green">Available</font>' : (($test == '0') ? '<b><font color="red">Not available</b><br>(or test could not be performed!)</font>': '<font color="red">Test disabled</font>')) . '</td></tr>';
    echo '<tr><td>The times below have to be longer than the maximum<br>upload duration! Otherwise the upload will fail.</td><td>&nbsp;</td></tr>';
    echo '<tr><td>PHP maximum execution time: </td><td>' . ini_get('max_execution_time') . ' s</td></tr>';
    echo '<tr><td>PHP maximum input time: </td><td>' . ini_get('max_input_time') . ' s</td></tr>';
    echo '<tr><td>PHP default socket timeout: </td><td>' . ini_get('default_socket_timeout') . ' s</td></tr>';
    echo '</table>';
    echo '</div>';
}

/*
  We check if we can create a image with image magick
  The check is only done once per session because in JFU this function is called every time we access
  a profile page. If this function has a problem because of the execute you would never get to the config page.
*/
function check_image_magic($path = "", $check_image_magic = true) {
   global $folder, $image_magic_path;

   if ($check_image_magic) {
     if (isset($_SESSION['IM_CHECK'])) {
        return ($_SESSION['IM_CHECK']);
     }
     $inputimage = dirname(__FILE__) . "/lang/de.gif";
     set_error_handler('on_error_no_output'); // is needed because the cache folder could be not reachable because of base_dir restrictions
     if (!file_exists($folder)) {
       $folder =  dirname(__FILE__) . "/../../../../cache"; // if the tfu folder is in the administration
          if (!file_exists($folder) && (!is_writeable($folder))) {
            $folder =  dirname(__FILE__) . "/../../../cache"; // if the tfu folder is in the frontend
            if (!file_exists($folder) && (!is_writeable($folder))) {
            // now we check if we can do the test in the local directoy
            $folder = dirname(__FILE__);
            if (!is_writeable($folder)) {
              set_error_handler('on_error');
              return '0';
            }
          }
       }
     }
     set_error_handler('on_error');

     if ($path != "") {
       $image_magic_path = $path;
     }
     $outputcachetest = $folder . "/_image_magick_test.jpg";
     $fh=fopen($outputcachetest,'w'); // fix for a but in some php - versions - thanks to Anders
     fclose($fh);
     $command = $image_magic_path. " \"" .  realpath($inputimage) . "\" -quality 80 -resize 120x81  \"" . realpath($outputcachetest) . "\"";
     @unlink($outputcachetest);
     $_SESSION['IM_CHECK'] = '0';
     execute_command($command);
     if (file_exists($outputcachetest)) {
        $ok = '1';
        @unlink($outputcachetest);
        $_SESSION['IM_CHECK'] = '1';
     } else {
       $ok = '0';
     }
     return $ok;
   } else {
    return '-1';
   }
}

/**
 *  Normalizes the file names - fix_encoding has to be called before this function is used.
 *  This isn't done here because normalize filenames is an optional step while fix_encoding
 *  is always done.
 **/
function normalizeFileNames($imageName){
   global $normalizeSpaces;

  // it's needed to decode first because str_replace does not handle str_replace in utf-8
  $imageName = utf8_decode($imageName);
  // we make the file name lowercase ִײÜ as well.
  // seems not to be available on all systems.
  if (function_exists("mb_strtolower")) { 
    $imageName = mb_strtolower($imageName); 
  } else {
    $imageName = strtolower($imageName); 
  }
  
  if ($normalizeSpaces == 'true') {
    $imageName=str_replace(' ','_',$imageName);
  }
  // Some characters I know how to fix ;).
  $imageName=str_replace(array('ה','צ','ü','ß'),array('ae','oe','ue','ss'),$imageName);
  // and some others might need
  $imageName=str_replace(array('ב','א','ד','ג','ח','¢','י','ך','ט','כ','ם','מ','ן','ל','ס','פ','ף','ץ','ע','','ת','ש','û','ü','‎','ÿ',''),
                         array('a','a','a','a','c','c','e','e','e','e','i','i','i','i','n','o','o','o','o','s','u','u','u','u','y','y','z'),$imageName);
 
  // we remove the rest of unwanted chars
  $patterns[] = '/[\x7b-\xff]/';  // remove all characters above the letter z.  This will eliminate some non-English language letters
  $patterns[] = '/[\x21-\x2c]/'; // remove range of shifted characters on keyboard - !"#$%&'()*+
  $patterns[] = '/[\x5b-\x60]/'; // remove range including brackets - []\^_`
  // we remove all kind of special characters for utf8 encoding as well
  $patterns[] = '/[\x7b-\xff]/u';  // remove all characters above the letter z.  This will eliminate some non-English language letters
  $patterns[] = '/[\x21-\x2c]/u'; // remove range of shifted characters on keyboard - !"#$%&'()*+
  $patterns[] = '/[\x5b-\x60]/u'; // remove range including brackets - []\^_`
  $replacement ="_";
  return utf8_encode(preg_replace($patterns, $replacement, $imageName));
}

function execute_command ($command) {
  $use_shell_exec = true;;
  ob_start();
  set_error_handler('on_error_no_output');
  if (substr(@php_uname(), 0, 7) == "Windows"){
  	   // Make a new instance of the COM object
  		$WshShell = new COM("WScript.Shell");
  		 // Make the command window but dont show it.
  	   $oExec = $WshShell->Run("cmd /C " . $command, 0, true);
  } else {
      if ($use_shell_exec) {
         shell_exec($command);
       } else {
  	      exec($command . " > /dev/null");
       }
  }
  set_error_handler('on_error');
  ob_end_clean();
}

/*
          Functions used for file handling
*/
function tfu_rename_file($dir, $file, $enable_file_rename, $keep_file_extension, $fix_utf8) {
    global $normalise_file_names;
    if ($enable_file_rename != 'true') {
        echo 'This action is not enabled!';
        exit(0);
    }
    $newName = parseInputParameterFile(trim(my_basename(' ' . $_GET['newfilename']))); // fixes that file can be renamed to an upper dir.
    if ($normalise_file_names) {
       $newName = normalizeFileNames($newName);
    }
    $newName = fix_decoding($newName, $fix_utf8);
    if ($keep_file_extension == 'true') {
        $newNameEx = getExtension($newName);
        $fileEx = getExtension($file);
        if (strtolower($newNameEx) != strtolower($fileEx)) {
            echo 'This action is not allowed. Changing file extensions is disabled because of security issues!';
            exit(0);
        }
    }

    $newName = $dir . '/' . $newName;
    if (!file_exists($newName) || ($file != $newName && strtolower($file) == strtolower($newName) )) {  // file_exists does not check case sensitive on windows
        if (is_writeable($file)) {
            $result = @rename($file, $newName);
            if ($result) {
                echo '&result=true';
            } else {
                echo '&result=false';
            }
        } else {
            echo '&result=perm';
        }
    } else {
        echo '&result=exists';
    }
}

function tfu_delete_file($file, $show_delete) {
    // first we check if delete is enabled!
    if ($show_delete != 'true') {
        echo 'This action is not enabled!';
        exit(0);
    }
    if (is_tfu_deletable($file)) {
        set_error_handler('on_error_no_output');
        @chmod($file , 0777);
        set_error_handler('on_error');
        $result = @unlink($file);
        if ($result) {
            echo '&result=true';
        } else {
            echo '&result=false';
        }
    } else {
        echo '&result=perm';
    }
}

function tfu_delete_files($file, $show_delete) {
    // first we check if delete is enabled!
    if ($show_delete != 'true') {
        echo 'This action is not enabled!';
        exit(0);
    }
    $deleted = 0;
    $perm = 0;
    $notdel = 0;
    foreach ($file as $ff) {
        if (is_tfu_deletable($ff)) {
            set_error_handler('on_error_no_output');
            @chmod($ff , 0777);
            set_error_handler('on_error');
            $result = @unlink($ff);
            if ($result) {
                $deleted++;
            } else {
                $notdel++;
            }
        } else {
            $perm++;
        }
    }

    echo '&result=multiple&nr_del=' . $deleted . '&nr_perm=' . $perm . '&nr_not_del=' . $notdel;
}

function tfu_copy_move($dir, $file, $enable_file_copymove, $enable_folder_copymove ) {
    // first we check if delete is enabled!
    if ($enable_file_copymove != 'true' && $enable_folder_copymove != 'true') {
        echo 'This action is not enabled!';
        exit(0);
    }
    $done = 0;
    $total = 0;
    $error = 0;
    $exists = 0;
    resetSessionTree();
    $overwrite = parseInputParameter($_GET['overwrite']);
    $folder = getDestinationFolder(parseInputParameter($_GET['target']));
    $dest_folder = $folder . '/' . my_basename($dir);
    if ($_GET['copyfolder'] == 'true') {
        if ($folder == $dir) {
            $error = 1;
        } else if (strpos ($folder, $dir) !== false) {
            $error = 2;
        } else if ($overwrite == 'false' && file_exists($dest_folder)) {
            $error = 3;
        } else {
            if (@rename($dir, $dest_folder)) {
                $done = 1;
                $upperdir = substr($dir, 0, strrpos ($dir, "/"));
                $_SESSION['TFU_DIR'] = $upperdir;
            } else {
                $error = 4;
            }
        }
    } else {
        foreach ($file as $ff) {
            $total++;
            $dest = $folder . '/' . my_basename($ff);
            if ($_GET['type'] == 'c') {
                if ($folder == $dir) {
                    $u_file = get_unique_filename($folder, my_basename($ff));
                    $dest = $folder . "/" . $u_file;
                }
                if (file_exists($dest) && $overwrite == 'false') { // if file exists and not overwrite = error
                    $exists++;
                } else {
                    if ($ff == $dest) {
                        $nr = 2;
                        $dest = $folder . '/Copy of ' . my_basename($ff);
                        while (file_exists($dest)) {
                            $dest = $folder . '/Copy (' . $nr++ . ') of ' . my_basename($ff);
                        }
                    }
                    if (@copy($ff, $dest)) {
                        $done++;
                    } else {
                        $error++;
                    }
                }
            } else {
                if ($ff != $dest) {
                    if (file_exists($dest) && $overwrite) {
                        @unlink($dest);
                    }
                    if (!file_exists($dest)) {
                        if (@rename($ff, $dest)) {
                            $done++;
                        } else {
                            $error++;
                        }
                    }
                }
            }
        }
    }
    echo '&total=' . $total . '&ok=' . $done . '&error=' . $error . '&exists=' . $exists ;
}

function tfu_preview($file) {
    global $use_image_magic, $image_magic_path, $pdf_thumb_format;
    $pdf_preview = false;
    if (file_exists(dirname(__FILE__) . '/thumbs') && is_writable(dirname(__FILE__) . '/thumbs')) { // is a caching dir available and writeable?
        $pdf_preview = true;
    }

    // we store the url of the last preview image in the session - use it if you need it ;).
    // we generate thumbs for jpge,png and gif!
    if (preg_match("/.*\.(j|J)(p|P)(e|E){0,1}(g|G)$/", $file) ||
            preg_match("/.*\.(p|P)(n|N)(g|G)$/", $file) ||
            preg_match("/.*\.(g|G)(i|I)(f|F)$/", $file)) {
        if (isset($_GET['big'])) {
            send_thumb($file, 90, 440, 280); // big preview 4x bigger!
        } else {
            send_thumb($file, 90, 80, 55); // small preview
        }
    } else if (preg_match("/.*\.(p|P)(d|D)(f|F)$/", $file) && $use_image_magic && $pdf_preview) {
            $cachename = dirname(__FILE__) . '/thumbs/' . sha1($file) . '.' . $pdf_thumb_format;
            if (!file_exists($cachename)) {
              $ima = realpath($file);
              $resize = '1000x1000';
              $command = $image_magic_path . ' -colorspace rgb "' . $ima . '[0]" -border 1x1 -quality 80 -thumbnail ' . $resize . ' "' . $cachename . '"';
              execute_command ($command);
            }
            if (isset($_GET['big'])) {
              send_thumb($cachename, 90, 440, 280); // big preview 4x bigger!
            } else {
              send_thumb($cachename, 90, 80, 55); // small preview
            }
            // the cleanup is done in the thumbs folder which is cleaned up regularly
            // @unlink($cachename);
            return;
    } else {
        return; // we return nothing if no image.
    }
}

function tfu_createThumb($file) {
      global $compression, $use_image_magic, $image_magic_path, $pdf_thumb_format;
      if (!(preg_match("/.*\.(p|P)(d|D)(f|F)$/", $file))) {
        $name = removeExtension($file) . "-" . $_GET['tfu_width'] . 'x' . $_GET['tfu_height'] . "." . getExtension($file);
        resize_file($file, $_GET['tfu_width'] . 'x' . $_GET['tfu_height'], $compression, basename($file), $name); 
        unset($_SESSION['TFU_LAST_UPLOADS']);
        $_SESSION['TFU_LAST_UPLOADS'] = array();
        $_SESSION['TFU_LAST_UPLOADS'][] = $name;
      } else if ($use_image_magic) {
          $name = dirname(__FILE__) . '/' . removeExtension($file) . "-" . $_GET['tfu_width'] . '.' . $pdf_thumb_format;
          // create a pdf thumbnail
          $ima = realpath($file);
          if (!file_exists($name)) {
             $ima = realpath($file);
             $resize = $_GET['tfu_width'] . 'x' . $_GET['tfu_height'];
             $command = $image_magic_path . ' -colorspace rgb "' . $ima . '[0]" -border 1x1 -quality 80 -thumbnail ' . $resize . ' "' . $name . '"';
             execute_command ($command);
          }
          unset($_SESSION['TFU_LAST_UPLOADS']);
          $_SESSION['TFU_LAST_UPLOADS'] = array();
          $_SESSION['TFU_LAST_UPLOADS'][] = $name;
      }     
}

function tfu_info($file) {
    global $use_image_magic;
    unset($_SESSION['TFU_LAST_UPLOADS']);
    $_SESSION['TFU_LAST_PREVIEW'] = fixUrl(getRootUrl() . $file);
    echo '&size=' . sprintf("%u", @filesize($file));
    // we check if the image can be resized
    if (is_supported_tfu_image($file,$file)) {
        set_error_handler('on_error_no_output'); // is needed because error are most likly but we don't care about fields we don't even know
        $oldsize = @getimagesize($file);
        set_error_handler('on_error');
        if ($oldsize) {
            if (isMemoryOk($oldsize, 400, "")) {
                echo '&hasPreview=true&tfu_x=' . $oldsize[0] . '&tfu_y=' . $oldsize[1] ; // has preview!
            } else {
                echo '&hasPreview=error&tfu_x=0&tfu_y=0'; // too big! - same error massage as hasPreview=false
            }
            return;
        }
        echo '&hasPreview=false'; // no image!
    }
    if (preg_match("/.*\.(p|P)(d|D)(f|F)$/", $file) && $use_image_magic &&
        file_exists(dirname(__FILE__) . '/thumbs') && is_writable(dirname(__FILE__) . '/thumbs')) {  // check if pdf
       echo '&hasPreview=true&tfu_x=1000&tfu_y=1000'; // has preview! - pdfs are max 1000x1000';
       return;
    }
    echo '&hasPreview=false&tfu_x=0&tfu_y=0';
}

function tfu_upload_info($dir) {
    $last_file = $_SESSION['TFU_LAST_UPLOADS'][0];
    $index = getFileName($dir,my_basename($last_file));    
    echo '&filename_index='.$index.'&filename=' .my_basename($last_file);
    
     $file = $last_file; 
     if (is_supported_tfu_image($file,$file)) {
        set_error_handler('on_error_no_output'); // is needed because error are most likly but we don't care about fields we don't even know
        $oldsize = @getimagesize($file);
        set_error_handler('on_error');
        if ($oldsize) {
            if (isMemoryOk($oldsize, 400, "")) {
                echo '&hasPreview=true&tfu_x=' . $oldsize[0] . '&tfu_y=' . $oldsize[1] ; // has preview!
            } else {
                echo '&hasPreview=error&tfu_x=0&tfu_y=0'; // too big! - same error massage as hasPreview=false
            }
            return;
        }
        echo '&hasPreview=false'; // no image!
    }
    if (preg_match("/.*\.(p|P)(d|D)(f|F)$/", $file) && $use_image_magic &&
        file_exists(dirname(__FILE__) . '/thumbs') && is_writable(dirname(__FILE__) . '/thumbs')) {  // check if pdf
       echo '&hasPreview=true&tfu_x=1000&tfu_y=1000'; // has preview! - pdfs are max 1000x1000';
       return;
    }
    echo '&hasPreview=false&tfu_x=0&tfu_y=0';
    
}

function  tfu_text($file) {
    if (is_writable($file)) {
        echo '&writeable=true';
    } else {
        echo '&writeable=false';
    }
    echo '&data=';
    $enc = 'UTF-8';
    $format = 'UNIX';
    $fp = fopen($file, 'rb');
    $content = '';
    if (filesize ($file) > 0) {
      $content = fread ($fp, filesize ($file));
    }
    // we replace \r with nothing
    $content_new = str_replace("\r", "", $content);
    if ($content_new != $content) {
        $format = 'DOS';
    }
    if (!tfu_seems_utf8($content_new)) {
        $content_new = cp1252_to_utf8($content_new);
        $enc = 'ANSI';
    }
    echo urlencode($content_new);
    echo '&encoding=' . $enc;
    echo '&format=' . $format;
    fclose($fp);
}

function tfu_savetext($file, $overwrite=true) {
    if (file_exists($file) && !$overwrite) {
      echo "&create_file=exists";
    } else {
      $content = urldecode($_POST['data']);
      if ($_POST['encoding'] == 'ANSI') {
          $content = utf8_to_cp1252($content);
      }
      if ($_POST['format'] == 'DOS') {
          $content = preg_replace("/\r\n|\r|\n/", chr(13) . chr(10), $content);
      } else {
          $content = preg_replace("/\r\n|\r|\n/", chr(10), $content);
      }
      // now we write the file again
      $file_local = fopen($file, 'w');
      if (getExtension($file) == 'php') { // we remove leading and trailing spaces returns if it is a php file!
          $content = trim($content);
      }
      fputs($file_local, $content);
      fclose($file_local);
      
      if (file_exists($file)) {
        echo "&create_file=true";
      } else {
        echo "&create_file=false";
      }
    }  
}

function tfu_download($file, $enable_file_download) {
    if ($enable_file_download == 'false' && !isset($_GET['fullscreen'])) {
        echo 'This action is not enabled!';
        exit(0);
    }

    if (isset($_GET['fullscreen'])) { // we check if we have an image in the cache folder!
       $cachename = dirname(__FILE__) . '/thumbs/' . sha1($file) . '.jpg';
       if (file_exists($cachename)) {
         $file = $cachename;
       }
    }
    ini_set('zlib.output_compression','Off');
    header("Content-Transfer-Encoding: binary");
    header('Content-type: application/octet-stream');
    header('Content-Length: ' . filesize($file));

    // small chunk size is used for IE! 1024 would be better but has only half the dl speed - 2048 seems to be the best trade off.
    $fp = fopen($file, 'rb');
    while ($content = fread($fp, 2048)) {
        echo $content;
        set_error_handler('on_error_no_output');
        @set_time_limit(20);
        @flush();
        @ob_flush(); // some server need this althou on some server this throws a warning
        set_error_handler('on_error');
    }
    fclose($fp);
}

function tfu_zip_download($files, $enable_file_download) {
    global $zip_folder, $zip_file_pattern; // The folder is used to create the temp download files!
    if ($enable_file_download == 'false' && !isset($_GET['fullscreen'])) {
        echo 'This action is not enabled!';
        exit(0);
    }

    /*
    $createZip = new createZip;
    $nrfiles = count($files);
    for ($i = 0; $i < $nrfiles; $i++) {
      $createZip -> addFile(file_get_contents($files[$i]), my_basename($files[$i]));
    }
    $fileName = $zip_folder . '/' . $_GET['zipname'];
    $fd = fopen ($fileName, "wb");
    $out = fwrite ($fd, $createZip -> getZippedfile());
    fclose ($fd);
    */
    $nrfiles = count($files);

    if ($zip_file_pattern == '') {
      $zipName = parseInputParameterFile(trim(my_basename(' ' . $_GET['zipname']))); // fixes that file can be renamed to an upper dir.
      $fileName = $zip_folder . '/' . $zipName;
    } else {
      // zip file pattern can have the following patterns {folder}, {number}, {date} e.g. "download-{number}-files_{date}.zip"
      // but here I only use {number} and {date} because this is enough to be unique. The filename itself is build in the flash.
      $newName = str_replace('{number}', $nrfiles, $zip_file_pattern);
      $newName = str_replace('{date}', date("Y-m-d"), $newName);
      $fileName = $zip_folder . '/' . $newName;
    }
    
    if (!is_writeable($zip_folder)) {
      tfu_debug("ERROR: The folder '" . $zip_folder . "' is not writeable. Please set the permissions properly to enable the download of multiple files.");
    }
    
    $fd = @fopen ($fileName, "wb");
    if ($fd) {
      $createZip = new TFUZipFile($fd);
      for ($i = 0; $i < $nrfiles; $i++) {
        $createZip -> addFile($files[$i], my_basename($files[$i]));
      }
      $createZip -> close();
  
      tfu_download($fileName, $enable_file_download);
      @unlink($fileName);
    } else {
      tfu_debug("ERROR: The file '" . $fileName . "' could not be created. Please set the permissions properly to enable the download of multiple files.");
    }
}

/*
          Functions used for directory handling
*/
function create_dir($dir, $enable_folder_creation, $fix_utf8) {
    global $normalise_directory_names, $dir_chmod;
    global $ftp_enable, $ftp_host, $ftp_port, $ftp_user, $ftp_pass, $ftp_root, $master_profile;
    if ($enable_folder_creation != 'true') {
            echo 'This action is not enabled!';
            exit(0);
        }
        resetSessionTree();
        $newdir = parseInputParameterFile(trim(my_basename(' ' . $_GET['newdir'])));
        if ($normalise_directory_names) {
           $newdir = normalizeFileNames($newdir);
        }
        $newdir = fix_decoding($newdir, $fix_utf8);
        $createdir = $dir . "/" . $newdir;
        if (file_exists($createdir)) {
            $status = '&create_dir=exists';
        } else {
            if (isset($ftp_enable) && $ftp_enable) {
               if ($master_profile) {
                 // we have to remove one level from the TFU_ROOT_DIR that was added automatically by the jfu wrapper
                 $parent_root = dirname($_SESSION['TFU_ROOT_DIR']);
                 $ftp_createdir = substr($createdir,strlen($parent_root)+1);
              } else {
               $ftp_createdir = substr($createdir,strlen($_SESSION['TFU_ROOT_DIR'])+1);
              }
              $conn_id = ftp_connect($ftp_host, $ftp_port);
              $login_result = ftp_login($conn_id, $ftp_user, $ftp_pass);
              ftp_chdir($conn_id, $ftp_root);
              $result = ftp_mkdir ($conn_id , $ftp_createdir);
              if ($result && $dir_chmod != 0) {
                @ftp_chmod($conn_id, $dir_chmod, $ftp_createdir);
              }
              ftp_close($conn_id);
            } else {
              $result = mkdir ($createdir);
              if ($result && $dir_chmod != 0) {
                @chmod($createdir, $dir_chmod);
              }
            }
            $status = ($result) ? '&create_dir=true':'&create_dir=false';
        }
    return $status;
}

function rename_dir( &$dir, $enable_folder_rename, $fix_utf8) {
    global $normalise_directory_names;

    if ($enable_folder_rename != 'true') {
            echo 'This action is not enabled!';
            exit(0);
        }
        resetSessionTree();
        $upperdir = substr($dir, 0, strrpos ($dir, "/"));
        $newdir = parseInputParameterFile(trim(my_basename(' ' . $_GET['newdir'])));
        if ($normalise_directory_names) {
           $newdir = normalizeFileNames($newdir);
        }
        $newdir = fix_decoding($newdir, $fix_utf8);
        if ($dir == $_SESSION["TFU_ROOT_DIR"]) {
            $status = "&rename_dir=main";
        } else {
            $createdir = $upperdir . "/" . $newdir;
            if (file_exists($createdir)) {
                $status = "&rename_dir=exists";
            } else {
                $result = rename ($dir, $upperdir . "/" . $newdir);
                if ($result) {
                    $dir = $createdir;
                    $_SESSION["TFU_DIR"] = $dir;
                    $status = "&rename_dir=true";
                } else {
                    $status = "&rename_dir=false";
                }
            }
        }
    return $status;
}

function delete_folder(&$dir, $enable_folder_deletion, $fix_utf8) {
   global $folder; // the root folder!
   if ($enable_folder_deletion != 'true') {
            echo 'This action is not enabled!';
            exit(0);
    }
    // we check if this is the root dir. We don't allow this even when the request was faked!
    if ($dir == $folder) {
       echo 'Deleting the root folder is not allowed!';
       exit(0);
    }
    resetSessionTree();
    $upperdir = substr($dir, 0, strrpos ($dir, "/"));
    $result = remove($dir);
    if ($result) {
        $status = "&delete_dir=true";
        $dir = $upperdir;
        $_SESSION["TFU_DIR"] = $dir;
    } else {
        $status = "&delete_dir=false";
    }
   return $status;
}

function change_folder($dir, $show_root, $enable_folder_browsing, $exclude_directories, $sort_directores_by_date) {
    global $hide_hidden_files;

    if ($enable_folder_browsing != 'true') {
        echo 'This action is not enabled!';
        exit(0);
    }
    $index = parseInputParameter($_GET['index']);
    if ($index == 0 && $show_root) { // we go up!
        $dir = substr($_SESSION["TFU_DIR"], 0, strrpos ($_SESSION["TFU_DIR"], "/"));
    } else { // we go deeper
        if ($show_root) {
            $index--;
        }
        $dirhandle = opendir($dir);
        $myDirs = array();
        while (false !== ($filed = readdir($dirhandle))) {
            if ($filed != "." && $filed != ".." && !in_array($filed, $exclude_directories) && (!($hide_hidden_files && (strpos($filed, '.') === 0)))) {
                if (is_dir($dir . '/' . $filed)) {
                   if ($sort_directores_by_date) {
                     $fdate = filemtime($dir . '/' . $filed);
                     $filed = $fdate . $filed;
                   }    
                    array_push($myDirs, $filed);
                }
            }
        }
        if ($sort_directores_by_date) {
            usort ($myDirs, "cmp_date_dec");
            $i = 0;
            foreach ($myDirs as $fieldName) {
                $myDirs[$i++] = substr($fieldName, 10);
            }
        } else {
            usort ($myDirs, "cmp_dir_dec");
        }
        $dir = $dir . "/" . $myDirs[$index];
    }
    $_SESSION["TFU_DIR"] = $dir;
    return $dir;
}

function create_directory_title($dir, $hide_directory_in_title, $truncate_dir_in_title, $fix_utf8) {
    if ($hide_directory_in_title == 'true') {
        $dirsub = " ";
    } else if ($truncate_dir_in_title == 'true') {
      $root = isset($_SESSION["TFU_ROOT_DIR"]) ? $_SESSION["TFU_ROOT_DIR"] : '';
      $folder =  substr($dir, strlen($root) + 1);
      $dirsub = " - Upload Folder: " . $folder;
    } else {
        // we only show the path - relative path is not shown! Therefore I replace some things.
        $dirsub = ($fix_utf8 == "") ? utf8_encode(str_replace("../", "", $dir)) : str_replace("../", "", $dir);
        $dirsub = str_replace("..", "", str_replace("//", "/", $dirsub)); // display fixes
        $dirsub = " - Upload Folder: " . $dirsub;
    }
    return $dirsub;
}

function read_dir($dir, &$myFiles, &$myDirs, $fix_utf8, $exclude_directories, $sort_files_by_date, $sort_directores_by_date) {
    global $hide_hidden_files, $show_server_date_instead_size;
    $size = 0;
    if (!file_exists($dir)) {
      return;
    }
    $dirhandle = opendir($dir);
    while (false !== ($file = readdir($dirhandle))) {
        if ($file != "." && $file != ".." && !in_array($file, $exclude_directories) && (!($hide_hidden_files && (strpos($file, '.') === 0)))) {
            $filepath = $dir . '/' . $file;
            if (is_dir($filepath)) {
                $dirname = fix_encoding($file, $fix_utf8);
                if ($sort_directores_by_date) {
                  $dirname = filemtime($filepath) . $dirname;
                }
                array_push($myDirs, urlencode($dirname));
            } else if (check_view_extension($file)) {
                set_error_handler("on_error_no_output");
                $current_size = sprintf("%u", @filesize($dir . '/' . $file));
                $size += $current_size;
                if ($show_server_date_instead_size=='true' || $sort_files_by_date) {
                   $fdate = filemtime($filepath);
                }
                if ($show_server_date_instead_size=='true') {
                   $current_size = $fdate;
                }
                // size or date is added.
                $file = $file . "**" . $current_size;
                if ($sort_files_by_date) {
                    $file = $fdate . $file;
                }
                set_error_handler("on_error");
                array_push($myFiles, urlencode(fix_encoding($file, $fix_utf8)));
            }
        }
    }
    closedir ($dirhandle);
    return $size;
}

function sort_data (&$myFiles, &$myDirs, $sort_files_by_date, $sort_directores_by_date) {
    if ($sort_files_by_date) {
        usort ($myFiles, "cmp_date_dec");
        $i = 0;
        foreach ($myFiles as $fieldName) {
            $myFiles[$i] = substr($myFiles[$i], 10);
            $i++;
        }
    } else {
        usort ($myFiles, "cmp_dec");
    }
    reset($myFiles);

    if ($sort_directores_by_date) {
        usort ($myDirs, "cmp_date_dec");
        $i = 0;
        foreach ($myDirs as $fieldName) {
            $myDirs[$i] = substr($myDirs[$i], 10);
            $i++;
        }
    } else {
        usort ($myDirs, "cmp_dir_dec");
    }
    reset($myDirs);
}

function check_restrictions($dir, $show_root, &$myFiles, $fix_utf8, $status) {
    global $enable_dir_create_detection, $check_safemode;
    // this is a check if the dir exists - this is a configuration error!
    if (file_exists($dir)) {
      $status .=  "&dir_exists=true";
    } else {
      $status .=  "&dir_exists=false";
      // no other checks are made because the directory is not available!
      return $status;
    }
    // now we check if we can delete the current folder - root folder cannot be deleted!
    $status .= (is_tfu_deletable($dir) && $show_root) ? "&dir_delete=true" : "&dir_delete=false";
    // new we check if we can create folders - we have to check safemode too!
    set_error_handler("on_error_no_output");
    $sm_prob = $check_safemode && has_safemode_problem_global() && runsNotAsCgi();

    if (is_writeable($dir)) {
        if ($enable_dir_create_detection) { // the detection of the safemode does not work on all systems - therefore it can be disabled.
          $status .= ($sm_prob) ? "&dir_create=subdir" : "&dir_create=true";
        } else {
          $status .= "&dir_create=true";
        }
    } else {
        $status .= ($sm_prob) ? "&dir_create=safemode" : "&dir_create=false";
    }
    set_error_handler("on_error");

    $nrFiles = count($myFiles);
    // now we check if can delete files - we only check the 1st file!
    if ($nrFiles > 0) { 
        $delfile = fix_decoding(urldecode($myFiles[0]), $fix_utf8);
        // we have to remove the ** before checking
        $delfile = substr($delfile, 0, strpos($delfile, "**"));
        $status .= (is_tfu_deletable($dir . "/" . $delfile)) ? "&file_delete=true" : "&file_delete=false";
    }
   return $status;
}

function get_server_name() {
  if(isset($_SERVER['HTTP_HOST'])) {
    $domain = $_SERVER['HTTP_HOST'];
  } else if(isset($_SERVER['SERVER_NAME'])) {
   $domain = $_SERVER['SERVER_NAME'];
  } else {
    $domain = '';
  }
  $port = strpos($domain, ':');
  if ( $port !== false ) $domain = substr($domain, 0, $port);
  return $domain;
}


/*
encodes only the part without the /
*/
function tfu_urlencode($data)
{
	$data = str_replace("/", "__TWG__", $data);
	$data = str_replace(":", "__QT__", $data);
	$parts = explode (" - ", $data); // descripton should not be encoded
	if (count($parts) > 1) {
      	$parts[0] = rawurlencode ($parts[0]);
      	$data = implode(" - ", $parts);
    } else {
      	$data = rawurlencode ($data);
    }
	$data = str_replace("__QT__", ":", $data);
	return str_replace("__TWG__", "/", $data);
}

// only executes basename if a / or \ is in the filename.
// Fixes the problem that basename e.g. destroys chinese filename encoded in utf-8
function my_basename($name) {
  if ((strpos($name, '\\') === false) && (strpos($name, '/') === false)) {
    return $name;
  } else {
    $sep = ((strpos($name, '/') === false)) ?  '\\' : '/';
    return ltrim(substr($name, strrpos($name, $sep) ), $sep);
  }
}

/**
 * Fixes the encoding of the file names we get from the flash. They come utf-8 encoded
 * from the flash and writing this directly to the filesystem produces depending on the
 * system unreadable file names. Especially if special characters like צהü or even
 * chinese Characters are used.
*/
function fix_decoding($encoded_filename, $fix_utf8) {
  if ($fix_utf8 == 'none') {
    return $encoded_filename;
  } else {
    $temp = str_replace("\\'", "'", $encoded_filename ); // we change escaped ' to normal '
    return ($fix_utf8 == '') ? utf8_decode($temp ) :  iconv('UTF-8', $fix_utf8, $temp);
  }
}

function fix_encoding($decoded_filename, $fix_utf8) {
  if ($fix_utf8 == 'none') {
    return $decoded_filename;
  } else {
    return ($fix_utf8 == '') ? utf8_encode($decoded_filename) : iconv($fix_utf8, 'UTF-8', $decoded_filename);
  }
}

// Specifiy the file_put_contents() function for PHP version 4
if (function_exists('file_put_contents') == false) {
  function file_put_contents($file, $string) {
  $f=fopen($file, 'w');
  fwrite($f, $string);
  fclose($f);
  }
}

function formatSize($size) {
		if ($size>=1048576*1000) {
			//  > 1000 MB - no komma - display e.g. 1421 MB
			$strSize = floor($size/1048576)." MB";
		} else if ($size>1048576*100) {
		  // > 100 MB - 1 digit - e.g.  71.5 MB
		  $num = floor($size/1048576.0);
			$strSize = $num . "." . floor(($size-($num*1048576))/104857.6) . " MB";
		} else if ($size>1048576) {
			// > 1 MB - 2 digit - e.g.  2.53 MB
			$num = floor($size/1048576.0);
			$komma = floor(($size-($num*1048576.0))/1048.576);
			$pad = "";
			if ($komma<100) {	$pad = "0";	}
			if ($komma<10) {	$pad = "00"; }
			$strSize = $num . "." . $pad. $komma ." KB";
		} else if ($size == 0) {
			$strSize = "0 KB";
		} else {
			$strSize = ceil($size/1024)." KB";
		}
		return $strSize;
}

function resetSessionTree() {
  unset($_SESSION["TREE_" . $_SESSION['TFU_ROOT_DIR']]);
}

function check_multiple_extensions($image, $remove_multiple_php_extension) {
  if ($remove_multiple_php_extension) {
    $ext = getExtension($image);
    if (substr($ext,0,2) != "php") {
      $image2 = str_replace(".php", "", $image);
      if ($image != $image2) {
          tfu_debug("SECURITY WARNING: Please check the file ".$image2.". It was uploaded with an image extensions and also a nested php extension. On some server this is a security problem (multiple extensions) and therefore the .php part of the file name was removed!" );
          $image = $image2;
      }
    }
  }
  return $image;
}


function getFolderSizeCached($path) {
     $md = md5($path);

     if (isset($_SESSION['TFU_TMP']) && isset($_SESSION['TFU_TMP']['FS' . $md] )) {
        echo "c";
        return $_SESSION['TFU_TMP']['FS' . $md];
     } else {
       echo "b";
       if (!isset($_SESSION['TFU_TMP'])) {
          $_SESSION['TFU_TMP'] = array();
       }
       $size = getFoldersize($path);
       $_SESSION['TFU_TMP']['FS' . $md] = $size;
       return $size;
     }
}

/**
 *   Optimized way to read the the size of a directoy.
 *
 *    First the windows or Unix way is tried. If this fails
 *    the php internal stuff is used.
 *
 *    if you select legacy = false only the pure php
 *    version is used.
 */
function getFoldersize($path, $legacy = true) {
  $size = -1;
  ob_start();
  set_error_handler('on_error_no_output');
  if ($legacy) {
       if (substr(@php_uname(), 0, 7) == "Windows"){
          // we have to make the path absolute !
          $path_ab = realpath($path);
          $obj = new COM ( 'scripting.filesystemobject' );
          if ( is_object ( $obj ) ) {
         	  $ref = $obj->getfolder ( $path_ab );
         	  $size = $ref->size;
         	  $obj = null;
         }
       } else { // hopefully unix -  du has to be in the path. If it is not you have to adjust the path.
         $io = popen ( 'du -sb ' . $path, 'r' );
         $usize = trim(fgets ( $io, 4096));
         $split = preg_split('/\s+/', $usize);
         $usize = $split[0];
         pclose ( $io );
         if (is_numeric($usize)) {
           $size = $usize;
         }
       }
  }
  set_error_handler('on_error');
  @ob_end_clean();
  // backup if both ways fail. It is ~ 18 times slower (tested on windows) than one of the solutions above.
  if ($size == -1) {
    $size = foldersize($path);
  }
  return $size;
}

/**
 *  The basic php way to go through all directories and adding the file sizes.
 *  It does also check the parameters $exclude_directories and $hide_hidden_files
 */
function foldersize($p) {
    global $exclude_directories, $hide_hidden_files;
    $size = 0;
    $fs = scandir($p);
    foreach($fs as $f) {
        if (is_dir(rtrim($p, '/') . '/' . $f)) {
            if ($f!='.' && $f!='..') {
                $size += foldersize(rtrim($p, '/') . '/' . $f);
            }
        } else {
            if ($f != '.' && $f != '..' && !in_array($f, $exclude_directories)&&
                (!($hide_hidden_files && (strpos($f, '.') === 0)))) {
              $size += filesize(rtrim($p, '/') . '/' . $f);
            }
        }
    }
    return $size;
}

function check_syntax($file)
{
// load file
$code = file_get_contents($file);
// remove non php blocks
$code = preg_replace('/(^|\?>).*?(<\?php|$)/i', '', $code);
// create lambda function
$f = @create_function('', $code);
// return function error status
return !empty($f);
}

function file_upload_error_message($error_code) {
    switch ($error_code) {
        case UPLOAD_ERR_INI_SIZE:
            return 'The uploaded file exceeds the upload_max_filesize directive in php.ini';
        case UPLOAD_ERR_FORM_SIZE:
            return 'The uploaded file exceeds the MAX_FILE_SIZE directive that was specified in the HTML form';
        case UPLOAD_ERR_PARTIAL:
            return 'The uploaded file was only partially uploaded. Please check your timeouts.';
        case UPLOAD_ERR_NO_FILE:
            return 'No file was uploaded';
        case UPLOAD_ERR_NO_TMP_DIR:
            return 'Missing a temporary folder';
        case UPLOAD_ERR_CANT_WRITE:
            return 'Failed to write file to disk';
        case UPLOAD_ERR_EXTENSION:
            return 'File upload stopped by extension';
        default:
            return 'Unknown upload error';
    }
}

@ob_end_clean();
?>