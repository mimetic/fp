<?php

/*
 *  CART
 * SESSION BASED SHOPPING CART CLASS FOR fpcart
 *
 * **********************************************************************
 *
 * DEPENDENCIES: This class is now dependent on FP libraries
 
	
	The cart is kept as a PHP session variable. Therefore, the PHP
	must start a "cart" session before anything else.
	
	// START SESSION
	session_name("fpcart");
	session_start();
	
	// INITIALIZE JCART AFTER SESSION START
	$cart =& $_SESSION["cart"]; 
	if(!is_object($cart)) 
		$cart = new FPCart();
		
**********************************************************************
	A cart contains the following fields:
	
			id
			qty
			price
			name
			options
			subtotal
			AND MORE...


**********************************************************************
*/



// fpcart
class FPCart {

	// Public cart values
	var $itemcount = 0;
	var $items = array();
	var $item = null;

	var $discount = 0;
	var $discount_amount = 0;
	
	// Shipping calculations
	var $zip = "";

	// Cart total
	var $total = 0;

	var $cartCompact = true;
	var $cartoutput = "";
	var $errorcode = "";
	var $currency = CURRENCY;
	var $dimAddButton = false;	// flag to indicate whether to dim the Add to Cart button
	
	var $finishingURL = "frameshop.php";
	var $buy_url = "";


	// Default text
	var $stage = "cart";
	var $finishing_name = "DEFAULT Frame Shop";
	var $remove_item_text = "DEFAULT Remove;";
	var $no_add_msg = "Sorry, there are no more items available to add.";
	var $yes_add_msg = ""; //"Added new items to the shopping cart.";
	var $add_remaining_msg = "Too many items to add...we added all available items.";

	/*
	// External functions

	// Verify product available for sale. This is a last-second check that no one else bought
	// the last item while user was filling his cart.
	// FP uses: imageIsForSale ($item_number)
	var $itemForSale = "imageIsForSale";
	
	// Save the cart vars before submitting, so that the response from the payment system
	// can be verified against the stored values.
	// FP uses: SaveCart($vars)
	var $saveCart = "SaveCart";

	// 
	// RecordImageSale
	*/




	// CONSTRUCTOR FUNCTION
	function FPCart() {

	}



	// ===== CHECKOUT ======================

	/*
	 *  checkout
	 * $params : general cart params for the payment system, e.g. user ID
	 * Be sure:
	 * - Items were chosen (not empty cart)
	 * - Shipping params are set
	 * - General cart params are set
	 */

	function checkout($params) {
		
		// Items in cart?
		if (count($this->items) < 1) {
			$this->error = "No items in cart.";
			return $this->error;
		}

		$this->set_cart_general_params($params);
		
		$this->update_totals();

		// Is there anything to buy? No shipping will also keep the total to zero.
		if (!$this->total) {
			return $this->error;
		}

		// Are the items in the cart still available (last chance check)
		$notAvailList = $this->unavailableItems ();
		if (!$notAvailList) {
			// Fetch item prices, discounts from the system to prevent fraud. It is possible the prices and discount rates were hacked on the client side.
			$this->rebuild_cart_prices();

			// shit...adapt SaveCart for multiple items?
			$cart = array();
			$cart['cart_params'] = $this->cart_params;
			$cart['items'] = $this->items;
			$cart['shipping_params'] = $this->shipping_params;
			$cart['total'] = $this->total;
			$cart['shipping'] = $this->totalshipping;
			$cart['handling'] = $this->totalhandling;
			$cart['discount_rate'] = $this->discount;
			$cart['discount_amount'] = $this->discount_amount;
			$cart['discount_description'] = $this->discount_description;

			$result = SaveMultiCart ($cart);
			if ($result) {
				$secret = $result['secret'];
				$order_time = $result['$order_time'];
				$this->invoice = $result['invoice'];

				//$vars['item_number'] = $orderID;
				//$vars['order_time'] = $order_time;
				$vars['notify_url'] .= "?secret={$secret}";
				//$imageID = $vars['itemID'];

				// Add quantities sold to the price sets, to track sales.
				// Also creates unique price sets as required for each item
				RecordMultiImageSales ($this->invoice);


				$this->buy_url = "https://" . PAYPAL_POST_TO_URL . "/webscr";
				$logme = $url;

				// Set the secret
				$this->cart_params['secret'] = $secret;

				$this->set_cart_general_params($params);
				$this->get_paypal_cart_vars($params);

				fp_error_log(__FUNCTION__.":".__LINE__.": Cart checked out:\n".print_r($cart, true), 3, FP_ACTIVITY_LOG );

				$this->buy_url = "https://".PAYPAL_POST_TO_URL."/cgi-bin/webscr?".$this->ppvars_encoded;
				//$this->buy_url = "https://www.sandbox.paypal.com/webscr?".$this->ppvars_encoded;

				$this->empty_cart();

				//return $this->ppvars_encoded;
				return null;

			} else {
				return "ERROR: Database error saving the order: Try again :".__FUNCTION__.":".__LINE__;
			}
		} else {
			$this->error = "Sorry, some or all of your items are sold out.\n";
			foreach ($notAvailList as $id => $remaining) {
				$itemName = $this->items[$id]['itemName'];
				$itemID = $this->items[$id]['itemID'];
				if ($remaining > 0) {
					$item = $this->items[$itemID];
					$item['itemQty'] = $remaining;
					$this->update_item($item);
					$this->error .= "\"$itemName\" has only $remaining available for sale. I have changed your order.\n";
				} else {
					$this->error .= "\"$itemName\" is sold out. I have removed this item from your cart.\n";
					$this->del_item($itemID);
				}
			}
		}
	}


