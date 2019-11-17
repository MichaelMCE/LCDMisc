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
#requires <util\tcx.c>


dll32 utillsDll "utills_32.dll" "Init";
dll64 utillsDll "utills_64.dll" "Init";

dll utillsDll int vlcGetWindowHandle "vlcGetWindowHandle" ();

dll utillsDll int hrmGetValue "hrmGetValue" (wstring name);
dll utillsDll int hrmGetValue64 "hrmGetValueQ" (wstring name);
dll utillsDll function hrmGetInfo "hrmGetInfo";

dll utillsDll int trackGetMetaInt "trackGetMetaInt" (int meta);
dll utillsDll string trackGetMetaStr "trackGetMetaStr" (int meta);
dll utillsDll function trackGetMetaInfo "trackGetMetaInfo";

/*
$info = trackGetMetaInfo();
$info[MTAG_Artist]: track artist
$info[MTAG_Album]: etc.. refer to constants.h for complete tag list
note that not all fields are filled


$info = hrmGetInfo();
$info[0]: current bpm
$info[1]: average
$info[2]: mode
$info[3]: lowest bpm read
$info[4]: highest bpm read
$info[5]: previous bpm
$info[6]: time (GetTickCount()) when last updated 

*/


struct USBD480Status {
	var %font, %modeFont, %bpmFont, %counters;
	//var %driveDelayShow;
	var %background, %origBackground;
	var %vlcTitle, %vlcTitleTimer, %vlcWindowHandle;
	var %vlcTitlePosX, %vlcTitlePosY;
	var %trackTitle, %trackAlbum, %trackNumber, %trackLength, %trackArtist, %trackAddedTime;
	var %trackArtworkMRL;
	var %trackArtworkImage;
	var %hrmIsConnected;
	var %tcxRoute;
	var %serial;
	
	function USBD480Status (){
		%counters = GetCounterManager();
		%font = Font("Arial", 26,0,0,0,CLEARTYPE_QUALITY);
		//%bpmFont = Font("Insane Hours",60,0,0,0,CLEARTYPE_QUALITY);
		//%modeFont = Font("Insane Hours", 32,0,0,0,CLEARTYPE_QUALITY);
		%hrmIsConnected = 0;
		//%driveDelayShow = GetTickCount();
		%origBackground = ImageLoad("statusBackground.png");
		%background = %origBackground.Copy();
		//%background.Clear();


		// (const char *Port, const int baud, const int Bitsize, const int StopBits*10, const int Parity)
		//%serial = ComPort("COM1", 9600, 8, 10, 0);

		%vlcTitleTimer = CreateTimer("GetVLCWindowTitle", 10, 1, $this);
		%GetVLCSMeta();

		%tcxRoute = Tcx(/*"C:/barnets-red-26_8_13 15_10_02_history.tcx"*/, "C:/Users/Administrator/AppData/Roaming/Garmin/Devices/3825666043/History/");
		%tcxRoute.update();
	}

	function GetVLCSMeta (){
		//%serial.Write("123456teSt");
		//%serial.Write(65);
		/*$alist = list(15);
		$alist[0] = 65;
		$alist[1] = 2.37;
		$alist[2] = 77;
		$alist[3] = "123456teSt";
		%serial.Write($alist);*/

		
		%trackTitle = trackGetMetaStr(MTAG_Title);
		if (IsNull(%trackTitle)) return;
		
		%trackAlbum = trackGetMetaStr(MTAG_Album);
		%trackNumber = trackGetMetaInt(MTAG_TrackNumber);
		%trackArtist = trackGetMetaStr(MTAG_Artist);
		//%trackAddedTime = trackGetMetaInt(MTAG_ADDTIME);
		%trackLength = trackGetMetaStr(MTAG_LENGTH);
		$trackArtwork = trackGetMetaStr(MTAG_ArtworkPath);
		
		if ($trackArtwork !=s %trackArtworkMRL){
			%trackArtworkMRL = $trackArtwork;
			%trackArtworkImage = LoadImage32(%trackArtworkMRL);
			
			if (!IsNull(%trackArtworkImage)){
				
				$height = %trackArtworkImage.Size()[1];
				%trackArtworkImage = %trackArtworkImage.Zoom(170.0/$height);	// 170 = space available below volume bar
				%trackArtworkImage = %trackArtworkImage.Fade(0.6, 1);
				//%trackArtworkImage = %trackArtworkImage.Flip(3);
				//%trackArtworkImage = %trackArtworkImage.Pixelize(4);
				//%trackArtworkImage = %trackArtworkImage.Blur(4, 0/*blurOp - todo*/);
				
				/*
				image32.Fade(level, mode). mode 0 = rgba, mode 1 = rgb, mode 2 = alpha channel only
				image32.Blur(radius). radius = 1,2,3,etc..
				image32.Flip(v_or_h). v_or_h = 1 for vertical, 2 for horizontal, 3 = flips both (identical to a 180degree rotate)
				image32.Pixelize(pixelizationSize);	// factor
				image32.Copy();	// clone the image
				image32.Clear(colour);	// clear image. colour defaults to 0xFF000000 if not supplied
				image32.Draw(desImage32, desX, desY, srcX, srcY, srcWidth, srcHeight);	//draw one image32 on to another. does not return a copy
				*/

				%background = %origBackground.Copy();
				%trackArtworkImage.Draw(%background,(DWIDTH - %trackArtworkImage.Size()[0]) - 6, 100);
			}
		}
		
		%tcxRoute.update();
	}
	
