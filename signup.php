<?php
// Sign up for a free account

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

$f = $_POST;

// Get form input
$vars = GetFormInput();
$actions = $vars['actions'];
$record = $vars['vars'];

// Start Session
session_name("fp_admin");
session_start();

// Must be set right away so page will draw properly (getting snippets from admin)
$_SESSION['theme'] = ADMIN_THEME;
$_SESSION['GroupID'] = FP_ADMINISTRATOR_GROUP;

$userID = null;
$user = new FPUser();
if (!FP_SIGNUP_OPEN) {
	$result = "closed";
} else {
	switch ($actions['action']) {
		case "activate" :
			$result = "activation_failed";	// default
			// ACTIVATE
			if (!isset ($record["hash"])) {
				$error .= "Activation attempted, but no activate code was sent!<BR>";
			} else {
				//This is the actual activation. User got the email and clicked on the special link we gave him/her
				$hash = $user->escape($_GET['hash']);
				$activation = $user->activateByHash ($hash);
				if (!$user->error) {
					//$msg .= "You have successfully activated your account.<br>";
					$result = "activation_OK";
				} else {
					$error .= "Error: {$user->error}<br>";
				}
			}
			break;
		case "enter" :
			if (trim($_POST['code']) == FP_SIGNUP_CODE) {
				$result = "apply";
				break;
			} else {
				$error .= FetchSnippet ("error_signup_code_wrong") . " ({$_POST['code']})<br>";
				$result = "ask";
				break;
			}
			// fall through if code matches
		case "apply" :
			// Use application form
			// We need these entries: Firstname, Lastname, Email, Referral
			$result = "signup_failed";	// default
			$allFilledOut = ($record['Firstname'] and $record['Lastname'] and $record['Email'] and $record['Referral'] and $record['user_terms_of_service']);
			$someFilled = $record['Firstname'] . $record['Lastname'] . $record['Email'] . $record['Referral'];
			
			// Check Captcha
			
						
			// If data entered, try to make a new entry with it.
			if ($allFilledOut) {
				$record['Username'] = $record['Email'];
				if ($user->userExists($record['Username'])) {
					$error .= "Sorry, we could not sign you up: We use your email (" . $record['Email'] . ") as your username, but it's already in the system. Either you've signed up before, or someone else is using your email!";
				} else {
					$userID = $user->createUserWithActivationEmail ($record);
					if ($userID) {
						$user->loadUser ($userID);
						$username = $user->Email;
						$password = $user->Password;
						//$msg .= "Success...new user created.<BR>";
						$result = "signup_OK";
					} else {
						$error .= "Sorry, we couldn't sign you up with that information. Some mysterious part of the system rejected it. Please, try again.<BR>";
						$result = "apply";
					}
				}
			} else if ($someFilled) {
				$error .= "Sorry, you didn't complete the whole form. Please try again.<BR>";
				$result = "apply";
			}
			$email = "There was an attempt to sign up for an account on ".FP_SYSTEM_DISPLAY_NAME." at " . date ("Y-m-d h:m:s") . ".\nusername : {$user->Email} \npassword : {$user->Password} \nSuccess : $error --- $result\n Referral : {$record['Referral']}\n ";
			$headers = 'From: ' .FP_INFO_EMAIL. "\r\n" . 'Reply-To: '.FP_INFO_EMAIL. "\r\n".'X-Mailer: PHP/' . phpversion();
			$res = mail( FP_SYSTEM_EMAIL , 'Application for Account on '.FP_SYSTEM_DISPLAY_NAME, $email, $headers);
			//print FP_SYSTEM_EMAIL . " - " . $email . "<BR>".$headers;
			break;
		default :
			if (trim(FP_SIGNUP_CODE))
				$result = "ask";
			else
				$result = "apply";
	}
}
switch ($result) {
	case "ask" :
		$form = FetchSnippet ("signup_code_entry");
		break;
	case "signup_OK" :
		$form = FetchSnippet ("signup_OK");
		break;
	case "activation_OK" :
		$form = FetchSnippet ("signup_activation_OK");
		break;
	case "signup_failed" :
		$form = FetchSnippet ("signup_info_form");
		break;
	case "activation_failed" :
		$form = FetchSnippet ("signup_activation_failed");
		break;
	case "closed" :
		$form = FetchSnippet ("signup_closed");
		break;
	case "apply" :
		// drop through
	default :
		$form = FetchSnippet ("signup_info_form");
		break;
}

$output = FetchSnippet ("main_signup");

$record['formaction'] = "http://".$_SERVER['HTTP_HOST'].$_SERVER['REQUEST_URI'];
$record['pixwaiting'] = "";
$record['userinfo'] = "";
$record['addnewrecord'] = "";
$record['javascript'] = FP_THEMES_DIR."/".ADMIN_THEME."/".FP_JSDIR."/signup.js";
$record['form'] = $form;
$record['title'] = "Signup";

$error = FormatErrorForOutput ($error);
$msg = FormatMessageForOutput ($msg);
$record['error'] = $error;
$record['msg'] = $msg;


$f['META_INDEX'] = FetchSnippet ('meta_robots_noindex_nofollow');

$f['form'] = $form;
$f['PAGEID'] = "signup";

$output = Substitutions ($output, $f);
$output = Substitutions ($output, $record);
$userID && $output = Substitutions ($output, $user->userData);
$output = ReplaceAllSnippets ($output);
$output = ReplaceSysVars ($output);
//$output = insertBGANCost ($output);
$output = DeleteUnusedSnippets ($output);

print $output;


// close MySQL connection
mysql_close($LINK);
$FP_MYSQL_LINK->close();

?>
