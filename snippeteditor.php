<?PHP
ini_set ("display_errors", "1");
error_reporting(E_ALL);
//error_reporting(E_ALL ^ E_NOTICE ^ E_WARNING);

//var_dump ($_REQUEST);
$text = "";
$theme = "";
$action = "";


isset($_REQUEST['action']) && $action = $_REQUEST['action'];

$action || $action = "list";
isset($_REQUEST['f']) && $filename = basename ($_REQUEST['f']);
// Note: I don't need stripslashes on my computer...but needed on server!
if (isset($_REQUEST['text'])) {
	$text = $_REQUEST['text'];
	if (get_magic_quotes_gpc ())
		$text = stripslashes($text);
	$text = stripslashes($_REQUEST['text']);
}
isset($_REQUEST['theme']) && $theme = $_REQUEST['theme'];
$theme || $theme = "default";


print "<div style='border:1px solid black;padding:20px;margin:10px;background:#FC0'>\n";
print "<h2>FP Snippet Editor</h2>\n";
print "</div>\n";

print "<div style='border:1px solid black;padding:20px;margin:10px;background:#CCC'>\n";
//print "Action: $action<BR>\n";
//print "Filename: $filename<BR>\n";


print '<form action="snippeteditor.php" enctype="multipart/form-data" method="post">';

$list = array ();
switch ($action) {
	case 'edit'	:
		$dir = null;
		isset($_REQUEST['dir']) && $dir = $_REQUEST['dir'];
		print "<div style='border:1px solid black;padding:20px;margin:10px;background:#EEA'>\n";
		editFile ($filename, $dir, $theme);
		print "</div>\n";
		break;
	case 'update' :
		$dir = null;
		isset($_REQUEST['dir']) && $dir = $_REQUEST['dir'];
		print "<div style='border:1px solid black;padding:20px;margin:10px;background:#EEA'>\n";
		$result = updateFile ($filename, $dir, $theme, $text);
		if ($result) 
			print "Successful update of $filename<BR>\n";
		displayList ($theme, $dir);
		print "</div>\n";
		break;
	case 'open' :
		$dir = null;
		isset($_REQUEST['dir']) && $dir = $_REQUEST['dir'];
		displayList ($theme, $dir);
		break;
	case 'restore' :
		$dir = null;
		isset($_REQUEST['dir']) && $dir = $_REQUEST['dir'];
		restoreFile($filename, $dir, $theme);
		displayList ($theme, $dir);
		break;
	default :
		displayList ($theme, null);
}

print '</form>';
print '</div><BR>';

// =============

function updateFile ($filename, $dir, $theme, $text) {
	print "Updated $filename in $dir<BR>";
	

	if (!$dir) {
		$dir = "_themes/$theme/_snippets";
	} else {
		//$dir = "_themes/$theme/_snippets";
	}
	
	$res = true;
	// make backup dir if necessary
	file_exists ("$dir/_backup") || mkdir ("$dir/_backup", 0777);
	
	// make copy of original if missing
	if (!file_exists ("$dir/_backup/$filename.bak")) {
		$res = copy ("$dir/$filename", "$dir/_backup/$filename.bak");
	}
	
	if ($res && WriteTextFile ("$dir/$filename", $text)) {
		$result = "Updated $filename";
	} else {
		$result = false;
	}

	return $result;
}

function restoreFile ($filename, $dir, $theme) {
	print "Restored $filename in $dir<BR>";
	

	if (!$dir) {
		$dir = "_themes/$theme/_snippets";
	} else {
		//$dir = "_themes/$theme/_snippets";
	}
	
	if (file_exists ("$dir/_backup") && copy ("$dir/_backup/$filename.bak", "$dir/$filename")) {
		$result = "Restored $filename";
		unlink("$dir/_backup/$filename.bak");
	} else {
		$result = false;
	}
	return $result;
}

