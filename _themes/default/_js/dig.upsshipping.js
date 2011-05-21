/*
 * UPS Shipping
 *
 * AJAX scripts to get shipping prices (currently for UPS)
 *
 * Copyright (C) 2009 David Gross (dgross@mimetic.com)
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */
 
 /*
	upsshipping (settings)
		Creates a new page element which shows the shipping price, based on the options.
		Note, many options set the names of the page elements which have the data.
		
		Source HTML page ID's: (See defaults at end of script)
			Many of the options let you set the ID of the HTML input elements which
			the script uses for calculations.
		
		Example (Javascript):
			var options = {
				shippingProductID	: "#myProductCode",
				origin_cityID		: "#myFormCityCode"
			}
			shipcalc = new UPSShippingCalculator('#shipping_price', options);
			shipcalc.getQuote();

	
		HTML:
			Your form must include all the input fields listed in the options,
			e.g. <input type="text" id="shipping-shippingProduct">
	
	Parameters:
		options:	(Object) (see defaults, at end, for property names)
	
	Returns:
		The wrapped set.




 */
 
function UPSShippingCalculator (target, options) {
	
	var rateCode, containerCode, originPostalCode, originCountryCode;
	
	// set context vars
	//----------------------------------------------------------------------
	
	var defaults = setDefaults();
	
	// build main options before element iteration
	// The defaults are defined below.
	//----------------------------------------------------------------------
	var opts = $.extend(
		defaults, 
		options||{}
	);
	
	////////////////////////////////////////////////////////////////////
	//
	// initial setup
	//
	////////////////////////////////////////////////////////////////////
	
	// upsshipping vars
	//------------------------------------------------------------------
	
	debug( opts.debug, '--------- BEGIN "UPS Shipping" PLUGIN ---------');

	// dispose of no-js warning
	if(opts.nojsclass) {
		$(opts.stage).find('.' + opts.nojsclass).remove();
	}
	

	// Set request variables
	rateCode = $(opts.rateCodeID).val();

	destPostalCode = $(opts.destPostalCodeID).val();
	destCountryCode = $(opts.destCountryID).val();
	
	originPostalCode = $(opts.originPostalCodeID).val();
	originCountryCode = $(opts.originCountryID).val();
	resComCode = $(opts.rescomID).val();
	containerCode = $(opts.shippingContainerCodeID).val();
	upsProductCode = $(opts.shippingProductID).val();
	packageWeight = $(opts.packageWeightID).val();
	shippingLength = $(opts.shippingLengthID).val();
	shippingHeight = $(opts.shippingHeightID).val();
	shippingWidth = $(opts.shippingWidthID).val();
	shippingValue = $(opts.shippingValueID).val();
	
	//init();
	
	
	/*
		AJAX get a quote from the shipping_calc on the server. We can't seem to call UPS directly,
		don't know why.
	*/
	
	this.getQuote = function (priceField,shippingMethodField){
		var xmlHttp, query, paramnames, paramvalues, url, i;
		
		try
			{
			// Firefox, Opera 8.0+, Safari
			xmlHttp=new XMLHttpRequest();
			}
		catch (e)
			{
			// Internet Explorer
			try
				{
				xmlHttp=new ActiveXObject("Msxml2.XMLHTTP");
				}
			catch (e)
				{
				try
					{
					xmlHttp=new ActiveXObject("Microsoft.XMLHTTP");
					}
				catch (e)
					{
					alert("Your browser does not support AJAX!");
					return false;
					}
				}
			}
		
		xmlHttp.onreadystatechange = function()
			{
			if(xmlHttp.readyState==4)
				{	
					// this will pass on error msgs and good results
					rate = xmlHttp.responseText;
					$(priceField).val(rate);
					i = document.getElementById("shipping-shippingProduct").selectedIndex;
					t = document.getElementById("shipping-shippingProduct").options[i].text;
					product = document.getElementById("shipping-shippingName").value + ": " + t + " ("+$('#shipping-shippingProduct').val()+")";
					$(shippingMethodField).val(product);
					UpdatePrices (FP_CURRENCY, FP_CURRENCY_POSITION);
				}
			}
		
		paramnames = ["13_product", "14_origCountry", "15_origPostal", "19_destPostal", "22_destCountry", "document", "currency", "23_weight", "24_value", "25_length", "26_width", "27_height", "ADS1", "47_rate_chart", "48_container", "49_residential" ];			
	
		paramvalues = [upsProductCode, originCountryCode, originPostalCode, destPostalCode, destCountryCode, "01", "USD", packageWeight, shippingValue, shippingLength, shippingWidth, shippingHeight, opts.ads, opts.ratechart, opts.container, resComCode ];			
		
		url = "shipping_calc.php"
		
		query = "10_action=3";
		//query = "";
		
		for (i=0;i<paramnames.length;i++) {
			n = paramnames[i];
			v = paramvalues[i];
			query = query + "&" + n + "=" + v;
		}
		//debug (opts.debug, 'URL+query: '+url+"?"+query);
		debug (opts.debug, 'shippingWidth, shippingHeight, packageWeight: '+shippingWidth + ", "+shippingHeight+', '+packageWeight);
//alert ('dig.upsshipping.js, line 167 :' + query);
		query = "shipper=" + FP_SHIPPER_CODE + "&query="+Url.encode(query);
		xmlHttp.open("GET",url+"?"+query);
		xmlHttp.send(null);

	}
	
	
	////////////////////////////////////////////////////////////////////////////
	//
	// defaults
	// set default options
	//
	////////////////////////////////////////////////////////////////////////////

	function setDefaults () {
		var defaults = {
			// default values that probably won't change
			ads:							"ADS",					// adult signature required
			ratechart:					"One+Time+Pickup",
			container:					"00",					// 00 = your packaging
			
			// Where we get data: ID's of HTML form elements
			shippingProductID:			"#shipping-shippingProduct",
			origin_cityID:				"#shipping-originCity",
			originPostalCodeID:			"#shipping-originPostalCode",
			originCountryID:				"#shipping-originCountry",
			destCityID:					"#shipping-destCity",
			destPostalCodeID:			"#shipping-destPostalCode",
			destCountryID:				"#shipping-destCountry",
			rateCodeID:					"#shipping-rateCode",
			shippingContainerCodeID:		"#shipping-shippingContainerCode",
			rescomID:					"#shipping-rescom",
			packageWeightID:				"#shipping-packageWeight",
			shippingLengthID:			"#shipping-shippingLength",
			shippingHeightID:			"#shipping-shippingHeight",
			shippingWidthID:				"#shipping-shippingWidth",
			shippingValueID:				"#shipping-shippingValue",
	
			debug:						false						// turn on console output (slows down IE8!)
	
		};
		return defaults;
	}

}
