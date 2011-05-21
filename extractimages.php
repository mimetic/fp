<?php
/*
 * Find any mail messages and extract the JPG files from them. Put the JPGs into
 * the input folders, so they'll be processed and sucked into the system.
 */

include "_config/sysconfig.inc";
include "_config/fpconfig.inc";
//include "_config/config.inc";
include "includes/functions.inc";
//include "includes/project_management.inc";
//include "includes/image_management.inc";

$LINK = StartDatabase(MYSQLDB);

$message = new FPMail ();
$message->GetPictureFromEmailFile ();
$DEVELOPING && var_dump ($message);
//var_dump ($message);

mysql_close($LINK);
$FP_MYSQL_LINK->close();

?>