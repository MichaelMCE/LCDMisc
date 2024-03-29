// uTorrent view.  Had planned on adding data from other sources, initially.

#import <Views\View.c>
#import <constants.h>
#requires <util\Text.c>
#requires <util\G15.c>
#requires <util\html.c>

struct DownloadInfo {
	var %hash,
		%name,
		%fileSize,
		%have,
		%uploaded,
		%ratio,
		%upstream,
		%downstream,
		%queuePos,
		%remaining,
		%status,
		%entry,
		%order;

	function DownloadInfo() {}
};

#define DOWNLOAD_COMPLETE			0
#define DOWNLOAD_ERROR				2
#define DOWNLOAD_STOPPED			4
#define DOWNLOAD_PAUSED				6
#define DOWNLOAD_CHECKING			8
#define DOWNLOAD_QUEUED				10
#define DOWNLOAD_SEEDING			12
#define DOWNLOAD_SEEDING_FORCED		13
#define DOWNLOAD_DOWNLOADING		14
#define DOWNLOAD_DOWNLOADING_FORCED 15

struct DownloadView extends View {
	var %url, %downloads, %sel, %talking, %queuedTalk, %token, %errorCount;
	var %pageHeight;

	function DownloadView ($_url) {
		%InitFonts();
		%InitImages();
		if (!IsString($_url)) {
			%url = GetString("URLs", "uTorrent");
		}
		else {
			%url = $_url;
		}
		%noDrawOnAudioChange = 1;

		// Actually do draw, but handles the message itself.
		%noDrawOnCounterUpdate = 1;
	}

	function Update() {
		if (%talking) {
			WriteLogLn(1);
			%queuedTalk = 1;
			return;
		}
		%talking = 1;
		%queuedTalk = 0;
		if (!size(%token)) {
			$temp = HttpGetWait(%url +s "?list=1");
		}
		else {
			$temp = HttpGetWait(%url +s "?token=" +s %token +s "&list=1");
		}

		if (!size($temp) || !size($json = JSONdecode($temp)) || !IsList($entries = $json["torrents"])) {
			if (size(%token)) {
				%token = null;
			}
			else {
				$tokenHtml = HttpGetWait(%url +s "token.html");
				if (!size($tokenHtml) || !size(%token = StripHTML($tokenHtml))) {
					%token = null;
				}
			}
			%talking = 0;
			if (%queuedTalk) {
				%queuedTalk = 0;
				SpawnThread("Update", $this);
			}
			%errorCount++;
			if (%errorCount >= 2) {
				%downloads = null;
				NeedRedraw();
			}
			return;
		}
		NeedRedraw();
		%errorCount = 0;
		if (IsNull($temp)) {
			%downloads = null;
		}
		else {
			$pos = 0;
			%downloads = list();

			$entries ;
			if (!IsList($entries)) {
				%token = null;
			}
			else {
				while ($pos < size($entries)) {
					$entry = $entries[$pos];
					$e = %downloads[$pos] = DownloadInfo();
					$pos ++;
					$e.hash     = $entry[0];
					$e.name     = $entry[2];
					$e.fileSize = $entry[3];
					$e.have     = $entry[5];
					$e.remaining= $entry[18];
					$e.uploaded = $entry[6];
					$e.ratio    = $entry[7]/1000.0;

					$e.upstream = $entry[8];
					$e.downstream = $entry[9];
					$status = $entry[1];

					// Basic code from Pleh on utorrent forums.
					if ($status & 1) {
						if ($status & 32) {
							$e.status = DOWNLOAD_PAUSED;
						}
						else if ($status & 64) {
							if ($e.remaining)
								$e.status = DOWNLOAD_DOWNLOADING;
							else
								$e.status = DOWNLOAD_SEEDING;
						}
						else {
							if ($e.remaining)
								$e.status = DOWNLOAD_DOWNLOADING_FORCED;
							else
								$e.status = DOWNLOAD_SEEDING_FORCED;
						}
					}
					else if ($t & 2) {
						$e.status = DOWNLOAD_CHECKING;
					}
					else if ($t & 16) {
						$e.status = DOWNLOAD_ERROR;
					}
					else if ($t & 64) {
						$e.status = DOWNLOAD_QUEUED;
					}
					else if (!$e.remaining) {
						$e.status = DOWNLOAD_COMPLETE;
					}
					else {
						$e.status = DOWNLOAD_STOPPED;
					}

					$e.order = $entry[17];
					if ($e.order < 0) {
						$e.order = 0xFFFFFFFF - ($e.status|1);
					}
				}
				for ($i = 1; $i < size(%downloads); $i++) {
					$entry = %downloads[$i];
					for ($j = $i; $j > 0; $j--) {
						//if (%downloads[$j-1].status > $entry.status) break;
						if (//%downloads[$j-1].status == $entry.status &&
							%downloads[$j-1].order < $entry.order) break;
						if (%downloads[$j-1].order == $entry.order &&
							%downloads[$j-1].name <s $entry.name) {
								break;
						}
						%downloads[$j] = %downloads[$j-1];
					}
					%downloads[$j] = $entry;
				}
			}
		}
		%talking = 0;
		if (%queuedTalk) {
			%queuedTalk = 0;
			SpawnThread("Update", $this);
		}
	}

