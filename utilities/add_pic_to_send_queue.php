<?php

	
include "_config/sysconfig.inc";
include "_config/config.inc";
include "includes/functions.inc";
include "includes/project_management.inc";
include "includes/image_management.inc";
include "includes/commerce.inc";

$DEBUG = 0;

$error = "";
$msg = "";

session_start();
isset($_REQUEST['GroupID']) && $_SESSION['GroupID'] = $_REQUEST['GroupID'];
isset($_SESSION['GroupID']) || $_SESSION['GroupID'] = PUBLIC_GROUP_ID;

isset($_REQUEST['theme']) && $_SESSION['theme'] = $_REQUEST['theme'];
isset($_SESSION['theme']) || $_SESSION['theme'] = DEFAULT_THEME;

$groupID = $_SESSION['GroupID'];

$vars = GetFormInput();
$vars = $vars['vars'];

$LINK = StartDatabase(MYSQLDB);
Setup ();

if ($_REQUEST['imageID']) {
	$imageID = $_REQUEST['imageID'];
	$supplierID = 1;
	
	//$r = FTPImageToSupplier ($ID, $supplierID) ;
	
	WriteSendOrder ($imageID, $supplierID);
	CopyImageToSendQueue ($imageID);
}

$page = '<form action="test.php" method="get">
ImageID: <input name="imageID" id="imageID" type="text" size="5" maxlength="5"><BR>
e.g. 445,446,447 <BR>
<button type="submit">Submit</button>
</form>';

print $page;

$f = file_get_contents (AS_SENDER_QUEUE_DIR."/".AS_SENDER_QUEUEFILE);
print (str_replace ("\n", "<br>\n", $f));

mysqli_close($LINK);
//$FP_MYSQL_LINK->close();


	
?>