	function unavailableItems ()
	{
		$notavail = array();
		foreach ($this->items as $item)
		{
			$id = $item['itemCatID'];
			$qty = $item['itemQty'];
			$remaining = NumImagesAvailableForSale($id, $qty);
			if ($remaining < $qty)
			{
				$notavail[$item['itemID']] = $remaining;
			}
		}
		return $notavail;
	}
	
	/*
	 * function rebuild_cart_prices ()
	 * Rebuild the cart prices from the server databases to prevent fraud.
	 * Someone might hack the Javascript cart, and send in low prices.
	 * This way, we ignore any prices from the client.
	 */
	function rebuild_cart_prices () {
		foreach ($this->items as &$item)
		{
			$itemSizeIndex = $item['itemSizeIndex'] || 0;
			$itemMatteIndex = $item['itemMatteIndex'] || 0;
			$itemFrameIndex = $item['itemFrameIndex'] || 0;
			$itemGlazingIndex = $item['itemGlazingIndex'] || 0;
			$itemPaperIndex = $item['itemPaperIndex'] || 0;

			$options = array (
				"size"			=> $itemSizeIndex,
				"matte"			=> $itemMatteIndex,
				"frame"			=> $itemFrameIndex,
				"glazing"		=> $itemGlazingIndex,
				"paper"			=> $itemPaperIndex
			);

			$imageID = $item['itemCatID'];
			
			//$itemDiscountedPrice = GetImageDiscountedPrice ($imageID, $cartCouponCode, $options);
			$itemPriceCost = GetImageSalePriceAndCost($imageID, $options);
			//$itemPrice = GetImageSalePrice ($imageID, $options);

			// Update the cart price with our calculated price, to avoid possible hacking of the cart
			$item['itemPrice'] = $itemPriceCost['price'];
			$item['itemCost'] = $itemPriceCost['cost'];
		}
	}

	
	function saveOrder ()
	{
		$result = SaveCart ($vars);
		return $result;
	}
	
	function recordAllItemSales ()
	{
	}
	
	function launchPaymentWebsite ()
	{
		if (!$this->unavailableItems())
		{
			return true;
		}
	}
	
	
	/*
	==========================================
	*/
	
	private function itemIsAvailable ($ID)
	{
		$imageID = $this->items[$ID];
		return NumImagesAvailableForSale($imageID);
	}





// ===========================


	// ADD AN ITEM
	function add_item($newitem) {

		$itemID = $newitem['itemID'];
		$this->msg = "";
		
		// IF THE ITEM PRICE IS A FLOATING POINT NUMBER
		if (!is_numeric($newitem['itemPrice'])) {
			$this->error_type = 'itemPrice';
			return false;
		}

		// ADD THE ITEM
		// IF THE ITEM IS ALREADY IN THE CART, and NOT unique, INCREASE THE QTY
		/*
		 * JS index
			itemID
			itemPrice
			itemName
			itemQty
			itemEdType,
			itemAvail
			itemDesc
		*/

		$test = $this->items;

		$this->dimAddButton = false;

		if (intval($newitem['itemAvail'] > 0) || $newitem['itemEdType'] == "unlimited") {
			if (isset($this->items[$itemID])) {
				switch ($newitem['itemEdType']) {
					case "original" :
						// you can only add one original item, of course
						$this->msg .= "You have already added this item.";
						$this->dimAddButton = true;
						//$this->items[$itemID]['itemQty'] = 1;
						break;
					case "limited" :
						$unordered = $this->total_items_unordered($itemID);
						if ($unordered < 1) {
							$this->msg .= $this->no_add_msg;
							$this->dimAddButton = true;
						}
						if ($newitem['itemQty'] > $unordered) {
							$newitem['itemQty'] = $unordered;
						}

						if ($this->items_are_available($itemID, $newitem['itemCatID'], $newitem['itemAvail'], $newitem['itemQty'])) {
							$this->items[$itemID]['itemQty'] += $newitem['itemQty'];
							$this->msg .= $this->yes_add_msg;
						} else {
							// Add maximum items available
							if ($newitem['itemQty'] > 0) {
								$this->items[$itemID]['itemQty'] += $newitem['itemQty'];
								$this->msg .= $this->add_remaining_msg;
							} else {
								$this->msg .= $this->no_add_msg;
							}
							$this->dimAddButton = true;
						}
						break;
					default :
						$this->items[$itemID]['itemQty'] += $newitem['itemQty'];
				}
			} else {
			// THIS IS A NEW ITEM
				if ($this->items_are_available($itemID, $newitem['itemCatID'], $newitem['itemAvail'], $newitem['itemQty'])) {
					$this->items[$itemID] = $newitem;
					$this->msg .= "";
				} else {
					$this->msg .= $this->no_add_msg;
				}
			}
		}
		// Update shipping values for item
		$this->set_item_shipping_costs ($itemID);

		// Fetch the current item for Javascript to play with
		$this->item = $this->items[$itemID];
		return $this->msg;
	}



