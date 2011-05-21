<?php

//======= Theme Editor
// This is a class for editing variations on themes for the FP system.

class Themes {
// ---------------------
// Variables
// ---------------------
	public $FP_THEMES_LIST, $FP_THEMES_PREVIEWS, $FP_THEMES_PREVIEWS_FOR_JS;
	public $FP_Themes;
// $userID is the ID of the current admin user. Useful for deciding which user directory to get/delete/save/whatever from...
	public $themeID, $userID;
// paths to theme files
	public $css, $extraCSS, $adminVariationCSS, $userVariationCSS, $systemVariationCSS, $galleryCSS, $newsletterCSS, $altCSS, $jspath, $jslibfile, $jsfile, $theme, $name;


// ---------------------	
// Functions
// ---------------------

// Constructure php4 style
	function Themes () {
		global $error, $msg;

		$this->FP_THEMES_LIST = array();
		$this->FP_THEMES_PREVIEWS = array();
		$this->FP_THEMES_PREVIEWS_FOR_JS = array();

		$this->userID || $this->userID = FP_ADMINISTRATOR;
		$this->LoadAllThemes ();
	}

// Built an input form for modifying a style sheet
// Variations are stored as themes, so themeID can get variations.
	function Command ( $command="choose", $data = null, $themeID = null) {
		global $error, $msg;

		$DEBUG = 0;

		if (!$themeID)
			$themeID = $this->themeID;

		// add keys to the $data['rules'] array. It won't come that way.
		$newrules = array ();
		foreach ($data['rules'] as $rule) {
			$ri = strtolower($rule['selector'].$rule['declaration']['property']);
			$newrules[$ri] = $rule;
		}

		if ($data['newname']) {
			$this->newThemeFileName = $this->CleanThemeName($data['newname']);
		} else {
			$this->newThemeFileName = '';
		}

		$command || $command = "choose";
		$this->newThemeFileName ? $n = ", {$this->newThemeFileName}<BR>" : $n = "";
		$DEBUG && $msg .= __FUNCTION__.": command=$command, themeid = {$themeID} $n<BR>";

		switch ($command) {
			case "choose" :
				break;
			case "edit" :
				break;
			case "delete" :
				$this->themeID = $themeID;
				$this->Delete ();
				$this->themeID = null;
				break;
			case "save" :
				$this->themeID = $themeID;
				$this->SaveVariation ($newrules);
				//reload themes/variations
				$this->LoadAllThemes ("themes");
				$themeID = $this->themeID;
				$this->FetchCurrentTheme ();
				$this->result = "Saved Variation: " . $this->name . "<BR>";
				break;
			case "cancel" :
				break;
			case "test" :
				$css = $this->FetchCSSFile ($themeID);
				$arr = $this->ParseCSStoRules ($themeID, $css);
				$this->css = $this->InsertValuesIntoCSS ($arr, $css, $data['templateThemeID']);
				$this->SaveCSS ($themeID);
				break;
			default:
				break;
		}
	}



// Built an input form for modifying a style sheet
// Variations are stored as themes, so themeID can get variations.
	function BuildEditorPage ( $subset="choose", $themeID = null) {
		global $error, $msg;

		$DEBUG = 0;

		$this->themeID = $themeID;
		$userinput = $this->CleanRequest();
		
		$this->newThemeFileName = $userinput['newname'];
		//$this->newThemeFileName =  str_replace("/","_",$this->newThemeFileName);
		$this->newThemeFileName = substr(trim(preg_replace ("/\W+/", "_", $this->newThemeFileName)), 0,31);
		$this->newThemeFileName = preg_replace ("/\s/", "_", $this->newThemeFileName);

		$DEBUG && $msg .= "New Name = ".$this->newThemeFileName."<BR>";

		$subset || $subset = "choose";

		$DEBUG && $msg .= __FUNCTION__.": subset=$subset, themeid = $themeID<BR>";

		switch ($subset) {
			case "choose" :
				$this->BuildChooserPage ();
				break;
			case "Edit" :
				$this->themeID ? $this->BuildStyleInputForm () : $this->BuildChooserPage ();
				break;
			case "Delete" :
				$this->Delete ($themeID);
				$this->themeID = null;
				$this->BuildChooserPage ();
				break;
			case "Save" :
				$this->UpdateVariation ($this->RulesFromRequest());
				$this->LoadAllThemes ();	//reload all data. Inefficient, but it will work
				$this->themeID = $themeID;
				$this->BuildChooserPage ();
				$themeID = "";
				break;
			case "Cancel" :
				$this->BuildChooserPage ();
				break;
			// "saveas" is only used to rewrite a variation using a template (or another
			// variation as a template).
			case "saveas" :
				$this->UpdateVariation ($this->RulesFromRequest());
				$this->BuildChooserPage ();
				break;
			case "test" :
				$css = $this->FetchCSSFile ($themeID);
				$arr = $this->ParseCSStoRules ($themeID, $css);
				$this->css = $this->InsertValuesIntoCSS ($arr, $css, $userinput['templateThemeID']);
				$this->SaveCSS ($themeID);
				$this->BuildChooserPage ();
				break;
			default:
				$this->BuildChooserPage ();
				break;
		}

		$f['name'] = $this->FP_Themes[$themeID]['name'];
		$f['ThemeID'] = $themeID;
		$f['action'] = "themeeditor";
		$f['subset'] = $subset;
		$f['LIST'] = $this->form;
		$this->page = Substitutions (FetchSnippet ("input_form_theme_editor"), $f);
	}

// Only delete user variations, not system variations
// If any project or group used this theme, switch it to the system version
// If it is a variation, and there is a system version, switch to the system version
// If it's a user theme with no system version, switch to default for that theme
// NOTE: does not delete if DEBUG is on!
	function Delete ($myThemeID =  null) {
		global $msg, $error;

		$DEBUG = 0;

		if (!$myThemeID)
			$myThemeID = $this->themeID;

		$DEBUG && $error .= __FUNCTION__.": delete {$myThemeID}<BR>";

		if ($this->FP_Themes[$myThemeID]['userfile']) {
			$themeDirs = $this->FetchThemeDirs($myThemeID);
			$path = $themeDirs['variationCSS'];
			$error .= "Deleted theme variation, " . $this->FP_Themes[$myThemeID]['name'] . "<BR>";
			$DEBUG && $error .= __FUNCTION__.": delete $path<BR>";
			if (!$DEBUG) {
				unlinkb ($path);
				if ($this->FP_Themes[$myThemeID]['is_variation']) {
					$newThemeID = $this->FP_Themes[$myThemeID]['id'];
					// if no system version of this user variation, use default for theme
					if (!$this->FP_Themes[$newThemeID])
						$newThemeID = $this->FP_Themes[$myThemeID]['theme_id'];
				} else {
					$newThemeID = DEFAULT_THEME;
				}
				unset ($this->FP_Themes[$myThemeID]);
				$myGroup = new FPGroup();
				$myGroup->ModifyGroups(array('Theme'=>$newThemeID), "Theme", $myThemeID);
			}
		} else {
			$error .= FetchSnippet("error_cannot_delete_system_themes");
		}
	}

// Renaming a variation means changing the name in all references
// because we use the file name, e.g. dark_gray, as the actual reference.
// Bad practice, of course.
	function RenameVariation () {

	}

// Save a variation to the user directory (we don't modify system files)
// If this is a new theme variation and no name is provided,
// we use "My Variation" as the name.
	function SaveVariation ( $newrules = null ) {
		global $error, $msg;

		$DEBUG = 0;

		$myTheme = $this->FP_Themes[$this->themeID];

		// If a new name is provided, create a new variation for it.
		// or, if the current theme is not a variation, make a new variation based on it.
		if ($this->newThemeFileName || !$myTheme['is_variation']) {
			$this->newThemeFileName || $this->newThemeFileName = "My Variation";
			$this->themeID = $this->NewVariation();
			$DEBUG && $msg .= "New variation created as ".$this->name."<br>";
		}

		$DEBUG && $msg .= __FUNCTION__.": Theme to save is \"".$this->themeID."\"<BR>";

		// Update the variation with any new rules
		$this->UpdateVariation ($newrules);
	}

