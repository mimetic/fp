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


mysql_query("ALTER TABLE `PriceSets` ADD COLUMN `a_EditionSize` TEXT;");
mysql_query("ALTER TABLE `PriceSets` ADD COLUMN `a_Size` TEXT;");
mysql_query("ALTER TABLE `PriceSets` ADD COLUMN `a_PrintCost` TEXT;");
mysql_query("ALTER TABLE `PriceSets` ADD COLUMN `a_PrintPrice` TEXT;");
mysql_query("ALTER TABLE `PriceSets` ADD COLUMN `a_Markup` TEXT;");
mysql_query("ALTER TABLE `PriceSets` ADD COLUMN `a_MatteCost` TEXT;");
mysql_query("ALTER TABLE `PriceSets` ADD COLUMN `a_MattePrice` TEXT;");
mysql_query("ALTER TABLE `PriceSets` ADD COLUMN `a_FrameToPrintCost` TEXT;");
mysql_query("ALTER TABLE `PriceSets` ADD COLUMN `a_FrameToPrintPrice` TEXT;");
mysql_query("ALTER TABLE `PriceSets` ADD COLUMN `a_FrameMatteCost` TEXT;");
mysql_query("ALTER TABLE `PriceSets` ADD COLUMN `a_FrameMattePrice` TEXT;");
mysql_query("ALTER TABLE `PriceSets` ADD COLUMN `a_Inactive` TEXT;");
mysql_query("ALTER TABLE `PriceSets` ADD COLUMN `a_PrintShipWeight` TEXT;");
mysql_query("ALTER TABLE `PriceSets` ADD COLUMN `a_MatteShipWeight` TEXT;");
mysql_query("ALTER TABLE `PriceSets` ADD COLUMN `a_FrameToPrintShipWeight` TEXT;");
mysql_query("ALTER TABLE `PriceSets` ADD COLUMN `a_FrameMatteShipWeight` TEXT;");
mysql_query("ALTER TABLE `PriceSets` ADD COLUMN `a_Amount` TEXT;");


mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `priceframed1`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `priceframed2`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `priceframed3`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `priceframed4`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `priceframed5`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `priceframed6`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `price1`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `price2`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `price3`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `price4`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `price5`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `price6`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `size1`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `size2`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `size3`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `size4`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `size5`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `size6`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `weight1`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `weight2`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `weight3`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `weight4`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `weight5`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `weight6`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `weightframed1`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `weightframed2`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `weightframed3`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `weightframed4`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `weightframed5`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `weightframed6`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `editionsize1`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `editionsize2`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `editionsize3`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `editionsize4`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `editionsize5`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `editionsize6`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `amount1`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `amount2`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `amount3`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `amount4`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `amount5`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `amount6`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `extrashipping1`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `extrashipping2`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `extrashipping3`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `extrashipping4`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `extrashipping5`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `extrashipping6`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `cost1`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `cost2`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `cost3`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `cost4`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `cost5`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `cost6`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `framecost1`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `framecost2`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `framecost3`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `framecost4`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `framecost5`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `framecost6`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `pricematted1`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `pricematted2`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `pricematted3`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `pricematted4`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `pricematted5`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `pricematted6`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `mattecost1`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `mattecost2`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `mattecost3`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `mattecost4`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `mattecost5`;");
mysql_query("ALTER TABLE `PriceSets` DROP COLUMN `mattecost6`;");
mysql_query("ALTER TABLE `PriceSets` ADD `SourceID` INT AFTER `ID`;");
mysql_query("ALTER TABLE `PriceSets` ADD `TotalEditionSize` INT AFTER `SupplierID`;");
mysql_query("ALTER TABLE `PriceSets` ADD `Inflation` INT;");


