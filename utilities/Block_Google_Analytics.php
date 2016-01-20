<?php

	
include "../_config/sysconfig.inc";
include "../_config/config.inc";
include "../includes/functions.inc";
include "../includes/project_management.inc";
include "../includes/image_management.inc";
include "../includes/commerce.inc";

$LINK = StartDatabase(MYSQLDB);
Setup ();

$js = '<script src="{JAVASCRIPT}" type="text/javascript"></script>';


$page = "<html><header>$js</header>";
$page = "<body onLoad=\"javascript pageTracker._setVar('test_value');\">Block Google Analytics from Reading this Computer.</body></html>";
$page = ReplaceSysVars ($output);
print $page;

mysqli_close($LINK);
//$FP_MYSQL_LINK->close();
?>