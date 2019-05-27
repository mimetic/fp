<?php

error_reporting(E_ALL ^ E_NOTICE);

$BASEDIR = dirname($_SERVER['SCRIPT_FILENAME']);
$themedirs = glob ("../_themes/*", GLOB_ONLYDIR);
$headerJS = "";
$page = "";
$header = "";


if ($_REQUEST['cancel'] || !$_REQUEST['t']) {
	foreach ($themedirs as $td) {
		$header .= "<h3>".basename($td)."</h3>";
		$header .= '<form action="' . $_SERVER['PHP_SELF'] . '" method="post">';
		$header .= "<ul>";
		$header .= '<input name="new" type="hidden" value="1"><input name="t" type="hidden" value="'.$td.'/_css/_alt">';
		$header .= "<li>Edit CSS for <a href='" . $_SERVER['PHP_SELF'] . "?t=$td/_css/extra_style.css'>".basename($td)."</a></li>";
		
		$altdirs = glob ("$td/_css/_alt/*.css");
		foreach ($altdirs as $ad){
			$header .= "<li>Edit variation: <a href='" . $_SERVER['PHP_SELF'] . "?t=$ad'>".basename($ad)."</a> | ";
			$header .= "<i><a onclick=\"return confirm('Are you sure you want to delete?')\" href='" . $_SERVER['PHP_SELF'] . "?t=$ad&amp;delete=1'>Delete</a></i>";
			$header .= " | <a href='$ad' target='_blank'>Download ".basename($ad)."</a>";
			$header .= "</li>\n";
		}
		$header .= '<li>Add new variation named <input name="name" type="text" value="" size="30"></li>';
		$header .= "</ul>";
		$header .= "</form>";
		$header .= "<hr>";
	}
} else {
	$fn = $_REQUEST['t'];
	if ($_REQUEST['delete']) {
		unlink ($fn);
		$link = "https://" . $_SERVER['PHP_SELF'];
		$link && header("Location: $link");
	} else if ($_REQUEST['new'] && $_REQUEST['name']) {
		file_exists ("$BASEDIR/$fn") || mkdir ("$BASEDIR/$fn", 0755);
		$name = preg_replace ("/\W/","_",$_REQUEST['name']);
		copy ("$BASEDIR/ce/blank.css", "$fn/$name.css");
		$link = "https://" . $_SERVER['PHP_SELF'];
		$link && header("Location: $link");
	} else {
		$css  = file_get_contents ($fn);
		//var_dump ($_REQUEST);
		$header .= "<h3>You are editing : <i>$fn</i></h3>";

		if ($_REQUEST['save'] == "save") {
			$header .= "SAVING FILE TO $fn<BR>";
		}
		$header .= '<form action="' . $_SERVER['PHP_SELF'] . '" method="post">';

		$header .= "<button type=\"submit\" name=\"save\" value=\"save\">Save Changes</button> \n";

		$header .= "<button type=\"submit\" name=\"cancel\" value=\"cancel\" >Cancel Editing</button> \n";

		$header .= "<input name=\"t\" type=\"hidden\" size=100 value=\"$fn\" readonly>";
	
		if ($_REQUEST['newname']) {
			$fn = dirname ($fn) . "/" . $_REQUEST['newname'] . ".css";
			$header .= "newname is $fn.css<BR>";
		}

		$header .= "<p>Save as new variation (enter as name only, no path, no .css): <input name=\"newname\" type=\"text\" size=\"60\">\n";
		$header .= "<button type=\"submit\" name=\"save\" value=\"save\">Save New Variation</button><br><br>\n";
		
		$css = SetColors ($css, $_REQUEST);
		ShowColors ($css);
		if ($css && $_REQUEST['save'] == "save") {
			file_put_contents ($fn, $css) || $page .= "COULD NOT WRITE FILE $fn!<BR>";
		}
	}
}

$p = file_get_contents ("ce/color_editor.txt");
$p = str_replace ("{HEADER}", $header, $p);
$p = str_replace ("{BODY}", $page, $p);
$p = str_replace ("{JAVASCRIPT}", $headerJS, $p);

print $p;

// =============



function SetColors ($css, $values) {
	global $page;

	list ($colors, $bgcolors) = FetchFields ($css);
	$s = array ();
	$r = array ();
	for ($i=0;$i<count ($colors[0]); $i++) {
		$name = $colors[1][$i];
		if (isset ($values["c_$i"])) {
			$value = trim($values["c_$i"]);
			if ($value) {
				//$page .= "Set color c_$i to $value<BR>";
				$name2 = preg_replace ("/(\.)/", '\\\.', $name);
				$search = "/($name2)\s*.*?\scolor.*;/i";
				$s[] = $search;
				
				$_REQUEST["h_$i"] ? $hilight = "text-decoration: underline overline;" : $hilight ="";
				$replace = trim($name) . " { color : " . trim($value) . ";$hilight";
				$r[] = $replace;

				//$page .= __FUNCTION__ . ": $search --> $replace<br>";
			}
		} else {
			//$css = "";
			break ;
		}
	}

	for ($i=0;$i<count ($bgcolors[0]); $i++) {
		$name = $bgcolors[1][$i];
		if (isset ($values["b_$i"])) {
			$value = trim($values["b_$i"]);
			if ($value) {
				//$page .= "Set bgcolor b_$i to $value<BR>";
				$name2 = preg_replace ("/(\.)/", '\\\.', $name);
				$search = "/($name2)\s*.*?\sbackground-color.*?;/i";
				$s[] = $search;
				$r[] = "$name { background-color : $value;";
			}
		} else {
			//$css = "";
			break ;
		}
	}

	if ($s && $r) {
		$css = preg_replace ($s, $r, $css);
		//$page .= "Replaced values<BR>";
	}
	return $css;
}
		


