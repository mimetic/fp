<?php

/* 	copydb will copy fields from one database to another, ignoring non-matching fields.
	It is useful for updating existing databases, by moving the data from an old database 
	to a new one with a slightly different structure.
*/


include "_config/sysconfig.inc";
include "_config/config.inc";
include "includes/functions.inc";
include "includes/project_management.inc";
include "includes/image_management.inc";
include "includes/commerce.inc";


define ("MYSQLHOST1", "localhost");
define ("MYSQLDB1", "fp_transfer");
define ("MYSQLUSER1", "frontlin_dgross");
define ("MYSQLPASSWORD1", "nookie");


define ("MYSQLHOST2", "localhost");
define ("MYSQLDB2", "mimetic_fp");
define ("MYSQLUSER2", "frontlin_dgross");
define ("MYSQLPASSWORD2", "nookie");


$LINK = StartDatabase(MYSQLDB1);


$sql = "SHOW TABLES FROM " . MYSQLDB1;
$result = mysqli_query ($sql);
while ($row = mysqli_fetch_row($result)) {
	$tables[] = $row[0];
}

$db = array ();

foreach ($tables as $table) {
	print "<h1>Table $table</h1>";
	
	$sql = "select * from $table";
	$result = mysqli_query ($sql);
	$x=1;
	while ($row = mysqli_fetch_assoc($result)) {
		$db[$table][] = $row;
		//print $table.": ". $x++ . "<BR>";
	}
}

mysqli_close($LINK);
//$FP_MYSQL_LINK->close();


$LINK2 = StartDatabase(MYSQLDB2);

// clear tables
$sql = "SHOW TABLES FROM " . MYSQLDB1;
$result = mysqli_query ($sql);
while ($row = mysqli_fetch_row($result)) {
	mysqli_query ("delete from $table");
}


while (list ($table, $rows) = each ($db)) {
	print "<h2>Table: $table</h2>";
	foreach ($rows as $row) {
		$row = StripNonExistantFields ($table, $row);
		print "Fields: ".join (", ", array_values($row)) . "<BR>";
		AddRecord( $table, $row );
	}
}

mysqli_close($LINK2);
?>