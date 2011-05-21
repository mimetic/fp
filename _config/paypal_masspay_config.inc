<?php 

// For security, these values are hidden outside the document root.
// Set owner/group on "fp_private" to owner=user, grp=nobody
// Set owner/group on "paypal_masspay_config.inc" to owner=user, grp=nobody
// Permissions for fp_private  : 640
//		drw-rw----   2 frontlin nobody   fp_private
// Permissions for paypal_masspay_config.inc : 640
// 		-rw-r-----   1 frontlin nobody   paypal_masspay_config.inc

require_once FP_HOME_REALPATH."/fp_private/paypal_masspay_config.inc";

?>