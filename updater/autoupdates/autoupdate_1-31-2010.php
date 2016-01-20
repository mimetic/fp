<?php
/*
 * An autoupdater for the FP system.
 * This PHP script is run by the index.php
 */
require_once "_config/sysconfig.inc";
require_once "_config/fpconfig.inc";
require_once "_config/config.inc";
require_once "includes/functions.inc";
require_once "includes/project_management.inc";

// ========= clear the website page caches ==================
// Assume that any change in Admin could change a page. Don't risk it,
// Clear all the caches for this website. The cache is now in tmp, locally,
// so we're not trashing cache for all users on the server
// However, we still have a problem with a multi-user system:
// we're clearing all cache... very inefficient!!!!
require_once('Cache/Lite.php');

// Set a few options
$options = array(
	'cacheDir' => 'tmp/cache/'
);

$Cache_Lite = new Cache_Lite($options);
$Cache_Lite->clean();
unset ($Cache_Lite);



$LINK = StartDatabase(MYSQLDB);
Setup ();
/*
 *
 * UPDATER CODE HERE
 */

$updaterdescription = "Update 1/22/2010: add new mysql fields";
//-----------------


mysqli_query ("ALTER TABLE `PriceSets` ADD COLUMN `a_EditionSize` TEXT;");
mysqli_query ("ALTER TABLE `PriceSets` ADD COLUMN `a_Size` TEXT;");
mysqli_query ("ALTER TABLE `PriceSets` ADD COLUMN `a_PrintCost` TEXT;");
mysqli_query ("ALTER TABLE `PriceSets` ADD COLUMN `a_PrintPrice` TEXT;");
mysqli_query ("ALTER TABLE `PriceSets` ADD COLUMN `a_Markup` TEXT;");
mysqli_query ("ALTER TABLE `PriceSets` ADD COLUMN `a_MatteCost` TEXT;");
mysqli_query ("ALTER TABLE `PriceSets` ADD COLUMN `a_MattePrice` TEXT;");
mysqli_query ("ALTER TABLE `PriceSets` ADD COLUMN `a_FrameToPrintCost` TEXT;");
mysqli_query ("ALTER TABLE `PriceSets` ADD COLUMN `a_FrameToPrintPrice` TEXT;");
mysqli_query ("ALTER TABLE `PriceSets` ADD COLUMN `a_FrameMatteCost` TEXT;");
mysqli_query ("ALTER TABLE `PriceSets` ADD COLUMN `a_FrameMattePrice` TEXT;");
mysqli_query ("ALTER TABLE `PriceSets` ADD COLUMN `a_Inactive` TEXT;");
mysqli_query ("ALTER TABLE `PriceSets` ADD COLUMN `a_PrintShipWeight` TEXT;");
mysqli_query ("ALTER TABLE `PriceSets` ADD COLUMN `a_MatteShipWeight` TEXT;");
mysqli_query ("ALTER TABLE `PriceSets` ADD COLUMN `a_FrameToPrintShipWeight` TEXT;");
mysqli_query ("ALTER TABLE `PriceSets` ADD COLUMN `a_FrameMatteShipWeight` TEXT;");
mysqli_query ("ALTER TABLE `PriceSets` ADD COLUMN `a_Amount` TEXT;");


mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `priceframed1`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `priceframed2`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `priceframed3`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `priceframed4`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `priceframed5`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `priceframed6`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `price1`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `price2`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `price3`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `price4`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `price5`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `price6`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `size1`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `size2`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `size3`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `size4`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `size5`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `size6`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `weight1`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `weight2`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `weight3`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `weight4`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `weight5`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `weight6`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `weightframed1`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `weightframed2`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `weightframed3`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `weightframed4`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `weightframed5`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `weightframed6`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `editionsize1`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `editionsize2`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `editionsize3`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `editionsize4`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `editionsize5`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `editionsize6`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `amount1`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `amount2`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `amount3`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `amount4`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `amount5`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `amount6`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `extrashipping1`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `extrashipping2`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `extrashipping3`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `extrashipping4`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `extrashipping5`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `extrashipping6`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `cost1`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `cost2`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `cost3`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `cost4`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `cost5`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `cost6`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `framecost1`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `framecost2`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `framecost3`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `framecost4`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `framecost5`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `framecost6`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `pricematted1`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `pricematted2`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `pricematted3`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `pricematted4`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `pricematted5`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `pricematted6`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `mattecost1`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `mattecost2`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `mattecost3`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `mattecost4`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `mattecost5`;");
mysqli_query ("ALTER TABLE `PriceSets` DROP COLUMN `mattecost6`;");
mysqli_query ("ALTER TABLE `PriceSets` ADD `SourceID` INT AFTER `ID`;");
mysqli_query ("ALTER TABLE `PriceSets` ADD `TotalEditionSize` INT AFTER `SupplierID`;");
mysqli_query ("ALTER TABLE `PriceSets` ADD `Inflation` INT;");


