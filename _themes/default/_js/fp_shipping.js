/*	
	FP Shipping Javascripts
*/

/*
function buildUPSQuoteURL (rateCode, resComCode){

	

	$url = "http://www.ups.com/using/services/rave/qcost_dss.cgi?AppVersion=1.2&AcceptUPSLicenseAgreement=yes&ResponseType=application/x-ups-rss&ActionCode=3&RateChart=" + rateCode + "&DCISInd=0&SNDestinationInd1=0&SNDestinationInd2=0&ResidentialInd=" + resComCode + "&PackagingType=" + $this->containerCode + "&ServiceLevelCode=" + $this->upsProductCode + "&ShipperPostalCode=" + $this->originPostalCode + "&ConsigneePostalCode=" + $this->destPostalCode + "&ConsigneeCountry=" + $this->destCountryCode + "&PackageActualWeight=" + $this->packageWeight + "&DeclaredValueInsurance=0";
	

	$result = explode("%", $result);
	
	$errcode = $result[4];
	switch($errcode){
		case 3:
			$returnval = $result[14];
			break;
	}

	if(! $returnval) { $returnval = "error"; }
		return $returnval;
	}
}
*/

function StorePreOrder()
{
	var xmlHttp, serverResponse;
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
									serverResponse = xmlHttp.responseText ;
									}
								}
	
	item_number = document.getElementById("currentimageID").value;
	item_number = item_number;
	spec = document.getElementById("spec").value;
	spec = Url.encode(spec);
	desc = document.getElementById("desc").value;
	desc = Url.encode(desc);
	xmlHttp.open("GET","writepreorder.php?item_number=" + item_number + "&spec=" + spec + "&desc=" + desc,true);
	xmlHttp.send(null);
}





/*

XML Testing: to https://wwwcie.ups.com/ups.app/xml/Rate


*/
// =================================
