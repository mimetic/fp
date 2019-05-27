<?php

class FPImage
{
// ---------------------
// Variables
// ---------------------
public $ID, $title;

// ---------------------	
// Functions
// ---------------------
function FPImage ($ID = null) {
	global $msg, $error;
	
	$DEBUG = 0;
	// Constructor class;
	if ($ID) {
		$this->image = FetchImage($ID);
	} else {
		$this->image = array();
	}
}

function FetchImage ($ID = null) {

	isset($ID) || $ID = $this->ID;
	if (!$ID)
			return false;

	$this->image = FetchImage($ID);
	return $this->image;
}



/*
 * SetAmountSoldOutside
 * Set the amount of prints already sold in a price set. This is useful for allocating
 * some of the prints for sale outside the system, or recording sold prints.
 * However, we don't know the sizes sold, so we can't record that.
 * The problem is, what if we alread sold 5 small and 5 large prints.
 * Now, the user wants to allocate more sold outside, e.g. 5 prints of unknown size.
 * What should he enter? 5 or 15? And where do we record this info???
 */
function SetOutsideSalesAmount ($amount = 0) {
	global $msg, $error;
	
	$this->SetParam (FP_PARAM_AMOUNT_SOLD_OUTSIDE, $amount );
}

function SetParam ($key, $value) {
	$table = DB_IMAGES;

	if (!$this->ID || !$key)
			return false;
	$ID = $this->ID;

	$params = FetchParams ($table, $ID);
	$params = SetParam ($params, $key, $value);
	SaveParams ($table, $ID, $params);
}

function SetParams ($pairs = array() ) {
	$table = DB_IMAGES;

	if (!$this->ID || $pairs)
			return false;
	$ID = $this->ID;
	$params = FetchParams ($table, $ID);
	foreach ($pairs as $key => $value) {
		$params = SetParam ($params, $key, $value);
	}
	SaveParams ($table, $ID, $params);
}



// =====================================================================================
} // class

?>