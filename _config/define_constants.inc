<?php
/*
FP: define constants
March 31, 2009
Define constants based on global variables defined in config files loaded before this one.
*/

if ($DEVELOPING) {
	define ("FP_IM_CONVERT", "/opt/local/bin/convert");
} else {
	define ("FP_IM_CONVERT", "/usr/local/bin/convert");
}
define ("FP_MAXCONFIRMTIME", $MAXCONFIRMTIME);
FP_MAXCONFIRMTIME < (47 * 3600) 
	? define('FP_SHOW_MAXCONFIRMTIME', floor(FP_MAXCONFIRMTIME / 3600) . " {fp:hours}")
	: define('FP_SHOW_MAXCONFIRMTIME', floor(FP_MAXCONFIRMTIME / (24 * 3600)) . " {fp:days}");
define("MAX_PROJECT_PICTURES", $MAXMAXPIX);
define("MAX_DAYS_FEATURED", 0);
define("MAX_DAYS_ACTIVE", 0);
define("SHOW_PUBLICPRIVATE_FOR_GROUPS", false);
define("SHOW_PUBLICPRIVATE_FOR_PROJECTS", true);
define ("FP_USERS_GET_GALLERY", true);
define("FP_DEFAULT_GALLERY_NAME", $default_gallery_name);
define("MAX_GROUP_SIZE", 15);
define ("OPTION_LIST_MAXSIZE", 10);	
define ("FP_IMAGE_TOOL", "im");
define ("FP_IMAGEMAGICK_QUALITY", 85);
define ("MATTESCALE", $mattescale);
define ("MATTEBOTTOM", $mattebottom);
define ("FP_GALLERY_ICON_WIDTH", 45);
define ("FP_GALLERY_ICON_HEIGHT", 30);
define ("FP_GALLERY_LOGO_WIDTH", 1000);
define ("FP_GALLERY_LOGO_HEIGHT", 1000);
define ("FP_GALLERY_ICON_TITLE_WIDTH", 50);
define ("FP_GALLERY_ICON_TITLE_HEIGHT", 30);
define ("FP_SHOW_BLANK_GALLERY_ICON", false);
define ("FP_SHOW_GALLERY_ICON", true);
define ("FP_CAPTION_SHOW_LINEBREAKS", true);
define ("GALLERYFADEDARKNESS", 0);
define ("FP_SLIDESHOW_PC", true);
define ("FP_SLIDESHOW_PC_COLUMNS", 5);
define ("FP_SLIDESHOW_TV", false);
define ("FP_SLIDESHOW_SV", true);
define ("FP_WATERMARK_OPACITY", 90);
define ("FP_WATERMARK_FONT_SIZE", 12.0);
define ("FP_WATERMARK_FONT_DIR", "fonts");
define ("FP_WATERMARK_FONT_NAME", "Vera.ttf");
define ("FP_WATERMARK_GALLERY_PIX", false);
define ("FP_SLIDE_WIDTH", $slide_size[0]);
define ("FP_SLIDE_HEIGHT", $slide_size[1]);
define ("FP_SLIDE_ADJUSTMENT", $FP_SLIDE_ADJUSTMENT);
define ("FP_DEFAULT_WIDTH", $default_size[0]);
define ("FP_DEFAULT_HEIGHT", $default_size[1]);
define ("FRAMED_MATTED_PIX", "true");
define ("FP_ARTIST_DEFAULT_COMMISSION", "20") ;	
define ("FP_ARTIST_DEFAULT_USERLEVEL", FP_SINGLE_GALLERY_SINGLE_USER) ;	
define ("FP_ARTIST_DEFAULT_ACCESSLEVEL", FP_PERSONAL_LEVEL) ;	
define ("FP_ARTIST_DEFAULT_STORAGE", 0) ;
define ('FP_DEFAULT_SLIDESHOW_PAUSE', $FP_DEFAULT_SLIDESHOW_PAUSE);
define ('FP_SSP_TRANSITION_LENGTH', $FP_SSP_TRANSITION_LENGTH);
define ("FRAMESHOP_PICTURE_SCALING", 0);
define ("FP_EDITION_CALC_METHOD", "total");
define ("CURRENCY", '<span>$</span>');	
define ("FP_CURRENCY_NAME", "USD");
define ("FP_DECIMAL", ".");		
define ("FP_THOUSANDS", ",");	
define ("CURRENCY_POSITION", "before");	
define ("CURRENCY_AFTER", "USD"); 
define ("UNIT", "inch");
define ("UNITS", "inches");
define ("FP_LABEL_INCHES", "in");
define ("FP_LABEL_CM", "cm");
define ("WEIGHT_UNIT", "lbs");
define ("FP_CUSTOMER_CHOOSES_PAPER", false);
define ("FP_CUSTOMER_CHOOSES_INKSET", false);
define ("FP_SHOW_CART_NAME", "showcart_paypal");
define ("FP_NAVBAR_SHOW_CART_NAME", "navbar_showcart_paypal");
define ("FP_EMAIL_HEADERS", $em_headers);
define ("PAYPAL_CURRENCY_CODE", "USD");
define ("FRAMESHOP_TITLE", "{FP:The Print Shop}");
define ("FP_FRAMESHOP_WALLCOLOR", "#e6e6e6");
define ("MAX_PRINTS_TO_SELL", 1);
define ("FRAMESHOP_REF_SIZE", 20);
define ("MATTE_BORDER_WIDTH", (1 - MATTESCALE));
define ("FRAMESHOP_ART_BORDER", 20);
define ("FRAMESHOP_ART_BORDER_COLOR", "#FDFDFD");
define ("FP_NEWSLETTER_PERIOD", 14);
define ('FP_MAILER_AUTHOR', $SYSTEMNAME);
define ('FP_MAILER_SUBJECT', "$SYSTEMNAME Newsletter of ". date("F j, Y"));
define ("FP_MAILER_LIST", $FP_MAILER_LIST);
define ("FP_INSTALLATION_SHOW_HIDE", $FP_INSTALLATION_SHOW_HIDE);
define ("FP_NAVBAR_FAQ", '2');
define ("FP_NAVBAR_SHOW_LOCATION", true);
define ("FP_RANDOM_SHAPE_TOLERANCE", .2);
define ("FP_COMMERCE_EMAIL_OBFUSCATED", "<span class=\"obfuscated\">".str_replace ("@", "(put the 'at' sign here)", FP_COMMERCE_EMAIL) . "</span>");
										
?>