<?php

class FPPriceSet
{
// ---------------------
// Variables
// ---------------------
public $ID, $title;

// ---------------------	
// Functions
// ---------------------
function FPPriceSet ($ID = null) {
	global $msg, $error;
	
	$DEBUG = 0;
	// Constructor class;

}


function FetchPriceSet ($ID  = null) {
	global $IMAGES, $PRICES, $ARTISTS, $TOPICS, $KEYWORDS, $COMMENTS, $RATINGS, $SETS, $GROUPS, $PROJECTS, $PARTS, $SNIPPETS, $STORIES;
	global $msg, $error;

	isset($ID) || $ID = $this->ID;
	$this->priceset = FetchPriceSetByID ($ID);
	return $this->priceset;
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
function SetAmountSoldOutside ($amount = 0) {
	
}



// =====================================================================================
} // class

?>