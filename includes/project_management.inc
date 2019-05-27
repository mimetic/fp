<?PHP
/* 
These functions are for managing projects, sys admin, basic functions, not the kind of thing to modify, etc.
*/

// MP3 utils. Used to read the duration of an MP3, to set the slide duration in a slide show
require_once ("mp3_id3_utils.php");

// MP3 utilities. Much better than the one above.
require_once('getid3/getid3.php');


// ------------------------
/* perform an action on a database table */
// This is the main admin event handler, for insert/delete/edit of the tables.
// 7/18/05 We are now using sessions. I'm adapting code from latest version
// of RoboAgent.
function HandleAction($actions, &$vars) {
	global $Themes;
	global $REMOTE_ADDR;
	global $BASEDIR;
	global $PHOTOS_GALLERY, $SLIDES, $THUMBNAILS, $MATTED, $ORIGINALS;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	global $items_per_page;	// a global that is modified by the arrange pictures form
	global $msg, $error;
	global $LINK;

	$DEBUG = 0;

	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ":BEGIN<BR>";
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": <BR>HandleAction:Actions<BR> " . ArrayToTable($actions);
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": <BR>HandleAction:Vars<BR> " . ArrayToTable($vars);

	// Assign variables for inputted values, e.g. $ProjectID, etc.
	// This is dangerous! We can set $fp_user, and all kinds of nasty stuff?!?!?
	// Better is to know what we want.
	foreach (array_keys ($vars) as $k) {
		$$k = $vars[$k];
	}

	$action = trim(strtolower($actions['action']));
	$table = trim($actions['table']);
	
	$queryset || $queryset = $table;

	// Get admin user info
	$fp_admin = new FPUser ($LINK, FP_ADMINISTRATOR);

	// Set vars which are checkboxes to their real values
	$vars = FixCheckBoxFields ($vars);

	$page = "";
	isset($ArtistID) || $ArtistID = "";
	isset($ProjectID) || $ProjectID = "";

	// Get the current user ID. If not set, there's a problem and we log out.
	isset($_SESSION['fp_user']) ? $fp_user = $_SESSION['fp_user'] : $fp_user = null;
	!$fp_user && $action = "logout";
	
	if (!$action) {
		$action = "show_list";
		$_SESSION['PageID'] = "tab3";
	}

	$DEBUG && var_dump ($_SESSION);
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": HandleAction: action = ".$action."<BR>";
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": HandleAction: table = ".$table."<BR>";
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": HandleAction: ArtistID = ".$ArtistID."<BR>";
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": HandleAction: ProjectID = ".$ProjectID."<BR>";
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": HandleAction: FP User = ".$fp_user."<BR>";
	$DEBUG && PrintArray ($vars);


	// =============== MAINTENANCE =====================

	// If this is a project record, then check if pictures to delete
	// and delete them.
	if ($table == DB_PROJECTS)
		DeletePicturesIfChecked ($vars);

	$accesslevel = FetchAccessLevel ($fp_user);

	// To create a new record, do an update with no ID, or an insert
	if (!$vars['ID']) {
		$action == "update_close" && $action = "insert_close";
		$action == "update" && $action = "insert";
	}


	switch ($action) {
		case "duplicate" :
			EditTable($action, $table, $vars['ID'], $vars);
			$nextaction = "show_list";
			break;
			
		case "choose_vocab_user" :
			$msg .= "User is $fp_user";
			if ($fp_user == FP_ADMINISTRATOR) {
				$ArtistList = OptionList (DB_ARTISTS, "vocabUserID", "ID", "CONCAT_WS(', ', Lastname, Firstname) AS Fullname", null, null, "Lastname", OPTION_LIST_IS_POPUP);
			} else {
				$ArtistList = str_replace('{ID}', $fp_user, FetchSnippet ("vocab_current_user_chooser"));
			}
			// Show vocab chooser form
			$fields = array (
				"chooser" => $ArtistList
			);
			$page = FetchSnippet ("vocab_choose_user_form");
			$page = Substitutions ($page, $fields);

			$nextaction = "show_vocab_list";

			break;

		case "show_vocab_list" :
			$msg .= $action;
			$vuser = $vars['vocabUserID'];
			$vocab = new FPVocabulary($vuser);
			$page = $vocab->EditPage();
			break;

		case "update_vocab" :
			$msg .= "Update Vocabulary";
			$vlist = $vars['vocablist'];
			$vuser = $vars['vocabUserID'];
			$vocab = new FPVocabulary($vuser);
			$vocab->UpdateList($vlist, $vuser);
			$nextaction = "choose_vocab_user";
			break;


		case "help" :
			isset($vars['subset']) ? $subset = "_" . $vars['subset'] : $subset = '';
			$page = FetchSnippet ("help{$subset}_{$accesslevel}");
			$page || $page = FetchSnippet ("help{$subset}");
			$page || $page = FetchSnippet ("help");

			$v = array(	"msg"	=> $msg,
				"error" => $error);
			$page = Substitutions ($page, $v);
			$_SESSION['PageID'] = "tab1";
			break;

		case "activate" :
			$hash = $vars['hash'];
			$user = new FPUser ($LINK);
			$activation = $user->activateByHash ($hash);
			!$user->error
				? $msg .= "You have successfully activated your account.<br>"
				: $error .= "Error: {$user->error}<br>";
			$page = FetchSnippet ("login");
			break;

		case "login"  :
			//$page = ReplaceSysVars (FetchSnippet ("loginOK"), '', $fp_user, "", "show_list");
			$page = FetchSnippet ("loginOK");
			break;

		case "logout" :
			$page = FetchSnippet ("login");
			$pageid = "nomenu";
			$vars['PageID'] = $pageid;
			break;

		case "insert_close" :
			$nextaction = "show_list";
			$action = "insert";
		case "insert" :
			$ArtistID || $ArtistID = $fp_user;
			switch ($table) {
				case $PRICESETS :
					$vars = SortAndCleanPriceSetRows($vars);
					$vars = CompactArraysToStrings ($vars);
					$newID = EditTable ($action, $table, "", $vars);
					$vars['ID'] = $newID;
					break;
				case $GROUPS :
				//be sure the owner gets a "part", is part of group!
					if ($ArtistID) {
						$newID = EditTable ($action, $table, "", $vars);
						$groupID = $newID;
						AddArtistToGroup ($ArtistID, $groupID);
						$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": Added Artist $ArtistID to Group $newID.<BR>";
						$vars['ID'] = $newID;
					}
					break;
				case $PROJECTS :
					$artist = FetchArtist ($ArtistID);
					//be sure the owner gets a "part", is part of project!
					if ($ArtistID) {
						// Set the group ID to the default group for the artist if not set.
						$vars['GroupID'] || ($vars['GroupID'] = $artist['GroupID']);
						
						// be sure the start date is set
						$vars['ProjectDate'] = date("Y-m-d");
						// if there's uploaded slideshow audio, add it in



						$newID = EditTable ($action, $table, "", $vars);
						$ProjectID = $newID;

						if (empty($vars['Nickname']))
							$vars['Nickname'] = trim($vars['Title']);
						// nickname must be unique!
						$vars['Nickname'] = UniqueProjectNickname ($ProjectID, $vars['Nickname']);
						EditTable ('update', DB_PROJECTS, $ProjectID, array("Nickname"=>$vars['Nickname']));

						AddArtistToProject ($ArtistID, $ProjectID);
						$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": Added Artist $ArtistID to Project $newID.<BR>";

						$vars['ID'] = $newID;
						
						/*
						foreach (array_keys ($vars) as $k) {
							$$k = $vars[$k];
						}
						*/

						/*
						$audio = ProcessUploadedSlideshowAudio ($fp_user, $newID, "userfile");
						if ($audio) {
							$vars["Audio"] = $audio;
						}
						*/
						/*
						$audioLength = GetSlideShowDuration ($newID);
						if ($audioLength) {
							$vars['SlideShowDuration'] || $vars['SlideShowDuration'] = $audioLength;
						}
						*/
					}
					break;
				case $ARTISTS :
				// If a new artist is created, he gets an "Artist's Gallery" project
				// Add the portrait after.
					$myUser = new FPUser($LINK);
					$newID = $myUser->newUser($vars, FP_NEW_ARTIST_GALLERY);
					if ($newID != FP_ADMINISTRATOR) {
						// Add artist to current group
						$groupID = $vars['GroupID'];
						$myGroup = new FPGroup ($LINK, $groupID);
						$myGroup->AddMembersToGroup ($newID, $groupID);
					}
					$vars['ID'] = $newID;
					break;
				case $STORIES :
					$newID = EditTable ($action, $table, "", $vars);
					AddStoryToProject ($newID, $ArtistID, $ProjectID);
					$vars['ID'] = $newID;
					break;
				default :
					$newID = EditTable ($action, $table, "", $vars);
					$vars['ID'] = $newID;
			}
			$ID = $newID;
			MarkProjectUpdated ($ProjectID);
			Maintenance();
			$page = BuildTablePage ($queryset, $table, $ProjectID, $fp_user);
			$nextaction || $nextaction = "display";
			break;

		case "delete" :
			$ID = $vars['ID'];

			EditTable ($action, $table, $ID, $vars);
			Maintenance();
			$page = BuildTablePage ($queryset, $table, $ProjectID, $fp_user);
			break;
		// multiedit might return a page, for FTP sending, for example.
		// usually it doesn't and we just go back to the listing
		case "multiedit" :
			$page = "";
			$page = MultiEdit ($table, $vars);
			Maintenance();
			if ($page) {
				$page = Substitutions ($page, array ('table' => $table,
					'queryset' => $queryset,
					'fp_user' => $fp_user
				));
			} else {
				$page = BuildTablePage ($queryset, $table, $ProjectID, $fp_user);
			}
			break;
		case "update_close" :
			$nextaction = "show_list";
			$action = "update";
		case "update" :
			$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": Begin $action for table $table<BR>";
			$ID = $vars['ID'];
			
			if ($fp_user != FP_ADMINISTRATOR)
				{
				// Does this user have the permission to modify this record?
				// User's can mod records they own.
				$m = __FUNCTION__.": Permissions testing:";
				$m .= "User: #{$fp_user}; ";
				$m .= "table: $table, id=$ID";
				$record = FetchRowsByValue ($table, "*", "ID", $ID, null, true);
				$record['ArtistID'] && $m .= "ArtistID = {$record['ArtistID']}<BR>";
				//var_dump($record);
	
				//$msg .= $m;
				}
			
			switch ($table) {
				// Update Pricesets
				case $PRICESETS :
					$DEBUG && $msg .= __FUNCTION__.__LINE__.": Update $PRICESETS<BR>";
					$vars = SortAndCleanPriceSetRows($vars);
					$vars = CompactArraysToStrings ($vars);
					EditTable ($action, $table, $ID, $vars);
					break;

				// Update Artists
				case $ARTISTS	:
					$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": $action $ARTISTS<BR>";
					$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": ".ArrayToTable ($vars);
					// Get an uploaded portrait if it exists
					$tag = ProcessUploadedSingleImage ($ID, "userfile");
					// set portrait to show uploaded file, even if set to show something else
					$tag == 'artist_portrait' && $vars['PortraitImageID']='0';

					// If flag set to clear uploaded portrait, then clear it
					if ($vars['PortraitImageID'] == 'none') {
						$vars['PortraitImageID'] = 0;
						$error .= __FUNCTION__."NONE";
						// delete existing files
						$n = "artist_portrait_".$fp_user.'.jpg';
						DeleteUserPicture ($n);
					}

					// Storage: Don't let them set the maximum for a user higher than the admin user's allocation
					list ($allocated, $total) = GetStorageStats($vars['ID']);
					if (($total>0) && $vars['Storage'] > $total - $allocated )
						$vars['Storage'] = $total - $allocated;

					EditTable ($action, $table, $ID, $vars);
					/*
					if (($fp_user == FP_ADMINISTRATOR) or ($accesslevel == FP_ADMINISTRATOR_LEVEL and $record['ArtistID'] == $fp_user)) {
						$nextaction = "show_list";
					} else {
						$nextaction = "display";
					}
					*/
					break;

				// Update Groups
				case $GROUPS :
					$myGroup = new FPGroup ($LINK, $ID );

					$arr = array ();
					isset($vars['artistlist']) || $vars['artistlist'] = $arr;

					switch ($vars['subset']) {
						case DB_ARTISTS :
						// change owner if necessary
						//$myGroup->SetOwner ($vars['ArtistID']);
							isset($vars['artistlist']) || $vars['artistlist'] = array();
							$myGroup->UpdateMembers ($vars);
							$myGroup->UpdateInfo ($vars);
							//$nextaction = "show_list";
							//$nextaction = "display";
							break;
						case "Logo" :
							ProcessUploadedSingleImage ($fp_user, "userfile");
							if (isset($vars['clear_uploaded_logo'])) {
								$vars['Logo'] = "";
								$myGroup->ClearLogo();
								//$error .= __FUNCTION__.__LINE__.": delete logo<BR>";
							}
							$myGroup->SetLogo ($vars['Logo']);
							$myGroup->UpdateInfo ($vars);
							//$nextaction = "display";
							break;

						case "Icon" :
							ProcessUploadedSingleImage ($fp_user, "userfile");
							if (isset($vars['clear_uploaded_icon'])) {
								$vars['Icon'] = "";
								$myGroup->ClearIcon();
							}
							// If "Icon" is set, it will be a number that is
							// the ID of an image.
							$myGroup->SetIcon ($vars['Icon']);
							$myGroup->UpdateInfo ($vars);
							//$nextaction = "display";
							break;

						default :
							$myGroup->UpdateInfo ($vars);
							//EditTable ($action, $table, $ID, $vars);
							break;
					}

					MarkProjectUpdated ($ID);
					break;

				// Update Projects
				case $PROJECTS :
					$project = new FPProject ($LINK, $ID);

					if (empty($vars['Title']) && isset($vars['Title']))
						$vars['Title'] = "Untitled Project";
					// Update project main information

					if (empty($vars['Nickname']) && isset($vars['Nickname']))
						$vars['Nickname'] = trim($vars['Title']);
					// nickname must be unique!
					$vars['Nickname'] = UniqueProjectNickname ($ID, $vars['Nickname']);

					// If there is a 'subset' parameter set, handle the appropriate
					// data for that form.

					// Validate entries for Projects
					$vars = ValidateProjectValues ($vars);

					/*
					$newaudio = false;
					if (isset($_REQUEST['userfileTag'])) {
						$tag = trim($_REQUEST['userfileTag']);
						switch ($tag) {
							case 'project_image' :
								ProcessUploadedImages ($ID, $fp_user);
								break;
							case 'project_audio' :
								// Process uploaded MP3 file if it exists
								$newaudio = ProcessUploadedSlideshowAudio ($fp_user, $ID, "userfile");
								break;
						}
					}

					$newaudio && $vars["Audio"] = $newaudio;
					// Set duration if new audio or no entry
					$audio = GetProjectSlideshowAudio ($ID);
					*/

					$audioFilename = $vars['Audio'];
					$audiochanged = ($audioFilename != $project->audio);
					// If audio has changed and there is no manual entry, then calc new time
					if ($audiochanged || $vars['SlideShowDuration'] == 0) {
						$audioPath = GetSlideshowAudioPath ($ID);
						$audioLength = GetAudioFileDuration ("$audioPath/$audioFilename");
						$vars['SlideShowDuration'] = $audioLength;
					} else {
						$audioLength = $vars['SlideShowDuration'];
					}

					// Clear audio if command given
					/*
					if ($vars["clearaudio"]) {
						DeleteSlideshowAudioFile ($vars['ID']);
						$vars["Audio"] = "";
					}
					*/
					// Update subsets of Projects
					switch ($vars['subset']) {
						case "Pictures" :
							$project->UpdateImageParts ($vars);
							$nextaction = "display";
							break;
						case "Sorter" :
							$project->UpdateImageParts ($vars);
							$nextaction = "display";
							break;
						case "PictureOrder" :
							UpdateImageOrder ($ID, $vars);
							// items shown per page can be set on the page
							isset($vars['items_per_page']) && $items_per_page = $vars['items_per_page'];
							//$nextaction = "display";
							break;
						case "PictureChooser" :
						//UpdateImageOrder ($ID, $vars);
						// items shown per page can be set on the page
							isset($vars['items_per_page']) && $items_per_page = $vars['items_per_page'];
							//$nextaction = "display";
							break;
						case "Stories" :
							UpdateStoryParts ($ID, $vars);
							//$nextaction = "display";
							break;
						case DB_ARTISTS :
							$DEBUG && $error .= __FUNCTION__.__LINE__.": ".ArrayToTable ($vars);
							UpdateArtistParts ($ID, $vars, $PROJECTS);
							EditTable ($action, $table, $ID, $vars);
							// If audio uploaded, return to page (don't return to list)
							// $newaudio ? $nextaction = "display" :	$nextaction = "show_list";
							break;
						default :
							EditTable ($action, $table, $ID, $vars);
							//$nextaction = "show_list";
							break;
					}


					MarkProjectUpdated ($ID);
					ClearProjectCache($ID);
					break;
					
				// Update Suppliers
				case $SUPPLIERS :
					$vars = ValidateSupplierValues($vars);
					EditTable ($action, $table, $ID, $vars);
					ClearProjectCache($ID);
					break;
					
				// Update Stories
				case $STORIES :
				// maybe this should find projects its part of and clear their caches?
					EditTable ($action, $table, $ID, $vars);
					ClearStoryCache($ID);
					break;

				// Update Images
				case $IMAGES :
				// If we change the owner, we must update all Parts, too..
				// This means adding new owner to project, and removing old one if the image was the only image.
				// TO DO...

					EditTable ($action, $table, $ID, $vars);
					if ($vars['LockPriceSet'])
						CreateUniqueImagePriceSet ($ID);
					ClearProjectCache($ID);
					break;

				// Update all others
				default:
					EditTable ($action, $table, $ID, $vars);
					($table == DB_PROJECTS) && ClearProjectCache($ID);
					break;
					break;
			}

			$nextaction || $nextaction = "display";
			//$page = BuildTablePage ($queryset, $table, $ID, $fp_user);

			break;

		case "add_artist_to_project" :
			AddArtistToProject ($ArtistID,$ProjectID);
			Maintenance();
			$page = BuildTablePage ($queryset, $table, $ProjectID, $fp_user);
			break;
		/*
		case "enter_new_row" :
			$page = NewRowForm ($table, $vars);
			break;
		*/
		case "show_list" :
			if (!$table) {
				($accesslevel == FP_SUPPLIER_LEVEL) ? $table = $SUPPLIERS : $table = DB_PROJECTS;
			}
			$page = BuildTablePage ($queryset, $table, $ProjectID, $fp_user, $limit);
			break;

		case "ftp" :
		//$DEBUG = TRUE;
			if (!isset($vars['cancel'])) {
				$SOURCEDIR = $ORIGINALS;
				foreach (array_keys ($vars) as $k) {
					$$k = $vars[$k];
				}

				$DEBUG && $msg .= ArrayToTable ($vars);

				$SOURCEDIR = $$SOURCEDIR;	//form provides which global var to access, not the datum
				$images = array();
				// Get custom file names
				for ($i = 0; $i < count($filename); $i++) {
					if (isset($filenameX[$i])) {
						$images[$ID[$i]] = $filenameX[$i];
					} else {
						$images[$ID[$i]] = $filename[$i];
					}
				}

				$DEBUG && $msg .= ArrayToTable ($images);

				$IDlist = explode (",", $IDlist);	//get the ID's into an array from the passed variable
				$msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": FTP Attempted.<BR>";
				// $ID is the array of IDs
				// $filenames is an array (ID=>filename)
				$msg .= FTPImages ($ID, $images, $ftp_server, $directory, $newfolder, $ftp_user_name, $ftp_user_pass, $SOURCEDIR);
				$msg .= OpenMessageWindow ($msg, "FTP RESULTS");
			}
			$page = BuildTablePage ($queryset, $table, $ProjectID, $fp_user, $msg, $limit);
			break;
		case "confirm" :
			if (! UserHasAccess ($table, $vars['ID'])) {
				$error .= FetchSnippet ("noaccess");
				$actions['action'] = "show_list";
				$page = HandleAction ($actions, $vars);
			} else {
				$extra = $vars['desc'];
				$page = ConfirmWindow ($table, $vars['ID'], $vars['command'], $extra);
			}
			break;
		case "display" :
		// this is trapped below this switch statement
			$nextaction = "display";
			break;
		case "themeeditor" :
			$Themes->userID = $fp_user;
			$Themes->BuildEditorPage($vars['subset'], $vars['ThemeID']);
			$page = $Themes->page;
			break;
		case "files" :
			$page = DisplayRecordForEdit (null, $userID, $accesslevel, $ID, "files", $showthumbstatus, null);
			$_SESSION['PageID'] = "tab13";
			break;
		
		default :
			$error .= "Unknown action : $action<BR>";
	}


	// By doing display this way, we can reset the action in the switch statement above,
	// and follow any action with a display action.
	$accesslevel = FetchAccessLevel ($fp_user);
	switch ($nextaction) {
		case "display" :
			Maintenance();
			$ID || $ID = $vars['ID'];
			//$page = EditRowForm ($table, $vars);
			//$page = DisplayRecordForEdit ($table, $vars);
			
			// Does this user have access to this record?
			// For user records, let's be careful!
			if ($table == DB_ARTISTS && $fp_user != FP_ADMINISTRATOR)
				$ID = $fp_user;
			$userID = $fp_user;
			! empty($ID) ? $record = GetRecord ($table, $ID) : $record = array();
			isset ($vars['subset']) ? $subset = $vars['subset'] : $subset = "";
			isset ($vars['showthumbstatus']) ? $showthumbstatus = $vars['showthumbstatus'] : $showthumbstatus = "";
			isset ($vars['start']) ? $start = $vars['start'] : $start = 0;	// picture number of first pic to show, e.g. 10-15 then start=10
			$page = DisplayRecordForEdit ($table, $userID, $accesslevel, $ID, $subset, $showthumbstatus, $start);
			break;
		case "show_list" :
			if (!isset($table)) {
				($accesslevel == FP_SUPPLIER_LEVEL) && $table = DB_SUPPLIERS;
				$table = DB_PROJECTS;
			}
			$page = BuildTablePage ($queryset, $table, $ProjectID, $fp_user, $limit);
			break;
	}
	isset($vars['PageID']) && SetPageCSSID ($vars['PageID']);
	return $page;
}


function SetPageCSSID ($pageid) {
	$pageid || $pageid = "tab";
	$_SESSION['PageID'] = $pageid;
}


