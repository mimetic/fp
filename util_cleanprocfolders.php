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

// --------------------------------
/*
Clean out:
$PROCESSEDDIR
$PROCESSED_PHOTOS
$PROCESSED_SLIDES
$PROCESSED_THUMBNAILS
$PROCESSED_MATTED
$PROCESSED_FRAMED
$PROCESSED_ORIGINALS
*/

CleanProcessedDirs();

$output = "Deleted all .jpg files in the processed folders<BR>";

print $output;


?>