<?php

$error = "";
$msg = "";

require_once "_config/sysconfig.inc";
require_once "_config/fpconfig.inc";
require_once "_config/config.inc";
require_once "includes/functions.inc";
require_once "includes/project_management.inc";
require_once "includes/image_management.inc";
require_once "includes/commerce.inc";


// report all errors except 'Notice', which is for undefined vars, etc. 
// All the isset clauses are to avoid 'notice' errors. 
error_reporting(E_ALL ^ E_NOTICE);

$LINK = StartDatabase(MYSQLDB);
Setup ();

$ADMINFILENAME = basename (__FILE__);

$record = CleanRequest();

isset ($record['command']) && $command = $record['command'];
isset ($record['desc']) && $desc = $record['desc'];
isset ($record['nextaction']) && $nextaction = $record['nextaction'];
isset ($record['ID']) && $ID = $record['ID'];
isset ($record['email']) && $email = $record['email'];

$artist = FetchArtistByUsername ($Username);



$form = FetchSnippet ("lostinfo");	
$text = FetchSnippet ("mainB");


// SUBSTITUTIONS SET 1
$text = Substitutions ($text,	array (	"form"		=>	$form));

// SUBSTITUTIONS SET 2
$text = Substitutions ($text,	array (	"noaccess"	=>	$noaccess,
										"error"		=>	$error,
										"msg"		=>	$msg,
										"email"		=>	$email,
										"picturemgmt"	=> $picturemgmt,
										"addnewrecord"	=> $addnewrecord,
										"Menu"		=>	$menu
										));
										
// SUBSTITUTIONS SET 3
$sysvars = array("table"	=>	$table,
				"PHOTOS"	=>	$PHOTOS_GALLERY,
				"SYSTEMNAME"	=>	$SYSTEMNAME,
				"basename"	=>	basename($PHP_SELF),
				"javascript"	=>	$js);
$text = Substitutions ($text, $sysvars);
$text = ReplaceSysVars ($text, $table, $userID, $ID, $nextaction);

// at this point, dump any {} codes, we're ready to print it.
$text = preg_replace("/(\{.*?\})/","", $text);

print $text;


?>
