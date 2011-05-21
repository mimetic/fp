<?php

/*
Configuration file for the storefront for selling subscriptions and such
using PayPal.

This site sells the services, and updates other sites' databases to reflect 
that a sale was made. There is a "standardized" database table that it writes
to in each site, and the site then does with that info whatever it wants.

SETUP: 

IMPORTANT: Remember to set your PayPal settings in your sellers account:
	- My Account : Profile : Instant Payment Notification Preferences
	- Set the IPN URL to http://www.mysellersite.com/pp/process.php (assuming pp is in a pp directory)

*/

// ----- SERVICES -----------------------------------

// Empty entry : choose a service
$chooseaservice		= array (
						"ID"			=>	"0",
						"desc"		=>	"Choose a Service"
					);
					
$DEFAULT_SERVICE = "0";

// PPF Service
$PPF_service = array (
	"ID"				=>	1,
	"name"			=>	$SYSTEMNAME,
	"accountrequired" => true,
	"business"		=>	"dgross@mimetic.com",
	"sandbox"		=>	"bear@mimetic.com",
	"desc"			=>	"A file transfer and photo filing system for photographers.",
	"users"			=>	"Users",
	"orders" 		=>	"Paypal",
	"mysqldb"		=>	MYSQLDB,
	"mysqlhost"		=>	MYSQLHOST,
	"mysqluser"		=>	MYSQLUSER,
	"mysqlpasswd"	=>	MYSQLPASSWORD,
	"usernameField"	=>	"Username",
	"passField"		=>	"Password",
	"orderIDField"	=>	"id",
	"usersIDField"	=>	"ID",
	"URL"			=>	"http://{$SYSTEMURL}",
	"loginURL"		=>	'http://{$SYSTEMURL}admin.php?login={$Username}&passwd={$Password}&action=login'

	);


// FP Gallery
$FP_service = array (	
	"ID"				=>	2,
	"name"			=>	$SYSTEMNAME,
	"accountrequired" => true,
	"business"		=>	"dgross@mimetic.com",
	"sandbox"		=>	"bear@mimetic.com",
	"desc"			=>	"A gallery system for photographers and artists for showing your work and selling limited edition fine art prints.",
	"users"			=>	"Artists",
	"orders" 		=>	"Paypal",
	"mysqldb"		=>	MYSQLDB,
	"mysqlhost"		=>	MYSQLHOST,
	"mysqluser"		=>	MYSQLUSER,
	"mysqlpasswd"	=>	MYSQLPASSWORD,
	"usernameField"	=>	"Username",
	"passField"		=>	"Password",
	"orderIDField"	=>	"id",
	"usersIDField"	=>	"ID",
	"URL"			=>	"http://{$SYSTEMURL}",
	"loginURL"		=>	'http://{$SYSTEMURL}admin.php?login={$Username}&passwd={$Password}&action=login'

	);

// FP website Hosting and Gallery
$FP_hosting_service = array (	
	"ID"				=>	3,
	"name"			=>	$SYSTEMNAME,
	"accountrequired" => true,
	"business"		=>	"dgross@mimetic.com",
	"sandbox"		=>	"bear@mimetic.com",
	"desc"			=>	"A gallery system for photographers and artists for showing your work and selling limited edition fine art prints.",
	"users"			=>	"Artists",
	"orders" 		=>	"Paypal",
	"mysqldb"		=>	MYSQLDB,
	"mysqlhost"		=>	MYSQLHOST,
	"mysqluser"		=>	MYSQLUSER,
	"mysqlpasswd"	=>	MYSQLPASSWORD,
	"usernameField"	=>	"Username",
	"passField"		=>	"Password",
	"orderIDField"	=>	"id",
	"usersIDField"	=>	"ID",
	"URL"			=>	"http://{$SYSTEMURL}",
	"loginURL"		=>	'http://{$SYSTEMURL}admin.php?login={$Username}&passwd={$Password}&action=login'

	);