	function UpdateVariation ($newrules = array ()) {
		global $msg, $error;

		$DEBUG = 0;
		$myTheme = $this->FP_Themes[$this->themeID];

		// We need to get rules from current user variation
		if ($myTheme['userfile']) {
			$CSS = GetUserVariationCSSFile;
		} else {
			$CSS = GetSystemVariationCSSFile;
		}

		if ($myTheme['is_variation'] && $myTheme['userfile']) {
			$this->ParseCSStoRules($this->themeID, $this->css);
		} else {
			$this->rules = array ();
		}
		// Merge new rules with existing rules
		if ($newrules) {
			$this->AddRules($this->AddVisitedLinkRules ($newrules));

			// Build page from $this->rules;
			$this->BuildCSSPage();

			$DEBUG && $msg .= "Not saved...debug is on.<BR>";
			$DEBUG || $this->SaveCSS();
		}
	}

// Prepare for a new variation CSS file by making sure the directories
// are ready to use.
	function NewVariation ($themeID = null) {
		global $error, $msg;

		$DEBUG = 0;

		// Where does it go?
		$themeID || $themeID = $this->themeID;
		$themeID || $themeID = $this->GetCurrentThemeID ();
		$myTheme = $this->FP_Themes[$themeID];
		// Source rules are blank if the source is a main theme (not a variation)
		// We don't want to copy the main theme rules to the variation css sheets
		$oldrules = null;
		// If the current theme is a variation, get the parent theme
		// and the rules.
		if (true) {
			// Do not add in old values. Image paths won't work. Besides, we
			// don't need them if we just load the variation stylesheet before the
			// use variation.
			if ($myTheme['is_variation']) {
				// Get the css of the old file
				$sourceCss = $this->FetchCSSFile ();
				$themeID = $myTheme['theme_id'];
				$myTheme = $this->FP_Themes[$themeID];
				$this->themeID = $themeID;
			}
		}
		// Check for user theme variations path
		//
		// If a $this->userID is not the admin user, we should save into a sub-user directory
		if ($this->userID && $this->userID != FP_ADMINISTRATOR) {
			$user_sub_path = "/user_". $this->userID;
		} else {
			$user_sub_path = "";
		}
		$userDir = FP_DIR_USER . "$user_sub_path";

		$csspath = $userDir."/".FP_THEMES_DIR."/".$myTheme['path']."/".FP_THEME_VARIATION_DIR;
		if (!file_exists($csspath)) {
			MakeDirectoryTree (FP_BASEDIR."/".dirname ($csspath));
			//mkdir (FP_BASEDIR."/$csspath", 0755);
			$DEBUG && $msg .= "Created directory:  = $csspath<BR>";
		}

		// Create the basics of the new variation, for saving.
		$myVariation = array ();
		$v_name = $myTheme['name'].":".mb_convert_case ($this->newThemeFileName, MB_CASE_TITLE);
		$v = preg_replace("/\W+/", "_", $this->newThemeFileName);
		$v_filename = $v . ".css";
		$v_id = $this->themeID . ":" . $v;
		$v_id = preg_replace("/\s/", "_", $v_id);
		$myVariation['id'] =  $v_id;
		$myVariation['name'] = $v_name;
		$myVariation['theme_id'] = $myTheme['id'];
		$myVariation['path'] = trim($v_filename);
		$myVariation['is_variation'] = true;
		$myVariation['userfile'] = true;
		$myVariation['css'] = $sourceCss;
		$this->FP_Themes[$v_id] = $myVariation;
		$this->FP_THEMES_LIST[$v_id] = $v_name;

		//$this->FP_Themes[$myTheme['id']] = $myTheme;

		//Set the current theme to this new one
		$this->themeID = $v_id;
		$this->FetchCurrentTheme();

		$DEBUG && $msg .= "Path for new variation will be $csspath<BR>";
		return $v_id;

	}

// Build a CSS text sheet from the $this->rules array
// This is a string that can be saved as a text file, a CSS style sheet.
// Note: we duplicate any :link to make :visited, e.g. x:link is copied to make x:visited
	function BuildCSSPage ($rules = array () ) {
		global $msg, $error;

		$DEBUG = 0;

		if (!$rules)
			$rules = $this->rules;

		$css = "";
		$css .= "/*\nFP Theme Variation CSS Page\nID : {$this->themeID}\nModified : ".date("F j, Y, g:i a") . "\n*/\n\n";

		foreach ($rules as $rule) {
			$name = trim($rule['selector']);
			$property = trim($rule['declaration']['property']);
			$value = trim($rule['declaration']['value']);
			$description = trim($rule['description']);
			$title = trim($rule['title']);

			// If no title, move to the "Other" or "advanced" section
			$title || $title = "ADVANCED ";
			$description || $description = "$property definition for $name";

			$r = "";
			// Descriptions are of the format, "title : description". The ":' is important.
			$r .= "/* $title : $description */\n";
			$r .= "$name { $property : $value; }";
			// later created versions of the property overwrite,
			// so we don't have two "size" properties, for example
			$list[$name.'@'.$property] = $r;
			
			// IF we have a :link, we must create a matching :visited
			$propertyVisited = str_replace (":link", ":visited", $property);
			if ( $propertyVisited != $property ) {
				// If no title, move to the "Other" or "advanced" section
				$description = "$property definition for $name";
				$r = "";
				$r .= "/* $title : $description *"."/\n";
				$r .= "$name { $propertyVisited : $value; }\n\n";
				$list[$name.'@'.$property] = $r;
			}

			$DEBUG && $msg .= __FUNCTION__.": ".str_replace("\n",'<br>',$r)."<hr>";
			$DEBUG && fp_error_log(__FUNCTION__.": $property", 3, FP_ACTIVITY_LOG);
			//$DEBUG && fp_error_log(__FUNCTION__.": ".print_r($rules,true), 3, FP_ACTIVITY_LOG);
		}
		$css = join("\n\n",$list);
		$this->css = $css;
	}

// Convert a $_REQUEST from the form-based editor into a CSS sheet object we can use.
// Return $rules, an array of css rules:
// Example: $rule[] = {'selector'=>'H1', 'declaration'=> {'property'=>'color', 'value'=>'red'}};
// *Important: $_REQUEST['count'] contains the number of rules!
	function RulesFromRequest () {
		global $msg, $error;

		$DEBUG = 0;

		$DEBUG && $msg .= __FUNCTION__."<BR>";
		$userinput = $this->CleanRequest();
		$DEBUG && var_dump ($userinput);
		$rules = array ();

		$count = $userinput['count'];
		$this->themeID = $userinput['ThemeID'];

		$DEBUG && $msg .= __FUNCTION__.": Rule Count = $count<BR>";
		for ($i=0;$i<$count; $i++) {
			if (isset ($userinput["c_$i"])) {
				$DEBUG && $msg .= __FUNCTION__.": c_$i = ".$userinput["c_$i"]."<BR>";
				$value = trim(stripslashes($userinput["c_$i"]));
				if ($value) {
					$name = $userinput["name_$i"];
					$title = $userinput["title_$i"];
					$desc = $userinput["desc_$i"];

					$DEBUG && $msg .= __FUNCTION__.": Rule = $name<BR>";

					// if multiple names (selectors) are together, e.g. h1,h2
					// then we split that into multiple rules, one for each
					if (strpos($name, ',') != false) {
						$names = explode(',', $name);
					} else {
						$names = array ($name);
					}

					foreach ($names as $name) {
						$name = trim($name);
						$property = $userinput["param_$i"];
						//$namefixed = preg_replace ("/(\.)/", '\\\.', $name);
						//$namefixed = preg_replace ("/(\s)/", '_', $namefixed);

						$ri = strtolower($name.$property);
						$rules[$ri] = array (
								'selector'=>$name,
								'title'=>$title,
								'description'=>$desc,
								'declaration'=> array ('property'=>$property, 'value'=>$value)
						);
						$DEBUG && $msg .= __FUNCTION__ . ": Rule: $name : {".$property." : $value};<BR>\n";
						$DEBUG && fp_error_log(__FUNCTION__.": ".print_r($rules,true), 3, FP_ACTIVITY_LOG);
					}
				}
			}
		}
		return $rules;
	}