	function CounterUpdate() {
		%Update();
	}

	function Show() {
		// Update to blank faster when changing windows.
		if (%errorCount) %downloads = null;
		SpawnThread("CounterUpdate", $this);
	}

	function PerformActionWait($index, $action) {
		if ($index >= 0 && $index < size(%downloads)) {
			return HttpGetWait(%url +s "?action=" +s $action +s "&hash=" +s %downloads[$index].hash);
		}
	}

	function KeyDown($event, $param, $modifiers, $vk) {
		$vk = MediaFlip($vk);
		$last = size(%downloads)-1;
		if ($vk == VK_VOLUME_DOWN || $vk == VK_UP) {
			if (size(%downloads)) {
				%sel --;
				if (%sel < 0) %sel = size(%downloads)-1;
				NeedRedraw();
			}
		}
		else if ($vk == VK_VOLUME_UP || $vk == VK_DOWN) {
			if (size(%downloads)) {
				%sel = (%sel + 1) % size(%downloads);
				NeedRedraw();
			}
		}
		else if ($vk == VK_MEDIA_STOP) {
			%PerformActionWait(%sel, "stop");
			%CounterUpdate();
		}
		else if ($vk == VK_MEDIA_PLAY_PAUSE) {
			$s = %downloads[%sel].status;
			if ($s >= DOWNLOAD_QUEUED) {
				%PerformActionWait(%sel, "pause");
			}
			else {
				%PerformActionWait(%sel, "start");
			}
			%CounterUpdate();
		}
		else if ($vk == VK_HOME) {
			%sel = 0;
		}
		else if ($vk == VK_END) {
			%sel = $last;
		}
		else if ($vk == VK_PRIOR) {
			%sel -= %pageHeight;
			if (%sel < 0) %sel = 0;
		}
		else if ($vk == VK_NEXT) {
			%sel += %pageHeight;
			if (%sel > $last) %sel = $last;
		}
		else return 0;

		NeedRedraw();
		return 1;
	}

	function Focus() {
		if (!%hasFocus) {
			RegisterKeyEvent(0, VK_HOME, VK_END, VK_PRIOR, VK_NEXT, VK_DOWN, VK_UP, VK_VOLUME_DOWN, VK_VOLUME_UP, VK_MEDIA_STOP, VK_MEDIA_PLAY_PAUSE);
			%hasFocus = 1;
		}
	}

	function Unfocus() {
		if (%hasFocus) {
			UnregisterKeyEvent(0, VK_HOME, VK_END, VK_PRIOR, VK_NEXT, VK_DOWN, VK_UP, VK_VOLUME_DOWN, VK_VOLUME_UP, VK_MEDIA_STOP, VK_MEDIA_PLAY_PAUSE);
			%hasFocus = 0;
		}
	}