$PP_SERVICES	=	array (	
	"0"	=>	$chooseaservice,
	"1"	=>	$PPF_service,
	"2"	=>	$FP_service,
	"3"	=>	$FP_hosting_service
	);
						


// Here are the services available for subscription
$SERVICES_NAMES = array (
	"0" => array ("name" => $PP_SERVICES[0]['name']),
	"1" => array ("name" => $PP_SERVICES[1]['name']),
	"2" => array ("name" => $PP_SERVICES[2]['name']),
	"3" => array ("name" => $PP_SERVICES[3]['name'])
	);


// ----- END SERVICES -----------------------------------

// ----- COUPONS -----------------------------------

/*
These next values are associated with any coupons or discounts you may offer ... 
They do NOT appear in the dropdown selections and remain invisible to users.
If a user enters one of the codes on the left, the values to the right of it replace 
any other Subscription variables of the same name being sent to PayPal. 
MUST BE LOWERCASE!!!

NOTE: coupondiscount is a percent discount, e.g. 25%, stored as an integer. So, 25% is stored as 25, and
means a reduction in the price (a3 variable) of 25% or $a3 = (1-.25) * $a3.
*/
$coupon_codes =  
	array (
//			"Tester" 	=> array ("service"=>"1", "a3"=>"15", "p3"=>"1", "t3"=>"M", "coupondesc"=>'Beta Tester Discount', "whm_name"=>'tester', )
//			"ep" 	=> array ("service"=>"1", "coupondiscount"=>"25", "p3"=>"1", "t3"=>"M", "coupondesc"=>'EP Member 75% Discount', "whm_name"=>'ep', )
//			"free" 	=> array ("service"=>"2", "a3"=>"0", "p3"=>"1", "t3"=>"M", "coupondesc"=>'Free Account', "whm_name"=>'free', ),
			);

// This text is added to a username to create the coupon code using MD5. By adding this text, which is arbitrary, no one can generate coupons just by using MD5 themselves.
$coupon_code_gen_mod = "15450";

// ----- END COUPONS -----------------------------------


// ----- PACKAGES -----------------------------------

// Here are the Packages for each service
// Note the "package" entry: it is an array of field/value to be updated into the service's user database
// Note that "0" is the trial account setting, like an unsubscribe.


// *** setup does NOT work with paypal,because if they start with a cheaper 
// service, then upgrade, Paypal will NOT charge the "trial period", which is 
// were I'm charging the set up fee. So, just adjust prices to include setup fee
// in the year's charge.


/*

THIS RECORD HAS BEEN MOVED TO THE FP CONFIG. FP IS NOW RESPONSIBLE FOR THE DATA, NOT PP.
FP Gallery:
User levels:
	FP_SINGLE_GALLERY_SINGLE_USER = 1
	FP_SINGLE_GALLERY_MULTI_USER = 2
Access levels:
	FP_ADMINISTRATOR_LEVEL = 1
	FP_NORMAL_LEVEL = 2
	FP_PERSONAL_LEVEL = 3
	FP_SUPPLIER_LEVEL = 4

*/

// p = number of periods, e.g. 1 means 1 x month
// t = billing cycle period, e.g. M for month
// *** setup does NOT work with paypal,because if they start with a cheaper 
// service, then upgrade, Paypal will NOT charge the "trial period", which is 
// were I'm charging the set up fee. So, just adjust prices to include setup fee
// in the year's charge.

// whm_name is a code for WHM to create or modify the server account

// rate = 0 means unsubscribe!!!

