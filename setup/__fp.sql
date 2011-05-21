-- MySQL dump 10.9
--
-- Host: localhost    Database: TEMP-FP
-- ------------------------------------------------------
-- Server version	5.0.27-standard

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE="NO_AUTO_VALUE_ON_ZERO" */;

--
-- Table structure for table `Artists`
--

DROP TABLE IF EXISTS `Artists`;
CREATE TABLE `Artists` (
  `ID` bigint(20) unsigned NOT NULL auto_increment,
  `Commission` int(2) default NULL,
  `Username` varchar(64) default NULL,
  `Password` varchar(64) default NULL,
  `AccessLevel` int(3) default NULL,
  `UserLevel` int(2) default '2',
  `Storage` int(11) default NULL,
  `Password_Reminder` varchar(64) default NULL,
  `Firstname` varchar(64) default NULL,
  `Middlename` varchar(64) default NULL,
  `Lastname` varchar(64) default NULL,
  `Agency` varchar(64) default NULL,
  `Tel1` varchar(32) default NULL,
  `Tel2` varchar(32) default NULL,
  `Tel3` varchar(32) default NULL,
  `Tel4` varchar(32) default NULL,
  `Address1` varchar(64) default NULL,
  `Address2` varchar(64) default NULL,
  `City` varchar(64) default NULL,
  `State` varchar(64) default NULL,
  `Zip` varchar(64) default NULL,
  `Country` varchar(64) default NULL,
  `Email` varchar(64) default NULL,
  `Email2` varchar(64) default NULL,
  `Website` varchar(64) default NULL,
  `Biography` text,
  `Statement` text,
  `DefaultPriceID` bigint(20) unsigned default NULL,
  `DefaultPriceSetID` bigint(20) default '1',
  `DefaultEditionSize` int(11) default NULL,
  `DefaultMatted` set('yes','no') default NULL,
  `DefaultCopyrightNotice` varchar(255) default NULL,
  `DefaultCredit` varchar(255) default NULL,
  `DefaultLifespan` int(11) default '14',
  `DefaultActiveLifespan` int(11) default '30',
  `Active` set('yes','no') default 'yes',
  `Featured` set('yes','no') default 'yes',
  `Timestamp` timestamp default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  `PortraitImageID` bigint(20) unsigned default NULL,
  `ProjectID` bigint(20) unsigned default NULL,
  `OutsourceID1` varchar(50) default NULL,
  `OutsourceID2` varchar(50) default NULL,
  `OutsourceID3` varchar(50) default NULL,
  `OutsourceID4` varchar(50) default NULL,
  `OutsourceID5` varchar(50) default NULL,
  `ftp_server` varchar(50) default NULL,
  `ftp_directory` varchar(64) default NULL,
  `ftp_user_name` varchar(50) default NULL,
  `ftp_user_pass` varchar(50) default NULL,
  `ftp_proj_is_dir` int(1) default '1',
  `Confirmed` varchar(50) default NULL,
  `SubscriptionID` int(11) default NULL,
  `SubscriptionDesc` text,
  `ActivationHash` varchar(150) default '',
  `CreationDate` datetime default '0000-00-00 00:00:00',
  `ShortName` varchar(32) default '',
  `PrintSaleCode` text,
  `PayPalBusiness` varchar(64) default NULL,
  `PrintSalesID` varchar(50) default NULL,
  `GroupID` bigint(20) default '1',
  `PictureFrameWidth` int(1) default '12',
  `PictureFrameColor` varchar(7) default '000000',
  `Vendor` int(3) default '1',
  `Ecommerce` int(3) default NULL,
  `NotesToVendor` text,
  `Awards` text,
  `FullBiography` text,
  `Exhibitions` text,
  `Publications` text,
  `Params` text,
  PRIMARY KEY  (`ID`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- Table structure for table `Comments`
--

DROP TABLE IF EXISTS `Comments`;
CREATE TABLE `Comments` (
  `ID` bigint(20) unsigned NOT NULL auto_increment,
  `ImageID` bigint(20) unsigned default NULL,
  `Comment` varchar(255) default NULL,
  `IP` varchar(96) default NULL,
  `ParentTopicID` bigint(20) unsigned default NULL,
  PRIMARY KEY  (`ID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- Table structure for table `Groups`
--

DROP TABLE IF EXISTS `Groups`;
CREATE TABLE `Groups` (
  `ID` bigint(20) unsigned NOT NULL auto_increment,
  `Title` varchar(64) default NULL,
  `Description` text,
  `ArtistID` bigint(20) default NULL,
  `Public` tinyint(1) default '0',
  `Icon` varchar(64) default NULL,
  `URL` varchar(64) default NULL,
  `Statement` text,
  `Theme` text,
  `Params` text,
  PRIMARY KEY  (`ID`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- Table structure for table `Images`
--

DROP TABLE IF EXISTS `Images`;
CREATE TABLE `Images` (
  `ID` bigint(20) unsigned NOT NULL auto_increment,
  `Title` varchar(255) default 'Untitled',
  `Type` set('normal','sample','thumbnail') default 'normal',
  `RollID` varchar(32) default '0',
  `FrameID` varchar(32) default '0',
  `ArtistID` bigint(20) unsigned default '0',
  `SetID` bigint(20) unsigned default '0',
  `ProjectID` bigint(20) unsigned default '0',
  `PriceID` bigint(20) unsigned default '0',
  `PriceSetID` bigint(20) default NULL,
  `URL` varchar(255) default NULL,
  `Status` set('hold','current','archive') default 'current',
  `Lifespan` int(11) default NULL,
  `Medium` varchar(64) default NULL,
  `Delivery` int(11) default NULL,
  `Caption` text,
  `Headline` varchar(255) default NULL,
  `SpecialInstructions` varchar(255) default NULL,
  `Byline` varchar(64) default NULL,
  `BylineTitle` varchar(64) default NULL,
  `Credit` varchar(64) default NULL,
  `Source` varchar(64) default NULL,
  `ObjectName` varchar(64) default NULL,
  `CreatedDate` date default NULL,
  `City` varchar(64) default NULL,
  `State` varchar(64) default NULL,
  `Country` varchar(64) default NULL,
  `Copyrighted` set('yes','no') default NULL,
  `CopyrightNotice` varchar(255) default NULL,
  `Featured` set('yes','no') default 'no',
  `Active` set('yes','no') default 'yes',
  `Timestamp` timestamp default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  `IPTCSubjectCode` varchar(12) default NULL,
  `Keywords` text,
  `editionsize1` int(5) default NULL,
  `editionsize2` int(5) default NULL,
  `editionsize3` int(5) default NULL,
  `editionsize4` int(5) default NULL,
  `editionsize5` int(5) default NULL,
  `editionsize6` int(5) default NULL,
  `amount1` int(5) default '0',
  `amount2` int(5) default '0',
  `amount3` int(5) default '0',
  `amount4` int(5) default '0',
  `amount5` int(5) default '0',
  `amount6` int(5) default '0',
  `size1` float default NULL,
  `size2` float default NULL,
  `size3` float default NULL,
  `size4` float default NULL,
  `size5` float default NULL,
  `size6` float default NULL,
  `EditionsLocked` tinyint(4) default '0',
  `Params` text,
  PRIMARY KEY  (`ID`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- Table structure for table `Keywords`
--

DROP TABLE IF EXISTS `Keywords`;
CREATE TABLE `Keywords` (
  `ID` bigint(20) unsigned NOT NULL auto_increment,
  `Keyword` varchar(255) default NULL,
  `ImageID` bigint(20) unsigned default '0',
  `ParentTopicID` bigint(20) unsigned default NULL,
  `Description` varchar(255) default NULL,
  `Views` int(11) default '0',
  `Rating` float default '0',
  PRIMARY KEY  (`ID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- Table structure for table `Parts`
--

DROP TABLE IF EXISTS `Parts`;
CREATE TABLE `Parts` (
  `ID` bigint(20) unsigned NOT NULL auto_increment,
  `ProjectID` bigint(20) unsigned default '0',
  `ArtistID` bigint(20) unsigned default '0',
  `PartTable` varchar(32) default 'Artists',
  `PartID` bigint(20) unsigned default '0',
  `OrderInGallery` int(2) unsigned default NULL,
  `OrderInProject` int(2) unsigned default NULL,
  PRIMARY KEY  (`ID`),
  KEY `OrderInProject` (`OrderInProject`),
  KEY `ProjectID` (`ProjectID`),
  KEY `PartID` (`PartID`),
  KEY `OrderInGallery` (`OrderInGallery`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- Table structure for table `Payments`
--

DROP TABLE IF EXISTS `Payments`;
CREATE TABLE `Payments` (
  `ID` bigint(20) NOT NULL auto_increment,
  `Amount` decimal(9,2) default NULL,
  `Email` varchar(60) default NULL,
  `UniqueID` varchar(30) default NULL,
  `Note` text,
  `SaleID` bigint(20) default NULL,
  `Payee` varchar(50) default NULL,
  `DateTime` datetime default NULL,
  PRIMARY KEY  (`ID`),
  KEY `SaleID` (`SaleID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- Table structure for table `Paypal`
--

DROP TABLE IF EXISTS `Paypal`;
CREATE TABLE `Paypal` (
  `id` int(11) NOT NULL auto_increment,
  `pp_comments` text,
  `pp_user_id` bigint(20) default NULL,
  `pp_service_id` bigint(20) default NULL,
  `pp_package_id` int(11) default NULL,
  `pp_status` text,
  `address_city` varchar(255) default '',
  `address_country` varchar(255) default '',
  `address_name` varchar(255) default '',
  `address_state` varchar(255) default '',
  `address_status` varchar(255) default '',
  `address_street` varchar(255) default '',
  `address_zip` varchar(255) default '',
  `amount1` varchar(127) default '0',
  `amount2` varchar(127) default '0',
  `amount3` varchar(127) default '0',
  `business` varchar(127) default NULL,
  `comment` varchar(255) default NULL,
  `coupon_code` varchar(15) default 'none',
  `custom` varchar(255) default NULL,
  `exchange_rate` varchar(127) default '0',
  `first_name` varchar(127) default NULL,
  `init_pass` varchar(8) default 'UNK',
  `invoice` varchar(127) default '0',
  `IP` varchar(15) default '000.000.000.000',
  `item_name` varchar(127) default 'UNK',
  `item_number` varchar(127) default '',
  `last_change` timestamp default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  `last_name` varchar(127) default NULL,
  `mc_amount1` varchar(127) default '0',
  `mc_amount2` varchar(127) default '0',
  `mc_amount3` varchar(127) default '0',
  `mc_currency` varchar(127) default 'USD',
  `mc_fee` varchar(127) default '0',
  `mc_gross` varchar(127) default '0',
  `mc_handling` decimal(9,2) default NULL,
  `mc_shipping` decimal(9,2) default NULL,
  `memo` tinytext,
  `notify_version` varchar(127) default NULL,
  `num_cart_items` int(2) default NULL,
  `option_name1` varchar(60) default NULL,
  `option_name2` varchar(60) default NULL,
  `option_selection1` varchar(200) default 'UNK',
  `option_selection2` varchar(200) default 'UNK',
  `parent_txn_id` varchar(127) default NULL,
  `password` varchar(127) default '0',
  `payer_email` varchar(75) default NULL,
  `payer_id` varchar(60) default NULL,
  `payer_status` varchar(50) default NULL,
  `payment_date` varchar(50) default NULL,
  `payment_fee` varchar(127) default '0',
  `payment_gross` varchar(127) default '0',
  `payment_status` varchar(127) default NULL,
  `payment_type` varchar(50) default NULL,
  `pending_reason` varchar(255) default '',
  `period1` varchar(127) default 'UNK',
  `period2` varchar(127) default 'UNK',
  `period3` varchar(127) default 'UNK',
  `quantity` int(11) default '0',
  `reason_code` varchar(127) default NULL,
  `reattempt` varchar(127) default '1',
  `receiver_email` varchar(127) default NULL,
  `recur_times` varchar(127) default '0',
  `recurring` varchar(127) default '1',
  `retry_at` varchar(127) default NULL,
  `settle_amount` varchar(127) default '0',
  `settle_currency` varchar(127) default 'USD',
  `shipping` decimal(9,2) default NULL,
  `shipping_method` varchar(50) default NULL,
  `subscr_date` varchar(127) default '0',
  `subscr_effective` varchar(127) default '0',
  `subscr_id` varchar(127) default NULL,
  `tax` decimal(9,2) default NULL,
  `txn_id` varchar(50) default NULL,
  `txn_type` varchar(255) default '',
  `username` varchar(127) default '0',
  `verify_sign` varchar(127) default NULL,
  `whm_name` varchar(127) default 'UNK',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `txn_id` (`txn_id`),
  KEY `item_number` (`item_number`)
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

--
-- Table structure for table `PriceSets`
--

DROP TABLE IF EXISTS `PriceSets`;
CREATE TABLE `PriceSets` (
  `ID` bigint(20) NOT NULL auto_increment,
  `ArtistID` bigint(20) default NULL,
  `ImageID` bigint(20) default '0',
  `SupplierID` bigint(20) default NULL,
  `Title` varchar(50) default NULL,
  `matchprintcost` float default NULL,
  `price1` float default NULL,
  `price2` float default NULL,
  `price3` float default NULL,
  `price4` float default NULL,
  `price5` float default NULL,
  `price6` float default NULL,
  `priceframed1` float default NULL,
  `priceframed2` float default NULL,
  `priceframed3` float default NULL,
  `priceframed4` float default NULL,
  `priceframed5` float default NULL,
  `priceframed6` float default NULL,
  `size1` float default NULL,
  `size2` float default NULL,
  `size3` float default NULL,
  `size4` float default NULL,
  `size5` float default NULL,
  `size6` float default NULL,
  `weight1` int(3) default NULL,
  `weight2` int(3) default NULL,
  `weight3` int(3) default NULL,
  `weight4` int(3) default NULL,
  `weight5` int(3) default NULL,
  `weight6` int(3) default NULL,
  `weightframed1` int(3) default NULL,
  `weightframed2` int(3) default NULL,
  `weightframed3` int(3) default NULL,
  `weightframed4` int(3) default NULL,
  `weightframed5` int(3) default NULL,
  `weightframed6` int(3) default NULL,
  `editionsize1` int(5) default '10',
  `editionsize2` int(5) default '10',
  `editionsize3` int(5) default '10',
  `editionsize4` int(5) default '10',
  `editionsize5` int(5) default '10',
  `editionsize6` int(5) default '10',
  `amount1` int(5) default NULL,
  `amount2` int(5) default NULL,
  `amount3` int(5) default NULL,
  `amount4` int(5) default NULL,
  `amount5` int(5) default NULL,
  `amount6` int(5) default NULL,
  `extrashipping1` float default NULL,
  `extrashipping2` float default NULL,
  `extrashipping3` float default NULL,
  `extrashipping4` float default NULL,
  `extrashipping5` float default NULL,
  `extrashipping6` float default NULL,
  `cost1` float default NULL,
  `cost2` float default NULL,
  `cost3` float default NULL,
  `cost4` float default NULL,
  `cost5` float default NULL,
  `cost6` float default NULL,
  `framecost1` float default NULL,
  `framecost2` float default NULL,
  `framecost3` float default NULL,
  `framecost4` float default NULL,
  `framecost5` float default NULL,
  `framecost6` float default NULL,
  `Paper` varchar(100) default NULL,
  `Inkset` varchar(100) default NULL,
  `PaperCode` varchar(32) default NULL,
  `InksetCode` varchar(32) default NULL,
  PRIMARY KEY  (`ID`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- Table structure for table `Projects`
--

DROP TABLE IF EXISTS `Projects`;
CREATE TABLE `Projects` (
  `ID` bigint(20) unsigned NOT NULL auto_increment,
  `ArtistID` bigint(20) unsigned NOT NULL default '0',
  `Title` varchar(255) default NULL,
  `ProjectDate` date default NULL,
  `LastUpdate` date default NULL,
  `Lifespan` int(4) unsigned default '30',
  `ActiveLifespan` int(4) unsigned default '30',
  `Statement` text,
  `FeaturedX` int(1) default NULL,
  `ActiveX` int(1) default NULL,
  `Description` text,
  `City` varchar(64) default NULL,
  `Country` varchar(64) default NULL,
  `Views` int(11) unsigned default '0',
  `Rating` float default '0',
  `Timestamp` timestamp  default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  `Nickname` varchar(64) default NULL,
  `Matted` int(1) unsigned default '0',
  `MaxPix` int(2) unsigned default '15',
  `ProjectPassword` varchar(64) default NULL,
  `pps` varchar(50) default 'default',
  `PriceSetID` bigint(20) default '0',
  `GroupID` bigint(20) unsigned  default '1',
  `RSS` varchar(80) default NULL,
  `Framewidth` smallint(5) unsigned default '0',
  `Framestyle` smallint(5) unsigned default '1',
  `Public` tinyint(1) default '0',
  `PaperCode` varchar(32) default NULL,
  `InksetCode` varchar(32) default NULL,
  `Matchprint` int(1) default NULL,
  `Audio` varchar(64) default NULL,
  `SlideShowDuration` int(6) default NULL,
  `Params` text,
  `Slides` tinyint(1)  default '0',
  PRIMARY KEY  (`ID`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- Table structure for table `Ratings`
--

DROP TABLE IF EXISTS `Ratings`;
CREATE TABLE `Ratings` (
  `ID` bigint(20) unsigned NOT NULL default '0',
  `SetID` bigint(20) unsigned  default '0',
  `Rating` int(11) default NULL,
  `IP` varchar(96) default NULL,
  `RateTime` int(11) default NULL,
  PRIMARY KEY  (`ID`)
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

--
-- Table structure for table `Sales`
--

DROP TABLE IF EXISTS `Sales`;
CREATE TABLE `Sales` (
  `id` int(11) NOT NULL auto_increment,
  `txn_id` varchar(127) default NULL,
  `item_name` varchar(127) default 'UNK',
  `item_number` varchar(127)  default '',
  `quantity` int(11) default '0',
  `weight` decimal(5,2) default '0.00',
  `weight_unit` varchar(3) default 'lbs',
  `mc_gross` decimal(9,2) default NULL,
  `mc_handling` decimal(9,2) default NULL,
  `mc_shipping` decimal(9,2) default NULL,
  `mc_fee` decimal(9,2) default NULL,
  `option_name1` varchar(60) default NULL,
  `option_name2` varchar(60) default NULL,
  `option_selection1` varchar(200) default 'UNK',
  `option_selection2` varchar(200) default 'UNK',
  `item_desc` text,
  `item_spec` text,
  `order_time` datetime default NULL,
  `secret` varchar(50) default NULL,
  `shipping_method` varchar(50) default NULL,
  `matchprintcost` float default NULL,
  `payment_date` datetime default NULL,
  PRIMARY KEY  (`id`),
  KEY `item_number` (`item_number`),
  KEY `txn_id` (`txn_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- Table structure for table `Sets`
--

DROP TABLE IF EXISTS `Sets`;
CREATE TABLE `Sets` (
  `ID` bigint(20) unsigned NOT NULL auto_increment,
  `Description` varchar(255) default NULL,
  `Active` set('yes','no') default 'yes',
  `Featured` set('yes','no') default 'no',
  PRIMARY KEY  (`ID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- Table structure for table `Stories`
--

DROP TABLE IF EXISTS `Stories`;
CREATE TABLE `Stories` (
  `ID` bigint(20) unsigned NOT NULL auto_increment,
  `ArtistID` bigint(20) unsigned  default '0',
  `ProjectID` bigint(20) unsigned  default '0',
  `Title` varchar(255) default NULL,
  `Filename` varchar(64) default NULL,
  `Story` text,
  PRIMARY KEY  (`ID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- Table structure for table `Suppliers`
--

DROP TABLE IF EXISTS `Suppliers`;
CREATE TABLE `Suppliers` (
  `ID` bigint(20) NOT NULL auto_increment,
  `ArtistID` bigint(20) default NULL,
  `Name` varchar(64) default NULL,
  `Firstname` varchar(127) default NULL,
  `Middlename` varchar(50) default NULL,
  `Lastname` varchar(50) default NULL,
  `Email` varchar(127) default NULL,
  `PayPalBusiness` varchar(64) default NULL,
  `Contact` varchar(127) default NULL,
  `EmailDelivery` varchar(127) default NULL,
  `ftp_server` varchar(127) default NULL,
  `ftp_user_name` varchar(50) default NULL,
  `ftp_user_pass` varchar(50) default NULL,
  `ftp_directory` varchar(127) default NULL,
  `URL` varchar(127) default NULL,
  `OrdersURL` varchar(127) default NULL,
  `Papers` text,
  `Inksets` text,
  `Frames` text,
  `Mattes` text,
  `MatteColors` text,
  `Glazing` text,
  `PaperCodes` text,
  `InksetCodes` text,
  `FrameCodes` text,
  `MatteCodes` text,
  `GlazingCodes` text,
  `Tel1` varchar(32) default NULL,
  `Tel2` varchar(32) default NULL,
  `Fax` varchar(32) default NULL,
  `Website` varchar(127) default NULL,
  `Address1` varchar(64) default NULL,
  `Address2` varchar(64) default NULL,
  `City` varchar(50) default NULL,
  `State` varchar(50) default NULL,
  `Zip` varchar(18) default NULL,
  `Country` varchar(50) default NULL,
  `Description` text,
  UNIQUE KEY `ID` (`ID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- Table structure for table `Topics`
--

DROP TABLE IF EXISTS `Topics`;
CREATE TABLE `Topics` (
  `ID` bigint(20) unsigned NOT NULL auto_increment,
  `Description` varchar(255) default NULL,
  `City` varchar(64) default NULL,
  `Country` varchar(64) default NULL,
  `Views` int(11) default '0',
  `Rating` float default '0',
  PRIMARY KEY  (`ID`)
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;