	// UPDATE AN ITEM
	// Original items cannot be updated; they only can be "1"
	// Limited Editions must be less than the available number of prints.
	// *** We just can't know how many are available from within this class!
	// That requires going outside the class, because that number might
	// have changed. So, for now, we use the Avail originally recorded, and
	// we'll have to verify on checkout.
	// RETURNS: an array (msg, command)
	function update_item($itemUpdated) {
		$this->error_type = "";
		$itemID = $itemUpdated['itemID'];

		if (isset($this->items[$itemID])) {
			$item = $this->items[$itemID];

			$newItemQty = intval($itemUpdated['itemQty']);
			$newItemQty < 0 && $newItemQty = 0;

			// Clear current quantity so it isn't counted when we total all prints in the order
			$this->items[$itemID]['itemQty'] = 0;

			// Update the number of available prints in the item records
			$this->update_available_items_fields($itemID);
			$available = intval($item['itemAvail']);

			//$newItemOptions = $itemUpdated['itemOptions'];

			$msg = "";
			// IF THE ITEM QTY IS AN INTEGER, OR ZERO
			// UPDATE THE ITEM
			if ($newItemQty < 1) {
				$msg = $this->del_item($itemID);
			} else {
				// Original items can only have quantity = 1
				if ($item['itemEdType'] == "original" && $newItemQty != 1) {
					$this->error_type = __LINE__ . ": Originals are unique.";
					return array();
				} else {
					// Limited editions <= num prints avail
					// If not enough items are available, set to the max available.
					if ($item['itemEdType'] == "limited") {
						//$otherOrders = $this->total_same_items_ordered($itemID);
						if ($newItemQty > $available) {
							$newItemQty = min($newItemQty, $available);
							$msg = $this->add_remaining_msg;
						}
					}
				}
				$this->items[$itemID]['itemQty'] = $newItemQty;
				
				// Update shipping values for item
				$this->set_item_shipping_costs($itemID);

				// Fetch the current item for Javascript to play with
				$this->item = $this->items[$itemID];
			}
		}
		$res = array(
				"msg"		=> $this->msg, 
				"command"	=> $this->command
				);
		return $res;
	}

	// REMOVE AN ITEM
	function del_item($itemID) {
		// Fetch the current item for Javascript to play with - we'll need the item ID to dim/undim buttons and
		// stuff on the HTML page.
		$this->item = $this->items[$itemID];
		//$this->items[$itemID]['itemQty'] = 0;
		unset ($this->items[$itemID]);
		// If nothing left in cart, then tell the front-end to "clear cart". This will reset the cart, 
		// so if the "buy now" was dimmed, it will reappear.
		// We dont clear cart here. It will be clear twice if do that.
		if (count($this->items) == 0) {
			//$this->empty_cart();
			$this->command = "clear_cart";
		} else {
			return "";
		}
	}

	// Find cart item by ID; return the item if found, false if not found
	function find_item_by_id($itemID) {
		foreach ($this->items as $item)
		{
			if ($item['itemID'] == $itemID)
				return $itemID;
		}
		return false;
	}

	// Find cart item by itemCatID; return the item if found, false if not found
	function find_item_by_catalogid($itemCatID) {
		foreach ($this->items as $item)
		{
			if ($item['itemCatID'] == $itemCatID)
				return $item;
				//return $itemCatID;
		}
		return false;
	}

	// EMPTY THE CART
	function empty_cart() {
		$this->subtotal = 0;
		$this->total = 0;
		$this->itemcount = 0;
		$this->items = array();
		$this->shipping_params = array();
		$this->cart_params = array();
		$this->msg = "Cart is empty.";
		$this->command = "clear_cart";
		return $this->msg;
		
	}


	// RECALCULATE TOTALS
	function update_totals() {
		$this->itemcount = 0;
		$this->subtotal = 0;
		$this->total = 0;

		// If there are shipping values, we can calculate using the shipping info.
		// Otherwise, shipping info should be zero.
		$this->calc_cart_total_shipping();

		if(sizeof($this->items > 0)) {
			foreach($this->items as $item) {
				$item = $this->update_item_totals($item['itemID']);
				$this->subtotal = $this->subtotal + ($item['subtotal']);

				// TOTAL ITEMS IN CART (ORIGINAL wfCart COUNTED TOTAL NUMBER OF LINE ITEMS)
				$this->itemcount += $item['itemQty'];
			}
		}
		($this->discount > 100) && $this->discount = 100;
		$this->discount_amount = $this->subtotal * ($this->discount / 100);
		$this->subtotal = $this->subtotal - ($this->discount_amount);

		if ($this->shipping_is_set)
		{
			$this->get_sales_tax_rate();
			$this->salestax = $this->subtotal * $this->sales_tax_rate;
			$this->total = $this->subtotal + $this->totalshipping + $this->totalhandling + $this->salestax;
		} else {
			$this->total = 0;
			$this->salestax = 0;
			$this->error = "Shipping not set.";
		}
	}

	// calc_sales_tax
	// If the destination state = the supplier's state, then there is sales tax
	function get_sales_tax_rate() {
		// Sales Tax
		if ($this->shipping_params['pickup'] || ($this->shipping_params['source_state'] == $this->shipping_params['state'])) {
			$this->sales_tax_rate = $this->cart_params['cart_supplier_tax_rate'] / 100;
		} else {
			$this->sales_tax_rate = 0;
		}
	}


	/*
	 * update_item_totals($itemID)
	 * Update the price of an item.
	 * item['subtotal'] = price * quantity
	 * item['discount_rate'] = same as cart discount rate
	 * item['discount_amount'] = the amount of the discount, e.g. 10% discount of $100 is $10.
	 * item['discounted_subtotal'] = total of items price not including shipping/handling
	 * item['total'] = total price for this quantity of items plus shipping/handling
	 */