	function BuildChooserPage ( $themeIDvarname = "ThemeID") {
		// THEME_LIST_FIELD popup

		$this->FetchVariationsList();
		$ThemeList = OptionListFromArray (
			$this->FP_VARIATIONS_LIST,
			$themeIDvarname,
			$this->themeID,
			TRUE,
			OPTION_LIST_IS_POPUP,
			$extrahtml=""
		);
		$f['THEMELIST'] = $ThemeList;
		$this->form = Substitutions (FetchSnippet ("input_form_theme_editor_chooser"), $f);

	}

// Return the CSS style sheet file for the variation (alt) CSS for a theme, if there's a user variation, get that.
	function FetchCSSFile ($myThemeID = null) {
		global $msg, $error;

		$DEBUG = 0;

		$DEBUG && $msg .= __FUNCTION__.": from $myThemeID (".$this->themeID.")<BR>";

		$myThemeID || $myThemeID = $this->themeID;

		$myTheme = $this->FP_Themes[$myThemeID];

		// If theme is missing, return error
		if (!$myTheme) {
			$error .= FetchSnippet ("error_theme_editor_not_exists");
			$this->BuildChooserPage ();
			return null;
		}

		// NOT IN USE:
		// Make sure it's a variation, not a real theme
		if (FALSE && $myTheme['is_variation']) {
			$error .= FetchSnippet ("error_theme_editor_not_variation");
			return null;
		}

		$themeDirs = $this->FetchThemeDirs($myThemeID);
		$td = $themeDirs['variationCSS'];
		$td ? $css = file_get_contents ($themeDirs['variationCSS']) : $css = "";

		return $css;
	}

// Return the CSS style sheet file for the variation (alt) CSS for a theme, but always get the system version, not the user version
	function FetchSystemCSSFile ($myThemeID = null) {
		global $msg, $error;

		$DEBUG = 0;

		$DEBUG && $msg .= __FUNCTION__.": from $myThemeID (".$this->themeID.")<BR>";

		$myThemeID || $myThemeID = $this->themeID;

		$myTheme = $this->FP_Themes[$myThemeID];

		// If theme is missing, return error
		if (!$myTheme) {
			$error .= FetchSnippet ("error_theme_editor_not_exists");
			$this->BuildChooserPage ();
			return null;
		}

		// NOT IN USE:
		// Make sure it's a variation, not a real theme
		if (FALSE && $myTheme['is_variation']) {
			$error .= FetchSnippet ("error_theme_editor_not_variation");
			return null;
		}

		$themeDirs = $this->FetchThemeDirs($myThemeID);
		$css = file_get_contents ($themeDirs['variationCSS']);
		return $css;
	}


// Write $this->css to a file.
// IF this->newThemeFileName is set, that name will be used for writing
// and a new file will be created.
// Otherwise use Variation name. If main theme is "default", the variation might be "ivory".
// We save file as "ivory" but it's linked to "default".
	function SaveCSS ($themeID = null) {
		global $error, $msg;

		$DEBUG = 0;
		$NOWRITE = 0;

		$DEBUG && $msg .= __FUNCTION__.": Begin<BR>";

		$themeID || $themeID = $this->themeID;
		$myTheme = $this->FP_Themes[$themeID];

		$DEBUG && $msg .= __FUNCTION__.": themeID = $themeID<BR>";

		// If theme is missing, return error
		if (!$myTheme) {
			$error .= FetchSnippet ("error_theme_editor_not_exists");
			$this->BuildChooserPage ();
			return null;
		}

		$myTheme['theme_id'] ? $dir = $this->FP_Themes[$myTheme['theme_id']]['path'] : $dir = DEFAULT_THEME;
		$filename = $myTheme['path'];
		// Be sure to replace spaces with _
		$filename = str_replace (" ","_",$filename);

		// If a $this->userID is not the admin user, we should save into a sub-user directory
		if ($this->userID && $this->userID != FP_ADMINISTRATOR) {
			$user_sub_path = "/user_". $this->userID;
		} else {
			$user_sub_path = "";
		}
		$userDir = FP_DIR_USER . "$user_sub_path";

		// Set the path+filename to write
		$path = "$userDir/".FP_THEMES_DIR."/$dir/".FP_THEME_VARIATION_DIR."/".$filename;

		$NOWRITE && $msg .= "Turn off NOWRITE, and I will write file " . $path . "<BR>";

		$DEBUG && $msg .= "{$this->css}<BR>";
		$DEBUG && $msg .= "File to write: $path<BR>";

		if (!$NOWRITE) {
			// Make sure the directories exist to save this file
			MakeDirectoryTree (FP_BASEDIR."/".dirname ($path));
			file_put_contents ($path, $this->css) || $error .= "COULD NOT WRITE FILE $path!<BR>";
		}
	}


// Built an input form for modifying a style sheet
// Variations are stored as themes, so themeID can get variations.
// $whichcss is 'css', 'extraCSS', 'variationCSS' which are keys that 
// CurrentThemeDir uses in the array it returns of paths to CSS
	function BuildStyleInputForm () {
		global $error, $msg;

		$DEBUG = 0;

		$myTheme = $this->FP_Themes[$this->themeID];
		//$this->css = $this->FetchCSSFile ();
		$this->ParseCSStoRules();	// set $this->rules

		$rows = "";
		$color_row = FetchSnippet ("input_form_theme_editor_color_row");
		$bgcolor_row = FetchSnippet ("input_form_theme_editor_bg_row");
		$border_row = FetchSnippet ("input_form_theme_editor_border_row");
		$font_row = FetchSnippet ("input_form_theme_editor_font_row");
		$anyproperty = FetchSnippet ("input_form_theme_editor_anything");

		$rows = "";
		$rowsAdvanced = "";
		$valsform = FetchSnippet("input_form_theme_row_vals");

		$i = 0;
		foreach ($this->rules as $rule) {
			$name = $rule['selector'];
			$property = $rule['declaration']['property'];
			$value = $rule['declaration']['value'];
			$description = $rule['description'];
			$title = $rule['title'];

			$js = "onChange=\"showColorC('$i')\"";
			switch ($property) {
				case "color"	: $row = $color_row;
					break;
				case "background-color"	: $row = $bgcolor_row;
					break;
				case "border-color"	: $row = $border_row;
					break;
				case "border-top"	: $row = $border_row;
					break;
				case "border-bottom"	: $row = $border_row;
					break;
				case "border-left"	: $row = $border_row;
					break;
				case "border-right"	: $row = $border_row;
					break;
				case "font-family"	: $row = $font_row;
					break;
				default: $row = $anyproperty;
			}


			$f = array ();
			$f['VALUES'] = $valsform;	// MUST BE FIRST, here...following vars fill it in!
			$f['NAME'] = $name;
			$f['CLEANNAME'] = trim ($name, '#.');
			$f['VALUE'] = htmlentities (stripslashes($value));
			$f['PARAM'] = $property;
			$f['DESCRIPTION'] = $description;
			$f['X'] = $i;	//must do last in list...used by other vars
			$f['TITLECODE'] = $title;
			if (preg_match ("/advanced\s*/i", $title)) {
				$f['TITLE'] = preg_replace ("/advanced\s*-?\s*/i", "", $title);
				$row = Substitutions ($row, $f);
				$rowsAdvanced .= $row;
			} else {
				$f['TITLE'] = $title;
				$row = Substitutions ($row, $f);
				$rows .= $row;
			}
			$i++;
		}

		$f = array ();
		$f["LIST"] = $rows;
		$f["LIST2"] = $rowsAdvanced;
		$f["COUNT"]  = count ($this->rules);

		$form = Substitutions (FetchSnippet ("input_form_theme_editor_block"), $f);

		$this->BuildChooserPage ("templateThemeID");
		$this->form = Substitutions ($form, array ("POPUPLIST"=>$this->form));
	}


// Built an input form for modifying a style sheet
// Variations are stored as themes, so themeID can get variations.
// $whichcss is 'css', 'extraCSS', 'variationCSS' which are keys that 
// CurrentThemeDir uses in the array it returns of paths to CSS
	function BuildStyleInputFormFromCSSText () {
		global $error, $msg;

		$DEBUG = 0;

		$myTheme = $this->FP_Themes[$this->themeID];
		$this->css = $this->FetchCSSFile ();

		// Parse the CSS to get the data
		// Format should be title, stylename, property, value, description
		$this->FetchVariationStyles ();

		//$styles = unserialize(FetchSnippet ("input_form_theme_editor_styles"));
		$rows = "";
		$color_row = FetchSnippet ("input_form_theme_editor_color_row");
		$bgcolor_row = FetchSnippet ("input_form_theme_editor_bg_row");
		$border_row = FetchSnippet ("input_form_theme_editor_border_row");
		$font_row = FetchSnippet ("input_form_theme_editor_font_row");
		$anyproperty = FetchSnippet ("input_form_theme_editor_anything");

		$rows = "";
		$rowsAdvanced = "";
		$valsform = FetchSnippet("input_form_theme_row_vals");

		for ($i=0;$i<count ($this->styles[1]); $i++) {
			$title = trim($this->styles[1][$i]);
			$name = trim($this->styles[2][$i]);
			$property = trim($this->styles[3][$i]);
			$value = trim($this->styles[4][$i]);
			list ($title, $description) = explode(":", $title);
			//$headerJS .= '$(\'#c3_' . $i . '\').colorPicker({ click: function(c){$(\'#c_' .$i. '\').val(c);}});' . "\n";

			$js = "onChange=\"showColorC('$i')\"";
			switch ($property) {
				case "color"	: $row = $color_row;
					break;
				case "background-color"	: $row = $bgcolor_row;
					break;
				case "border-color"	: $row = $border_row;
					break;
				case "border-top"	: $row = $border_row;
					break;
				case "border-bottom"	: $row = $border_row;
					break;
				case "border-left"	: $row = $border_row;
					break;
				case "border-right"	: $row = $border_row;
					break;
				case "font-family"	: $row = $font_row;
					break;
				default: $row = $anyproperty;
			}


			$f = array ();
			$f['VALUES'] = $valsform;	// MUST BE FIRST, here...following vars fill it in!
			$f['NAME'] = $name;
			$f['CLEANNAME'] = trim ($name, '#.');
			$f['VALUE'] = htmlentities (stripslashes($value));
			$f['PARAM'] = $property;
			$f['DESCRIPTION'] = $description;
			$f['X'] = $i;	//must do last in list...used by other vars

			if (preg_match ("/advanced\s*/i", $title)) {
				$title = preg_replace ("/advanced\s*-?\s*/i", "", $title);
				$f['TITLE'] = $title;
				$row = Substitutions ($row, $f);
				$rowsAdvanced .= $row;
			} else {
				$f['TITLE'] = $title;
				$row = Substitutions ($row, $f);
				$rows .= $row;
			}
		}

		$f = array ();
		$f["LIST"] = $rows;
		$f["LIST2"] = $rowsAdvanced;
		$f["COUNT"]  = count ($this->styles[1]);

		$form = Substitutions (FetchSnippet ("input_form_theme_editor_block"), $f);

		$this->BuildChooserPage ("templateThemeID");
		$this->form = Substitutions ($form, array ("POPUPLIST"=>$this->form));
	}


// Parse the a CSS string into an array containing the important
// styles. We can use his input and output CSS sheets.
// It also sets $this->rules to an object of the CSS rules (different from the
// css array the function returns).
	function ParseCSStoRules ($myThemeID = null, $myCss = null) {
		global $error, $msg;

		$DEBUG = 0;
		$rules = array ();
		$cssArray = array ();

		$myThemeID
				? $myTheme = $this->FP_Themes[$myThemeID]
				: $myTheme = $this->FP_Themes[$this->themeID];

		$myCss
				? $this->css = $myCss
				: $this->css = $this->FetchCSSFile ();

		// Parse the CSS to get the data
		// Format should be title, stylename, property, value, description
		$this->FetchVariationStyles ();

		for ($i=0;$i<count ($this->styles[2]); $i++) {
			$name = trim($this->styles[2][$i]);
			$property = trim($this->styles[3][$i]);
			$value = trim($this->styles[4][$i]);
			$fulltitle = trim($this->styles[1][$i]);
			list ($title, $description) = explode(":", $fulltitle);
			$title = trim($title);
			$description = trim($description);

			// if multiple names (selectors) are together, e.g. h1,h2
			// then we split that into multiple rules, one for each
			if (strpos($name, ',') != false) {
				$names = explode(',', $name);
			} else {
				$names = array ($name);
			}

			foreach ($names as $name) {
				$name = trim($name);
				$cssArray[$name] = array (
						"name"			=> $name,
						"property"		=> $property,
						"value"			=> $value,
						"title"			=> $title,
						"description"	=> $description
				);
				$ri = strtolower($name.$property);
				$ri = preg_replace ("/\W/", "__", $ri);
				$DEBUG && $msg .= __FUNCTION__.": <i>$ri:</i>  $name { $property : $value }<BR>";

				// preserve title/desc from prev. rules on page, if a later rule overwrites
				if ($rules[$ri]) {
					$title || $title = $rules[$ri]['title'];
					$description || $description = $rules[$ri]['description'];
					$DEBUG && $msg .= __FUNCTION__.": title = $title<BR><BR>";
					$DEBUG && $msg .= __FUNCTION__.": description = $description<BR><BR>";
				}
				$rules[$ri] = array (
						'selector'=>$name,
						'declaration'=> array (
								'property'=>$property,
								'value'=>$value
						),
						"title"			=> $title,
						"description"	=> $description
				);
			}
		}
		$this->rules = $rules;
		//$DEBUG && var_dump ($rules);
		return $cssArray ;
	}


// Be sure each :link rule has a corresponding :visited rule
// I.e., if a.ooga:link exists, then a.ooga:visited must also exist
	function AddVisitedLinkRules ($newrules = array () ) {
		$newrules || $newrules = $this->rules;

		while (list ($k, $v) = each ($newrules)) {
			$r2 = str_replace (":link", ":visited", $k);
			if ($r2 != $k) {
				// we found a ":link". Does the ":visited" exists in array at all?
				if (!$newrules[$r2]) {
					// :visited doesn't exist, create it
					$newrule = $v;
					$newrule['selector'] = str_replace (":link", ":visited", $v['selector']);
					$newrules[$r2] = $newrule;
				}
			}
		}
		return $newrules;
	}

// Merge rules into $this->rules.
// Used to add new rules, or to overwrite existing ones.
	function AddRules ( $newrules = array() ) {
		global $error, $msg;

		$DEBUG = 0;

		$newrules || $newrules = $this->rules;
		$oldrules = array_intersect_key($this->rules, $newrules);

		// Merge matching new rules to keep title/description fields and values from new
		while (list ($key, $oldrule) = each ($oldrules)) {
			$DEBUG && $msg .=  __FUNCTION__.": $key<BR>";
			$newrule = $newrules[$key];
			// preserve title/desc from prev. rules on page, if a later rule overwrites
			$newrule['title'] ? $title = $newrule['title'] : $title = $oldrule['title'];
			$newrule['description'] ? $description = $newrule['description'] : $description = $oldrule['description'];
			$DEBUG && $msg .=  __FUNCTION__.": $title : $description <BR>";

			$this->rules[$key] = array (
					'selector'=>$newrule['selector'],
					'declaration'=> array (
							'property'=>$newrule['declaration']['property'],
							'value'=>$newrule['declaration']['value']
					),
					"title"			=> $title,
					"description"	=> $description
			);
		}

		// Now, merge new and old, overwriting old rules.
		// Our keys are a combination of selector and property, so each will
		// unique, and we won't have the problem of destroying different properties
		// of the same selector, e.g font-size and color (properties) of #mystyle
		$this->rules = array_merge($this->rules, $newrules);
	}

// Given an array prepared by ParseCSStoRules, insert the values
// into a CSS sheet and return the css text. 
// InsertValuesIntoCSS
	function InsertValuesIntoCSS ($arr, $css, $templateThemeID) {
		global $msg, $error;

		$DEBUG = 0;

		$s = array ();
		$r = array ();
		foreach ($arr as $style) {
			$name = $style['name'];
			$value = $style['value'];
			$DEBUG && $msg .= __FUNCTION__.": Set $name to $value<BR>";
			if ($value) {
				$property = $style['property'];
				$DEBUG && $msg .= __FUNCTION__.": Set $property to $value<BR>";
				$search = "|$name\s*{\s*(.*?)\s*$property\s*:\s*.+?\s*;|i";
				$s[] = $search;
				$replace = trim($name) . " { \$1" . trim ($property) . " : " . trim($value) . ";";
				$r[] = $replace;
				$DEBUG && $msg .= __FUNCTION__ . ": $search --> $replace<br>";
			}
		}

		// Rescue the bottom of the style sheet, marked by the
		// comment "OTHER". We'll tack this on to the new template.
		// It can contain specilized stuff, like centering, etc.
		$pattern = "|(/\*\s*\-*\s*OTHER\s*\-*\s*\*/)\s*(.*)|si";
		preg_match ($pattern, $css, $matches);
		$other = $matches[2];

		// Fetch template CSS
		$css = $this->FetchCSSFile ($templateThemeID);
		// Add "other" to the end
		$css .= $other;
		$msg .= "Using template $templateThemeID to rewrite ".$this->themeID.".<BR><BR>";

		$css = preg_replace ($s, $r, $css);
		// Change modification date-time
		$s = "|modified\s*:.*?\r*\*/|is";
		$r = "Modified : ".date("F j, Y, g:i a") . "  */";
		$css = preg_replace ($s, $r, $css);

		return $css;
	}


