<?php
$DEBUG = false;

require_once "_config/sysconfig.inc";
require_once "_config/fpconfig.inc";
require_once "_config/config.inc";
require_once "includes/functions.inc";
require_once "includes/project_management.inc";
require_once "includes/image_management.inc";
require_once "includes/commerce.inc";

$error = "";
$msg = "";

session_name("fp_admin");
session_start();
$_SESSION['theme'] = ADMIN_THEME;
$_SESSION['GroupID'] = FP_ADMINISTRATOR_GROUP;

$LINK = StartDatabase(MYSQLDB);
Setup ();

// --------------------------------


$pix = array ();
$f = array ();
$list = "<h3>Delete Unused Photos</h3>";
$list .= "<div class='helptext'>This utility looks for photos in the PHOTOS folder which are not referenced by an image record and deletes them.</div><BR><BR>";
$list .= '<form>
<button type="submit">Refresh List</button>
<button type="submit" name="delete" value="1">Delete Extra Files</button>
</form>';

$query = "SELECT * FROM ".DB_IMAGES." ORDER BY ID";
$result = mysql_query($query);

while ($record = mysql_fetch_array($result, MYSQL_ASSOC)) {
	$ID = $record['ID'];
	$pix[$record['URL']] = $record['URL'];
}

// Get list of all images
$images = array ();
foreach (glob (FP_DIR_ORIGINAL_IMAGES . "/*") as $filename) {

	$info = pathinfo($filename);
	if (strtolower($info['extension']) == "jpg") {
		$p = substr(basename($filename), 0,6);
		if ($p != "artist" && $p != "group" && $p != "missin" ) {
			$images[] = basename($filename);
		}
	}
}

// compare lists
$bad =  array();
$good =  array();
foreach ($images as $image) {
	if (!isset($pix[$image])) {
		$bad[] = "Image $image is not in use<BR>";
		if ($_REQUEST['delete'])
			DeleteAllPicVersions ($image);
	} else {
		$good[] = "<B>*Image $image is in use</b><BR>";
	}
}

$list .= "<h3>Missing Images</h3>";
if ($bad) {
	sort($bad);
	reset ($bad);
	foreach ($bad as $row) {
		$list .= $row;
	}
}

$list .= "<h3>Found Images</h3>";
if ($good) {
	sort($good);
	reset ($good);
	foreach ($good as $row) {
		$list .= $row;
	}
}


$f['form'] = $list;
$f['pixwaiting'] = "";
// --------------------------------

$text = Substitutions (FetchSnippet ('main'), $f);
$text = ReplaceAllSnippets ($text);
$text = ReplaceSysVars ($text, $table, $fp_user, $ID, $nextaction);
$text = DeleteUnusedSnippets ($text);
print $text;

mysql_close($LINK);
$FP_MYSQL_LINK->close();

?>