<?PHP

// DELETE DUPLICATE IMAGES
// This script deletes duplicate entries in the Images database
// That is, if two entries use the same URL, it deletes one of them.
// It might kill the one with the caption you like...so beware!

/*NOTE TO MYSELF: 
$first ? $second : $third
*/

include "_config/sysconfig.inc";
include "_config/config.inc";
include "includes/functions.inc";
include "includes/project_management.inc";
include "includes/image_management.inc";
include "includes/commerce.inc";

error_reporting  (E_ERROR | E_WARNING | E_PARSE); 

$DEBUG = true;

$LINK = StartDatabase(MYSQLDB);
Setup ();
//SELECT BookISBN, count(BookISBN) FROM Books GROUP BY BookISBN HAVING COUNT(BookISBN)>1;
$query = "select ID, Title, URL, COUNT(URL) AS kURL from Images GROUP BY URL HAVING (COUNT(URL) > 1 );";
print "$query<BR>";
$result = mysqli_query ($LINK, $query);
$k =1;
while ($line = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
	//print ArrayToTable ( $line );
	print "$k) " ;
	foreach ($line as $key => $value) {
		print "$key = $value<BR>";
	}
	$where = "ID = " . $line['ID'];
	DeleteRow( $IMAGES, $where );
	$where = "PartTable = 'Images' and PartID = " . $line['ID'];
	DeleteRow( $PARTS, $where );
	print "Deleting	image " . $line['ID'] . "<BR>";
	print "<hr>";
	
}

mysqli_close($LINK);
//$FP_MYSQL_LINK->close();


?>