$PPF_packages = array (
	"0"	=>	array (
		"rate"		=> 	"0",
		"setup"		=>	"0",
		"p"			=>	"0",
		"t"			=>	"M",
		"accountrequired" => true,
		"name"		=>	'Trial Account',
		"whm_name"	=>	'3',
		"desc"		=>	'Free trial account, 5MB storage',
		"data"		=>	array ( array ( 
								"UserLevel"		=>	2,
								"SubscriptionID"	=>	0,
								"SubscriptionDesc"	=>	"Free trial account, 5MB storage",
								"Storage"		=>	10) )
		),
	"1"		=>	array (	
		"rate"		=> 	"10",
		"setup"		=>	"0",
		"p"			=>	"1",
		"t"			=>	"M",
		"accountrequired" => true,
		"name"		=>	'Professional Account',
		"whm_name"	=>	'3',
		"desc"		=>	'$10 per month, 200MB storage',
		"data"		=>	array ( array (
								"UserLevel"		=>	2,
								"SubscriptionID"	=>	1,
								"SubscriptionDesc"	=>	"$10 per month, 100MB storage",
								"Storage"		=>	200) )
		),
	"2"		=>	array (	
		"disabled"	=>	false,
		"rate"		=> 	"20",
		"setup"		=>	"0",
		"p"			=>	"1",
		"t"			=>	"M",
		"accountrequired" => true,
		"name"		=>	'Big Professional Account',
		"whm_name"	=>	'3',
		"desc"		=>	'$20 per month, 400MB storage',
		"data"		=>	array ( array (
								"UserLevel"		=>	2,
								"SubscriptionID"	=>	2,
								"SubscriptionDesc"	=>	"$20 per month, 300MB storage",
								"Storage"		=>	400) )
		)
);
		

/*
USER TYPE (USER LEVEL)
1=FP_SINGLE_GALLERY_SINGLE_USER cannot add galleries or users
2=FP_SINGLE_GALLERY_MULTI_USER cannot add galleries
3=FP_MULTI_GALLERY_SINGLE_USER can add new galleries, but not users
4=FP_MULTI_GALLERY_MULTI_USER can add new galleries, and new users
*/

$FP_packages =	array (
	"0"		=>	array (	
		"rate"		=> 	"0",
		"setup"		=>	"0",
		"p"			=>	"0",
		"t"			=>	"M",
		"accountrequired" => true,
		"name"		=>	'Trial Account',
		"whm_name"	=>	'3',
		"desc"		=>	'Free trial account, 5MB storage, multiple exhibitions',
		"data"		=>	array ( array ( 
								"UserLevel"		=>	2,
								"AccessLevel"		=>	3,
								"SubscriptionID"	=>	0,
								"SubscriptionDesc"	=>	"Free trial account, 5MB storage, multiple exhibitions",
								"Storage"			=>	50) )
		),
	"1"		=>	array (	
		"rate"		=> 	"35",
		"setup"		=>	"0",
		"p"			=>	"1",
		"t"			=>	"M",
		"accountrequired" => true,
		"name"		=>	'Single User, Single Gallery',
		"whm_name"	=>	'3',
		"desc"		=>	'$35 per month, 300MB storage, multiple exhibitions, personal gallery',
		"data"		=>	array ( array (
								"UserLevel"		=>	2,
								"AccessLevel"		=>	3,
								"SubscriptionID"	=>	1,
								"SubscriptionDesc"	=>	"$35 per month, 300MB storage, multiple exhibitions, personal gallery",
								"Storage"			=>	300) )
		),
	"2"		=>	array (	
		"disabled"	=>	true,
		"rate"		=> 	"45",
		"setup"		=>	"0",
		"p"			=>	"1",
		"t"			=>	"M",
		"accountrequired" => true,
		"name"		=>	'Single User, Multiple Galleries',
		"whm_name"	=>	'3',
		"desc"		=>	'$45 per month, 300MB storage, unlimited exhibitions, multiple galleries',
		"data"		=>	array ( array (
								"UserLevel"		=>	2,
								"AccessLevel"		=>	1,
								"SubscriptionID"	=>	2,
								"SubscriptionDesc"	=>	"$45 per month, 300MB storage, unlimited exhibitions, multiple galleries",
								"Storage"		=>	500) )
		)
);