function ShowColors ($css) {
	global $page, $headerJS;

	list ($colors, $bgcolors) = FetchFields ($css);
	$result = count ($colors[0]);
	//$page .= "Found $result colors<BR>";
	$page .= "TEXT COLORS:<BR><table>\n";
	for ($i=0;$i<count ($colors[0]); $i++) {
		$name = $colors[1][$i];
		$page .= "";
		$value = $colors[2][$i];
		//$headerJS .= '$(\'#c3_' . $i . '\').colorPicker({ click: function(c){$(\'#c_' .$i. '\').val(c);}});' . "\n";
		
		$js = "onChange=\"showColorC('$i')\"";
		
		$page .= "<tr>\n";
		$page .= "<td align=right style='background-color:#ccc'>$name = </td>";
		$page .= "<td style='height:30px;background-color:#ccc;'>";

		$page .= "<a href=\"javascript:void(0);\" rel=\"colorpicker&amp;objcode=c_".$i."&amp;objshow=showcolor_$i&amp;showrgb=1&amp;okfunc=showColorC('$i')\" style=\"text-decoration:none;\" >\n";
		$page .= "<div style=\"float:left; width:17px;height:17px;border:1px solid #888;\"><div id=\"showcolor_$i\" style=\"width:15px;height:15px;border:1px solid black;\">&nbsp;</div></div></a>\n";
		
		//$page .= "<div id='c3_$i'></div>
		//		<div id='c4_$i'></div>\n";
		$page .= "<input $js id=\"c_$i\" name=\"c_$i\" type=\"text\" value=\"$value\">\n</td>\n";
				
		$page .= "<td><div id= \"c1_$i\" style=\"height:30px;padding:0px 30px 0px 30px;background-color:#222;color:" . $value . ";\">Sample</div></td>
				<td><div id= \"c2_$i\" style=\"height:30px;padding:0px 30px 0px 30px;background-color:#eee;color:" . $value . ";\">Sample</div></td>
				<td align=right style=\"white-space: nowrap;background-color:#ccc\"><input name=\"h_$i\" type=\"checkbox\" value=\"blink\">Highlight?</td>\n";
		$page .= "</tr>\n";
	}

	$page .= "<tr><td colspan=4>BACKGROUND COLORS:<BR></td></tr>\n";
	for ($i=0;$i<count ($bgcolors[0]); $i++) {
		$name = $bgcolors[1][$i];
		$page .= "";
		$value = $bgcolors[2][$i];
		//$headerJS .= '$(\'#b3_' . $i . '\').colorPicker({ click: function(c){$(\'#b_' . $i . '\').val(c);}});' . "\n";

		$js = "onChange=\"showColorB('$i')\"";

		$page .= "<tr>\n";
		$page .= "	<td align=right style=\"background-color:#ccc\">$name = </td>
				<td style=\"height:30px;background-color:#ccc;\">";

		$page .= "<a href=\"javascript:void(0);\" rel=\"colorpicker&amp;objcode=b_".$i."&amp;objshow=showbcolor_$i&amp;showrgb=1&amp;okfunc=showColorB('$i')\" style=\"text-decoration:none;\" >\n";
		$page .= "<div style=\"float:left; width:17px;height:17px;border:1px solid #888;\"><div id=\"showbcolor_$i\" style=\"width:15px;height:15px;border:1px solid black;\">&nbsp;</div></div></a>\n";

		$page .= "<input id= \"b_$i\" $js name=\"b_$i\" type=\"text\" value=\"$value\">\n</td>\n";
		$page .= "<td colspan=2><div id= \"b1_$i\" style=\"height:30px;padding:0px 30px 0px 30px;background-color:" . $value . ";\">Sample&nbsp;<span style=\"color:#eee\">Sample</span></div></td>
		";
		$page .= "</tr>\n";
	}
	$page .= "</table>";
}


function FetchFields ($css) {
	global $page;

	// remove comments
	$css = preg_replace ("|/\*(.*?)\*/|s","",$css);

	preg_match_all ("/(.*?){.*?\scolor\s*:\s*(.+?)\s*;/i", $css, $colors);
	preg_match_all ("/(.*?){.*?background-color\s*:\s*(.+?)\s*;/i", $css, $bgcolors);
	return array ($colors, $bgcolors);
}
?>