	/* 	Important:
	All properties must be written as single entries, e.g. mystyle { color: #123 }
	and on a single line. Do NOT combine entries if you want them to appear
	in the editor.
	*/
//	You can have separators, like /* ==== FONTS ==== */
//	These won't be a problem unless they're followed by a style.
//	Entries include a comment and the property:
//		/* User-Friend Title: Fuller Description */
//		myStyle { color:#123;}
//
	function FetchVariationStyles () {
		global $msg, $error;
		//preg_match_all ("|/\*\s*(.*?)\s*\*/\n(.*?)\s*{.*?\s(\S*)\s*:\s*(.+?)\s*;|i", $this->css, $this->styles);
		preg_match_all ("|(?:/\*\s*(.*?)\s*\*/)\s*(.*?)\s*{\s*.*?\s*(\S*)\s*:\s*(.+?)\s*;\s*}|i", $this->css, $this->styles);
	}


// Loads themes from disk files into arrays
// This can be hundreds of files (mostly code snippets).
	function LoadAllThemes ($loadThemesOnly = null) {

		$this->FP_THEMES_LIST = array ();		// list of themes for creating menus
		if ($loadThemesOnly) {
			// Remove existing variations, leaving themes & snippets alone
			foreach ($this->FP_Themes as $theme) {
				if ($theme['is_variation'])
					unset ($this->FP_Themes[$theme['id']]);
			}
		} else {
			$this->FP_THEMES = array ();		// list of themes for creating menus
		}

		$themedirs = glob (FP_THEMES_DIR."/*", GLOB_ONLYDIR);

		// Build array of theme objects which contain all theme info.
		$themeClassName = null;	// a temp var filled in by the 'include', below
		foreach ( $themedirs as $themepath) {
			$this->LoadTheme($themepath, $loadThemesOnly);
		}
	}