	function G15ButtonDown($event, $param, $button, $keyboard) {
		$button = FilterButton($button);
		if ($button & 0x3F) {
			if ($button == G15_LEFT) {
				if (HasAllDirections($keyboard)) {
					%KeyDown(0,0,0, VK_PRIOR);
				}
				else {
					%KeyDown(,,, VK_UP);
				}
			}
			else if ($button == G15_RIGHT) {
				if (HasAllDirections($keyboard)) {
					%KeyDown(0,0,0, VK_NEXT);
				}
				else {
					%KeyDown(,,, VK_DOWN);
				}
			}
			else if ($button == G15_UP || $button == G15_LEFT) {
				%KeyDown(,,, VK_UP);
			}
			else if ($button == G15_DOWN || $button == G15_RIGHT) {
				%KeyDown(,,, VK_DOWN);
			}
			else if ($button == G15_OK) {
				%KeyDown(,,, VK_MEDIA_PLAY_PAUSE);
			}
			else if ($button == G15_CANCEL) {
				%Unfocus();
			}
			NeedRedraw();
			return 1;
		}
	}

	function Draw($event, $param, $name, $res) {
		ClearScreen();
		$right = $res[0]-1;
		$bottom = $res[1]-1;
		$bpp = $res[2];

		$highRes = IsHighRes(@$res);
		UseThemeFont(%fontIds[$highRes]);

		$fh = GetFontHeight();
		%pageHeight = $bottom / $fh;

		if (IsNull(%downloads)) {
			DisplayText("Not connected to uTorrent.");
		}
		else if (!size(%downloads)) {
			DisplayText("No torrents in list.");
		}
		else {
			if (%sel >= size(%downloads)) {
				%sel = size(%downloads)-1;
				if (%sel < 0) %sel = 0;
			}

			$pageLen = $bottom / $fh;

			$i = (%sel - %sel%$pageLen);
			$end = $i+6;
			$h = 0;
			$status = "C E H P V Q SSDD";
			$colLen = TextSize(" 0.00M")[0]+1;
			$col5 = $right;
			$col4 = $right - $colLen;
			$col3 = $col4 - $colLen;
			$col2 = $col3 - $colLen;
			$col1 = $col2 - TextSize(" S")[0]-1;

			if ($bpp>1) {
				ColorRect($col4, 0, $right, $bottom, colorColumnBg[0]);
				ColorRect($col3, 0, $col4+2, $bottom, colorColumnBg[1]);
				ColorRect($col2, 0, $col3+2, $bottom, colorColumnBg[0]);
				ColorRect($col1, 0, $col2+2, $bottom, colorColumnBg[1]);
				ColorRect(0, 0, $col1+2, $bottom, colorFirstColumnBg);
				$col5-=2;
				$col4--;
				$col3--;
			}

			for (; $h < $bottom && $i<size(%downloads); $h+=$fh) {
				if ($i == %sel) {
					if (%hasFocus) {
						$bgColor = colorHighlightBg;
						SetDrawColor(colorHighlightText);
					}
					else if ($bpp>1) {
						$bgColor = colorHighlightUnfocusedBg;
						SetDrawColor(colorHighlightUnfocusedText);
					}
					ColorRect(0, $h, $right, $h+$fh, $bgColor);
				}
				$entry = %downloads[$i];
				DrawClippedText($entry.name, 1, $h, $col1, $h+$fh);

				DisplayTextRight($status[$entry.status], $col2, $h);
				DisplayTextRight(FormatSize($entry.remaining, 2,0,1), $col3, $h);
				DisplayTextRight(FormatSize($entry.downstream,1,0,1), $col4, $h);
				DisplayTextRight(FormatSize($entry.upstream,1,0,1), $col5, $h);
				if ($i == %sel) {
					SetDrawColor(colorText);
				}
				$i++;
			}
		}
	}
};