	function update_item_totals($itemID) {
		$item = $this->items[$itemID];
		
		$item['subtotal'] = $item['itemQty'] * $item['itemPrice'];
		($this->discount > 100) && $this->discount = 100;
		$item['discount_rate'] = $this->discount;
		$item['discount_amount'] = $item['subtotal'] * ($this->discount / 100);
		$item['discounted_subtotal'] = $item['subtotal'] - $item['discount_amount'];

		if ($this->shipping_is_set)
		{
			$item['total']= $item['discounted_subtotal']+ $item['item_total_shipping'] + $item['handling'];
		} else {
			$item['total']= $item['discounted_subtotal'];
		}
		$this->items[$itemID] = $item;
		return $item;
	}

	// Check if an item is available for sale from the API.
	// If the item is not in the cart, then it must be available.
	// If the item is in the cart, check to see if there are more for sale
	function item_available ($itemID) {
		$item = $this->items[$itemID];
		if (!$item) {
			// Not in cart
			return true;
		}
		$res = $this->items_are_available($itemID, $this->items[$itemID]['itemCatID'], $this->items[$itemID]['itemAvail'], 1);
		return $res;
	}



	// Check if there are items available to add to the cart.
	// We'll need to check all cart items with same catalog id.
	// If the item is not in the cart, then it is available as far as we know!
	function items_are_available($itemID, $itemCatID, $available, $itemQty=1) {
		if (!$itemID)
			return false;

		$item = $this->items[$itemID];
		if (!$item)
			return true;

		$this->update_available_items_fields($itemID);
		$totalordered = $this->total_same_items_ordered_by_catalogid($itemCatID);

		if ($item['itemEdType'] == "unlimited") {
			$res = true;
		} else {
			$res = ($totalordered + $itemQty) <= $available;
		}
		return $res;
	}


	// Calculate how many prints available.
	// Update the "available" field in the cart by asking the PriceSet database
	// how many are really free, at this moment. Someone might have bought while
	// the current buyer was picking his nose.
	// Update the "available" field for each item with the same itemCatID code.
	private function update_available_items_fields($itemID) {
		if (!$itemID)
			return false;
		$item = $this->items[$itemID];
		$itemCatID = $item['itemCatID'];
		$available = NumImagesAvailableForSale($itemCatID);
		// get count of all items ordered with this same catalog number
		$k = 0;
		foreach ($this->items as $i) {
			if ($i['itemCatID'] == $itemCatID) {
				$this->items[$i['itemID']]['itemAvail'] = $available;
				$k += intval($i['itemQty']);
			}
		}
//		// Update all items with new avail number
//		foreach ($this->items as $i)
//		{
//			$this->items[$i['itemID']]['itemAvail'] = $aMin;
//		}
	}

	// Get the total of items ordered with the same catalog number,
	// NOT including item $itemID! The items differ by options only.
	function total_same_items_ordered ($itemID)
	{
		if (!$itemID)
			return false;
		$item = $this->items[$itemID];
		$itemCatID = $item['itemCatID'];
		$k = 0;
		foreach ($this->items as $i)
		{
			if (($i['itemID'] != $itemID) && ($i['itemCatID'] == $itemCatID))
			{
				$k += intval($i['itemQty']);
			}
		}
		return $k;
	}

	// How many items are available to be added for an item?
	function total_items_unordered ($itemID)
	{
		$ordered = intval($this->items[$itemID]['itemQty']) + $this->total_same_items_ordered ($itemID);
		$available = intval($this->items[$itemID]['itemAvail']);
		$unordered = $available - $ordered;
		return $unordered;
	}

		// Get the total of items ordered with the same catalog number,
	// NOT including item $itemID! The items differ by options only.
	function total_same_items_ordered_by_catalogid ($itemCatID)
	{
		if (!$itemCatID)
			return false;
		$k = 0;
		foreach ($this->items as $i)
		{
			if ($i['itemCatID'] == $itemCatID)
			{
				$k += $i['itemQty'];
			}
		}
		return $k;
	}



	// Decide whether the Add to Cart button should be active or not.
	// Dim the Add... button if
	// - no remaining items
	// Return the itemCatID (not the itemID) of the item to dim.
	function get_add_button_state($itemID) {
		$this->addButtonState = 0;
		if ($this->items[$itemID]['itemEdType'] != "unlimited") {
			$unordered = $this->total_items_unordered($itemID);
			if ($unordered > 0) {
				$this->item = $this->items[$itemID];
				$this->addButtonState = $this->items[$itemID]['itemCatID'];
			} else {
				$this->addButtonState = 0;
			}
		} else {
			$this->addButtonState = 1;
		}
		return $this->addButtonState;
	}

	// PROCESS AND DISPLAY CART
	function display_cart() {
		echo $this->build_cart($params);
	}


