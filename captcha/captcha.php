<?php
session_start("captcha");
    define("WIDTH", 200);
    define("HEIGHT", 70);

	define("F_SIZE", 20);
    define("F_ANGLE", 0);
// NOTE: fonts 3 and 4 are hard to read.
$rnd = rand(1,2);
switch ($rnd) 
{
	case 1:
		define("F_FONT", "myfont.ttf");
		break;
	case 2:
		define("F_FONT", "myfont2.ttf");
		break;
	case 3:
		define("F_FONT", "myfont3.ttf");
		break;
	case 4:
		define("F_FONT", "myfont4.ttf");
		break;
}

	$img = imagecreate(WIDTH, HEIGHT);

    $white = imagecolorallocate($img, 255,255,255);
    $brdr = imagecolorallocate($img, 0,0,0);
$black = imagecolorallocate($img, rand(0,150),rand(0,150),rand(0,150));

    $start_x = rand(10,15);
    $start_y = (int)HEIGHT/2;

    imagerectangle($img, 0,0,WIDTH-1,HEIGHT-1, $brdr);

	$text = chr(rand(65,90));
	$key = $text;
    imageTTFtext($img, F_SIZE, F_ANGLE + rand(-30,30), $start_x, $start_y + (rand(-5,5)), $black, F_FONT, $text);
	$text = chr(rand(65,90));
	$key = $key.$text;
	imageTTFtext($img, F_SIZE, F_ANGLE + rand(-30,30), $start_x+30, $start_y + (rand(-5,5)), $black, F_FONT, $text);
	$text = chr(rand(65,90));
	$key = $key.$text;
	imageTTFtext($img, F_SIZE, F_ANGLE + rand(-30,30), $start_x+60, $start_y + (rand(-5,5)), $black, F_FONT, $text);
	$text = chr(rand(65,90));
	$key = $key.$text;
	imageTTFtext($img, F_SIZE, F_ANGLE + rand(-30,30), $start_x+90, $start_y + (rand(-5,5)), $black, F_FONT, $text);
	$text = chr(rand(65,90));
	$key = $key.$text;
	imageTTFtext($img, F_SIZE, F_ANGLE + rand(-30,30), $start_x+120, $start_y + (rand(-5,5)), $black, F_FONT, $text);
	$text = chr(rand(65,90));
	$key = $key.$text;
	imageTTFtext($img, F_SIZE, F_ANGLE + rand(-30,30), $start_x+150, $start_y + (rand(-5,5)), $black, F_FONT, $text);

$_SESSION["hash"]=$key;

$rnd = rand(1,9);
switch ($rnd) 
{
	case 1:
		$img_copy = imagecreatefrompng("captcha.png");
		break;
	case 2:
		$img_copy = imagecreatefrompng("captcha2.png");
		break;
	case 3:
		$img_copy = imagecreatefrompng("captcha3.png");
		break;
	case 4:
		$img_copy = imagecreatefrompng("captcha4.png");
		break;
	case 5:
		$img_copy = imagecreatefrompng("captcha5.png");
		break;
	case 6:
		$img_copy = imagecreatefrompng("captcha6.png");
		break;
	case 7:
		$img_copy = imagecreatefrompng("captcha7.png");
		break;
	case 8:
		$img_copy = imagecreatefrompng("captcha8.png");
		break;
	case 9:
		$img_copy = imagecreatefrompng("captcha9.png");
		break;
}
imagecopymerge($img, $img_copy, 0, 0, 0, 0, imagesx($img), imagesy($img), rand(10,30));
header("Content-Type: image/png");
imagepng($img);
imagedestroy($img);
?>