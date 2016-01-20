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

$LINK = StartDatabase(MYSQLDB);
Setup ();

// --------------------------------

$query = "SELECT * FROM ".DB_PARTS." ORDER BY PartTable, PartID";
$result = mysqli_query ($query);

while ($record = mysqli_fetch_array($result, MYSQLI_ASSOC)) {

	$ID = $record['ID'];

	$table = $record['PartTable'];
	$partID = $record['PartID'];
	
	
	switch ($table) {
		case DB_GROUPS :	
		
			// Artists in groups
			if ($record['ArtistID'] > 0) {
				$table = DB_ARTISTS;
				$partID = $record['ArtistID'];
			}
			$r2 = mysqli_query ($q);
			if (mysqli_num_rows($r2) > 0) {
				//print "---> Part ID ($partID) exists in table ($table)<BR>";
				print ".";
			} else {
				print "<BR>Part ID# $ID : ". $record['PartTable'] ;
				print " *** Part ID ($partID) does not exist in table ($table) ***<BR>";
				
				DeleteRowByID( DB_PARTS, $ID );
				
			}
			break;
		case DB_ARTISTS :
			// Artists in projects
			if ($record['ArtistID'] > 0) {
				$table = DB_ARTISTS;
				$partID = $record['ArtistID'];
			}
			$q = "SELECT ID FROM ".DB_PARTS." where ArtistID IN (SELECT ID FROM ".DB_ARTISTS." where ID = {$record['ArtistID']}) AND ProjectID IN (SELECT ID FROM ".DB_PROJECTS." where ID = {$record['ProjectID']}) limit 1";

			$r2 = mysqli_query ($q);
			if (mysqli_num_rows($r2) > 0) {
				//print "---> Part ID ($partID): artist ID={$record['PartID']} exists, and project ID={$record['ProjectID']} exists<BR>";
				print ".";
			} else {
				print "<BR>###> Part ID ($partID): Either artist ID={$record['PartID']} does not exist, or project ID={$record['ProjectID']} does not exist.<BR>";
				
				DeleteRowByID( DB_PARTS, $ID );
				
			}
			break;
		default :
			
			// Does the part exist?
			$q = "SELECT * FROM $table where ID = $partID";
			$r2 = mysqli_query ($q);
			if (mysqli_num_rows($r2) > 0) {
				//print "---> Part ID ($partID) exists in table ($table)<BR>";
				print ".";
			} else {
				print "<BR>Part ID# $ID : ". $record['PartTable'] ;
				print " *** Part ID ($partID) does not exist in table ($table) ***<BR>";
				
				DeleteRowByID( DB_PARTS, $ID );
				
			}
	}
}



// --------------------------------


mysqli_close($LINK);
//$FP_MYSQL_LINK->close();

?>