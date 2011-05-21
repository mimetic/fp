<?php
/**
 * PHP Class to user access (login, register, logout, etc)
 * 
 * <code><?php
 * include('access.class.php');
 * $user = new PPUser();
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

class PPUser{
	/*Settings*/
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
	var $remCookieName = 'ckSavePass';
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
	var $displayErrors = false;
	/*Do not edit after this line*/
	var $userID;
	var $dbConn;
	var $userData=array();
	// List of fields in the database table
	var $fieldlist;
	
	/*
	Read these, but don't try to write. We won't be storing their values.
	*/
	public $ID, $Lastname, $Firstname, $Fullname;

	/**
	 * Class Constructure
	 * 
	 * @param string $dbConn
	 * @param array $settings
	 * @return void
	 */
	function PPUser($dbConn = '', $settings = '')
	{
		$this->fieldlist = GetFieldList ($this->dbTable);
		if ( is_array($settings) ){
			foreach ( $settings as $k => $v ){
					if ( !isset( $this->{$k} ) ) die('Property '.$k.' does not exists. Check your settings.');
					$this->{$k} = $v;
			}
		}
		$this->remCookieDomain = $this->remCookieDomain == '' ? $_SERVER['HTTP_HOST'] : $this->remCookieDomain;
		$this->dbConn = ($dbConn=='')? mysql_connect($this->dbHost.':'.$this->dbPort, $this->dbUser, $this->dbPass):$dbConn;
		if ( !$this->dbConn )
			die(mysql_error($this->dbConn));
		mysql_select_db($this->dbName, $this->dbConn) or die(mysql_error($this->dbConn));
		if( !isset( $_SESSION ) )
			session_start();
		if ( !empty($_SESSION[$this->sessionVariable]) ) {
			$this->loadUser( $_SESSION[$this->sessionVariable] );
		}
		//Maybe there is a cookie?
		if ( isset($_COOKIE[$this->remCookieName]) && !$this->is_loaded()){
			//echo 'I know you<br />';
			$u = unserialize(base64_decode($_COOKIE[$this->remCookieName]));
			$this->login($u['username'], $u['password']);
		}
	}
	
	/**
		* Login function
		* @param string $username
		* @param string $password
		* @param bool $loadUser
		* @return bool
	*/
	function login($username, $password, $remember = false, $loadUser = true) {
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
		if ( @mysql_num_rows($res) == 0)
			return false;
		if ( $loadUser ) {
			$this->userData = mysql_fetch_array($res);
			$this->userID = $this->userData[$this->tbFields['userID']];
			$_SESSION[$this->sessionVariable] = $this->userID;
			
			if ( $remember ){
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
		error_reporting(E_ALL ^ E_NOTICE);
	

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
		return $this->get_property($prop)==1?true:false;
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
	// Save User data to the database
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
			$this->userData['Params'] = MergeParams ($this->Params, $vars);
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
	function loadUser($userID = null) {
		global $error, $msg;
		
		$userID || $userID = $this->userID;
		$fullname = "CONCAT_WS(' ', Firstname, Lastname) AS Fullname";
		$creationdate = "UNIX_TIMESTAMP(Timestamp) AS CreationDate";
		$res = $this->query("SELECT *, $fullname, $creationdate FROM `{$this->dbTable}` WHERE `{$this->tbFields['userID']}` = '".$this->escape($userID)."' LIMIT 1");
		
		if ( mysql_num_rows($res) == 0 )
				return false;
		
		$this->userData = mysql_fetch_array($res);
		
		// Add object variables for each field in the table
		// This is for read-only! We won't store values set this way!
		foreach ($this->fieldlist as $f) {
			$this->$f = $this->userData[$f];
		}
		
		$this->Fullname = $this->userData['Fullname'];
		$this->CreationDate = $this->userData['CreationDate'];

		// Set AccessLevel
		$this->AccessLevel || $this->AccessLevel = FP_NORMAL_LEVEL;

		// Convert Params to an array for easy access
		$this->Params = DecodeArrayFromDB($this->Params);

		// Add params variables to the object
		while (list ($k,$v) = each ($this->Params)) {
			$this->$k = $v;
		}
		$this->userID = $userID;
		$_SESSION[$this->sessionVariable] = $this->userID;
		
		
		return true;
	}
	
	
	
	// Check if a user already exists
	function userExists ($username) {
		$q = "SELECT * FROM `{$this->dbTable}` WHERE `{$this->tbFields['login']}` = '$username' LIMIT 1";
		$exists = mysql_num_rows($this->query($q));
		return $exists;
	}

	// Check if a user already exists
	function userIDExists ($userID) {
		$q = "SELECT * FROM `{$this->dbTable}` WHERE `{$this->tbFields['userID']}` = '$userID' LIMIT 1";
		$exists = mysql_num_rows($this->query($q));
		return $exists;
	}

	function escape($str) {
		$str = get_magic_quotes_gpc()?stripslashes($str):$str;
		$str = mysql_real_escape_string($str, $this->dbConn);
		return $str;
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
	private function query($sql, $line = 'Uknown')
	{
		//if (defined('DEVELOPMENT_MODE') ) echo '<b>Query to execute: </b>'.$sql.'<br /><b>Line: </b>'.$line.'<br />';
		$res = mysql_db_query($this->dbName, $sql, $this->dbConn);
		if ( !res )
			$this->error(mysql_error($this->dbConn), $line);
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

	private function get_property($property)
	{
		if (empty($this->userID)) $this->error('No user is loaded', __LINE__);
		if (!isset($this->userData[$property])) $this->error('Unknown property <b>'.$property.'</b>', __LINE__);
		return $this->userData[$property];
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
}
?>