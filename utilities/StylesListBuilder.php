<?php 

$BASEDIR = dirname($_SERVER['SCRIPT_FILENAME']);
$SCRIPTNAME = basename ($_SERVER['SCRIPT_FILENAME']);
$page = "";
$page .= "<h1>Edit Array File</h1>This script allows you to edit a serialized array stored in a file. This is used to create the file, $fn, which is read by the Theme Editor in the FP Admin system. <br><br>
How to use: <ul><li>Create a tab-separated spreadsheet</li><li> save it as '$raw' in the same directory as this script</li><li> open this script in the browser</li><li> Click Import to bring in the new values</li><li> Tweak valus as need, then Save</li><li> Then, Export to create the file, '$fn'</li><li> Move $fn to the admin:_snippets directory</li></ul>Reload reads the serialized file in again. Use it if you enter some wrong values and want to start over, but no to reimport the raw file.<BR>";

$fn = "input_form_theme_editor_styles.txt";
$raw = "styles-raw.txt";

$page .= "The script imports text-separated lines from $raw,<br>
exports tab-separated lines to $raw,<br>
and creates a serialized array in the file, $fn<BR>";


if ($_REQUEST['Save']) {
	$page .= "<h2>Save</h2>";
	$rows = $_REQUEST['rows'];
	$cols = $_REQUEST['cols'];
	
	for ($r=0;$r<$rows;$r++) {
		for ($i=0;$i<$cols;$i++) {
			$arr[$r][$i] = $_REQUEST["$r-$i"];
			//print "Reading $r-$i<BR>";
		}
	}
	// Save Array

	file_put_contents ($fn, serialize ($arr));
	
} else if ($_REQUEST['Reload']) {
	$page .= "<h2>Reload</h2>";
	$arr = unserialize(file_get_contents ($fn));
	
} else if ($_REQUEST['Export']) {
	$page .= "<h2>Export</h2>";
	$rows = $_REQUEST['rows'];
	$cols = $_REQUEST['cols'];
	
	$output = "";
	for ($r=0;$r<$rows;$r++) {
		$row = "";
		for ($i=0;$i<$cols;$i++) {
			$arr[$r][$i] = $_REQUEST["$r-$i"];
			$row .= $_REQUEST["$r-$i"]."\t";
		}
		$output .= trim ($row) . "\r"; 
	}
	file_put_contents ("$BASEDIR/$raw", $output);
	
} else if ($_REQUEST['Import']) {
	$source = file_get_contents ($raw);
	$source = explode("\r", $source);

	$r = 0;
	$arr = array ();
	foreach ($source as $line) {
		$line = trim($line);
		if ($line) {
			$row = explode("\t", $line);
			for ($i=0;$i<count($row);$i++) {
				$arr[$r][$i]= trim ($row[$i]);
				//print $arr[$r][$i] . "<BR>";
			}
			$r++;
		}
	}
}




$r = 0;
$page .= "<form action='$SCRIPTNAME' method=\"post\">";
$page .= "<table style='padding:0px;border:1px solid #888;'>";
foreach ($arr as $row) {
	$page .= "<tr><th>$r:</th>";
	$i = 0;
	foreach ($row as $item) {
		$page .= "<td><input name=\"". $r . "-" . $i ."\" value=\"". htmlentities(stripslashes($item)) . "\" ></td>";
		$i++;
	}
	$page .= "</tr>";
	$r++;
}
$page .= "</table>";
$page .= "<button type=\"submit\" value=\"Save\" name=\"Save\">Save</button>";
$page .= "<button type=\"submit\" value=\"Reload\" name=\"Reload\">Reload</button>";
$page .= "<button type=\"submit\" value=\"Export\" name=\"Export\">Export</button>";
$page .= "<button type=\"submit\" value=\"Import\" name=\"Import\">Import</button>";
$page .= "<input name=\"rows\" type=\"hidden\" value=\"$r\">";
$page .= "<input name=\"cols\" type=\"hidden\" value=\"$i\">";
$page .= "</form>";
print $page;

?>