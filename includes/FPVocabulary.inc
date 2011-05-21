<?php
/**
 * PHP Class to read, write, edit, and translate vocabularies in FP Galleries
 * 
 * <code><?php
 * include('access.class.php');
 * $vocab = new FPVocabulary();
 * ? ></code>
 * 
 * ==============================================================================
 * 
 * The system has a list of keywords which are replaced when displayed. For example
 * "project" can become "Exhibition", "group" can become "Gallery" and so on.
 * Each user has a list which can override the main list for anything owned
 * by that user.
 *
 * Note, the FP_LANGUAGE in the fp_config file
 * determines whic vocab we're using.
 *
 * For the following, if a $userID is provided, we load the list from that user.
 * Otherwise, use the loaded list, $this->vocabulary.
 * If no list is loaded we load the system list.
 *
 * $this->LoadList ($userID)
 * $this->SaveList ($userID)
 * $this->EditPage ($userID)
 * $this->UpdateList ($arr, $userID)
 *
 * A list is an array of arrays:
 * $list = array ( array ( "key", "value" ) )
 * 
 *
 * ==============================================================================
 */

/**
 * Flexible Access - The main class
 * 
 * @param string $userID
 */

class FPVocabulary{

	public $list, $userID;

	/**
	 * Class Constructure
	 * 
	 * @param string $userID
	 *	ID of the user whose vocabulary list we are editing/using/fetching. Default is Admin user.
	 * @return void
	 */
	function FPVocabulary($userID = null)
	{
		global $msg, $error;
		$DEBUG = 0;

		$userID || $userID = FP_ADMINISTRATOR;
		$this->userID = $userID;
		$this->LoadList ($userID);

	}

	/*
	 * Load a vocabulary list.
	 * If no userID is specified, we get the system vocabular list.
	 * Return an empty array if no list is found.
	 */
	function LoadList ($userID = null) {
		$vocabfile = $this->ListPath($userID);
		$q = 1;
		if ($vocabfile) {
			$this->list = $this->VocabFileToArray($vocabfile);
		} else{
			$this->list = array();
		}
//		print __FUNCTION__.": $vocabfile";
		return $this->list;
	}

	/*
	 * Save a vocabulary list.
	 * If no userID is specified, save to the system list.
	 * Make a backup!
	 */
	function SaveList ($userID = null) {
		$vocabfile = $this->ListPath($userID);

		if ($this->list) {
			file_put_contents ($vocabfile, $this->list);
		}
	}

	function EditPage ($list=null) {
		$list || $list = $this->list;
		
		$t = "";
		$rowf = FetchSnippet ("vocab_edit_row");
		$rowc = FetchSnippet ("vocab_edit_row_comment");
		$keys = array ('/{KEY}/i', '/{VALUE}/i');
		foreach ($list as $def) {
			if (is_array($def)) {
				$row = preg_replace ($keys, $def, $rowf);
			} else {
				$def = trim ($def, "//#");
				$row = str_replace ('{COMMENT}', $def, $rowc);
			}
			$t .= $row;
		}
		$fields['list'] = $t;
		$fields['language'] = FP_LANGUAGE;
		$artist = FetchArtist ($this->userID);
		$fields = array_merge($fields, $artist);
		$vlist = Substitutions (FetchSnippet("vocab_list"), $fields);
		return $vlist;
	}

	function UpdateList ($arr = null, $userID = null) {
		$arr || $arr = $this->list;
	}





	/*
	 * ======= Private Functions =======
	 */

	private function ListPath ($userID) {
		$userID || $userID = $this->userID;
		$userID || $userID = FP_ADMINISTRATOR;

		(FP_LANGUAGE == "en" or FP_LANGUAGE == "") ? $lang = "" : $lang = "_" .FP_LANGUAGE ;

		if ($userID == FP_ADMINISTRATOR) {
			$vocabfile = FP_THEMES_DIR . "/" . DEFAULT_THEME . "/" . FP_SNIPPETSDIR ."/{$lang}vocabulary.txt";
		} else {
			$vocabfile = FP_DIR_USER."/user_{$this->userID}/{$lang}vocabulary.txt";
		}

		if (!file_exists($vocabfile))
			return false;

		return $vocabfile;


	}
	
	private function VocabFileToArray ($fn) {
		$farr = file($fn);
		$arr = array ();
		foreach ($farr as $row) {
			$row = trim ($row);
			if ($row && substr($row,0,2) != "//" && substr($row,0,2) != "#" ) {
				preg_match ("/(.*?),(.*)/", $row, $matches);
				$k = trim($matches[1]);
				$v = trim($matches[2]);
				$v || $v = $k;
				$arr[$k] = array ($k,$v);
			} else {
				trim($row) && $arr[] = trim($row);
			}
		}
		return $arr;
	}


}
?>