// Display Shows a record for editing
// subset is used to show pictures in the Projects table, instead of the 
// default, which is the project data.
// If the incomcing $ID is blank (or 0) then we should show a blank record, ready for user input.
function DisplayRecordForEdit ($table, $userID, $accesslevel, $ID, $subset="", $showthumbstatus = "Hide", $start=1) {
	global $Themes;
	global $IMAGES, $PRICES, $PRICESETS, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES, $SUPPLIERS;
	global $PHOTOS_GALLERY, $THUMBNAILS, $MATTED, $ORIGINALS;
	global $BASEDIR, $MAILED_DIR, $LOGS;
	global $UPLOADER_ENDPAGE, $UPLOADER_DEST;
	global $SYSTEM_FRAMESTYLES, $FRAMESTYLESWIDTHS;
	global $ECOMMERCE, $MAX_LIMITED_EDITION_SETS;
	global $FP_NAVBAR_ITEM_NAMES, $FP_NAVBAR_POPUP_MENU;
	global $PP_PACKAGES;
	global $FP_ACCESS_LEVELS, $FP_GALLERY_TYPES;
	global $USER_AUDIO_EXTENSIONS, $USER_VIDEO_EXTENSIONS, $USER_IMAGE_EXTENSIONS;
	global $user;
	global $msg, $error;
	global $LINK;

	$DEBUG = 0;
	IF ($DEBUG) {
		$msg .= __FUNCTION__.":".__LINE__.": DEBUG IS ON.<BR>";
	}

	isset($_SESSION['fp_user']) ? $fp_user = $_SESSION['fp_user'] : $fp_user = FP_ADMINISTRATOR;

	$fp_admin = new FPUser ($LINK, FP_ADMINISTRATOR);

	$showthumbstatus || $showthumbstatus = "Hide";

	// Default for showing public/private checkbox
	$showpp = true;

	$submit = FetchSnippet ("input_form_submit");	//default submit/reset buttons

	// local variable, used to fill out the forms
	$currvals = array();

	// Set default Session var for the WTF flash uploader
	// When uploading to projects, the ProjectID will be added as a directory
	$_SESSION['TFU_DIR'] = "$BASEDIR/$MAILED_DIR";

	IF ($DEBUG) {
		$msg .= __FUNCTION__.":".__LINE__.":". $_SESSION['TFU_DIR']."<BR>";
		$msg .= __FUNCTION__.":".__LINE__.": SID = ". SID."<BR>";
		$msg .= __FUNCTION__.":".__LINE__.": Session name = ". session_name()."<BR>";
	}

	list($action,$record,$params) = GetRecordForEditForm($ID, $table);

	// could be a bad request
	if (!$action) {
		$error .= "The requested record (#{$ID}) does not exist!<BR>";
		fp_error_log(__FUNCTION__.": The requested record (#{$ID}) does not exist!", 3, FP_ERROR_LOG);
		return false;
	}
	
	$ownername = "";

	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.":" . ": DisplayRecordForEdit<BR>";
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.":" . '(table, record, userID, accesslevel, ID, subset, showthumbstatus)<BR>';
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.":" . ": (table=$table, userID=$userID, Accesslevel=$accesslevel, ID=$ID, subset=$subset, showthumbs=$showthumbstatus)<BR>";
	//$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.":" . ArrayToTable ($record);
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.":" . ArrayToTable ($_SESSION);

	switch ($subset) {
		case "Pictures" :
			$form = ManagementFormPictures ($ID, $showthumbstatus, "", "thumbnail", $start);
			break;
		case "PictureOrder" :
			$form = ManagementFormOrderPictures ($ID, $showthumbstatus, "", "thumbnail_order", $start);
			break;
		case "PictureChooser" :
			$form = ManagementFormChoosePictures ($ID, $showthumbstatus, "", "thumbnail_order", $start);
			break;
		case "Stories" :
			$form = ManagementFormStories ($ID);
			break;
		case "ImageSorter" :
			$form = ManagementFormImageSorter ($ID);
			break;
		default :
			($table == DB_ARTISTS && $record['ID'] == FP_ADMINISTRATOR) ? $ext = "_administrator" : $ext = "";
			$form = FetchSnippet ("input_form_{$table}{$ext}");
			break;
	}

	$form = ReplaceSnippets ($form);

	$userID ? $artistID = $userID : $artistID = $fp_user;
	isset($record['ArtistID']) && $artistID = $record['ArtistID'];
	$values = FetchArtistImageTitles ($artistID);

	// Get project list for this artist
	// Admin can see all projects.
	$checked = array ($record['ProjectID']);
	$ProjectList = OptionList ($PROJECTS, "ProjectID", "ID", "Title", "ArtistID = '$artistID'", $checked, "Title", TRUE, $extrahtml="");


	// Build Projects pop-up list for this artist
	// If admin, can see all projects in system
	isset ($record['ArtistID']) ? $artistID = $record['ArtistID'] : $artistID = $fp_user;
	if (($fp_user == FP_ADMINISTRATOR) or ($accesslevel < FP_NORMAL_LEVEL and $record['ArtistID'] == $fp_user)) {
		// all projects, for admin
		if ($artistID) {
			$checked = array ($record['ProjectID']);
			$ProjectList = OptionList ($PROJECTS, "ProjectID", "ID", "Title", "", $checked, "Title", false, $extrahtml="");
		} else {
			$ProjectList = "Missing Artist?!?!";
		}
	} else {
		// Projects for normal artist
		$checked = array ($record['ProjectID']);
		$ProjectList = OptionList ($PROJECTS, "ProjectID", "ID", "Title", "ArtistID = $fp_user", $checked, "Title", false, $extrahtml="");
	}


	// Fill in all params. These may be overwritten, e.g. checkboxes
	foreach ($params as $k => $v) {
		$currvals['param_'.$k] = GetParam ($params, $k);
	}

	// Return to list link. The pageid must be set by each kind of page
	$returntolist = FetchSnippet ("link_return_to_list");	// code linking to list of records


	switch ($table) {
// *********************** ARTISTS
		case $ARTISTS :
			$user->CheckStorageSpace();
			// STORAGE
			// Get space already given out and total space on system (based on Admin User's allocation)
			list ($allocated, $total) = GetStorageStats ($record['ID']);
			if ($total > 0) {
				$currvals['storage_available'] = $total-$allocated;
			} else {
				// unlimited available
				$currvals['storage_available'] = "{FP:unlimited}";
			}

			// If config.inc: FP_ADMIN_CONTROLS_ARTIST_SALES is true, then
			// we don't show artist's commerce settings except to the admin user
			if (FP_ADMIN_CONTROLS_ARTIST_SALES) {
				if ($fp_user == FP_ADMINISTRATOR) {
					$f["ArtistSalesForm"] = FetchSnippet ("input_form_Artists_artistsalesform");
				} else {
					$f["ArtistSalesForm"] = "<!-- only admin can change an artist's sales settings -->";
				}
			} else {
				$f["ArtistSalesForm"] = FetchSnippet ("input_form_Artists_artistsalesform");
			}


			// If no value given, then the value will be "unlimited"
			// show the total amount one could allocate
			// $record['Storage'] || $record['Storage'] = $currvals['storage_available'];

			// load last calculation of storage used by this user
			$currvals['storage_used'] = $user->StorageUsed;
			intval($record['Storage']) > 0
				? $currvals['storage_remaining'] = $record['Storage'] - $user->StorageUsed
				: $currvals['storage_remaining'] = $record['Storage'];


			$currvals['param_'.FP_PARAM_ARTIST_BLOG] = GetParam ($params, FP_PARAM_ARTIST_BLOG);

			// *** these are not currently in use ***
			$currvals['param_'.FP_PARAM_ARTIST_LINKS] = GetParam ($params, FP_PARAM_ARTIST_LINKS);
			$currvals['param_'.FP_PARAM_ARTIST_LINKNAMES] = GetParam ($params, FP_PARAM_ARTIST_LINKNAMES);

			// LINK TO GALLERIES?
			// Default to true
			$x = GetParam ($params, FP_PARAM_ARTIST_GALLERY_LINK);
			if ($x != '0') {
				$params = SetParam ($params, FP_PARAM_ARTIST_GALLERY_LINK, 1);
			}
			$currvals['param_'.FP_PARAM_ARTIST_GALLERY_LINK."_CHECKBOX"] = YesNo ('param_'.FP_PARAM_ARTIST_GALLERY_LINK, GetParam ($params, FP_PARAM_ARTIST_GALLERY_LINK));

			$artistID = $record['ID'];	// A user always owns his own record
			//$FP_ACCESS_LEVELS;

			// Access Level menu
			$checked = array ($record['AccessLevel']);
			$currvals['AccessLevelMenu'] = OptionListFromArray (	$FP_ACCESS_LEVELS,
				"AccessLevel",
				$checked,
				TRUE,
				OPTION_LIST_IS_POPUP,
				$extrahtml=""
			);

			// User Level/Type Menu
			$checked = array ($record['UserLevel']);
			$currvals['UserLevelMenu'] = OptionListFromArray (
				$FP_GALLERY_TYPES,
				"UserLevel",
				$checked,
				TRUE,
				OPTION_LIST_IS_POPUP,
				$extrahtml=""
			);

			// Generate PP secret code so user can upgrade account
			$currvals['PP_SECRET_CODE'] = Generate_PP_Secret ($artistID);


			// SUBSCRIPTION BLOCK
			if ($fp_user == $artistID && ShowSubscriptionLink($fp_user)) {
				$subscription_block = FetchSnippet ("subscribe");
				$record['SubscriptionDesc'] || $record['SubscriptionDesc'] = "{fp:Trial Account}";
				$currvals['SubscriptionDesc'] = convertDollarToHTML(htmlentities($record['SubscriptionDesc']));
			} else {
				$subscription_block = "<!-- no subscription needed -->";
			}
			$subscription_block = Substitutions ($subscription_block, $currvals);
			$currvals['SUBSCRIBE'] = Substitutions ($subscription_block, $record);


			// Confirmed user?
			// If it's a blank entry, auto confirm the user because user was created by sysadmin!
			if (!$record['ID'])
				$record['Confirmed'] = 1;
			if ($fp_user == FP_ADMINISTRATOR) {

				$currvals['ShowConfirmedEntry'] = "table-row";
				$currvals['Confirmed_CHECKBOX'] = YesNo ('Confirmed', $record['Confirmed']);
			} else {
				$currvals['ShowConfirmedEntry'] = "none";
			}
			$record['Confirmed']
				? $record['Confirmed'] = ""
				: $record['Confirmed'] = " ({fp:Not Confirmed}) ";

			
			
			// Coupon codes for discounts or whatever on sales page
			$currvals['param_'.FP_PARAM_ARTIST_COUPONS] = GetParam ($params, FP_PARAM_ARTIST_COUPONS);

			// Show Accesslevel field for editing in Users if user has level FP_ADMINISTRATOR
			// otherwise, hide it.

			/*
			Editing Access Level:
			Private Installation
				Multi-gallery and accesslevel is Admin User and User is non-admin
					Can set  user's access level, else cannot set access level
					Cannot set UserLevel (user type)
			Public Installation:
				accesslevel = Admin: can set user/access
				accesslevel = other: cannot set user/access
				
				- Admin user can 
			*/

			// DEFAULTS:
			$record['AccessLevel'] || $record['AccessLevel'] = FP_PERSONAL_LEVEL;
			$record['UserLevel'] || $record['UserLevel'] = FP_SINGLE_GALLERY_SINGLE_USER;

			// ACCESSLEVEL
			$DEBUG = 0;
			$DEBUG && $error .= __FUNCTION__.__LINE__."<br>";
			if (FP_INSTALLATION_TYPE == FP_INSTALLATION_PRIVATE) {
				// Private server installation
				
				// IF config setting allows (FP_ADMIN_CHANGE_COMMISSION) then
				// Admin user can change website commission taken by the website
				// Non-admin users cannot.
				// All users can change 'commission2', the optional commission.

				if ($accesslevel == FP_ADMINISTRATOR_LEVEL && $record['ID'] != FP_ADMINISTRATOR && FP_ADMIN_CHANGE_COMMISSION) {
					$DEBUG && $error .= __FUNCTION__.__LINE__.": User is Admin : Can change user commissions<BR>";
					$f["commission"]= FetchSnippet ("EnterCommission");
					$f["commission2"]= FetchSnippet ("EnterCommission2");
				} else {
					$DEBUG && $error .= __FUNCTION__.__LINE__.": Cannot change commissions<BR>";
					$f["commission"]= FetchSnippet ("ShowCommission");
					$f["commission2"]= FetchSnippet ("EnterCommission2");
				}

				$DEBUG && $error .= __FUNCTION__.__LINE__.": Private Installation<BR>";
				if ($user->UserType == FP_MULTI_GALLERY_SINGLE_USER || $user->UserType == FP_MULTI_GALLERY_MULTI_USER) {
					$DEBUG && $error .= __FUNCTION__.__LINE__.": Installation is Private Server, multi-gallery<BR>";
					if ($accesslevel == FP_ADMINISTRATOR_LEVEL && $record['ID'] != FP_ADMINISTRATOR) {
						$DEBUG && $error .= __FUNCTION__.__LINE__.": OK, can set user's access level<BR>";
						$f["accesslevelinput"]= FetchSnippet ("accesslevelinput");
					} else {
						$DEBUG && $error .= __FUNCTION__.__LINE__.": Cannot set user's access level<BR>";
						$f["accesslevelinput"] = FetchSnippet ("accesslevelinput");
						$f['AccessLevelMenu'] = $FP_ACCESS_LEVELS[$record['AccessLevel']];
					}
					// Single Gallery
				} else {
					$DEBUG && $error .= __FUNCTION__.__LINE__.": Installation is Private Server, Single-gallery<BR>";
					$DEBUG && $error .= __FUNCTION__.__LINE__.": Cannot set user's access level<BR>";
					$f["accesslevelinput"] = FetchSnippet ("accesslevelinput");
					$f['AccessLevelMenu'] = $FP_ACCESS_LEVELS[$record['AccessLevel']];
				}
				$DEBUG && $error .= __FUNCTION__.__LINE__.": Cannot set user's type (currently = {$user->UserType})<BR>";
				$f["userlevelinput"]= FetchSnippet ("userlevelhidden");
				$f["userlevelname"]= $FP_GALLERY_TYPES[$user->UserType];

				// Public installation (accounts for sale)
			} else {
				$DEBUG && $error .= __FUNCTION__.__LINE__.": Public Installation<BR>";
				if ($accesslevel == FP_ADMINISTRATOR_LEVEL && $record['ID'] != FP_ADMINISTRATOR) {
					$DEBUG && $error .= __FUNCTION__.__LINE__.": OK, can set user's access level<BR>";
					$f["accesslevelinput"]= FetchSnippet ("accesslevelinput");
					$f["userlevelinput"]= FetchSnippet ("userlevelhidden");
					$f["userlevelname"]= $FP_GALLERY_TYPES[$user->UserType];
					$f["commission"]= FetchSnippet ("EnterCommission");
					$f["commission2"]= FetchSnippet ("EnterCommission2");
				} else {
					$DEBUG && $error .= __FUNCTION__.__LINE__.": Cannot set user's access level<BR>";
					$f["accesslevelinput"] = FetchSnippet ("accesslevelinput");
					$f['AccessLevelMenu'] = $FP_ACCESS_LEVELS[$record['AccessLevel']];
					$f["userlevelinput"]= FetchSnippet ("userlevelhidden");
					$f["userlevelname"]= $FP_GALLERY_TYPES[$user->UserType];
					$f["commission"]= FetchSnippet ("ShowCommission");
					$f["commission2"]= FetchSnippet ("EnterCommission2");
				}

			}

			if ($fp_user == FP_ADMINISTRATOR && $record['ID'] != FP_ADMINISTRATOR) {
				$f["PASSWORD_INPUT_TYPE"]= "input";
				$f["userstorageinput"]= FetchSnippet ("userstorageinput");
			} else {
				$f["PASSWORD_INPUT_TYPE"]= "password";
				$f["userstorageinput"]= FetchSnippet ("userstoragehidden");
			}

			$form = Substitutions ($form, $f);


			// If this is Private installation, then no user can set his UserLevel

			// This is the on/off for eCommerce
			$currvals['CommerceStatus_CHECKBOX'] = YesNo ('CommerceStatus', $record['CommerceStatus']);

			// Show the default group for the user.
			// If sysadmin, show a popup list. If normal user, just show the
			// user's group as text (can't be changed).
			if (($fp_user == FP_ADMINISTRATOR) or ($accesslevel < FP_NORMAL_LEVEL and $record['ArtistID'] == $fp_user)) {
				$DefaultGroupID = FetchGroupListHTML ($artistID, $record['GroupID']);
			} else {
				$group = FetchGroup ($record['GroupID']);
				$DefaultGroupID = $group['Title'];
			}


			// Choose the default project for the user from projects owned.
			$DefaultProjectID = FetchArtistProjectListHTML ($artistID, $record['ProjectID']);




			$checked = array ($record['DefaultPriceID']);
			$usedefault = empty($checked);
			$label = "concat(Price, ' ', Currency, ' : ', Paperheight, ' x ', Paperwidth, ' ', Medium) AS label";
			$PriceList = OptionList ( DB_PRICES, "DefaultPriceID", "ID", $label, "", $checked, "Price", FALSE, $extrahtml="",
				$extraline = array('value'		=> "",
				'checked'	=> $usedefault,
				'label'		=> "Use Default Pricing")
			);
			// eCommerce method
			$ecommerceList = $ECOMMERCE;
			$checked = array ($record['Ecommerce']);
			$usedefault = empty($checked);
			$ecommerceList = OptionListFromArray (	$ecommerceList,
				"Ecommerce",
				$checked,
				TRUE,
				OPTION_LIST_IS_POPUP,
				$extrahtml="",
				$extraline = array('value'		=> "0",
				'checked'	=> "",
				'label'		=> "NO SALES ALLOWED")
			);


			// Vendors popup
			$VendorList = FetchSupplierIDs();

			$checked = array ($record['Vendor']);
			$VendorList = OptionListFromArray (	$VendorList,
				"Vendor",
				$checked,
				TRUE,
				OPTION_LIST_IS_POPUP,
				$extrahtml="",
				$extraline = array('value'		=> "0",
				'checked'	=> "",
				'label'		=> "Choose a print provider")
			);


			if ($record['ID']) {
				// NOT blank record
				$PortraitIMG = FetchArtistPortrait ($record['ID'], "thumbnail", "style='border:1px solid black;'");
				$portraitEntry = FetchSnippet ("input_form_artists_portrait");

				// List of images, so artist can choose one to be his portrait
				$values = FetchArtistImageTitles ($artistID);
				$checked = array ($record['PortraitImageID']);
				$values['0'] = '(Use Uploaded Portrait)';
				natsort ($values);
				//print_r ($values);
				$ImageList = OptionListFromArray (	$values,
					"PortraitImageID",
					$checked,
					false,
					OPTION_LIST_IS_POPUP,
					$extrahtml="OnChange=\"ChoosePic('Picture', this.name, '".FP_DIR_THUMBNAILS_IMAGES."')\""
				);
			} else {
				$portraitEntry = "";
			}

			$pageTabID = "tab2";	// for return to list link
			break;

// *********************** GROUPS
		case $GROUPS :
			$DEBUG = false;
			$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.":" . ": Display: $GROUPS<BR>";
			
			$currvals['FP_PREVIEW_TYPE'] = "group";
			
			$myGroup = new FPGroup ($LINK, $ID);
			$showpp = SHOW_PUBLICPRIVATE_FOR_GROUPS;

			// Add owner's name to Group page
			if ($ID) {
				$artist = $myGroup->OwnerInfo();
				// if artist is missing (deleted?) then give this gallery to sys admin
				if (!$artist)
					$artist = FetchArtist (FP_ADMINISTRATOR);
				$ownername = $artist['Fullname'];
			} else {
				$artist = FetchArtist ($artistID);
				$ownername = $artist['Fullname'];
			}
			
			// Hide listing of projects? Useful if you turn on drop-down menus
			$currvals['param_'.FP_PARAM_GROUP_HIDE_LISTING.'_CHECKBOX'] = YesNo ('param_'.FP_PARAM_GROUP_HIDE_LISTING, GetParam ($params, FP_PARAM_GROUP_HIDE_LISTING ) );

			// Show web fonts fields
			$currvals['param_'.FP_PARAM_GROUP_WEB_FONTS_HEAD] = GetParam ($params, FP_PARAM_GROUP_WEB_FONTS_HEAD);
			$currvals['param_'.FP_PARAM_GROUP_WEB_FONTS_CSS] = GetParam ($params, FP_PARAM_GROUP_WEB_FONTS_CSS);
			$currvals['param_'.FP_PARAM_GROUP_WEB_FONTS_NAMES] = GetParam ($params, FP_PARAM_GROUP_WEB_FONTS_NAMES);

			// Navbar editor
			$currvals['param_'.FP_PARAM_GROUP_CUSTOM_MENU.'_CHECKBOX'] = YesNo ('param_'.FP_PARAM_GROUP_CUSTOM_MENU, GetParam ($params, FP_PARAM_GROUP_CUSTOM_MENU ) );

			for ($i=1;$i<=8;$i++) {
				$checked = array (GetParam ($params, constant("FP_PARAM_GROUP_MENU_1_$i")));
				$checked || $checked = "";
				$currvals["NAVBARMENU_1_$i"] =  OptionListFromArray (	$FP_NAVBAR_POPUP_MENU,
					"param_".constant("FP_PARAM_GROUP_MENU_1_$i"),
					$checked,
					TRUE,
					OPTION_LIST_IS_POPUP,
					"",
					$extraline = array('value'		=> "",
					'checked'	=> "",
					'label'		=> "")
				);
			}

			for ($i=1;$i<=8;$i++) {
				$checked = array (GetParam ($params, constant("FP_PARAM_GROUP_MENU_2_$i")));
				$checked || $checked = "";
				$currvals["NAVBARMENU_2_$i"] =  OptionListFromArray (	$FP_NAVBAR_POPUP_MENU,
					"param_".constant("FP_PARAM_GROUP_MENU_2_$i"),
					$checked,
					TRUE,
					OPTION_LIST_IS_POPUP,
					"",
					$extraline = array('value'		=> "",
					'checked'	=> "",
					'label'		=> "")

				);
			}
			/*
			// THIS IS A BITCH TO FIGURE OUT. KEEP OLD VERSION HERE JUST IN CASE.
			$currvals['param_'.FP_PARAM_GROUP_MENU_CUSTOM_1] = htmlentities(stripslashes(GetParam ($params, FP_PARAM_GROUP_MENU_CUSTOM_1 )));
			$currvals['param_'.FP_PARAM_GROUP_MENU_CUSTOM_2] = htmlentities(stripslashes(GetParam ($params, FP_PARAM_GROUP_MENU_CUSTOM_2 )));
			$currvals['param_'.FP_PARAM_GROUP_MENU_CUSTOM_3] = htmlentities(stripslashes(GetParam ($params, FP_PARAM_GROUP_MENU_CUSTOM_3 )));
			$currvals['param_'.FP_PARAM_GROUP_MENU_CUSTOM_4] = htmlentities(stripslashes(GetParam ($params, FP_PARAM_GROUP_MENU_CUSTOM_4 )));
			$currvals['param_'.FP_PARAM_GROUP_MENU_CUSTOM_5] = htmlentities(stripslashes(GetParam ($params, FP_PARAM_GROUP_MENU_CUSTOM_5 )));
			*/

			$currvals['param_'.FP_PARAM_GROUP_MENU_CUSTOM_1] = htmlentities(utf8_to_html(stripslashes(GetParam ($params, FP_PARAM_GROUP_MENU_CUSTOM_1 ))));
			$currvals['param_'.FP_PARAM_GROUP_MENU_CUSTOM_2] = htmlentities(utf8_to_html(stripslashes(GetParam ($params, FP_PARAM_GROUP_MENU_CUSTOM_2 ))));
			$currvals['param_'.FP_PARAM_GROUP_MENU_CUSTOM_3] = stripslashes(GetParam ($params, FP_PARAM_GROUP_MENU_CUSTOM_3 ));
			$currvals['param_'.FP_PARAM_GROUP_MENU_CUSTOM_4] = stripslashes(GetParam ($params, FP_PARAM_GROUP_MENU_CUSTOM_4 ));
			$currvals['param_'.FP_PARAM_GROUP_MENU_CUSTOM_5] = stripslashes(GetParam ($params, FP_PARAM_GROUP_MENU_CUSTOM_5 ));


			// List of images to be used as an logo
			$checked = $myGroup->logo;
			$ImageListLogo = OptionListFromArray (	$values,
				"Logo",
				$checked,
				TRUE,
				OPTION_LIST_IS_POPUP,
				"",
				array('value'		=> "0",
				'checked'	=> "",
				'label'		=> "(Use Uploaded Picture)")
			);

			// List of images to be used as an icon
			$checked = $myGroup->icon;
			$ImageListIcon = OptionListFromArray (	$values,
				"Icon",
				$checked,
				TRUE,
				OPTION_LIST_IS_POPUP,
				"",
				array('value'		=> "0",
				'checked'	=> "",
				'label'		=> "(Use Uploaded Picture)")
			);

			// THEME_LIST_FIELD popup
			$checked = array ($record['Theme']);
			$ThemeList = OptionListFromArray (	$Themes->FP_THEMES_LIST,
				"Theme",
				$checked,
				TRUE,
				OPTION_LIST_IS_POPUP,
				"id='themelist'",
				array('value'		=> "0",
				'checked'	=> "",
				'label'		=> "System Default")
			);


			// Only do these tasks if this is NOT a blank, new record...so an ID must exist
			if (isset($ID)) {
				$logo = $myGroup->LogoFilename();
				if ($logo) {
					$GroupLogo = $myGroup->LogoHTML ("name='LogoPicture' style='max-width:500px;'");
				} else {
					$GroupLogo = "<!--no logo-->";
				}

				$icon = $myGroup->IconFilename();
				if ($icon) {
					$GroupIcon = $myGroup->IconHTML ("name='IconPicture'");
				} else {
					$GroupIcon = "<!--no icon-->";
				}

				//$error .= __FUNCTION__.__LINE__.": " . $logo . "<BR>";
				// show list of artists you can add/remove, not including the owner of the group (who cannot be removed)

				// If this is the public gallery (the main gallery, ID = 1), then all artists are a member.
				// Otherwise, show the memberlist for editing
				if ($ID != 1) {
					$currvals['artistlist'] = $myGroup->MemberListHTML (TRUE);
				} else {
					$currvals['artistlist'] = FetchSnippet ("everyone_is_a_gallery_member");
				}

			}
			$pageTabID = "tab4";	// for return to list link
			break;

// *********************** PROJECTS
		case $PROJECTS :
			$DEBUG = 0;
			$DEBUG && $msg .=  basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": DEBUG ON<BR>";
			$DEBUG && $msg .= __LINE__.":" . ": Display: $PROJECTS<BR>Project ID=$ID<BR>";

			// Set $artistID to the project owner, not the current person signed in
			$artistID = $record['ArtistID'];
			$artist = FetchArtist($artistID);
			$ownername = $artist['Fullname'];
			$ownername || $ownername = "NO ONE";
			$DEBUG && $msg .= __FUNCTION__ .":".__LINE__.":" . ": Project owner is $ownername (ID=$artistID)<BR>";

			// Show public/private checkbox? Used to hide draft projects.
			$showpp = SHOW_PUBLICPRIVATE_FOR_PROJECTS;

			// Get age of project and days to live
			$projectstatus = GetProjectStatus ($ID);

			// Add project status info into the record for replacement in the form
			$currvals = $currvals + $projectstatus; // union of arrays!
			$projectage = $projectstatus['Age'];
			$projectdaystolive = $projectstatus['DaysToLive'];

			// List of pictures in project
			$pixlist = FetchProjectImageList ($ID);

			// Date is MySQL format yyyy-mm-dd
			$record['ProjectDate'] || $record['ProjectDate'] = date ("Y-m-d");

			// THEME_LIST_FIELD popup
			$checked = array ($params[FP_PARAM_GALLERY_THEME]);
			$ThemeList = OptionListFromArray (
				$Themes->FP_THEMES_LIST,
				"param_".FP_PARAM_GALLERY_THEME,
				$checked,
				TRUE,
				OPTION_LIST_IS_POPUP,
				"id='themelist'",
				$extraline = array(
				'value'		=> "0",
				'checked'	=> "",
				'label'		=> "Use Gallery Theme")
			);
			
			// Which theme are we using? If $params[FP_PARAM_GALLERY_THEME] is 0,
			// then we're using the gallery theme...what is it?
			if ($params[FP_PARAM_GALLERY_THEME] != 0) {
				$currvals['THEME_ID'] = $params[FP_PARAM_GALLERY_THEME];
			} else {
				$myProject = new FPProject ($LINK, $ID);
				$currvals['THEME_ID'] = $myProject->GetProjectThemeID();
			}
			
			// OwnerAccessOnly
			// Only the owner of the project can modify it, even if other artists are members.
			// Don't need to record the actual value of this: it will be zero if missing, which is fine.
			if (($fp_user == FP_ADMINISTRATOR) or ($accesslevel <= FP_NORMAL_LEVEL and (!$record['ArtistID'] or $record['ArtistID'] == $fp_user))) {
				$currvals['OwnerAccessOnly_CHECKBOX'] = YesNo ('OwnerAccessOnly', $record['OwnerAccessOnly']);
			} else {
				$record['OwnerAccessOnly'] 
				? $currvals['OwnerAccessOnly_CHECKBOX'] = "{fp:Yes}."
				: $currvals['OwnerAccessOnly_CHECKBOX'] = "{fp:No}.";
			}

			// Client list:
			// List of client sign-in names that allow viewer to see the
			// this particular exhibition.
			// NOW it is a real field.
			// $currvals['client_list'] = $params[FP_PARAM_GALLERY_CLIENTLIST];

			
			// Captions: show, hide, or popup
			$checked = array ($params[FP_PARAM_GALLERY_SHOWCAPTIONS]);
			$choices = explode("\n", FetchSnippet ("input_form_Projects_captions_popup"));
			$currvals["param_".FP_PARAM_GALLERY_SHOWCAPTIONS] = OptionListFromArray (
				$choices,
				"param_".FP_PARAM_GALLERY_SHOWCAPTIONS,
				$checked,
				TRUE,
				OPTION_LIST_IS_POPUP,
				$extrahtml=""
			);


			// Popup list of groups to assign the project to
			// If user owns the gallery or user is admin
			// Don't do if this is a 'personal' user, with no access to galleries
			$currentgroup = $record['GroupID'];
			$currentgroup || $currentgroup = $artist['GroupID']; //default group for the artist
			$currentgroup || $currentgroup = PUBLIC_GROUP_ID;
			if (($fp_user == FP_ADMINISTRATOR) or ($accesslevel <= FP_NORMAL_LEVEL and (!$record['ArtistID'] or $record['ArtistID'] == $fp_user))) {
				// popup list of groups to assign the project to.
				$fp_user == FP_ADMINISTRATOR
					? $currvals['GroupIDList'] = FetchGroupListHTML ($fp_user, $currentgroup)
					: $currvals['GroupIDList'] = FetchGroupListHTML ($artistID, $currentgroup);
			} else {
				// Gallery title
				$group = FetchGroup ($currentgroup);
				$currvals['GroupIDList'] = $group['Title'];
			}


			// Picture Frame
			$record['Framestyle'] || $record['Framestyle'] = 0;	// default to no frame
			$FramestyleList = OptionListFromArray ($SYSTEM_FRAMESTYLES, "Framestyle", $record['Framestyle'], true, OPTION_LIST_IS_POPUP);

			// These are checkboxes (Public is set lower because it's user level sensitive)
			$currvals['Matted_CHECKBOX'] = YesNo ('Matted', $record['Matted']);
			$currvals['Matchprint_CHECKBOX'] = YesNo ('Matchprint', $record['Matchprint']);

			// *** Show link for viewer comments?
			$currvals['param_'.FP_PARAM_GALLERY_COMMENTS.'_CHECKBOX'] = YesNo ('param_'.FP_PARAM_GALLERY_COMMENTS, GetParam ($params, FP_PARAM_GALLERY_COMMENTS ));


			// Show captions?
			$currvals['param_'.FP_PARAM_GALLERY_SHOWCAPTIONS.'_CHECKBOX'] = YesNo ('param_'.FP_PARAM_GALLERY_SHOWCAPTIONS, GetParam ($params, FP_PARAM_GALLERY_SHOWCAPTIONS ) );
			$currvals["Slides".'_CHECKBOX'] = YesNo ('Slides', $record['Slides'] );

			// Other params: grouping, spacing, group spacing, scaling
			$currvals['param_'.FP_PARAM_GALLERY_GROUPING] = GetParam ($params, FP_PARAM_GALLERY_GROUPING);
			$currvals['param_'.FP_PARAM_GALLERY_SPACING] = GetParam ($params, FP_PARAM_GALLERY_SPACING);
			$currvals['param_'.FP_PARAM_GALLERY_GROUP_SPACING] = GetParam ($params, FP_PARAM_GALLERY_GROUP_SPACING);
			$currvals['param_'.FP_PARAM_GALLERY_SCALING] = GetParam ($params, FP_PARAM_GALLERY_SCALING);


			// Image price sets are where we lock editions, not at the project level.
			// Therefore, price, ink, and paper can change even after an image is sold.
			// However, the image itself will then determine this info.

			// Pricing
			$pricesetID = $record['PriceSetID'];
			$pricesetID || $pricesetID = 0;	//use default, which should be ID=0, which means not for sale
			$PriceSetList = FetchPriceSetsHTML ($artistID, $pricesetID, "");

			// Ink
			$supplier = GetRecord ($SUPPLIERS, $artist["Vendor"]);
			// Following is an unused method means we store the vendor code for a paper/ink, instead of an offset
			// in the list. The offset is better, because that way the vendor can change the
			// code but we don't care. Also, we use offsets for lookups of papers/inks in the frameshop.
			//$inksets = array_combine (explode ("\r", trim ($supplier["InksetCodes"])), explode ("\r", trim ($supplier["Inksets"])));
			$inksets = explode ("\r", trim ($supplier["Inksets"]));
			$inksetCode = $record['InksetCode'];
			empty($inksetCode) && $inksetCode = 0;	//use first in list, could be "0"
			$inkSetList = OptionListFromArray ($inksets, "InksetCode", $inksetCode, true, OPTION_LIST_IS_POPUP);

			// Paper
			$supplier = GetRecord ($SUPPLIERS, $artist["Vendor"]);
			//$papers = array_combine (explode ("\r", trim ($supplier["PaperCodes"])), explode ("\r", trim ($supplier["Papers"])));
			$papers = explode ("\r", trim ($supplier["Papers"]));
			$paperCode = $record['PaperCode'];
			empty($paperCode) && 0;	//use first in list
			$paperList = OptionListFromArray ($papers, "PaperCode", $paperCode, true, OPTION_LIST_IS_POPUP);

			// {AUDIOTRACK}
			// Show list of audio tracks from the user's uploaded files
			$audioFilesList = FetchUserFileList ($USER_AUDIO_EXTENSIONS, $artistID);
			$audioFilesList = array(null=>"(none)") + $audioFilesList;

			if ($record['Audio']) {
				$audioHelpMessage = FetchSnippet("audio_file_help_message");
				$audioPath = GetSlideshowAudioPath ($ID);
				$currvals['AUDIOLENGTH'] = GetAudioFileDuration ($audioPath."/".$record['Audio']);
				$currvals['AUDIO_HELP_MESSAGE'] = str_replace("{AUDIOLENGTH}", $currvals['AUDIOLENGTH'], FetchSnippet("audio_file_help_message"));
				$currvals['AUDIO_HELP_MESSAGE'] = str_replace("{AUDIO}", $record['Audio'], $currvals['AUDIO_HELP_MESSAGE']);
			} else {
				$currvals['AUDIO_HELP_MESSAGE'] = FetchSnippet("audio_default_help_message");
			}

			// Only do these tasks if this is NOT a blank, new record, which we are showing
			// for the user to create a new record
			// ...so an ID must exist
			if ($ID) {
				$DEBUG && $msg .= __FUNCTION__ .":".__LINE__.":" . ": Preparing artistlist, storylist, imagelist for the form, for fp_user=$fp_user<BR>";

				// Other Artists to add:
				// Get list of all artists in the group (not all artists!)
				// Don't do for personal gallerists.
				// Instead, show artists who are part of this project
				if (($fp_user == FP_ADMINISTRATOR) or ($accesslevel <= FP_NORMAL_LEVEL and $record['ArtistID'] == $fp_user)) {
					$otherArtistList = FetchArtistListHTML ($currentgroup, $record['ID'], true);
				} else {

					$myProject = new FPProject ($LINK, $ID);
					$artistlist = $myProject->MemberList(true);


					//$myGroup = new FPGroup ($LINK, $record['GroupID'] );
					//$artistlist = $myGroup->MemberList( true );
					$namelist = array();
					foreach ($artistlist as $a) {
						$namelist[] = join (" ", array ($a['Firstname'], $a['Lastname']));
					}
					$otherArtistList = join ("<br>", $namelist);
					// If we are doing display only, give the field a special value so we know
					// not to update it!
					$otherArtistList .= "<INPUT TYPE=hidden NAME='artistlist' VALUE='noedit'>";
				}
				$currvals['OTHER_ARTISTS_LIST'] = Substitutions(FetchSnippet ("input_form_projects_artist_list"), array ("artistlist"=>$otherArtistList));

				// Story list
				$currvals['storylist'] = BuildProjectStories ($ID, $userID);


				// Links to other pages/lists
				//$currvals['imagelist'] = BuildOptionTransfer ($ID, "", true);
				$manageproject = FetchSnippet ("link_manageproject");	// code linking to manage project page
				$uploadpictures = FetchSnippet ("link_uploadpictures");	// code linking to manage pictures page
				$managepicturesjq = FetchSnippet ("link_managepictures_jq");	// code linking to manage pictures page
				$orderpictures= FetchSnippet ("link_orderpictures");	// code linking to arrange pictures page
				$choosepictures= FetchSnippet ("link_choosepictures");	// code linking to arrange pictures page
				$managestories= FetchSnippet ("link_managestories");	// code linking to manage stories page

				$pixcount = FetchProjectImageCount ($ID);
				$pixcount ? $record['PictureCount'] = $pixcount : $record['PictureCount'] = "0";

				// no need, $record has this already...
				//$currvals['Audio'] = trim(GetProjectSlideshowAudio ($ID));
				/*
				$record['Audio'] || $record['Audio'] = "(none)";
				
				$record['SlideShowDuration'] 
					? $audioLength = $record['SlideShowDuration']
					: $audioLength = GetSlideShowDuration ($ID);
				($audioLength > 0 && $pixcount)
					? $d = ceil( $audioLength / $pixcount)
					: $d = FP_DEFAULT_SLIDESHOW_PAUSE; 
				
				$record['SlideDuration'] = $d;
				*/

			} else {

				$uploadpictures = "<!-- new project, no pictures -->\n";
				$managestories = "<!-- new project, no stories -->\n";
			}
			
			$pageTabID = "tab3";	// for return to list link
			$currvals['InksetList'] = $inkSetList;
			$currvals['PaperList'] = $paperList;
			$currvals['AudioList'] = OptionListFromArray ($audioFilesList, "Audio", $record['Audio'], true, OPTION_LIST_IS_POPUP);
			$currvals['PriceSetList'] = $PriceSetList;

			break;

// *********************** IMAGES
		case $IMAGES :
			$currvals = BuildEditImageForm ($ID, $record, $params, $currvals);
			$pageTabID = "tab5";	// for return to list link
			break;

// *********************** STORIES
		case $STORIES :
			$record['Story'] = utf8_to_html($record['Story']);
			$pageTabID = "tab6";	// for return to list link
			break;

// *********************** PRICESETS
		case $PRICESETS :
			$currvals = BuildEditPricesetForm ($ID, $record, $params, $currvals);
			$pageTabID = "tab7";	// for return to list link
			break;

// *********************** SUPPLIERS
		case $SUPPLIERS :
			/*
			$currvals['PrintCostUnitPopup'] = OptionListFromArray (
				array("cm","inches"),
				'PrintCostUnit',
				$record['PrintCostUnit'],
				true,
				OPTION_LIST_IS_POPUP
				);
			
			$currvals['PrintCostAreaUnitPopup'] = OptionListFromArray (
				array("square cm","square feet"),
				'PrintCostAreaUnit',
				$record['PrintCostAreaUnit'],
				true,
				OPTION_LIST_IS_POPUP
				);
			
			$currvals['PrintCostMethodPopup'] = OptionListFromArray (
				array("paper", "image"),
				'PrintCostMethod',
				$record['PrintCostMethod'],
				false,
				OPTION_LIST_IS_POPUP
				);
	
			$units = array("cm", "inches");
			$currvals['PrintCostUnitDisplay'] = $units[$record['PrintCostUnit']];
	
			*/

			// Use square feet OR United Inch method for calculating framing cost
			$currvals['param_'.FP_PARAM_SUPPLIER_FRAME_COST_METHOD] = OptionListFromArray (
				array("sq. ft.","U.I."),
				'param_'.FP_PARAM_SUPPLIER_FRAME_COST_METHOD,
				GetParam ($params, FP_PARAM_SUPPLIER_FRAME_COST_METHOD),
				true,
				OPTION_LIST_IS_POPUP
				);

			// Use square feet OR United Inch method for calculating framing cost
			$currvals['param_'.FP_PARAM_SUPPLIER_MATTE_COST_METHOD] = OptionListFromArray (
				array("sq. ft.","U.I."),
				'param_'.FP_PARAM_SUPPLIER_MATTE_COST_METHOD,
				GetParam ($params, FP_PARAM_SUPPLIER_MATTE_COST_METHOD),
				true,
				OPTION_LIST_IS_POPUP
				);

			
			/*
			$currvals['param_'.FP_PARAM_SUPPLIER_FRAME_COST_METHOD.'_CHECKBOX'] = YesNo ('param_'.FP_PARAM_SUPPLIER_FRAME_COST_METHOD, GetParam ($params, FP_PARAM_SUPPLIER_FRAME_COST_METHOD), "id=\"{$SUPPLIERS}_param_".FP_PARAM_SUPPLIER_FRAME_COST_METHOD."\"");
			*/
			// Get frame widths so we can show frames
			for ($i=1;$i<=count($FRAMESTYLESWIDTHS); $i++) {
				$currvals["FP_FRAME_WIDTH_$i"] = $FRAMESTYLESWIDTHS[$i];
			}

			$pageTabID = "tab8";	// for return to list link
			break;

// *********************** DEFAULT
		default :
			//$error .= __FUNCTION__.__LINE__.": OY!";
			break;
	}

	// Build Artists pop-up list to set the owner of the record
	// If admin, can use list
	// If owner, can use list
	// If missing owner ID, can use list
	// If this is a group, and user is >= FP_PERSONAL_LEVEL, then cannot change ownership.
	// Otherwise, can only see current owner's name
	isset ($record['ArtistID']) ? $artistID = $record['ArtistID'] : $artistID = $fp_user;
	$artist = FetchArtist ($artistID);
	$ownername = $artist['Fullname'];
	if (($fp_user == FP_ADMINISTRATOR)
		or !$artistID
		or ($accesslevel <= FP_PERSONAL_LEVEL and $record['ArtistID'] == $fp_user)
		and ($accesslevel < FP_PERSONAL_LEVEL and $table == DB_GROUPS)
	) {
		if ($artistID) {
			$fieldlabel = "CONCAT_WS(', ', Lastname, Firstname) AS Fullname";
			$checked= array ($artistID);
			$ArtistIDList = OptionList ($ARTISTS, "ArtistID", "ID", $fieldlabel, "", $checked, "Lastname",OPTION_LIST_IS_POPUP);
		} else {
			$ArtistIDList = "<INPUT TYPE=hidden NAME=ArtistID VALUE=\"$artistID\">$ownername";
		}
	} else {
		$ArtistIDList = "$ownername";
	}

	// Only an admin user (FP_ADMINISTRATOR) or the owner can change the public/private
	// status of a record.
	if ($showpp && ($accesslevel <= FP_ADMINISTRATOR_LEVEL | $artistID == $userID)) {
		$publicprivateinput = FetchSnippet ("publicprivateinput");
		$form = preg_replace ("/\{publicprivateinput\}/", $publicprivateinput, $form);
		$currvals['Public_CHECKBOX'] = YesNo ('Public', $record['Public']);
	}

	// This method which works for just as easily as the above. We don't have 'ARRAY' variables
	$record['ArtistID'] || $record['ArtistID'] = $userID;

	//$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__. ArrayToTable ($record);


	// ----------
	// UPLOADER


	IF ($DEBUG) {
		$msg .= __FUNCTION__.":".__LINE__.": TFU_DIR:". $_SESSION['TFU_DIR']."<BR>";
		$msg .= __FUNCTION__.":".__LINE__.": SID = ". $_SESSION['session_id']."<BR>";
		$msg .= __FUNCTION__.":".__LINE__.": Session ID:". $_SESSION['ID']."<BR>";
		$msg .= __FUNCTION__.":".__LINE__.": Session name = ". session_name()."<BR>";
	}

	// Don't show if user is over storage limit
	if (!$user->UserOverStorageLimit()) {
		// Substitutions for the Java picture uploader
		switch ($table) {
			case DB_PROJECTS :
				$uploader = FetchSnippet ("uploader");
				// Picture Upload Directory
				// Create an upload directory for this project, if needed
				// Set the TFU uploader to the right directory
				$uldir = "$BASEDIR/$MAILED_DIR/".$record['ID'];
				file_exists ($uldir) || mkdir ($uldir, 0755);
				$_SESSION['TFU_DIR'] = $uldir;
				$_SESSION["tfu_upload_extensions"] = null;
				break;
			case DB_FILES :
				$uploader = FetchSnippet ("uploader_files");
				$uldir = GetUserDirectory ($artistID) . FP_SUBDIR_USER_FILES;
				file_exists ($uldir) || mkdir ($uldir, 0755);
				$_SESSION['TFU_DIR'] = $uldir;
				$_SESSION["tfu_upload_extensions"] = "all";
				break;
		}

		$HTTPuploader = FetchSnippet ("uploader_http");
		// Note: order of replacement matters...uploader must be first, as other vars
		// appear inside it.

		$language = $record['Language'];
		$language || $language = FP_LANGUAGE;
		$s = "$language\">";
		$r = "$language\" SELECTED>";
		$langpop = str_ireplace ($s, $r, $langpop);

		//$DEBUG && $msg .= __FUNCTION__.__LINE__."basedir/mailed_dir=$BASEDIR/$MAILED_DIR<BR>";
		//$DEBUG && $msg .= __FUNCTION__.__LINE__."Uploader endpage: $UPLOADER_ENDPAGE<BR>";
	} else {
		$e = FetchSnippet ('error_user_over_storage');
		$uploader = "$e";	// interprets $vars in the string
		$HTTPuploader = "";
	}

	$form = Substitutions ($form, array (
		//"ArtistSalesForm"	=> $ArtistSalesForm,
		"uploader"		=> $uploader,
		"UPLOADER_HTTP"	=> $HTTPuploader,
		"destination"	=> $UPLOADER_DEST."?uploaddir=$projectdirUPLOAD",
		"endpage"		=> $UPLOADER_ENDPAGE,
		"target_path"	=> "$BASEDIR/$MAILED_DIR",
		"langpop"		=> $langpop
	));

	// ----------

	// At this point, some   all?   values should be cleaned up for display so we don't have
	// trouble with quotation marks
	//$keys = array ("Title", "Nickname", "Statement", "Description", "City", "Country");
	//$record = FixArrayFormValue ($record, $keys);
	//var_dump ($record);
	$record = FixQuotesForForms ($record);
	//print __FUNCTION__.__LINE__;
	//var_dump ($record);

	$form = Substitutions ($form, $record);
	$form = Substitutions ($form, $currvals);
	$form = Substitutions ($form, $formvals);
	$addnewrecord = "";
	$form = Substitutions ($form, array (
		"artistID"		=> $artistID,
		"ownername"		=> $ownername,
		"age"			=> $projectage,
		"addnewrecord"		=> $addnewrecord,
		"ReturnToList" 		=> $returntolist,
		"PageTabID"				=> $pageTabID,	//must follow "ReturnToList"
		"Manageproject" 	=> $manageproject,
		"Uploadpictures" 	=> $uploadpictures,
		"Managepicturesjq" 	=> $managepicturesjq,
		"Managestories" 	=> $managestories,
		"Orderpictures" 	=> $orderpictures,
		"Choosepictures" 	=> $choosepictures,
		//"dimensions"		=> $image_pixel_dimensions,
		"ENTER_PORTRAIT"	=> $portraitEntry,
		"ArtistIDList" 		=> $ArtistIDList,
		"PortraitIMG"		=> $PortraitIMG,
		"GroupLogo"		=> $GroupLogo,
		"GroupIcon"		=> $GroupIcon,
		"ImageList"		=> $ImageList,
		"ImageListLogo"		=> $ImageListLogo,
		"ImageListIcon"		=> $ImageListIcon,
		"PriceList"		=> $PriceList,
		"ProjectIDList"		=> $ProjectList,
		//"PriceSetList"		=> $PriceSetList,
		"DefaultGroupID"	=> $DefaultGroupID,
		"DefaultProjectID"	=> $DefaultProjectID,
		"action"		=> $action,
		"FramestyleList"	=> $FramestyleList,
		"DaysToLive"		=> $projectdaystolive,
		"VendorList"		=> $VendorList,
		"ThemeList"		=> $ThemeList,
		"ecommerceList"		=> $ecommerceList,
		"submit"		=> $submit,
		"Title"			=> ""	//if blank record, don't fill in
	));
	return $form;
}


