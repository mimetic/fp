<?php

/*

Admin users: age for showing information such as FAQ or About this Website.

*/
	
// =========== BEGIN CACHING CODE ================
// Include the package
require_once('Cache/Lite.php');

// Get the id for this cache
preg_match("|^/\w+/(.+?)/|",__FILE__, $m);
$username = $m[1];
$cachegroup = "{$username}_info";
$cacheid = preg_replace ("/clearcache=\w+/i", "", $_SERVER['REQUEST_URI']);
$cacheid = $cachegroup . preg_replace ("/\W/","",$cacheid);

// Force a clear of cache, e.g .after admin change?
isset($_REQUEST['clearcache'])
? $clearcache = $_REQUEST['clearcache']
: $clearcache = null;

// Set a few options
$options = array(
	'cacheDir' => 'tmp/cache/',
	'automaticCleaningFactor' => 1,
	'lifeTime' => 86400	// one day
);

// Create a Cache_Lite object
$Cache_Lite = new Cache_Lite($options);
//$Cache_Lite->setToDebug();

$clearcache && $Cache_Lite->remove($cacheid, $cachegroup);

// Test if there is a valid cache for this id

if (!($output = $Cache_Lite->get($cacheid, $cachegroup))) {
	
	// =========== NO CACHE, BUILD THE PAGE ================
	$error = "";
	$msg = "";

	include "_config/sysconfig.inc";
	include "_config/fpconfig.inc";
	include "_config/config.inc";
	include "includes/functions.inc";
	include "includes/project_management.inc";
	include "includes/image_management.inc";
	include "includes/commerce.inc";
	
	// report all errors except 'Notice', which is for undefined vars, etc. 
	// All the isset clauses are to avoid 'notice' errors. 
	error_reporting(E_ALL ^ E_NOTICE);
	
	$LINK = StartDatabase(MYSQLDB);
	Setup ();
	
	$ADMINFILENAME = basename (__FILE__);
	
	$f = $_POST;
	
	// Get form input
	$vars = GetFormInput();
	$actions = $vars['actions'];
	$vars = $vars['vars'];
	
	// Start Session
	session_name("fp_admin");
	session_start();
	
	isset($_SESSION['theme']) || $_SESSION['theme'] = ADMIN_THEME;
	
	$myTheme = CurrentThemeID ();
	
	$info = trim($vars["subject"]);
	$info || $info = "terms";

	// What to show?
	$body = FetchSnippet ($vars['topic']);

	$list = FetchSnippet ("info_wrapper_open") . FetchSnippet ("info_".$info) . FetchSnippet ("info_wrapper_close");
	
	$output = FetchSnippet ('main_info');
	$title = FP_SYSTEM_DISPLAY_NAME . " {fp:InfoPageTitle}";
	
	$output = Substitutions ($output, array(
		'META_INDEX'			=> FetchSnippet ('meta_robots_noindex'),
		'body'				=> $body, 
		'NAVBAR'			=> $navbar,
		'NAVBAR_2'			=> $navbar2,
		'header'			=> $header,
		'subtitle'			=> FP_SYSTEM_DISPLAY_NAME,
		'title'				=> $title,
		'pagetitle' 			=> $title,
		'message'			=> $msg,
		'error'				=> $error,
		'master_page_popups'		=> FetchSnippet("client_access_dialog")
		));
	$output = ReplaceAllSnippets ($output);
	$output = ReplaceSysVars ($output);
	$output = DeleteUnusedSnippets ($output);

	mysqli_close($LINK);
	//$FP_MYSQL_LINK->close();
	
	$output = compress_html($output);
	$DEVELOPING || $Cache_Lite->save($output, $cacheid, $cachegroup);
}
print $output;

?>