mysqli_query ("ALTER TABLE `Suppliers` ADD COLUMN `PrintMinPrice` FLOAT;");
mysqli_query ("ALTER TABLE `Suppliers` ADD COLUMN `PrintAreaPrice` FLOAT;");
mysqli_query ("ALTER TABLE `Suppliers` ADD COLUMN `PrintHandling` FLOAT;");
mysqli_query ("ALTER TABLE `Suppliers` ADD COLUMN `PrintHandlingIntl` FLOAT;");
mysqli_query ("ALTER TABLE `Suppliers` ADD COLUMN `PrintPacking` FLOAT;");
mysqli_query ("ALTER TABLE `Suppliers` ADD COLUMN `PrintDepth` FLOAT;");
mysqli_query ("ALTER TABLE `Suppliers` ADD COLUMN `PrintWeight` FLOAT;");
mysqli_query ("ALTER TABLE `Suppliers` ADD COLUMN `MatteMinPrice` FLOAT;");
mysqli_query ("ALTER TABLE `Suppliers` ADD COLUMN `MatteAreaPrice` FLOAT;");
mysqli_query ("ALTER TABLE `Suppliers` ADD COLUMN `MatteHandling` FLOAT;");
mysqli_query ("ALTER TABLE `Suppliers` ADD COLUMN `MatteHandlingIntl` FLOAT;");
mysqli_query ("ALTER TABLE `Suppliers` ADD COLUMN `MattePacking` FLOAT;");
mysqli_query ("ALTER TABLE `Suppliers` ADD COLUMN `MatteDepth` FLOAT;");
mysqli_query ("ALTER TABLE `Suppliers` ADD COLUMN `MatteWeight` FLOAT;");
mysqli_query ("ALTER TABLE `Suppliers` ADD COLUMN `FrameMinPrice` FLOAT;");
mysqli_query ("ALTER TABLE `Suppliers` ADD COLUMN `FrameAreaPrice` FLOAT;");
mysqli_query ("ALTER TABLE `Suppliers` ADD COLUMN `FrameHandling` FLOAT;");
mysqli_query ("ALTER TABLE `Suppliers` ADD COLUMN `FrameHandlingIntl` FLOAT;");
mysqli_query ("ALTER TABLE `Suppliers` ADD COLUMN `FramePacking` FLOAT;");
mysqli_query ("ALTER TABLE `Suppliers` ADD COLUMN `FrameDepth` FLOAT;");
mysqli_query ("ALTER TABLE `Suppliers` ADD COLUMN `FrameWeight` FLOAT;");
mysqli_query ("ALTER TABLE `Suppliers` ADD COLUMN `SalesTaxRate` FLOAT;");
mysqli_query ("ALTER TABLE `Suppliers` ADD COLUMN `PrintCostUnit` INT;");
mysqli_query ("ALTER TABLE `Suppliers` ADD COLUMN `PrintCostMethod` INT AFTER `PrintCostAreaUnit`;");
mysqli_query ("ALTER TABLE `Suppliers` ADD `MatchPrintPrice` FLOAT NOT NULL AFTER `Description`;");

mysqli_query ("ALTER TABLE `Artists` ADD COLUMN `Commission2` int(2) DEFAULT 0 AFTER `Commission`, ADD COLUMN `PayPalBusiness2` varchar(64) AFTER `PayPalBusiness`;");

mysqli_query ("ALTER TABLE `Paypal` ADD COLUMN `amount` float AFTER `id`;");
mysqli_query ("ALTER TABLE `Paypal` ADD COLUMN `shipping` float AFTER `amount`;");
mysqli_query ("ALTER TABLE `Paypal` ADD COLUMN `handling` float AFTER `shipping`;");
mysqli_query ("ALTER TABLE `Paypal` ADD COLUMN `discount_amount` float AFTER `handling`;");
mysqli_query ("ALTER TABLE `Paypal` ADD COLUMN `discount_rate` float AFTER `discount_amount`;");
mysqli_query ("ALTER TABLE `Paypal` ADD COLUMN `weight` float AFTER `discount_rate`;");
mysqli_query ("ALTER TABLE `Paypal` ADD COLUMN `weight_unit` INT AFTER `weight`;");
mysqli_query ("ALTER TABLE `Paypal` ADD COLUMN `on0` text AFTER `weight_unit`;");
mysqli_query ("ALTER TABLE `Paypal` ADD COLUMN `on1` text AFTER `on0`;");
mysqli_query ("ALTER TABLE `Paypal` ADD COLUMN `os0` text AFTER `on1`;");
mysqli_query ("ALTER TABLE `Paypal` ADD COLUMN `os1` text AFTER `os0`;");


mysqli_query ("ALTER TABLE `Sales` ADD `tax` FLOAT NOT NULL AFTER `shipping_method`;");
mysqli_query ("ALTER TABLE `Sales` ADD COLUMN `amount` float AFTER `payment_date`;");
mysqli_query ("ALTER TABLE `Sales` ADD `cost` FLOAT NOT NULL AFTER `weight_unit`;");
mysqli_query ("ALTER TABLE `Sales` ADD `MatchprintRequired` TINYINT;"); 
mysqli_query ("ALTER TABLE `Sales` ADD `SupplierID` INT AFTER `item_number`;");
mysqli_query ("ALTER TABLE `Sales` ADD `Size` INT AFTER `item_number`;");

 
//----------------- WRITE LOG -----------------
fp_error_log("AUTOUPDATER: $updaterdescription", 3, FP_MAINTENANCE_LOG);

/*
 * END UPDATER CODE
 */
mysqli_close($LINK);
//$FP_MYSQL_LINK->close();

/*
 * RENAMER:
 * This part of the script renames the file after running, so it won't be run again.
 */
$BASEDIR = dirname(__FILE__);

if (!$DEVELOPING) {

	// legacy fix
	$x = "$BASEDIR/_user/_audio/slideshow/.htaccess";
	if (file_exists($x)) {
		unlink ($x);
		fp_error_log("Fix access to slide show audio files.", 3, FP_MAINTENANCE_LOG);
	}
	$f = __FILE__;
	$k = 1;
	$ff = $f;
	while (file_exists($ff)) {
		$ff = $f.".completed.".$k;
		$k++;
	}
	rename(__FILE__, $ff);
}


?>