/*
USER TYPE (USER LEVEL)
1=FP_SINGLE_GALLERY_SINGLE_USER cannot add galleries or users
2=FP_SINGLE_GALLERY_MULTI_USER cannot add galleries
3=FP_MULTI_GALLERY_SINGLE_USER can add new galleries, but not users
4=FP_MULTI_GALLERY_MULTI_USER can add new galleries, and new users

$AccessLevel controls administrative access level.
*/

$FP_hosting_packages =	array (
	"0"		=>	array (	
		"rate"		=> 	"0",
		"setup"		=>	"0",
		"p"			=>	"0",
		"t"			=>	"M",
		"accountrequired" => true,
		"name"		=>	'Setup/Disabled Account',
		"whm_name"	=>	'3',
		"desc"		=>	'Trial account',
		"data"		=>	array ( array (
								"UserLevel"			=>	1,
								"AccessLevel"		=>	3,
								"SubscriptionID"	=>	0,
								"SubscriptionDesc"	=>	"Trial account",
								"Storage"			=>	50) )
		),

	"1"		=>	array (	
		"rate"			=> 	"50",
		"setup"			=>	"0",
		"p"				=>	"1",
		"t"				=>	"M",
		"accountrequired"	=> true,
		"name"			=>	'Single User, Single Gallery',
		"whm_name"			=>	'2',
		"desc"			=>	'$50 per month, webserver account, 500MB storage, single user, single gallery',
		"data"		=>	array ( array (
								"UserLevel"			=>	1,
								"AccessLevel"		=>	1,
								"SubscriptionID"	=>	1,
								"SubscriptionDesc"	=>	"$50 per month, webserver account, 500MB storage, single user, single gallery",
								"Storage"			=>	500) )
		),
	"2"		=>	array (	
		"rate"		=> 	"65",
		"setup"		=>	"0",
		"p"			=>	"1",
		"t"			=>	"M",
		"accountrequired" => true,
		"name"		=>	'Many Users, Single Gallery',
		"whm_name"		=>	'2',
		"desc"		=>	'$65 per month, webserver account, 500MB storage, many users, single gallery',
		"data"		=>	array ( array (
								"UserLevel"			=>	2,
								"AccessLevel"		=>	1,
								"SubscriptionID"	=>	2,
								"SubscriptionDesc"	=>	"$65 per month, webserver account, 500MB storage, many users, single gallery",
								"Storage"			=>	500) )
		),
	"3"		=>	array (	
		"rate"		=> 	"75",
		"setup"		=>	"0",
		"p"			=>	"1",
		"t"			=>	"M",
		"accountrequired" => true,
		"name"		=>	'Single User, Many Galleries',
		"whm_name"		=>	'2',
		"desc"		=>	'$75 per month, webserver account, 500MB storage, single user, multiple galleries',
		"data"		=>	array ( array (
								"UserLevel"			=>	3,
								"AccessLevel"		=>	1,
								"SubscriptionID"	=>	3,
								"SubscriptionDesc"	=>	"$75 per month, webserver account, 500MB storage, single user, multiple galleries",
								"Storage"			=>	500) )
		),
	"4"		=>	array (	
		"rate"		=> 	"150",
		"setup"		=>	"0",
		"p"			=>	"1",
		"t"			=>	"M",
		"accountrequired" => true,
		"name"		=>	'Many Users, Many Galleries',
		"whm_name"		=>	'2',
		"desc"		=>	'$150 per month, webserver account, 1500MB storage, 500MB storage, many users, multiple galleries',
		"data"		=>	array ( array (
								"UserLevel"			=>	4,
								"AccessLevel"		=>	1,
								"SubscriptionID"	=>	4,
								"SubscriptionDesc"	=>	"$150 per month, webserver account, 1500MB storage, 500MB storage, many users, multiple galleries",
								"Storage"			=>	1500) )
		)
);


$PP_PACKAGES =  array (	
	"1" =>	$PPF_packages,
	"2" =>	$FP_packages,
	"3" =>	$FP_hosting_packages
	);
						
// ----- END PACKAGES -----------------------------------


?>