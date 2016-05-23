<?php
/**
 * PHP Class to user access (login, register, logout, etc)
 * 
 * <code><?php
 * include('access.class.php');
 * $user = new FPUser();
 * ? ></code>
 * 
 * For support issues please refer to the webdigity forums :
 *				http://www.webdigity.com/index.php/board,91.0.html
 * or the official web site:
 *				http://phpUserClass.com/
 * ==============================================================================
 * 
 * @version $Id: access.class.php,v 0.93 2008/05/02 10:54:32 $
 * @copyright Copyright (c) 2007 Nick Papanotas (http://www.webdigity.com)
 * @author Nick Papanotas <nikolas@webdigity.com>
 * @license http://opensource.org/licenses/gpl-license.php GNU General Public License (GPL)
 * 
 * MODIFIED BY DAVID GROSS 2008
 *
 * ==============================================================================

 */

/**
 * Flexible Access - The main class
 * 
 * @param string $dbName
 * @param string $dbHost 
 * @param string $dbUser
 * @param string $dbPass
 * @param string $dbTable
 */

class FPUser{
	/*Settings*/

	/**
	 * The mysqli database connection
	 */
	var $mysqli_connection;

	/**
	 * The database that we will use
	 * var string
	 */
	var $dbName = MYSQLDB;
	/**
	 * The database host
	 * var string
	 */
	var $dbHost = MYSQLHOST;
	/**
	 * The database port
	 * var int
	 */
	var $dbPort = 3306;
	/**
	 * The database user
	 * var string
	 */
	var $dbUser = MYSQLUSER;
	/**
	 * The database password
	 * var string
	 */
	var $dbPass = MYSQLPASSWORD;
	/**
	 * The database table that holds all the information
	 * var string
	 */
	var $dbTable	= DB_ARTISTS;
	/**
	 * The session variable ($_SESSION[$sessionVariable]) which will hold the data while the user is logged on
	 * var string
	 */
	var $sessionVariable = 'userSessionValue';
	/**
	 * Those are the fields that our table uses in order to fetch the needed data. The structure is 'fieldType' => 'fieldName' and allows you to have differently named table fields
	 * var array
	 */
	var $tbFields = array(
		'userID'=> 'ID', 
		'login' => 'Username',
		'pass'	=> 'Password',
		'email' => 'Email',
		'active'=> 'Confirmed'
	);
	/**
	 * When user wants the system to remember him/her, how much time to keep the cookie? (seconds)
	 * var int
	 */
	var $remTime = FP_COOKIELIFE;
	/**
	 * The name of the cookie which we will use if user wants to be remembered by the system
	 * var string
	 */
	var $remCookieName = 'FP_User_SavePass';
	/**
	 * The cookie domain
	 * var string
	 */
	var $remCookieDomain = '';
	/**
	 * The method used to encrypt the password. It can be sha1, md5 or nothing (no encryption)
	 * var string
	 */
	//var $passMethod = 'sha1';
	var $passMethod = 'nothing';
	/**
	 * Display errors? Set this to true if you are going to seek for help, or have troubles with the script
	 * var bool
	 */
	var $displayErrors = true;
	/*Do not edit after this line*/
	var $userID;
	
	var $userData=array();
	// List of fields in the database table
	var $fieldlist;
	
	/*
	Read these, but don't try to write. We won't be storing their values.
	GODDAMN, STANDARDIZE...these are capitalized, others are not in other objects
	*/
	public $ID, $Lastname, $Firstname, $Email, $Fullname, $AccessLevel, $UserType, $params, $StorageAllocated, $StorageAllocatedText, $StorageUsed, $StorageFree, $StorageFreeText;

