<?php
// =========== BEGIN CACHING CODE ================
// Include the package
require_once('Cache/Lite.php');

// Get the id for this cache
preg_match("|^/\w+/(.+?)/|",__FILE__, $m);
$username = $m[1];
$cachegroup = "story{$username}";
$cacheid = "story".$_REQUEST['storyID'];

// Force a clear of cache, e.g .after admin change?
isset($_REQUEST['clearcache']) && isset($_REQUEST['clearcache'])
? $clearcache = $_REQUEST['clearcache']
: $clearcache = null;

// Set a few options
// Cache is set to 1 hour: If pix arrive by email, or other non-admin method
// then we won't know the gallery needs updating.
$options = array(
    'cacheDir' => 'tmp/cache/',
    'automaticCleaningFactor' => 1,
    'lifeTime' => 3600	// one hour
);

// Create a Cache_Lite object
$Cache_Lite = new Cache_Lite($options);
//$Cache_Lite->setToDebug();

$clearcache && $Cache_Lite->remove($cacheid, $cachegroup);

// Test if there is a valid cache for this id

if (!($output = $Cache_Lite->get($cacheid, $cachegroup))) {

    include "_config/sysconfig.inc";
    include "_config/fpconfig.inc";
    include "_config/config.inc";
    include "includes/functions.inc";
    include "includes/project_management.inc";
    include "includes/image_management.inc";
    include "includes/commerce.inc";

    $DEBUG = false;

    $LINK = StartDatabase(MYSQLDB);
    Setup ();

    $vars = GetFormInput();
    $vars = $vars['vars'];
    $error = "";
    $msg = "";

    session_name("fp_gallery_session");
    session_start();

    isset($vars['GroupID']) && $_SESSION['GroupID'] = $vars['GroupID'];
    isset($_SESSION['GroupID']) || $_SESSION['GroupID'] = PUBLIC_GROUP_ID;

    // isset($vars['theme']) && $_SESSION['theme'] = $vars['theme'];
    isset($_SESSION['theme']) || $_SESSION['theme'] = DEFAULT_THEME;

    $projectID = $vars['ProjectID'];

    $groupID = $_SESSION['GroupID'];
    $myGroup = new FPGroup ($groupID);

    $GroupBannerURL = $myGroup->LogoFilename();
    $GroupBanner = $myGroup->LogoHTML("style='border:1px solid black;margin-right:10px;'");
    $GroupIcon = $myGroup->IconHTML();

    $myTheme = CurrentThemeID ();

    $params = FetchParams ($PROJECTS, $projectID);
    if ($params[FP_PARAM_GALLERY_THEME]) {
	$_SESSION['theme'] = $FP_Themes[$params[FP_PARAM_GALLERY_THEME]]['id'];
    } else {
	$_SESSION['theme'] = CurrentThemeID ();
    }

    $storyID = $vars['storyID'];

    $story = FetchStory ($storyID);
    $artist = FetchArtist ($story['ArtistID']);
    $project = FetchProject ($projectID);
    $group = FetchGroup ($groupID);


    $story['Story'] = FormatText (trim($story['Story'])."\n") ;

    $p1 = substr ($story['Story'], 0, strpos ($story['Story'], "\r"));
    $p2 = substr ($story['Story'], strpos ($story['Story'], "\r"));
    $story['Story'] = $p1 . "{sampleimage}" . $p2;

    $text = FetchSnippet ("story_item");
    $text = Substitutions ($text, $story);
    $text = Substitutions ($text, $artist);
		$sampleImage = FetchRandomImage ("", 'sample_picture',null,$projectID,$groupID);
		$randomImageList = FetchRandomImageList($groupID, $projectID);

    $page = FetchSnippet ("master_page_single_story");
    $page = Substitutions ($page, 	array("text"	=>	$text,
	'sectionclass'		=> 	"story",
	"sampleimage"		=>		$sampleImage,
	"projecttitle"		=>		$project['Title']
    ));

    $page = ReplaceAllSnippets ($page);
    $page = ReplaceSysVars ($page);
    $page = DeleteUnusedSnippets ($page);

    //print $page;

    mysql_close($LINK);
$FP_MYSQL_LINK->close();

    $output = compress_html($page);
    $DEVELOPING || $Cache_Lite->save($output, $cacheid, $cachegroup);
}
print $output;

?>