// GetRecordForEditForm ($ID, $table)
// Get data from a record to fill out a form for editing, or if no data exists, set up a blank record.
// If $ID, then get the data and fill out the form
// If no $ID, then it's request for a blank for a new record
function GetRecordForEditForm ($ID, $table) {
	global $msg, $error;
	global $LINK;
	// If $ID, then get the data and fill out the form
	// If no $ID, then it's request for a blank for a new record
	$params = array();
	$record = array();
	$action = "";
	
	if ( $ID ) {
		$record = GetRecord ($table, $ID);
	
		// bad request?
		if (!$record) {
			$error .= "The requested record (#{$ID}) does not exist!<BR>";
			fp_error_log(__FUNCTION__.": The requested record (#{$ID}) does not exist!", 3, FP_ERROR_LOG);
			$action = "";
		} else {
			$params = $record['ParamsUnpacked'];
			$action = "update";
		}
	} else {
		$record = array();
		$params = GetDefaultParams ($table);
		$action = "insert";
	}
	
	$params || $params = array();
	return array($action, $record, $params);
}


// BuildEditImageForm
// Build the form for editing an image record for use in DisplayRecordForEdit().
function BuildEditImageForm ($ID, $record, $params, $currvals) {
	global $msg, $error;
	global $LINK;

	isset($_SESSION['fp_user']) && $fp_user = $_SESSION['fp_user'];
	if ($fp_user) {
		isset ($record['ArtistID']) ? $artistID = $record['ArtistID'] : $artistID = $fp_user;
		$f = GetImageSizeByID ($record['ID']);
		if ($f) {

			$extrahtml = "id='FP_PARAM_IMAGE_OFFSITE'";
			
			$f['LOCALDIMENSIONS'] = Substitutions(FetchSnippet ("input_form_Images_uploaded_dimensions"), GetImageSizeByID ($ID));
	
			IF (FP_ADMIN_CONTROLS_PRINT_FILE && $fp_user != FP_ADMINISTRATOR) {
				$f['OFFSITE'] = "";
				$image_pixel_dimensions = Substitutions(FetchSnippet ("input_form_Images_dimensions_locked"), $f);
			} else {
				$f['OFFSITE'] = YesNo ('param_'.FP_PARAM_IMAGE_OFFSITE, GetParam ($params, FP_PARAM_IMAGE_OFFSITE), $extrahtml) . FetchSnippet("input_form_Images_picstoredbyprinter");
				$image_pixel_dimensions = Substitutions(FetchSnippet ("input_form_Images_dimensions"), $f);
			}
			
			$usedefault = empty($checked);
			$label = "concat(Price, ' ', Currency, ' : ', Paperheight, ' x ', Paperwidth, ' ', Medium) AS label";
	
			// PRICING SETTINGS
			// These live in PriceSet. Records in PriceSet with ImageID's belong to actual images. Really.
			// A record is locked after the first sale (or locked by user). Until then it is unlocked.
			$pricesetID = FetchPriceSetID ($ID);
			$SalesForm = FetchSnippet ("input_form_Images_Sales");
			$salesinfo = ImageSalesInfo ($ID);
			$printsizes = $salesinfo['printsizes'];
			$editionsizes = $salesinfo['editionsizes'];
			$amounts = $salesinfo['amount'];	// will only appear if there's a dedicated PriceSet entry
			
			
			// Parent Price Set
			// Get the parent project's price set title
			$parentProject = FetchProject($record['ProjectID']);
			$PPPriceSet = FetchPriceSet($parentProject['PriceSetID']);
			$ParentPricesetTitle = $PPPriceSet['Title'];
			
			// Sales information
			$i = 0;
			$SalesRow = FetchSnippet ("input_form_Images_Sales_row");
			$SalesInfoOutput = "";
			
			// Edition Size
			// If taken from the price set, then either limited ed., or set to "0" which means unlimited.
			// If a value is entered for this image, then that overrules the price set value, and we
			// completely ignore the editionsize-per-print size settings.
			// Unless, the price set for the image is locked, in which case we use the price set's value!
			
			$editionsize = GetImageEditionSize ($ID, $salesinfo['totaledition'], $record);
			$pseditionsize = $salesinfo['priceseteditionsize'];
			$customeditionsize = GetParam ($params, FP_PARAM_IMAGE_EDITION_SIZE);

			$fixedprice = GetParam ($params, FP_PARAM_IMAGE_FIXED_PRICE);

			// Display of edition size (If edition size = 0, then it's an open (unlimited) edition)
			if ($editionsize > 0) {
				$editionsizeshow = $editionsize;
			} else {
				$editionsizeshow = FetchSnippet ("unlimited_edition_en");
			}
			
			
			// Build sales data rows
			$total = $salesinfo["outsidesales"];
			while (isset ($printsizes[$i])) {
				$amount = (0+$amounts[$i]);
				$SalesInfoOutput .= Substitutions ($SalesRow, array (
					"size"		=> $printsizes[$i], 
					"amount"	=> $amount, 
					"editionsize"	=> $editionsize
					));
				$total += $amount;
				$i++;
			}
			
			// Get next print number, based on total sold
			if ($total < $editionsize) {
				$nextPrintNum = $total + 1;
			} else {
				$nextPrintNum = "{fp:editionclosed}";
			}


			// Editions Form
			$EditionsForm = FetchSnippet ('input_form_Images_editioning');
			if ($record['EditionsLocked']) {
				$enterCustomSize = FetchSnippet ('msg_locked_no_change_ed_size');
			} else {
				$enterCustomSize = FetchSnippet ('input_form_Images_editioning_custom');
			}
			
			$EditionsForm = Substitutions ($EditionsForm, array (
				"ENTER_CUSTOM_EDITION_SIZE"		=> $enterCustomSize,
				'param_'.FP_PARAM_IMAGE_EDITION_SIZE 	=> $customeditionsize,
				'param_'.FP_PARAM_IMAGE_FIXED_PRICE 	=> $fixedprice,
				"editionsize"				=> $editionsize,
				"pseditionsize"				=> $pseditionsize,
				"editionsizeshow"			=> $editionsizeshow,
				"NEXTPRINTNUMBER"			=> $nextPrintNum
				));
	
			
			// Sales Form

			// if no settings...
			if (!$SalesInfoOutput)
				$SalesForm = FetchSnippet ("input_form_Images_Sales_nosettings");
				
			$SalesForm = Substitutions ($SalesForm, $record);
			$SalesForm = Substitutions ($SalesForm, array (
				"SalesInfoOutput"		=> $SalesInfoOutput,
				"editionsize"			=> $editionsize,
				"pseditionsize"			=> $pseditionsize,
				"sold"				=> $total,
				"remaining"			=> $editionsize - $total,
				"NEXTPRINTNUMBER"		=> $nextPrintNum
				));
	
			// Is the price set locked? If not, show pricing pop-up of pricing lists.
			$locked = $record['EditionsLocked'];
		 	if (!$record['EditionsLocked']) {
		 		// show lock
	 			$usingparent = FetchSnippet("msg_using_parent_priceset");

				$PriceSetList = FetchPriceSetsHTML ($artistID, $record['PriceSetID'], $usingparent);
				$pricesetname = "";
		 		$lockpriceset = FetchSnippet("input_form_images_pricesetlocker");
		 	} else {
				$PriceSetList = CatalogNumber ($ID);
				$pricesetname = $PriceSetList;
		 		$lockpriceset = FetchSnippet("msg_priceset_locked");
		 	}
		 	
		 	// Vimeo Video (not a picture)
			$currvals['param_'.FP_PARAM_IMAGE_IS_VIDEO."_CHECKBOX"] = YesNo ('param_'.FP_PARAM_IMAGE_IS_VIDEO, GetParam ($params, FP_PARAM_IMAGE_IS_VIDEO), 'id="FP_PARAM_IMAGE_IS_VIDEO"');
			$currvals['param_'.FP_PARAM_IMAGE_VIDEO_EMBED_CODE] = GetParam ($params, FP_PARAM_IMAGE_VIDEO_EMBED_CODE);
			$currvals['param_'.FP_PARAM_IMAGE_VIDEO_WIDTH] = GetParam ($params, FP_PARAM_IMAGE_VIDEO_WIDTH);
			$currvals['param_'.FP_PARAM_IMAGE_VIDEO_HEIGHT] = GetParam ($params, FP_PARAM_IMAGE_VIDEO_HEIGHT);
			
		 	// Original Artwork, e.g. painting (and not a print in an edition)?
			$currvals['param_'.FP_PARAM_IMAGE_IS_ORIGINAL_ART."_CHECKBOX"] = YesNo ('param_'.FP_PARAM_IMAGE_IS_ORIGINAL_ART, GetParam ($params, FP_PARAM_IMAGE_IS_ORIGINAL_ART), 'id="FP_PARAM_IMAGE_IS_ORIGINAL_ART"');
			
		 	// Fixed size artwork (not calculated based on pixels and price sets)?
			$currvals['param_'.FP_PARAM_IMAGE_IS_FIXED_SIZE."_CHECKBOX"] = YesNo ('param_'.FP_PARAM_IMAGE_IS_FIXED_SIZE, GetParam ($params, FP_PARAM_IMAGE_IS_FIXED_SIZE), "id='FP_PARAM_IMAGE_IS_FIXED_SIZE'");
			$currvals['param_'.FP_PARAM_IMAGE_FIXED_WIDTH] = GetParam ($params, FP_PARAM_IMAGE_FIXED_WIDTH);
			$currvals['param_'.FP_PARAM_IMAGE_FIXED_HEIGHT] = GetParam ($params, FP_PARAM_IMAGE_FIXED_HEIGHT);
			
			// frame / matte settings
			$currvals['param_'.FP_PARAM_IMAGE_NO_SELL_MATTE."_CHECKBOX"] = YesNo ('param_'.FP_PARAM_IMAGE_NO_SELL_MATTE, GetParam ($params, FP_PARAM_IMAGE_NO_SELL_MATTE), "id='FP_PARAM_IMAGE_NO_SELL_MATTE'");
			$currvals['param_'.FP_PARAM_IMAGE_NO_SELL_FRAME."_CHECKBOX"] = YesNo ('param_'.FP_PARAM_IMAGE_NO_SELL_FRAME, GetParam ($params, FP_PARAM_IMAGE_NO_SELL_FRAME), "id='FP_PARAM_IMAGE_NO_SELL_FRAME'");

			
			$currvals['ImageSaleStatus'] = ImageOnSaleStatus($ID, null, false);

			// *** Values to return ***
			$currvals['SalesForm'] 			= $SalesForm;
			$currvals['EditionsForm'] 		= $EditionsForm;
			$currvals['PriceSetList'] 		= $PriceSetList;
			$currvals['dimensions']			= $image_pixel_dimensions;
			$currvals['pricesetlocker']		= $lockpriceset;
			$currvals['pricesetname'] 		= $pricesetname;
			$currvals['param_'.FP_PARAM_IMAGE_EDITION_SIZE] = $customeditionsize;
			$currvals['ParentPricesetTitle'] 	= $ParentPricesetTitle;
			
		}
	}
	return $currvals;
}


// BuildEditPricesetForm
// Build the form for editing an Price Set record for use in DisplayRecordForEdit().
function BuildEditPricesetForm  ($ID, $record, $params, $currvals) {
	global $msg, $error;
	global $LINK;

	isset($_SESSION['fp_user']) && $fp_user = $_SESSION['fp_user'];
	if ($fp_user) {
		isset ($record['ArtistID']) ? $artistID = $record['ArtistID'] : $artistID = $fp_user;
		
		if ($record['ImageID'])
			$warning = FetchSnippet ("fixed_price_set_warning");
	
		$image = FetchImage ($record['ImageID']);
		$imageParams = DecodeArrayFromDB ($image['Params']);

		// Amount sold outside the system
		$outsideSales = GetParam($imageParams, FP_PARAM_AMOUNT_SOLD_OUTSIDE);

		$arr = array (	"CATALOGNUMBER"	=> CatalogNumber ($image['ID']),
			"IMAGETITLE"	=> $image['Title']
		);
		$warning = Substitutions ($warning, $arr);
		$msg .= $warning;
		
		// Tiering (inflation)
		$params = $record['ParamsUnpacked'];
		$tierPeriod = GetParam($params, FP_PARAM_PRICESET_TIER);
		(empty($tierPeriod) || $tierPeriod < 0) && $tierPeriod = 1;
		
		
		$supplierID = $record['SupplierID'];

		$VendorList = FetchSupplierIDs();
		$checked = array ($supplierID);
		$currvals['SupplierPopup'] = OptionListFromArray (
			$VendorList,
			"SupplierID",
			$checked,
			TRUE,
			OPTION_LIST_IS_POPUP,
			$extrahtml="id='SupplierID'",
			$extraline = array('value'		=> "0",
			'checked'	=> "",
			'label'		=> "Choose a print provider")
		);
	
		$supplierPopup = OptionListFromArray (
			$VendorList,
			"SupplierID",
			$checked,
			TRUE,
			OPTION_LIST_IS_POPUP,
			$extrahtml="",
			$extraline = array('value'		=> "0",
			'checked'	=> "",
			'label'		=> "Choose a print provider")
		);

		// Ink & Paper
		if (!$supplierID) {
			$inkSetList = "(No {fp:supplier} chosen)";
			$paperList = "(No {fp:supplier} chosen)";
		} else {
			$supplier = GetRecord (DB_SUPPLIERS, $supplierID);
	
			// Ink
			// Following is an unused method means we store the vendor code for a paper/ink, instead of an offset
			// in the list. The offset is better, because that way the vendor can change the
			// code but we don't care. Also, we use offsets for lookups of papers/inks in the frameshop.
			//$inksets = array_combine (explode ("\r", trim ($supplier["InksetCodes"])), explode ("\r", trim ($supplier["Inksets"])));
			$inksets = explode ("\r", trim ($supplier["Inksets"]));
			$inksetCode = $record['InksetCode'];
			empty($inksetCode) && $inksetCode = 0;	//use first in list, could be "0"
			$inkSetList = OptionListFromArray ($inksets, "InksetCode", $inksetCode, true, OPTION_LIST_IS_POPUP);
	
			// Paper
			$supplier = GetRecord (DB_SUPPLIERS, $supplierID);
			//$papers = array_combine (explode ("\r", trim ($supplier["PaperCodes"])), explode ("\r", trim ($supplier["Papers"])));
			$papers = explode ("\r", trim ($supplier["Papers"]));
			$paperCode = $record['PaperCode'];
			empty($paperCode) && 0;	//use first in list
			$paperList = OptionListFromArray ($papers, "PaperCode", $paperCode, true, OPTION_LIST_IS_POPUP);
		}

		// Show edition price info
		$rowtemplate = FetchSnippet("input_form_pricesets_row");
		$rowtemplatep = FetchSnippet("input_form_pricesets_row_estimates");
		$rows = "<!-- ".__FUNCTION__.": case Pricesets : $rows -->\n";
		$cmImg = FetchSnippet('priceset_calculation_haveimage_message');
		$cmNoImg = FetchSnippet('priceset_calculation_estimated_message');
		$sizesTemplate = FetchSnippet('input_form_pricesets_sample_sizes');
	
		$sizes = explode(',', $record["a_Size"]);
		$edsizes = explode(',', $record["a_EditionSize"]);
	
		$pp = GetSamplePricing ($supplierID, $record['ID'], $record['ImageID']);
	
	
		// only get matchprint price once; it doesn't change per item
		$currvals['MatchPrintPrice'] = $pp[0]['MatchPrintPrice'];
	
		// Show all sizes
		$rows = array();
		$i = 0;
		$k = 1;
		while (isset($sizes[$i])) {
			if($sizes[$i] > 0) {
				$info = UnpackPriceSetRow($record, $i);
				$info['k'] = $k;
				$info['index'] = $i;
				$info['SupplierPopup'] = $supplierPopup;
				$info['Inactive'] > 0 ? $unused = 1 : $unused = 0;
				$info['InactivePopup'] = OptionListFromArray (
					array("For Sale","On Hold"),
					"Inactive[]",
					$unused,
					false,
					OPTION_LIST_IS_POPUP,
					"id=\"dimmer-{$k}\""
					);
				$unused ? $checked = "CHECKED" : $checked="";
				$info['InactiveCHECKED'] = $checked;
	
				if ($record['ImageID'] > 0) {
					$calcmsg = Substitutions($cmImg, $image);
				} else {
					$w = number_format(round($sizes[$i],2), 2) ;
					$h = number_format(round($sizes[$i]*(2/3),2), 2) ;
					$calcmsg = Substitutions($cmNoImg, array("width"=>$w,"height"=>$h));
				}
				
				// Sample sizing
				$dims = GetSampleSize ($sizes[$i]);
				$ss = array (
					SS_WIDTH			=> number_format ($dims['imageDims']['width'], 2, FP_DECIMAL, FP_THOUSANDS),
					SS_HEIGHT		=> number_format ($dims['imageDims']['height'], 2, FP_DECIMAL, FP_THOUSANDS),
					SS_PAPER_WIDTH	=> number_format ($dims['paperDims']['width'], 2, FP_DECIMAL, FP_THOUSANDS),
					SS_PAPER_HEIGHT	=> number_format ($dims['paperDims']['height'], 2, FP_DECIMAL, FP_THOUSANDS),
					SS_FRAME_WIDTH	=> number_format ($dims['frameDims']['width'], 2, FP_DECIMAL, FP_THOUSANDS),
					SS_FRAME_HEIGHT	=> number_format ($dims['frameDims']['height'], 2, FP_DECIMAL, FP_THOUSANDS)
					);
				
				$info['SAMPLE_SIZES'] = Substitutions ($sizesTemplate, $ss);
		
				$info['CALCMESSAGE'] = $calcmsg;
				$ppi = $pp[$i];
	
				// matchprint warning
				($ppi['TotalPrintPrice'] - ($ppi['PretaxPrintCost'] + $ppi['MatchPrintPrice']) <= 0)
				? $info['MatchPrintWarning'] = FetchSnippet("price_too_low_for_matchprint")
				: $info['MatchPrintWarning'] = "";
				
				// format all ppi nums as currency (DO AFTER CALCULATIONS, ABOVE!)
				$ppi && array_walk ($ppi, 'FormatAsCurrency');
	
				$info['MatchPrintPrice'] =  $ppi['MatchPrintPrice'];
				$info['MatchPrintPrice'] = number_format ($info['MatchPrintPrice'], 2, FP_DECIMAL, FP_THOUSANDS);
				
				$info['ESTIMATES'] = Substitutions($rowtemplatep, $ppi);
	
				$rows[$info['Size']] = Substitutions($rowtemplate, $info);
				$k++;
			}
			$i++;
		}
		ksort ($rows);
	
		// Add 1 blank rows
		for ($i=$k;$i<$k+1;$i++) {
			$info = array ();
			$info['k'] = "New";
			$rows[] = Substitutions($rowtemplate, $info);
		}
		
		$rows = implode("\n",$rows);
	
		$priceset = GetPriceSet ($record['ID']);
		$currvals['ROWS'] = $rows;
		if ($record['ImageID']) {
			$currvals['SALESFIGURES'] = SalesFiguresForm ($priceset, $outsideSales);
		} else {
			$currvals['SALESFIGURES'] = "";
		}

		// Don't allow changing of form unless user is admin
		if ($accesslevel > FP_ADMINISTRATOR_LEVEL) {
			$submit = FetchSnippet ("input_form_showlist");
			$action = "display";
		}
		
		
		// *** Values to return ***
		$currvals['InksetList']				= $inkSetList;
		$currvals['PaperList'] 				= $paperList;
		$currvals['submit'] 				= $submit;
		$currvals['action'] 				= $action;
		$currvals['VendorList']				= $VendorList;
		$currvals['param_'.FP_PARAM_PRICESET_TIER]	= $tierPeriod;
		

	}
	return $currvals;
}

function FixQuotesForForms ($arr, $keys = array ()) {
	$keys || $keys = array_keys ($arr);
	foreach ($keys as $key) {
		if (substr($key,0,6) != "Params")
			$arr[$key] = hscFixed($arr[$key]);
	}
	return $arr;
}

function hscFixed($str) {
	return preg_replace("/&amp;(#[0-9]+|[a-z]+);/i", "&$1;", htmlspecialchars($str));
}


// Make each element of an array suitable for HTML output, e.g. convert slashes, etc.
function HTMLEntitiesAllValues ($arr, $keys = array ()) {
	$keys || $keys = array_keys ($arr);
	foreach ($arr as $key => $value) {
		$arr[$key] = htmlentities ($value);
	}
	return $arr;
}

// Adds values to the array where the value has double-quotes slashed, e.g. "Joe "Joey" Jones" 
// becomes "Joe /"Joey/" Jones". The new values have a key prefixed with "value_". This is useful
// for creating values to fill out HTML forms when the value might contain a " mark.
function AddArrayFormValues ($arr, $keys = array ()) {
	$keys || $keys = array_keys ($arr);
	foreach ($keys as $key) {
		if ($key != "Params")
			$arr["value_".$key] = str_replace('"','\"', ($arr[$key]));
	}
	return $arr;
}


// ---------------------------------------
// BuildProjectImagesMulti
// For each artists in a project, show a list of images to choose from using SELECT field.
// The images must be active.
// if $numbered is true, then the names of each box will differ instead of forming one large array
// called imagelist[].
function BuildProjectImagesMulti ($projectID, $fp_user = "", $numbered = false, $showSelectedLists = false, $extrahtml ="") {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	global $msg, $error;
	global $LINK;

	$DEBUG = 0;

	$fp_user = $_SESSION['fp_user'];

	// Get formats
	$blockformat = FetchSnippet('MultiChooserBlock');
	$rowformat = FetchSnippet('MultiChooserRow');

	$artistlist = FetchProjectArtists ($projectID, $fp_user, False, $PROJECTS);

	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.":" . ": BuildProjectImagesMulti : Current artist list = " . ArrayToTable($artistlist) . "<BR>";

	$numbered ? $x = 1 : $x = "";
	$list = "<!--".__FUNCTION__.": MultiChooser-->\n";
	foreach ($artistlist as $ArtistID => $name) {
		$IDlist = array();

		// Get current image IDs
		$IDlist = FetchProjectImageList ($projectID);
		$checked = implode (", " , $IDlist);
		$checked = $IDlist;
		//		Not only active, because artist might be joining a project
		//		$where = "ArtistID = $ArtistID AND Active = \"yes\"";
		$where = "ArtistID = $ArtistID";
		$fieldlabel = "CONCAT_WS('-', Title, RollID, FrameID, CONCAT (' [', URL, ']')) AS ImageTitle";
		$order = "ImageTitle";
		$fromlist = OptionList ($IMAGES, "imagelist".$x."[]", "ID", $fieldlabel, $where, $checked, $order, OPTION_LIST_IS_MULTI, $extrahtml);

		if (!empty($fromlist))
			$list .= Substitutions ($rowformat, array ('title'=> $name, 'text' => $fromlist));

		$numbered && $x++;
	}
	$blockformat = Substitutions ($blockformat, array ('list' => $list));
	return $blockformat;
}



// ---------------------------------------
// Show stories which are part of a project, from the set of stories for each participant
function BuildProjectStories ($projectID) {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	global $msg, $error;
	global $LINK;

	$DEBUG = false;

	$fp_user = $_SESSION['fp_user'];

	// Get formats
	$blockformat = FetchSnippet('MultiChooserBlock');
	$rowformat = FetchSnippet('MultiChooserRow');

	$artistlist = FetchProjectArtists ($projectID, $fp_user, False, $PROJECTS);

	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.":" . ": Current list = " . ArrayToTable($artistlist) . "<BR>";

	$list = "<!--MultiChooser-->\n";
	foreach ($artistlist as $ArtistID => $name) {
		$IDlist = array();

		// Get current story IDs
		$IDlist = FetchProjectStories ($projectID);
		$checked = implode (", " , $IDlist);
		$checked = $IDlist;
		//		Not only active, because artist might be joining a project
		//		$where = "ArtistID = $ArtistID AND Active = \"yes\"";
		$where = "ArtistID = $ArtistID";
		$fieldlabel = "Title";
		$order = "Title";
		$artistList = OptionList ($STORIES, "storylist[]", "ID", $fieldlabel, $where, $checked, $order, OPTION_LIST_IS_MULTI);
		if (!empty($artistList))
			$list .= Substitutions ($rowformat, array ('title'=> $name, 'text' => $artistList));
	}
	$blockformat = Substitutions ($blockformat, array ('list' => $list));
	return $blockformat;
}


// ---------------------------------------
// BuildOptionTransfer
// Uses the OT javascripts
// For each artists in a project, show a list of images to choose from using SELECT field.
// The images must be active.
// if $numbered is true, then the names of each box will differ instead of forming one large array
// called imagelist[].
//
// 9/13/08
// Include previewing of clicked images.
//
function BuildOptionTransfer ($projectID, $extrahtml ="", $numbered = false) {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	global $msg, $error;
	global $LINK;
	global $PHOTOS_GALLERY, $THUMBNAILS, $MATTED, $ORIGINALS;
	global $OTCODE;	// See note near the end of admin.php. $OTCODE is built here to go into the <body> tag.


	$DEBUG = 0;

	$fp_user = $_SESSION['fp_user'];

	// Get formats
	$blockformat = FetchSnippet('OptionTransferBlock');
	$rowformat = FetchSnippet('OptionTransferRow');

	//$artistlist = FetchProjectArtists ($projectID, $fp_user, False, $PROJECTS);

	$project = new FPProject ($LINK, $projectID);
	$artistlist = $project->MemberNames();

	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.":" . ": Current artist list = " . ArrayToTable($artistlist) . "<BR>";

	$numbered ? $x = 1 : $x = "";
	$list = "<!--".__FUNCTION__."-->\n";
	$OTCODE = "";
	foreach ($artistlist as $artistID => $name) {
		$fields = array ();
		$IDlist = array();

		// list 1 is of pictures NOT in the project by artist
		// list 2 is of pictures in the project by artist
		$DEBUG && $msg .= "inProjectIDs = FetchProjectImageIDs ($projectID, $artistID)<BR>";
		$inProjectIDs = FetchProjectImageIDs ($projectID, $artistID);
		$artistPix = FetchArtistImageTitles ($artistID);
		$outProjectIDs = array_diff($artistPix,$inProjectIDs);

		$DEBUG && $msg .= __FUNCTION__.": inProjectIDs:".ArrayToTable ($inProjectIDs);
		$DEBUG && $msg .= __FUNCTION__.": FetchArtistImageTitles ($artistID):".ArrayToTable (FetchArtistImageTitles ($artistID));

		$checked = array ();
		// Used to be "onClick" instead of "onChange", but this allows scrolling with arrows
		$extrahtml = "onChange='showme($x, \"allpix\");'";
		$extrahtml .= " onDblClick='ot{x}.transferRight()' class='optionblocklist'";
		$extrahtml .= " id=\"allpix$x\"";
		$fromlist = OptionListFromArray ($outProjectIDs, "allpix".$x."[]", $checked, true, 10, $extrahtml);

		$extrahtml = "onChange='showme($x, \"imagelist\");'";
		$extrahtml .= " onDblClick='ot{x}.transferLeft()' class='optionblocklist'";
		$extrahtml .= " id=\"imagelist$x\"";
		$tolist = OptionListFromArray ($inProjectIDs, "imagelist".$x."[]", $checked, true, 10, $extrahtml);

		$DEBUG && $msg .= __FUNCTION__.ArrayToTable ($outProjectIDs);

		$fields["title"] = $name;
		$fields["list1"] = $fromlist;
		$fields["list2"] = $tolist;
		$fields["x"] = $x;

		// OTCODE is only used where the OptionTransfer javascript is
		// but we need to drop this code into the <body>
		// We build it here. It's a global. admin.php puts it in the HTML.
		$OTCODE .= "ot$x.init(document.choosepix);";

		// JS array of thumbnail URLs for previewing
		$artistPixURLs = FetchArtistImageURLs ($artistID);
		$inProjectURLs = FetchProjectImageURLs ($projectID, $artistID);
		$outProjectURLs = array_diff($artistPixURLs,$inProjectURLs);

		$pixurls = "";
		foreach ($artistPixURLs as $id => $url) {
			$pixurls .= "allurls['$id'] = '$url';\n";
		}
		//$artistPixURLs = "allurls[$x] = new Array (\"$THUMBNAILS/" . implode ("\",\"$THUMBNAILS/", $outProjectURLs) ."\");\n";

		//$projectPixURLs = "projecturls[$x] = new Array (\"$THUMBNAILS/" . implode ("\",\"$THUMBNAILS/", $inProjectURLs) ."\");\n";

		$fields["allpixurls"] = $pixurls;
		//$fields["imagelisturls"] = $projectPixURLs;


		if (!empty($fromlist))
			$list .= Substitutions ($rowformat, $fields);

		$numbered && $x++;

	}
	$OTCODE && $OTCODE = 'onLoad="' . $OTCODE . '"';
	$blockformat = Substitutions ($blockformat, array ('list' => $list));
	return $blockformat;
}


// ---------------------------------------
// BuildOptionTransferStories
// Uses the OT javascripts
// For each artists in a project, show a list of stories to choose from using SELECT field.
// if $numbered is true, then the names of each box will differ instead of forming one large array
// called imagelist[].

