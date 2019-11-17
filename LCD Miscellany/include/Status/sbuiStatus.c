#requires <framework\header.c>
#requires <util\CPUSaver.c>
#requires <util\Audio.c>
#requires <Modules\DisplayText.c>
#import <constants.h>
#import <Modules\Graph.c>
#requires <Modules\DriveInfo.c>
#requires <Modules\CounterManager.c>
#requires <Shared Memory.c>
#requires <util\button.c>


dll32 sbuiDll "sbui_32.dll";
dll64 sbuiDll "sbui_64.dll";
dll sbuiDll int sbuiDynamicKeySetImage "sbuiDynamicKeySetImage" (int key, int state, wstring path);
dll sbuiDll int sbuiSetGestureCfg "sbuiSetGestureCfg" (int op, int gesture, int state);
dll sbuiDll int sbuiReconnect "sbuiReconnect" ();
dll sbuiDll int sbuiInit "Init" ();


function SetGestureConfig ($op, $gesture, $state)
{
	sbuiSetGestureCfg($op, $gesture, $state);
}

function DynamicKeySetImage ($key, $state, $image)
{
	//WriteLogLn(IMAGE_PATH +s SBUI_PATH +s $image, 1);
	sbuiDynamicKeySetImage($key, $state, IMAGE_PATH +s SBUI_PATH +s $image);
}

struct sbuiStatus extends MenuHandler {
	var %miniFont, %font, %counters;
	var %driveDelayShow;
	var %background;
	var %graphDisplay;
	var %colours;
	
	function sbuiStatus() {
		// enough colours for 8 cores
		%colours = list(RGB(0,224,0), RGB(255,0,0), RGB(0,255,255), RGB(255,255,0), RGB(255,0,255), RGB(240,240,240), RGB(244,100,50), RGB(0,0,244));
		
		sbuiInit();
		DynamicKeySetImage(SBUI_DK_1, SBUI_DK_UP, "dkLeft.png");
		DynamicKeySetImage(SBUI_DK_2, SBUI_DK_UP, "dk2.png");
		DynamicKeySetImage(SBUI_DK_3, SBUI_DK_UP, "dk3.png");
		DynamicKeySetImage(SBUI_DK_4, SBUI_DK_UP, "dk4.png");
		DynamicKeySetImage(SBUI_DK_5, SBUI_DK_UP, "dkRight.png");
		DynamicKeySetImage(SBUI_DK_6, SBUI_DK_UP, "dk6.png");
		DynamicKeySetImage(SBUI_DK_7, SBUI_DK_UP, "dk7.png");
		DynamicKeySetImage(SBUI_DK_8, SBUI_DK_UP, "dk8.png");
		DynamicKeySetImage(SBUI_DK_9, SBUI_DK_UP, "dk9.png");
		DynamicKeySetImage(SBUI_DK_10,SBUI_DK_UP, "dk10.png");
		
		DynamicKeySetImage(SBUI_DK_1, SBUI_DK_DOWN, "dkLeftAlt.png");
		DynamicKeySetImage(SBUI_DK_2, SBUI_DK_DOWN, "dkAlt2.png");
		DynamicKeySetImage(SBUI_DK_3, SBUI_DK_DOWN, "dkAlt3.png");
		DynamicKeySetImage(SBUI_DK_4, SBUI_DK_DOWN, "dkAlt4.png");
		DynamicKeySetImage(SBUI_DK_5, SBUI_DK_DOWN, "dkRightAlt.png");
		DynamicKeySetImage(SBUI_DK_6, SBUI_DK_DOWN, "dkAlt6.png");
		DynamicKeySetImage(SBUI_DK_7, SBUI_DK_DOWN, "dkAlt7.png");
		DynamicKeySetImage(SBUI_DK_8, SBUI_DK_DOWN, "dkAlt8.png");
		DynamicKeySetImage(SBUI_DK_9, SBUI_DK_DOWN, "dkAlt9.png");
		DynamicKeySetImage(SBUI_DK_10,SBUI_DK_DOWN, "dkAlt10.png");

		%miniFont = Font("6px2bus", 6);
		%counters = GetCounterManager();
		%font = Font("Arial", 30,0,0,0,CLEARTYPE_QUALITY);
		%driveDelayShow = GetTickCount();
		%background = ImageLoad("statusBackground.png");
		%graphDisplay = 0;
	}