	// BUILD CART AS HTML TO PUT ON A PAGE
	// $params : options from the JS, variables used to draw the cart, e.g. frame shop name in a link.
	function build_cart($params) {

		($params['cartCompact'] != "") && $this->cartCompact = $params['cartCompact'];
		
		// IF THIS ERROR IS TRUE THE VISITOR UPDATED THE CART FROM THE CHECKOUT PAGE USING AN INVALID PRICE FORMAT
		// PASSED AS A SESSION VAR SINCE THE CHECKOUT PAGE USES A HEADER REDIRECT
		// IF PASSED VIA GET THE QUERY STRING STAYS SET EVEN AFTER SUBSEQUENT POST REQUESTS
		if ($_SESSION['quantity_error'] == true) {
			$this->msg = "Invalid price format [".__LINE__."]";
			unset($_SESSION['quantity_error']);
		}

		// OUTPUT THE CART

		// IF THERE'S AN ERROR MESSAGE WRAP IT IN SOME HTML
		if ($error_message) {
			$error_message = "<p class='fpcart-error'>$error_message</p>";
		}


		
		if(count($this->items) > 0) {
			if ($this->cartCompact)
			{
				$cartHTML = FetchSnippet("fpcart_cart_compact");
				$cartFooterHTML = FetchSnippet("fpcart_cart_footer_compact");
				$cartItemHTML = FetchSnippet("fpcart_item_compact");
				$cartUniqueItemHTML = FetchSnippet("fpcart_unique_item_compact");
			} else {
				$cartHTML = FetchSnippet("fpcart_cart");
				$cartFooterHTML = FetchSnippet("fpcart_cart_footer");
				$cartItemHTML = FetchSnippet("fpcart_item");
				$cartUniqueItemHTML = FetchSnippet("fpcart_unique_item");
			}
			//-------
			// Fill out rows of items

			// Calculate cart totals and item calculations, e.g. shipping per item
			$this->update_totals();

			// Items marked as groupable, e.g. prints
			$rows = '';
			foreach($this->items as $item) {
				$item['subtotal'] = number_format($item['itemQty'] * $item['itemPrice'],2);
				$item['itemPrice'] = number_format($item['itemPrice'],2);

				// Complicated...need the item cost to avoid selling below cost.
				// Finish later!
				//$item['itemPriceDiscounted'] = number_format($item['itemPriceDiscounted'],2);


				$item['item_display_shipping'] = number_format($item['item_display_shipping'],2);
				$item['item_display_handling'] = number_format($item['item_display_handling'],2);

				// Add parentheses to short desc
				if ($item['itemShortDesc'])
					$item['itemShortDesc'] = "(".$item['itemShortDesc'].")";
				// Add in params
				$item = array_merge($item, array (
					"stage"			=> $params['stage'],
					"finishing_name"	=> $params['finishing_name'],
					"remove_item_text"	=> $params['remove_item_text']
					));


				if ($item['itemEdType'] != "original") {
					$rows.= Substitutions($cartItemHTML,$item) . "\n";
				}
				else {
					$rows.= Substitutions($cartUniqueItemHTML,$item) . "\n";
				}
			}

			// Fill in cart template
			$f = array ();
			$f['FOOTER'] = $cartFooterHTML;
			$f['TITLE'] = "Shopping Cart";
			// Number of items in the cart, NOT the number of rows. A row may have more than one item!
			$f['ITEM_COUNT'] = $this->itemcount;
			$f['CART_ITEMS'] = $rows;
			$f['DISCOUNT'] = $this->discount;
			$f['DISCOUNT_DESCRIPTION'] = $this->discount_description;
			$f['DISCOUNT_AMOUNT'] = number_format($this->discount_amount,2);
			$f['SHIPPING'] = number_format($this->totalshipping,2);
			$f['HANDLING'] = number_format($this->totalhandling,2);
			$f['TAX'] = number_format($this->salestax,2);
			if ($this->salestax == 0) {
				$f['DISPLAY_SALES_TAX'] = "display:none;";
			} else {
				$f['DISPLAY_SALES_TAX'] = "";
			}
			$f['SUBTOTAL'] = number_format($this->subtotal,2);
			$f['TOTAL'] = number_format($this->total,2);
			$f['CURRENCY'] = $this->currency;
			$f['ERRORMESSAGE'] = $error_message;
			$f['CART_HAS_ITEMS'] = $this->itemcount;

			// Buttons
			if ($this->have_shipping_params()) {
				$f['SET_SHIPPING_LABEL'] = "Change Shipping";
				$f['CHECKOUT_BUTTON_CLASS'] = "";
			} else {
				$f['SET_SHIPPING_LABEL'] = "Set Shipping";
				$f['CHECKOUT_BUTTON_CLASS'] = "fp_dim";
			}


			if (!$this->discount)
			{
				$f['SHOWHIDE_DISCOUNT'] = "style='display:none;'";
			}
			
			$output = Substitutions ($cartHTML, $f);
		}
		else {
			$output = "<!-- empty cart --><i>Empty Shopping Cart</i>\n";
		}


		$this->cartoutput = $output;
		return $output;
	}


	function set_discount_code ($code, $imageID) {
		$this->cart_params['couponCode'] = $code;
		if ($code && $imageID) {
			$image = FetchImage($imageID);
			$couponInfo = GetCouponDiscount ($code, $image['ArtistID']);
			$this->discount = round($couponInfo['value'],0);
			$this->discount_description = $couponInfo['description'];
		} else {
			$this->discount = 0;
			$this->discount_description = "";
		}
	}



	function set_discount($d,$desc)
	{
		$this->discount = round($d,0);
		$this->discount_description = $desc;
	}


	// ----------------------------------------
	// set_cart_general_params ()
	// Set the cart object's general params, i.e. cart-wide variables, such as payments system ID, return page URL, etc.
	// Be careful, don't send just anything — it will all be added to the $this->cart_params
	// These params will be passed at checkout
	function set_cart_general_params ($params)
	{
		$this->cart_params = array();
		
		foreach ($params as $k => $v) {
			$this->cart_params[$k] = $v;
		}
	}