function BuildOptionTransferStories ($projectID, $extrahtml ="", $numbered = false) {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $STORIES, $PRICESETS, $SUPPLIERS, $PAYPAL, $SALES, $SNIPPETS;
	global $msg, $error;
	global $LINK;
	global $PHOTOS_GALLERY, $THUMBNAILS, $MATTED, $ORIGINALS;
	global $OTCODE;	// See note near the end of admin.php. $OTCODE is built here to go into the <body> tag.


	$DEBUG = 0;

	$fp_user = $_SESSION['fp_user'];

	// Get formats
	$blockformat = FetchSnippet('OptionTransferBlockStories');
	$rowformat = FetchSnippet('OptionTransferRowStories');

	//$artistlist = FetchProjectArtists ($projectID, $fp_user, False, $PROJECTS);

	$project = new FPProject ($LINK, $projectID);
	$artistlist = $project->MemberNames();

	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.":" . ": Current artist list = " . ArrayToTable($artistlist) . "<BR>";

	$numbered ? $x = 1 : $x = "";
	$list = "<!--".__FUNCTION__."-->\n";
	$OTCODE = "";
	foreach ($artistlist as $artistID => $name) {
		$fields = array ();

		$DEBUG && $msg .= "<hr>" . __FUNCTION__.": STORIES FOR $name<BR>";

		// list 1 is of stories NOT in the project
		// $inProject is of stories in the project
		//$inProject = FetchProjectStories ($projectID);
		$inProject = FetchProjectStoriesByArtist ($projectID, $artistID);
		$allStories = FetchArtistStories ($artistID);
		$outProject = array_diff($allStories,$inProject);

		$DEBUG && $msg .= __FUNCTION__.": inProject:".ArrayToTable ($inProject);
		$DEBUG && $msg .= __FUNCTION__.": allStories:".ArrayToTable ($allStories);
		$DEBUG && $msg .= __FUNCTION__.": outProject:".ArrayToTable ($outProject);

		$checked = array ();
		$extrahtml = "";
		$extrahtml .= " onDblClick='ot{x}.transferRight()' class='optionblocklist'";
		$extrahtml .= " id=\"allstories$x\"";
		$fromlist = OptionListFromArray ($outProject, "allstories".$x."[]", $checked, true, 8, $extrahtml);

		$extrahtml = "";
		$extrahtml .= " onDblClick='ot{x}.transferLeft()' class='optionblocklist'";
		$extrahtml .= " id=\"storylist$x\"";
		$tolist = OptionListFromArray ($inProject, "storylist".$x."[]", $checked, true, 8, $extrahtml);

		$DEBUG && $msg .= __FUNCTION__.ArrayToTable ($outProject);

		$fields["title"] = $name;
		$fields["list1"] = $fromlist;
		$fields["list2"] = $tolist;
		$fields["x"] = $x;

		// OTCODE is only used where the OptionTransfer javascript is
		// but we need to drop this code into the <body>
		// We build it here. It's a global. admin.php puts it in the HTML.
		$OTCODE .= "ot$x.init(document.choosestories);";

		if (!empty($fromlist))
			$list .= Substitutions ($rowformat, $fields);

		$numbered && $x++;

	}
	$OTCODE && $OTCODE = 'onLoad="' . $OTCODE . '"';
	$blockformat = Substitutions ($blockformat, array ('list' => $list));
	return $blockformat;
}



// ====================================================================================
// FetchPriceSetsHTML ($artistID, $pricesetID)
// Return an HTML menu of PriceSets, NOT including sets for individual images (i.e. locked images)
// For now, we allow ALL price sets, not just the artist' creations.
function FetchPriceSetsHTML ($artistID, $pricesetID='0', $blankentry = "", $includeCustom=false, $extrahtml = "") {
	global $IMAGES, $PRICES, $PRICESETS, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;
	global $LINK;

	$DEBUG = 0;
	$DEBUG && $msg .= "DEBUG ".__FUNCTION__."<BR>";
	if (!$blankentry)
		$priceset || $priceset = 1;

	$pricesetID ? $x = " OR (ID = $pricesetID)" : $x = "";
	$query = "select * from $PRICESETS WHERE (ImageID is NULL) OR (ImageID = 0) $x ORDER BY Title";
	$DEBUG && $msg .= "$query<BR>";
	$result = mysqli_query ($LINK, $query);

	if ($result) {
		while ($priceset = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
			$values[$priceset['ID']] = $priceset['Title'];
			$DEBUG && $msg .= __FUNCTION__.": pricesetid = {$priceset['ID']}, Title = {$priceset['Title']}<BR>";
		}
		$listname = "PriceSetID";
		$checked = $pricesetID;
		if ($blankentry) {
			$checked || $checked = "0";
			$extraline  = array ('label'=>$blankentry,'value'=>"0");
		} else {
			$checked || $checked = "1";
		}
		$pricesetIDList = OptionListFromArray ($values, $listname, $checked, TRUE, OPTION_LIST_IS_POPUP, $extrahtml, $extraline );
	} else {
		$pricesetIDList = "(no prices entered)";
	}
	$DEBUG && $msg .= __FUNCTION__.": pricesetid = $pricesetID, checked = $checked<BR>";
	return $pricesetIDList;
}


// ====================================================================================
// Show Artists which are members of the group. These are the artists who can be
// added to the project (or other table) on an edit record form.
// If flag is set, don't show the current fp_user
// Build an HTML multiple choice box for input.
// - Everyone is part of group 1 (the main gallery) (Note, not group 0!)

function FetchArtistListHTML ($groupID = null, $projectID = null, $hideCurrentUser = false) {
	global $msg, $error;
	global $LINK;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;

	isset($_SESSION['fp_user']) ? $fp_user = $_SESSION['fp_user'] : $fp_user = FP_ADMINISTRATOR;

	$DEBUG = 0;

	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.":" . ": BEGIN<BR>";
	$DEBUG && $msg .= __FUNCTION__ .":".__LINE__.":" . ": groupID: $groupID<BR>";
	$DEBUG && $msg .= __FUNCTION__ .":".__LINE__.":" . ": projectID: $projectID<BR>";
	$DEBUG && $msg .= __FUNCTION__ .":".__LINE__.":" . ": ownerID: $ownerID<BR>";
	$DEBUG && $msg .= __FUNCTION__ .":".__LINE__.":" . ": hideCurrentUser: $hideCurrentUser<BR>";

	$checked = array();
	// Default is we get artists for a project
	// Other possibility is $GROUPS
	$table || $table = $PROJECTS;
	$DEBUG && $msg .= __FUNCTION__ .":".__LINE__.":" . ": Table: $table<BR>";

	// Check accesslevel of current user (not project owner)
	$accesslevel = FetchAccessLevel ($fp_user);

	if ($groupID) {
		//get list of artists in this project, make them the checked in option list
		$myGroup = new FPGroup ($LINK, $groupID );
		$artistlist = $myGroup->MemberList( true );
		$DEBUG && $msg .= __FUNCTION__ .":".__LINE__.": artistlist:".ArrayToTable (array_keys ($artistlist));
	} else {
		$artistlist = FetchAllArtistArrays();
	}

	if ($hideCurrentUser)
		unset ($artistlist[$fp_user]);

	if ($projectID) {
		$project = FetchProject($projectID);
		$checkedArtists = FetchProjectArtists ($projectID, $fp_user, ($accesslevel > FP_ADMINISTRATOR_LEVEL), DB_PROJECTS);
		foreach (array_keys ($checkedArtists) as $c) {
			$checked[] = $c;
		}
	}


	$values = array();
	foreach ($artistlist as $artist) {
		$values[$artist['ID']] = $artist['Fullname'];
	}

	$DEBUG && $msg .= __FUNCTION__ .":".__LINE__.": Checked :".ArrayToTable ($checked);
	$DEBUG && $msg .= __FUNCTION__ .":".__LINE__.": Artists available in Group ID=$groupID:".ArrayToTable ($values);

	$listname = "artistlist[]";
	$extrahtml = "style='width:200px;'";
	$artistList = OptionListFromArray ($values, $listname, $checked, TRUE, OPTION_LIST_IS_MULTI, $extrahtml);

	$DEBUG && $msg .= __FUNCTION__ .":".__LINE__.":" . ": END FetchArtistListHTML";

	return $artistList;

}


// ========================================================================================
// Create a sorter for images.
// Note: $thumb_size is the height/width of a thumbnail, from the config file
function ManagementFormImageSorter ($projectID)
	{
	global $PHOTOS_GALLERY, $SLIDES, $THUMBNAILS, $MATTED, $ORIGINALS, $SITEIMAGES;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $BASEDIR, $LOGS;
	global $fp_user;
	global $msg, $error;
	global $LINK;
	
	$sorter = FetchSnippet("imagesorter-main");
	$item = FetchSnippet("imagesorter-image");
	$inListRows = "<!-- inListRows -->\n";
	$outListRows = "<!-- outListRows -->\n";
	
	$project = new FPProject ($LINK, $projectID);
	// List of pix in the project
	$inlist = $project->PictureList();
	$allpix = ProjectArtistsPixlist($projectID);
	// CSS class to mark a video item
	$videoclass = "video";
	$imageclass = "";
	$artworkclass = "artwork";
	
	
	// Remove pix in the project (the keys are the ID's of the pic)
	$outlist = array_diff_assoc($allpix,$inlist);
	
	$k=0;
	foreach ($inlist as $picture)
		{
		isset ($picture['Params']) ? $params = DecodeArrayFromDB($picture['Params']) : $params = array ();
		$IMG = URLtoIMG (FP_DIR_THUMBNAILS_IMAGES."/".$picture['URL'], '', 150, 100, $picture['Caption'], 1);
		if (GetParam($params,FP_PARAM_IMAGE_IS_VIDEO)) {
			$itemclass = $videoclass;
		} else {
			$itemclass = $imageclass;
		}

		$oneitem = Substitutions($item, array(
			// KEY: ID's cannot start with a number in HTML/CSS, and "pic_" is used by the OT javascript version, so let's keep it.
			"KEY"			=> "pic_{$picture['ID']}",
			"IMG"			=> $IMG,
			"URL"			=> $picture['URL'],
			"CAPTION"		=> $picture['Caption'],
			"TITLE"			=> $picture['Title'],
			"ITEM_CLASS"	=> $itemclass,
			"PID"			=> 'active',
			"ID"			=> $picture['ID']
		));
		$oneitem = Substitutions($oneitem, $picture);
		$k++;
		$inListRows .= $oneitem;
		}
	
	$k=0;
	foreach ($outlist as $picture)
		{
		isset ($picture['Params']) ? $params = DecodeArrayFromDB($picture['Params']) : $params = array ();
		if (GetParam($params,FP_PARAM_IMAGE_IS_VIDEO)) {
			$itemclass = $videoclass;
		} else {
			$itemclass = $imageclass;
		}
		$IMG = URLtoIMG (FP_DIR_THUMBNAILS_IMAGES."/".$picture['URL'], '', 150, 100, $picture['Caption'], 1);
		$oneitem = Substitutions($item, array(
			//"KEY"			=> "out_$k",
			"KEY"			=> "pic_{$picture['ID']}",		// ID's cannot start with a number in HTML/CSS
			"IMG"			=> $IMG,
			"URL"			=> $picture['URL'],
			"CAPTION"		=> $picture['Caption'],
			"TITLE"			=> $picture['Title'],
			"ITEM_CLASS"		=> $itemclass,
			"PID"			=> $picture['ProjectID'],
			"ID"			=> $picture['ID']

		));
		$oneitem = Substitutions($oneitem, $picture);
		$k++;
		$outListRows .= $oneitem;
		}
	
	// show by project
	$extraline  = array ('label'=>"All {fp:projects}",'value'=>"all", 'checked'=>'');
	if ($fp_user == FP_ADMINISTRATOR) {
		$checked = array ($projectID);
		$projectList = OptionList (DB_PROJECTS, "showImgsFromProjectID", "ID", "Title", "", $checked, "Title", false, $extrahtml="id='imagesort_select_project'",$extraline);
	} else {
		// Projects for normal artist
		$checked = array ($projectID);
		$projectList = OptionList (DB_PROJECTS, "showImgsFromProjectID", "ID", "Title", "ArtistID = '$fp_user'", $checked, "Title", false, $extrahtml="id='imagesort_select_project'",$extraline);
	}
	
	$sorter = Substitutions ($sorter, array (
		'INLIST'			=> $inListRows,
		'OUTLIST'		=> $outListRows,
		'PROJECTID'		=> $projectID,
		'PROJECTSLIST'	=> $projectList
		
		));
		
	return $sorter;
	
}



// ========================================================================================
// Create a grid of pictures in a project.
// Note: $thumb_size is the height/width of a thumbnail, from the config file
function ManagementFormPictures ($ID, $showthumbstatus = "Hide", $form = "", $itemFormat = "", $start = 1) {
	global $PHOTOS_GALLERY, $SLIDES, $THUMBNAILS, $MATTED, $ORIGINALS, $SITEIMAGES;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $BASEDIR, $LOGS;
	global $msg, $error;
	global $LINK;
	global $display_cols, $items_per_page;
	global $OTCODE;

	$DEBUG = 0;
	$DEBUG && $msg .= __FILE__.":".__FUNCTION__.":".__LINE__.": DEBUG ON<BR>";
	isset($_SESSION['fp_user']) ? $fp_user = $_SESSION['fp_user'] : $fp_user = FP_ADMINISTRATOR;

	// flip value of thumbnail status
	$NOTshowthumbstatus = $showthumbstatus;
	$showthumbstatus == "Show" ? $showthumbstatus = "Hide" : $showthumbstatus = "Show";

	$record = FetchProject ($ID);

	// Get list of pictures in the project
	// Picture list for preview/thumbnails. Only contains processed pictures. List of ID's
	// Order is set by OrderInProject field of Parts
	$pixlist = Pixlist($ID);

	$extraline  = array ('label'=>" Choose a picture",'value'=>"images/blank.gif", 'checked'=>'CHECKED');
	$extrahtml = 'OnChange="ChangePic(this.name)"';

	$values = array ();
	foreach ($pixlist as $picture) {
		$values[$THUMBNAILS."/".$picture['URL']] = $picture['Title'] . " [" . $picture['URL'] . "] ";
	}

	// Popup
	// get array for popup list of pictures for previewing
	$ProjectPix = OptionListFromArray ($values, "PictureList", array(), FALSE, OPTION_LIST_IS_POPUP, $extrahtml, $extraline);

	// Get array for ordering the pictures
	$f =  "{title} [{URL}]";
	$DEBUG && $msg .= __FILE__.":".__FUNCTION__.":".__LINE__.":format = $f<BR>$BASEDIR<BR>$THUMBNAILS<BR>";
	$orderingvalues = PixlistForOrdering ($ID, $f);
	$OrderPix = OptionListFromArray ($orderingvalues, "list", array(), FALSE, OPTION_LIST_IS_MULTI, "", "");


	// OK, this is weird. We have to make a select field of just numbers,
	// which the Javascript manipulates, to give us the final ordering.
	// I'm sure there's a more elegent way, but this works.

	$k=1;
	foreach ($orderingvalues as $partid=>$title) {
		$newordervals[$k++] = $partid;
	}

	$NewOrderSelect = OptionListFromArray ($newordervals, "list2", array(), FALSE, OPTION_LIST_IS_POPUP, "", "");

	//$ProjectPix = OptionList ($IMAGES, "PictureList", "URL", "Title",
	//							"ProjectID = '$ID'", "", "Title", $extrahtml, $extraline);
	$ProjectPix || $ProjectPix = "No Pictures";


	$form || $form = "input_form_ProjectsPictures";
	$form = FetchSnippet ($form);
	$form = preg_replace ("/\{ProjectPix\}/", $ProjectPix, $form);

	// print list of pictures

	// check if there are pix before doing anything more
	if (count($pixlist)) {
		// Show which pix on the page?
		// show how many pix per page?
		$start ? $from = $start : $from = 0;
		$to = min (count ($pixlist), $from + $items_per_page);

		// prev/next code snippets
		// set current info
		$current = FetchSnippet ("tablecurrentpix");
		$current = str_replace ("{from}", $from+1, $current);
		$current = str_replace ("{to}", $to, $current);
		$DEBUG && $error .= $current;
		// set prev link
		$prev = "";
		$from > 0 && $prev = FetchSnippet ("tablepreviouspix");
		$prev = str_replace("{NOTshowthumbstatus}", $NOTshowthumbstatus, $prev);
		$prev = str_replace("{start}", max($from-$items_per_page-1, 0), $prev);
		$DEBUG && $error .= $prev;
		// set next link
		$next = "";
		$to < count ($pixlist) && $next = FetchSnippet ("tablenextpix");
		$next = str_replace("{NOTshowthumbstatus}", $NOTshowthumbstatus, $next);
		$next = str_replace("{start}", min($to, count ($pixlist)), $next);
		$DEBUG && $error .= $next;
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__. ": To = $to<BR>";

		// Fetch the according snippet for the item template
		$onepicTemplate = FetchSnippet ($itemFormat . strtolower($showthumbstatus));
		$onepicTemplateNoLink = FetchSnippet ($itemFormat . "_nolink_" . strtolower($showthumbstatus));

		$picturelist = "";
		$k = $from;
		$c=1;
		$numcols = $display_cols;
		$collen = ceil(count($pixlist) / $numcols);
		$picturelist .=  "<table border=\"0\" width=\"90%\" cellpadding=\"5\">\r\t<tr>\r";
		// $pixlist comes sorted by OrderInProject from PixList() so don't sort...
		// ksort ($pixlist);


		$pixlist = array_slice ($pixlist, $from, $to - $from);
		$DEBUG && $msg .= "from $from to $to ipp $items_per_page<BR>";
		//$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__. ArrayToTable ($pixlist);

		if (!empty($pixlist)) {
			foreach ($pixlist as $pic) {
				// if show thumbnails, add in a thumbnail of the picture
				// If showthumbstatus is "hide", then it's offering the user to 'hide' them,
				// so we should be showing them. Weird, but a nice trick for output.

				$picture = FetchImage ($pic['ID']);
				$picID = $picture['ID'];
				//$DEBUG && $error .= __FUNCTION__ .":".__LINE__ ." PIC $k ****" . ArrayToTable ($picture) . "<BR>";
				$img = "\t\t<td valign=\"top\" WIDTH=\"$numcols" . "0%\">\r";

				if ($picture['ArtistID'] == $fp_user) {
					$img .= $onepicTemplate;
				} else {
					$img .= $onepicTemplateNoLink;
				}

				// Thumbnails have an un-square format, too, the THUMBNAILS2 version. Use that
				$src = "$THUMBNAILS/" . $picture['URL'];

				// If the picture is in the system ONLY as a HIRES,
				// we flag that for the user.
				$name = $picture['Title'] . " [" . $picture['URL'] . "] ";
				if ((! $picture['URL']) || (! file_exists ($src))) {
					$displayname .= "*";
					$src = FP_SYSTEM_IMAGES . "nopreview.gif";
				}
				$caption = htmlentities (str_replace ("\r", " ", $picture['Caption']), ENT_QUOTES);

				$img = str_replace("{src}", $src, $img);
				$img = str_replace("{picID}", $picID, $img);
				$img = str_replace("{picfilename}", $name, $img);
				$img = str_replace("{picdisplayname}", $name, $img);
				$img = str_replace("{caption}", $caption, $img);
				$img = str_replace("{k}", $k+1, $img);

				$img .= "\t\t</TD>";

				$picturelist .=  "\t\t\t" . $img;


//				$picturelist .=  "<small><font color=\"#404040\">" . ($k+1) . ") </font><B>".$pic['ID']."</B></small><BR>\r";
				$k++;	// comes here because we definitely add an item in line above...don't move
				$c++;
				if ($c > $numcols) {
					(($k % $collen) == 0) && ($picturelist .= "\r\t</tr>\r\t<tr>");
					$picturelist .= "\r\t</tr>\r\t<tr>";
					$c = 1;
				}
			}
		}
		$picturelist .= "\r\t</tr>\r</table>";
	} else {
		//$picturelist = "<HR><CENTER><i>There are no pictures in this project</i></CENTER>";
		$picturelist = "";
	}

	// ADD UNRELATED PICTURES/STORIES
	// List of artists working on this project
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": projectID $ID, fp_user=$fp_user<BR>";
	$allpicturelist = BuildProjectImagesMulti ($ID);
	$picturesOptionTransfer = BuildOptionTransfer ($ID, "", true);

	// SUBSTITUTIONS
	$javascript = FetchSnippet ("javascript");
	$form = Substitutions ($form, array (	"picturelist"		=> $picturelist,
		"ProjectPixB"		=> $ProjectPixB,
		"ProjectPix"		=> $ProjectPix,
		"OrderPix"			=> $OrderPix,
		"NewPixOrder"		=> $NewOrderSelect,
		"thumb_size"		=> $thumb_size,
		"showthumbstatus"	=> $showthumbstatus,
		"Javascript"		=> $javascript,
		"current"			=> $current,
		"next"				=> $next,
		"prev"				=> $prev,
		"start"				=> $from+1	,
		"allpicturelist"	=> $allpicturelist,
		"picturesOptionTransfer"	=> $picturesOptionTransfer
	));
	return $form;
}


// ========================================================================================
// Form for choosing PICTURES in a project.
// Must allow multiple artists, etc.
// This is the new, visual chooser
// Note: $thumb_size is the height/width of a thumbnail, from the config file
function ManagementFormChoosePictures ($ID, $showthumbstatus = "Hide", $form = "", $itemFormat = "", $start = 1) {
	global $PHOTOS_GALLERY, $SLIDES, $THUMBNAILS, $MATTED, $ORIGINALS, $SITEIMAGES;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $BASEDIR, $LOGS;
	global $display_cols, $items_per_page, $items_per_page_arr;
	global $OTCODE;
	global $msg, $error;
	global $LINK;

	$DEBUG = false;
	$DEBUG && $msg .= __FUNCTION__.__LINE__.": DEBUG ON<BR>";
	isset($_SESSION['fp_user']) ? $fp_user = $_SESSION['fp_user'] : $fp_user = FP_ADMINISTRATOR;

	// flip value of thumbnail status
	$NOTshowthumbstatus = $showthumbstatus;
	$showthumbstatus == "Show" ? $showthumbstatus = "Hide" : $showthumbstatus = "Show";

	// Get the project
	$record = FetchProject ($ID);
	$project = new FPProject ($LINK, $ID);

	// Picture list for preview/thumbnails. Only contains processed pictures. List of ID's
	// Order is set by OrderInProject field of Parts
	//$pixlist = Pixlist($ID);
	$pixlist = $project->PictureList();

	// get array of imageID => partID
	$orderingvalues = PartsForImages ($ID);

	// How many items to show per page?
	$items_per_page_Popup = OptionListFromArray ($items_per_page_arr, "items_per_page", array($items_per_page), FALSE, OPTION_LIST_IS_POPUP, "onChange='submit();'", "");


	$DEBUG && $msg .= __FUNCTION__.": Number of pix in project = ".count($pixlist) . "<BR>";

	$form || $form = "input_form_projects_choosepictures";
	$form = FetchSnippet ($form);
	$form = preg_replace ("/\{ProjectPix\}/", $ProjectPix, $form);

	// print list of pictures


	if (count($pixlist)) {	// be sure there are pix before doing anything more
		// Show which pix on the page?
		// show how many pix per page?
		$start ? $from = $start : $from = 1;
		$to = min (count ($pixlist), $from + $items_per_page - 1);

		$DEBUG && $msg .= __FUNCTION__.__LINE__.": from=$from, to=$to<br>";

		// prev/next code snippets
		// set current info
		$current = FetchSnippet ("tablecurrentpix");
		$current = str_replace ("{from}", $from, $current);
		$current = str_replace ("{to}", $to, $current);
		// set prev link
		$prev = "";
		$from > 1 && $prev = FetchSnippet ("tablepreviouspix_order");
		$prev = str_replace("{NOTshowthumbstatus}", $NOTshowthumbstatus, $prev);
		$prev = str_replace("{start}", max($from-$items_per_page, 1), $prev);
		//$DEBUG && $error .= $prev;
		// set next link
		$next = "";
		$to < count ($pixlist) && $next = FetchSnippet ("tablenextpix_order");
		$next = str_replace("{NOTshowthumbstatus}", $NOTshowthumbstatus, $next);
		$next = str_replace("{start}", min($to+1, count ($pixlist)), $next);
		//$DEBUG && $error .= $next;
		$DEBUG && $msg .= __FUNCTION__ .":".__LINE__. ": To = $to<BR>";

		// Fetch the according snippet for the item template
		$onepicTemplate = FetchSnippet ($itemFormat . "_" . strtolower($showthumbstatus));
		$onepicTemplateNoLink = FetchSnippet ($itemFormat . "_nolink_" . strtolower($showthumbstatus));

		$picturelist = "\r<!-- start picturelist -->\r";
		$k = $from;
		$c=1;
		$numcols = $display_cols;
		$collen = ceil(count($pixlist) / $numcols);
		$picturelist .=  "";
		// $pixlist comes sorted by OrderInProject from PixList() so don't sort...
		// ksort ($pixlist);


		$pixlist = array_slice ($pixlist, $from - 1, $to - $from + 1);
		$dif = $to - $from;
		$DEBUG && ($msg .= __FUNCTION__.": from=$from to=$to, (items per page = $items_per_page )<BR>");
		//$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__. ArrayToTable ($pixlist);

		$DEBUG && $msg .= "count:" . count ($pixlist) . "<BR>";
		if (!empty($pixlist)) {
			foreach ($pixlist as $pic) {
				// if show thumbnails, add in a thumbnail of the picture
				// If showthumbstatus is "hide", then it's offering the user to 'hide' them,
				// so we should be showing them. Weird, but a nice trick for output.

				$picture = FetchImage ($pic['ID']);
				$picID = $picture['ID'];
				//$DEBUG && $error .= __FUNCTION__ .":".__LINE__ ." PIC $k ****" . ArrayToTable ($picture) . "<BR>";
				// Get the part ID for the image in the project
				$elementID = $orderingvalues[$picture['ID']];

				// Assign an ID to each element. Starts with "pic_" since it must start with A-z.
				$img = "\t<div id=\"pic_" . $elementID . "\" class=\"picturelist$showthumbstatus\" >\r";

				if ($picture['ArtistID'] == $fp_user) {
					$img .= $onepicTemplate;
				} else {
					$img .= $onepicTemplateNoLink;
				}

				// Thumbnails have an un-square format, too, the THUMBNAILS2 version. Use that.
				$src = "$THUMBNAILS/" . $picture['URL'];

				// If the picture is in the system ONLY as a HIRES,
				// we flag that for the user.
				$name = $picture['Title'] . "<br>[" . $picture['URL'] . "] ";
				if ((! $picture['URL']) || (! file_exists ($src))) {
					$displayname .= "*";
					$src = FP_SYSTEM_IMAGES . "nopreview.gif";
				}
				$caption = htmlentities (str_replace ("\r", " ", $picture['Caption']), ENT_QUOTES);

				$img = str_replace("{src}", $src, $img);
				$img = str_replace("{picID}", $picID, $img);
				$img = str_replace("{picfilename}", $name, $img);
				$img = str_replace("{picdisplayname}", $name, $img);
				$img = str_replace("{caption}", $caption, $img);
				$img = str_replace("{k}", $k, $img);

				$img .= "</div>\r";

				$picturelist .=  $img;

				// unused...
				$k++;	// comes here because we definitely add an item in line above...don't move
				$c++;
				if ($c > $numcols) {
					(($k % $collen) == 0) && ($picturelist .= "\n");
					$picturelist .= "\n";
					$c = 1;
				}
			}
		}
		$picturelist .= "\r<!--end picturelist -->\r";
	} else {
		//$picturelist = "<HR><CENTER><i>There are no pictures in this project</i></CENTER>";
		$picturelist = "(No Pictures)";
	}

	// SUBSTITUTIONS
	$javascript = FetchSnippet ("javascript");
	$form = Substitutions ($form, array (		"picturelist"			=> $picturelist,
		"thumb_size"				=> $thumb_size,
		"showthumbstatus"		=> $showthumbstatus,
		"currentthumbstatus"		=> $NOTshowthumbstatus,
		"Javascript"				=> $javascript,
		"current"				=> $current,
		"next"					=> $next,
		"prev"					=> $prev,
		"start"					=> $from,
		"items_per_page_popup"	=> $items_per_page_Popup
	));
	return $form;
}


// ========================================================================================
// Form for handling PICTURES in a project
// Note: $thumb_size is the height/width of a thumbnail, from the config file
function ManagementFormOrderPictures ($ID, $showthumbstatus = "Hide", $form = "", $itemFormat = "", $start = 1) {
	global $PHOTOS_GALLERY, $SLIDES, $THUMBNAILS, $MATTED, $ORIGINALS, $SITEIMAGES;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $BASEDIR, $LOGS;
	global $msg, $error;
	global $LINK;
	global $display_cols, $items_per_page, $items_per_page_arr;
	global $OTCODE;

	$DEBUG = 0;
	isset($_SESSION['fp_user']) ? $fp_user = $_SESSION['fp_user'] : $fp_user = FP_ADMINISTRATOR;

	// flip value of thumbnail status
	$NOTshowthumbstatus = $showthumbstatus;
	$showthumbstatus == "Show" ? $showthumbstatus = "Hide" : $showthumbstatus = "Show";

	// Get the project
	$record = GetRecord ($PROJECTS, $ID);

	// Picture list for preview/thumbnails. Only contains processed pictures. List of ID's
	// Order is set by OrderInProject field of Parts
	$pixlist = Pixlist($ID);

	// get array of imageID=>partID
	$orderingvalues = PartsForImages ($ID);

	// How many items to show per page?
	$items_per_page_Popup = OptionListFromArray ($items_per_page_arr, "items_per_page", array($items_per_page), FALSE, OPTION_LIST_IS_POPUP, "onChange='submit();'", "");


	$DEBUG && $msg .= __FUNCTION__.": Number of pix in project = ".count($pixlist) . "<BR>";

	$form || $form = "input_form_projects_orderpictures";
	$form = FetchSnippet ($form);
	$form = preg_replace ("/\{ProjectPix\}/", $ProjectPix, $form);

	// print list of pictures


	if (count($pixlist)) {	// be sure there are pix before doing anything more
		// Show which pix on the page?
		// show how many pix per page?
		$start ? $from = $start : $from = 1;
		$to = min (count ($pixlist), $from + $items_per_page - 1);

		$DEBUG && $msg .= __FUNCTION__.__LINE__.": from=$from, to=$to<br>";

		// prev/next code snippets
		// set current info
		$current = FetchSnippet ("tablecurrentpix");
		$current = str_replace ("{from}", $from, $current);
		$current = str_replace ("{to}", $to, $current);
		// set prev link
		$prev = "";
		$from > 1 && $prev = FetchSnippet ("tablepreviouspix_order");
		$prev = str_replace("{NOTshowthumbstatus}", $NOTshowthumbstatus, $prev);
		$prev = str_replace("{start}", max($from-$items_per_page, 1), $prev);
		//$DEBUG && $error .= $prev;
		// set next link
		$next = "";
		$to < count ($pixlist) && $next = FetchSnippet ("tablenextpix_order");
		$next = str_replace("{NOTshowthumbstatus}", $NOTshowthumbstatus, $next);
		$next = str_replace("{start}", min($to+1, count ($pixlist)), $next);
		//$DEBUG && $error .= $next;
		$DEBUG && $msg .= __FUNCTION__ .":".__LINE__. ": To = $to<BR>";

		// Fetch the according snippet for the item template
		$onepicTemplate = FetchSnippet ($itemFormat . "_" . strtolower($showthumbstatus));
		$onepicTemplateNoLink = FetchSnippet ($itemFormat . "_nolink_" . strtolower($showthumbstatus));

		$picturelist = "\r<!-- start picturelist -->\r";
		$k = $from;
		$c=1;
		$numcols = $display_cols;
		$collen = ceil(count($pixlist) / $numcols);
		$picturelist .=  "";
		// $pixlist comes sorted by OrderInProject from PixList() so don't sort...
		// ksort ($pixlist);


		$pixlist = array_slice ($pixlist, $from - 1, $to - $from + 1);
		$dif = $to - $from;
		$DEBUG && ($msg .= __FUNCTION__.": from=$from to=$to, (items per page = $items_per_page )<BR>");
		//$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__. ArrayToTable ($pixlist);

		$DEBUG && $msg .= "count:" . count ($pixlist) . "<BR>";
		if (!empty($pixlist)) {
			foreach ($pixlist as $pic) {
				// if show thumbnails, add in a thumbnail of the picture
				// If showthumbstatus is "hide", then it's offering the user to 'hide' them,
				// so we should be showing them. Weird, but a nice trick for output.

				$picture = FetchImage ($pic['ID']);
				$picID = $picture['ID'];
				//$DEBUG && $error .= __FUNCTION__ .":".__LINE__ ." PIC $k ****" . ArrayToTable ($picture) . "<BR>";
				// Get the part ID for the image in the project
				$elementID = $orderingvalues[$picture['ID']];

				if (false) {
					$f = array ();
					$img= "";
					$itemwrapper = FetchSnippet("orderpictures_item");

					// Assign an ID to each element. Starts with "pic_" since it must start with A-z.
					$f['img_id'] = "pic_" . $elementID;
					$f['img_class'] = "picturelist$showthumbstatus";

					if ($picture['ArtistID'] == $fp_user) {
						$img .= $onepicTemplate;
					} else {
						$img .= $onepicTemplateNoLink;
					}

					// Thumbnails have an un-square format, too, the THUMBNAILS2 version. Use that.
					$src = "$THUMBNAILS/" . $picture['URL'];

					// If the picture is in the system ONLY as a HIRES,
					// we flag that for the user.
					$name = $picture['Title'] . "<br>[" . $picture['URL'] . "] ";
					if ((! $picture['URL']) || (! file_exists ($src))) {
						$displayname .= "*";
						$src = FP_SYSTEM_IMAGES . "nopreview.gif";
					}
					$caption = htmlentities (str_replace ("\r", " ", $picture['Caption']), ENT_QUOTES);

					$img = str_replace("{src}", $src, $img);
					$img = str_replace("{picID}", $picID, $img);
					$img = str_replace("{picfilename}", $name, $img);
					$img = str_replace("{picdisplayname}", $name, $img);
					$img = str_replace("{caption}", $caption, $img);
					$img = str_replace("{k}", $k, $img);

					$f['img'] = $img;
					$img = Substitutions ($itemwrapper, $f);

				} else {
					// Assign an ID to each element. Starts with "pic_" since it must start with A-z.
					$img = "\t<div id=\"pic_" . $elementID . "\" class=\"picturelist$showthumbstatus\" >\r";

					if ($picture['ArtistID'] == $fp_user) {
						$img .= $onepicTemplate;
					} else {
						$img .= $onepicTemplateNoLink;
					}

					// Thumbnails have an un-square format, too, the THUMBNAILS2 version. Use that.
					$src = "$THUMBNAILS/" . $picture['URL'];

					// If the picture is in the system ONLY as a HIRES,
					// we flag that for the user.
					$name = $picture['Title'] . "<br>[" . $picture['URL'] . "] ";
					if ((! $picture['URL']) || (! file_exists ($src))) {
						$displayname .= "*";
						$src = FP_SYSTEM_IMAGES . "nopreview.gif";
					}
					$caption = htmlentities (str_replace ("\r", " ", $picture['Caption']), ENT_QUOTES);

					$img = str_replace("{src}", $src, $img);
					$img = str_replace("{picID}", $picID, $img);
					$img = str_replace("{picfilename}", $name, $img);
					$img = str_replace("{picdisplayname}", $name, $img);
					$img = str_replace("{caption}", $caption, $img);
					$img = str_replace("{k}", $k, $img);

					$img .= "</div>\r";
				}

				$picturelist .=  $img;

				// unused...
				$k++;	// comes here because we definitely add an item in line above...don't move
				$c++;
				if ($c > $numcols) {
					(($k % $collen) == 0) && ($picturelist .= "\n");
					$picturelist .= "\n";
					$c = 1;
				}
			}
		}
		$picturelist .= "\r<!--end picturelist -->\r";
	} else {
		//$picturelist = "<HR><CENTER><i>There are no pictures in this project</i></CENTER>";
		$picturelist = "(No Pictures)";
	}


	if (false) {
		$picturelist = "<ul class=\"picturelist\">\n$picturelist\n</ul>";
	}


	// SUBSTITUTIONS
	$javascript = FetchSnippet ("javascript");
	$form = Substitutions ($form, array (		"picturelist"			=> $picturelist,
		"thumb_size"				=> $thumb_size,
		"showthumbstatus"		=> $showthumbstatus,
		"currentthumbstatus"		=> $NOTshowthumbstatus,
		"Javascript"				=> $javascript,
		"current"				=> $current,
		"next"					=> $next,
		"prev"					=> $prev,
		"start"					=> $from,
		"items_per_page_popup"	=> $items_per_page_Popup
	));
	return $form;
}


