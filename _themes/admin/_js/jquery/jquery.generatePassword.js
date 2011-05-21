/*
 * Generate Password 1.0.0
 *
 * Copyright (c) 2009 Lesnykh IV
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Dual licensed under the MIT and GPL licenses.
 *
 * @copyright	2009 Лесных Илья Владимирович
 * @license		http://www.gnu.org/licenses/gpl.html	GNU GPL
 * @license		http://www.opensource.org/licenses/mit-license.php	MIT
 * @version		Release: $Id: jquery.generatePassword.js, v 1.0.0 2009-06-09 Lesnykh IV $
 * @contact		leonclan@yandex.ru
 * @author		Lesnykh IV
 * @since		Script available since jQuery 1.3.2
 */
(function($)
	{
		$.fn.generatePassword = function( options )
		{
			var options = $.extend(
				{
					duplicate: '#retype-pwd',
					nums: [ '0','1','2','3','4','5','6','7','8','9' ],
					lower_chars: [ 'q','w','e','r','t','y','u','i','o','p','a','s','d','f','g','h','j','k','l','z','x','c','v','b','n','m' ],
					upper_chars: [ 'Q','W','E','R','T','Y','U','I','O','P','A','S','D','F','G','H','J','K','L','Z','X','C','V','B','N','M' ],
					special_chars: [ '[',']',';',"'",',','.','/','!','@','#','$','%','^','&','*','(',')','{','}',':','"','<','>','?' ]
				},
				options
			);
			return this.each( function()
				{
					var linkID = $( this ).attr( 'id' ) + '-link';
					var link = $( '<br /><a href="#" id="' + linkID + '" class="generate-password">Generate password</a>' );
					$( this ).after( link );
					( function( $this, _linkID )
						{
							$( '#' + _linkID ).click( function( event )
								{
									var evt = event || window.event;
									if ( evt )
									{
										if ( evt.preventDefault )
											evt.preventDefault();
										evt.returnValue = false;
									}

									// Generator
									var _generated_password = [];
									var _generator = [ options.nums, options.lower_chars, options.upper_chars, options.special_chars ];
									var _generated_password_length = ( Math.floor( Math.random() * 25 ) + 8 );
									for ( var l = 0; l < _generated_password_length + 1; l++ )
									{
										var __generator = _generator[ Math.floor( Math.random() * 4 ) ];
										_generated_password.push( __generator[ Math.floor( Math.random() * __generator.length ) ] );
									}
									_generated_password = _generated_password.join( '' );
									// generated password + it's length
									// alert( _generated_password.length + ': ' + _generated_password.join( '' ) );

									$this.val( _generated_password );

									if ( options.duplicate )
									{
										$( options.duplicate ).val( _generated_password );
									}
								}
							)
						}
					)( $( this ), linkID );
				}
			);
		}
	}
)(jQuery);