	function GetVLCWindowTitle (){
		%GetVLCSMeta();

		%hrmIsConnected = abs(GetTickCount() - (hrmGetValue64("HRM_bpmTime"))) < 10000; // 10000 = 10 seconds. don't display if hrm isn't connected or updating
		//$hlist = hrmGetInfo();
		//WriteLogLn($hlist[6],1);
		//%hrmIsConnected = (GetTickCount() - $hlist[6]) < 10000; // 10000 = 10 seconds. don't display if hrm isn't connected or updating
		if (%hrmIsConnected){
			if (IsNull(%bpmFont))
				//%bpmFont = Font("Insane Hours",60,0,0,0,CLEARTYPE_QUALITY);
				//%bpmFont = Font("Shuttle-Extended",96,0,0,0,CLEARTYPE_QUALITY);
				%bpmFont = Font("SF Square Head",96,0,0,0,CLEARTYPE_QUALITY);
				
			if (IsNull(%modeFont))
				%modeFont = Font("Insane Hours", 32,0,0,0,CLEARTYPE_QUALITY);
			return;	// HRM display and vlc title share the same display area
		}
		
		// get the title..
		%vlcWindowHandle = vlcGetWindowHandle();
		if (!%vlcWindowHandle) return;

		%vlcTitle = GetWindowText(%vlcWindowHandle);
		if (%vlcTitle ==s "VLC media player"){		// VLC is running but not playing anything
			%vlcWindowHandle = 0;
			return;
		}

		// remove file extension if possible
		$breakdown = strsplit(%vlcTitle, ".", 0, 200, );
		if (size($breakdown) == 2){		// if there is only a before and after, then this must be an extension
			%vlcTitle = $breakdown[0];
		}else{
			$breakdown = strsplit(%vlcTitle, " - ", 0, 200, ); // remove the vlc program name which follows the hyphen
			if (size($breakdown) > 2){
				$total = size($breakdown);
				%vlcTitle = "";
								
				for ($i = 0; $i < $total-1; $i++){
					%vlcTitle = %vlcTitle +s $breakdown[$i];
					if ($i < $total-2) %vlcTitle = %vlcTitle +s " - ";
				}
			}else{
				%vlcTitle = $breakdown[0];
			}
		}
		
		%vlcTitlePosY = 40;
		UseFont(%font);
		$twidth = TextSize(%vlcTitle)[0];
		if ($twidth >= DWIDTH-128)
			%vlcTitlePosX = 128;
		else
			%vlcTitlePosX = DWIDTH-2 - $twidth;
	}
		
	function drawVolume ($posx, $posy, $vwidth, $vheight, $vol){
		DrawRectangle($posx, $posy, $posx+$vwidth, $posy+$vheight, RGB(20,255,20));
		DrawRectangleFilled($posx+1, $posy+1, $posx+$vwidth-1, $posy+$vheight-1, RGBA(200,60,0,80));
		DrawRectangleFilled($posx+2, $posy+2, $posx+2+((($vwidth-4)/100.0)*$vol), $posy+$vheight-2, RGB(2.55*$vol,255-(2.00*$vol),255-(2.55*$vol)));
	}

	function Hide (){
		//%driveDelayShow = GetTickCount;
		StopTimer(%vlcTitleTimer);
		
	}
		
	function Show (){
		//%driveDelayShow = GetTickCount();
		StartTimer(%vlcTitleTimer);
	}
	