// Form for working with stories
function ManagementFormStories ($ID) {
	global $PHOTOS_GALLERY, $SLIDES, $THUMBNAILS, $MATTED, $ORIGINALS, $SITEIMAGES;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;
	global $LINK;
	global $display_cols, $items_per_page;

	$DEBUG = false;
	isset($_SESSION['fp_user']) ? $fp_user = $_SESSION['fp_user'] : $fp_user = 1;

	$record = GetRecord (DB_PROJECTS, $ID);
	$form = FetchSnippet ("input_form_ProjectsStories");
	$stories = FetchProjectStories ($ID);	// get list of story ID's
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": ".ArrayToTable ($stories);

	if (count($stories)) {
		// Fetch the according snippet for the item template
		$onestoryTemplate = FetchSnippet ("input_form_projects_stories_item");
		$storylistblock = FetchSnippet ("input_form_projects_stories_block");

		ksort ($stories);

		$k = 0;
		if (! empty($stories)) {
			foreach ($stories as $storyID) {
				$f = array ();
				$story = FetchStory ($storyID);

				$lead = htmlentities (str_replace ("\r", " ", $story['Lead']), ENT_QUOTES);
				$lead = $story['Lead'];

				//$body = htmlentities (str_replace ("\r", " ", $story['Story']), ENT_QUOTES);
				$body = $story['Story'];
				// if lead = story, dump story
				$lead == $body
					? $body = "<!-- same as lead -->"
					: $lead .= "...";

				//$body = str_replace ("\n", "<BR>\n", $body);
				$lead = preg_replace("/\r|\n|\t/", "", $lead);
				$lead = utf8_to_html ($lead);

				$body = preg_replace("/\r|\n|\t/", "", $body);
				$body = utf8_to_html ($body);

				$f["Title"] = $story['Title'];
				$f["Lead"] = $lead;
				$f["Story"] = $body;
				$f["k"] = $k+1;
				$f["ID"] = $storyID;
				$storyitem = Substitutions ($onestoryTemplate, $f);
				$k++;	// comes here because we definitely add an item in line above...don't move
				$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": $storyitem<BR>";
				$storylist .= $storyitem;
			}
		}
		$storylist = Substitutions ($storylistblock, array ("LIST"=>$storylist));
	} else {
		$storylist = FetchSnippet ("NoStoriesInProject");
	}

	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": projectID $ID, fp_user=$fp_user<BR>";

	// Show all PICTURES/STORIES to add/remove from the project
	$allstorylist = BuildOptionTransferStories ($ID, $extrahtml ="", TRUE);

	// SUBSTITUTIONS
	$javascript = FetchSnippet ("javascript");
	$form = Substitutions ($form, array (	"storylist"			=> $storylist,
		"Javascript"		=> $javascript,
		"allstorylist"		=> $allstorylist

	));
	return $form;
}


// Form for working with stories
function ManagementFormStoriesOLD ($ID) {
	global $PHOTOS_GALLERY, $SLIDES, $THUMBNAILS, $MATTED, $ORIGINALS, $SITEIMAGES;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;
	global $LINK;
	global $display_cols, $items_per_page;

	$DEBUG = false;
	isset($_SESSION['fp_user']) ? $fp_user = $_SESSION['fp_user'] : $fp_user = 1;

	$record = GetRecord (DB_PROJECTS, $ID);
	$form = FetchSnippet ("input_form_ProjectsStories");
	$stories = FetchProjectStories ($ID);	// get list of story ID's
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": ".ArrayToTable ($stories);

	if (count($stories)) {
		// Fetch the according snippet for the item template
		$onestoryTemplate = FetchSnippet ("input_form_ProjectsStories_story");

		$storylist = "";
		$storylist .=  "<table border=\"0\" width=\"90%\" cellpadding=\"5\">\r\t<tr>\r";
		ksort ($stories);

		$k = 0;
		if (! empty($stories)) {
			foreach ($stories as $storyID) {
				$story = FetchStory ($storyID);
				$lead = htmlentities (str_replace ("\r", " ", $story['Lead']), ENT_QUOTES);
				$body = htmlentities (str_replace ("\r", " ", $story['Story']), ENT_QUOTES);
				// if lead = story, dump story
				$lead == $body && $body = "<!-- same as lead -->";
				$lead .= "...";

				$body = str_replace ("\n", "<BR>\n", $body);

				$storyitem = $onestoryTemplate;
				$storyitem = str_replace("{Title}", $story['Title'], $storyitem);
				$storyitem = str_replace("{Lead}", $lead, $storyitem);
				$storyitem = str_replace("{Story}", $body, $storyitem);
				$storyitem = str_replace("{k}", $k+1, $storyitem);
				$storyitem = Substitutions ($storyitem, $story);
				$storyitem .= "\t\t</TD>";
				$storylist .=  "\t\t\t" . $storyitem;
				$k++;	// comes here because we definitely add an item in line above...don't move
				$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": $storyitem<BR>";
			}
		}
		$storylist .= "\r\t</tr>\r</table>";
	} else {
		$storylist = FetchSnippet ("NoStoriesInProject");
	}

	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": projectID $ID, fp_user=$fp_user<BR>";

	// Show all PICTURES/STORIES to add/remove from the project
	$allstorylist = BuildProjectStories ($ID);

	// SUBSTITUTIONS
	$javascript = FetchSnippet ("javascript");
	$form = Substitutions ($form, array (	"storylist"			=> $storylist,
		"Javascript"		=> $javascript,
		"allstorylist"		=> $allstorylist

	));
	return $form;
}


// =================================================================
// Add an Story to a Project
// Make a new Part, return the ID of the new part.
function AddStoryToProject ($StoryID, $ArtistID, $projectID) {
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;
	global $LINK;

	$DEBUG = false;
	$DEBUG && $msg .= __FILE__. __LINE__ . "AddStoryToProject<BR>" . $query;
	$pairs = array ('ProjectID'	=> $projectID,
		'ArtistID'	=> $ArtistID,
		'PartTable'	=> $STORIES,
		'PartID'	=> $StoryID
	);
	$newID = EditTable ('insert', 'Parts', '', $pairs);
	MarkProjectUpdated ($projectID);
	return $newID;
}


// =================================================================
// SetProjectOwner ($projectID, $ownerID);
// Set the owner of a project
// - If the owner has a Part, remove that part
function SetProjectOwner ($projectID, $ownerID) {
	global $msg, $error;
	global $LINK;

	if (!$projectID)
		return false;
	if (!$ownerID)
		return false;
	$pairs = array ('ArtistID' => $ownerID);
	EditTable ('update', DB_PROJECTS, $projectID, $pairs);
	$query = "select * from Parts where PartID = '$ownerID' AND ProjectID = '$projectID' AND PartTable = '".DB_ARTISTS."' LIMIT 1";
	$result = mysqli_query ($LINK, $query);
	// If part exists for the owner, delete it
	if (mysqli_num_rows($result)) {
		$query  = "delete from ".DB_PARTS." where PartID = '$ownerID' AND ProjectID = '$projectID' AND PartTable = '".DB_ARTISTS."'";
		mysqli_query ($LINK, $query);
	}
}

// =================================================================
// SetProjectTheme ($projectID, $themeID);
function SetProjectTheme ($projectID, $themeID) {
	$pairs = array ("param_".FP_PARAM_GALLERY_THEME => $themeID);
	//FP_PARAM_GALLERY_THEME
	EditTable ('update', DB_PROJECTS, $projectID, $pairs);
}

// =================================================================
// Add an Artist to a Project
// Make a new Part, return the ID of the new part.
// 		PartTable 	= Artists
//		PartID 		= the artist ID
// 		ProjectID 	= the project ID
// 		ArtistID 	= the artist ID
//		GroupID		= null

function AddArtistToProject ($artistID, $projectID) {
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;
	global $LINK;

	$DEBUG = 0;

	// check to see if artist is already in the project
	$query = "select * from ".DB_PARTS." where PartID = '$artistID' AND ProjectID = '$projectID' AND PartTable = '".DB_ARTISTS."'";
	$res = mysqli_num_rows (mysqli_query ($LINK, $query));
	$DEBUG && $msg .= __FUNCTION__.__LINE__. ": $query<BR>add artist to project<BR>";
	$DEBUG && $msg .= __FUNCTION__.__LINE__. ": Found $res results for artist in project<BR>";
	if ($res == 0) {
		$pairs = array (
			'ProjectID'	=> $projectID,
			'ArtistID'		=> $artistID,
			'PartTable'	=> $ARTISTS,
			'PartID'		=> $artistID,
			'GroupID'		=> null
		);
		$newID = EditTable ('insert', DB_PARTS, '', $pairs);
		$DEBUG && $msg .= __FUNCTION__.__LINE__. ": Added new part ID = $newID for artist $artistID<BR>";
		return $newID;
	} else {
		$DEBUG && $msg .= __FUNCTION__.__LINE__. ": Artist $artistID already has a part in this project<BR>";
		return FALSE;
	}
}


// =================================================================
// Add an Artist to a Group
// Make a new Part, return the ID of the new part.
// 		PartTable 	= Groups
//		PartID 		= the group ID
// 		ArtistID 	= the artist ID
// 		others		= null

function AddArtistToGroup ($artistID, $groupID) {
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;
	global $LINK;

	$DEBUG = false;
	// check to see if artist is already in the group
	$query = "select * from Parts where ArtistID = '$artistID' AND GroupID = '$groupID' AND PartTable = 'Groups'";
	$result = mysqli_query ($LINK, $query);
	$result && $arr = mysqli_fetch_array($result, MYSQLI_ASSOC);
	$DEBUG && print "AddArtistToProject:<BR>$query<BR>add artist to project<BR>";
	$DEBUG && var_dump ($arr);
	if (empty($arr)) {
		$pairs = array ('ArtistID'	=> $artistID,
			'PartTable'	=> $GROUPS,
			'PartID'	=> $groupID,
			'GroupID'	=> $groupID,
			'ProjectID'	=> null
		);
		$newID = EditTable ('insert', 'Parts', '', $pairs);
		$DEBUG && print "AddArtistToGroup: new part ID = $newID<BR>";
		return $newID;
	} else {
		$DEBUG && print "AddArtistToGroup: Artist already has a part in this group<BR>";
		return FALSE;
	}
}





// =================================================================
// DeletePicturesIfChecked
// Deletes pictures. Check boxes give the pictures[] array,
// which has which ones to delete.
// ONLY CALL THIS WITH A $PROJECT RECORD!!!
function DeletePicturesIfChecked ($record) {
	global $msg, $error;
	global $LINK;
	global $IMAGES;
	$DEBUG = false;

	// delete picture if checked
	$pix = array();
	$DEBUG && $error .= "Picture Checked Array:" . ArrayToTable ($record['picturechecked']);

	if (isset($record['picturechecked'])) {
		$pix  = $record['picturechecked'];
	}

	$DEBUG && $error .= "DeletePicturesIfChecked: Array contains " .var_dump ($record) ."<BR>";

	if (count ($pix) > 0) {
		foreach ($pix as $ID) {
			EditTable ("delete", $IMAGES, $ID, array());
			$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": Deleted Image ID=$ID<BR>";
		}
	}
}




//---------
// Project nick-names must be unique
// Check that it is unique, and if not, return 
// a unique name (adds the project ID to the nickname)
// UniqueProjectNickname ($nickname);
// Check that this project nickname isn't already in use.
// If so, modify it so it's safe
// return original, or modified nickname
function UniqueProjectNickname ($projectID, $nickname=null) {
	global $msg, $error;
	global $LINK;

	$k = 1;
	$project = FetchProject ($projectID);
	$Title = $project['Title'];

	if (!$nickname) {
		$nickname || $nickname = str_replace(" ", "_", trim($Title));
	}

	$result = FetchAllProjects ("Nickname = \"$nickname\" AND ID != \"$projectID\"");
	$k = mysqli_num_rows($result);

	if ( $k > 0) {
		$oldnickname = $nickname;
		$nickname = "{$nickname}_".time();
		// Give warning of changed name
		$nicknamewarn = FetchSnippet ("nicknamewarn");
		$subs = array("nickname"=>$nickname,
			"Title"=>$Title,
			"oldnickname"=>$oldnickname);
		$nicknamewarn = Substitutions ($nicknamewarn, $subs);
		$msg .= $nicknamewarn;
	}
	// Replace non-word chars with "_"
	$nickname = strtolower(preg_replace ("/\W+/", "_", trim($nickname)));
	return $nickname;
}


// List all groups a particular member can put projects into.
// That is, find groups he is a member or owner of.
function FetchGroupListHTML ($userID = null, $groupID, $extraHTML="") {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;
	global $LINK;

	$DEBUG = 0;

	$DEBUG && $msg .= __FUNCTION__.": userID=$userID, groupID=$groupID<BR>";
	$fp_user = $_SESSION['fp_user'];
	$userID || $userID = $fp_user;

	$groups = WhichGroups ($userID);
	foreach ($groups as $group) {
		$values[$group['ID']] = $group['Title'];
		$DEBUG && $msg .= __FUNCTION__.": {$group['ID']} = {$group['Title']}<BR>";
	}
	$listname = "GroupID";
	$checked = array($groupID);
	$groupIDList = OptionListFromArray ($values, $listname, $checked, TRUE, OPTION_LIST_IS_POPUP, $extraHTML);
	return $groupIDList;
}



// List all groups a particular member can put projects into.
// That is, find groups he is a member or owner of.
function FetchArtistProjectListHTML ($userID = null, $projectID, $extraHTML="") {
	global $Themes;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;
	global $LINK;

	$DEBUG = 0;

	$DEBUG && $msg .= __FUNCTION__.": userID=$userID, projectID=$projectID<BR>";
	$fp_user = $_SESSION['fp_user'];
	$userID || $userID = $fp_user;

	$projects = FetchArtistProjects ($userID, true);
	foreach ($projects as $project) {
		$values[$project['ID']] = $project['Title'];
		$DEBUG && $msg .= __FUNCTION__.": {$project['ID']} = {$project['Title']}<BR>";
	}
	$listname = "ProjectID";
	$checked = array($projectID);
	$projectIDList = OptionListFromArray ($values, $listname, $checked, TRUE, OPTION_LIST_IS_POPUP, $extraHTML);
	return $projectIDList;
}




//---------
// Call this function with the data OR with the ID of the project
// Give project data or ID, function
// returns the dates of the project, AND whether the project is still valid
// array (time: $fdAsTime, 
//		time: $adAsTime, 
//		text: $ActiveDeadlineShow, 
//		text: $FeaturedDeadlineShow, 
//		text:  $LastUpdateShow, 
//		text:  $ProjectDateShow,
//		bool:  $Active,
//		bool:  $Featured
//		text:	Age => days since last update, considered the creation date
//		text:	DaysToLive => days left before deletion
//	);
// Y-m-d format for dates, comes from MySQL date format!

function FetchProjectDates ($p) {
	global $msg, $error;
	global $LINK;
	global $BASEDIR, $LOGS;

	$DEBUG = 0;

	$oneday = 60 * 60 * 24;	//seconds in a day

	// Call this function with the data OR with the ID of the project
	if (!is_array($p)) {
		$projectvals = FetchRowsByValue (DB_PROJECTS, "*", "ID", $p, '', TRUE);
	} else {
		$projectvals = $p;
	}

	if (!isset($projectvals['Lifespan']))
		$projectvals['Lifespan'] = 0;
	if (!isset($projectvals['ActiveLifespan']))
		$projectvals['ActiveLifespan'] = 0;

	// Project date is beginning of project
	isset($projectvals['ProjectDate']) || $projectvals['ProjectDate'] = date("Y-m-d");
	isset($projectvals['LastUpdate'])  || $projectvals['LastUpdate'] = $projectvals['ProjectDate'];
	$pd = explode ("-", $projectvals['ProjectDate']);
	$lu = explode ("-", $projectvals['LastUpdate']);

	// Y-m-d format for dates, comes from MySQL date format!
	$luAsTime = mktime (0,0,0, $lu[1]  ,$lu[2], $lu[0]);
	$pdAsTime = mktime (0,0,0, $pd[1]  ,$pd[2], $pd[0]);

	// If last update ($lu) is BEFORE the project date, use the project date
	// This would happen if the project is slated to happen in the future,
	// and the user has been working on it, perhaps to change text info
	// or set up some pix before hand.

	if ($luAsTime < $pdAsTime) {
		$lu = explode ("-", $projectvals['ProjectDate']);
	}

	// Featured days remaining
	$projectvals['Lifespan'] > 0 ? $fdr = $lu[2] + $projectvals['Lifespan'] : $fdr = 0;

	if ($fdr > 0) {
		// "featured" deadline based on last update (not project date!)
		// Y-m-d format for dates, comes from MySQL date format!
		$fdAsTime = mktime (0,0,0, $lu[1]  ,$fdr, $lu[0]);
		$FeaturedDeadlineShow = date("F j, Y", $fdAsTime);

		/* testing
			// "featured" deadline based on project start date
			$DEBUG && $fdAsTime = mktime (0,0,0, $pd[1]  ,$pd[2] + $projectvals['Lifespan'], $pd[0]);
			$error .= __FUNCTION__.__LINE__.": Calculating from project start date for testing!<br>";
		*/

		$fd = explode ("-", date("Y-m-d", $fdAsTime));

		// if featured deadline is later than today, this is a featured project
		$featured = (($fdAsTime + $oneday) > time());



		// Active days remaining
		$projectvals['ActiveLifespan'] > 0 ? $adr = $lu[2] + $fdr : $adr = 0;

		if ($adr > 0) {
			// "active" deadline based on featured deadline
			$adAsTime = mktime (0,0,0, $fd[1]  ,$fd[2] + $projectvals['ActiveLifespan'], $fd[0]);

			$ActiveDeadlineShow = date("F j, Y", $adAsTime);

			// if active deadline is later than today, this is an active project
			$active = (($adAsTime + $oneday) > time());

			$daystolive = floor(($adAsTime + $oneday - time ()) / $oneday);
			$daystolive > 0 || $daystolive = 0;
			$daystolive = str_replace ("{daystolive}", $daystolive, FetchSnippet ("willbedeleted"));

		} else {
			$ActiveDeadlineShow = "Forever";
			$daystolive = FetchSnippet ("willnotbedeleted");
			$active = true;
			$adAsTime = time()+$oneday;	// always will be tomorrow!
		}
	} else {
		$FeaturedDeadlineShow = "Forever";
		$ActiveDeadlineShow = "Forever";
		$featured = true;
		$active = true;
		$daystolive = FetchSnippet ("willnotbedeleted");
		$adAsTime = time()+$oneday;	// always will be tomorrow!
		$fdAsTime = time()+$oneday;	// always will be tomorrow!
	}

	$LastUpdateShow = date("F j, Y", $luAsTime);
	$ProjectDateShow = date("F j, Y", $pdAsTime);

	$age = abs (floor(($luAsTime + $oneday - time ()) / $oneday));

	$r = array (
		"fdAsTime"				=> $fdAsTime,
		"adAsTime"				=> $adAsTime,
		"ActiveDeadlineShow"		=> $ActiveDeadlineShow,
		"FeaturedDeadlineShow"	=> $FeaturedDeadlineShow,
		"LastUpdateShow"		=> $LastUpdateShow,
		"ProjectDateShow"		=> $ProjectDateShow,
		"Active"				=> $active,
		"Featured"				=> $featured,
		"Age"					=> $age,
		"DaysToLive"			=> $daystolive
	);

	if ($DEBUG) {
		$t = time();
		$ad = date("F j, Y", $adAsTime);
		$fd = date("F j, Y", $fdAsTime);
		$pdAsDate = date("F j, Y", $pdAsTime);

		$featured ? $f = "featured" : $f = "---";
		$active ? $a = "active" : $a = "---";

		$error .= __FUNCTION__."<HR> " . $projectvals['Title'] . " : $f, $a<P>";
		$now = date ("Y-m-d");
		$error .= "Today : $now<BR>
				____Project Date : $pdAsDate<BR>
				____Last Update : {$projectvals['LastUpdate']}<br>
				____Featured Lifespan : {$projectvals['Lifespan']} days<BR>
				____Active Lifespan : {$projectvals['ActiveLifespan']} days<BR>
				____Featured Deadline : $fd featured=[$featured]<BR>
				____Active Deadline : $ad active=[$active]<BR>
				____Age : $age<BR>
				____Days To Live : $daystolive<BR>";
	}

	return $r;
}





//---------
// Delete Project
// If we delete a project, we also remove ALL images and stories
// which aren't in use by another project.
// Usually, this deletion is by the system, part of throwing out old stuff.
function DeleteProject ($projectID, $verbose = true) {
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $BASEDIR, $LOGS;
	global $msg, $error;
	global $LINK;

	$DEBUG = 0;
	$NODELETE = 0;

	$verbose && $msg .= "Deleted {fp:Project} #{$projectID}<BR>";
	fp_error_log(__FUNCTION__.": Deleting project #{$projectID}", 3, FP_MAINTENANCE_LOG);

	if ($NODELETE) {
		$error .= __FUNCTION__.":".__LINE__.": DELETE turned off for testing. I wanted to delete project $projectID<BR>";
	} else {
		ReassignPicturesToNewProject ($projectID);
		DeleteProjectPictures ($projectID);
		CascadeDelete ($PROJECTS, $projectID);
		DeleteImageUploadDir ($projectID);
	}
}

// Delete a project's directory in the 'input' directory inside Photos
// This will remove any pix currently uploaded, not processed, which were 
// destined for a project which has been deleted.
function DeleteImageUploadDir ($projectID = null) {
	global $msg, $error;
	global $LINK;
	$DEBUG = false;

	if (!$projectID)
		return false;

	$path = FP_DIR_MAILED_DIR . "/" . $projectID;
	if (file_exists($path)) {
		foreach (new DirectoryIterator($path) as $file) {
			if (true === $file->isFile()) {
				unlinkb($file->getPathName());
			}
		}
		rmdir($path);
		fp_error_log(__FUNCTION__.": Deleted project uploading directory #{$projectID}", 3, FP_MAINTENANCE_LOG);
	}
}

// Reassign pictures from project id=$projectID to some new project they're part of.
function ReassignPicturesToNewProject ($projectID) {
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $BASEDIR, $LOGS;
	global $msg, $error;
	global $LINK;

	$DEBUG = 0;
	$DEBUG && $msg .= __FUNCTION__."<BR>";

	$query = "SELECT * FROM $IMAGES WHERE ProjectID='$projectID'";
	$result = mysqli_query ($LINK, $query);
	$DEBUG && $msg .= __FUNCTION__.__LINE__ .": $query<BR>";

	while ($image = mysqli_fetch_array ( $result, MYSQLI_ASSOC)) {
		$imageID = $image['ID'];

		// Get images which are shown in other projects than the one to be deleted.
		$query = "SELECT * FROM $PARTS WHERE PartID = '$imageID' AND PartTable = '$IMAGES' AND ProjectID != '$projectID' LIMIT 0,1";
		$res2 = mysqli_query ($LINK, $query);

		$DEBUG && $msg .= __FUNCTION__.__LINE__ .": $query<BR>";

		if (mysqli_num_rows($res2) > 0) {
			// Change image projectID to some different project that uses the image
			$part = mysqli_fetch_array ( $res2, MYSQLI_ASSOC);
			UpdateRecord ($IMAGES, $imageID, array('ProjectID' => $part['ProjectID']));
			$DEBUG || fp_error_log(__FUNCTION__.": Reassigning image #{$imageID} to a Project #{$part['ProjectID']}.", 3, FP_MAINTENANCE_LOG);
			$DEBUG && $msg .= __FUNCTION__." : Image $imageID will be reassigned to project #{$part['ProjectID']}<BR>";
		}
	}
}

//--------
// Which pictures to delete when deleting a project?
// Choose pictures belonging to that project which don't belong to any other.
// That is, pictures whose PictureID matches the project's ID.
// We've got a nice SQL script here to do that...but I'm going to
// use PHP to do it, rather than create a temporary DB and all that.
//
// If a picture has been sold, we don't delete it. Before this function,
// we tried to reassign the picture to another project. If that failed,
// we have to leave it unassigned to any project, but not deleted!
function DeleteProjectPictures ($projectID) {
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $BASEDIR, $LOGS;
	global $msg, $error;
	global $LINK;

	$DEBUG = 0;

	$DEBUG && $msg .= __FUNCTION__."<BR>";

	$query = "SELECT * FROM $IMAGES WHERE ProjectID = '$projectID'";
	$result = mysqli_query ($LINK, $query);
	$DEBUG && $msg .= "Query: $query<BR>";
	while ($image = mysqli_fetch_array ( $result, MYSQLI_ASSOC)) {
		$ID = $image['ID'];

		// If image has a price site (meaning it has sales), don't delete!
		$s = ImageTotalSalesCount($ID);
		if ($s) {
			// Assign to no project
			$image['ProjectID'] = 0;
			$command = "update";
			$msg .= "<TT>...</TT>assigned image ".$image['Title']." to no {fp:project} (Image number: $ID)<BR>";
			$DEBUG || fp_error_log("Assigned image #{$ID} to no {fp:project}, instead of deleting it, because it has sales ($s).\n", 3, FP_MAINTENANCE_LOG);
		} else {
			$command = "delete";
			//$msg .= "<TT>...</TT>Delete image $ID<BR>";
			$DEBUG || fp_error_log("Deleted image #{$ID} because project #{$projectID} was deleted.", 3, FP_MAINTENANCE_LOG);
			$DEBUG && $msg .= "We will now delete image $ID!<BR>";
		}
		$DEBUG && $msg .= __FUNCTION__.": DO NOT DELETE..TESTING.<BR>";
		$DEBUG || EditTable ($command, $IMAGES, $ID, $image);
	}
}

// Calculate total prints sold for an image. Also useful to know
// if there are any sales for an image.
function ImageTotalSalesCount($ID) {
	$total = 0;
	$image = FetchImage($ID);
	for ($i=1;$i<= FP_MAX_LIMITED_EDITION_SETS; $i++) {
		$total += $image['amount'.$i];
	}
	return $total;
}


//------------------------
// Allow an editor to log in and download an image
// Pass project ID and entered password to check
function ProjectLogin ($projectID, $ProjectPassword) {
	global $msg, $error;
	global $LINK;
	$DEBUG = false;

	if (!(empty($username) OR empty($password))) {
		$fields = "ProjectPassword";
		$where = "WHERE ID = '$projectID' and ProjectPassword = '$ProjectPassword'";
		$result = mysqli_query ("SELECT $fields from Projects $where");
		$record = mysqli_fetch_array ($result, MYSQLI_ASSOC);
		if (empty($record)) {
			//$msg .= "Incorrect password or username.";
			//$ArtistID = '';
			return FALSE;
		} else {
			// If we get here, the password matched
			//$msg .= "Welcome, " . $record['Firstname'] . " " . $record['Lastname'] . ".";
			$DEBUG && print "OK, we have access to project ID# $projectID<BR>";
			return TRUE;
		}
	}
}

//--------------------
// If a project is updated, we note that in 
// project's update date field. This allows us
// to keep alive projects which still get new
// material.
// This should be called by 'add picture' or 'add story'
// functions.
function MarkProjectUpdated ($projectID) {
	$today = getdate();
	$month = $today['mon'];
	$mday = $today['mday'];
	$year = $today['year'];
	$LMD = "$year-$month-$mday";
	$fields = array();

	$fields['LastUpdate'] = $LMD;

	//If not project date set, put in the current date as the first time it is used.
	$project = FetchProject($projectID);
	if (($project['ProjectDate'] == '0000-00-00') OR (empty($project)))
		$fields['ProjectDate'] = $LMD;

	$fields['Timestamp'] = timestamp_to_mysqldatetime (time());

	$success = UpdateRecord (DB_PROJECTS, $projectID, $fields);
	return $success;
}





// --------------------
// Add an image to a Project 
// at the beginning of the list of pictures
// Make a new Part, return the ID of the new part.
// If we add an image to a project, we mark it as Updated
// by adding the modification date to the project.
// This way, the project stays alive if we add things to it.
// If there are more pictures than MaxPix, we remove the oldest from the project
function AddImageToProject ($ImageID, $projectID) {
	global $PHOTOS_GALLERY, $SLIDES, $THUMBNAILS, $MATTED, $ORIGINALS;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $BASEDIR, $MAILED_DIR, $LOGS;
	global $msg, $error;
	global $LINK;

	$DEBUG = 0;
	$DEBUG && $msg .= __FUNCTION__."<BR>";

	$image = FetchImage ($ImageID);
	$ArtistID = $image['ArtistID'];

	// Too many pix? If we're over the limited, remove the first in the order to make room for the new last picture
	$project = FetchProject ($projectID);
	$maxpix = $project['MaxPix'];
	$maxpix || $maxpix = 10000;	// infinite!

	$DEBUG && $msg .= __FUNCTION__.__LINE__.": Max Pictures for Project $projectID is $maxpix<BR>";

	// remove oldest pic from project if too many pix
	// $imagelist[id] = orderInProject
	$imagelist = FetchProjectImageOrder ($projectID);
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__. ArrayToTable ($imagelist);

	$imagelist = array_keys ($imagelist);
	while (sizeof($imagelist) >= $maxpix) {
		$oldest = array_shift ($imagelist);
		RemoveImageFromProject ($oldest, $projectID);
		$msg .= "Maximum number of pictures for this project is $maxpix, so I'm removing the oldest picture from the project (id=$oldest).<BR>";
		fp_error_log("AddImageToProject: RemoveImageFromProject (image #{$oldest}, project #{$projectID})", 3, FP_MAINTENANCE_LOG);
	}

	// Put this image at the END of list
	$pairs = array (
		'ProjectID'	=> $projectID,
		'ArtistID'		=> $ArtistID,
		'PartTable'	=> DB_IMAGES,
		'PartID'		=> $ImageID,
		'OrderInProject'	=> 0
	);


	$newID = EditTable ('insert', $PARTS, '', $pairs);

	$DEBUG && $msg .= "AddImageToProject: adding image ID=$ImageID, Project=$projectID, New part = $newID<BR>";

	$imagelist = FetchProjectImageOrder ($projectID);
	$DEBUG && $msg .= __FUNCTION__ .__LINE__. ": Final list of parts/images<BR>".ArrayToTable ($imagelist);

	MarkProjectUpdated ($projectID);

	return $newID;
}


