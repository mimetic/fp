<?php
/*	
Export a project so that it can be moved to another server.

- get project
- package all the MYSQL records from projects, parts, images, stories, files into a directory
- package all the images and files into a directory
- Need some clue as to how to rebuild it. Import the SQL? That's probably the way.
- Or, we could have a series of instructions for rebuilding, which would allow movement to a differently built system!
e.g. commands: new project; add image (filename, {image info}); set theme, stuff like that.

 */


$start = microtime (true);

$error = "";
$msg = "";

include "_config/sysconfig.inc";
include "_config/fpconfig.inc";
include "_config/config.inc";
include "includes/functions.inc";
include "includes/project_management.inc";
include "includes/image_management.inc";
include "includes/commerce.inc";

//error_reporting  ( E_ERROR | E_WARNING | E_PARSE ); 

$DEBUG = 1;
$DEBUG && print "DEBUG IS ON<BR>\n";
isset ($_REQUEST['debug']) && $DEBUG = true;


$LINK = StartDatabase(MYSQLDB);


$projectID = $_REQUEST['projectid'];

$project = new FPProject ($LINK, $projectID);
if (!$project->ID) {
	$error = "Project {$projectID} does not exist.";
}

if (!$error) {
	$project->ExportProject();
}


mysqli_close($LINK);
//$FP_MYSQL_LINK->close();

?>