	// ----------------------------------------
	// set_cart_shipping_params ()
	// Set the cart object's shipping params
	// Be careful, don't send just anything — it will all be added to the $this->shipping_params
	function set_cart_shipping_params ($params)
	{
		foreach ($params as $k => $v) {
			$this->shipping_params[$k] = $v;
		}

		// Update item shipping values. This includes calls to a shipper's system to get prices!
		foreach ($this->items as $item) {
			$this->set_item_shipping_costs($item['itemID']);
		}
	}



	// ----------------------------------------
	// calc_cart_total_shipping
	//
	// If we can calculate shipping, set the flag to true:
	//		$this->shipping_is_set
	//
	// We can calc shipping and handling when we have cart shipping params:
	// e.g. ZIP code, country, method, residence/business
	//
	// Shipping per item:
	// Some items are groupable for shipping, such as flat paper like photo prints.
	// There's a first item shipping cost, and additional item cost for prints.
	// Handling is charged once per shipment.
	// All other items are considered individual shipments
	//
	// *** We assume all items have their shipping costs set. ***
	// This should be set when the shipping info is set for the cart. This way, we don't keep calling the shipper's
	// system to get prices; we only do it once.
	
	function calc_cart_total_shipping() {
		$this->shipping_is_set = false;

		$totalshipping = 0;
		$totalhandling = 0;
		$shippingGroups = array();

		if ($this->have_shipping_params()) {			
			foreach ($this->items as $item) {
				if (!$this->have_item_shipping_info($item)) {
					$this->totalshipping = 0;
					$this->totalhandling = 0;
					$this->shipping_is_set = false;
					$this->error = "Item is missing shipping information, such as weight and dimensions.";
				}
				// If in a group, then charge 2nd rate for more than 1 item
				$itemID = $item['itemID'];
				if ($item['itemShippingGroup'] > 0) {
					if ($shippingGroups[$item['itemShippingGroup']] < 1) {
						// 1st item in group (not identical items, but we group them for shipping, like prints)
						$totalshipping += $item['shipping'];
						$totalhandling += $item['handling'];
						$shippingGroups[$item['itemShippingGroup']]['shipping'] = $item['shipping'];
						$shippingGroups[$item['itemShippingGroup']]['shipping2'] = $item['shipping2'];
						$shippingGroups[$item['itemShippingGroup']]['handling'] = $item['handling'];
						// if the quantity of this one item is > 1, then we add costs of secondary items
						$totalshipping += $item['shipping2'] * ($item['itemQty'] - 1);
						// update item values for display and for shipper ordering (uses total of shipping, not quant * shipping)
						$this->items[$itemID]['item_display_shipping'] = $item['shipping'] + ($item['shipping2'] * ($item['itemQty'] - 1));
						$this->items[$itemID]['item_total_shipping'] = $item['shipping'] + ($item['shipping2'] * ($item['itemQty'] - 1));
						$this->items[$itemID]['item_display_handling'] = $item['handling'];
					} else {
						// Additional items in the group (not identical items, but we group them for shipping, like prints)
						$totalshipping += $item['shipping2'];
						$shippingGroups[$item['itemShippingGroup']]['shipping'] += $item['shipping2'] * $item['itemQty'];
						// update item values for display and for shipper ordering (uses total of shipping, not quant * shipping)
						$this->items[$itemID]['item_display_shipping'] = $item['shipping2'] * ($item['itemQty'] - 1);
						$this->items[$itemID]['item_total_shipping'] = $item['shipping2'] * ($item['itemQty'] - 1);
						$this->items[$itemID]['item_display_handling'] = 0;
					}
					// Add quantity to the count of items in this group
					$shippingGroups[$item['itemShippingGroup']]['count'] += $item['itemQty'];
				} else {
					// Not in a group; charge full shipping/handling per item
					$totalshipping += $item['shipping'] * $item['itemQty'];
					$totalhandling += $item['handling'] * $item['itemQty'];
					// update item values for display and for shipper ordering (uses total of shipping, not quant * shipping)
					$this->items[$itemID]['item_display_shipping'] = $item['shipping'] * $item['itemQty'];
					$this->items[$itemID]['item_total_shipping'] = $item['shipping'] * $item['itemQty'];
					$this->items[$itemID]['item_display_handling'] = $item['handling'] * $item['itemQty'];
				}
			}
			$this->totalshipping = $totalshipping;
			$this->totalhandling = $totalhandling;
			$this->shipping_is_set = true;
		} else {
			$this->totalshipping = 0;
			$this->totalhandling = 0;
			$this->shipping_is_set = false;
		}
	}

	/*
	 * set_item_shipping_costs ($itemID)
	 * Set the shipping/handling costs for an item.
	 * This requires getting the price from the shipper, so we try to minimize the number of calls.
	 */

	function set_item_shipping_costs ($itemID) {
		$shippingCosts = $this->fetch_item_shipping_cost ($this->items[$itemID]);
		$this->items[$itemID]['shipping'] = $shippingCosts['shipping'];
		$this->items[$itemID]['shipping2'] = $shippingCosts['shipping2'];
		$this->items[$itemID]['handling'] = $shippingCosts['handling'];
	}



	/*
	================================================================
	 * Fetch shipping costs for an item.
	 * This includes a call to the shipper!
	 * If the costs are not set in the item array,
	 * get them from the shipper.
	 * Handling does vary depending on the item --- large, framed
	 * things are different from little tiny things, right?
	 * Packing, forms, etc. are different, so handling is different.
	================================================================
	*/