	/*
Load a theme from disk, using the path, e.g. mytheme1001
It loads the theme into $this->FP_Themes, $this->FP_THEMES_LIST

Local over-rides:
- FP_DIR_USER ("_user") directory may contain files to replace those in _themes
- They must be in exactly the same path inside of FP_DIR_USER, 
	e.g. _user/_themes/default/_snippets/frameshop/frameshop_shipping_popup.txt
- The user, site-wide vocabulary list is also in _user/vocabulary.txt. Entries in this are merged with
  entries in the _themes/vocabulary.txt lists, overriding them.
 * $type = string, what to load: If typeToLoad == "themes", just reload themes.
	*/
	function LoadTheme ($themepath, $loadThemesOnly = null) {
		$DEBUG = false;
		$DEBUG && print __FILE__.":".__FUNCTION__.": DEBUG ON<HR>";
		$themedir = basename ($themepath);
		if (file_exists("$themepath/theme.ini")) {
			$myTheme = parse_ini_file ($themepath . "/theme.ini");
			if (isset($myTheme['disabled']) && $myTheme['disabled'] == true)
				return;
			$myTheme['id'] = str_replace (":","_",$myTheme['id']);	// just making sure there are no ":" in the ID!
			$myTheme['path'] = basename($themepath);

			if (!$loadThemesOnly) {
				// Load all code snippets in _snippets AND subdirectories (handy for organization)
				$snippets = array ();
				$snippetdirs = glob ($themepath . "/" . FP_SNIPPETSDIR . "/*", GLOB_ONLYDIR);
				$snippetdirs[] = ".";
				$userDir = FP_DIR_USER;
				foreach ($snippetdirs as $dir) {
					$dir = basename ($dir);
					foreach (glob ($themepath . "/" . FP_SNIPPETSDIR . "/$dir/*.txt") as $fn) {
						// If there is a User version of the snippet, use that instead
						// Check exact for file in exactly the same dir structure
						$fnUser = $userDir . "/$fn";
						if (file_exists($fnUser)) {
							$fn = $fnUser;
						} else {
							// Check if the file is only in the user's snippets dir
							$fnUser = "$userDir/$themepath/".FP_SNIPPETSDIR.'/'.basename($fn);
							file_exists($fnUser) && $fn = $fnUser;
						}
						// trim 3 or 4 char extension to get snippet name
						$name = strtolower(preg_replace ("/(\.....?)$/","",basename($fn)));
						$snippets[$name] = file_get_contents ($fn);
					}
				}

				// Get user "vocabulary.txt" file
				file_exists($userDir."/".'vocabulary.txt')
						? $snippets['vocabulary_user'] = file_get_contents ($userDir."/".'vocabulary.txt')
						: $snippets['vocabulary_user'] = "";

				foreach (glob ($themepath . "/" . FP_JSDIR . "/*.js") as $fn) {
					$name = strtolower(trim (basename($fn)));
					$snippets[$name] = file_get_contents ($fn);
				}
				$myTheme['snippets'] = $snippets;
			} else {
				// Restore the snippets from the existing theme
				$myTheme['snippets'] = $this->FP_Themes[$myTheme['id']]['snippets'];
			}

			// add theme to the list
			$this->FP_Themes[$myTheme['id']] = $myTheme;
			// add "default" to the base theme name, e.g. mytheme:default
			$this->FP_Themes[$myTheme['id']]['name'] .= ":Default";

			// Add any variations as themes, also.
			// Built menus and lists for setting themes
			// A variation shows up as a theme, but it has the 'variation' flag set.
			$myTheme['is_variation'] = false; // default is false
			$previewpath = $themepath."/".FP_PREVIEWDIR;
			if ($themedir != "admin") {
				// add this theme to the menu listing
				// Add a space before 'default' to push it above others when sorting
				$this->FP_THEMES_LIST[$myTheme['id']] = $myTheme['name'].": Default";
				// Get theme preview file
				$previewfile = $previewpath."/default.png";
				file_exists($previewfile) && $this->FP_THEMES_PREVIEWS[$myTheme['id']] = $myTheme['id'] . ":" . $previewfile;
				file_exists($previewfile) && $this->FP_THEMES_PREVIEWS_FOR_JS[$myTheme['id']] = '_'.$myTheme['id'] . ':"' . $previewfile . '"';

				// variations for this theme
				// ID is just the dir name of the variation
				// Name looks like, ThemeName:VariationName
				// Add a new "theme" to the list, that's really a place-holder
				// for the current theme + variation
				//
				if ($this->userID && $this->userID != FP_ADMINISTRATOR) {
					$user_sub_path = "/user_". $this->userID ;
				} else {
					$user_sub_path = "";
				}
				$userDir = FP_DIR_USER . $user_sub_path;
				$vglob = $themepath."/".FP_THEME_VARIATION_DIR . "/*.css";
				$vlist = glob ($vglob);
				$user_vlist = glob ("$userDir/$vglob");

				// load system variation files
				foreach ($vlist as $v) {
					$myVariation = array ();
					$v_filename = str_replace(".css", "", basename($v));
					$v_id = $myTheme['id'] . ":" . $v_filename;
					$v_id = preg_replace("/\s/", "_", $v_id);
					$v_name = $myTheme['name'].":".mb_convert_case (basename ($v_filename), MB_CASE_TITLE);
					$v_name = str_replace("_", " ", $v_name);
					$myVariation['id'] =  $v_id;
					$myVariation['name'] = $v_name;
					$myVariation['theme_id'] = $myTheme['id'];
					$myVariation['path'] = trim(basename($v));
					$myVariation['is_variation'] = true;
					$myVariation['userfile'] = false;
					$this->FP_Themes[$v_id] = $myVariation;
					$this->FP_THEMES_LIST[$v_id] = $v_name;

					// Get theme preview file
					$previewfile = $previewpath."/{$v_filename}.png";

					file_exists($previewfile) && ($this->FP_THEMES_PREVIEWS[$v_id] = $v_id . ":" . $previewfile);
					// note : => __ (two underscores)
					$k = str_replace(":","__", $v_id);
					file_exists($previewfile) && $this->FP_THEMES_PREVIEWS_FOR_JS[$v_id] = '_'.$k . ':"' . $previewfile . '"';
				}

				// Load admin user variation files
				if ($this->userID != FP_ADMINISTRATOR) {
					$admin_vlist = glob (FP_DIR_USER."/".$vglob);
					// load admin user variation files
					foreach ($admin_vlist as $v) {
						$myVariation = array ();
						$v_filename = str_replace(".css", "", basename($v));
						$v_id = $myTheme['id'] . ":" . $v_filename;
						$v_id = preg_replace("/\s/", "_", $v_id);
						$v_name = $myTheme['name'].":".mb_convert_case (basename ($v_filename), MB_CASE_TITLE);
						$v_name = str_replace("_", " ", $v_name);
						$myVariation['id'] =  $v_id;
						$myVariation['name'] = $v_name;
						$myVariation['theme_id'] = $myTheme['id'];
						$myVariation['path'] = trim(basename($v));
						$myVariation['is_variation'] = true;
						$myVariation['userfile'] = false;
						$this->FP_Themes[$v_id] = $myVariation;
						$this->FP_THEMES_LIST[$v_id] = $v_name;
					}
				}

				// load user CSS variation files
				// mark them in the list with a "*"
				($DEBUG && $loadThemesOnly) && print __FUNCTION__.": Load Themes Only ON<BR>";
				($DEBUG && $loadThemesOnly) && var_dump ($user_vlist);
				foreach ($user_vlist as $v) {
					$myVariation = array ();
					$v_filename = str_replace(".css", "", basename($v));
					$v_id = $myTheme['id'] . ":" . $v_filename;
					$v_id = preg_replace("/\s/", "_", $v_id);
					$v_name = $myTheme['name'].":".mb_convert_case (basename ($v_filename), MB_CASE_TITLE) . "*";
					$v_name = str_replace("_", " ", $v_name);
					$myVariation['id'] =  $v_id;
					$myVariation['name'] = $v_name;
					$myVariation['theme_id'] = $myTheme['id'];
					$myVariation['path'] = trim(basename($v));
					$myVariation['is_variation'] = true;
					$myVariation['userfile'] = true;
					$this->FP_Themes[$v_id] = $myVariation;
					$this->FP_THEMES_LIST[$v_id] = $v_name;
					($DEBUG && $loadThemesOnly) && print __FUNCTION__.": Loaded v_id<BR>";

				}
			}
		}
	}