	/**
	 * Class Constructure
	 * 
	 * @param string $dbConn
	 * @param array $settings
	 * @return void
	 */
	function __construct ($mysqli_connection, $id = null, $keepInSession = false, $dbConn = '', $settings = '')
	{
		global $msg, $error;
		$DEBUG = 0;
		
		$DEBUG && $starttime = microtime(true);
		
		$this->mysqli_connection = $mysqli_connection;
		
		$this->fieldlist = MysqlListFields ($this->dbTable);

		// Check that requested setting actually exists
		if ($settings) {
			if ( is_array($settings) ){
				foreach ( $settings as $k => $v ){
						if ( !isset( $this->{$k} ) ) die('Property '.$k.' does not exists. Check your settings.');
						$this->{$k} = $v;
				}
			}
		}
		
		// DO NOT CONNECT TO A DB
		/*
		$this->remCookieDomain = $this->remCookieDomain == '' ? $_SERVER['HTTP_HOST'] : $this->remCookieDomain;
		$this->dbConn = ($dbConn=='')? mysqli_connect($this->dbHost.':'.$this->dbPort, $this->dbUser, $this->dbPass):$dbConn;
		
		if ( !$this->dbConn )
			die(mysqli_error($this->dbConn));
		
		mysqli_select_db($this->dbName, $this->dbConn) or die(mysqli_error($this->dbConn));
		*/

		// If $id is set then we load that user
		if ($id) {
			$this->loadUser ($id);
		} else {
			
			if( $keepInSession && isset( $_SESSION ) ) {
				if ( !empty($_SESSION[$this->sessionVariable]) )
					$this->loadUser( $_SESSION[$this->sessionVariable] );
			}
	
			//Maybe there is a cookie? (set if "remember" was on)
			if ( isset($_COOKIE[$this->remCookieName]) && !$this->is_loaded()){
				//$msg .= 'I remember you.<br>';
				$u = unserialize(base64_decode($_COOKIE[$this->remCookieName]));
				$this->login($u['username'], $u['password']);
			}
		}
		$DEBUG && $msg .= __FUNCTION__.__LINE__.": Time elapsed: ". round (microtime(true) - $starttime, 2) . " seconds<BR>";
	}
	