function displayList ($theme, $dir) {
	$themelist = fetchThemeList ();
	print "<div style='border:1px solid black;padding:20px;margin:10px;background:#EEA'>\n";
	print "<h3>List of Themes:</h3>\n";
	print "<ol>\n\n";
	foreach ($themelist as $t) {
		print "<li><a href=\"./snippeteditor.php?action=list&theme=$t\">$t</a></li>\n\n";
		//print "<li>" . $t . "</li>\n\n";
	}
	print "</ol>\n\n";
	print "</div>\n";
	print "<div style='border:1px solid black;padding:20px;margin:10px;background:#EEA'>\n";
	
	if ($dir) {
		print "<h3>Sub-Folder : <em>$dir</em></h3>\n";
		print "<a href=\"./snippeteditor.php?dir=$dir&action=list&theme=$theme\">(Go up)</a>\n";
	} else {
		print "<h3>Snippets in : <em>$theme</em></h3>\n";
	}

	if (!$dir) {
		$dir = "_themes/$theme/_snippets";
	}
	
	
	// FILES
	$themefilelist = fetchFileList ($dir);
	
	//print 'Theme Name: <input name="theme" type="text" value="' . $theme . '" size="30">&nbsp;&nbsp;<button type="submit" value="list" name="action">Show List</button>';
	$k = 1;
	print "<ol style='width:60%;'>\n\n";
	
	$filelist = $themefilelist['files'];
	$dirlist = $themefilelist['dirs'];
	
	$bkgd1 = "#EEA";
	$bkgd2 = "#fafac8";
	$bkgd = $bkgd1;
	
	
	foreach ($filelist as $file) {
		$f = $file['filename'];
		$t = $file['type'];
		$fdisplay = basename ($f);
		
		$bakfile = basename ($f).".bak";
		if (file_exists ("$dir/_backup/$bakfile") ) {
			$restore = "<span style=''/><i><a href=\"./snippeteditor.php?f=$f&dir=$dir&action=restore&theme=$theme\">Restore Original Values</a></i></span>";
		} else {
			$restore = "<span style='margin-left:100px;'/><i>(unchanged)</i></span>";
		}

		if ($bkgd == $bkgd1) {
			$bkgd = $bkgd2;
		} else {
			$bkgd = $bkgd1;
		}
		print "<li style='background:$bkgd;height:1.5em;padding:3px;font:Trebuchet MS;'><div style='float:right;'/>$restore</div> <a href=\"./snippeteditor.php?f=$f&dir=$dir&action=edit&theme=$theme\">$fdisplay</a> <br style='clear:all;'/></li>\n\n";
		$k++;
	}
	print "</ol>\n\n";


	// SUB-FOLDERS

	print "<h3>Sub-Folders in <em>$dir</em></h3>\n";
	$k = 1;
	print "<ol>\n\n";
	
	foreach ($dirlist as $file) {
		$f = $file['filename'];
		$t = $file['type'];
		$fdisplay = basename ($f);
		print "<li><a href=\"./snippeteditor.php?dir=$f&action=open&theme=$theme\">$fdisplay</a>\n\n";
		$k++;
	}
	print "</ol>\n\n";
	
	//
	print "</div>\n";

}

function editFile ($filename, $dir, $theme) {
	
	if (!$dir) {
		$dir = "_themes/$theme/_snippets";
	}
	$fname = basename ($filename);
	print "<h2>Edit <em>$fname</em></h2>\n";
	$text = ReadTextFile ("$dir/$filename");
	print '
	<textarea name="text" rows="20" cols="120">' . $text . '</textarea><BR>';

	print "<input name='f' type='hidden' value='$filename'>\n";
	print "<input name='dir' type='hidden' value='$dir'>\n\n";
	print "<input name='theme' type='hidden' value='$theme'>\n\n";

	print '<button type="submit" name="action" value="update">Save Changes</button> <button type="submit" name="action" value="list">Cancel</button><BR>';
}

//------------------
// get contents of a file into a string
function ReadTextFile ($filename) {
	if (file_exists($filename)) {
		$contents = file_get_contents ($filename);
// 		$fd = fopen ($filename, "rb");
// 		$fs = filesize ($filename);
// 		if ($fs) {
// 			$contents = fread ($fd, $fs);
// 		} else {
// 			$contents = "";
// 		}
//		fclose ($fd);
		return $contents;
	} else {
		return "file $filename not found";
	}
}

function WriteTextFile ($filename, $str) {
// 	$mq = get_magic_quotes_runtime ();
// 	set_magic_quotes_runtime ( 0 );

	$result = file_put_contents ($filename, $str);
	
// 	$fp = fopen($filename, "w+b");
// 	if(!$fp) {
// 		return FALSE;
// 	}
// 	fputs($fp, $str);
// 	fclose ($fp);
// 	set_magic_quotes_runtime ($mq);
	return $result;
}

function fetchFileList ($dir) {
	$filelist = array ();
	$dirlist = array ();
	//foreach (glob("$dir/*.txt") as $filename) {
	foreach (glob("$dir/*") as $filename) {
		if (substr($filename,1,1) != ".") {
			$t = filetype($filename);
			if ($t == "dir") {
			 	if (basename($filename) !="_backup") {
					$dirlist[] = array ("filename"=>$filename, "type"=>filetype($filename) );
				}
			} else {
				$filelist[] = array ("filename"=>$filename, "type"=>filetype($filename) );
			}
		}
	}
	$a = array ("files" => $filelist, "dirs" => $dirlist);
	return $a;
}

function fetchThemeList () {
	$themes = array ();
	$filelist = array ();
	foreach (glob("./_themes/*") as $theme) {
		if (is_dir ($theme))
			$themes[] .= basename ($theme);
	}
	return $themes;
}