	function Show (){
		DynamicKeySetImage(SBUI_DK_3, SBUI_DK_UP, "dkQ.png");
		DynamicKeySetImage(SBUI_DK_3, SBUI_DK_DOWN, "dkAltQ.png");
		
		DynamicKeySetImage(SBUI_DK_6, SBUI_DK_UP, "dkStatsCycle.png");
		DynamicKeySetImage(SBUI_DK_6, SBUI_DK_DOWN, "dkAltStatsCycle.png");
		
		DynamicKeySetImage(SBUI_DK_9, SBUI_DK_UP, "dkClockA.png");
		DynamicKeySetImage(SBUI_DK_9, SBUI_DK_DOWN, "dkAltClockA.png");
		
		DynamicKeySetImage(SBUI_DK_10, SBUI_DK_UP, "dkClockD.png");
		DynamicKeySetImage(SBUI_DK_10, SBUI_DK_DOWN, "dkAltClockD.png");
		
		%driveDelayShow = GetTickCount();
	}
	
	function Hide (){
		DynamicKeySetImage(SBUI_DK_3, SBUI_DK_UP, "dk3.png");
		DynamicKeySetImage(SBUI_DK_3, SBUI_DK_DOWN, "dkAlt3.png");
		
		DynamicKeySetImage(SBUI_DK_6, SBUI_DK_UP, "dk6.png");
		DynamicKeySetImage(SBUI_DK_6, SBUI_DK_DOWN, "dkAlt6.png");
		
		DynamicKeySetImage(SBUI_DK_9, SBUI_DK_UP, "dk9.png");
		DynamicKeySetImage(SBUI_DK_9, SBUI_DK_DOWN, "dkAlt9.png");
		
		DynamicKeySetImage(SBUI_DK_10, SBUI_DK_UP, "dk10.png");
		DynamicKeySetImage(SBUI_DK_10, SBUI_DK_DOWN, "dkAlt10.png");
		
		//%driveDelayShow = GetTickCount;
	}
	
    function HandleEvent ($event, $param, $key) {
		if ($event ==s "sbuiEventDKDown"){
			//WriteLogLn($event +s " @ " +s $key, 1);

			if ($key == SBUI_DK_6){
				if (++%graphDisplay > 4)
					%graphDisplay = 0;
				
				NeedRedraw();
			}else if ($key == SBUI_DK_3){
				Quit();
			}else if ($key == SBUI_DK_9){
				%setClockState(SHOW_CLOCKA);
			}else if ($key == SBUI_DK_10){
				%setClockState(SHOW_CLOCKD);
			}

			return 1;
			
		}else if ($event ==s "sbuiEventDKUp"){
			//WriteLogLn($event +s " - " +s $key, 1);
			return 1;
			
		}else if ($event ==s "touchDownEvent"){
			//WriteLogLn($event +s " - " +s $key, 1);
			return 1;
		}
		return 0;
	}
	
	
	function drawVolume ($posx, $posy, $vwidth, $vheight, $vol){
		DrawRectangle($posx, $posy, $posx+$vwidth, $posy+$vheight, RGB(20,255,20));
		DrawRectangleFilled($posx+1, $posy+1, $posx+$vwidth-1, $posy+$vheight-1, RGBA(0,60,0,80));
		DrawRectangleFilled($posx+2, $posy+2, $posx+2+((($vwidth-4)/100.0)*$vol), $posy+$vheight-2, RGB(2.55*$vol,255-(2.00*$vol),255-(2.55*$vol)));
	}

