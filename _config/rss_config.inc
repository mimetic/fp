<?php

// CONFIG FILE FOR FP PICTURE GALLERY SYSTEM
// RSS Settings

//=============  RSS
// Return all images in a project, or just one
// RSS readers will group by project unless we set the link on each picture to something unique
define ("FP_RSS_ALL_IN_PROJ", false);

// Add an HTML img in the description field. I think this is non-standard, but most RSS readers can probably handle it.
$FP_RSS_IMG_IN_DESC = true;
define ("FP_RSS_IMG_IN_DESC", $FP_RSS_IMG_IN_DESC);

// Maximum sizes for pictures embedded in the description
define ("FP_RSS_IMG_MAX_H", 200);
define ("FP_RSS_IMG_MAX_W", 200);

?>