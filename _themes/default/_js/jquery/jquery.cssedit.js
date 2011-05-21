/*
 * jQuery cssEdit Plugin
 * version: 0.01 (24-APR-2009)
 *
 * Examples and documentation at:
 * Dual licensed under the MIT and GPL licenses:
 *   http://www.opensource.org/licenses/mit-license.php
 *   http://www.gnu.org/licenses/gpl.html
 */

/*

REQUIRES:
	jquery.cookies.js	:	http://code.google.com/p/cookies/wiki/Documentation
	jquery.form.js ?

*/

/*
    Usage Note:
    -----------

    cssEditPush (objectindex):
    		Add a version of an object to a queue for that object
    cssEditPop (objectindex)
    		Restore a version of an object from a queue for that object
    cssEditRestoreAll ()
    		Restore all objects to their original states from their queues

*/

(function($) {
	var cssStack = [];
	var cssRules = [];
	var cssChanges = {
		'rules':[]
	};
	var cssChangesSelectors = [];
	var cssSave = {
		'set':false
	};
	var cookieName = "cssEditSwitch";
	var cookieValue = {};
	var cookieOptions = {
		expires: 0.1
	};
	var cssEditThemeID, cssEditThemeName, cssEditSelector, cssEditDefaultTheme;

	// Show the console --- useful for debugging
	var cssEditShowConsole = false;

	var text_turn_editor_on = "Turn On Editor", text_turn_editor_off = "Turn On Navigation";

	// Turn off caching of all files!
	$('head').prepend("<META HTTP-EQUIV='Expires' CONTENT='Tue, 01 Jan 1980 1:00:00 GMT'><META HTTP-EQUIV='Pragma' CONTENT='no-cache'>");


	// Install the css editor for editable items.
	// Note, use '*.myclass' because that way you know the class name of the item
	// Example: $('*.editable').cssEdit();

	$.fn.cssEdit = function (themeID, themeName) {
		// set cssEdit globals from global vars on the HTML page
		cssEditThemeID = themeID;
		cssEditThemeName = themeName;
		cssEditDefaultTheme = FP_DEFAULT_THEME;
		cssEditSelector = this.selector;
		cookieValue = $.GetCookie(cookieName);
		
		// Current user must be owner of page, or admin user
		if (cookieValue.userID <= 1 || cookieValue.userID == FP_USER_ID) {
			// add current themeID to global changes list
			cssChanges.themeid = themeID;
			// create dialog box, etc.
			$.cssEditSetupEditor();
			// Bind functionality to the wrapped set
			this.cssEditBindEditor().cssEditAddSwitch().cssEditBindHighlighter();

			// If this is not a page belonging to the FP_USER_ID, then no editing
		} else {
			//$('#error').css({'top':'10px','right':'10px','position':'fixed','text-align':'right'})
			//.html("<div style='height:30px;' id='cssEditTopBar'><div id='cssEditControls'>Sorry, you can't edit this theme because you don't own this page. Go to a page you own if you want to edit the theme. <button type='button' id='cssEditClose'>Close Editor</button></div></div>");
			var controlbar = '<div id="cssEditTopBar" style="height:27px;">';
			controlbar += '<span id="cssEditEditorTitle">Theme Editor</span>';
			controlbar += '<div id="cssEditControls">';
			controlbar += "Sorry, you can't edit this theme because you don't own this page. Go to a page you own if you want to edit the theme. <button type='button' id='cssEditClose'>Close Editor</button></div></div>";
			controlbar += '</div></div>';

			$('body').append(controlbar);
			$('#cssEditClose').click(function () {
				CloseEditor();
			});
			//alert ("Live Theme Editor says:\nSorry, you can't edit this theme because you don't own this page. Go to a page you own if you want to edit the theme.")
		}

		return this;
	}

	// Add highlighting to editable items
	$.fn.cssEditBindHighlighter = function () {
		// Check highlight flag
		return this.css('cursor', 'pointer')
		.bind('mouseover', function(e) {
			var myName = $.getCssName(this);
			$(this).cssEditHighlight(true);
			
			// THIS WORKS, BUT IT IS ANNOYING!
			// show little flag to tell you what you could edit
			// $(this).cssEditFlag(true, e.pageX, e.pageY);
			
			showMessage('Click to edit : ' + myName);
			return false;
		})
		.bind('mouseout', function() {
			$(this).cssEditHighlight(false);
			$('#message').html('Choose an item to edit.');

			// Remove little flag to tell you what you could edit
			// THIS WORKS, BUT IT IS ANNOYING!
			// $(this).cssEditFlag(false,0,0);

			return false;
		})
		.filter('menuonly').cssEditAddToMenu()
		.end();
	}

	// Add a 'disable/enable' button 
	$.fn.cssEditAddSwitch = function () {
		t = this.selector;
		//$('#cssEditSwitch').
		//	toggle(cssEditReenable(t), cssEditDisable(t));
		$('#cssEditSwitch').
			toggle(function() {
				cssEditDisable(t);
			}, function () {
				cssEditReenable(t);
		});
		return this;
	}

	// Turn off editor and refresh the page
	function CloseEditor() {
		$.SetCookie(cookieName, {}, cookieOptions);
		window.location.reload();
	}

// Opens up the gallery with the theme editor activated.
// The FP_USER_ID is set through Javascript in the system_vars.txt, which is a javascript text file loaded up with admin.php
	$.fn.cssEditActivate = function () {
		return this.each(function() {
			$(this).click(function() {
				cookieValue = {'status' : 'edit', 'userID' : FP_USER_ID};
				$.SetCookie(cookieName, cookieValue, cookieOptions);
				window.open(FP_GALLERY_URL, '_blank');
			});
		});
	}

	// Disable editor on this page only...it will reactivate if you go to another page
	function cssEditDisable (t) {
		$(t).unbind('mouseover').unbind('mouseout').unbind('click');
		// don't turn off cookie, because we want to be able to
		// click a link to next page and have editor again
		// $.SetCookie(cookieName, '', cookieOptions);
		$('#cssEditSwitch').text(text_turn_editor_on).addClass('cssEditSwitchLight');//.css('background-color', 'orange');
		//alert ("The editor is temporarily off. If you go to a new page, you will lose your changes unless you save.");
	}

	function cssEditReenable (t) {
		showConsole ('Reenable for '+t);
		$(t).cssEditBindEditor().cssEditBindHighlighter();
		cookieValue.status = 'edit';
		$.SetCookie(cookieName, cookieValue, cookieOptions);
		$('#cssEditSwitch').text(text_turn_editor_off).removeClass('cssEditSwitchLight');
		showMessage ("The editor is on.");
	}

	// Make something into a switch for cssEdit
	// Switch could be a checkbox, a button, or an anchor
	// HOWEVER, we haven't made it so a change to shows on all switches!
	$.fn.cssEditSwitch = function () {
		return this.each(function() {
			$(this).filter('a').toggle(
				function() {
					$(this).html('On');
					cookieValue.status = 'edit';
					$.SetCookie(cookieName, cookieValue, cookieOptions);
				},
				function() {
					$(this).html('Off');
					cookieValue.status = 'off';
					$.SetCookie(cookieName, cookieValue, cookieOptions);
				}
				)
			.end()
			.filter(':checkbox').bind('change',
				function() {
					if ($(this).is(":checked")) {
						cookieValue.status = 'edit';
					$.SetCookie(cookieName, cookieValue, cookieOptions);
					} else {
						cookieValue.status = 'off';
						$.SetCookie(cookieName, cookieValue, cookieOptions);
					}
				})
			.end()
			.filter(':button').toggle(
				function() {
					$(this).text('On');
					cookieValue.status = 'edit';
					$.SetCookie(cookieName, cookieValue, cookieOptions);
				}, function() {
					$(this).text('Off');
					cookieValue.status = 'off';
					$.SetCookie(cookieName, cookieValue, cookieOptions);
				});
		});
	}

	// #mark * Open Editor
	// Fill in and open Dialog Box Editor
	// Regarding pseudo classes:
	// - you must build the style so the links are of the form a.class.class:hover
	$.fn.cssEditBindEditor = function() {
		return this.unbind('click').click(function(event) {
			var myName = $.getCssName(this);
			var myTag = this.tagName;

			var obj = $(myName);
			var form = $('#cssEdit-form').clearForm();
			var n, v, vs;

			var target = this;
			if ($(target).attr('id') == "page") {
				target = $('body');
				myName = 'body';
				myTag = 'body';
				obj = $(myName);
				//showError ('cssEditBindEditor: Modifying PAGE');
			}

			showConsole ("cssEditBindEditor: myName="+myName);
			showConsole ("cssEditBindEditor: tagname="+myTag);

			$(target).cssEditHighlight(false);
			$(':input[name=cssEdit-theme-id]').val(cssEditThemeID);
			$('#cssEdit-dialog-header').html(myName);
			$('#cssEdit-dialog-subheader').html('Theme: &ldquo;'+cssEditThemeName+'&rdquo;');
			$(':input[name=cssEdit-css-name]').val(myName);

			// If this is link, show link colors, else show only text color
			if (myTag == "A") {
				$('#cssEdit-linkcolors').show();
				$('#cssEdit-textcolor').hide();
				showConsole ("Yes, this is a link (tage=A)");
			} else {
				$('#cssEdit-linkcolors').hide();
				$('#cssEdit-textcolor').show();
			}

			// Fill in form from object css values
			$(':input[name^=cssEdit-]').each(function() {
				if ((myTag == "A") && this.name.match('cssEdit-a')) {
					n = this.name.replace('cssEdit-a-', '');
					showConsole ("cssEditBindEditor: look for a"+myName+":"+n)
					r = FindCSSRule("a"+myName+":"+n);
					if (r) {
						v = r.style.color;
					} else {
						v = "";
					}
					showConsole ("cssEditBindEditor: linkname="+"a"+myName+":"+n+" : "+v);
				} else {
					n = this.name.replace('cssEdit-', '');
					// Borders are weird. There is no border-color, only border-top-color, etc.
					// So, let's just use the top color
					if (n == 'border-width' || n=='border-color') {
						v = obj.css(n.replace(/-/,'-top-'));
					} else {
						v = obj.css(n);
					}
					//showConsole ("cssEditBindEditor: Fetch value of " + myName + ":" + n + ":" + v);
					if (n == "color") {
						showConsole ("cssEditBindEditor: color="+myName+" color="+" : "+v);
					}
				}
				v = $.trim(v);
				// convert colors to hex
				if (v.substr(0,3) == 'rgb') {
					v = $.toHex(v);
				}

				if (v)
					$(':input[name='+this.name+']').val(v);
			});

			// Let's limit what you can modify when you choose to edit 'body'
			// to only "background-color"

			// Activate the color picker
			$(iColorPicker());

			// Set an 'undo' point for cancel
			//$(myName).cssEditMark();
			//$('#error').empty();

			if (myName == "body") {
				$('.cssEdit-dialog-section, #cssEdit-form h4, #cssEdit-form p').not('.cssEdit-dialog-bodyOK').hide();
			} else {
				$('.cssEdit-dialog-section, #cssEdit-form h4, #cssEdit-form p').show();
			}

			$('#dialog').dialog('open');

			showConsole ("cssEditBindEditor: END ---------------");
			return false;
		});
	}


	// Apply CSS values in jQuery dialog object to an object
	// Accepts a set of name/value, what you'd get from a form like this
	// $(':input)
	// Also pushes the previous version onto the undo queue
	// If a name has an underscore "_" it's translated to a colon, ":",
	// so we can have a:hover written as a_hover
	$.fn.cssEditApply = function (targetElement) {
		var myName, myTag, obj, r;
		return this.each(function() {
			var target = this;
			if ($(target).attr('id') == "page") {
				target = $('body');
				myName = 'body';
				myTag = 'body';
				obj = $(myName);
			}

			var n = target.name;
			var v = $(target).val();

			if (v != "") {
				// If the value is a child anchor (a link belonging to this class)
				// Then we change the link color values.


				// If this is an anchor value (a pseudo class) we have to parse
				if (n.match('cssEdit-a')) {
					n = n.replace('cssEdit-a-', '');
					// Add a new DOM stylesheet rule. This is the only way to modify a pseudo element.
					// icp is the id of the color preview box
					s = 'color';

					// Get the current CSS rule to save it
					prev = FindCSSRule("a" + targetElement + ":" + n);

					var r = {
						'type': 'style',
						'tag': targetElement,
						'name': n,
						'style': s,
						'icp' : '#icp_link_preview_cssEdit-a-' + n,
						'value': prev.style.color
					};

					$.cssTextColor(targetElement, n, s, v);
					$(targetElement).cssEditPush(r);
					// Record this change to our global changes object
					rr = {
						'selector' 	: 'a'+targetElement+':'+n,
						'declaration' : {
							'property':s,
							'value':v
						}
					};
					$.cssEditAddChange (rr);

				} else {
					n = n.replace('cssEdit-', '').replace('_',':');
					// Save original css and set the style of the DOM element
					var oldV = $(targetElement).css(	n);
					// convert colors to hex
					if (oldV != null && oldV.substr(0,3) == 'rgb') {
						oldV = $.toHex(oldV);
					}

					r = {
						'type': 'attribute',
						'tag': targetElement,
						'name': n,
						'icp' : '#icp_link_preview_cssEdit-' + n,
						'value': oldV
					};
					//$(targetElement).cssEditPush(n).css(n, v);
					$(targetElement).cssEditPush(r).css(n, v);

					// Record this change to our global changes object
					rr = {
						'selector' 	: targetElement,
						'declaration' : {
							'property':n,
							'value':v
						}
					};
					$.cssEditAddChange (rr);

				//showConsole("cssEditApply: Set " + targetElement + " {" + n + " : " + oldV + '==>' + v + "}");
				}

			}
		});
	}

	// #mark * Add Changes to list
	// Add a style rule to the list of changes.
	// We can save this this to create a style sheet which reflects all changes
	// made by the user.
	// Save to global variable, "cssChanges"
	// Note: CSS is made up of rules. Each rule has a selector (e.g. h1) and a declaration (e.g. color:red).
	// A declaration has two parts: a property ('color') and a value ('red').
	$.cssEditAddChange = function (rule) {
		var s,p,v;
		s = rule.selector;
		p = rule.declaration.property;
		v = rule.declaration.value;
		// See if selector exists in the array
		var offset = null;
		for (var i in cssChanges.rules) {
			if ( s+p == cssChanges.rules[i].selector + cssChanges.rules[i].declaration.property ) {
				offset = i;
				break;
			}
		}
		if (offset == null)
			offset = cssChanges.rules.length;

		cssChanges.rules[offset] = {
			'selector' : s,
			'declaration' : rule.declaration
			};
		showConsole ("cssEditAddChange: ("+offset+") "+s+" {"+p+":"+v+";}");
	}

	// #mark * Save Changes with AJAX
	// Save changes from the "cssChanges" variable
	// "cssChanges" is an object:
	//   cssChanges[selector].declaration
	//   declaration.property, declaration.value
	$.cssEditSaveChanges = function(saveAs) {
		var i,t,r;
		var nosave = false;
		
		for (i in cssChanges.rules) {
			cleani = i.replace ("/\W/", "__");

			r = cssChanges.rules[cleani];
			// 		showConsole ('name:'+i);
			// 		showConsole ('selector:'+r.selector);
			// 		showConsole ('declaration.property:'+r.declaration.property);
			// 		showConsole ('declaration.value:'+r.declaration.value);
			t = r.selector + " {" + r.declaration.property + ":" + r.declaration.value + ";}";
			showConsole (t);

		}
		cssChanges.command = 'save';
		cssChanges.projectid = FP_PROJECT_ID;	// global var declared on the master page
		cssChanges.groupid = FP_GROUP_ID;	// global var declared on the master page
		//cookieValue = $.GetCookie(cookieName);	// refresh the cookie value
		cssChanges.userid = cookieValue.userID;	// get the user from the cookie

		var encoded = $.toJSON(cssChanges);

		// If the theme ID isn't a variation, e.g. "default", then
		// create a new variation.
		if (!IsVariation(cssEditThemeID) || saveAs) {
			showConsole ("Theme ID: "+cssEditThemeID);
			cssChanges.newname = prompt ("Enter a new name for this theme variation:", "My Variation");
			if (!cssChanges.newname) {
				alert ("You did not enter a name, so nothing was saved.");
				return false;
			}
		} else {
			// don't alert...we'll know from the AJAX response.
			//alert ('Save as '+cssEditThemeName);
		}

		showConsole ('JSON Encoded: '+encoded);

		var dataString = JSON.stringify(cssChanges);
		/*
		$.post('variation_update.php', {data: dataString}, 
			function(res) {
				showResult(res);
				alert ("jquery.cssedit.js, line 492: Wee!");
			}, "text");
		*/
		$.post('variation_update.php', 
			{data: dataString}, 
			showResult,
			"text");


		return (!nosave);
	}

	$.cssEditDeleteVariation = function() {
		cssChanges.command = 'delete';
		cssChanges.userid = cookieValue.userID;	// get the user from the cookie
		var dataString = JSON.stringify(cssChanges);
		$.post('variation_update.php', 
			{data: dataString},
			showResult, "text");
	}

// Show result of AJAX submit using JSON
	function showResult(res) {
		res = JSON.parse(res);
		res = res.replace(/<br>/gi,"\n");
		var obj = {
			'result' : res
		};
		alert (obj.result);
		// Reload is part of post-processing function, or it will come to quickly
		// and the database won't be in sync with the changes!
		window.location.reload();
	}

	function AppendToBody(res) {
		$('body').append(res);
	}

	// Push a current CSS value of a set of elements onto a stack for retrieval (stack is unique to the set of elements)
	// e.g. save the font-size for all elements of class 'header'
	$.fn.cssEditPush = function (styleObj) {
		return this.each(function() {
			var myStack = $.data(this, "cssStack");

			if (myStack == undefined)
				myStack = [];

			myStack.push(styleObj);
			$.data(this, "cssStack", myStack);
			myStack = $.data(this, "cssStack");
		});
	}



	// Restore a CSS value from the stack for a given set
	$.fn.cssEditUndo = function () {
		var myName = this.selector;
		var icp;
		return this.each(function() {
			var myStack = $.data(this, "cssStack");

			if ((myStack != undefined) && (myStack.length > 0)) {
				popMe = myStack.pop();
				if (popMe.type == 'style') {
					showConsole ("Undo: Restore style "+popMe.tag+":"+popMe.name+" : "+popMe.name + "=" + popMe.value);
					//document.styleSheets[popMe.stylesheet].deleteRule(popMe.rule);
					$.cssTextColor(popMe.tag, popMe.name, popMe.style, popMe.value);
					icp = $('#icp_link_preview_cssEdit-a-'+popMe.name);
					if (icp.size() > 0) {
						icp.css('background-color',popMe.value);
					}

					// If we undo a "link", we should also undo the "visited" the corresponds to it.
					if (popMe.name == 'link')
						$.cssTextColor(popMe.tag, 'visited', popMe.style, popMe.value);

				} else {
					showConsole ("Undo: Restore attribute for "+myName+" "+popMe.name + "=" + popMe.value);
					// restore preview color box if it exists
					icp = $('#icp_link_preview_cssEdit-'+popMe.name);
					if (icp.size() > 0) {
						icp.css('background-color',popMe.value);
					}
					$(this).css(popMe.name, popMe.value);
				}
				// reset editing form item
				$(':input[name=cssEdit-'+popMe.name+']').val(popMe.value);
				showMessage('Undo: Reset '+'cssEdit-'+popMe.name+' to '+popMe.value);
				$.data(this, "cssStack", myStack);
			} else {
				showMessage("No more undo's for "+myName);
			}
		});
	}


	// Mark current state of stack so we can go backward multiple pops
	// to restore current state
	$.fn.cssEditMark = function () {
		return this.each(function() {
			myName = $.data(this);
			// fetch stack
			var myStack = $.data(this, "cssStack");
			if (myStack != undefined) {
				top = myStack.pop();

				t = $(top).css('font-size');
				$('#error').append("EditMark: Set mark at font size = " + t + "<BR>");

				top.cssEditMarker = true;
				myStack.push(top);
				// store stack
				$.data(this, "cssStack", myStack);
			}
		});
	}

	// Undo until previous marker
	$.fn.cssEditUndoToMark = function() {
		return this.each(function() {
			myName = $.data(this);
			// fetch stack
			var myStack = $.data(this, "cssStack");
			if (myStack != undefined) {
				marked = false;
				while (!marked) {
					prevElement = myStack.pop();
					if (prevElement != null) {
						marked = prevElement.cssEditMark;
					} else {
						marked = true;
					}
					t = $(prevElement).css('font-size');
					$('#error').append("UndoToMark: Restored font size = " + t + "<BR>");
				}
				$(this).replaceWith(prevElement).data(prevElement, "cssStack", myStack);
			} else {
				alert (myName + "Cannot undo any more...this is the original state.");
			}
		});
	}


	// Dialog box editor
	$.cssEditSetupEditor = function () {
		var r;
		// Make space for editor info and controls
		$('body').addClass('cssEdit')
		.append('<div style="display:block;"><div id="dialog" title="Style Editor"><form id="cssEdit-form" action="variation_update.php"><div class="cssEdit-dialog-section cssEdit-dialog-bodyOK"><div id="cssEdit-dialog-header"></div><input name="cssEdit-theme-id" type="hidden" readonly><div id="cssEdit-dialog-subheader"></div><input id="cssEdit-css-name" name="cssEdit-css-name" type="hidden" readonly></div><h4 class="cssEdit-dialog-bodyOK">Box</h4><div class="cssEdit-dialog-section cssEdit-dialog-bodyOK"><p class="cssEdit-dialog-bodyOK">Background Color: <input id="cssEdit-background-color" name="cssEdit-background-color" class="iColorPicker " type="text" size="11"></p><p>Text Alignment: <select id="cssEdit-text-align" name="cssEdit-text-align"><option label="None" value="none"></option><option label="Left" value="left"></option><option label="Right" value="right"></option><option label="Center" value="center"></option></select></p><p>Opacity: <input id="cssEdit-opacity" name="cssEdit-opacity" type="text" size="4"> (0.0 - 1.0)</p></div><h4>Font</h4><div class="cssEdit-dialog-section"><div id="cssEdit-linkcolors"><p>Font Color: <input id="cssEdit-a-link" name="cssEdit-a-link" class="iColorPicker" type="text" size="11"><br>Hover Color: <input id="cssEdit-a-hover" name="cssEdit-a-hover" class="iColorPicker" type="text" size="11"><br>Clicked Color: <input id="cssEdit-a-active" name="cssEdit-a-active" class="iColorPicker" type="text" size="11"></p></div><p id="cssEdit-textcolor">Font Color: <input id="cssEdit-color" name="cssEdit-color" class="iColorPicker" type="text" size="11"></p><p id="cssEdit-text-shadow">Text Shadow: <select name="cssEdit-text-shadow"><option value="none">No Shadow</option><option value="rgb(0, 0, 0) 0px 0px 1px">Black - 1 pixel</option><option value="rgb(136, 136, 136) 0px 0px 1px">Gray - 1 pixel</option><option value="rgb(255, 255, 255) 0px 0px 1px">White - 1 pixel</option><option value="rgb(0, 0, 0) 0px 0px 2px">Black - 2 pixel</option><option value="rgb(136, 136, 136) 0px 0px 2px">Gray - 2 pixel</option><option value="rgb(255, 255, 255) 0px 0px 2px">White - 2 pixel</option><option value="rgb(0, 0, 0) 1px 1px 3px">Black - 3 pixel</option><option value="rgb(136, 136, 136) 1px 1px 3px">Gray - 3 pixel</option><option value="rgb(255, 255, 255) 1px 1px 3px">White - 3 pixel</option><option value="rgb(0, 0, 0) 1px 1px 5px">Black - 5 pixel</option><option value="rgb(136, 136, 136) 1px 1px 5px">Gray - 5 pixel</option><option value="rgb(255, 255, 255) 1px 1px 5px">White - 5 pixel</option><option value="rgb(0, 0, 0) 1px 1px 8px">Black - 8 pixel</option><option value="rgb(136, 136, 136) 1px 1px 8px">Gray - 8 pixel</option><option value="rgb(255, 255, 255) 1px 1px 8px">White - 8 pixel</option><option value="rgb(0, 0, 0) 1px 1px 10px">Black - 10 pixel</option><option value="rgb(136, 136, 136) 1px 1px 10px">Gray - 10 pixel</option><option value="rgb(255, 255, 255) 1px 1px 10px">White - 10 pixel</option></select><br><small>Text shadow current works only with Safari, FireFox 3.5, Chrome.</small></p><p>Font Size: <select name="cssEdit-font-size"><option value="8px">8px</option><option value="9px">9px</option><option value="10px">10px</option><option value="11px">11px</option><option value="12px">12px</option><option value="14px">14px</option><option value="16px">16px</option><option value="18px">18px</option><option value="22px">22px</option><option value="24px">24px</option><option value="28px">28px</option><option value="32px">32px</option><option value="36px">36px</option><option value="40px">40px</option><option value="56px">56px</option><option value="64px">64px</option><option value="72px">72px</option><option value="96px">96px</option></select></p><p>Font: <select name="cssEdit-font-family"><option value="Arial, Helvetica, sans-serif">Arial</option><option value="\'Arial Black\', Gadget, sans-serif">Arial Black</option><option value="\'Comic Sans MS\', cursive">Comic Sans MS</option><option value="\'Courier New\', Courier, monospace">Courier New</option><option value="Georgia, serif">Georgia</option><option value="Impact, Charcoal, sans-serif">Impact</option><option value="\'Lucida Console\', Monaco, monospace">Lucida Console</option><option value="\'Lucida Sans Unicode\', \'Lucida Grande\', sans-serif">Lucida Sans Unicode</option><option value="\'Palatino Linotype\', \'Book Antiqua\', Palatino, serif">Palatino Linotype</option><option value="Tahoma, Geneva, sans-serif">Tahoma</option><option value="\'Times New Roman\', Times, serif">Times New Roman</option><option value="\'Trebuchet MS\', Helvetica, sans-serif">Trebuchet MS</option><option value="Verdana, Geneva, sans-serif">Verdana</option><option value="Symbol">Symbol</option><option value="Webdings">Webdings</option><option value="Wingdings,&nbsp;Zapf&nbsp;Dingbats (Wingdings,&nbsp;Zapf&nbsp;Dingbats)">Wingdings</option><option value="\'MS Sans Serif\', Geneva, sans-serif">MS Sans Serif</option><option value="\'MS Serif\', \'New York\', serif">MS Serif</option></select></p><p>Font-weight: <select name="cssEdit-font-weight"><option value="normal">normal</option><option value="bold">bold</option></select></p><p>Font-style: <select name="cssEdit-font-style"><option value="normal">normal</option><option value="italic">italic</option></select></p><p>Text-transform: <select name="cssEdit-text-transform"><option value="none">none</option><option value="uppercase">uppercase</option><option value="lowercase">lowercase</option><option value="capitalize">capitalize</option></select></p><p>Text-decoration: <select name="cssEdit-text-decoration"><option value="none">none</option><option value="underline">underline</option><option value="line-through">line-through</option><option value="overline">overline</option></select></p><p>Letter-spacing: <select name="cssEdit-letter-spacing"><option value="0.00em">none</option><option value="0.05em">0.05em</option><option value="0.10em">0.10em</option><option value="0.15em">0.15em</option><option value="0.25em">0.25em</option><option value="0.50em">0.50em</option><option value="0.75em">0.75em</option><option value="1.00em">1.00em</option><option value="-0.05em">-0.05em</option><option value="-0.10em">-0.10em</option><option value="-0.15em">-0.15em</option><option value="-0.25em">-0.25em</option><option value="-0.50em">-0.50em</option><option value="-0.75em">-0.75em</option><option value="-1.00em">-1.00em</option></select></p><p>Line-height: <select name="cssEdit-line-height"><option value="0.5em">0.5em</option><option value="0.75em">0.75em</option><option value="1.0em">1.0em</option><option value="1.25em">1.25em</option><option value="1.50em">1.50em</option><option value="1.75em">1.75em</option><option value="2.00em">2.00em</option><option value="2.5em">2.5em</option><option value="3.0em">3.0em</option><option value="4.0em">4.0em</option><option value="5.0em">5.0em</option><option value="7.0em">7.0em</option><option value="10em">20em</option></select></p></div><h4>Dimensions</h4><div class="cssEdit-dialog-section"><p>Width: <input name="cssEdit-width" type="text" size="6"> Height: <input name="cssEdit-height" type="text" size="6"></p></div><h4>Margins</h4><div class="cssEdit-dialog-section"><p>Top: <input name="cssEdit-margin-top" type="text" size="6"> Bottom: <input name="cssEdit-margin-bottom" type="text" size="6"></p><p>Left: <input name="cssEdit-margin-left" type="text" size="6"> Right: <input name="cssEdit-margin-right" type="text" size="6"></p><h4>Padding</h4><p>Top: <input name="cssEdit-padding-top" type="text" size="6"> Bottom: <input name="cssEdit-padding-bottom" type="text" size="6"></p><p>Left: <input name="cssEdit-padding-left" type="text" size="6"> Right: <input name="cssEdit-padding-right" type="text" size="6"></p></div><h4>Border</h4><div class="cssEdit-dialog-section"><p>Border Color: <input id="cssEdit-border-color" name="cssEdit-border-color" class="iColorPicker" type="text" size="11"></p><p>Border Width: <input name="cssEdit-border-width" id="cssEdit-border-width" type="text" size="5"></p><p>Top: <select id="cssEdit-border-top-style" name="cssEdit-border-top-style"><option label="None" value="none"></option><option label="Hidden" value="hidden"></option><option label="Solid" value="solid"></option><option label="Dotted" value="dotted"></option><option label="Dashed" value="dashed"></option><option label="Double" value="double"></option></select> Bottom: <select id="cssEdit-border-bottom-style" name="cssEdit-border-bottom-style"><option label="None" value="none"></option><option label="Hidden" value="hidden"></option><option label="Solid" value="solid"></option><option label="Dotted" value="dotted"></option><option label="Dashed" value="dashed"></option><option label="Double" value="double"></option></select><br>Left: <select id="cssEdit-border-left-style" name="cssEdit-border-left-style"><option label="None" value="none"></option><option label="Hidden" value="hidden"></option><option label="Solid" value="solid"></option><option label="Dotted" value="dotted"></option><option label="Dashed" value="dashed"></option><option label="Double" value="double"></option></select> Right: <select id="cssEdit-border-right-style" name="cssEdit-border-right-style"><option label="None" value="none"></option><option label="Hidden" value="hidden"></option><option label="Solid" value="solid"></option><option label="Dotted" value="dotted"></option><option label="Dashed" value="dashed"></option><option label="Double" value="double"></option></select><br></p></div></form></div><div id="dialogcontent"></div><div id="cssEdit-results" class="ui-dialog ui-widget ui-widget-content ui-corner-all undefined ui-draggable ui-resizable" title="Results"></div></div>');
		$('#foot').addClass('cssEdit');
		
		var controlbar = '<div id="cssEditTopBar">';
		controlbar += '<span id="cssEditEditorTitle">Theme Editor</span>';
		controlbar += '<div id="cssEditControls">';
		controlbar += 'Theme: <span id="csseditchangetheme"></span>';
		controlbar += ' <input id="cssEditHighlightFlag" type="checkbox"> Highlighting ';
		controlbar += ' <button type="button" id="cssEditSave">Save</button>';
		controlbar += ' <button type="button" id="cssEditSaveAs">Save As...</button>';
		controlbar += ' <button type="button" id="cssEditRevert">Revert To Saved</button>';
		controlbar += ' <button type="button" id="cssEditDelete">Delete Variation</button>';
		controlbar += ' <button type="button" id="cssEditSwitch">'+text_turn_editor_off+'</button>';
		controlbar += ' <button type="button" id="cssEditClose"> Close Editor </button>';
		//controlbar += ' <button type="button" id="cssEditFlag">?</button>';
		controlbar += '</div></div>';
		controlbar += '<div id="cssEditFlag"></div>';

		$('body').append(controlbar);

		// Activate the save button
		$('#cssEditSave').click(function()
			{$.cssEditSaveChanges()}
		);

		// Activate the save as button
		$('#cssEditSaveAs').click(function()
			{$.cssEditSaveChanges(true)}
		);

		// Activate the delete button
		$('#cssEditDelete').click(function()
			{$.cssEditDelete();}
		);

		// Activate the Revert to Saved button
		$('#cssEditRevert').click(function()
			{$.cssEditRevertToSaved();}
		);

		// Insert the popup of themes to change to
		var datastring = JSON.stringify({'themeid':cssEditThemeID, 'userid' : cookieValue.userID});
		$.post('themelist.php', 
			{data: datastring},
			function (res) {
				$('#csseditchangetheme').html(res);
				},
			"text"
		);


		if (cssEditShowConsole) {
			$('body').append('<div id="cssEditConsole"></div>')
		}


		// Make sure popup dialog doesn't actually submit
		$('#cssEdit-form').bind('submit', function() {
			return false;
		});

		// Bind dialog checkbox switches to their fields
		$(':input[name^=dialog-cssEdit-switch]').each(function() {
			target = 'cssEdit-' + $(this).attr('name').replace('dialog-cssEdit-switch-','');
			v = this.val();
			$(this).change(function() {
				if ($(this).is(":checked")) {
					$(':input[name='+target+']').val(v);
				} else {
					$(':input[name='+target+']').val('');
				}
			});
		});

		$('#cssEditClose').click(function () {
			CloseEditor();
		});


		// Bind dialog box to #dialog definition
		$('#dialog').
		dialog({
			bgiframe: true,
			autoOpen: false,
			height: 500,
			modal: false,		// we DON'T want the background to darken...we can't see it then!
			position: 'top',
			stack: true,
			buttons: {
				/*
				Cancel: function() {
					// Undo changes of the open dialog
					targetName = $('#cssEdit-css-name').val();
					$(targetName).cssEditUndoToMark();
					$("#iColorPicker").fadeOut();
					$(this).dialog('close');
				},
				*/
				'Undo': function() {
					// Undo changes of the open dialog
					targetName = $('#cssEdit-css-name').val();
					$(targetName).cssEditUndo();
				},
				/*
				'Save': function() {
					// save changes to css style sheet
					r = $.cssEditSaveChanges();
					if (r) {
						$("#iColorPicker").fadeOut();
						$(this).dialog('close');
					}
				},

				'Revert to Saved': function() {
					// undo ALL changes this session - restore user sheet
					targetName = $('#cssEdit-css-name').val();
					$(this).dialog('close');
					window.location.reload();
				},

				'Delete Variation': function() {
					// undo ALL changes - delete the_user style sheet
					r = $.cssEditDelete();
				},
				*/
				'Close': function() {
					// close dialog without saving to theme file but keeping session changes
					$("#iColorPicker").fadeOut();
					$(this).dialog('close');
				}
			}
		});

		// Show changes as you edit them!
		$(':input[name^=cssEdit]').change(function() {
			targetName = $('#cssEdit-css-name').val();
			$(this).cssEditApply(targetName);
		});

	}

	// Delete all changes with dialog box of the current theme ( which is cssEditThemeID)
	$.cssEditDelete = function () {
		// undo ALL changes - delete the_user style sheet
		if (IsVariation(cssEditThemeID)){
			var r  =confirm ('Delete the theme variation, "'+cssEditThemeName+'"?\nIf you do this, all pages using this theme will revert to the system version of '+cssEditThemeName+', if it exists, or to the "'+cssEditDefaultTheme+'" theme, if no system version exists.');
			if ( r == true ) {
				$.cssEditDeleteVariation();
				$(this).dialog('close');
				alert ("Deleted the theme variation, "+cssEditThemeName+".")
			}
		} else {
			alert ("You cannot delete the theme, "+cssEditThemeName+".")
		}
		return r;
	}

	// undo ALL changes this session - restore user sheet
	$.cssEditRevertToSaved = function () {
		var r  =confirm ('Revert to the last saved version of "'+cssEditThemeName+'"?\nYou will lose all changes you made before you last clicked, "Save".');
		if ( r == true ) {
			window.location.reload();
		}
	}


	// Flash or outline or whatever the object, so user can see it when chosen
	// The clickable edit-me flag is important, because it allows us to edit links
	$.fn.cssEditHighlight = function (showme) {
		if ($('#cssEditHighlightFlag').is(":checked") ) {
			var myName = $.getCssName($(this));
			//showConsole("cssEditHighlight:"+myName);

			// remove any other highlights
			$('.cssEditHighlight').not(this).removeClass('cssEditHighlight');

			// Hilight all affected instances
			// Toggling should work, but is unreliable because mouseover is unreliable with nested objects!
			if (showme == undefined) {
				$(myName).not('.nohilight').toggleClass('cssEditHighlight');
			} else {
				if (showme) {
					$(myName).not('.nohilight').addClass('cssEditHighlight');
				} else {
					$(myName).not('.nohilight').removeClass('cssEditHighlight');
				}
			}
		}
		return this;
	}


	// Popup a little flag to click on to get to the editor dialog
	$.fn.cssEditFlag = function (showme, mouseX, mouseY) {
		var myName = $.getCssName($(this));
		//var obj = $(this);
		//return this.each(function() {
		var text = "<span class='cssEditButtonText'>Edit "+myName+"</span>";
		$('#cssEditFlag').html(text);
		//h = $('#cssEditFlag').height();
		//pos = $(obj).cssEditPosition();
		var pos = { top: mouseY, left:mouseX+20 };
		//$('#cssEditFlag').addClass(this.attr('class')).appendTo(this).css(pos);
		$('#cssEditFlag').css(pos);
		if (showme) {
			$('#cssEditFlag').show();
		} else {
			$('#cssEditFlag').hide();
		}
		//});
		return this;
	}

   
   
	// Return the CSS name of an object
	// Of the form, myID#class.class.class
	// Useful for getting wrapped items
	$.getCssName = function (obj) {
		var j = "";
		var myID = "";
		var myTag = "";
		obj = $(obj);

		if (obj.attr('id'))
			myID = '#' + obj.attr('id');

		if (myID == "#page") {
			myID = "";
			obj = $('body');
			myTag = 'body';
		}

		// separate class with a period
		var myClass = $.trim(obj.attr('class').replace("cssEditHighlight", "").replace("cssEdit","").replace("editable", "").replace("hover", "").replace("menuonly", "").replace("clickable", "") ).split(' ').join('.');

		if (myClass)
			j = ".";

		var myName = $.trim(myTag + myID + j + myClass);
		return myName;
	}

	// Convert a color of the form, "rgb(x,y,z)" to hex, "#aabbcc"
	// Convert rgba values to rgb by taking the first 3 values and dumping the
	// transparency. An rgbA value of 0,0,0,0 is transparent, we return "transparent"
	// for such a value (which is what Safari gives)
	$.toHex = function (rgbString) {
		var parts = rgbString.match(/^rgb[a]?\((\d+),\s*(\d+),\s*(\d+)(?:,\s*(\d+))?\)$/);
		// parts now should be ["rgb(0, 70, 255", "0", "70", "255"]
		// rgba fourth part = 0 means "transparent"
		if (parts && (parts.length > 3) && (parts[4] == '0')) {
			return "transparent";
		} else {
			if (parts) {
				delete(parts[0]);
				for (var i = 1; i <= 3; ++i) {
					parts[i] = parseInt(parts[i]).toString(16);
					if (parts[i].length == 1) parts[i] = '0' + parts[i];
				}
				var hexString = '#' + parts.join('');
				// "0070ff"
				return hexString;
			}
		}
		return rgbString;
	}


	/*
// Old version
// convert a color of the form, "rgb(x,y,z)" to hex, "#aabbcc"
$.XtoHex = function (rgbString) {
	var parts = rgbString.match(/^rgb[a?]\((\d+),\s*(\d+),\s*(\d+)\)$/);
	// parts now should be ["rgb(0, 70, 255", "0", "70", "255"]
	if (parts) {
		delete(parts[0]);
		for (var i = 1; i <= 3; ++i) {
			parts[i] = parseInt(parts[i]).toString(16);
			if (parts[i].length == 1) parts[i] = '0' + parts[i];
		}
		var hexString = '#' + parts.join('');
		// "0070ff"
		return hexString;
	}
	return rgbString;
}
*/

	function showError (msg) {
		$('#error').html(msg);
	}

	function showMessage (msg) {
		$('#message').html(msg);
	}

	function showConsole (msg) {
		if (cssEditShowConsole) {
			$('#cssEditConsole').prepend(msg+'<br>');
		}
	}

	// Taken from btPosition, from the bt plugin
	$.fn.cssEditPosition = function() {
		function num(elem, prop) {
			return elem[0] && parseInt(jQuery.curCSS(elem[0], prop, true), 10) || 0
		}
		var left = 0,
		top = 0,
		results;
		if (this[0]) {
			var offsetParent = this.offsetParent(),
			offset = this.offset(),
			parentOffset = /^body|html$/i.test(offsetParent[0].tagName) ? {
				top: 0,
				left: 0
			}: offsetParent.offset();
			offset.top -= num(this, "marginTop");
			offset.left -= num(this, "marginLeft");
			parentOffset.top += num(offsetParent, "borderTopWidth");
			parentOffset.left += num(offsetParent, "borderLeftWidth");
			results = {
				top: offset.top - parentOffset.top,
				left: offset.left - parentOffset.left
			}
		}
		return results
	};

	// Replace one element with another
	$.fn.replaceWith = function(o) {
		return this.after(o).remove();
	};


	function cssStyle (name, value) {
		this.name = name;
		this.value = value;
		this.type = "element";
	}

	$.fn.cssEditMakeEditable = function () {
		return this.each(function() {
			showConsole ("Made editable: "+$(this).attr('class'));
			$(this).addClass('editable');
		});
	}


	// Assign color text by changing the style sheets, not the DOM elements.
	// This allows us to set text color and pseudo classes, e.g. :hover or :active.
	// This can't be done with $.css, so we'll do it here.
	// If you try, $(selector).css('color','red'), then both the text
	// and the pseudo classes will be red.
	// It probably doesn't work with all browsers!
	// This must be done only after the style sheets have loaded,
	// so it should be called with window.onload
	$.cssTextColor = function (myName, pseudoClass, stylename, value) {
		//showConsole('<hr>');

		// Apply 'link' value to 'visited'
		if (pseudoClass == "link")
			$.cssTextColor(myName, "visited", stylename, value);

		if (pseudoClass != '') {
			n = 'a'+myName + ":"+pseudoClass;
		} else {
			n = myName;
		}


		//showConsole('cssTextColor: '+n+' {'+stylename+':'+value+"}");
		r = FindCSSRule(n);
		if (r.style == undefined) {
			var newrule = n + "{"+stylename+":"+value+";}";
			l = (document.styleSheets.length);
			var oLength = document.styleSheets[l-1].cssRules.length;
			document.styleSheets[0].insertRule(newrule,oLength); //add a new rule at the end
			var r = document.styleSheets[0].cssRules[oLength]; //reference the new rule we just added
			r = FindCSSRule(n);
		//showConsole('cssTextColor:...Created rule : '+newrule);
		}
		//showConsole('cssTextColor...Current value : '+r.style[stylename]);
		r.style[stylename] = value;
		//showConsole('cssTextColor...New value : '+r.style[stylename]);
		return this;
	}

	// Assign values to css pseudo classes, such as :hover or :active
	// This can't be done with $.css, so we'll do it here.
	// It probably doesn't work with all browsers!
	// This must be done only after the style sheets have loaded,
	// so it should be called with window.onload
	// This should work like .css(name, value);
	$.setCSSPseudo = function (rule, pseudo, stylename, value) {
		n = "a" + rule + ":" + pseudo;
		//showConsole ('setCSSPseudo: Change rule '+ n +' to '+stylename+':'+value);
		r = FindCSSRule(n);
		if (r != null) {
			r.style[stylename] = value;
		}
	}


	// Assign values to css pseudo classes, such as :hover or :active
	// This can't be done with $.css, so we'll do it here.
	// It probably doesn't work with all browsers!
	// This must be done only after the style sheets have loaded,
	// so it should be called with window.onload
	// This should work like .css(name, value);
	$.fn.csspseudo = function (pseudo, stylename, value) {
		var myname;
		return this.each(function() {
			myname = $.getCssName(this);
			n = "a" + myname + ":" + pseudo;
			//showConsole ('csspseudo: Change '+n+' to '+stylename+':'+value);
			r = FindCSSRule(n);
			if (r != null) {
				r.style[stylename] = value;
			}
		});
	}

	// Find rule in css sheets, then return the object
	function FindCSSRule (myRule) {
		var myrules, mysheet, h, i, ss;
		var targetrule = null;

		myRule.toLowerCase();
		//showConsole('FindCSSRule:'+myRule);
		for (var ss in document.styleSheets) {
			if (!isNaN(ss) ) {
				mysheet=document.styleSheets[ss];
				// Is this one of our stylesheets?
				h = String(mysheet.href);
				if ( h && (h.search(/_themes/i) != -1) && (h.search(/_js/i) == -1) ) {
					if (mysheet.cssRules && mysheet.cssRules.length > 0) {
						myrules = mysheet.cssRules;
					} else {
						myrules = mysheet.rules;
					}
					if (myrules) {
						for (i=0; i<myrules.length; i++){
							if (myrules[i].selectorText.toLowerCase() == myRule) {
								targetrule=myrules[i];
								break;
							}
						}
					}
				}
			}
		}
		if (targetrule == null)
			showError('Not Found : '+ myRule);

		return targetrule;
	}

	// Reload all stylesheets
	function ReloadStylesheets() {
		var i,a,s;
		a=document.getElementsByTagName('link');
		for(i=0;i<a.length;i++) {
			s=a[i];
			if(s.rel.toLowerCase().indexOf('stylesheet')>=0&&s.href) {
				var h=s.href.replace(/(&|\\?)forceReload=d /,'');
				s.href=h+(h.indexOf('?')>=0?'&':'?')+'forceReload='+(new Date().valueOf());
			}
		}
	}

	function IsVariation(themeID) {
		q = themeID.indexOf(':');
		return q != -1;
	}

	// NOT IN USE
	// Bind actions for those items which don't highlight on screen, but only in a menu bar or at the top of the screen, e.g. something hard to click on or otherwise odd.
	$.fn.cssEditAddToMenu = function () {
		return this.each(function() {
			});
	}

	$.SetCookie = function (name, val, opt) {
		$.cookies.set(name, JSON.stringify(val), opt);
	}

	$.GetCookie = function (name) {
		return JSON.parse($.cookies.get(name));
	}


})(jQuery);
