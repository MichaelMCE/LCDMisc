#requires <Modules/SysTrayIcons.c>



function IconEventHandler($event, $param, $obj, $lParam) {
	
	if ($event ==s "WindowsMessage") {
		if ($lParam == WM_LBUTTONDBLCLK) {
			Run("taskmgr.exe",,2);

		}else if ($lParam == WM_RBUTTONUP) {
			$sep = "        ---      ";
			$res = ContextMenuWait(@GetCursorPos(), "Save snapshot", $sep, "Lock Workstation", "Restart LCDMisc.", $sep, "Quit");
			if ($res == 0){
				$name = FormatTime("DDMMYY") +s "_" +s FormatTime("HHNNSS") +s ".bmp";
				SaveSnapshot($name);
				
			}else if ($res == 1){
				
			}else if ($res == 2){
				LockSystem();
				
			}else if ($res == 3){
				if (Is64Bit(Is64Bit_LCDMISC))
					Run("lcdmisc64.exe",,2);
				else
					Run("lcdmisc.exe",,2);
			}else if ($res == 4){
				
			}else if ($res == 5){
				Quit();
			}
		}
	}
}

function CpuIconHandler($event, $param, $obj, $res) {
	if ($event ==s "DrawIcon")
		DrawMultiGraph(GetCounterManager().cpuGraph, 0, 15, 15, 0);
	else
		IconEventHandler(@$);
}

function CpuIconTotalHandler($event, $param, $obj, $res) {
	if ($event ==s "DrawIcon") {
		ColorRect(0, 0, 15, 15, RGB(0,0,96));
		SetDrawColor(RGB(130, 180, 255));
		GetCounterManager().cpuTotalGraph.Draw(-1, 16, 16, -1);
	}else{
		IconEventHandler(@$);
	}
}

function CpuNetIconHandler($event, $param, $obj, $res) {
	if ($event ==s "DrawIcon") {
		$counters = GetCounterManager();
		DrawMultiGraph(list($counters.downGraph, $counters.upGraph), 0, 15, 15, 0, RGB(255,255,255));
	}else{
		IconEventHandler(@$);
	}
}

function AddCpuIcon() {
	SysTrayIcon("Core Usage", "CpuIconHandler");
}

function AddTotalCpuIcon() {
	SysTrayIcon("CPU Usage", "CpuIconTotalHandler");
}

function AddNetIcon() {
	SysTrayIcon("Network Bandwidth", "CpuNetIconHandler");
}


