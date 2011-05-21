<?php

print "<h1>This utility backs up system themes (inside of _alt folders) and reformats them.</h1>";
print "If the theme backup exists, it is left alone. If not, a backup is created (e.g. mytheme.css.bak).<BR>";
print "If the URL contains the parameter 'restore' (e.g. myurl?restore=1) then we restore all sheets from the backups.<BR>";
?>
<form action="util_cleancss.php" method="get">
Theme: <input name="theme" type="text" value="default"><br>
<input name="restore" type="checkbox" value="1"> Restore original files?<br>
<button type="submit">Submit</button>
</form>

<?
print "<BR><HR><BR>";

$restore = isset($_REQUEST['restore']);

isset($_REQUEST['theme'])
	? $themedir = $_REQUEST['theme']
	: $themedir = "default";

if ($restore) {
	$themepath = "_themes/{$themedir}/_css/_alt/*.bak";
	$themedirs = glob ($themepath);
	foreach ( $themedirs as $themepath) {
		if (file_exists($themepath)) {
			$f = file_get_contents($themepath);
			$fname = str_replace (".bak", "", $themepath);
			file_put_contents ($fname, $f);
			print "Restored $fname from $themepath<BR>";
		} else {
			print "$themepath is missing...cannot restore from it<BR>";
		}
	}
} else {
	
	include("includes/cssparser.php");
	
	$themepath = "_themes/{$themedir}/_css/_alt/*.css";
	$themedirs = glob ($themepath);
	foreach ( $themedirs as $themepath) {
		//print "Fetching $themepath";
		$themes[$themepath] = file_get_contents($themepath);
		/*
		// rename and backup current iteration?
		$prev = "$themepath.previous";
		file_exists ($prev) && unlink ($prev);
		rename ($themepath, $prev);
		*/
		if (!file_exists("{$themepath}.bak")) {
			file_put_contents ("{$themepath}.bak", $themes[$themepath]);
			print "...Backed up $themepath.<BR>";
		}
	}
	
	
	
	$css = new cssparser();
	$css->html = false;
	
	foreach ($themes as $theme=>$sheet) {
		$css->Clear();
		$css->ParseStr($sheet);
		//print_r ($css->GetSection('#header'));
		//echo $css->GetCSS();
		file_put_contents ($theme, $css->GetCSS());
		print "Updated $theme<BR>";
	}
}

print "###ENDIT";


?>