	function Draw ($event, $param, $name, $res) {
		//ClearScreen();
		DrawImage(%background, 0, 0);
		DisplayHeader($res);

		UseFont(%font);
		$memory = GetMemoryStatus();
		DisplayTextShadowed("CPU: "+s FormatValue(%counters.cpuData["_Total"], 0, 1)+s "%", 1, 28, RGBA(0x50,0x8D,0xC5,156), RGBA(255,255,255,240));
		DisplayTextShadowed("Mem: "+s FormatValue($memory[1]/1024.0/1024.0), 1, 54, RGBA(0x50,0x8D,0xC5,156), RGBA(255,255,255,240));
		//DisplayTextCenteredShadowed("VM: "+s FormatValue($memory[3]/1024.0/1024.0), 290, 50, RGBA(0x0,0x0,0x0,255), RGBA(0xFF,0x00,0x00,255));

		if (%hrmIsConnected){
			UseFont(%bpmFont);
			DisplayTextCenteredShadowed(FormatValue(hrmGetValue("HRM_bpm")), (DWIDTH/2)-16, 0/*20*/, RGBA(0xFF,0x1D,0x0,255), RGBA(255,255,255,255));
			UseFont(%modeFont);
			DisplayTextShadowed(FormatValue(hrmGetValue("HRM_mode")), 400, 40, RGBA(0xFF,0x1D,0x0,255), RGBA(255,255,255,255));
			UseFont(%font);

		}else if (%vlcWindowHandle){
			DisplayTextShadowed(%vlcTitle, %vlcTitlePosX, %vlcTitlePosY, RGBA(0xf6,0x7f,0x0,156), RGBA(255,255,255,240));
		}

		%drawVolume(7, 82, 465, 12, GetMaxAudioState(0,0)[0]);

		if (!IsNull(%trackTitle)){
			DisplayTextShadowed(%trackNumber +s ": " +s %trackArtist, 1, 100, RGBA(0xf6,0x7f,0x0,156), RGBA(255,255,255,240));
			DisplayTextShadowed(%trackTitle, 1, 128, RGBA(0xf6,0x7f,0x0,156), RGBA(255,255,255,255));
			DisplayTextShadowed(%trackLength, 1, 156, RGBA(0xf6,0x7f,0x0,156), RGBA(255,255,255,240));
			DisplayTextShadowed(%trackAlbum, 1, 184, RGBA(0xf6,0x7f,0x0,156), RGBA(255,255,255,240));
		}


		//$ati = GetATTData();
		/*$speed = GetSpeedFanData();
		DisplayTextCenteredWithBack("CPU:" +s $speed["temps"][1]+s "�c", 160, 23);
		DisplayTextCenteredWithBack("Fan:" +s $speed["fans"][0], 250, 23);
		DisplayTextCenteredWithBack("GPU:" +s $speed["temps"][6]+s "�c", 160, 42);
		//DisplayTextCenteredWithBack("Core:" +s $speed["temps"][8]+s "�c", 250, 42);
		DisplayTextCenteredWithBack("HD0:" +s $speed["temps"][0]+s "�c", 160, 60);
		DisplayTextCenteredWithBack("HD1:" +s $speed["temps"][5]+s "�c", 250, 60);
		*/

		// delay showing the drive info at startup as this function is quite slow when first executed.
		//if (GetTickCount() - %driveDelayShow > 2000)
		//	DisplayDriveInfo(315, 23, 460, 84, -4);

		//DisplayTextCentered(GetRemoteIP(), 240, 77);

		/*SetDrawColor(RGB(0,255,0));
		DisplayTextWithBack("Dn:"+s FormatSize(%counters.down, 1,0,1), 250, 95);
		SetDrawColor(RGB(255,0,0));
		DisplayTextWithBack("Up:"+s FormatSize(%counters.up, 1,0,1), 400, 95);
		SetDrawColor(RGB(0,0,0));*/
		
		//DrawMultiGraph(%counters.cpuGraph, 1, DHEIGHT-2, DWIDTH-2, 100, RGBA(0,0,0,0),list(0,2));
		DrawMultiGraph(%counters.cpuGraph, 1, DHEIGHT-2, (DWIDTH/2)-2, 100, RGBA(0,0,0,0),list(0,2));
		DrawMultiGraph(list(%counters.downGraph, %counters.upGraph), 241, 270, 478, 116,RGBA(0,0,0,0),list(0,2));
		
		%tcxRoute.render($res[0]+0.0, $res[1]+0.0);
	}
};