	function Draw ($event, $param, $name, $res) {

		//ClearScreen();
		DrawImage(%background, 0, 0);
		DisplayHeader($res);
		UseFont(%font);

		$memory = GetMemoryStatus();
		DisplayTextShadowed("CPU: "+s FormatValue(%counters.cpuData["_Total"], 0, 1)+s "%", 1, 40, RGBA(0x50,0x8D,0xC5,156), RGBA(255,255,255,240));
		DisplayTextShadowed("Mem: "+s FormatValue($memory[1]/1024.0/1024.0), 1, 70, RGBA(0x50,0x8D,0xC5,156), RGBA(255,255,255,240));
		DisplayTextShadowed("VM: "+s FormatValue($memory[3]/1024.0/1024.0), 1, 100, RGBA(0x50,0x8D,0xC5,156), RGBA(255,255,255,240));

		%drawVolume(7, 135, 783, 20, GetMaxAudioState(0,0)[0]);
		//%drawVolume(12, 100, 214, 6, GetMaxAudioState(0,0)[1]);


		//$ati = GetATTData();
		$speed = GetSpeedFanData();
		$x = 300; $y = 38; $line = 32;
		
		DisplayTextCenteredShadowed("CPU:" +s FormatSize($speed["temps"][4], 0, 3, 1)+s "°c", $x, $y, RGBA(0x50,0x8D,0xC5,156), RGBA(255,255,255,240));
		DisplayTextCenteredShadowed("Fan:" +s $speed["fans"][3], $x+200, $y, RGBA(0x50,0x8D,0xC5,156), RGBA(255,255,255,240));
		
		DisplayTextCenteredShadowed("GPU:" +s FormatSize($speed["temps"][6], 0, 3, 1)+s "°c", $x, $y+$line, RGBA(0x50,0x8D,0xC5,156), RGBA(255,255,255,240));
		DisplayTextCenteredShadowed("Fan:" +s $speed["fans"][8], $x+200, $y+$line, RGBA(0x50,0x8D,0xC5,156), RGBA(255,255,255,240));
		//DisplayTextCenteredShadowed("Core:" +s $speed["temps"][8]+s "°c", $x, $y, RGBA(0x50,0x8D,0xC5,156), RGBA(255,255,255,240));
		DisplayTextCenteredShadowed("HD0:" +s $speed["temps"][3]+s "°c", $x, $y+($line*2), RGBA(0x50,0x8D,0xC5,156), RGBA(255,255,255,240));

		

		// delay showing the drive info at startup as this function is quite slow when first executed.
		//if (GetTickCount() - %driveDelayShow > 2000)
		//	DisplayDriveInfo(315, 23, 460, 84, -4);

		//DisplayTextCentered(GetRemoteIP(), 240, 77);

		/*SetDrawColor(RGB(0,255,0));
		DisplayTextShadowed("Dn:"+s FormatSize(%counters.down, 1,0,1), 250, 95, RGBA(0,0,0,200), RGBA(255,255,255,240));
		SetDrawColor(RGB(255,0,0));
		DisplayTextShadowed("Up:"+s FormatSize(%counters.up, 1,0,1), 400, 95, RGBA(0,0,0,200), RGBA(255,255,255,240));
		SetDrawColor(RGB(0,0,0));*/

		if (%graphDisplay == 0){
			DrawMultiGraph(%counters.cpuGraph, 1, 479, 800-2, 164, RGBA(20,20,20,120),list(0,2));
			
		}else if (%graphDisplay == 1){
			SetDrawColor(RGB(0,255,0));
			DisplayTextShadowed("Dn:"+s FormatSize(%counters.down, 1,0,1), 406, 105, 0x00FF1E72, RGBA(255,255,255,240));
			SetDrawColor(RGB(255,0,0));
			DisplayTextShadowed("Up:"+s FormatSize(%counters.up, 1,0,1), 648, 104, 0x00FF1E72, RGBA(255,255,255,240));
			SetDrawColor(RGB(0,0,0));

			DrawMultiGraph(%counters.cpuGraph, 1, 479, 400-2, 164, RGBA(20,20,20,120),list(0,2));
			DrawMultiGraph(list(%counters.downGraph, %counters.upGraph), 400+2, 479, 800-2, 164, RGBA(20,20,20,120),list(0,2));
			
		}else if (%graphDisplay == 2){
			SetDrawColor(RGB(0,255,0));
			DisplayTextShadowed("Dn:"+s FormatSize(%counters.down, 1,0,1), 406, 105, 0x00FF1E72, RGBA(255,255,255,240));
			SetDrawColor(RGB(255,0,0));
			DisplayTextShadowed("Up:"+s FormatSize(%counters.up, 1,0,1), 648, 104, 0x00FF1E72, RGBA(255,255,255,240));
			SetDrawColor(RGB(0,0,0));

			DrawMultiGraph(list(%counters.downGraph, %counters.upGraph), 1, 479, 800-2, 164, RGBA(20,20,20,120),list(0,2));
			
		}else if (%graphDisplay == 3){
			$cpuTotal = size(%counters.cpuGraph);
			
			$gap = 2;
			$width = (800- $gap*$cpuTotal) / $cpuTotal ;
			$x = 1;
			for ($i = 0; $i < $cpuTotal; $i++){
				$graph = %counters.cpuGraph[$i];
				DrawGraph($graph, $x, 479, $x+$width-1, 164, RGBA(20,20,20,120), %colours[$i], list(0,2));
				$x += $width + $gap;
			}

		}else if (%graphDisplay == 4){
			DrawMultiGraph(%counters.cpuGraph, 1, 479, 400-2, 164, RGBA(20,20,20,120),list(0,2));
			
			$cpuTotal = size(%counters.cpuGraph);
			$gap = 6;
			$x = 400+4;
			$width = ((800-$x)- $gap*$cpuTotal) / $cpuTotal;
			
			for ($i = 0; $i < $cpuTotal; $i++){
				$graph = %counters.cpuGraph[$i];
				DrawBarGraph($graph, $x, 479, $x+$width-1, 164, RGBA(20,20,20,120), %colours[$i], list(1,0,1));
				$x += $width + $gap;
			}
		}
	}
};