	/**
		* Login function
		* @param string $username
		* @param string $password
		* @param bool $loadUser
		* @return bool
	*/
	function login($username, $password, $remember = false, $loadUser = true) {
		global $error, $msg;
		
		$username = $this->escape($username);
		$password = $originalPassword = $this->escape($password);
		switch(strtolower($this->passMethod)){
			case 'sha1':
				$password = "SHA1('$password')"; break;
			case 'md5' :
				$password = "MD5('$password')";break;
			case 'nothing':
				$password = "'$password'";
		}
		$res = $this->query("SELECT * FROM `{$this->dbTable}` 
		WHERE `{$this->tbFields['login']}` = '$username' AND `{$this->tbFields['pass']}` = $password LIMIT 1",__LINE__);
		if ( @mysqli_num_rows($res) == 0)
			return false;
		if ( $loadUser ) {
			$this->userData = mysqli_fetch_array($res);
			$this->userID = $this->userData[$this->tbFields['userID']];
			$this->ID = $this->userID;
			$_SESSION[$this->sessionVariable] = $this->userID;
			
			if ( $remember ){
				//$error .= __FUNCTION__.__LINE__.": Cookie set<br>";
				$cookie = base64_encode(serialize(array('username'=>$username,'password'=>$originalPassword)));
				$a = setcookie($this->remCookieName, $cookie, time()+$this->remTime, '/', $this->remCookieDomain);
			}
		}
		return true;
	}
	
	/**
		* Logout function
		* param string $redirectTo
		* @return bool
	*/
	function logout ($redirectTo = '') {
		// remove cookie of username/password
		setcookie($this->remCookieName, '', time()-3600, '/', $this->remCookieDomain);
		$this->userData = '';

		// Unset all of the session variables.
		$_SESSION = array();

		// If it's desired to kill the session, also delete the session cookie.
		// Note: This will destroy the session, and not just the session data!
		if (isset($_COOKIE[session_name()])) {
			setcookie(session_name(), '', time()-3600, '/');
		}
		// Finally, destroy the session.
		session_destroy();
		//error_reporting(E_ALL ^ E_NOTICE);


		if ( $redirectTo != '' && !headers_sent()){
			header('Location: '.$redirectTo );
			exit;//To ensure security
		}
	}


	/**
		* Function to determine if a property is true or false
		* param string $prop
		* @return bool
	*/
	function is($prop){
		return $this->get_property($prop)==1 ? true : false;
	}
	
	// Get a property of a user (field from the database record)
	// If we request a param, it will also get that as though it
	// were a normal variable.
	function getval($property)
	{
		
		if (empty($this->userID))
			$this->error('No user is loaded', __LINE__);
		if (in_array($property, $this->fieldlist)) {
			return $this->userData[$property];
		} else if (isset($this->Params[$property])) {
			return $this->getParam ($property);
		} else {
			$this->error('Unknown property <b>'.$property.'</b>', __LINE__);
			return null;
		}
	}
	
	// Set a property (field) of the user (field in database)
	// If we give a parameter value (for $params) that will be
	// figured out, and set properly.
	// This DOES NOT write the user info to the database
	function setval($property, $value)
	{
		if (empty($this->userID))
			$this->error('No user is loaded', __LINE__);
		if (in_array($property, $this->fieldlist)) {
			$this->userData[$property] = $value;
			return true;
		} else if (isset($this->Params[$property])) {
			$this->setParam ($key, $value);
		} else {
			$this->error('Unknown property <b>'.$property.'</b>', __LINE__);
			return null;
		}
	}
	
	/**
		* Is the user an active user?
		* @return bool
	*/
	function is_active()
	{
		return $this->userData[$this->tbFields['active']];
	}
	
	/**
	 * Is the user loaded?
	 * @ return bool
	 */
	function is_loaded()
	{
		return empty($this->userData) ? false : true;
	}
	
	function unload()
	{
		unset($this->userData);
	}

	/**
		* Activates the user account
		* @return bool
	*/
	function activate()
	{
		if (empty($this->userID))
			$this->error('No user is loaded', __LINE__);
		if ( $this->is_active())
			return true;
			//$this->error('That account is active already.', __LINE__);
		$res = $this->query("UPDATE `{$this->dbTable}` SET {$this->tbFields['active']} = 1 
				WHERE `{$this->tbFields['userID']}` = '".$this->escape($this->userID)."' LIMIT 1");
		if (@mysqli_affected_rows() == 1)
			{
				$this->userData[$this->tbFields['active']] = true;
				return true;
			}
	return false;
	}

	function activateByHash ($hash) {
		global $msg, $error;
		
		if (!$hash) {
			$this->error = "Failed to activate account: No code provided.";
			return false;
		}
		
		$res = $this->query("SELECT `{$this->tbFields['userID']}` FROM `{$this->dbTable}` WHERE `activationHash` = '$hash' LIMIT 1",__LINE__);
		if ( $rec = mysqli_fetch_array($res) ){
			$this->userID = $rec[$this->tbFields['userID']];
			$this->activate ();
			$this->loadUser();
			return true;
		} else {
			$this->error = "Failed to activate account: We have no record of an account with that activation code.";
			return false;
		}
	}



	/*
	 * Creates a user account. The array should have the form 'database field' => 'value'
	 * @param array $data
	 * return int
	 */	 
	function newUser($data){
		if (!is_array($data)) $this->error('Data is not an array', __LINE__);
		switch(strtolower($this->passMethod)){
			case 'sha1':
				$password = "SHA1('".$data[$this->tbFields['pass']]."')"; break;
			case 'md5' :
				$password = "MD5('".$data[$this->tbFields['pass']]."')";break;
			case 'nothing':
				$password = $data[$this->tbFields['pass']];
		}
		$data[$this->tbFields['pass']] = $password;
		$newID = AddRecord( $this->dbTable, $data );
		
		// Create user's gallery (if appropriate)
		if ($newID && FP_USERS_GET_GALLERY && (FP_MULTI_GALLERY_SINGLE_USER or FP_MULTI_GALLERY_MULTI_USER)) {
			$myGroup = new FPGroup ($this->mysqli_connection);
			$myGroup->MakeGroupForUser ($newID);
		}
		
		// Create portfolio exhibition for the artist
		$projectID = MakeArtistProject($newID, $myGroup->ID);
		$this->params = array();
		$this->setParam ('ProjectID', $projectID);
		$this->setParam (FP_PARAM_ARTIST_GALLERY_LINK, 1);
		$this->updateUser ();
		return $newID;
	}


	/*
	 * Creates a random password. You can use it to create a password or a hash for user activation
	 * param int $length
	 * param string $chrs
	 * return string
	 */
	function randomPass($length = 10, $chrs = '1234567890qwertyuiopasdfghjklzxcvbnm'){
		for($i = 0; $i < $length; $i++) {
				$pwd .= $chrs{mt_rand(0, strlen($chrs)-1)};
		}
		return $pwd;
	}
	
	// --------------
	// Save User date to the database
	// If $vars are passed, those values override object values,
	// e.g. $this->myvar = $vars['myvar']
	// Then, merge in any params values from the $vars
	// Then, gather the object values
	function updateUser ($vars = null) {
		
		// Merge public object variables into the object main array
//		while (list ($k,$v) = each (get_object_vars($this)))
//			$this->userData[$k] = $this->$k;
		// Capture any values in the $vars and merge those values into the existing params array
		if (is_array ($vars)) {
			// merge $vars['params'] into the object params values
			$this->userData['params'] = MergeParams ($this->Params, $vars);
			// merge $vars into object values
//			while (list ($k,$v) = each ($vars))
//				isset($this->userData[$k]) && $this->userData[$k] = $v;
		}
		$ID = $this->userData[$this->tbFields['userID']];
		$success = UpdateRecord ($this->dbTable, $ID, $this->userData);					
		$this->loadUser();	//update user info
	}


	// --------------
	/**
		* A function that is used to load one user's data
		* @access private
		* @param string $userID
		* @return bool
	*/
	function loadUser($userID = null, $fieldname = "userID") {
		global $error, $msg;
		
		$DEBUG = 1;
		
		$userID || $userID = $this->userID;
		$fullname = "CONCAT_WS(' ', Firstname, Lastname) AS Fullname";
		$creationdate = "UNIX_TIMESTAMP(Timestamp) AS CreationDate";
		$res = $this->query("SELECT *, $fullname, $creationdate FROM `{$this->dbTable}` WHERE `{$this->tbFields[$fieldname]}` = '".$this->escape($userID)."' LIMIT 1");
		
		if ( mysqli_num_rows($res) == 0 )
				return false;

		// clear user variables in $this
		foreach ($this->fieldlist as $f) {
			$this->$f = null;
		}

		$this->userData = mysqli_fetch_assoc($res);

		// Add object variables for each field in the table
		// This is for read-only! We won't store values set this way!
		while (list($k,$v) = each ($this->userData)) {
			$this->$k = $v;
		}

		$this->Fullname = $this->userData['Fullname'];
		$this->CreationDate = $this->userData['CreationDate'];

		// Set AccessLevel
		$this->AccessLevel || $this->AccessLevel = FP_NORMAL_LEVEL;

		// Set User Type
		$this->UserType = $this->UserLevel;
		$this->UserType || $this->UserType = FP_SINGLE_GALLERY_SINGLE_USER;

		// Convert params to an array for easy access
		$this->Params = DecodeArrayFromDB($this->Params);

		// Add params variables to the object
		while (list ($k,$v) = each ($this->Params)) {
			$this->$k = $v;
		}
		$this->userID = $this->ID;
		$_SESSION[$this->sessionVariable] = $this->userID;
		
		return true;
	}
	


	//Calculate storage used and storage free for the current user ($this)
	function CheckStorageSpace () {
		
		isset($this->userData['Storage']) || $this->userData['Storage'] = 100;
		
		$this->StorageAllocated = $this->userData['Storage'];
		$this->StorageUsed = $this->CalculateStorageUsed ();

		if ($this->StorageAllocated == 0) {
			$this->StorageFree = 0;
		} else {
			$this->StorageFree = $this->StorageAllocated - $this->StorageUsed;
		}
		
		if (intval ($this->StorageAllocated) > 0) {
			$this->StorageFreeText = $this->StorageFree;
			$this->StorageAllocatedText = $this->StorageAllocated;
		} else {
			$this->StorageFreeText = "{fp:unlimited}";
			$this->StorageAllocatedText = "{fp:unlimited}";
		}
	}
	
	
	
	// Insert a new user with values from $data and from default settings
	// Then, email the user an activation message, with a hash for confirmation
	// Upon confirmation, the user will become "confirmed."
	// Send activation message to user by email
	// 
	// Depending on a config flag, each new user gets a personal gallery, just as he gets a portfolio.
	// This only applies in multi-gallery settings, of course.
	function createUserWithActivationEmail ($data) {	
		global $error, $msg;
		global $SYSTEMURL;
		
		$DEBUG = 0;
		
		//Get an activation hash and mail it to the user
		//We need a unique hash, length 50
		$hash = $this->randomPass(50);
		while( mysqli_num_rows($this->query("SELECT * FROM `{$this->dbTable}` WHERE `ActivationHash` = '$hash' LIMIT 1")) == 1)
			$hash = $this->randomPass(50);
		//Adding the user. The logic is simple. We need to provide an associative array, where keys are the field names and values are the values
		$data['ActivationHash'] = $hash;
		$data['CreationDate'] = date ("Y-m-d h:m:s");

		$data = $this->setUserDefaultValues($data);
		//The method returns the userID of the new user or 0 if the user is not added
		$userID = $this->newUser($data);
		if ($userID==0) {
			//user is already registered or something like that
			$msg .= 'User not registered';
		} else {			
			$data['LINK'] = $SYSTEMURL . FP_SYSTEM_SIGNUP_SCRIPT . "?passwd={$data[$this->tbFields['pass']]}&login={$data[$this->tbFields['login']]}&action=activate&hash=$hash";
			$email = ReplaceSysVars(Substitutions (FetchSnippet ("signup_activation_email"), $data));
			$DEBUG && $error .= __FUNCTION__.": DATA:".ArrayToTable ($data);
			isset($data['Email']) ? $whereto = $data['Email'] : $whereto = $_POST['Email'];
			$headers = 'From: ' .FP_INFO_EMAIL. "\r\n" . 'Reply-To: '.FP_INFO_EMAIL. "\r\n".'X-Mailer: PHP/' . phpversion();
			$res = mail( $whereto , 'Activate your account', $email, $headers);
			$res || $error .= FetchSnippet ('signup_failed_to_send_email');
			$res || fp_error_log ('Signup_failed_to_send_email to $whereto', 3, FP_ERROR_LOG);
			$DEBUG && $error .= "Result of mailing: $res<BR>";
			$DEBUG && $res = mail( "david.mimetic@gmail.com" , 'This is a test of the signup email!', __FUNCTION__.": This is a test of the signup email", $headers);
			$DEBUG && $error .= $email."<BR>";
			$DEBUG && $error .= "Result of mailing to david.mimetic@gmail.com: $res<BR>";
		}
		return $userID;
	}

	
	// Check if a user already exists
	function userExists ($username) {
		$q = "SELECT * FROM `{$this->dbTable}` WHERE `{$this->tbFields['login']}` = '$username' LIMIT 1";
		$exists = mysqli_num_rows($this->query($q));
		return $exists;
	}

	// Check if a user already exists
	function userIDExists ($userID) {
		$q = "SELECT * FROM `{$this->dbTable}` WHERE `{$this->tbFields['userID']}` = '$userID' LIMIT 1";
		$exists = mysqli_num_rows($this->query($q));
		return $exists;
	}

	/*
	 * FetchByEmail
	 * Given an email address, get the user associated with it.
	 */
	function fetchByEmail ($email) {
		$q = "SELECT * FROM `{$this->dbTable}` WHERE Email = '$email' LIMIT 1";
		$res = $this->query($q);
		if ( mysqli_num_rows($res) == 0 )
		    return false;

		$user = mysqli_fetch_assoc($res);
		$userID = $user['ID'];
		$this->loadUser ($userID);
		return true;
	}

	function escape($str) {
		$str = get_magic_quotes_gpc()?stripslashes($str):$str;
		$str = mysqli_real_escape_string($this->mysqli_connection, $str);
		return $str;
	}
	
	function get_property($property) {
		if (empty($this->userID) or empty($this->userData)) {
			$this->error("No user is loaded or no userdata loaded while trying to get property $property: ", __LINE__);
			return null;
		}
		if (isset($this->userData[$property])) {
			return $this->userData[$property];
		} else {
			$this->error('Unknown property <b>'.$property.'</b>', __LINE__);
			return null;
		}
	}
	
	
	// Storage space used by this user
	function CalculateStorageUsed ($ID = null) {
		global $msg, $error;
		
		$DEBUG = 0;
		
		$ID || $ID = $this->ID;
		
		$idlist = FetchArtistImageIDs ($ID);
		$totalstorage = 0;
		foreach ($idlist as $imageID) {
			$totalstorage += image_storage_size ($imageID);
		}
		$DEBUG && $msg .= __FUNCTION__.": storage used = $totalstorage<br>";
		$totalstorage = ceil ($totalstorage/100000) / 10;
		$this->StorageUsed = $totalstorage;
		return $totalstorage;
	}
	
	// Return true if user is over storage limit.
	// This uses the $this->StorageUsed. 
	// $refresh forces a recalculation of the space used.
	function UserOverStorageLimit ($ID = null, $refresh = false) {
		global $msg, $error;
		
		$DEBUG = 0;
		
		$ID || $ID = $this->ID;
		
		$refresh && $this->CalculateStorageUsed ($ID);
		$DEBUG && $msg .= __FUNCTION__.": storage free = ".($this->StorageAllocated - $this->StorageUsed)."<br>";
		if (intval($this->StorageAllocated ) > 0) {
			return ($this->StorageAllocated < $this->StorageUsed );
			$error .= __FUNCTION__.": User over limit: {$this->StorageAllocated} - {$this->StorageUsed}<br>";
		} else {
			return false; //unlimited storage
		}
	}
	
	////////////////////////////////////////////
	// PRIVATE FUNCTIONS
	////////////////////////////////////////////
	
	/**
		* SQL query function
		* @access private
		* @param string $sql
		* @return string
	*/
	private function query($sql, $line = 'Unknown')
	{
		//if (defined('DEVELOPMENT_MODE') ) echo '<b>Query to execute: </b>'.$sql.'<br /><b>Line: </b>'.$line.'<br />';
		$res = mysqli_query ($this->mysqli_connection, $sql);
		if ( !$res )
			$this->error(mysqli_error( $this->mysqli_connection ), $line);
		return $res;
	}
	
	/**
		* Produces the result of addslashes() with more safety
		* @access private
		* @param string $str
		* @return string
	*/	
	/**
		* Error holder for the class
		* @access private
		* @param string $error
		* @param int $line
		* @param bool $die
		* @return bool
	*/	
	function error($myerror, $line = '', $die = false) {
		global $error;
		if ( $this->displayErrors ) {
			echo '<b>Error: </b>'.$myerror.'<br /><b>Line: </b>'.($line==''?'Unknown':$line).'<br />';
		} else {
			$error .= $myerror."<BR>\n";
		}
		if ($die) exit;
		return false;
	}

	private function getParam ($key) {
		return $this->Params[$key];
	}
	
	private function setParam ($key, $value) {
		$this->Params = SetParam ($this->Params, $key, $value);
	}
	
	// Set default values for a user
	// Better to do this here than in mysql
	private function setUserDefaultValues ($data) {
		$data['Commission'] = FP_ARTIST_DEFAULT_COMMISSION;
		// start out at lowest level
		$data['UserLevel'] = FP_ARTIST_DEFAULT_USERLEVEL;
		$data['AccessLevel'] = FP_ARTIST_DEFAULT_ACCESSLEVEL;
		$data['Storage'] = FP_ARTIST_DEFAULT_STORAGE;
		return $data;
	}
	
	
	function sendPasswordToUser ($userID) {
		global $msg, $error;
		
		$DEBUG = false;
		
		if ($userID)
			$artist = FetchArtistByUsername ($userID);
		$this->loadUser($artist['ID']);
		$email = $this->Email;
		if ($email) {
			$res .= __FUNCTION__.": Password sent to {$email}.";
			$to = "{$this->Fullname} <{$email}>";						
			$subject = FP_SYSTEM_DISPLAY_NAME . ": Lost Password";
		
			$text = FetchSnippet ("SendPasswordByEmail");
			$text = Substitutions ($text, array (
				'Fullname'	=> $this->Fullname,
				'Username'	=> $this->Username,
				'Password'	=> $this->Password
				));
			$text = ReplaceSysVars ($text);
		
			$result = mail($to, $subject, $text, FP_EMAIL_HEADERS_SYSTEM);
			if ($result) {
				fp_error_log(__FUNCTION__.": Sent password to user #{$this->ID} ({$this->Fullname}) at $to.", 3, FP_MAINTENANCE_LOG);
				$res = "Sent password to user $userID";
				return $res;
			} else {
				fp_error_log(__FUNCTION__.": FAILED TO send password to user #{$this->ID} ({$this->Fullname}) at $to.", 3, FP_MAINTENANCE_LOG);							
				$DEBUG && $msg .= __FUNCTION__."::".__LINE__.":: FAILED $to, $toName, $subject<BR>";
				$res = "Could not send email! (I don't recognize &ldquo;$userID&rdquo;)<BR>";
				return $res;
			}
	
		} else {
			$res = "Could not send email! (I don't recognize &ldquo;$userID&rdquo;)<BR>";
		}
		return $res;
	}

// ========== end ==========
}
?>