// Remove an item from a project
// The $ID is the PartID, so it will match the ImageID or StoryID or whatever.
// It's NOT the ID of the part itself.
function RemoveImageFromProject ($ID, $projectID) {
	global $msg, $error;
	global $LINK;
	
	$DEBUG = false;
	$query = "DELETE FROM Parts WHERE Parts.ProjectID = $projectID AND Parts.PartID = $ID";
	$result = mysqli_query ($LINK, $query);

	$DEBUG && print "DELETE FROM Parts WHERE Parts.ProjectID = $projectID AND Parts.PartID = $ID<BR>";
}
//	SetCheckBoxesInForm
// 	$form = SetCheckBoxesInForm($form, $currvals);
// 	For each variable in $currvals, check if it evals to TRUE or FALSE and
//	set the CHECKED in the <INPUT...> accordingly.
// 	Note, we don't check every checkbox, but every var in $currvals to see if it's a
//	checkbox.

function SetCheckBoxesInForm ($form, $values) {
	// PrintArray ($values);

	foreach ($values as $var => $value) {
		//print "var = $var -> ".$value."<BR>";
		$search = '/\<\s*input\s+type\s*=\s*"?checkbox"?\s+name\s*=\s*"?'.$var.'"?\s*value\s*=\s*"?(\S*?)"?\s*\>/i';

		$c = "";
		preg_match ("/yes|1/i", $value) && $c = "CHECKED";
		$replace = '<INPUT TYPE="CHECKBOX" NAME="'.$var.'" VALUE="$1" '.$c.'>';
		// Add a hidden field to notify system a checkbox var was pass, so even if
		// returned value is unchecked (i.e. 0), we record that!
		$replace .= '<INPUT TYPE="HIDDEN" NAME="'.$var.'_CHECKBOX" VALUE="SET">';
		//echo "REPLACE STRING IS : $replace <BR>";
		$form = preg_replace ($search, $replace, $form);
	}
	return $form;

}

// FTP one file to the archives
// This requires knowing where!
// Currently, we're going to use WirePhoto
// The project ID name will be the directory
// As a photo is submitted, we'll FTP it off automatically
function FTPToArchive ($ID) {


}



// Send a list of Images by ID,
// using a form for the user to fill out
// telling where to send.
function BuildFTPForm ($IDlist, $ftp_server, $directory, $ftp_user_name, $ftp_user_pass, $SOURCEDIR) {
	global $PHOTOS_GALLERY, $THUMBNAILS, $MATTED, $ORIGINALS;

	$DEBUG = false;

	isset($SOURCEDIR) || $SOURCEDIR = $ORIGINALS;
	$list = implode (",", $IDlist);
	$page = FetchSnippet ("ftp_form");
	$vars = array (	'list' 		=> $list,
		'ftp_server' => $ftp_server,
		'directory'	=> $directory,
		'ftp_user_name'	=> $ftp_user_name,
		'ftp_user_pass'	=> $ftp_user_pass,
		'SOURCEDIR'	=> $SOURCEDIR);

	$filetable = "";
	$k = 1;
	// fetch file name and build names based on IPTC annotations
	foreach ($IDlist as $ID) {
		$i = FetchImage($ID);
		$byline = $i['Byline'];

		// get initials of credit
		$initials = "";
		preg_match_all ("/(\w+)\W*\s*/m", $byline, $names);
		foreach ($names[1] as $n) {
			$initials .= substr($n,0,1);
		}
		$newfilename = "";
		!empty($initials) && $newfilename = $initials."-";
		!empty($i['DateCreated']) && $newfilename .= $i['DateCreated']."-";
		!empty($i['Source']) && $newfilename .= $i['Source']."-";
		!empty($i['IPTCSubjectCode']) && $newfilename .= $i['IPTCSubjectCode']."-";
		!empty($i['RollID']) && $newfilename .= $i['RollID']."-";
		!empty($i['FrameID']) && $newfilename .= $i['FrameID']."-";

		$newfilename .=  ".jpg";
		$DEBUG && print "New file name: $newfilename<BR>";
		$filename = $i['URL'];
		$row = '<TR>';
		$row .= "<TD>" . $i['Title'] . '<BR><input name=ID[] type="hidden" value="'.$ID.'">';
		$row .= "<IMG SRC=\"$THUMBNAILS/" . $i['URL'] . "\"></TD>";
		$row .= '<TD><input type="checkbox" name="filenameX[]'.'" value="'  .$filename . '">'. $filename . '</TD>';
//		$row .= '<TD><input type="radio" name="filename'.$k.'[]'.'" value="' . $newfilename . '" CHECKED>' . $newfilename . '</TD>';
		$row .= '<TD><input type="text" name="filename[]'.'" size="32" maxlength="32" value="' .$newfilename. '"></TD>';
		$row .= "</TR>\n";
		$filetable .= $row;
		$k = $k + 1;
	}

	$tablehdr = "<HR><FONT size=-1><table border=0 cellpadding=5 width='100%'><tr><th align=left> </th>
				<th align=left>Use the System Name,</th>
				<th align=left>OR, enter a Name</th>\n";
	$tableend = "$filetable\n</table></FONT><HR>";
	$vars['filetable'] = $tablehdr . $tableend;
	$page = Substitutions ($page, $vars);
	return $page;
}

// FTPImages
// Send pictures by FTP, given a list of IDs
// Given the ID of the image, send it
// $SOURCEDIR is the directory inside of the photos directory 
// Usually, you want to send $ORIGINALS files, the big ones. You might
// want to send some FPO pictures, however, from the $PHOTOS_GALLERY directory.
function FTPImages ($IDlist, $filenames, $ftp_server, $directory, $newfolder, $ftp_user_name, $ftp_user_pass, $SOURCEDIR) {
	global $PHOTOS_GALLERY, $THUMBNAILS, $MATTED, $ORIGINALS;
	$DEBUG = FALSE;

	// default source directory is the big files, ORIGINALS
	isset($SOURCEDIR) || $SOURCEDIR = $ORIGINALS;
	$newfolder || $newfolder = "";

	//convert $IDlist to array of one element if it isn't one
	if (!is_array($IDlist))
		$IDlist = array ($IDlist);

	// Build array of file names to send
	$files = array ();
	$newfilenames = array ();
	$filecount = 0;
	foreach ($IDlist as $ID) {
		$image = FetchImage ($ID);
		if ($image) {
			$filename = $image['URL'];
			$filecount = array_push($files, $filename);
			$newfilenames[$filename] = $filenames[$ID];
			$DEBUG && print "--> $filename<BR>";
		}
	}

	// Send files
	$DEBUG && print "Sending $filecount files.<BR>";
	$filecount && ($results = FTPOriginalFiles ($files, $newfilenames, $ftp_server, $directory, $newfolder, $ftp_user_name, $ftp_user_pass, $SOURCEDIR ));
	$results .= FetchSnippet ("ftp_failure_suggestion");
	return $results;
}	

// Send a picture, or set, to another FTP server
// FTP (filename|array, ftp_server, ftp_user_name, ftp_user_pass
// NOTE: We're sending from the 'originals' folder in photos. 
// 		 Just list the file name, no directory info.
//		 This function provides it.
function FTPOriginalFiles ($files, $newfilenames, $ftp_server, $directory, $newfolder, $ftp_user_name, $ftp_user_pass , $SOURCEDIR ) {
	global $PHOTOS_GALLERY, $THUMBNAILS, $MATTED, $ORIGINALS;
	global $msg, $error;
	global $LINK;

	$DEBUG = false;

	$DEBUG && $error .= __FUNCTION__."<BR>";

	$transcript = "Send files by FTP to $ftp_server, $directory, $newfolder, $ftp_user_name :<BR>";

	// default source directory is the big files, ORIGINALS
	$SOURCEDIR || $SOURCEDIR = $ORIGINALS;

	//convert $files to array of one element if it isn't one
	if (!is_array($files))
		$files = array ($files);


	// set up basic connection
	$conn_id = ftp_connect("$ftp_server");

	// login with username and password
	$login_result = ftp_login($conn_id, "$ftp_user_name", "$ftp_user_pass");

	// check connection
	if ((!$conn_id) || (!$login_result)) {
		$transcript .= "Ftp connection has failed!<br>";
		$transcript .= "Attempted to connect to $ftp_server for user $ftp_user_name<br>";
		return $transcript;
	} else {
		$transcript .= "Connected to $ftp_server, for user $ftp_user_name<br>";
	}

	// switch to passive PASV mode
	if (!ftp_pasv ($conn_id, TRUE))
		$transcript .= "Could not turn on PASSIVE mode<br>";

	$d = ftp_pwd($conn_id);
	$DEBUG && print "Current directory: $d<P>";

	// change to directory
	if ($directory) {
		if (!ftp_chdir ($conn_id, $directory)) {
			$transcript .= "Ftp could not change to $directory<br>";
			return $transcript;
		}
	}

	// create a new folder
	if ($newfolder) {
		$newfolder = str_replace(" ", "_", $newfolder);

		// see if the folder already exists
		if (ftp_chdir ($conn_id, $newfolder)) {
			$transcript .= "Folder $newfolder already exists...no need to make it<br>";
		} else {
			// create the new folder because it doesn't already exist
			if (!ftp_mkdir ($conn_id, $newfolder)) {
				$transcript .= "Ftp could not create a new folder called $newfolder<br>";
				return $transcript;
			} else {
				$transcript .= "Created new folder called $newfolder<br>";
				if (!ftp_chdir ($conn_id, $newfolder)) {
					$transcript .= "Ftp could not change to $newfolder<br>";
					return $transcript;
				}
			}
		}
	}

	$d = ftp_pwd($conn_id);
	$transcript .= "Changed directory to $d<br>";
	$DEBUG && print "Current directory: $d<br>";

	foreach ($files as $source_file) {
		$d = $newfilenames[$source_file];
		if ($d) {
			$destination_file = $d;
		} else {
			$destination_file = $source_file;
		}
		$transcript .= "Sending $SOURCEDIR/$source_file to $ftp_server as $destination_file<br>";
		// upload the file
		$upload = ftp_put($conn_id, "$destination_file", "$SOURCEDIR/$source_file", FTP_BINARY);

		// check upload status
		if (!$upload) {
			$transcript .= "$source_file : Ftp upload has failed!<br>";
			$result = "Failed to send $SOURCEDIR/$source_file to $ftp_server as $destination_file<br>";
		} else {
			$transcript .=  $transcript . "Uploaded $source_file to $ftp_server/$directory as $destination_file<br>";
			$result = "Uploaded $source_file to $ftp_server/$directory as $destination_file<br>";
		}
	}

	// close the FTP stream
	ftp_quit($conn_id);

	$DEBUG && print $transcript;

	return $result;
}

// Send an image by FTP to the artist's archive/distribution site
// The image $ID let's you find everything else.
// Called by InsertLocalImages
function FTPImageToArchiveSite ($ID) {
	global $PHOTOS_GALLERY, $THUMBNAILS, $MATTED, $ORIGINALS;
	global $msg, $error;
	global $LINK;

	$DEBUG = false;

	$DEBUG && $msg .= "FTPImageToArchiveSite: begin<BR>";

	// Get FTP info from Artist record
	$image = FetchImage ($ID);
	$artist = FetchArtist($image['ArtistID']);

	// if no FTP site set, don't do it
	if (!empty ($artist['ftp_server'])) {

		(isset ($image['ObjectName'])) ? $filename = $image['ObjectName'] : $filename = $image['URL'];
		$filenames = array ($ID => $filename);	// we're only doing one file at a time, this function is called on a single image
		$DEBUG && print "--> $filename<BR>";

		$ftp_server = $artist['ftp_server'];
		(isset ($artist['ftp_proj_is_dir'])) ? ($ftp_proj_is_dir = $artist['ftp_proj_is_dir']) : ($ftp_proj_is_dir = true);
		(isset ($artist['ftp_directory'])) ? ($directory = $artist['ftp_directory']) : ($directory = "");
		(isset ($artist['ftp_user_name'])) ? ($ftp_user_name = $artist['ftp_user_name']) : ($ftp_user_name = "");
		(isset ($artist['ftp_user_pass'])) ? ($ftp_user_pass = $artist['ftp_user_pass']) : ($ftp_user_pass = "");

		// if this flag is set, use the project nickname as a folder name for the FTP
		$newfolder = "";
		if ($ftp_proj_is_dir) {
			$projectID = $image['ProjectID'];
			$project = FetchProject ($projectID);
			$newfolder = $project['Nickname'];
		}

		$SOURCEDIR = $ORIGINALS;
		$results = FTPImages ($ID, $filenames, $ftp_server, $directory, $newfolder, $ftp_user_name, $ftp_user_pass, $SOURCEDIR);
		$DEBUG && $msg .= __FUNCTION__.": tried to FTPImages ($ID, ".$filenames["$ID"].", $ftp_server, $directory, $newfolder, $ftp_user_name, $ftp_user_pass, $SOURCEDIR)<BR>";

		$DEBUG && $msg .= "FTPImageToArchiveSite: tried to FTPImages ($ID, ".$filenames["$ID"].", $ftp_server, $directory, $newfolder, $ftp_user_name, $ftp_user_pass, $SOURCEDIR)<BR>";
		$DEBUG && $msg .= "RESULTS: $results<BR>";
	} else {
		$DEBUG && $msg .= "No FTP server for this artist, did not try to send this picture (ID=$ID)<BR>";
		//$error .= __FUNCTION__.": No FTP server for this supplier, did not try to send this picture (ID=$ID)<BR>";
	}
	$error && fp_error_log($error, 3, FP_ORDER_LOG );
	$DEBUG && $msg .= "FTPImageToArchiveSite: end<BR>";
	return $results;
}


// Send an image by FTP to the artist's archive/distribution site
// The image $ID let's you find everything else.
function FTPImageToSupplier ($ID, $supplierID) {
	global $PHOTOS_GALLERY, $THUMBNAILS, $MATTED, $ORIGINALS;
	global $BASEDIR, $LOGS;
	$DEBUG = false;

	$DEBUG && print "FTPImageToSupplier: begin<BR>";

	// Get FTP info from Supplier record
	$image = FetchImage ($ID);
	$supplier = FetchSupplier($supplierID);
	$message = __FUNCTION__." ($ID, $supplierID)";	// this msg replaced, below

	// if no FTP site set, don't do it
	if (!empty ($supplier['ftp_server'])) {

		(isset ($image['ObjectName'])) ? $filename = $image['ObjectName'] : $filename = $image['URL'];
		$filenames = array ($ID => $filename);	// we're only doing one file at a time, this function is called on a single image
		$DEBUG && print "--> $filename<BR>";

		$ftp_server = $supplier['ftp_server'];
		//(isset ($supplier['ftp_proj_is_dir'])) ? ($ftp_proj_is_dir = $supplier['ftp_proj_is_dir']) : ($ftp_proj_is_dir = true);
		(isset ($supplier['ftp_directory'])) ? ($directory = $supplier['ftp_directory']) : ($directory = "");
		(isset ($supplier['ftp_user_name'])) ? ($ftp_user_name = $supplier['ftp_user_name']) : ($ftp_user_name = "");
		(isset ($supplier['ftp_user_pass'])) ? ($ftp_user_pass = $supplier['ftp_user_pass']) : ($ftp_user_pass = "");

		// if this flag is set, use the project nickname as a folder name for the FTP
		$newfolder = "";

// 		if ($ftp_proj_is_dir) {
// 			$projectID = $image['ProjectID'];
// 			$project = FetchProject ($projectID);
// 			$newfolder = $project['Nickname'];
// 		}

		$SOURCEDIR = $ORIGINALS;
		$results = FTPImages ($ID, $filenames, $ftp_server, $directory, $newfolder, $ftp_user_name, $ftp_user_pass, $SOURCEDIR);
		$message = __FUNCTION__.": tried to FTPImages ($ID, ".$filenames["$ID"].", $ftp_server, $directory, $newfolder, $ftp_user_name, $ftp_user_pass, $SOURCEDIR)<BR>";

		$DEBUG && print "FTPImageToArchiveSite: tried to FTPImages ($ID, ".$filenames["$ID"].", $ftp_server, $directory, $newfolder, $ftp_user_name, $ftp_user_pass, $SOURCEDIR)<BR>";
		$DEBUG && print "RESULTS: $results<BR>";
	} else {
		$DEBUG && print "No FTP server for this supplier, did not try to send this picture (ID=$ID)<BR>";
		$message = __FUNCTION__.": No FTP server for this supplier, did not try to send this picture (ID=$ID)<BR>";
	}
	$message && fp_error_log($message, 3, FP_ORDER_LOG );
	$DEBUG && print "FTPImageToSupplier: end<BR>";
	return $results;
}



/////////////////////////////////////////////////////////////////////
//
// Module	 : SimpleXMLParser.php
// DateTime	 : 26/07/2005 11:32
// Author	 : Phillip J. Whillier
// Purpose	: Very lightwieght "simple" XML parser does not support attributes.
//
/////////////////////////////////////////////////////////////////////

class SimpleXMLParser {

	// Find the max number of specific nodes in the XML
	function MaxElements($XMLSource, $XMLName) {
		$MaxElements = 0;
		$XMLTag = "<" . $XMLName . ">";
		$Y = $this->instr($XMLSource, $XMLTag);
		while($Y>=0) {
			$MaxElements = $MaxElements + 1;
			$Y = $this->instr($XMLSource, $XMLTag, $Y + strlen($XMLTag));
		}
		return $MaxElements;
	}

	// Parse xml to retrieve a specific element
	// Instance number is a zero based index.
	function Parse($XMLSource, $XMLName, $aInstance = 0, $Default = "") {
		$XMLLength = strlen($XMLSource);
		$XMLTag = "<" . $XMLName . ">";
		$XMLTagEnd = "</" . $XMLName . ">";
		$Instance = $aInstance + 1;

		/* Find the start of the requested instance... */
		$XMLStart = 0;

		for($x = 1; $x < $Instance + 1; $x++) {
			$Y = $this->instr($XMLSource, $XMLTag, $XMLStart);

			if ($Y >= $XMLStart) {
				$XMLStart = $Y + strlen($XMLTag);
			}
			else {
				return $Default;
			}
		}

		/* Find the end of the instance... */
		$XMLEnd = $XMLStart;
		$XMLMatch = 1;

		while($XMLMatch) {
			$c = substr($XMLSource, $XMLEnd, strlen($XMLTagEnd));
			if($c == $XMLTagEnd) {
				$XMLMatch = $XMLMatch - 1;
			}
			else {
				if (substr($c, 0, 1) == $XMLTag) {
					$XMLMatch = $XMLMatch + 1;
				}
			}
			$XMLEnd = $XMLEnd + 1;
			if ($XMLEnd == $XMLLength) {
				return $Dufault;
			}
		}
		return substr($XMLSource, $XMLStart, $XMLEnd - $XMLStart - 1);
	}

	// Helper function for finding substrings
	function instr($haystack, $needle, $pos = 0) {
		$thispos = strpos($haystack, $needle, $pos);
		if ($thispos===false)
			$thispos = -1;
		return $thispos;
	}
}

/*
======================================================================
Check if the user has the access level that lets him add records
Access Levels:
	FP_ADMINISTRATOR_LEVEL
	// Can edit galleries and exhibitions (multi-gallery administrator)
	FP_NORMAL_LEVEL
	// Can edit only exhibitions
	FP_PERSONAL_LEVEL
	// Can edit only supplier info
	FP_SUPPLIER_LEVEL

User types:
	FP_SINGLE_GALLERY_SINGLE_USER
	FP_SINGLE_GALLERY_MULTI_USER
	FP_MULTI_GALLERY_SINGLE_USER
	FP_MULTI_GALLERY_MULTI_USER
	
If there are two or fewer users, allow user creation.
*/
function UserCanAddRecord ($table, $fp_user) {
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $FP_GALLERY_TYPE;
	global $msg, $error;
	global $LINK;

	$accesslevel = FetchAccessLevel ($fp_user);

	$multiuser = array (FP_MULTI_GALLERY_MULTI_USER, FP_SINGLE_GALLERY_MULTI_USER);
	$multigroup = array (FP_MULTI_GALLERY_SINGLE_USER, FP_MULTI_GALLERY_MULTI_USER);

	$access = false;
	if ($accesslevel != FP_SUPPLIER_LEVEL) {
		switch ($table) {
			case $ARTISTS :
				$result = mysqli_query ($LINK, "SELECT ID from ".DB_ARTISTS);
				if (mysqli_num_rows($result) < 2) {
					$access = true;
				} else {
					switch ($accesslevel) {
						case FP_ADMINISTRATOR_LEVEL :
							$access = in_array ($FP_GALLERY_TYPE, $multiuser);
							break;
						default:
							$access = false;
					}
				}
				break;
			case $GROUPS :
				$result = mysqli_query ($LINK, "SELECT ID from ".DB_GROUPS);
				if (mysqli_num_rows( $result) < 1) {
					$access = true;
				} else {
					switch ($accesslevel) {
						case FP_ADMINISTRATOR_LEVEL :
							$access = in_array ($FP_GALLERY_TYPE, $multigroup);
							break;
						case FP_NORMAL_LEVEL :
							$access = in_array ($FP_GALLERY_TYPE, $multigroup);
							break;
						default:
							$access = false;
					}
				}
				break;
			case DB_PROJECTS :
				$access = ($fp_user != FP_ADMINISTRATOR);
				break;
			case DB_STORIES :
				$access = ($fp_user != FP_ADMINISTRATOR);
				break;
			case DB_SETS :
				$access = ($fp_user == FP_ADMINISTRATOR);
				break;
			case DB_SUPPLIERS :
				$access = ($fp_user == FP_ADMINISTRATOR);
				break;
			default :
				$access = in_array($accesslevel, array (FP_NORMAL_LEVEL, FP_ADMINISTRATOR_LEVEL) );
		}
	}

	return $access;
}

// ------------------------------------------
// UpdateStoryParts ($ID, $vars)
// The $vars will include an array of chosen stories for a project belonging
// to the owner of the project. The $ArtistID field
// will be of the owner of the project.
// The array is a list of IDs.
// It also includes the Artists for the project!!!
function UpdateStoryParts ($projectID, $vars) {
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;
	global $LINK;

	$DEBUG = 0;
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": Beginning :<BR>";

	// Just in case, to avoid errors, check if there's an storylist. If NOT, don't do anything!
	// This might be called from a form that has no storylist, e.g. the upload form might end up here
	// because it came through the generic handler
	// I think an empty list will still be set.
	if (!isset($vars['updatestorylist']))
		return FALSE;

	if (! $projectID)
		return FALSE;

	isset($_SESSION['fp_user']) ? $fp_user = $_SESSION['fp_user'] : $fp_user = 1;


	$artists = array_keys (FetchProjectArtists ($projectID, $fp_user, FALSE));
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": VARS".ArrayToTable ($vars);
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": ARTISTS".ArrayToTable ($artists);

	// storylist comes as a series of lists, newright1, newright2, etc.
	// newleftx is a list of stories NOT in a project
	// The numbering of the newleft/newright vars, e.g. newleft2, is arbitrary. Each left/right
	// set is for an artist in the project.
	// We can't just delete and add parts, since they probably have ordering in them which
	// will be lost if we delete the parts.
	//
	// The values of the stories include "pic_" because ID's in CSS cannot be numbers.
	// So, we use id="pic_123" and now we have to strip "pic_".

	// Build the list of stories to add to the project
	$x = 1;
	$stories_to_add = array ();
	while (isset($vars["addedRight".$x])) {
		if ($vars["addedRight$x"])
			$stories_to_add = array_merge ($stories_to_add, explode ("|", str_replace ("pic_", "", $vars["addedRight$x"])));
		$DEBUG && $msg .= __FUNCTION__.": stories_to_add : ". ArrayToTable($stories_to_add);
		$x++;
	}

	// Build the list of all stories to remove from the project
	$x = 1;
	$stories_to_remove = array ();
	while (isset($vars["removedRight".$x])) {
		if ($vars["removedRight$x"])
			$stories_to_remove = array_merge ($stories_to_remove, explode ("|", str_replace ("pic_", "", $vars["removedRight$x"])));
		$DEBUG && $msg .= __FUNCTION__.": stories_to_remove : ". ArrayToTable($stories_to_remove);
		$x++;
	}

	// string list of all stories in project for the mysql update
	$stories_to_remove = join (",", $stories_to_remove);

	// delete part records for rejected stories
	if ($stories_to_remove) {
		$query = "DELETE from Parts where ProjectID = $projectID AND PartTable = '$STORIES' and PartID in ($stories_to_remove)";
		$result = mysqli_query ($LINK, $query);
	}

	// insert new stories not already in the project
	if ($stories_to_add) {
		$storiesInProject = FetchProjectStories ($projectID);
		$stories_to_add = array_diff($stories_to_add, $storiesInProject);
		foreach ($stories_to_add as $ID) {
			// Is this story already in the project (a browser "back" could try to add an existing story)

			// Need the artist ID for ease of sort, less searching later. We assume this is set.
			$story = FetchStory ($ID);
			$ArtistID = $story['ArtistID'];
			$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": adding story ID=$ID, artistID = $ArtistID, Project=$projectID<BR>";
			// be sure the artist for this story is actually in the project
			// it could be the story was passed on, but the artist unselected.
			if (in_array($ArtistID, $artists)) {
				$pairs = array (	'ProjectID'		=> $projectID,
					'ArtistID'		=> $ArtistID,
					'PartTable'		=> $STORIES,
					'PartID'			=> $ID,
					'OrderInProject'	=> 0
				);
				$newID = EditTable ('insert', $PARTS, '', $pairs);
				$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": Adding story ID=$ID, artistID = $ArtistID, Project=$projectID<BR>";
			} else {
				$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": HEY! the story=$ID is here but not the artist=$ArtistID<BR>";
			}
		}
	}
	return TRUE;
}

// ------------------------------------------
// UpdateStoryParts ($ID, $vars)
// The $vars will include an array of chosen stories for a project belonging
// to the owner of the project. The $ArtistID field
// will be of the owner of the project.
// The array is a list of IDs.
// It also includes the Artists for the project!!!
function UpdateStoryPartsOLD ($projectID, $vars) {
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;
	global $LINK;

	$DEBUG = false;
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": Beginning :<BR>";

	if (! $projectID)
		return FALSE;

	isset($_SESSION['fp_user']) ? $fp_user = $_SESSION['fp_user'] : $fp_user = 1;

	// Just in case, to avoid errors
	if (!isset($vars['storylist']))
		$vars['storylist'] = array();

	$artists = array_keys (FetchProjectArtists ($projectID, $fp_user, FALSE));
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": ".ArrayToTable ($artists);

	$stories = $vars['storylist'];
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": ".ArrayToTable ($stories);

	// delete all parts of this project
	$query = "DELETE from Parts where ProjectID = $projectID AND PartTable = 'Stories'";
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": $query<BR>";
	$result = mysqli_query ($LINK, $query);

	// insert story parts
	foreach ($stories as $ID) {
		$story = FetchStory ($ID);
		$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__. "ID=$ID " . ArrayToTable ($story);
		$ArtistID = $story['ArtistID'];
		// be sure the artist for this story is actually in the project
		// it could be the story was passed on, but the artist unselected.
		if (in_array($ArtistID, $artists)) {
			$pairs = array ('ProjectID'	=> $projectID,
				'ArtistID'	=> $ArtistID,
				'PartTable'	=> $STORIES,
				'PartID'	=> $ID
			);
			$newID = EditTable ('insert', 'Parts', '', $pairs);
			$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": adding story ID=$ID, artistID = $ArtistID, Project=$projectID<BR>";
		} else {
			$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__.": HEY! the story=$ID is here but not the artist=$ArtistID<BR>" ;
		}
	}
	return TRUE;
}


// ------------------------------------------
// UpdateImageParts ($ID, $vars)
// The $vars will include an array of chosen images for a project belonging
// to the owner of the project. The $ArtistID field
// will be of the owner of the project.
// The array is a list of IDs.
// It also includes the Artists for the project!!!
/*
function UpdateImageParts ($projectID, $vars) {
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;
	global $LINK;
	
	$DEBUG = 0;
	$DEBUG && $msg .= __FUNCTION__ .":".__LINE__.": Beginning UpdateImageParts:<BR>";
	
	// Just in case, to avoid errors, check if there's an imagelist. If NOT, don't do anything!
	// This might be called from a form that has no imagelist, e.g. the upload form might end up here
	// because it came through the generic handler 
	// I think an empty list will still be set.
	if (!isset($vars['updateimagelist']))
		return FALSE;

	if (! $projectID)
		return FALSE;

	isset($_SESSION['fp_user']) ? $fp_user = $_SESSION['fp_user'] : $fp_user = 1;
	
	//$artists = array_keys (FetchProjectArtists ($projectID, $fp_user, FALSE));
	$project = new FPProject ($LINK, $projectID);
	$artistIDs = array_keys($project->MemberList());
	
	$DEBUG && $msg .= __FUNCTION__ .":".__LINE__.": VARS".ArrayToTable ($vars);
	$DEBUG && $msg .= __FUNCTION__ .":".__LINE__.": ARTISTS in project".ArrayToTable (array_keys($artistIDs));

	// imagelist comes as a series of lists, newright1, newright2, etc.
	// newleftx is a list of pictures NOT in a project
	// The numbering of the newleft/newright vars, e.g. newleft2, is arbitrary. Each left/right
	// set is for an artist in the project.
	// We can't just delete and add parts, since they probably have ordering in them which 
	// will be lost if we delete the parts.
	//
	// The values of the images include "pic_" because ID's in CSS cannot be numbers.
	// So, we use id="pic_123" and now we have to strip "pic_".
	
	// Build the list of images to add to the project
	$x = 1;
	$images_to_add = array ();
	while (isset($vars["addedRight".$x])) {
		if ($vars["addedRight$x"])
			$images_to_add = array_merge ($images_to_add, explode ("|", str_replace ("pic_", "", $vars["addedRight$x"])));
		$DEBUG && $msg .= __FUNCTION__.": images_to_add : ". ArrayToTable($images_to_add);
		$x++;
	}

	// Build the list of all images to remove from the project
	$x = 1;
	$images_to_remove = array ();
	while (isset($vars["removedRight".$x])) {
		if ($vars["removedRight$x"])
			$images_to_remove = array_merge ($images_to_remove, explode ("|", str_replace ("pic_", "", $vars["removedRight$x"])));
		$DEBUG && $msg .= __FUNCTION__.": images_to_remove : ". ArrayToTable($images_to_remove);
		$x++;
	}

	// string list of all images in project for the mysql update
	$images_to_remove = join (",", $images_to_remove);
	
	// delete part records for rejected images
	$query = "DELETE from Parts where ProjectID = $projectID AND PartTable = '".DB_IMAGES."' and PartID in ($images_to_remove)";
	$DEBUG && $msg .= __FUNCTION__.": all images in project = $images_in_project<BR>";
	$result = mysqli_query ($LINK, $query);

	// insert new images not already in the project
	foreach ($images_to_add as $ID) {
		$DEBUG && $msg .= __FUNCTION__ .":".__LINE__.": adding image ID=$ID, artistID = $ArtistID, Project=$projectID<BR>";
		// Need the artist ID for ease of sort, less searching later. We assume this is set.
		$image = FetchImage ($ID);
		$ArtistID = $image['ArtistID'];
		// be sure the artist for this image is actually in the project
		// it could be the image was passed on, but the artist unselected.
		if (in_array($ArtistID, $artistIDs)) {
			$pairs = array (	'ProjectID'		=> $projectID,
							'ArtistID'		=> $ArtistID,
							'PartTable'		=> $IMAGES,
							'PartID'			=> $ID,
							'OrderInProject'	=> 0
							);
			$newID = EditTable ('insert', $PARTS, '', $pairs);
			$DEBUG && $msg .= __FUNCTION__ .":".__LINE__.": UpdateImageParts: adding image ID=$ID, artistID = $ArtistID, Project=$projectID<BR>";
		} else {
			$DEBUG && $msg .= __FUNCTION__ .":".__LINE__.": HEY! the image=$ID is here but not the artist=$ArtistID<BR>";
		}
	}
	
	return TRUE;
}
*/


