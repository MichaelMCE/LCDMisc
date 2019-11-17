#import <Views\View.c>
#import <framework\framework.c>
#requires <framework\header.c>
#requires <util\Graphics.c>
#requires <util\button.c>

struct ApplLaunch extends View {
	var %touchDownEvent, %tpoint;
	var %tx, %ty, %tdt, %ttime;
	var %buttons;
	var %background;

	function ApplLaunch ($iconPath, $iconPathL, $backgroundPath, $appIcons) {
		%touchDownEvent = 0;
		%hasFocus = 0;
		%noDrawOnCounterUpdate = 0;
		%noDrawOnAudioChange = 1;
		%tpoint;
		%tx = 0;
		%ty = 0;
		%tdt = 0;
		%ttime = 0;
		%toolbarImage = LoadImage32($iconPath);
		%toolbarImageL = LoadImage32($iconPathL);
		%background = LoadImage32($backgroundPath);

		%buttons = $appIcons;		
		%buttons.addIcon(3, 6,  104,"images\left.png",  "G15ButtonDown", G15_LEFT);
		%buttons.addIcon(3, 412,104,"images\right.png", "G15ButtonDown", G15_RIGHT);
	}
	
    function HandleEvent ($event, $param, $tpos) {
		if ($event ==s "touchDown2"){
			%touchDownFn(@$);
			return 1;
		}
		return 0;
	}

	function touchDownFn ($event, $param, $tpos) {
		%tpoint = strsplit($tpos, ",", 0, 0, 0); // "x,y,dt,time"
		%tx = %tpoint[0];
		%ty = %tpoint[1];
		%touchDownEvent++;
		return %buttons.runTouchedIcon(%tx, %ty);
	}
	
	function Focus() {
		if (!%hasFocus) {
			%hasFocus = 1;
		}
	}

	function Unfocus() {
		if (%hasFocus) {
			%hasFocus = 0;
		}
	}

	function KeyDown($event, $param, $modifiers, $vk, $string) {
		//NeedRedraw();
		if ($vk == VK_ESCAPE) {
			%Unfocus();
		}else if ($vk == VK_RETURN) {
		}else if ($vk == VK_UP || $vk == VK_DOWN) {
		}else {
		}
		return 1;
	}

	function G15ButtonDown($event, $param, $button) {
		DisplayText($button, 0, 60);
		
		$button = FilterButton($button);
		if ($button & 0x3F) {
			if ($button == G15_UP) {
				%KeyDown(,,,VK_UP);
			}
			else if ($button == G15_DOWN) {
				%KeyDown(,,,VK_DOWN);
			}
			else if ($button == G15_CANCEL) {
				%Unfocus();
			}
			NeedRedraw();
			return 1;
		}
	}

	function Draw ($event, $param, $name, $res) {
		$w = $res[0];
		$h = $res[1];
		DrawImage(%background, 0, 0);
		if (!%hasFocus)
			%buttons.drawIcons();
		else
			%buttons.drawIconsWithBorder(RGB(255,255,0));
	}
};