mysql_query("ALTER TABLE `Suppliers` ADD COLUMN `PrintMinPrice` FLOAT;");
mysql_query("ALTER TABLE `Suppliers` ADD COLUMN `PrintAreaPrice` FLOAT;");
mysql_query("ALTER TABLE `Suppliers` ADD COLUMN `PrintHandling` FLOAT;");
mysql_query("ALTER TABLE `Suppliers` ADD COLUMN `PrintHandlingIntl` FLOAT;");
mysql_query("ALTER TABLE `Suppliers` ADD COLUMN `PrintPacking` FLOAT;");
mysql_query("ALTER TABLE `Suppliers` ADD COLUMN `PrintDepth` FLOAT;");
mysql_query("ALTER TABLE `Suppliers` ADD COLUMN `PrintWeight` FLOAT;");
mysql_query("ALTER TABLE `Suppliers` ADD COLUMN `MatteMinPrice` FLOAT;");
mysql_query("ALTER TABLE `Suppliers` ADD COLUMN `MatteAreaPrice` FLOAT;");
mysql_query("ALTER TABLE `Suppliers` ADD COLUMN `MatteHandling` FLOAT;");
mysql_query("ALTER TABLE `Suppliers` ADD COLUMN `MatteHandlingIntl` FLOAT;");
mysql_query("ALTER TABLE `Suppliers` ADD COLUMN `MattePacking` FLOAT;");
mysql_query("ALTER TABLE `Suppliers` ADD COLUMN `MatteDepth` FLOAT;");
mysql_query("ALTER TABLE `Suppliers` ADD COLUMN `MatteWeight` FLOAT;");
mysql_query("ALTER TABLE `Suppliers` ADD COLUMN `FrameMinPrice` FLOAT;");
mysql_query("ALTER TABLE `Suppliers` ADD COLUMN `FrameAreaPrice` FLOAT;");
mysql_query("ALTER TABLE `Suppliers` ADD COLUMN `FrameHandling` FLOAT;");
mysql_query("ALTER TABLE `Suppliers` ADD COLUMN `FrameHandlingIntl` FLOAT;");
mysql_query("ALTER TABLE `Suppliers` ADD COLUMN `FramePacking` FLOAT;");
mysql_query("ALTER TABLE `Suppliers` ADD COLUMN `FrameDepth` FLOAT;");
mysql_query("ALTER TABLE `Suppliers` ADD COLUMN `FrameWeight` FLOAT;");
mysql_query("ALTER TABLE `Suppliers` ADD COLUMN `SalesTaxRate` FLOAT;");
mysql_query("ALTER TABLE `Suppliers` ADD COLUMN `PrintCostUnit` INT;");
mysql_query("ALTER TABLE `Suppliers` ADD COLUMN `PrintCostMethod` INT AFTER `PrintCostAreaUnit`;");
mysql_query("ALTER TABLE `Suppliers` ADD `MatchPrintPrice` FLOAT NOT NULL AFTER `Description`;");

mysql_query("ALTER TABLE `Artists` ADD COLUMN `Commission2` int(2) DEFAULT 0 AFTER `Commission`, ADD COLUMN `PayPalBusiness2` varchar(64) AFTER `PayPalBusiness`;");

mysql_query("ALTER TABLE `Paypal` ADD COLUMN `amount` float AFTER `id`;");
mysql_query("ALTER TABLE `Paypal` ADD COLUMN `shipping` float AFTER `amount`;");
mysql_query("ALTER TABLE `Paypal` ADD COLUMN `handling` float AFTER `shipping`;");
mysql_query("ALTER TABLE `Paypal` ADD COLUMN `discount_amount` float AFTER `handling`;");
mysql_query("ALTER TABLE `Paypal` ADD COLUMN `discount_rate` float AFTER `discount_amount`;");
mysql_query("ALTER TABLE `Paypal` ADD COLUMN `weight` float AFTER `discount_rate`;");
mysql_query("ALTER TABLE `Paypal` ADD COLUMN `weight_unit` INT AFTER `weight`;");
mysql_query("ALTER TABLE `Paypal` ADD COLUMN `on0` text AFTER `weight_unit`;");
mysql_query("ALTER TABLE `Paypal` ADD COLUMN `on1` text AFTER `on0`;");
mysql_query("ALTER TABLE `Paypal` ADD COLUMN `os0` text AFTER `on1`;");
mysql_query("ALTER TABLE `Paypal` ADD COLUMN `os1` text AFTER `os0`;");


mysql_query("ALTER TABLE `Sales` ADD `tax` FLOAT NOT NULL AFTER `shipping_method`;");
mysql_query("ALTER TABLE `Sales` ADD COLUMN `amount` float AFTER `payment_date`;");
mysql_query("ALTER TABLE `Sales` ADD `cost` FLOAT NOT NULL AFTER `weight_unit`;");
mysql_query("ALTER TABLE `Sales` ADD `MatchprintRequired` TINYINT;"); 
mysql_query("ALTER TABLE `Sales` ADD `SupplierID` INT AFTER `item_number`;");
mysql_query("ALTER TABLE `Sales` ADD `Size` INT AFTER `item_number`;");

 
//----------------- WRITE LOG -----------------
fp_error_log("AUTOUPDATER: $updaterdescription", 3, FP_MAINTENANCE_LOG);

/*
 * END UPDATER CODE
 */
mysql_close($LINK);
$FP_MYSQL_LINK->close();

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