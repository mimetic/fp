<?php

// This shows a single picture with information about that picture.
// It's for making certificates of authenticity


include "_config/sysconfig.inc";
include "_config/fpconfig.inc";
include "_config/config.inc";
include "includes/functions.inc";
include "includes/project_management.inc";
include "includes/image_management.inc";
include "includes/commerce.inc";

$error = "";
$msg = "";

$LINK = StartDatabase(MYSQLDB);
Setup ();

session_name("fp_admin");
session_start();

$results = GetFormInput();
$actions = $results['actions'];
$action = $results['actions']['action'];
$vars = $results['vars'];
isset($vars['GroupID']) && $_SESSION['GroupID'] = $vars['GroupID'];
isset($_SESSION['GroupID']) || $_SESSION['GroupID'] = PUBLIC_GROUP_ID;

// isset($_REQUEST['theme']) && $_SESSION['theme'] = $_REQUEST['theme'];
isset($_SESSION['theme']) || $_SESSION['theme'] = DEFAULT_THEME;

$_SESSION['theme'] = ADMIN_THEME;

// Get user info, if it exists. It would come because the admin passed a user ID variable
$vars['fp_user'] && $_SESSION['fp_user'] = $vars['fp_user'];

// Get Sale Info
$saleID = $vars['saleID'];

if ($saleID) {
	
	
	$sale = FetchSale($saleID);
	$size = $sale['Size'];
	$sizeForDisplay = PrintSizeForDisplay ($image, $size);
	// If matchprint required, then we note there is an artist's proof print.
	if ($sale['MatchprintRequired']) {
		$artistProof = FetchSnippet("coa_artist_proof");
	}
	$printNumber = $sale['PrintNumber'];
	
	
	//$imageID = $vars['ImageID'];
	$imageID = $sale['item_id'];
	$image = FetchImage ($imageID);
	$image['Date'] = strftime("%B %e, %G", mysqldate_to_timestamp($image['CreatedDate']));
	$image["Place"] = ($image['City'] and $image['Country']) ? $image['City'].", ".$image['Country'] : $image['Country'];
	$image["Artist"] = join (" ", StripBlankFields (array ($artist["Firstname"] ,  $artist['Lastname'])));
	$image['FullCredit'] = join (" : ", StripBlankFields (array ($image["Place"], $image['Date'], $image["Artist"])));
	
	$fields['ITEM_NAME'] = htmlspecialchars($image['Title'], ENT_QUOTES);
	
	
	
	// Get Artist Info
	$artistID = $image['ArtistID'];
	$artist = FetchArtist ($artistID);
	
	// rename keys to avoid possible bad substitutions
	foreach ($artist as $k => $v) {
		$artistinfo[DB_ARTISTS."_$k"] = $v;
	}
	
	// Project Info
	$projectID = $vars['ProjectID'];
	$project = FetchProject ($projectID);
	
	// Get sales settings
	$salesinfo = ImageSalesInfo ($imageID, $projectID);
	$sizeIndex = array_search($size, $salesinfo['index']);
	$printSizeFormatted = $salesinfo['printsizesformatted'][$sizeIndex];
	$paperSizeFormatted = $salesinfo['papersizesformatted'][$sizeIndex];
	
	// $supplierID = $artist['Vendor'];	<-- wrong...artist could change vendors, but an image stays with original printer
	$supplierID = $salesinfo['supplierID'];
	$supplier = GetRecord( DB_SUPPLIERS, $supplierID);
	
	// INK/PAPER
	// Get paper/ink from fixed price set, NOT supplier. Their offering might change
	$priceset = GetPriceset($image['PriceSetID']);
	$mainfields['PAPER'] = $priceset['Paper'];
	$mainfields['INKSET'] = $priceset['Inkset'];
	
	
	// -------
	// Picture 
	
	// What size pictures?
	$WHICHPIX = $SLIDES;
	
	$URL = $image["URL"];
	$fields['WHICHPIX'] = $WHICHPIX;
	$fields['URL'] = $URL;
	
	
	// CALC SCREEN SIZES
	// Set sizes for screen display
	// Draw with or without a matte
	$imgsize = GetLocalImageSize("$BASEDIR/$WHICHPIX/$URL");
	// Fit picture into space allocated
	if ($imgsize[0] > $imgsize[1]) {
		$w = COA_IMAGE_W;
		$h = floor($imgsize[1] * (COA_IMAGE_W/$imgsize[0]));
	} else {
		$h = COA_IMAGE_H;
		$w = floor($imgsize[0] * (COA_IMAGE_H/$imgsize[1]));
	}
	
	$fields['IMG_Width'] = $w;
	$fields['IMG_Height'] = $h;
	
	$fields['SUPPLIER_NAME'] = $supplier["Name"];
	$fields['ISSUING_AUTHORITY'] = $supplier["Name"]." (issuing authority)";
	
	$fields['SUPPLIER_FULL_ADDRESS'] = $supplier['Address1'].', '
		.($supplier['Address2'] ? $supplier['Address2'].', ' : '')
		.$supplier['City'].', '
		.($supplier['State'] ? $supplier['State'].' ' : '')
		.$supplier['Zip'].', '
		.$supplier['Country'];
	
	$fields['SUPPLIER_EMAIL'] = $supplier['Email'];
	$fields['SUPPLIER_TEL'] = $supplier['Tel1'];
	
	$fields['SUPPLIER_FULL_ADDRESS_LINES'] = $supplier['Address1'].'<br>'
		.($supplier['Address2'] ? $supplier['Address2'].'<br>' : '')
		.$supplier['City'].', '
		.($supplier['State'] ? $supplier['State'].' ' : '')
		.$supplier['Zip'].'<br>'
		.$supplier['Country'];
	
	
	
	// CATALOG NUMBER USES IMAGE ID
	$artistInitials = $artist["Firstname"][0] . $artist["Lastname"][0];
	$catalogNum = $artistInitials . "-". str_pad($imageID, 6, "0", STR_PAD_LEFT);	//left-pad with zeros
	$fields['CATALOGNUMBER'] = $catalogNum;
	
	
	$salesinfo = ImageSalesInfo ($imageID, $projectID);
	
	$fields['SIZES'] = join(",",$salesinfo['printsizes']);
	
	$fields['DIMENSIONS'] = join(",",$salesinfo['dims']);
	
	// SUPPLIER ID : WHICH SUPPLIER IS THE ARTIST USING?
	$fields['SUPPLIERID'] = $salesinfo['supplierID'];
	
	$invoice = date("Ymd-G:i:s");			// e.g. 20080214-15:16:08
	$fields['INVOICE'] = $invoice;
	
	// Full original offering of the entire edition
	$allPriceSet = ImageSalesInfo ($imageID, '', true);
	$allPrintsizes = $allPriceSet['printsizes'];
	is_array($allPrintsizes) || $allPrintsizes = array ($allPrintsizes);
	$allprices = $allPriceSet['PrintPrices'];
	$allavailable = $allPriceSet['available'];
	$alleditionsizes = $allPriceSet['editionsizes'];
	
	$totaledition = $allPriceSet['totaledition'];
	$totalsold = $allPriceSet['totalsold'];
	if ($totaledition > 0) {
		$available = $totaledition - $totalsold;
	} else {
		$available = "unlimited";
	}
	
	// Buttons across top:
	// SHOW BLANK FORM ONLY
	$showblankformonly = "false";
} else {
	// Buttons across top:
	// SHOW BLANK FORM ONLY
	if (isset ($vars['artistID'])) {
		$artistID = $vars['artistID'];
		$artist = FetchArtist ($artistID);
		$fields['Artists_Fullname'] = join (" ", StripBlankFields (array ($artist["Firstname"] ,  $artist['Lastname'])));
}
	
	// But, show supplier name if sent
	if (isset ($vars['supplierID'])) {
		$supplierID = $vars['supplierID'];
		$supplier = GetRecord( DB_SUPPLIERS, $supplierID);
		if ($supplier["Name"]) {
			$fields['SUPPLIER_NAME'] = $supplier["Name"];
			$fields['ISSUING_AUTHORITY'] = $supplier["Name"]." (issuing authority)";
		}
	}
	$showblankformonly= "true";
}