	function fetch_item_shipping_cost($item) {

		if ($this->have_shipping_params() && $this->have_item_shipping_info($item) ) {

			if ($this->shipping_params['pickup']) {
				$shippingCosts['shipping'] = 0;
				$shippingCosts['shipping2'] = 0;
				$shippingCosts['handling']= 0;
			} else {
				$quote = $this->get_shipping_costs_from_shipper($item, "Ups");
				$shippingCosts['shipping'] = $quote;
				$shippingCosts['shipping2'] = 0;
				if ($this->shipping_params['origCountry'] == $this->shipping_params['destCountry']) {
					$shippingCosts['handling'] = $item['handling_local'];
				} else {
					$shippingCosts['handling'] = $item['handling_intl'];
				}
			}
		} else {
			$this->error = "Shipping information incomplete.";
		}

		return $shippingCosts;
	}


	// function: have_shipping_params
	// Check we have all params to calculate shipping/handling costs
	function have_shipping_params() {
		if (($this->shipping_params['pickup']) || ($this->shipping_params['product'] &&
				$this->shipping_params['origPostal'] && $this->shipping_params['origCountry']
				&& $this->shipping_params['destPostal'] && $this->shipping_params['destCountry']
				&& $this->shipping_params['rateCode']
				&& $this->shipping_params['shippingContainerCode']
				&& $this->shipping_params['rescom']
				)) {
			return true;
		} else {
			return false;
		}
	}

	/*
	 * item_shipping_info ($item = array() )
	 */
	function have_item_shipping_info ($item = array()) {
		return ($item['shipping_packageWeight'] &&
				$item['shipping_shippingHeight'] &&
				$item['shipping_shippingLength'] &&
				$item['shipping_shippingWidth']
				);
	}


	/*
	================================================================
	Shipping calculator
	================================================================
	*/

	/*
	 * $this->shipping_params key names:
		product
		origCountry
		origPostal
		destPostal
		destCountry
		currency
		customValue
		pickup
		ship_to_state_box
		packageWeight
		rateCode
		rescom
		shippingContainerCode
		shippingHeight
		shippingLength
		shippingName
		shippingValue
		shippingWidth
		state
		weight_std

	 */
	function get_shipping_costs_from_shipper($item, $shipper = "Ups") {
		switch ($shipper) {
			case "Ups" :

			default :
				$rate = new Ups;

				$rate->weight($item['shipping_packageWeight']);
				$rate->shippingValue = $item['itemPrice'];

				$rate->upsProduct($this->shipping_params['product']); // See upsProduct() function for codes
				$rate->origin($this->shipping_params['origPostal'], $this->shipping_params['origCountry']); // Use ISO country codes!
				$rate->dest($this->shipping_params['destPostal'], $this->shipping_params['destCountry']); // Use ISO country codes!
				$rate->rate($this->shipping_params['rateCode']); // See the rate() function for codes
				$rate->container($this->shipping_params['shippingContainerCode']); // See the container() function for codes
				$rate->rescom($this->shipping_params['rescom']); // See the rescom() function for codes
				$quote = $rate->getQuote();

				return $quote;

				break;
		}
	}

	/*
	================================================================
	Error Messages
	================================================================
	*/

	function error_message ($errorcode) {
		$errorcode || $errorcode = $this->errorcode;
		switch($errorcode) {
			case 'itemQty':
				$msg = "Invalid quantity.";
				break;
			case 'itemPrice':
				$msg = "Invalid item price.";
				break;
			case 'dup':
				$msg = "Item is already in the cart.";
		}
		return $msg;
	}




	/*
	================================================================
	AJAX
	================================================================
	*/

	function get_ajax_contents () {
		$DEBUG = true;

		// Get JSON data
		if ($_POST) {
			$data = json_decode(stripslashes($_POST['data']), true);
		}

		$result = array();

		// Return JSON values
		header("Content-type: text/plain");
		echo json_encode($result);
	}


	function return_ajax_result ($result) {
		$result || $result = $this->result;

		// Return JSON values
		header("Content-type: text/plain");
		echo json_encode($result);
	}

	/*
	================================================================
	get_paypal_cart_vars ($params : array)
	 * Build array of variables to "submit "upload"
	 * cart as a Paypal cart.
	 * $params overwrites any default settings, such as the charset.
	 *
	 * Paypal user info, should be filled in before reaching Paypal.
	 * This prevents a change of address, which would affect shipping.
	 * Also, we can gather user info this way.
	 * So, the following should be already in the fpCart object.

	 	business
		first_name
		last_name
		address1
		address2
		city
		state
		zip
		country
		night_phone_a
		night_phone_b
		night_phone_c
		email

	================================================================
	*/