// ====================================================================================
// UpdateArtistParts ($id, $vars)
// Update the artist parts of a group or project. For example, this will update 
// which artists are in a group or project. Call this function when modifying a group or project.
// The $vars will include an array of chosen artists for a project. The $fp_user field
// will be of the owner of the project...he must ALWAYS be in the project,
// even if not in the array. The array is of IDs.
// HOWEVER, if the fp_user is an admin user, he MAY NOT BE IN THE PROJECT,
// AND ISN'T THE OWNER!
// If we remove an artist from a project, we must also remove all pictures of that artist, too.
// $ID is a group OR project ID!!!
// If the owner of the project changes, we must not lose that artist! Rather,
// we make him a member.
function UpdateArtistParts ($ID, $vars, $table) {
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;
	global $LINK;

	$DEBUG = 0;

	$DEBUG && $msg .= __FUNCTION__."<BR>";

	// DEFAULT TABLE IS PROJECTS
	$table || $table = $PROJECTS;

	// no param passed, do nothing.
	if (! isset ($ID))
		return FALSE;
	// If no artistlist is returned, it means the user unselected all other artists
	// for an empty list.
	// 
	// However, the artistlist could be "noedit", which means there's no meaningful data in it.
	// This can happen if we only display a list of artists in the project, but don't allow user input.
	if ($vars['artistlist'] == "noedit")
		return FALSE;

	// fetch the updated artist list
	$newArtistList = array ();
	$vars['artistlist'] && $newArtistList = $vars['artistlist'];

	// Set the current web site user, $fp_user
	isset($_SESSION['fp_user']) ? $fp_user = $_SESSION['fp_user'] : $fp_user = FP_ADMINISTRATOR;


	($DEBUG && $newArtistList) && $msg .= __FUNCTION__.__LINE__.": New Artist List:".join (", ", $newArtistList)."<BR>";

	// Reminder: $ID is the group or project ID
	switch ($table) {
		case $GROUPS :
			$myGroup = new FPGroup ($LINK, $ID );

			// Get the owner, change to new owner if necessary
			if (isset ($vars['ArtistID'])) {
				$ArtistID = $vars['ArtistID'];
				// new owner, maybe?
				if ($ArtistID != $myGroup->owner) {
					// yes, new owner. Add old owner to list of artists in project
					$newArtistList[] = $myGroup->owner;
					$DEBUG && $msg .= __FUNCTION__ .":".__LINE__ . ": Changed owners, new is $ArtistID, old was {$myGroup->owner}<br>";
					// Add the old owner to the project, like a regular member
					$myGroup->AddMembers ($myGroup->owner);
				}
			} else {
				$ArtistID = $myGroup->owner;
				$ArtistID || $ArtistID = $fp_user;	// probably an unnecessary step
			}

			// If owner not in the artistlist, add owner in.
			if (!in_array($ArtistID, $newArtistList))
				$newArtistList[] = $ArtistID;

			// if no artists (shouldn't happen) then assign the current user.
			//if (! count($newArtistList)) {
			//	$newArtistList = array ($ArtistID);
			//}

			//$oldArtistList = array_keys (FetchGroupArtists ($ID, $fp_user, FALSE));
			$oldArtistList = array_keys ($myGroup->MemberList() );

			// Get artists in the old list NOT in the new list
			// Expel the unwanted artists to the Public group
			$artistsToDelete = array_diff ($oldArtistList, $newArtistList);
			// Remove owner from the list
			unset ($artistsToDelete[$ArtistID]);
			$DEBUG && $msg .= __FUNCTION__ .":".__LINE__ . ": remove artist :" . ArrayToTable ($artistsToDelete);
			$DEBUG && $msg .= __FUNCTION__ .":".__LINE__ . ": remove artist :" . $a;
			$myGroup->ExpelMembers ($artistsToDelete);

			// Add those in the new list but not in the old list
			$artistsToAdd = array_unique(array_diff ($newArtistList, $oldArtistList));
			$DEBUG && $msg .= __FUNCTION__ .":".__LINE__ . ": add new artists :" . ArrayToTable ($artistsToAdd);

			// insert artist parts
			$myGroup->AddMembers ($artistsToAdd);
			break;

		case $PROJECTS :
		/*
				- If owner has changed, set new owner, add old owner to project
				- remove owner from list of new members
				- Add parts for new members
				- Remove members not in the list
		*/

			$myProject = FetchProject ( $ID );
			$DEBUG && $msg .= __FUNCTION__ .":".__LINE__ . ": Current owner is {$myProject['ArtistID']}<BR>";
			$DEBUG && $msg .= __FUNCTION__ .":".__LINE__ . ": NEW artist list :" . ArrayToTable ($newArtistList);

			// Get the owner, change to new owner if necessary
			if (isset ($vars['ArtistID'])) {
				$ArtistID = $vars['ArtistID'];
				// new owner, maybe?
				if ($ArtistID != $myProject['ArtistID']) {
					$newArtistList[] = $myProject['ArtistID'];
					// Add the old owner to the project, like a regular member
					AddArtistToProject ($myProject['ArtistID'], $ID);
					SetProjectOwner ($ID, $ArtistID);
					$DEBUG && $msg .= __FUNCTION__ .":".__LINE__ . ": Changed owners, new is $ArtistID, old was {$myProject['ArtistID']}<br>";
				}
			} else {
				$ArtistID = $myProject['ArtistID'];
				$ArtistID || $ArtistID = $fp_user;	// probably an unnecessary step
			}

			// Remove owner from new list of artists, in case it snuck in.
			$keys = array_keys($newArtistList, $ArtistID);
			foreach ($keys as $k) {
				unset ($newArtistList[$k]);
			}
			$DEBUG && $msg .= __FUNCTION__ .":".__LINE__ . ": Owner is $ArtistID<BR>";

			// if no artists (shouldn't happen) then assign the current user.
			//if (! count($newArtistList)) {
			//	$newArtistList = array ($ArtistID);
			//}

			// Get list of artists in the project before changes, excluding the owner
			$oldArtistList = array_keys (FetchProjectArtists ($ID, $fp_user, true));



			$DEBUG && $msg .= __FUNCTION__ .":".__LINE__ . ": OLD artist list :" . ArrayToTable ($oldArtistList);
			$DEBUG && $msg .= __FUNCTION__ .":".__LINE__ . ": NEW artist list :" . ArrayToTable ($newArtistList);
			// Get artists in the old list NOT in the new list
			$artistsToDelete = array_diff ($oldArtistList, $newArtistList);
			foreach ($artistsToDelete as $a) {
				$DEBUG && $msg .= __FUNCTION__ .":".__LINE__ . ": remove artist :" . $a . "<BR>";
				RemoveArtistFromProject ($ID, $a);
			}
			// Add those in the new list but not in the old list
			$artistsToAdd = array_unique(array_diff ($newArtistList, $oldArtistList));
			$DEBUG && $msg .= __FUNCTION__ .":".__LINE__ . ": add new artists :" . ArrayToTable ($artistsToAdd);

			// insert artist parts
			foreach ($artistsToAdd as $artistID) {
				$DEBUG && $msg .= __FUNCTION__ .":".__LINE__ . ": Add artist to project : $artistID joins project #{$ID}<br>";
				AddArtistToProject ($artistID, $ID);
			}

	}
	return TRUE;
}


// ====================================================================================
// RemoveArtistFromProject ($projectID, $artistID)
// If we remove an artist from a project, we must also remove all pictures of that artist, too.
// Return true if any changes made, false if not.
function RemoveArtistFromProject ($projectID, $artistID) {
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;
	global $LINK;

	// delete parts for this project which involve this artist.
	$where = "ProjectID = '$projectID' AND ArtistID = '$artistID'";

	if (DeleteRow ($PARTS, $where)) {
		$artist = FetchArtist ($artistID);
		$project = FetchProject ($projectID);
		$msg .= $artist['Firstname'] . " " . $artist['Lastname'] . " removed from project <b>". $project['Title'] . "</b> (#". $projectID . ")<BR>";
		return true;
	}
	return false;
}

// ====================================================================================
// Confirm an action with a dialog box

function ConfirmWindow ($table, $ID, $command, $extra = "") {
	global $msg, $error;
	global $LINK;
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $error, $msg;

	if ($ID) {
		$record = getRecord ($table, $ID);
		$record = HTMLEntitiesAllValues ($record);
		switch ($table) {
			case $ARTISTS :
				$description = "Artist " . $record['Firstname'] . " " . $record['Lastname'];
				break;
			case $GROUPS :
				$description = "Group " . $record['Title'] . " :<BR>" . $record['Description'];
				break;
			case $PROJECTS :
				$description = "Project " . $record['Title'] . " :<BR>" . $record['Description'];
				break;
			case $IMAGES :
				$description = "Picture " . $record['Title'] . " : " . $record['URL'];
				break;
			case $STORIES :
				$description = "Story " . $record['Title'] . " :<BR>" . $record['Summary'];
				break;
			default :
				break;
		}
		if (! empty($record)) {
			$confirmtext = FetchSnippet ("confirmwindow");
		} else {
			$confirmtext = "Oops. Go back. Sorry.<BR>";
		}
		$confirmtext = Substitutions ($confirmtext, array ("description"=>$description, "command" => $command, "extra" => $extra));
		$confirmtext = Substitutions ($confirmtext, $record);
	} else {
		$confirmtext = "";
	}
	return $confirmtext;
}

// ====================================================================================
// UserHasAccess
// Check user's level, see if he has access to can do the operation
// If there's no ID, return false
function UserHasAccess ($table, $ID = 0, $public = true) {
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;
	global $LINK;

	if (! $ID)
		return false;

	if ($ID) {
		isset($_SESSION['fp_user']) && $fp_user = $_SESSION['fp_user'];
		$accesslevel = FetchAccessLevel ($fp_user);
		$ownerID = $fp_user;
		$record = GetRecord ($table, $ID);
		$access = true;

		// admin always has access
		if ($accesslevel == FP_ADMINISTRATOR)
			return true;

		// Restrict access to database to only the user's own record and public records if admin level < FP_ADMINISTRATOR
		switch ($table) {
			case $ARTISTS :
				$access = ( $record['ID'] == $fp_user );
				break;
			case $GROUPS :
				$access = ( $record['ArtistID'] == $fp_user );
				break;
			default :
				$access = ( $record['ArtistID'] == $fp_user );
				break;
		}

		($accesslevel > FP_PERSONAL_LEVEL) && $access = false;
	} else {
		$access = false ;
	}
	return $access;
}


// Get Total of space allocated to artists
// Useful for knowing how much is available to allocate to new artists
// We DO NOT add in the amount allocated to user $ID if sent
function GetStorageStats ($ID = null) {
	global $msg, $error;
	global $LINK;

	$query = "SELECT SUM(Storage) FROM " . DB_ARTISTS . " WHERE ID != " . FP_ADMINISTRATOR;
	$ID && $query .= " AND ID != $ID";
	$result = mysqli_query ($LINK, $query);
	$record = mysqli_fetch_array($result, MYSQLI_ASSOC);
	$allocated = array_shift($record);

	$query = "SELECT Storage FROM " . DB_ARTISTS . " WHERE ID = " . FP_ADMINISTRATOR;
	$record = mysqli_fetch_array(mysqli_query ($LINK, $query), MYSQLI_ASSOC);
	$total = array_shift($record);

	return array ($allocated, $total);
}


// Find all groups a member is a part of.
function WhichGroups ($member = null) {
	global $Themes;
	global $IMAGES, $PRICES, $PRICESETS, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;
	global $LINK;

	$DEBUG = 0;

	$groups = array();
	$fp_user = $_SESSION['fp_user'];
	$member || $member = $fp_user;

	if ($member != FP_ADMINISTRATOR) {
		$where = "WHERE ( $PARTS.ArtistID='$member' AND $PARTS.PartTable = '$GROUPS') OR ($GROUPS.ArtistID = '$member' OR $GROUPS.ID = '" . PUBLIC_GROUP_ID . "' OR $GROUPS.Public > 0 )";
		$order = "ORDER BY SortTitle";
		$query = "SELECT DISTINCT Title, $GROUPS.ID, $PARTS.PartID, TRIM(LEADING 'The ' from $GROUPS.Title) as SortTitle FROM $GROUPS LEFT JOIN $PARTS ON $GROUPS.ID = $PARTS.PartID $where $order";
	} else {
		$query = "SELECT * FROM $GROUPS";
	}
	$result = mysqli_query ($LINK, $query);
	$DEBUG && $error .= __FUNCTION__ .":<BR>" . $query . "<BR>";
	while ($record = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
		$groups[$record['ID']] = $record;
		$DEBUG && $error .= __FUNCTION__.": --> ".$record['Title']."<BR>";
	}
	$public = FetchGroup (PUBLIC_GROUP_ID);
	$public && $groups = array_merge ($groups, array (PUBLIC_GROUP_ID => $public));
	return $groups;
}


// Set the default project for an artist
function SetDefaultProject ($ArtistID, $projectID) {
	global $Themes;
	global $IMAGES, $PRICES, $PRICESETS, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;
	global $LINK;
	$artist = FetchArtist ($ArtistID);
	$artist['ProjectID'] = $projectID;
	$result = EditTable ("update", $ARTISTS, $ArtistID, $artist);
	return $result;
}


// Given an artist's ID, make a new project for that artist
// which is the artist's default gallery
// This is made when a new artist joins the system, deleted when he's deleted
function MakeArtistProject ($ArtistID, $groupID = null) {
	global $Themes;
	global $PHOTOS_GALLERY, $SLIDES, $THUMBNAILS, $MATTED, $ORIGINALS;
	global $IMAGES, $PRICES, $PRICESETS, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;
	global $LINK;

	$DEBUG = false;
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": MakeArtistProject ($ArtistID)<BR>";
	if (!$ArtistID)
		return false;

	$artist = FetchArtist ($ArtistID);
	$artistname = $artist['Firstname'] . " " . $artist['Lastname'];
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": MakeArtistProject ($ArtistID) = $artistname<BR>";

	// Create the new project
	$vars = array();
	$vars['ProjectDate'] = date("Y-m-d");
	$vars['Title'] = Substitutions (FP_DEFAULT_GALLERY_NAME, array("name" => $artistname));
	$vars['Lifespan'] = 0;
	$vars['ActiveLifespan'] = 0;
	$vars['City'] = $artist['City'];
	$vars['Country'] = $artist['Country'];
	$vars['ArtistID'] =$ArtistID;
	$vars['GroupID'] =$groupID;


	$projectID = EditTable ("insert", DB_PROJECTS, "", $vars);
	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": New gallery is projectID=$projectID<BR>";

	//be sure the owner gets a "part", is part of project!
	AddArtistToProject ($ArtistID, $projectID);

	// Update the ProjectID in the artist database
	$vars['ProjectID'] = $projectID;
	EditTable ("update", DB_ARTISTS, $ArtistID, $vars);

	//UpdateArtistParts ($ID, $vars, $PROJECTS);
	//UpdateImageParts ($ID, $vars);
	//UpdateStoryParts ($ID, $vars);

	$nickname = str_replace ( " ", "_", $artistname);
	$vars['Nickname'] = UniqueProjectNickname ($projectID, $nickname);

	// Add this stuff to the new project
	EditTable ("update", DB_PROJECTS, $projectID, $vars);
	MarkProjectUpdated ($projectID);
	// DON'T call Maintenance...this is called BY Maintenance!
	//Maintenance();
	return $projectID;
}


// If a project is updated, we note that in 
// project's update date field. This allows us
// to keep alive projects which still get new
// material.
// This should be called by 'add picture' or 'add story'
// functions.



// If $ID has a value, get values for it.
function BuildDefaultInputForm ($action, $table, $formaction, $ID, $AccessLevel) {
	global $Themes;
	$DEBUG = false;
	global $IMAGES, $PRICES, $PRICESETS, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $PHOTOS_GALLERY, $THUMBNAILS, $MATTED, $ORIGINALS;
	global $error, $msg;

	$DEBUG && $msg .= "Begin 475:BuildDefaultInputForm<BR>";

	$fieldnames = FetchFieldNames ($table);
	$list = "<INPUT TYPE=hidden NAME=\"ID\" VALUE=\"$ID\">";
	$currvals = array ();
	$IMG = "<!-- no image -->\n";
	if (! empty ($ID) ) {
		$currvals = FetchRowsByValue ($table, '*', 'ID', $ID, '', TRUE);
		if (isset($currvals['URL']))
			$IMG = URLtoIMG ("$PHOTOS_GALLERY/" .  $currvals['URL']);
	}
	$excludefields = array('timestamp','id');
	$filler = "";
	foreach ($fieldnames as $k) {
		$k = trim($k);
//		isset($currvals[$k]) ? $var = $currvals[$k] : $var = FIELDPREFIX . $k . FP_FIELDSUFFIX;
		isset($currvals[$k]) ? $var = $currvals[$k] : $var = FP_FIELDSTART . $k . FP_FIELDSUFFIX;
		if (! in_array(strtolower(trim($k)), $excludefields)) {
			$filler = $var;
			$var = trim($var);
			$line = "<tr>
			<td>$k</td>
			<td><input type=\"text\" name=\"$k\" value=\"$filler\">
			</td>
			</tr>";
			$list .= $line;
		}
	}

	$formname = strtolower("entry_form_" . $table . "_" . $AccessLevel);
	$form = FetchSnippet ($formname);

	$formname = strtolower("entry_form_" . $table);
	empty($form) && $form = FetchSnippet ($formname);

	empty ($form) && ($form = FetchSnippet("default_entry_form"));

	$form = Substitutions ($form, array ('$listing' => $list,
		'$formaction' => $formaction,
		'$table' => $table,
		'IMG'	=> $IMG,
		'$action' => $action
	));

	return $form;
}

// Validate entered values for a project
// For example, be sure the featured lifespan is within config settings, etc.

function ValidateProjectValues ($vars) {
	global $Themes;
	global $IMAGES, $PRICES, $PRICESETS, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $PHOTOS_GALLERY, $THUMBNAILS, $MATTED, $ORIGINALS;
	global $error, $msg;
	global $LINK;

	$DEBUG = false;

	$DEBUG && $msg .=  __FUNCTION__.": Vars".ArrayToTable ($vars);

	if ($vars['ArtistID']) {
		$user = new FPUser ($LINK, $vars['ArtistID']);

		// "0" lifespan means "infinite"
		if ($vars['Lifespan'] == null)
			$vars['Lifespan'] = $user->get_property('DefaultLifespan') ? $user->get_property('DefaultLifespan') : 0;

		if (MAX_DAYS_FEATURED > 0 )
			($vars['Lifespan'] > MAX_DAYS_FEATURED or $vars['Lifespan'] == 0)  && $vars['Lifespan'] = MAX_DAYS_FEATURED;

		if (MAX_DAYS_ACTIVE > 0 )
			($vars['ActiveLifespan'] > MAX_DAYS_ACTIVE or $vars['ActiveLifespan'] == 0) && $vars['ActiveLifespan'] = MAX_DAYS_ACTIVE;

		if (MAX_PROJECT_PICTURES > 0 )
			($vars['MaxPix'] > MAX_PROJECT_PICTURES or $vars['MaxPix'] == 0) && $vars['MaxPix'] = MAX_PROJECT_PICTURES;
	}
	return $vars;
}


// Validate entered values for a project
// For example, be sure the featured lifespan is within config settings, etc.

function ValidateSupplierValues ($vars) {
	global $error, $msg;

	$DEBUG = false;
	$DEBUG && $msg .=  __FUNCTION__.": Vars".ArrayToTable ($vars);
	
	$vars['Country'] = strtoupper(substr ($vars['Country'],0,2));

	return $vars;
}



//--------------
// BTPFetchRecords
// Fetch records for BuildTablePage
// NOTICE: some of the queries I originally did, in the $tables/$sets/$wheres are now superceded, below.
function BTPFetchRows ($queryset, $table, $projectID, $ArtistID, $limit=0) {
	global $Themes;
	global $IMAGES, $PRICES, $PRICESETS, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES, $SUPPLIERS, $SALES, $PAYPAL, $PAYMENTS;
	global $msg, $error;
	global $LINK;

	$DEBUG = 0;	// NOTE: there's second $DEBUG setting below.
	$DEBUG && $msg .= "Begin " . __FUNCTION__ ."<BR>";

	$accesslevel = FetchAccessLevel ($ArtistID);
	$DEBUG && $msg .=  __FUNCTION__ .":Table=$table, queryset=$queryset, ArtistID = $ArtistID, Accesslevel = $accesslevel<BR>";


	$tables = array (
		DB_PROJECTS	=> $PROJECTS,
		DB_ARTISTS		=> $ARTISTS,
		DB_IMAGES		=> "$IMAGES, $PROJECTS",
		'LightTable'		=> $IMAGES,
		'Stories'		=> $STORIES,
		'Parts'		=> "$PARTS, $ARTISTS",
		'ProjectArtists'	=> "$PARTS, $ARTISTS",
		'ProjectStories'	=> "$PARTS, $SNIPPETS",
		'ProjectImages'	=> "$PARTS, $IMAGES",
		DB_SALES		=> "$SALES, $ARTISTS, $IMAGES, $PAYPAL, $PAYMENTS"
	);

	$sets 	= array (
		DB_PROJECTS		=> "*, TRIM(LEADING 'The ' from Title) as SortTitle",
		DB_ARTISTS		=> "*, CONCAT_WS(' ', Firstname, Lastname) AS Artist_Fullname",
		DB_IMAGES		=> "$IMAGES.*, $PROJECTS.Title AS ProjectTitle",
		"LightTable"		=> "*",
		"Stories"		=> "$STORIES.Title, ID, concat(substring(Story,1,60),'...') AS Lead",
		"Parts"		=> "Lastname, Firstname, PartID, PartTable",
		"ProjectArtists"	=> "$ARTISTS.ID as ArtistID, $ARTISTS.Lastname, $ARTISTS.Firstname, $PARTS.ProjectID, $PARTS.PartID, $PARTS.PartTable, $PARTS.ID AS ID",
		"ProjectStories"	=> "Snippets.ID as SnippetsID, Title, Text, URL, $PARTS.ProjectID, $PARTS.PartID, $PARTS.PartTable, $PARTS.ID AS ID",
		"ProjectImages"	=> "$IMAGES.ID as ImagesID, Title, RollID, FrameID, $PARTS.ProjectID, $PARTS.PartID, $PARTS.PartTable, $PARTS.ID AS ID",
		DB_SALES		=> "distinct $PAYPAL.*, $SALES.*, $SALES.id as SaleID, $PAYMENTS.*, $PAYPAL.payment_date, STR_TO_DATE($PAYPAL.payment_date, '%T %b %d, %Y') as paymentdatetime"
	);

	$wheres = array (
		DB_IMAGES 		=> "$IMAGES.ArtistID = $ArtistID AND $IMAGES.Type = 'normal' AND $PROJECTS.ID = $IMAGES.ProjectID",
		'LightTable' 	=> "$IMAGES.ArtistID = $ArtistID AND $IMAGES.Type = 'normal'",
		'Stories' 		=> "$STORIES.ArtistID = $ArtistID",
		'Parts'		=> "$PARTS.ProjectID = $projectID",
		'ProjectArtists'	=> "$PARTS.ProjectID = $projectID AND $ARTISTS.ID = $PARTS.PartID AND $PARTS.PartTable = $ARTISTS",
		'ProjectStories'	=> "$PARTS.ProjectID = $projectID AND $SNIPPETS.ID = $PARTS.PartID AND $PARTS.PartTable = 'Snippets'",
		'ProjectImages'	=> "$PARTS.ProjectID = $projectID AND $IMAGES.ID = $PARTS.PartID AND $PARTS.PartTable = '$IMAGES'",
		DB_SALES		=> "$ARTISTS.ID = '$ArtistID' and $IMAGES.ArtistID = '$ArtistID' and $SALES.item_id = $IMAGES.ID and $PAYPAL.txn_id = $SALES.txn_id and $PAYMENTS.SaleID = $SALES.id and $PAYMENTS.Payee = 'artist'"
	);


	$queryset || $queryset = $table;
	isset($sets[$queryset]) ? $set = $sets[$queryset] : $set = "*";
	isset($wheres[$queryset]) ? $where = " WHERE " . $wheres[$queryset] : $where = "";
	isset($tables[$queryset]) ? $querytable = $tables[$queryset] : $querytable = $table;

	// Sort the rows in different ways for each table
	// - put Featured/Active stuff first
	// - order pix by project as well
	$order="";

	$DEBUG = 0;

	//$DEBUG && $error .= __LINE__.$queryset;
	switch ($queryset) {
		case $GROUPS :
		// default is only show groups the user owns
			$order = "ORDER BY SortTitle";
			$query = "SELECT $GROUPS.*, substring(Description,1,300) AS Summary,TRIM(LEADING 'The ' from $GROUPS.Title) as SortTitle from $GROUPS where $GROUPS.ArtistID = '$ArtistID' $order";
			// admin users can see all groups
			$accesslevel < 2 && $query = "SELECT *, substring(Description,1,300) AS Summary,TRIM(LEADING 'The ' from Title) as SortTitle from $GROUPS $order";
			$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": query=$query<BR>";
			$result = mysqli_query ($LINK, $query);
			break;
		case $PROJECTS :
		// Do it this way because the FetchAllProjects gets calculated fields
		// not just ordinary fields.
			$order = "ORDER BY $GROUPS.Title, SortTitle";
			// If the user isn't admin, should not have access to all projects, no?
			// User should see his projects, and projects in which he has a part
			//$accesslevel > FP_ADMINISTRATOR ? $where = "$PROJECTS.ArtistID = $ArtistID" : $where = "";
			//$result = FetchAllProjects ($where, $order);
			//$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": BTPFetchRows, queryset=$queryset, where = $where<BR>";
			if ($accesslevel >= FP_NORMAL_LEVEL) {
				$query = "SELECT DISTINCT $PROJECTS.ID, $PROJECTS.*, substring($PROJECTS.Description,1,300) AS Summary, TRIM(LEADING 'The ' from $PROJECTS.Title) as SortTitle, $GROUPS.Title as GroupTitle from $PARTS, $PROJECTS, $GROUPS WHERE (IF ($PROJECTS.OwnerAccessOnly>0, $PROJECTS.ArtistID = '{$ArtistID}',$PARTS.PartTable = '$ARTISTS' and $PARTS.ArtistID = '{$ArtistID}' and $PARTS.ProjectID = $PROJECTS.ID) OR $PROJECTS.ArtistID = '{$ArtistID}') AND IF($PROJECTS.GroupID>0, $GROUPS.ID = $PROJECTS.GroupID, $GROUPS.ID = ".PUBLIC_GROUP_ID.") $order";

				// working version w/o owneronlyaccess
				//$query1 = "SELECT DISTINCT $PROJECTS.ID, $PROJECTS.*, substring($PROJECTS.Description,1,300) AS Summary, TRIM(LEADING 'The ' from $PROJECTS.Title) as SortTitle, $GROUPS.Title as GroupTitle from $PARTS, $PROJECTS, $GROUPS WHERE ($PARTS.PartTable = '$ARTISTS' and $PARTS.ArtistID = '{$ArtistID}' and $PARTS.ProjectID = $PROJECTS.ID OR $PROJECTS.ArtistID = '{$ArtistID}') AND IF($PROJECTS.GroupID>0, $GROUPS.ID = $PROJECTS.GroupID, $GROUPS.ID = ".PUBLIC_GROUP_ID.") $order";
				
			} else {
				$query = "SELECT DISTINCT $PROJECTS.ID, $PROJECTS.*, substring($PROJECTS.Description,1,300) AS Summary, TRIM(LEADING 'The ' from $PROJECTS.Title) as SortTitle, $GROUPS.Title as GroupTitle from $PARTS, $PROJECTS LEFT JOIN $GROUPS ON IF($PROJECTS.GroupID>0, $GROUPS.ID = $PROJECTS.GroupID, $GROUPS.ID = ".PUBLIC_GROUP_ID.") $order";
			}

			$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": query=$query<BR>";
			$result = mysqli_query ($LINK, $query);
			break;
		case $IMAGES :
			$order = " ORDER BY ProjectSortTitle, SortTitle";
			
			$DEBUG && $msg .= __FUNCTION__.__LINE__.": ProjectID = $projectID<BR>";
			$limit
				? $limit = "LIMIT $limit"
				: $limit = "";
			
			if ($accesslevel == FP_ADMINISTRATOR_LEVEL) {
				$projectID 
					? $byproject = " WHERE $IMAGES.ProjectID = '$projectID' "
					: $byproject = "";
				$query = "SELECT DISTINCT $IMAGES.*, if($PROJECTS.Title != '', TRIM(LEADING 'The ' from $PROJECTS.Title), 'no {fp:project}') AS ProjectSortTitle, TRIM(LEADING 'The ' from $IMAGES.Title) as SortTitle, $PROJECTS.Title as ProjectTitle FROM $IMAGES LEFT JOIN $PROJECTS ON $IMAGES.ProjectID = $PROJECTS.ID $byproject $order $limit";
			} else {
				$projectID 
					? $byproject = " $IMAGES.ProjectID = '$projectID' AND "
					: $byproject = "";
				$query = "SELECT DISTINCT $IMAGES.*, if($PROJECTS.Title != '', TRIM(LEADING 'The ' from $PROJECTS.Title), 'no {fp:project}') AS ProjectSortTitle, TRIM(LEADING 'The ' from $IMAGES.Title) as SortTitle, $PROJECTS.Title as ProjectTitle FROM $IMAGES LEFT JOIN $PROJECTS ON $IMAGES.ProjectID = $PROJECTS.ID WHERE $byproject $IMAGES.ArtistID = $ArtistID AND $IMAGES.Type = 'normal' $order $limit";
			}
			$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": query=<br>$query<BR>";
			$result = mysqli_query ($LINK, $query);
			break;
		case $STORIES :
			$order = " ORDER BY SortTitle";
			$query = "SELECT $STORIES.*, if($PROJECTS.Title != '', $PROJECTS.Title, 'no {fp:project}') AS ProjectTitle, TRIM(LEADING 'The ' from $STORIES.Title) as SortTitle FROM $STORIES LEFT JOIN $PROJECTS ON $STORIES.ProjectID = $PROJECTS.ID WHERE $STORIES.ArtistID = $ArtistID $order";
			$result = mysqli_query ($LINK, $query);
			$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": query=$query<BR>";
			break;
		case $ARTISTS :
			$order = " ORDER BY Lastname";
			// If the user isn't admin, should not have access to all artists, no?
			$accesslevel > FP_ADMINISTRATOR  ? $where = "WHERE ID = $ArtistID" : $where = "";
			$query = "SELECT $set FROM $querytable $where $order";
			$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": BTPFetchRows, Artist = $ArtistID<BR>$query<BR>";
			$result = mysqli_query ($LINK, $query);
			break;
		case $PRICESETS :
			$order = " ORDER BY Title";
			$accesslevel > FP_ADMINISTRATOR_LEVEL ? $where = "WHERE ISNULL(ImageID) OR ArtistID = $ArtistID OR ArtistID = " . FP_ADMINISTRATOR . " OR ISNULL(ArtistID) " : $where = "" ;
			$query = "SELECT $set FROM $querytable $where $order";
			$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": BTPFetchRows, Artist = $ArtistID<BR>$query<BR>";
			$result = mysqli_query ($LINK, $query);
			break;
		case $SUPPLIERS :
			$order = " ORDER BY Name";
			// If the user isn't admin, should not have access to all suppliers, no?
			// In any case, only show a person his stuff if he's not admin user
			$accesslevel > FP_ADMINISTRATOR  ? $where = "WHERE ArtistID = $ArtistID" : $where = "";
			$query = "SELECT $set FROM $querytable $where $order";
			$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": BTPFetchRows, Artist = $ArtistID<BR>$query<BR>";
			$result = mysqli_query ($LINK, $query);
			break;
		case $SALES :
			$order = " ORDER BY paymentdatetime";
			$query = "SELECT $set FROM $querytable $where $order";
			$DEBUG && $msg .= "Query: $query<BR>";
			$result = mysqli_query ($LINK, $query);
			break;
		default :
			$query = "SELECT $set FROM $querytable $where $order";
			$result = mysqli_query ($LINK, $query);
	}

	return $result;
}


//--------------
// FetchFormats
// Fetch formatting for BuildTablePage
function BTPFetchFormat ($queryset, $table) {
	global $Themes;
	global $error, $msg;

	$DEBUG = false;

	$DEBUG && $msg .= "BEGIN ".__FUNCTION__."<BR>";

	$pageformat = FetchSnippet ("list_tableformat_{$table}");
	$pageformat || $pageformat = FetchSnippet ('list_tableformat');
	$fieldhdr = "Items";
	$rowheader = FetchSnippet ("listitem_{$table}_header");
	$rowformat = FetchSnippet ("listitem_$table");
	$listheader = FetchSnippet ("listitem_{$table}_list_header");
	$listfooter = FetchSnippet ("listitem_{$table}_list_footer");
	$rowtotalsformat = FetchSnippet ("listitem_{$table}_totals");
	// If DEFAULT ROW FORMAT, then show all fields in the formlets
	if ( empty($rowformat) ) {
		$fieldnames = FetchFieldNames ($table);
		$fieldhdr = ArrayToTableH ($fieldnames);
		$fieldhdr = implode (",", $fieldnames);
		// Legacy style for replacing vars, e.g. $myvar in text
		$fieldnamesX = ArrayPrefix($fieldnames, FIELDPREFIX);

		$DEBUG && $msg .= ArrayToTable ($fieldnames);

		$rowformat = Substitutions ($rowformat, array ('$fields' => implode (", ", $fieldnamesX)));
		// New style, with a prefix/suffix, e.g. {myfield}
		$fieldnamesX = ArrayWrap($fieldnames, FP_FIELDSTART, FP_FIELDSUFFIX);
		$rowformat = Substitutions ($rowformat, array ('$fields' => implode (", ", $fieldnamesX)));
	}
	$pageformat = Substitutions ($pageformat, array (
		'LIST_HEADER' => $listheader,
		'LIST_FOOTER' => $listfooter
	));
	return array($pageformat, $rowformat, '', '', '',  $rowheader, $rowtotalsformat);
}


// --------------------
// BTPBuildRows
// Build the table of rows (apply the formatting to the data)
// $projectresult is the result of MYSQL SELECT.

