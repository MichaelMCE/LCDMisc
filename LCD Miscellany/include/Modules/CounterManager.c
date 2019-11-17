#requires <Modules\Graph.c>
// Updates graphs at constant rates.  May add long term graphs at some point.
// To add to this, you can use inheritance instead of creating a new class.
// Note that you will have to either replace GetCounterManager() calls in
// the classes that use it, or (preferably) set _counterManager to point to
// an object of the new class before initializing views.

struct CounterManager {
	var %cpu, %cpuGraph, %downGraph, %upGraph, %miniFont, %cpuData, %down, %up, %cpuTotalGraph;
	var %cpuSum;
	function CounterManager() {
		%cpu = PerformanceCounter("Processor", "*", "% Processor Time");
		%cpuGraph = list();
		%cpuTotalGraph = Graph(0,0,100, 272, 3);
		%downGraph = Graph(0,0,1.5*1024*1024, 272, 3);
		%upGraph = Graph(0,0,128*1024, 272, 3);
		eventHandler.Insert(-1, $this);
	}

	function CPUUpdate() {
		%cpuData = %cpu.GetValue();
		if (!size(%cpuGraph[$i])) {
			for ($i = 0; $i<size(%cpuData)-1; $i++)
				%cpuGraph[$i] = Graph(0,0,100, DWIDTH/2, 3);
		}

		%cpuTotalGraph.Update(%cpuData["_Total"]);
		for ($i = size(%cpuData)-2; $i >= 0; $i--) {
			%cpuGraph[$i].Update(%cpuData[$i]);
		}
	}

	function NetworkUpdate() {
		//%down = GetDownstream();
		%down = GetAllDown()["RT73 USB Wireless LAN Card #2"];
		%downGraph.Update(%down);

		//%up = GetUpstream();
		%up = GetAllUp()["RT73 USB Wireless LAN Card #2"];
		%upGraph.Update(%up);
	}

	function CounterUpdate() {
		%CPUUpdate();
		%NetworkUpdate();
	}
};

function GetCounterManager() {
	if (IsNull(_counterManager)) {
		_counterManager = CounterManager();
	}
	return _counterManager;
}