	function get_paypal_cart_vars ($params = array())
	{
		$pp = array();

		if(count($this->items) > 0) {
			$pp['cmd'] = "_cart";
			//$pp['display'] = "1";
			$pp['upload'] = "1";
			//$pp['paymentaction'] = "sale";
			$pp['custom'] = "custom";
			$pp['invoice'] = $this->invoice;
			$pp['charset'] = "utf-8";
			$pp['currency_code'] = "USD";

			// Cart variables
			$pp['business'] = $this->cart_params['cart_payment_id'];
			//$pp['add'] = $this->cart_params['cart_add'];
			$pp['return'] = $this->cart_params['cart_return'];
			$pp['shopping_url'] = $this->cart_params['cart_shopping_url'];
			$pp['rm'] = $this->cart_params['cart_rm'];
			$pp['page_style'] = $this->cart_params['cart_page_style'];
			$pp['cancel_return'] = $this->cart_params['cart_cancel_return'];
			$pp['currency_code'] = $this->cart_params['cart_currency_code'];
			$pp['weight'] = $this->cart_params['cart_weight'];
			$pp['weight_unit'] = $this->cart_params['cart_weight_unit'];
			$pp['lc'] = $this->cart_params['cart_lc'];
			//$pp['bn'] = $this->cart_params['cart_bn'];
			$pp['notify_url'] = $this->cart_params['cart_notify_url'];

			if ($this->shipping_params['pickup']) {
				$pp['no_shipping'] = "1";
			} else {
				$pp['no_shipping'] = "0";
			}

			// ****** TESTING
			//$pp['notify_url'] = "http://24.6.122.195/fp/ipn_res.php";

			$pp['notify_url'] = PAYPAL_NOTIFY_URL;

			//$pp['invoice'] = $this->cart_params['cart_invoice'];
			$pp['custom'] = $this->cart_params['cart_custom'];

			// Shipping is an item entry, not a cart-wide entry
			// However, we can simply add it this way:
			//$pp['shipping_1'] = $this->totalshipping;
			$pp['handling_cart'] = $this->totalhandling;

			// DISCOUNTS
			// Cart-wide discount. Apply the discount from the coupon???
			// No, should only apply to items. We don't discount less than cost!
			$pp['discount_amount_cart'] = "";
			$pp['discount_rate_cart'] = "";

			/*
			// The following should be entered by the buyer
			// before calling this function.
			$pp['business'] = $this->pp_business;
			$pp['first_name'] = $this->pp_first_name;
			$pp['last_name'] = $this->pp_last_name;
			$pp['address1'] = $this->pp_address1;
			$pp['address2'] = $this->pp_address2;
			$pp['city'] = $this->pp_city;
			$pp['state'] = $this->pp_state;
			$pp['zip'] = $this->pp_zip;
			$pp['night_phone_a'] = $this->pp_night_phone_a;
			$pp['night_phone_b'] = $this->pp_night_phone_b;
			$pp['night_phone_c'] = $this->pp_night_phone_c;
			$pp['email'] = $this->pp_email;
			// not sure about this one:
			$pp['country'] = $this->pp_country;

			// The above will override the user's Paypal settings
			$pp['address_override'] = "1";
			*/

			// Item variables
			/*
				amount
				discount_amount
				discount_amount2
				discount_rate
				discount_rate2
				discount_num

				item_name
				item_number
				mc_gross
				quantity
				shipping
				shipping2
				tax
				tax_rate

				weight
				weight_unit

				on0
				on1
				os0
				os1

			 */

			$rows = '';
			$x = 1;
			$itemcounter = array();
			foreach($this->items as $item) {
				$itemCatID = $item['itemCatID'];
				
				$item['subtotal'] = number_format($item['itemQty'] * $item['itemPrice'],2);
				$item['itemPrice'] = number_format($item['itemPrice'],2);

				/*
				 * FPCart javascript variables available
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
				*/

				// PP variables for each item
				$pp['amount_'.$x] = $item['itemPrice'];

				$pp['discount_amount_'.$x] = $item['discount_amount'];
				$pp['discount_amount2_'.$x] = $item['discount_amount2'];
				$pp['discount_rate_'.$x] = $item['discount_rate'];
				$pp['discount_rate2_'.$x] = $item['discount_rate2'];
				$pp['discount_num_'.$x] = $item['discount_num'];

				$pp['item_name_'.$x] = $item['itemName'];
				$itemcounter[$itemCatID]++;
				
				// This is a unique ID in this record, and unique combined with Invoice#
				$itemnumber = str_pad($itemCatID, 6, "0", STR_PAD_LEFT)."_".$itemcounter[$itemCatID];
				$pp['item_number_'.$x] = $itemnumber;
				
				// old: use item number from cart, which is an encoded weird number
				//$pp['item_number_'.$x] = $item['itemID'];	// This is a unique ID in this record, and unique combined with Invoice#
				$pp['quantity_'.$x] = $item['itemQty'];

				//$pp['shipping_'.$x] = $item['shipping'];
				$pp['shipping_'.$x] = $item['item_total_shipping'];
				$pp['shipping2_'.$x] = $item['shipping2'];


				$pp['tax_'.$x] = $item['tax'];
				$pp['tax_rate_'.$x] = $item['tax_rate'];
				$pp['weight_'.$x] = $item['weight'];
				$pp['weight_unit_'.$x] = $item['weight_unit'];

				$pp['on0_'.$x] = 'Desc';
				//$pp['on1_'.$x] = 'Short_desc';
				$pp['os0_'.$x] = $item['itemDesc'];
				//$pp['os1_'.$x] = $item['itemShortDesc'];

				$x++;
			}
		}

		//$pp = array_merge($pp, $params);
		// Return encoded URL data, to add to the paypal processor, e.g. http://paypal.com?
		$ppurl = $pp;
		//array_walk($ppurl , create_function('&$v,$k', '$v && ($v = $k."=".urlencode($v));'));
		//array_walk($ppurl , create_function('&$v,$k', '$v && ($v = "$k=$v");'));
		$ppurl2 = array();
		foreach ($ppurl as $k => $v) {
			$v && ($ppurl2[$k] = "$k=".urlencode($v));
		}
		ksort ($ppurl2);
		$this->ppvars_encoded = implode("&",$ppurl2);

		$this->ppvars = $pp;
		return $pp;
	}

}	// END fpcart

?>
