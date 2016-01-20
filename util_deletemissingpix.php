<?php
$DEBUG = false;

include "_config/sysconfig.inc";
include "_config/fpconfig.inc";
include "_config/config.inc";
include "includes/functions.inc";
include "includes/project_management.inc";
include "includes/image_management.inc";
include "includes/commerce.inc";

$error = "";
$msg = "";

session_start();
isset($_REQUEST['GroupID']) && $_SESSION['GroupID'] = $_REQUEST['GroupID'];
isset($_SESSION['GroupID']) || $_SESSION['GroupID'] = PUBLIC_GROUP_ID;

isset($_REQUEST['theme']) && $_SESSION['theme'] = $_REQUEST['theme'];
isset($_SESSION['theme']) || $_SESSION['theme'] = DEFAULT_THEME;

$groupID = $_SESSION['GroupID'];

$LINK = StartDatabase(MYSQLDB);
Setup ();

// --------------------------------

$query = "SELECT * FROM ".DB_IMAGES." ORDER BY ID";
$result = mysqli_query ($query);

$bad = array();
$good = array();
$output = "<h2>FP: Delete Missing Pictures</h2>";

while ($record = mysqli_fetch_array($result, MYSQLI_ASSOC)) {

	$ID = $record['ID'];
	
	$text = "";
	
	$text .= "&rarr; Picture #{$ID} : Title={$record['Title']} : Filename={$record['URL']})";
	$url = $record['URL'];
	$missing =0;
	
	file_exists("$BASEDIR/$PHOTOS_GALLERY/$url") || $missing++;
	$missing && $text .= "MISSING $BASEDIR/$PHOTOS_GALLERY$url <BR>" ;
 
	file_exists("$BASEDIR/$SLIDES/$url") || $missing++;
	$missing && $text .= "MISSING $BASEDIR/$SLIDES$url <BR>" ;
 
	file_exists("$BASEDIR/$THUMBNAILS/$url") || $missing++;
	$missing && $text .= "MISSING $BASEDIR/$THUMBNAILS$url <BR>" ;
 
// 	file_exists("$BASEDIR/$MATTED/$url") || $missing++;
// 	$missing && $text .= "MISSING $BASEDIR/$MATTED$url <BR>" ;
//  
// 	file_exists("$BASEDIR/$FRAMED/$url") || $missing++;
// 	$missing && $text .= "MISSING $BASEDIR/$FRAMED$url <BR>" ;
 
	file_exists("$BASEDIR/$ORIGINALS/$url") || $missing++;
	$missing && $text .= "MISSING $BASEDIR/$ORIGINALS$url <BR>" ;
	
	if ($missing) {
		DeleteRowByID( DB_IMAGES, $ID);
		// parts?
		$a = array ();
		
		$query = "SELECT * FROM ".DB_PARTS." WHERE ( PartTable = \"".DB_IMAGES."\" AND PartID = \"$ID\" ) ORDER BY ID";
		$parts = mysqli_query ($query);
		while ($part = mysqli_fetch_array($parts, MYSQLI_ASSOC)) {
			$partinfo = "Part fields:";
			$partinfo .= join(", ", $part);
			$a[] = $part['ID'] . ": ".$partinfo . "<BR>";
			DeleteRowByID( DB_PARTS, $part['ID'] );
		}
		$row = "<i>Parts referring to this image</i> : ";
		$bad[] = "$row<font color=\"#CC0000\">" . join (",", $a) . '</font><br>';
	
	} else {
		$good[] = "<font color=\"#999999\">{$text}All files accounted for.</font><br>";
	}
	
}

$output .= "<h3>Missing Image Records</h3>";
if ($bad) {
	sort($bad);
	reset ($bad);
	foreach ($bad as $row) {
		$output .= $row;
	}
}

$output .= "<h3>Good Image Records</h3>";
if ($good) {
	sort($good);
	reset ($good);
	foreach ($good as $row) {
		$output .= $row;
	}
}
$output .= "<h3>End</h3>";

print $output;
print "<BR>";
$output = "";



// --------------------------------

$output = FetchSnippet ('master_page');

$output = Substitutions ($f, $output);

$output = ReplaceAllSnippets ($output);
$output = ReplaceSysVars ($output);
$output = DeleteUnusedSnippets ($output);

print $output;

mysqli_close($LINK);
//$FP_MYSQL_LINK->close();

?>