function BTPBuildRows ($projectresult, $format, $queryset, $table, $projectID, $fp_user) {
	global $Themes;
	global $IMAGES, $PRICES, $PRICESETS, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES, $SALES, $PAYMENTS, $SUPPLIERS;
	global $FP_GALLERY_TYPE;
	global $msg, $error;
	global $LINK;


	$DEBUG = 0;

	$DEBUG && $msg .= basename(__FILE__).":".__FUNCTION__ .":".__LINE__ . ": Begin ".__FUNCTION__." ($projectresult, $format, $queryset, $table, $projectID, $fp_user)<BR>";
	$ArtistID = $fp_user;
	list ($pageformat, $rowformat, $cellformat, $fieldhdr, $numcols, $header, $rowtotalsformat) =  $format;
	$checked = array ('yes' => "CHECKED", 'no' => '');
	$checkedbinary = array ('1' => "CHECKED", '0' => '');
	$list = array();
	$colnum = 0;
	$cells = array();
	$hiddenmark = FetchSnippet ("listitem_hidden_mark");
	$publicmark = FetchSnippet ("listitem_public_mark");
	$IsPrivateFlag = FetchSnippet ("listitem_is_private_mark");
	$prev = "";
	$next = "";
	$VIDEOFLAG = FetchSnippet ("listitem_Images_video_flag");

	// warning about deleting images that have sales
	$imagewarning = FetchSnippet ("image_with_sale_delete_warning");

	$totals = array ();	// use for totals, of course

	$TIMER = 0;
	$TIMER && $starttime = microtime(true);

	// Preload common values for each table display
	switch ($table) {
		case DB_IMAGES :
			// preload list of price set names
			$pricesetNames = FetchPriceSetNames ();
			break;
	}

	// Build the rows
	if ($projectresult) {

		$rowArtist = FetchArtist ($fp_user);
		$Fullname = $rowArtist['Fullname'];
		$fp_user_accesslevel = $rowArtist['AccessLevel'];


		while ($row = mysqli_fetch_array($projectresult, MYSQLI_ASSOC)) {
			//$DEBUG && print __FILE__ ." ". __FUNCTION__." ".__LINE__." ".ArrayToTable ($row);
			
			array_walk ($row, 'trimME');

			// Replace blank "title" fields with "untitled."
			$row['Title'] || $row['Title'] = "Untitled";
			$row['Fullname'] = $Fullname;
			$row['DELETE_MESSAGE'] = FetchSnippet ("listitem_delete_message_{$table}");
			isset ($row['Params']) ? $params = DecodeArrayFromDB($row['Params']) : $params = array ();
			switch ($table) {
			
				case $ARTISTS :
				// To Do: show total storage used in total line
				//$row["Storage_Formatted"] = number_format ($row["Amount"], 2, FP_DECIMAL, FP_THOUSANDS);
				// The Edit/Delete snippets use "Title"
					$row['Title'] = $row['Artist_Fullname'];
					if ($row['Confirmed']) {
						$row['Confirmed'] = "";
					} else {
						$row['Confirmed'] = "{fp:Not Confirmed}, ";
						// time remaining before deletion
						$delay = FP_MAXCONFIRMTIME - (time() - strtotime($row['Timestamp']));	// seconds
						$row['Confirmed'] .= MinutesToTime (floor($delay/60)) . " remaining | ";
					}
					// Single-User system, cannot add/delete users
					if (FP_INSTALLATION_TYPE == FP_INSTALLATION_PRIVATE && ($FP_GALLERY_TYPE == FP_MULTI_GALLERY_SINGLE_USER || $FP_GALLERY_TYPE == FP_SINGLE_GALLERY_SINGLE_USER)) {
						$row['DELETE_COMMAND'] = Substitutions(FetchSnippet ("listitem_nodelete"), $row);
					} else {
						if ($row['ID'] == FP_ADMINISTRATOR) {
							$row['DELETE_COMMAND'] = Substitutions(FetchSnippet ("listitem_nodelete"), $row);
						} else {
							$row['DELETE_COMMAND'] = Substitutions(FetchSnippet ("listitem_delete"), $row);
						}
					}

					$row['Storage'] == 0 && $row['Storage'] = "{fp:unlimited}";

					break;

				case $PROJECTS :
					$projectdates = FetchProjectDates ($row['ID']);
					$DEBUG && $msg .= $row['Title'] . " (" . $row['ID'] . ")<BR>";
					$row["featured_truth"] = ShowIfTrue($projectdates['Featured']);
					$row["active_truth"] = ShowIfTrue($projectdates['Active']);
					$row["PublicPrivateCheckMark"] = ($row['Public'] ? $hiddenmark : $publicmark);
					$row["IsPrivateFlag"] = ($row['client_list'] ? $IsPrivateFlag : "");
					
					$row["ClientListFormatted"] = ($row['client_list'] ? Substitutions(FetchSnippet("listitem_client_list"), $row)  : "");
					
					
					// field for sorting in a table
					$row["PublicPrivateCheckMarkSortField"] = ($row['Public'] ? "0" : "1");
					if ($row['ID'] == PUBLIC_GROUP_ID) {
						$row['DELETE_COMMAND'] = Substitutions(FetchSnippet ("listitem_nodelete"), $row);
					} else {
						$row['DELETE_COMMAND'] = Substitutions(FetchSnippet ("listitem_delete"), $row);
					}
					
					$row["Summary"] = strip_tags($row["Summary"]);
					
					// THEME_LIST field popup
					$checked = array ($params[FP_PARAM_GALLERY_THEME]);
					$ThemeList = OptionListFromArray (
						$Themes->FP_THEMES_LIST,
						"param_".FP_PARAM_GALLERY_THEME,
						$checked,
						TRUE,
						OPTION_LIST_IS_POPUP,
						//$extrahtml="onChange='submit()';", 	// the submit is done with jQuery
						$extrahtml="id='autosubmit_{$row['ID']}'",
						$extraline = array(
						'value'		=> "0",
						'checked'	=> "",
						'label'		=> "Use Gallery Theme")
					);
					$row['THEME_MENU'] = $ThemeList;
					$row['THEME_MENU_SORTFIELD'] = $params[FP_PARAM_GALLERY_THEME];

					// GROUP_MENU field popup
					$GroupList = FetchGroupListHTML (
						$fp_user, 
						$row['GroupID'],
						$extrahtml="id='autosubmit_b_{$row['ID']}' class='my-widget'"
					);
					
					$row['GROUP_MENU'] = $GroupList;

					$myGroup = new FPGroup ($LINK, $row['GroupID']);
					$row['GROUP_MENU_SORTFIELD'] = trim($myGroup->title);

					break;

				case $IMAGES :
					if (ImageFilesExist ($row['URL']) && (!empty($row['URL']))) {
						$ID = $row['ID'];
						$URL = $row['URL'];
						$b = basename($URL);
						$params = DecodeArrayFromDB($row['Params']);
						if (GetParam($params,FP_PARAM_IMAGE_IS_VIDEO)) {
							$row['VIDEOFLAG'] = $VIDEOFLAG;
							$row['HIDE_IF_VIDEO'] = 'style="display:none;"';
							$row['SHOW_IF_VIDEO'] = 'style="display:inline;"';
						} else {
							$row['VIDEOFLAG'] = '';
							$row['HIDE_IF_VIDEO'] = 'style="display:inline;"';
							$row['SHOW_IF_VIDEO'] = 'style="display:none;"';
						}


						$row['$URL'] = $b;
						$row['$IMG'] = URLtoIMG ($THUMBNAILS . "/" . $b);
						$row['$IMG_THUMBNAIL'] = URLtoIMG ($THUMBNAILS . "/" . $b);
						$row['$IMG_NORMAL'] = URLtoIMG ($PHOTOS_GALLERY . "/" . $b);

						$row['$PHOTOS_GALLERY'] = $PHOTOS_GALLERY;

						$row['$THUMBNAILS'] = $THUMBNAILS;
						$row['$ORIGINALS'] = $ORIGINALS;

						$dimensions = GetImageSizeByID ($row['ID']);
						$row['DIMENSIONS'] = Substitutions(FetchSnippet ("show_image_dimensions"), $dimensions);
						if (GetParam ($params, FP_PARAM_IMAGE_OFFSITE)) {
							$row['DIMENSIONS_PRINT'] = Substitutions(FetchSnippet ("show_image_print_dimensions"),  $dimensions);
						} else {
							$row['DIMENSIONS_PRINT'] = "";
						}
						
						if (GetParam ($params, FP_PARAM_IMAGE_IS_FIXED_SIZE)) {
							$row['DIMENSIONS_FIXED'] = Substitutions(FetchSnippet ("show_image_fixed_dimensions"),  $dimensions);
						} else {
							$row['DIMENSIONS_FIXED'] = "";
						}

						$row['CATALOGNUMBER'] = CatalogNumber ($row['ID']);

						// If an image is undeletable, don't allow deletion.
						// This could be because the image has a price set id, which means it's been sold!

						// default
						$row['DELETE_COMMAND'] = Substitutions(FetchSnippet ("listitem_delete"), $row);

						if ($fp_user != FP_ADMINISTRATOR && $row["EditionsLocked"])
							$row['DELETE_COMMAND'] = Substitutions(FetchSnippet ("listitem_nodelete"), $row);

						if ($fp_user == FP_ADMINISTRATOR && $row["EditionsLocked"])
							$row['DELETE_COMMAND'] = Substitutions(FetchSnippet ("listitem_admin_delete"), $row);
						$row['DELETE_COMMAND'] = Substitutions($row['DELETE_COMMAND'], array ("desc"=>$imagewarning));

						// If admin user, then also get artist info
						if ($fp_user == FP_ADMINISTRATOR) {
							$rowArtist = FetchArtist ($row['ArtistID']);
							$row['Fullname'] = $rowArtist['Fullname'];
						}
						// For sale status -- show whether for sale. (show as tip)
						$notip = (FP_ADMIN_CONTROLS_ARTIST_SALES && $fp_user != FP_ADMINISTRATOR);
						$row['ImageSaleStatus'] = ImageOnSaleStatus($row['ID'], null, true, $notip);
						
						if ( (!FP_ALL_METADATAEDIT_ACCESS || FP_ADMIN_CONTROLS_PRINT_FILE) && $fp_user != FP_ADMINISTRATOR) {
							$metadataeditor = "";
						} else {
							$metadataeditor = FetchSnippet("listitem_Images_metadataeditor");
						}
						$row['PriceSetName'] = $pricesetNames[$row['PriceSetID']];
						$row['PriceSetName'] || ($row['PriceSetName'] = FetchSnippet("msg_using_parent_priceset"));
						
						// ====================
						// PRICING SETTINGS
						// These live in PriceSet. Records in PriceSet with ImageID's belong to actual images. Really.
						// A record is locked after the first sale (or locked by user). Until then it is unlocked.
						$pricesetID = FetchPriceSetID ($ID);
						$salesinfo = ImageSalesInfo ($ID);
						$printsizes = $salesinfo['printsizes'];
						$totalsold = $salesinfo['totalsold'];
						//$outsidesales = $params[FP_PARAM_AMOUNT_SOLD_OUTSIDE];
						// Use this outside sales figure --- it has been checked against the edition size and corrected if necessary
						$outsidesales = $salesinfo['outsidesales'];
						if ($outsidesales) {
							$row["ShowOutsideSales"] = "$outsidesales";
						} else {
							$row["ShowOutsideSales"] = "";
						}

						$customeditionsize = GetParam ($params, FP_PARAM_IMAGE_EDITION_SIZE);
						$editionsize = GetImageEditionSize ($ID, $salesinfo['totaledition'], $record);
						$pseditionsize = $salesinfo['priceseteditionsize'];

						// Display of edition size (If edition size = 0, then it's an open (unlimited) edition)
						if ($editionsize > 0) {
							$row["ShowEditionSize"] = "$editionsize";
						} else {
							$row["ShowEditionSize"] = FetchSnippet ("unlimited_edition_en");
						}
						$editionsizeshow = $row["ShowEditionSize"];
						
						// Original artwork?
						if (GetParam($params,FP_PARAM_IMAGE_IS_ORIGINAL_ART)) {
							$artworkname = GetParam($params,FP_PARAM_IMAGE_ARTWORK_NAME);
							$artworkname || $artworkname = "Artwork";
							$row['ORIGINAL_ART'] = str_ireplace ('{ARTWORK_NAME}', $artworkname, FetchSnippet("listitem_Images_original_art"));
						}
						
						// Is the price set locked? If not, show pricing pop-up of pricing lists.
						$locked = $row['EditionsLocked'];
						if (!$row['EditionsLocked']) {
							// show lock
							$usingparent = FetchSnippet("msg_using_parent_priceset");
			
							$priceSetList = FetchPriceSetsHTML ($rowArtist, $row['PriceSetID'], $usingparent,null,"id=\"PriceSetID{$row['ID']}\" class=\"ajaxsave\" title=\"{$row['ID']}\"");
							
							//FetchPriceSetsHTML ($artistID, $pricesetID='0', $blankentry = "", $includeCustom=false, $extrahtml = "") 
							
							$pricesetname = "";
							$lockpriceset = FetchSnippet("input_form_images_pricesetlocker");
						} else {
							$priceSetList = CatalogNumber ($ID);
							$pricesetname = $priceSetList;
							$lockpriceset = FetchSnippet("msg_priceset_locked");
						}

			
						// Get next print number, based on total sold
						if ($totalsold < $editionsize) {
							$nextPrintNum = $totalsold + 1;
						} else {
							$nextPrintNum = "{fp:editionclosed}";
						}
			

						// Editions Form
						$EditionsForm = FetchSnippet ('input_form_Images_editioning');
						if ($row['EditionsLocked']) {
							$enterCustomSize = FetchSnippet ('msg_locked_no_change_ed_size');
						} else {
							$enterCustomSize = FetchSnippet ('input_form_Images_editioning_custom');
						}

						// Image quick editor dialog
						// If edition is locked, don't allow changing of edition size
						if ($row['EditionsLocked']) {
							$qeShowHideEdSize = "none";
							$qeShowHideEdNoEditSize = "block";
						} else {
							$qeShowHideEdSize = "block";
							$qeShowHideEdNoEditSize = "none";
						}

						$SalesForm = FetchSnippet('image_quick_editor_sales_settings');							
						$SalesForm = Substitutions ($SalesForm, array (
							"SalesInfoOutput"			=> $SalesInfoOutput,
							"editionsize"				=> $editionsize,
							"pseditionsize"				=> $pseditionsize,
							"sold"					=> $totalsold,
							"remaining"				=> $editionsize - $totalsold,
							"ENTER_CUSTOM_EDITION_SIZE"		=> $enterCustomSize,
							'param_'.FP_PARAM_IMAGE_EDITION_SIZE 	=> $customeditionsize,
							"NEXTPRINTNUMBER"			=> $nextPrintNum,
							"PRICESETLIST"				=> $priceSetList,
							"ID"					=> $row['ID'],
							"SHOWHIDE_EDITION_SIZE"		=> $qeShowHideEdSize,
							"SHOWHIDE_NOEDIT_EDITION_SIZE"		=> $qeShowHideEdNoEditSize
							));
						$row['SALESFORM'] = $SalesForm;
						// =====
						
						
					} else {
						$error .= "{fp:image} {$row['Title']} is missing its files. ";
						DeleteImage ($row['ID']);
						$row = array();
					}
					break;

				case $PRICESETS :
				// if this is an image priceset, not a general one, mark it to avoid confusion!
				// ID = 1 is the "not for sale"
					if ($row['ImageID'] > 0) {
						$row['DIM_ME'] = 'style="color:#888;"';
						$row['Title'] = "&bull; {fp:priceset} for #" . CatalogNumber ($row['ImageID']);
						$row['DELETE_COMMAND'] = Substitutions(FetchSnippet ("listitem_delete"), $row);
					} elseif ($row['ID'] == 1) {
						$row['DELETE_COMMAND'] = Substitutions(FetchSnippet ("listitem_nodelete"), $row);
					} else {
						$row['DELETE_COMMAND'] = Substitutions(FetchSnippet ("listitem_delete"), $row);
					}
					
					// If the set is for a unique image, show that image's name.
					
					$image = FetchImage ($row['ImageID']);
					$row['ImageID']
					? $row['Image_Name'] = $image['Title']
					: $row['Image_Name'] = "";
					
					$row['DUPLICATE_COMMAND'] = Substitutions(FetchSnippet ("listitem_duplicate"), $row);

					break;

				case $SALES :
				
					// Get all entries for this sale
					// Note, Sales database id is "id", not "ID".
					$items = FetchRowsByValueAsArray (DB_PAYMENTS, "ID, SaleID, Payee, Amount", "SaleID", $row['id'], 'ID', 'Payee');
					$vals = array();
					foreach ($items as $item)
						{
						$vals[$item['Payee']] = $item['Amount'];
						}
					$vals['total']= array_sum($vals);
					$summary = Substitutions (FetchSnippet("listitem_Sales_sale_summary"), $vals);
					$row['SALES_SUMMARY'] = $summary;
					
					
					$row["Amount_Formatted"] = number_format ($row["Amount"], 2, FP_DECIMAL, FP_THOUSANDS);
					$row["mc_gross_Formatted"] = number_format ($row["mc_gross"], 2, FP_DECIMAL, FP_THOUSANDS);
					$row["item_desc"] = urldecode($row["item_desc"]);
					break;

				case $GROUPS :
				// Cannot delete group if:
				// - it is the main gallery (ID=PUBLIC_GROUP_ID)
				// - the user is a personal gallery owner
					if ($row['ID'] == PUBLIC_GROUP_ID || $fp_user_accesslevel >= FP_PERSONAL_LEVEL) {
						$row['DELETE_COMMAND'] = Substitutions(FetchSnippet ("listitem_nodelete"), $row);
					} else {
						$row['DELETE_COMMAND'] = Substitutions(FetchSnippet ("listitem_delete"), $row);
					}
					
					$row["Summary"] = strip_tags($row["Summary"]);
					
					$checked = array ($row['Theme']);
					$ThemeList = OptionListFromArray (
						$Themes->FP_THEMES_LIST,
						"Theme",
						$checked,
						TRUE,
						OPTION_LIST_IS_POPUP,
						//$extrahtml="onChange='submit()';", 	// the submit is done with jQuery
						$extrahtml="id='autosubmit_{$row['ID']}'",
						$extraline = array(
						'value'		=> "0",
						'checked'	=> "",
						'label'		=> "Use Gallery Theme")
					);
					$row['THEME_MENU'] = $ThemeList;

					break;
					
				case $SUPPLIERS :
					$row['Title'] = $row['Name'];
					$row['DELETE_COMMAND'] = Substitutions(FetchSnippet ("listitem_delete"), $row);
					break;
				default :
					$row['DELETE_COMMAND'] = Substitutions(FetchSnippet ("listitem_delete"), $row);
					break;
			}

			// $row might have been emptied, above, if it was invalid
			if ($row) {
				$row['EDIT_COMMAND'] = Substitutions(FetchSnippet ("listitem_edit"), $row);

				// TOTALS
				foreach ($row as $k => $v) {
					is_numeric ($v) && $totals[$k] += $v;
				}


				// Fill in all params.
				foreach ($params as $k => $v) {
					$row['param_'.$k] = GetParam ($params, $k);
				}

				if ($cellformat) {
					$cells[] = Substitutions ($cellformat, $row);
					$colnum++;
					if ($colnum >= $numcols) {
						$colnum = 0;
						// wrap in row format
						$oneline = implode ("\n", $cells);
						$list[] = str_replace ('text', $oneline, $rowformat);
						//$list[] = Substitutions ($rowformat, array('text' => $oneline));
						$cells = array ();
					}
				} else {
					$list[] = Substitutions ($rowformat, $row);
				}
			}
		}
	}

	if ($TIMER) {
		$t = round (microtime(true) - $starttime, 3);
		if ($t>0.01) {
			$error .= __FUNCTION__.__LINE__.": Time elapsed: ".round (microtime(true) - $starttime, 3) . "<BR>";
		}
	}


	// if a row of CELLS is filled out but not finished, finish it up.
	if (count ($cells)) {
		$cells[] = Substitutions ($cellformat, $row);
		$oneline = implode ("\n", $cells);
		$list[] = Substitutions ($rowformat, array('text' => $oneline));
	}

	isset($ProjectTitle) || $ProjectTitle = "no {fp:project}";

	$listing = implode ("\n", $list);

		// ITEM SUBSTITUTIONS
	$listing = Substitutions ($listing, array (
		'table' => $table,
		'queryset' => $queryset,
		'ProjectID' => $projectID,
		'ArtistID' => $ArtistID,
		'ProjectTitle' => $ProjectTitle,
		'fp_user' => $fp_user
	));

	// Don't do this:
	//$listing = $header.$listing;

	// TOTALS ROW
	$row = $totals;
	$rowc = array ();
	foreach ($row as $k => $v) {
		$rowc[$k."_currency"] = number_format ($v, 2, FP_DECIMAL, FP_THOUSANDS);
	}
	$totalrow = Substitutions ($rowtotalsformat, $row);
	$tr = Substitutions ($rowtotalsformat, $rowc);
	$totalsrow = Substitutions ($totalrow, $rowc);



	//Get User Info
	$artistinfo = FetchRowsByValue ($ARTISTS, '*', 'ID', $fp_user, 'Lastname', TRUE);

	//Get Project Info
	$projectinfo = FetchRowsByValue ($PROJECTS, '*', 'ID', $projectID, 'Title', TRUE);
	$ProjectIDList = OptionList ($PROJECTS, "ProjectID", "ID", "Title", "ArtistID = $fp_user", $projectID, "Title", OPTION_LIST_IS_POPUP);
	$fieldlist = FieldOptionList ($table, "multieditfieldname", array('ID', 'Active', 'Featured'));

	// pricing setup on the Pictopia print sales system
	//isset($pps) || $pps = "default";
	isset ($pps) || $pps = "<!-- do not sell prints -->\n";
	( $pps == "" ) || $pps = "<!-- do not sell prints -->\n";
	
	// ----------------------------------
	// HEADER STUFF:
	// Get header stuff: projectlists, etc.
	$projectlist = OptionList (DB_PROJECTS, "ProjectID", "ID", "Title", "ArtistID = $fp_user", $checked, "Title", false, $extrahtml="");
	
	// Priceset listing
	//$x = "Using parent {fp:project} pricing ({ParentPricesetTitle}";
	$usingparent = FetchSnippet("msg_using_parent_priceset");
	$PriceSetList = FetchPriceSetsHTML ($fp_user, 0, $usingparent, null, 'id="PriceSetID"' );
	$projectinfo['PRICESET_LIST'] = $PriceSetList;


	
	// ----------------------------------
	// PAGE SUBSTITUTIONS
	$page = $pageformat;
	$page = Substitutions ($page, array (
		'table'			=> $table,
		'queryset'		=> $queryset,
		'fp_user'		=> $fp_user,
		'pps'			=> $pps,
		'ArtistID'		=> $ArtistID,
		'ProjectID'		=> $projectID,
		'header'		=> $header,
		'listing'		=> $listing,
		'totals'		=> $totalsrow,
		'fieldlist'		=> $fieldlist,
		'fieldnames'		=> $fieldhdr,
		'projectlist'		=> $projectlist,
		'metadataeditor'	=> $metadataeditor
	));
	$page = ReplaceAllSnippets ($page);
	$page = Substitutions ($page, $artistinfo);
	$page = Substitutions ($page, $projectinfo);
	return $page;
}

// ---------------------------------------
// Show a list of records
// Use admin info to decide which version to show
// $msg might be blank
function BuildTablePage ($queryset, $table, $projectID, $fp_user, $limit=0) {
	global $msg, $error;
	global $LINK;
	//global $Themes;
	$format = BTPFetchFormat ($queryset, $table);
	
	// Build Projects pop-up list for this artist
	// Useful to limit a long list, such as Images
	// If admin, can see all projects in system

	$extraline  = array ('label'=>"All {fp:projects} (slow)",'value'=>"", 'checked'=>'');
	if ($fp_user == FP_ADMINISTRATOR) {
		$checked = array ($projectID);
		$ProjectList = OptionList (DB_PROJECTS, 'ProjectID', 'ID', 'Title', '', $checked, 'Title', false, $extrahtml='class="autosubmit"',$extraline);
	} else {
		// Projects for normal artist
		$checked = array ($projectID);
		$ProjectList = OptionList (DB_PROJECTS, 'ProjectID', 'ID', 'Title', 'ArtistID = "'.$fp_user.'"', $checked, 'Title', false, $extrahtml='class="autosubmit"',$extraline);
	}
	
	$f = array ( "PROJECTSLIST"	=> $ProjectList );	

	$rows = BTPFetchRows ($queryset, $table, $projectID, $fp_user, $limit);
	$page = BTPBuildRows ($rows, $format, $queryset, $table, $projectID, $fp_user);
	$page = Substitutions ($page, $f);

	//dump unused $variables
	// DELETE THIS? DUNNO...IT IS TAKEN CARE OF LATER... MIGHT CAUSE TROUBLE TO DELETE!
	$page = preg_replace ('/(\\' . FIELDPREFIX . '\w+\b)/','',$page);

	// Add link to add new records
	if (UserCanAddRecord ($table, $fp_user)) {
		$addnewrecord = AddNewRecordLink ($table);
	} else {
		$addnewrecord = "<!-- no permission to add new records-->\n";
	}
	$page = $addnewrecord . $page;

	return $page;
}

// My special sorter, ignores "The" at the beginning of the value
// Assume (key=>value), sorts on value
// So, "The Cow" come before "Cup".

function TitleSort ($list) {
	global $Themes;
	uasort ($list, "SortNoThe");
	return $list;
}

// sorter function for TitleSort function
global $Themes;
function SortNoThe ($a, $b) {
	global $Themes;
	$a = preg_replace ('/^the /i', "", $a);
	$b = preg_replace ('/^the /i', "", $b);
	if ($a == $b)
		return 0;
	return ($a < $b) ? -1 : 1;
}

function GroupsPopup ($where = null) {
	global $Themes;
	global $msg, $error;
	global $LINK;

	$grouplist = FetchAllGroupsArrays ();
	$extrahtml = "";
	$extraline  = array ('label'=>"Go to group...",'value'=>"", 'checked'=>'CHECKED');
	$values = array ();
	foreach ($grouplist as $group) {
		$v = "projects.php?GroupID=" . $group['ID'];
		$values[$v] = $group['Title'];
	}
	$extrahtml = "OnChange=\"javascript:changeLocation(this)\"";
	$groupPopup = OptionListFromArray ($values, "GroupList", array(), TRUE, FALSE, $extrahtml, $extraline);
	$groupPopup || $groupPopup = "No Groups";	// this won't happen
	return $groupPopup;
}



// ------------------------------------------------------------
// Check if necessary folders exist; create them if necessary
/* 
	Used by setup.php, also
	The $printout param is so setup.php can show results.
	===== Directories that must exist
	This is used by ConfirmSetup to be sure that 
	all working directories actually exist. 

	Any constant starting with "FP_DIR_" is an FP directory
	and it should exist. So, while the following is a good list
	we build the list from the constants in the system.
		
	FP_SYSTEM_IMAGES
	FP_DIR_CONFIG
	FP_DIR_TMP
	FP_DIR_LOG
	FP_DIR_USER
	FP_DIR_USER_IMAGES
	FP_SUBDIR_AUDIO
	FP_DIR_NEWSLETTER
	FP_SUBDIR_SLIDESHOW_AUDIO
	
	FP_DIR_GALLERY_IMAGES
	FP_DIR_SLIDES_IMAGES
	FP_DIR_THUMBNAILS_IMAGES
	//FP_DIR_MATTED
	//FP_DIR_FRAMED
	FP_DIR_ORIGINAL_IMAGES
	FP_DIR_AV
	FP_DIR_MAILED_DIR
	FP_DIR_PROCESSEDDIR
	FP_DIR_PROCESSED_PHOTOS
	FP_DIR_PROCESSED_SLIDES
	FP_DIR_PROCESSED_THUMBNAILS
	FP_DIR_PROCESSED_MATTED
	FP_DIR_PROCESSED_FRAMED
	
	AS_SENDER_QUEUE_DIR
*/

function MakeDirectoriesFromFPConstants ($DEBUG = FALSE) {
	global $error, $msg;
	global $BASEDIR, $PICTURE_DIRECTORIES;

	// this now set as a param of this function
	//$DEBUG = FALSE;

	// Fetch directories and permissions
	// This list may not be a complete list of all diretories,
	// but it is sufficient for setting permissions
	// However!!! This is not a list of directories, but of the constants
	// which keep the name of the directory.
	// Why? Because it's easier to change one constant and have all
	// directories move than to fix it here, too.
	// Each entry is a CONSTANT_NAME,PERMISSION
	$dc = file (FP_DIR_CONFIG.'/directories_constants.txt', FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES );
	$dp = array ();
	foreach ($dc as $d) {
		$r = explode (',', $d);
		$dp[trim($r[0])] = trim ($r[1]);
	}

	// Get list of constants starting with FP_DIR_
	$c = preg_grep ("/^\FP_DIR_/", array_keys(get_defined_constants ()) );
	foreach ($c as $cname ) {
		$dirname = constant($cname);
		// Remove $BASEDIR from $dirname, if it's there
		$dirname = str_replace ("{$BASEDIR}/", "", $dirname);
		//substr($dirname,0,1) == "/" || $dirname = $BASEDIR . '/' . $dirname;
		// $DEBUG && $msg .= "$cname = $dirname ( {$dp[$cname]} ) <BR>";

		if (!file_exists("$dirname")) {
			if (isset ($dp[$cname])) {
				$p = octdec($dp[$cname]);
				$pp = $dp[$cname];
			} else {
				$p = 0755;
				$pp = "0755";
			}
			mkdir ($dirname, $p) || $error .= __FUNCTION__.": Cannot make new directory $dirname ($p)<br>" ;
			file_exists("$BASEDIR/$dirname/index.html") || copy ("$BASEDIR/setup/files/index.html", "$BASEDIR/$dirname/index.html");
			$msg .= "--> Copied index.html to $BASEDIR/$dirname<br>";
			$DEBUG && $msg .= "...Creating directory ".constant($cname)." as {$pp}<BR>";
		}
		// CANNOT CHANGE OWNERSHIP WITH SAFE MODE ON
		//if (isset ($dp[$cname]) && ($dp[$cname] != 0755) )
		//	chmod ($dirname, octdec($dp[$cname])) || $error .= "Failed to CHMOD $dirname<BR>";

	}
}

// Install .htaccess and index.html files into photo directories to protect the data
function ProtectPhotoDirectories ($setupdir = "setup") {
	global $error, $msg;
	global $BASEDIR, $PICTURE_DIRECTORIES;

	// install .htaccess from setup into Originals to protect it
	$d = array ( FP_SUBDIR_SLIDESHOW_AUDIO, FP_DIR_ORIGINAL_IMAGES, FP_DIR_AV, FP_DIR_MAILED_DIR, FP_DIR_PROCESSEDDIR);
	foreach ($d as $dir) {
		file_exists("$BASEDIR/$dir/.htaccess") || copy ("$setupdir/files/htaccess.txt", "$BASEDIR/$dir/.htaccess");
		$msg .= "--> set up $dir with .htaccess<br>";
	}

	// install index.html into each photo directory
	foreach ($PICTURE_DIRECTORIES as $dir) {
		file_exists("$BASEDIR/$dir/index.html") || copy ("$setupdir/files/index.html", "$BASEDIR/$dir/index.html");
		$msg .= "--> set up $dir with index.html<br>";
	}

}


/*
Return the a subscription link for the menu bar.
$user is an $FPUser object
If this is a private server installation, then only the admin can subscribe. Show only if they haven't signed up.
If it's a public server installation, then users can subscribe. Show only if they haven't signed up.
*/
function GetSubscribeMenuItem ($user) {
	global $error, $msg;

	$link = FetchSnippet ("menu_subscribe_link");
	$menuitem = "";
	switch (FP_INSTALLATION_TYPE) {
		case FP_INSTALLATION_PRIVATE :
			if ($user->ID == FP_ADMINISTRATOR && !$user->getval('SubscriptionID')) {
				$menuitem = $link;
			} else {
				$menuitem = "";
			}
			break;

		case FP_INSTALLATION_PUBLIC :
			if ($user->ID != FP_ADMINISTRATOR && !$user->getval('SubscriptionID')) {
				$menuitem = $link;
			} else {
				$menuitem = "";
			}
			break;
	}
	return $menuitem;
}

// Should we show the subscription form or link for a given user?
function ShowSubscriptionLink($userID) {
	global $error, $msg;
	global $LINK;

	$user = new FPUser ($LINK, $userID);
	$show = false;
	switch (FP_INSTALLATION_TYPE) {
		case FP_INSTALLATION_PRIVATE :
			if ($user->ID == FP_ADMINISTRATOR)
				$show = true;
			break;

		case FP_INSTALLATION_PUBLIC :
			if ($user->ID != FP_ADMINISTRATOR)
				$show = true;
			break;
	}
	return $show;
}


// Show the sales figures for a price set record.
// When a sale is made, the record becomes unique to a picture.
// Show sales per picture size, so artist knows what's selling.
// If the TotalEditionSize is zero, it's an open edition.
function SalesFiguresForm ($record, $outsidesales) {

	// show sales figures fields
	$row = FetchSnippet ("priceSet_sales_figures_row");
	$x = FetchSnippet ("priceSet_sales_figures_form");
	$rows = "";
	$es = array ();
	$edsizes = $record["EditionSize"];
	$sizes = $record["Size"];
	$amount = $record["Amount"];
	$outsideSales = $record['outsidesales'];
	$record['TotalEditionSize'] > 0
	? $edHdr = FetchSnippet ("priceset_sales_figures_ltd_ed_title")
	: $edHdr = FetchSnippet ("priceset_sales_figures_open_ed_title");

	$i = 0;
	$total = 0;
	foreach ($sizes as $size) {
		// get dimensions for show
		if ($size > 0) {
			$showsize = PrintSizeForDisplay (FetchImage($record['ImageID']), $size);
			//$edsizes >= 1 ? $es = $edsize : $es = "unlimited edition";
			$es = "";
		} else {
			$showsize = "&mdash;";
			$es = "&mdash;";
		}
		$arr = array(
			"SIZE"			=> $showsize,
			"EDITIONSIZE"		=> $es,
			"k"			=> $i+1,
			"AMOUNT"		=> $amount[$i],
			"UNITS"			=> "{fp:prints}"
		);
		$rows .= Substitutions ($row, $arr);
		$total += $amount[$i];
		$i++;
	}

	$total += $outsidesales;
	
	if ($total < $record['TotalEditionSize']) {
		$nextPrintNum = $total + 1;
	} else {
		$nextPrintNum = "{fp:editionclosed}";
	}
	
	$f = array(
		"ROWS"			=> $rows,
		"EDITIONHEADER"		=> $edHdr,
		"NEXTPRINTNUMBER"	=> $nextPrintNum,
		"OUTSIDESALES"		=> $outsidesales
		);
	$text = Substitutions (FetchSnippet ("priceSet_sales_figures_form"), $f);
	$text = Substitutions ($text, $record);

	return $text;
}


// Clean up an X,Y,Z entry
// return XxYxZ ('x' between values)
function CleanXYZ ($t) {
	$s = "/(\d{1-3}).*?(\d{1-3}).*?(\d{1-3})/";
	$r = "1x\2x\3";
	$t = preg_replace($s, $r, $t);
	return $t;
}


?>