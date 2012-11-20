/*
 * jQuery fpcart
 *
 * shopping cart
 *
 * Copyright (C) 2010 David Gross (dgross@mimetic.com)
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */
 
 /*
//----------------------------------------------------------------------
//	FPCart (settings)
//----------------------------------------------------------------------
		
		
Example:
	
	
	HTML:
	<!-- CART FIELDS -->
	<input name="itemID" type="text">
	<input name="itemCatID" type="text">
	<input name="itemPrice" type="text">
	<input name="itemName" type="text">
	<input name="itemQty" type="text">
	<input name="itemEdType" type="text">
	<input name="itemAvail" type="text">
	<input name="itemDesc" type="text">
	<input name="itemOptions" type="text">
	<input name="itemFinishing" type="text">

	<!-- Hidden FPCart vars  for this item -->
		<input name="edition_type" type="hidden" value="{EDITION_TYPE}">
		<input name="prints_available" type="hidden" value="{AVAILABLE}">
		<input name="cart_unique_id" id="cart_unique_id" type="hidden">
		<input name="cart_finishing_url" id="cart_finishing_url" type="hidden">

	<!-- SHOPPING CART SHOWS HERE -->
	<div id="fpcart"></div>
	<!-- end shopping cart -->
	
	
	JAVASCRIPT:
	
	// SET UP AND ACTIVATE SHOPPING CART
	if ( $.fn.fpcart ) {
		var options = {
			finishing_name : 'Frameshop'		// Name of the link to the finishing page, e.g. Frame Shop
		}
		$('form.fpcart').fpcart(options);
		$.updateCartOnPage();

	}
	
//----------------------------------------------------------------------
Parameters:

//----------------------------------------------------------------------
Returns:
	The wrapped set.


 */
 
