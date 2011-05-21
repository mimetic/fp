<?php
$DEBUG = false;

include "_config/sysconfig.inc";
include "_config/config.inc";
include "includes/functions.inc";
include "includes/project_management.inc";
include "includes/image_management.inc";
include "includes/commerce.inc";

$error = "";
$msg = "";


$LINK = StartDatabase(MYSQLDB);
Setup ();

// --------------------------------


$picpath = $_REQUEST['path'];
$picpath || $picpath = $ORIGINALS;

	$msg .= "<h1>IPTC INFO FOR FILES IN $BASEDIR/$picpath/</h1>";
	$msg .= "add ?path=x to look for pics in directory 'x'<BR>";

	$files = array();

	$handle=opendir("$BASEDIR/$picpath/");

	$msg .= "Looking in " . "$BASEDIR/$picpath/<BR>\n";
	while ($file = readdir($handle)) {
		if ((substr($file,1,1) != ".") AND (preg_match("/\.jpg$/i", $file))) { 
			$files[] = $file;
			$DEBUG &&  $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": Found $file<BR>\n";
		}
	}
	closedir($handle);

	foreach ($files as $filename) {
		$msg .= "<h2>$filename</h2>";
		$path = "$BASEDIR/$picpath";
		$sourceFile = "$path/$filename";

		$fileinfo = FetchIPTCInfo ($sourceFile);
		
		reset ($fileinfo);
		
		$fi = array ();
		while (list ($k,$v) = each ($fileinfo)) {
			$v && $fi[$k] = $v;
		}
		
		
			$artistinfo = FindArtistByName ($fileinfo['Author']);
			if (!$artistinfo)
				$artistinfo = FindArtistByName ($fileinfo['Byline']);
			if (!$artistinfo)
				$artistinfo = FindArtistByName ($fileinfo['Credit']);
			if (!$artistinfo)
				$artistinfo = FindArtistByName ($fileinfo['Source']);
				
		$artistfullname = $artistinfo['Firstname'] . " " . $artistinfo['Lastname'];

		$msg .= "File Credit: " . $fileinfo['Credit'] . "<BR>\n";
		$msg .= "File Byline: " . $fileinfo['Byline'] . "<BR>\n";
		$msg .= "File Source: " . $fileinfo['Source'] . "<BR>\n";
		$msg .= "File Author: " . $fileinfo['Author'] . "<BR>\n";
		$msg .= "File Caption: " . $fileinfo['Caption'] . "<BR>\n";
		$msg .= "Artist's name: " . $artistinfo['Firstname'] . " " . $artistinfo['Lastname'] . "<BR>\n";
		
		$info = "";
		$info .= "Artist: " . $artistinfo['Firstname'] . " " . $artistinfo['Lastname'] . " | ";
		$info .= "Headline: " . $fileinfo['Headline'] ;

		
		
		$msg .= "IPTC INFO for $sourceFile" . ArrayToTable ($fi);
		print $msg;
	}

// --------------------------------


mysql_close($LINK);
$FP_MYSQL_LINK->close();

?>