// page to return to is gallery we just came from
//$returnURL = urlencode("http://".$SYSTEMURL."gallery.php?ProjectID=".$projectID);
$returnURL = "http://".$SYSTEMURL."gallery.php?ProjectID=".$projectID;

$page = FetchSnippet ("coa_master_page");

// Picture
$picture = Substitutions (FetchSnippet ("coa_picture"), $image);
$picture = Substitutions ($picture, $project);
$picture = Substitutions ($picture, $mainfields);
$picture = Substitutions ($picture, $fields);

// Info
$text = Substitutions (FetchSnippet ("coa_text"), $image);
$text = Substitutions ($text, $project);
$text = Substitutions ($text, $mainfields);
$text = Substitutions ($text, $fields);

$textblank = FetchSnippet ("coa_text_blank");

$navbar = FetchSnippet ('navbar-goback');	// we use of limited navbar on this page

$page = Substitutions ($page, $mainfields);
$info = array(
	'META_INDEX'					=> FetchSnippet ('meta_robots_noindex'),
	'NAVBAR'					=> $navbar,
	'NAVBAR_2'				=> $navbar2,
	'GALLERY_STYLESHEET'		=> "",
	'header'					=> $header,
	'text'					=> $text,
	'TEXT_BLANK'				=> $textblank,
	'image'					=> $picture,
	'pagetitle' 				=> FP_COA_TITLE,
	'imagetitle' 			=> $image['Title'],
	'title' 					=> FP_COA_TITLE,
	'projectTitle' 			=> $project['Title'],
	'ProjectID'				=> $projectID,
	'grouptitle'				=> $myGroup->title,
	'grouplogo'				=> $myGroup->icon,
	'GROUPICON'				=> $GroupIcon,
	'GROUPBANNER'			=> $GroupBanner,
	'GROUPBANNERURL'			=> $GroupBannerURL,
	'sectionclass'			=> "",	//class name for some objects
	'message'	 			=> $msg,
	'error' 					=> $error,
	'master_page_popups'		=> FetchSnippet("client_access_dialog"),
	'pps' 					=> $project['pps'],
	'slideshowpopup'			=> $sspop,
	'ImageID'				=> $imageID,
	'EDITIONSIZE'			=> $totaledition,
	'AVAILABLE'				=> $available,
	'IMAGESIZE'				=> $printSizeFormatted,
	'PAPERSIZE'				=> $paperSizeFormatted,
	'ARTISTPROOF'			=> $artistProof,
	'PRINTNUMBER'			=> $printNumber,
	'RETURNURL'			=> $returnURL,
	'RETURNURL_ENCODED'		=> urlencode($returnURL),
	'FP_COA_WIDTH'			=> FP_COA_WIDTH,
	'FP_COA_HEIGHT'			=> FP_COA_HEIGHT,
	'SHOWBLANKFORMONLY'		=> $showblankformonly
);

$page = Substitutions ($page, $info);
$page = Substitutions ($page, $artistinfo);
$page = ReplaceAllSnippets ($page);
$page = ReplaceSysVars ($page);
$page = DeleteUnusedSnippets ($page);

print $page;

mysqli_close($LINK);
//$FP_MYSQL_LINK->close();
?>