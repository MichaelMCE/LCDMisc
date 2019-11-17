#import <Views\View.c>
#import <framework\framework.c>
#requires <framework\header.c>
#requires <util\Graphics.c>
#requires <util\button.c>



struct Clock extends View {
	var %background1, %background2;
	var %digits1, %digits2;
	var %aface;
	var %secFace;
	var %hrFace;
	var %minFace;
	var %dotFace;
	var %base;
	
	function Clock () {
		%hasFocus = 0;
		%noDrawOnCounterUpdate = 0;
		%noDrawOnAudioChange = 1;

		%toolbarImage = LoadImage32("images/dclock1.png");
		%toolbarImageL = LoadImage32("images/dclock2.png");
		//%background1 = LoadImage32("images/clock/backgroundr.png");	// sun rise
		//%background2 = LoadImage32("images/clock/backgroundn.png");	// night
		//%base = LoadImage32("images/clock/base.png");
		//%aface = LoadImage32("images/clock/face.png");
		//%secFace = LoadImage32("images/clock/sec.png");
		//%hrFace = LoadImage32("images/clock/hr.png");
		//%minFace = LoadImage32("images/clock/min.png");
		//%dotFace = LoadImage32("images/clock/dot.png");
		
		/*%digits1 = list();	// time
		$i = 10;
		while ($i--)
			%digits1[$i] = LoadImage32("images/clock/digits1/" +s $i +s ".png");

		%digits2 = list();	// date
		$i = 10;
		while ($i--)
			%digits2[$i] = LoadImage32("images/clock/digits2/" +s $i +s ".png");*/
	}

	function DrawBackground (){
		//$hrs = FormatTime("HH");
		//if ($hrs >= 06 && $hrs <= 22)
			DrawImage(%background1, 0, 0);
		//else
			//DrawImage(%background2, 0, 0);
	}
	
	function DrawForegroundD ($_X){
		if ($_X){
			$posX = 88;	// clock view
			$posY = 59;
		}else{
			$posX = 88;	// clock overlay view
			$posY = 98;
		}

		$time = FormatTime("HHNN");
		$th1 = $time[0];
		$th2 = $time[1];
		$tm1 = $time[2];
		$tm2 = $time[3];

		DrawImage(%base, $posX, $posY-55);

		// hours
		$posX += 28;
		DrawImage(%digits1[$th1], $posX, $posY); DrawImage(%digits1[$th2], $posX+54, $posY);

		// minutes
		$posX += 139;
		DrawImage(%digits1[$tm1], $posX, $posY); DrawImage(%digits1[$tm2], $posX+54, $posY);
	}

	function DrawForegroundA (){
		$xc = 480.0/2.0;
		$yc = 272.0/2.0;

		$hr = FormatTime("H");
		$min = FormatTime("N");
		//$sec = FormatTime("S");
		//$s =  (6.0 * $sec + 0.5) * 0.0174532925195;	// 6 = (360.0 / 60.0)
		$m = (6.0 * $min) * 0.0174532925195;			// 30 = 360.0 / 12.0
		$h = (30.0 * ($hr +((1.0/60.0)*$min)) + 0.5) * 0.0174532925195;

		DrawImage(%aface, 115, 11);
		DrawRotatedScaledImage(%hrFace, $xc-12, $yc-125, $h);
		DrawRotatedScaledImage(%minFace, $xc-12, $yc-125, $m);
		//DrawRotatedScaledImage(%secFace, $xc-24, $yc-64, $s);
	}

	// this is also called from framework.c
	function DrawForeground ($clockType, $i){
		if ($clockType)
			%DrawForegroundA();
		else
			%DrawForegroundD($i);
	}
			
	function DrawDate ($x, $y){
		$dy = FormatTime("DD");
		$mn = FormatTime("MM");
		$yr = FormatTime("YY");
		
		// day
		DrawImage(%digits2[$dy[0]], $x, $y); DrawImage(%digits2[$dy[1]], $x+62, $y);

		// month
		$x += 132;
		DrawImage(%digits2[$mn[0]], $x, $y); DrawImage(%digits2[$mn[1]], $x+62, $y);

		// year
		$x += 132;
		DrawImage(%digits2[$yr[0]], $x, $y); DrawImage(%digits2[$yr[1]], $x+62, $y);
	}
	
	function Draw ($event, $param, $name, $res) {
		%DrawBackground();
		%DrawForeground(0, 1);
		%DrawDate(47, 189);
	}
};
