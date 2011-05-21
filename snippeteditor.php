<?PHP
ini_set ("display_errors", "1");
error_reporting(E_ALL);
//error_reporting(E_ALL ^ E_NOTICE ^ E_WARNING);

//var_dump ($_REQUEST);
$text = "";
$theme = "";

$action = $_REQUEST['action'];
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

print "<div style='border:1px solid black;padding:20px;margin:10px;background:#FC0'>";
print "<h2>FP Snippet Editor</h2>";
print "</div>";

print "<div style='border:1px solid black;padding:20px;margin:10px;background:#CCC'>";
//print "Action: $action<BR>";
//print "Filename: $filename<BR>";


print '<form action="snippeteditor.php" enctype="multipart/form-data" method="post">';

$list = array ();
switch ($action) {
	case 'edit'	:
		print "<div style='border:1px solid black;padding:20px;margin:10px;background:#EEA'>";
		print "<input name=\"theme\" type=\"hidden\" value=\"$theme\">";
		print "<input name=\"f\" type=\"hidden\" value=\"$filename\">";
		editFile ($filename, $theme);
		print "</div>";
		break;
	case 'update' :
		print "<div style='border:1px solid black;padding:20px;margin:10px;background:#EEA'>";
		$result = updateFile ($filename, $theme, $text);
		if ($result) 
			print "Successful update of $filename<BR>";
		displayList ($theme);
		print "</div>";
		break;
	default :
		displayList ($theme);
}

print '</form>';
print '</div><BR>';

// =============

function updateFile ($filename, $theme, $text) {
	$dir = "_themes/$theme/_snippets";
	file_exists ("$dir/_backup") || mkdir ("$dir/_backup", 0777);
	if (copy ("$dir/$filename", "$dir/_backup/$filename.bak")) {
		if (WriteTextFile ("$dir/$filename", $text)) {
			$result = "Updated $filename";
		} else {
			$result = false;
		}
	} else {
		$result = false;
	}
	return $result;
}

function displayList ($theme) {
	$themelist = fetchThemeList ();
	print "<div style='border:1px solid black;padding:20px;margin:10px;background:#EEA'>";
	print "<h3>List of Themes:</h3>";
	print "<ol>\n";
	foreach ($themelist as $t) {
		print "<li><a href=\"./snippeteditor.php?action=list&theme=$t\">$t</a></li>\n";
		//print "<li>" . $t . "</li>\n";
	}
	print "</ol>\n";
print "</div>";
print "<div style='border:1px solid black;padding:20px;margin:10px;background:#EEA'>";
	print "<h3>Snippets in <em>$theme</em></h3>";
	$dir = "_themes/$theme/_snippets";
	$filelist = fetchFileList ($dir);
	//print 'Theme Name: <input name="theme" type="text" value="' . $theme . '" size="30">&nbsp;&nbsp;<button type="submit" value="list" name="action">Show List</button>';
	$k = 1;
	print "<ol>\n";
	foreach ($filelist as $f) {
		$fdisplay = basename ($f);
		print "<li><a href=\"./snippeteditor.php?f=$f&action=edit&theme=$theme\">$fdisplay</a></li>\n";
		$k++;
	}
	print "</ol>\n";
	print "</div>";
}

function editFile ($filename, $theme) {
	$dir = "_themes/$theme/_snippets";
	$fname = basename ($filename);
	print "<h2>Edit <em>$fname</em></h2>";
	$text = ReadTextFile ("$dir/$filename");
	print '
	<textarea name="text" rows="20" cols="120">' . $text . '</textarea><BR>';
	print '<button type="submit" name="action" value="update">Update</button> <button type="submit" name="action" value="list">Cancel</button><BR>';
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
	foreach (glob("$dir/*.txt") as $filename) {
		$filelist[] .= $filename;
	}
	return $filelist;
}

function fetchThemeList () {
	$filelist = array ();
	foreach (glob("./_themes/*") as $theme) {
		if (is_dir ($theme))
			$themes[] .= basename ($theme);
	}
	return $themes;
}