	/* Build a list of variations available for all themes */
	function FetchVariationsList () {

		$this->FP_VARIATIONS_LIST =  array();
		foreach ($this->FP_Themes as $myTheme) {
			if ($myTheme['is_variation']) {
				$myTheme['userfile'] ? $asterisk = "*" : $asterisk = "";
				$this->FP_VARIATIONS_LIST[$myTheme['id']] = $asterisk . $myTheme['name'];
			}
		}
	}

	function FetchCurrentTheme ($themeID=null) {
		$themeID || $themeID = $this->themeID;
		if ($themeID) {
			$myTheme = $this->FP_Themes[$themeID];
			$this->themeID = $themeID;
			if (is_array($myTheme)) {
				while (list($k,$v) = each($myTheme)) {
					$this->$k = $v;
				}
			}
		}
	}


	/* 	Get the current theme ID.
	The current theme ID depends on the current session groupID.
	IF the groupID is not set, then the current theme ID follows 
	from the Public group's theme ID.
	*/
	function GetCurrentThemeID () {
		global $msg, $error;

		if (isset($_SESSION['GroupID'])) {
			if ($_SESSION['GroupID'] == FP_ADMINISTRATOR_GROUP) {
				$_SESSION['theme'] = ADMIN_THEME;
			} else {
				$myGroup = new FPGroup ($_SESSION['GroupID']);
				$_SESSION['theme'] = $myGroup->theme;
			}
		} else {
			$_SESSION['GroupID'] = PUBLIC_GROUP_ID;
			$myGroup = new FPGroup ($_SESSION['GroupID']);
			$_SESSION['theme'] = $myGroup->theme;
		}

		// set session vars to defaults if not set, for some reason
		isset($_SESSION['theme']) || $_SESSION['theme'] = DEFAULT_THEME;

		$this->themeID = $_SESSION['theme'];

		// Make sure that theme ID really exists and isn't a bug!
		if (!$this->themeID || !isset($Themes->FP_THEMES_LIST[$this->themeID])) {
			$this->themeID = DEFAULT_THEME;
			$_SESSION['theme'] = $this->themeID;
		}
		return $this->themeID;
	}

// Return paths to theme directories inside the theme directories
// array = ("theme"=>themepath, "css"=>csspath, "js"=>jspath)
// If the theme doesn't have a directory for something, we use the default directories
// If the theme is a 'variation', then CSS will point to that variation css path.
	function FetchCurrentThemeDirs ($themeID = null) {
		global $error, $msg;

		$DEBUG = 0;

		$themeID || $themeID = $this->themeID;
		$themeID || $themeID = $this->GetCurrentThemeID ();
		$result = $this->FetchThemeDirs ($themeID);

		while (list ($k,$v) = each ($result)) {
			$this->$k = $v;
		}

		return $result;
	}

// Return paths to theme directories inside the theme directories
// array = ("theme"=>themepath, "css"=>csspath, "js"=>jspath)
// If the theme doesn't have a directory for something, we use the default directories
// If the theme is a 'variation', then CSS will point to that variation css path.
	function FetchThemeDirs ($themeID = null) {
		global $error, $msg;

		$DEBUG = 0;

		$themeID || $themeID = $this->themeID;
		$themeID || $themeID = $this->GetCurrentThemeID ();
		$myTheme = $this->FP_Themes[$themeID];

		$DEBUG && $error .= __FUNCTION__.": theme id=" . $themeID ."<BR>";

		// *** Here's where variations come into play ***

		// If this theme is, in fact, a variation, then we get a third CSS from the variation
		// If there is a user variation of the variation, we get that, too
		// After that, we switch over to the main theme
		if ($myTheme['is_variation']) {
			$variationFilename = str_replace(" ", "_", $myTheme['path']);

			// Get main theme and it's path
			$myTheme = $this->FP_Themes[$myTheme['theme_id']];
			$themePathName = $myTheme['path'];
			file_exists (FP_THEMES_DIR."/".$themePathName) || $themePathName = DEFAULT_THEME;

			$baseThemePath = FP_THEMES_DIR."/$themePathName/".FP_THEME_VARIATION_DIR;
			
			// Set up paths for checking
			$systemVariationCSS = "$baseThemePath/$variationFilename";
			$adminVariationCSS = FP_DIR_USER . "/$systemVariationCSS";
			if ($this->userID && $this->userID != FP_ADMINISTRATOR) {
				$userVariationCSS = FP_DIR_USER . "/user_{$this->userID}/$systemVariationCSS";
			} else {
				$userVariationCSS = "";
			}
			
			file_exists ($systemVariationCSS)|| $systemVariationCSS = "";

			file_exists ($adminVariationCSS) || $adminVariationCSS = "";

			file_exists ($userVariationCSS)|| $userVariationCSS = "";

			$DEBUG && print "systemVariationCSS = $systemVariationCSS<br>";
			$DEBUG && print "adminVariationCSS = $adminVariationCSS<br>";
			$DEBUG && print "userVariationCSS = $userVariationCSS<br>";
			
			// The user variation is either the admin or user version, depending on who's asking.
			// If $this->userID is not the admin user, we should look in a user sub-directory
			if ($this->userID && $this->userID != FP_ADMINISTRATOR) {
				$variationCSS = $userVariationCSS;
			} else {
				$variationCSS = $adminVariationCSS;
			}

		} else {
			if ($myTheme['path'] && file_exists (FP_THEMES_DIR."/".$myTheme['path'])) {
				$themePathName = $myTheme['path'];
			} else {
				$themePathName = DEFAULT_THEME;
			}
		}

		// get the javascript path
		// The theme may borrow the default one
		// (For variations, this comes from the master theme)
		if (file_exists (FP_THEMES_DIR."/".$themePathName."/".FP_JSDIR)) {
			$jspath = FP_THEMES_DIR."/".$themePathName."/".FP_JSDIR;
		} else {
			$jspath = FP_THEMES_DIR."/".DEFAULT_THEME."/".FP_JSDIR;
		}

		// get the main javascript library path/name (the fp_javascript_lib.js file)
		// (For variations, this comes from the master theme)
		if (file_exists ($jspath ."/".FP_THEME_JS_LIB_NAME)) {
			$jslibfile = $jspath ."/".FP_THEME_JS_LIB_NAME;
		} else {
			$jslibfile = $jspath ."/".FP_THEME_JS_LIB_NAME;
		}

		// get the main javascript path/name (the fp_javascript.js file)
		// (For variations, this comes from the master theme)
		if (file_exists ($jspath ."/".FP_THEME_JS_NAME)) {
			$jsfile = $jspath ."/".FP_THEME_JS_NAME;
		} else {
			$jsfile = $jspath ."/".FP_THEME_JS_NAME;
		}



		// Get the CSS path/name
		// Main CSS comes from the theme (also for variations)
		if (file_exists (FP_THEMES_DIR."/".$themePathName."/".FP_CSSDIR."/".FP_THEME_CSS_NAME)) {
			$csspath = FP_THEMES_DIR."/".$themePathName."/".FP_CSSDIR."/".FP_THEME_CSS_NAME;
		} else {
			$csspath = FP_THEMES_DIR."/".DEFAULT_THEME."/".FP_CSSDIR."/".FP_THEME_CSS_NAME;
		}

		// Extra CSS path
		if (file_exists (FP_THEMES_DIR."/".$themePathName."/".FP_CSSDIR."/".FP_THEME_EXTRA_CSS_NAME)) {
			$csspath2 = FP_THEMES_DIR."/".$themePathName."/".FP_CSSDIR."/".FP_THEME_EXTRA_CSS_NAME;
		} else {
			//$csspath2 = FP_THEMES_DIR."/".DEFAULT_THEME."/".FP_CSSDIR."/".FP_THEME_EXTRA_CSS_NAME;
			$csspath2 = "";
		}
		
		$previewpath = FP_THEMES_DIR."/".$themePathName."/".FP_PREVIEWDIR;
		if (!file_exists (FP_THEMES_DIR."/".$themePathName."/".FP_PREVIEWDIR)) {
			$previewpath = "";
		}

		// Newsletter additional modification CSS path
		$csspath5 = "";
// 	if (file_exists (FP_THEMES_DIR."/".$themePathName."/".FP_CSSDIR."/".FP_THEME_NEWSLETTER_CSS_NAME)) {
// 		$csspath5 = FP_THEMES_DIR."/".$themePathName."/".FP_CSSDIR."/".FP_THEME_NEWSLETTER_CSS_NAME;
// 	} else {
// 		$csspath5 = FP_THEMES_DIR."/".DEFAULT_THEME."/".FP_CSSDIR."/".FP_THEME_NEWSLETTER_CSS_NAME;
// 	}

		$DEBUG && $msg .= __FUNCTION__.": Theme Style: CSS=$csspath<BR>";
		$DEBUG && $msg .= __FUNCTION__.": Extra Styling: extraCSS=$csspath2<BR>";
		$DEBUG && $msg .= __FUNCTION__.": Variation: variationCSS=$variationCSS<BR>";
		$DEBUG && $msg .= __FUNCTION__.": Gallery Style: CSS4=$csspath4<BR>";
//	$DEBUG && $msg .= __FUNCTION__.": CSS5=$csspath5<BR>";
		$DEBUG && $msg .= __FUNCTION__.": JS=$jspath<BR>";
		$DEBUG && $msg .= __FUNCTION__.": theme=$themePathName<hr><BR>";


		$result = array (
				"css"			=> $csspath,
				"extraCSS"		=> $csspath2,
				"variationCSS"		=> $variationCSS,
				"adminVariationCSS"	=> $adminVariationCSS,
				"userVariationCSS"	=> $userVariationCSS,
				"systemVariationCSS"	=> $systemVariationCSS,
				"newsletterCSS"		=> $csspath5,
				"altCSS"		=> $variationpath,
				"jspath"		=> $jspath,
				"jslibfile"		=> $jslibfile,
				"jsfile"		=> $jsfile,
				"theme"			=> $themePathName,
				"preview"		=> $previewpath
		);
		
		$DEBUG && fp_error_log(__FUNCTION__.": ".print_r($result,true), 3, FP_ACTIVITY_LOG);
		
		return $result;
	}


// Given stylesheet name(s), return HTML to call it
	function StyleSheetHTML ($sheetnames = "") {
		if (!$sheetnames)
			return "";

		is_array ($sheetnames) || $sheetnames = array ($sheetnames);
		$output = "";
		foreach ($sheetnames as $s) {
			$output .= "<link rel=\"Stylesheet\" rev=\"Stylesheet\" href=\"$s\"  type=\"text/css\">\n";
		}
		return $output;
	}

// Clean up a user provided variation name
	function CleanThemeName ($name) {
		$name = mb_convert_case ($name, MB_CASE_LOWER);
		$name = preg_replace ("/\s+/", " ", $name);
		$name = substr(trim(preg_replace ("/[^a-zA-Z0-9 ]+/", " ", $name)), 0,31);
		$name = preg_replace ("/\s+/", " ", $name);
		return $name;
	}

// Get $_REQUEST array from user, i.e. URL input, and CLEAN IT!
	function CleanRequest () {
		$vars = array();
		while( list($var, $value) = each($_REQUEST) ) {
			$vars[CleanUserInput($var)] = CleanUserInput($value);
		}
		return $vars;
	}


// ===== end functions

}	// end class


?>