(function ($) {
	var mycart = {};
	var opts = {};
	var fieldnames = {};
	
	////////////////////////////////////////////////////////////////////////////
	//
	// $.fn.fpcart
	// fpcart definition
	// Send it a list of FORM elements.
	//
	////////////////////////////////////////////////////////////////////////////
	$.fn.fpcart = function (options) {
	
		// set context vars
		//----------------------------------------------------------------------
		var obj = $(this);
		
		// build main options before element iteration
		// The defaults are defined below.
		//----------------------------------------------------------------------
		opts = $.extend(
			defaults, 
			options||{}
		);
		
		// build main options before element iteration
		// The defaults are defined below.
		//----------------------------------------------------------------------
		fieldnames = $.extend(
			defaultfieldnames, 
			fieldnames||{}
		);
		
		////////////////////////////////////////////////////////////////////////
		//
		// -> start
		// apply fpcart to all calling instances
		// There can only be one one item to sell per form.
		//
		////////////////////////////////////////////////////////////////////////
		return this.each(function(){

			var obj = $(this);

			////////////////////////////////////////////////////////////////////
			//
			// initial setup
			//
			////////////////////////////////////////////////////////////////////
			
			debug ('--------- BEGIN "FPCART PLUGIN ---------');
			
			var item = fetch_item(obj);
			disableAddIfInCart (item.itemID, item.itemCatID)
			
			// DETERMINE IF THIS IS THE CHECKOUT PAGE BY CHECKING FOR HIDDEN INPUT VALUE
			// SENT VIA AJAX REQUEST TO fpcart.php WHICH DECIDES WHETHER TO DISPLAY THE CART CHECKOUT BUTTON OR THE PAYPAL CHECKOUT BUTTON BASED ON ITS VALUE
			// WE NORMALLY CHECK AGAINST REQUEST URI BUT AJAX UPDATE SETS VALUE TO ajax_cart.php
			var isCheckout = $('#fpcart-is-checkout').val();
		
			// IF THIS IS NOT THE CHECKOUT THE HIDDEN INPUT DOESN'T EXIST AND NO VALUE IS SET
			if (isCheckout !== 'true')
				{ isCheckout = 'false'; }
		
			// ADD TO CART
			$('button[name=add_to_cart]').click(function()
				{
				var formID = $(this).attr("value");
				var item = fetch_item($("form#"+formID));
				addToCart(this.id, item);
						
				// PREVENT DEFAULT FORM ACTION
				return false;		
			});
			
			// WHEN THE VISITOR HITS THEIR ENTER KEY
			// THE UPDATE AND EMPTY BUTTONS ARE ALREADY HIDDEN
			// BUT THE VISITOR MAY UPDATE AN ITEM QTY, THEN HIT THEIR ENTER KEY BEFORE FOCUSING ON ANOTHER ELEMENT
			// THIS MEANS WE'D HAVE TO UPDATE THE ENTIRE CART RATHER THAN JUST THE ITEM WHOSE QTY HAS CHANGED
			// PREVENT ENTER KEY FROM SUBMITTING FORM SO USER MUST CLICK CHECKOUT OR FOCUS ON ANOTHER ELEMENT WHICH TRIGGERS CHANGE FUNCTION BELOW
			$('#fpcart input').live('keydown', function(e) {
				// IF ENTER KEY
				if(event.keyCode == 13)
					{
					$(event.target).blur();
					$(event.target).change();
					event.preventDefault();
					return false;
					}
				}
			    );
			
			// JQUERY live METHOD MAKES FUNCTIONS BELOW AVAILABLE TO ELEMENTS ADDED DYNAMICALLY VIA AJAX
		
			// WHEN A REMOVE LINK IS CLICKED
			$('#fpcart .fpcart-remove').live('click', function() {
				var itemID = $(this).attr('value');
				removeFromCart (itemID);

				// PREVENT DEFAULT LINK ACTION
				return false;
			});
		
		
		
			// BUTTON TO FINISHING (FRAMESHOP)
			$('button.fpcart-finishing-link').live('click', function() {
				var url = $(this).attr('value');
				document.location = url;
			});
			
			/*
			// Show item previews on mouseover
			$('.fpcart-preview-trigger').live('mouseover', function() {
				var t = $(this).attr('name');
				$('#'+t).slideDown();
			}).live('mouseout', function() {
				var t = $(this).attr('name');
				$('#'+t).slideUp();
			});
			*/

			// CLEAR
			$('button[name=fpcart_clear]').live('click', function()
				{
				clearCart ();
				// PREVENT DEFAULT FORM ACTION
				return false;		
			});
		
		
			// ----------------------------------------------------------------------
			// Modify Cart:
			//
			// WHEN AN ITEM QTY CHANGES
			$('#fpcart input.fpcart-item-qty').live('change', function(){
				// GET ITEM ID FROM THE ITEM QTY INPUT ID VALUE, FORMATTED AS fpcart-item-id-n
				var itemID = $(this).attr('id');
				itemID = itemID.split('-');

				// THE ID OF THE ITEM TO UPDATE
				itemID = itemID[3];

				// GET THE NEW QTY
				var itemQty = $(this).val();
				
				debug ("Update quantity (Qty= "+itemQty+", ID="+itemID+")")

				// AS LONG AS THE VISITOR HAS ENTERED A QTY
				if (itemQty !== '')
				{
					// SEND ITEM INFO VIA POST TO INTERMEDIATE SCRIPT WHICH CALLS fpcart.php AND RETURNS UPDATED CART HTML
					var data = {
						itemID 		: itemID,
						itemQty		: itemQty,
						defaults 	: defaults
					};
					var cmd = "update";
					cartAjax (cmd, data, false);
				}
		
			});

			// ----------------------------------------------------------------------
			// Shipping:
			$('button#'+fieldnames.shipping_button_id).live('click', function() {
				if ($('shipping').val() || $('#pickup:checked').val() == 1) {
					$('button[name=fpcart_checkout]').fadeIn();
				} else {
					$('button[name=fpcart_checkout]').fadeOut();
				}
			});

			
			// ----------------------------------------------------------------------
			// Checkout:
			$('button[name=fpcart_checkout]').live('click', function()
				{
				checkout ();				
				return false;		
			});
			
			
			// ----------------------------------------------------------------------
			// Discount button:
			$('#discount').live("change", function()
				{
				var params = {
					discount 		: $('#'+fieldnames.cart_discount).val(),
					discount_description 	: $('#'+fieldnames.cart_discount_desc).val(),
					couponCode 		: $('#'+fieldnames.cart_couponcode).val(),
					itemCatID		: $(obj).find('*[name='+fieldnames.item_cat_id_name+']').val()
					};
				cartAjax ("discount", params, false);
				}
			);
				
			// Draw the cart on the page
			$.UpdateCartParams();
			updateCartOnPage();
			
		});
					
// ===== END MAIN ================================================================================
		
		
		// --------  UTILITY FUNCTIONS
		
		// Check if an ID is already in the cart.
		// If so and the item is not an unlimited edition,
		// disable the Add item button that goes with it.
		function disableAddIfInCart (itemID, itemCatID) {
			var data = {
				'cmd'			: "available",
				'data'			: {itemID : itemID},
				'defaults'		: defaults
			};
			var dataString = JSON.stringify(data);
			$.post('ajax_fpcart.php',  {data: dataString}, function(res) {
					var res = JSON.parse(res);
					if (res.available) {
						$.EnableAddButton(itemCatID);
					} else {
						$.DisableAddButton(itemCatID);
					}
				}, "text");
		}
		
				
		// REMOVE
		function removeFromCart (itemID)
			{
			$('#' + fieldnames.stage).fadeTo(100,0.5);
			var data = {
				'cmd': "remove",
				'data': {itemID: itemID},
				'defaults' : defaults 
			};
			var dataString = JSON.stringify(data);
			$.post('ajax_fpcart.php',  {data: dataString}, function(res) {
					var res = JSON.parse(res);
					if (res.msg)
						alert (res.msg);
					$.SetAddButton(res.item.itemCatID, 1);
					var cartHTML = res.output;
					$('#' + fieldnames.stage).html(cartHTML).fadeTo(100,1.0).addActionsToCartItems();
				}, "text");
			
			debug ("removeFromCart ("+itemID+")")
			// PREVENT DEFAULT FORM ACTION
			return false;
			}

		// CHECKOUT AJAX
		function checkout ()
			{
			debug ("checkout");
			
			// we need to run: VerifyPurchase to check shipping is set, etc. ???
			// Or, we don't turn on checkout until everything is set? (better)
			
			//var go = confirm("Are you ready to buy the items in your cart?");
			go = 1;
			if (go)
			{
				$('#' + fieldnames.stage).fadeTo(100,0.5);
				var cart_params = $.GetCartParams();
				// Package data
				var data = {
					'cmd'			: 'checkout',
					'data'			: cart_params,
					'defaults'		: defaults
				};
				var dataString = JSON.stringify(data);
			
				$.post('ajax_fpcart.php',  {data: dataString}, function(res) {
						var res = JSON.parse(res);
						
						if (res.error) {
							alert (res.error);
							var cartHTML = res.output;
							$('#' + fieldnames.stage).html(cartHTML).fadeTo(100,1.0).addActionsToCartItems();
						} else {
						
							if (res.msg)
								alert (res.msg);
						
							//var cartHTML = "CHECKOUT"+res.output;
							
							//if (res.ppvars_encoded)
							//	alert (res.ppvars_encoded);
								
							// Open Paypal
							// var URL = "send_order.php?"+res.ppvars_encoded
	
	
							// alert (res.buy_url);
							
							window.location.href = res.buy_url;
							
							// This is blocked by popup-blockers
							//window.open (res.buy_url);
							
							// SHOULD CLEAR CART AT THIS POINT! clearCart(all items, no message)
							clearCart (null, true);
							
							$('#' + fieldnames.stage).fadeTo(100,1.0);
						}
						
						return false;
						
					}, "text");

			}
			// PREVENT DEFAULT FORM ACTION
			return false;
			}
		

		// WHEN AN ADD-TO-CART FORM IS SUBMITTED
		// Dim and disable the button with ID = add_to_cart_X, where X is the itemID number
		function addToCart (myID, item)
			{
			debug ("addToCart "+item.itemID);
			
			// If dimmed, do nothing.
			if ($('#'+myID).hasClass("fp_dim")) {
				debug ("addToCart "+item.itemID+": BUTTON IS DIMMED");
				return false;
			}
			
			$('#' + fieldnames.stage).fadeTo(100,0.5);
			
			// Package data
			var data = {
				'cmd'			: 'add',
				'data'			: item,
				'defaults'		: defaults
			};
			var dataString = JSON.stringify(data);
			$.post('ajax_fpcart.php',  {data: dataString}, function(res) {
					var res = JSON.parse(res);
					var cartHTML = res.output;
					if (res.msg)
						alert (res.msg);
						
					$.SetAddButton(res.item.itemCatID, res.addButtonState);
					
					$('#' + fieldnames.stage).html(cartHTML).fadeTo(100,1.0).addActionsToCartItems();
				}, "text");

			
			// PREVENT DEFAULT FORM ACTION
			return false;
			}


		function clearCart (itemID, hidemsg) {
			$.clearCart(itemID, hidemsg);
		}
		
		
		// Fetch item values from page object (from form fields)
		function fetch_item(obj)
			{
			// GET INPUT VALUES FOR USE IN AJAX POST
			var itemID = $(obj).find('*[name='+fieldnames.item_id_name+']').val();
			var itemCatID = $(obj).find('*[name='+fieldnames.item_cat_id_name+']').val();
			var itemPrice = $(obj).find('*[name='+fieldnames.item_price_name+']').val();
			var itemName = $(obj).find('*[name='+fieldnames.item_name_name+']').val();
			var itemQty = $(obj).find('*[name='+fieldnames.item_qty_name+']').val();
			var itemEdType = $(obj).find('*[name='+fieldnames.item_edition_type_name+']').val();
			var itemAvailable = $(obj).find('*[name='+fieldnames.item_available_name+']').val();
			var itemDesc = $(obj).find('*[name='+fieldnames.item_desc_name+']').val();
			var itemShortDesc = $(obj).find('*[name='+fieldnames.item_short_desc_name+']').val();
			var itemOptions = $(obj).find('*[name='+fieldnames.item_options_name+']').val();
			var itemFinishing = $(obj).find('*[name='+fieldnames.item_finishing_name+']').val();
			var itemPreviewURL = $(obj).find('*[name='+fieldnames.item_preview_url_name+']').val();
			
			var itemProjectID = $(obj).find('*[name='+fieldnames.item_project_id_name+']').val();
			
			// choices from the options listings: size, frame, matte, glazing, ink
			var itemProjectID = $(obj).find('*[name='+fieldnames.item_project_id_name+']').val();
			
			var itemSizeIndex = $(obj).find('*[name='+fieldnames.item_size_index_name+']').val();
			var itemFrameIndex = $(obj).find('*[name='+fieldnames.item_frame_index_name+']').val();
			var itemMatteIndex = $(obj).find('*[name='+fieldnames.item_matte_index_name+']').val();
			var itemGlazingIndex = $(obj).find('*[name='+fieldnames.item_glazing_index_name+']').val();
			var itemPaperIndex = $(obj).find('*[name='+fieldnames.item_paper_index_name+']').val();

			
			// can this item be grouped for shipping, or must it be sent by itself?
			var itemShippingGroup = $(obj).find('*[name='+fieldnames.item_shipping_group_name+']').val();

			// shipping vars to calc shipping rate
			var shipping_shippingValue = $('#'+fieldnames.shipping_shippingValue_id).val();
			var shipping_packageWeight = $('#'+fieldnames.shipping_packageWeight_id).val();
			var shipping_shippingHeight = $('#'+fieldnames.shipping_shippingHeight_id).val();
			var shipping_shippingLength = $('#'+fieldnames.shipping_shippingLength_id).val();
			var shipping_shippingWidth = $('#'+fieldnames.shipping_shippingWidth_id).val();
			var shipping_handling_local = $('#'+fieldnames.shipping_handling_local_id).val();
			var shipping_handling_intl = $('#'+fieldnames.shipping_handling_intl_id).val();

			var item = 
				{
				itemID		: itemID,
				itemCatID	: itemCatID,
				itemPrice	: itemPrice,
				itemName	: itemName,
				itemQty		: itemQty,
				itemEdType	: itemEdType,
				itemAvail	: itemAvailable,
				itemDesc	: itemDesc,
				itemShortDesc	: itemShortDesc,
				itemOptions	: itemOptions,
				itemFinishing	: itemFinishing,
				itemPreviewURL	: itemPreviewURL,
				
				itemProjectID 	: itemProjectID,
				itemSizeIndex 	: itemSizeIndex,
				itemFrameIndex 	: itemFrameIndex,
				itemMatteIndex 	: itemMatteIndex,
				itemGlazingIndex : itemGlazingIndex,
				itemPaperIndex 	: itemPaperIndex,
				
				itemShippingGroup	: itemShippingGroup,

				shipping_shippingValue 	: shipping_shippingValue,
				shipping_packageWeight	: shipping_packageWeight,
				shipping_shippingHeight	: shipping_shippingHeight,
				shipping_shippingLength : shipping_shippingLength,
				shipping_shippingWidth	: shipping_shippingWidth,
				handling_local		: shipping_handling_local,
				handling_intl		: shipping_handling_intl,
				};
			return item;
			}
		
		// AJAX: send command to PHP scripts
		// Automatically update cart with the result.
		function cartAjax (cmd, data, updatecart) {
			$.cartAjax (cmd, data, updatecart);
		}
		

		// Debugging console output
		//----------------------------------------------------------------------
		function debug(msg) {
			if (window.console && window.console.log && opts.debug) {
				window.console.log(msg);
			}
		}
		
		// This allows us to call the update cart form inside the closure.
		function updateCartOnPage()
		{
			$.updateCartOnPage();		
		}
		

	};	// end $.fn.fpcart = function...
	
	
	// PUBLIC FUNCTIONS


		// WHEN AN CLEAR-CART IS SUBMITTED
		$.clearCart = function (itemID, hidemsg) 
			{
			if (!hidemsg && !confirm('Remove all items from the shopping cart?'))
				return false;

			var data = {
				'itemID':		itemID,
				'defaults' :		defaults
			};
			
			$('#'+fieldnames.shipping_destPostal_id).val('');
			$('#'+fieldnames.shipping_pickup_id).val('').attr('checked', false).change();
			
			// The cart has been 100% cleared, so we must be sure to
			// set up all the basic cart params again.
			var callback = $.UpdateCartParams
			$.cartAjax ("clear", data, false, callback);
			
			// activate all Add buttons
			$("*[id^=add_to_cart]").each(function()
				{
				UnDimButton(this.id);
				}
			);

			debug ("clear cart");
			// PREVENT DEFAULT FORM ACTION
			return false;			
			}
			
		
	$.fpdebug = function (msg) {
		if (window.console && window.console.log && opts.debug) {
			window.console.log(msg);
		}
	}

	
	// Update current page shipping HTML fields from the PHP cart
	// Be careful not to overwrite existing values with empty strings (was happening before)
	// when the AJAX doesn't respond fast enough, or something like that.
	$.UpdatePageShippingFieldsFromCart = function (shipping_params)
	{
		if (shipping_params) {
			shipping_params.product && $('#'+fieldnames.shipping_product_id).val(shipping_params.product);
			shipping_params.origCountry && $('#'+fieldnames.shipping_origCountry_id).val(shipping_params.origCountry);
			shipping_params.origPostal && $('#'+fieldnames.shipping_origPostal_id).val(shipping_params.origPostal);
			shipping_params.destPostal && $('#'+fieldnames.shipping_destPostal_id).val(shipping_params.destPostal);
			shipping_params.destCountry && $('#'+fieldnames.shipping_destCountry_id).val(shipping_params.destCountry);
			shipping_params.currency && $('#'+fieldnames.shipping_currency_id).val(shipping_params.currency);
			//$('#'+fieldnames.shipping_customValue_id).val(shipping_params.customValue);
			shipping_params.pickup && $('#'+fieldnames.shipping_pickup_id).val(shipping_params.pickup);
			shipping_params.rateCode && $('#'+fieldnames.shipping_rateCode_id).val(shipping_params.rateCode);
			shipping_params.rescom && $('#'+fieldnames.shipping_rescom_id).val(shipping_params.rescom);
			shipping_params.shippingContainerCode && $('#'+fieldnames.shipping_shippingContainerCode_id).val(shipping_params.shippingContainerCode);
			shipping_params.shippingName && $('#'+fieldnames.shipping_shippingName_id).val(shipping_params.shippingName);
			shipping_params.state && $('#'+fieldnames.shipping_state_id).val(shipping_params.state);
			shipping_params.weight_std && $('#'+fieldnames.shipping_weight_std_id).val(shipping_params.weight_std);
		}
	}
	
	
	// Update Cart object shipping params
	$.UpdateCartShippingParams = function ()
	{
		var shipping_product = $('#'+fieldnames.shipping_product_id).val();
		var shipping_origCountry = $('#'+fieldnames.shipping_origCountry_id).val();
		var shipping_origPostal = $('#'+fieldnames.shipping_origPostal_id).val();
		var shipping_destPostal = $('#'+fieldnames.shipping_destPostal_id).val();
		var shipping_destCountry = $('#'+fieldnames.shipping_destCountry_id).val();
		var shipping_currency = $('#'+fieldnames.shipping_currency_id).val();
		var shipping_customValue = $('#'+fieldnames.shipping_customValue_id).val();
		var shipping_pickup = $('#'+fieldnames.shipping_pickup_id).val();
		var shipping_rateCode = $('#'+fieldnames.shipping_rateCode_id).val();
		var shipping_rescom = $('#'+fieldnames.shipping_rescom_id).val();
		var shipping_shippingContainerCode = $('#'+fieldnames.shipping_shippingContainerCode_id).val();
		var shipping_shippingName = $('#'+fieldnames.shipping_shippingName_id).val();
		var shipping_state = $('#'+fieldnames.shipping_state_id).val();
		var shipping_weight_std = $('#'+fieldnames.shipping_weight_std_id).val();
		var shipping_source_state = $('#'+fieldnames.shipping_source_state_id).val();
		
		var shipping_params =
			{
			product			: shipping_product,
			origCountry		: shipping_origCountry,
			origPostal		: shipping_origPostal,
			destPostal		: shipping_destPostal,
			destCountry		: shipping_destCountry,
			currency		: shipping_currency,
			customValue		: shipping_customValue,
			pickup			: shipping_pickup,
			rateCode		: shipping_rateCode,
			rescom			: shipping_rescom,
			shippingContainerCode	: shipping_shippingContainerCode,
			shippingName		: shipping_shippingName,
			state			: shipping_state,
			source_state		: shipping_source_state,
			weight_std		: shipping_weight_std
			}
			
		$.cartAjax ("update_cart_shipping_params", shipping_params, true);
		$.fpdebug ('Update cart shipping params');
	}
			
		
	// Update Cart object general params
	$.UpdateCartParams = function ()
	{
		var cart_params = $.GetCartParams();
		$.cartAjax ("update_cart_general_params", cart_params, false);
		$.fpdebug ('Update cart general params');
	}
			
		

	// Update Cart object general params
	$.GetCartParams = function ()
	{
		// Paypal variables
		var cart_payment_id = $('#'+fieldnames.cart_payment_id_id).val();
		var cart_add = $('#'+fieldnames.cart_add_id).val();
		var cart_return = $('#'+fieldnames.cart_return_id).val();
		var cart_shopping_url = $('#'+fieldnames.cart_shopping_url_id).val();
		var cart_rm = $('#'+fieldnames.cart_rm_id).val();
		var cart_page_style = $('#'+fieldnames.cart_page_style_id).val();
		var cart_cancel_return = $('#'+fieldnames.cart_cancel_return_id).val();
		var cart_currency_code = $('#'+fieldnames.cart_currency_code_id).val();
		var cart_weight = $('#'+fieldnames.cart_weight_id).val();
		var cart_weight_unit = $('#'+fieldnames.cart_weight_unit_id).val();
		var cart_lc = $('#'+fieldnames.cart_lc_id).val();
		var cart_bn = $('#'+fieldnames.cart_bn_id).val();
		var cart_notify_url = $('#'+fieldnames.cart_notify_url_id).val();
		var cart_invoice = $('#'+fieldnames.cart_invoice_id).val();
		var cart_custom = $('#'+fieldnames.cart_custom_id).val();

		// COUPON
		var cart_coupon_code = $('#'+fieldnames.item_coupon_code_id).val();
		
		// SUPPLIER
		var cart_supplier_id = $('#'+fieldnames.cart_supplier_id_id).val();
		var cart_supplier_tax_rate = $('#'+fieldnames.cart_supplier_tax_rate_id).val();
			
		
		var cart_params =
			{
			cart_payment_id : 	cart_payment_id,
			cart_add : 		cart_add,
			cart_return : 		cart_return,
			cart_shopping_url : 	cart_shopping_url,
			cart_rm : 		cart_rm,
			cart_page_style : 	cart_page_style,
			cart_cancel_return : 	cart_cancel_return,
			cart_currency_code : 	cart_currency_code,
			cart_weight : 		cart_weight,
			cart_weight_unit : 	cart_weight_unit,
			cart_lc : 		cart_lc,
			cart_bn : 		cart_bn,
			cart_notify_url : 	cart_notify_url,
			cart_invoice : 		cart_invoice,
			cart_custom : 		cart_custom,
			cart_coupon_code :	cart_coupon_code,
			cart_supplier_id :	cart_supplier_id,
			cart_supplier_tax_rate :	cart_supplier_tax_rate
			}
		mycart.cart_params = cart_params;
		return cart_params;
	}
			
		

	
	// Set the quantity popup menu to amounts between 1 and the number of available prints
	$.SetQuantityMenu = function(maxItems, fieldID, fieldName)
	{
		// 
	}
	
	
	$.SetAddButton = function (itemCatID, enableButton) {
		if (enableButton) {
			$.EnableAddButton(itemCatID);
		} else {
			$.DisableAddButton(itemCatID);
		}
		//disableAddIfInCart (item.itemID, item.itemCatID);
	}
		

	$.EnableAddButton = function (itemID)
		{
		var myID = "add_to_cart_"+itemID;
		UnDimButton(myID);
		/*
		$('#'+myID).click(function()
			{
			var formID = $(this).attr("value");
			var item = fetch_item($("form#"+formID));
			addToCart(item);
			});
		*/
		}

		
	$.DisableAddButton = function (itemID)
		{
		var myID = "add_to_cart_"+itemID;
		DimButton(myID);
		//$('#'+myID).unbind("click");
		}
		

	/*
	NOT NECESSARY: DO IN PHP
	UpdateCartButtons ()
	Set cart buttons depending on status of cart. This means,
	show/hide/dim buttons. If no shipping set, dim the checkout button
	for example.
	*/
	$.updateCartButtons = function()
	{
		// Disable Checkout
		if($('#fpcart_cart_empty_status').val() == '')
		{
			// DISABLE THE PAYPAL CHECKOUT BUTTON
			$('#fpcart-paypal-checkout').attr('disabled', 'disabled');
		}
		
	
	
	}



	// This way we can call the update cart function from elsewhere.
	$.updateCartOnPage = function()
		{
		$('#' + fieldnames.stage).fadeTo(100,0.5);
		var params = opts;
		
		// Get the compact cart flag
		params.cartCompact = $('#'+fieldnames.cart_compact_id).val();
		params.discount = $('#'+fieldnames.cart_discount).val();

		var data = {
			'cmd'			: "build",
			'data'			: params,
			'defaults'		: defaults
		};
		var dataString = JSON.stringify(data);
		$.post('ajax_fpcart.php',  {data: dataString}, function(res) {
				var res = JSON.parse(res);
				var cartHTML = res.output;
				if (res.shipping_params)
					$.UpdatePageShippingFieldsFromCart(res.shipping_params);
				$('#' + fieldnames.stage).html(cartHTML).fadeTo(100,1.0).addActionsToCartItems();
			}, "text");
		}
		
	// AJAX: send command to PHP scripts
	// Automatically update cart with the result.
	$.cartAjax = function (cmd, data, updatecart, callback) {
		$('#' + fieldnames.stage).fadeTo(100,0.5);
		var data = {
			'cmd'			: cmd,
			'data'			: data,
			'defaults'		: defaults
		};
		var dataString = JSON.stringify(data);
		$.post('ajax_fpcart.php',  {data: dataString}, function(res) {
			//alert (res);
			var res = JSON.parse(res);
			var cartHTML = res.output;
			if (res.error)
				alert (res.error);
			if (res.msg)
				alert (res.msg);
			//alert ("addButtonState: "+res.addButtonState);
			if (res.addButtonState != undefined )
				$.SetAddButton(res.item.itemCatID, res.addButtonState);
				
			// Perform any follow up commands, e.g clear cart
			if (res.command) {
				switch (res.command) {
					case "clear_cart" :
						$.clearCart (null, true);	// no message
						break;
				}
			}
			
			if (callback) {
				callback()
			}
			
			// Update the cart on screen
			if (updatecart) {
				$.updateCartOnPage();
			}
			$('#' + fieldnames.stage).html(cartHTML).fadeTo(100,1.0).addActionsToCartItems();
		}, "text");
	}


	// jQuery "live" can't do everything, so let's add some actions to the cart items.
	// 1) show/hide preview pictures
	// NOTE: this $.fn because we chain it!!!!
	$.fn.addActionsToCartItems = function()
	{
		// Show item previews on mouseover
		$('.fpcart-preview-trigger').hover(function() {
			var t = $(this).attr('name');
			$('.'+t).slideDown();
		}, function() {
			var t = $(this).attr('name');
			$('.'+t).slideUp();
		});
		// give roll-over, etc., to cart buttons
		$('.fpcart-cart .ui-button').button();

	}





	////////////////////////////////////////////////////////////////////////////
	//
	// Defaults:
	// Set default options, including names and IDs of HTML fields on page.
	// 
	//
	////////////////////////////////////////////////////////////////////////////

	var defaults = {
		remove_item_text :		'Remove',		// Remove link/button text
		//order_script :			'ajax_fporder.php',	// PHP script for sending orders to the payment system
		debug:				true			// turn on console output (slows down IE8!)
	}
	
	var defaultfieldnames = {
		stage :				'fpcart',		// id of the stage element...where to show the pictures
		finishing_name :		'Frameshop',		// text for the link/button to the finishing page, e.g. Frame Shop
		shipping_button_id :		'ui-edit-shipping',	// id of the edit/set shipping button
		
	// form field names (not ID's)
		item_id_name :			'cart_unique_id',			// UNIQUE code of the item to add
		item_cat_id_name :		'item_number',
		item_price_name :		'unitPrice',
		item_name_name :		'item_name',
		item_qty_name :			'quantity',
		item_edition_type_name :	'edition_type',
		item_available_name :		'prints_available',
		item_desc_name :		'os0',
		item_short_desc_name :		'cart_item_short_desc',
		item_options_name :		'spec',
		item_finishing_name :		'cart_finishing_url',
		item_preview_url_name :		'cart_item_preview',
		item_project_id_name :		'projectid',

		item_size_index_name : 		'currentsize',	
		item_frame_index_name : 	'currentframe',	
		item_matte_index_name : 	'currentmatte',	
		item_paper_index_name : 	'currentpaper',	
		item_glazing_index_name : 	'currentglazing',	

		item_size_x :	 		'currentsize',	
		
	// cart general parameters, for the payment system field ID's (e.g. Paypal cart vars)
	// These currently come from Paypal required vars.
		cart_payment_id_id :		'business',
		cart_add_id :			'add',
		cart_return_id :		'return',
		cart_shopping_url_id :		'shopping_url',
		cart_rm_id : 			'rm',
		cart_page_style_id : 		'page_style',
		cart_cancel_return_id : 	'cancel_return',
		cart_currency_code_id : 	'currency_code',
		cart_weight_id : 		'weight',
		cart_weight_unit_id : 		'weight_unit',
		cart_lc_id : 			'lc',
		cart_bn_id : 			'bn',
		cart_notify_url_id :		'notify_url',
		cart_invoice_id : 		'invoice',
		cart_custom_id :		'custom',
		cart_supplier_id_id :		'supplierid',
		cart_supplier_tax_rate_id :	'supplier_tax_rate',
		
		item_coupon_code_id :		'couponcode',

	// item shipping field names
		item_shipping_group_name :	'shipping_group',
		//item_shipping_name :		'shipping',
		//item_shipping2_name :		'shipping2',
		shipping_handling_local_id : 	'handling',
		shipping_handling_intl_id : 	'handling_intl',

	// cart shipping field ID's
		shipping_product_id		: 'shipping-shippingProduct',
		shipping_origCountry_id		: 'shipping-originCountry',
		shipping_origPostal_id		: 'shippingOriginPostalCode',
		shipping_destPostal_id		: 'shipping-destPostalCode',
		shipping_destCountry_id		: 'shipping-destCountry',
		shipping_currency_id		: 'shipping-currency',
		shipping_customValue_id		: 'customValue',
		shipping_pickup_id		: 'pickup',
		shipping_packageWeight_id	: 'shipping-packageWeight',
		shipping_rateCode_id		: 'shipping-rateCode',
		shipping_rescom_id		: 'shipping-rescom',
		shipping_shippingContainerCode_id	: 'shipping-shippingContainerCode',
		shipping_shippingHeight_id	: 'shipping-shippingHeight',
		shipping_shippingLength_id	: 'shipping-shippingLength',
		shipping_shippingName_id	: 'shipping-shippingName',
		shipping_shippingValue_id	: 'shipping-shippingValue',
		shipping_shippingWidth_id	: 'shipping-shippingWidth',
		shipping_state_id		: 'state',
		shipping_source_state_id	: 'supplierstate',
		shipping_weight_std_id		: 'shipping_weight_std',
		
	// cart params
		cart_compact_id 		: 'fpcart_cart_compact',
		cart_discount 			: 'discount',
		cart_discount_desc 		: 'coupon_description',
		cart_couponcode			: 'couponcode',
		
	};


// end of closure, bind to jQuery Object
})(jQuery); 
