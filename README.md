

LCD Miscellany 0.4.6 <br />
LCD Miscellany is a tool allowing one to write scripts to display stuff on external displays or as desktop windows <br />
 <br />
![screenshots of app](./screenshots/171119_221042.png)
![screenshots of app](./screenshots/applauncher1.png)
![screenshots of app](./screenshots/mediaplayer_video.png)
![screenshots of app](./screenshots/weather.png)
![screenshots of app](./screenshots/status_aclock.png)
![screenshots of app](./screenshots/rss.png)
![screenshots of app](./screenshots/taskman.png)
![screenshots of app](./screenshots/control.png)

Changes: <br />

Added Device Arrive and Depart notification event ("DeviceChange"). Useful to detect when a usb device is inserted/removed. <br />
Added experimental serial port access via "ComPort" object. <br />
Added a Garmin Traianing Center datafile parser class (.tcx files). <br />
Modified GetTickCount/NoOverflow() to use GetTickCount64() <br />
Added IsImage(obj) and IsImage32(obj) <br />
Added SetForegroundWindow(hWnd) and ShowWindow((hWnd, SW_ ) <br />
Added GetFocussedProcess(). Returns filepath of topmost window. <br />
Added IsAdmin(). Returns true if current user is a local administrator. <br />
Added GetDesktopInfo(). Returns a list containing width, height, bpp and hz. <br />
Added Is64Bit(flags). Returns whether operating system or LCDMisc is 64bit (flags:Is64Bit_OS and Is64Bit_LCDMISC) <br />
Added GetPowerStatus(). Returns a list containing SYSTEM_POWER_STATUS <br />
Added SaveSnapshot(path) <br />
Added Drive & Media change notification events ("DriveChange"). Sent upon on storage media insertion or removal <br />
Added GetChar(), Suspend(), Shutdown(), AbortShutdown() and LockSystem() to the HTML documents. <br />
Fixed a tokenizer bug with out of bounds access when processing "|x####" strings <br />
Added Load support for .PSD bitmap images <br />
Image32's may now be drawn on to other images <br />
Expanded Image32 class, adding Fade(), Draw(), Clear(), Copy(), Blur(), FLip() and Pixelize(). <br />
Added NewImage32() <br />
Removed font size limitation <br />
Removed intrinsic support for Logitech devices. They may be added through a plugin (but not by me). <br />
Fixed image loading with absolute paths' <br />
Added example App. launcher view/script based upon the app launcher class. <br />
Added an icon based application launcher class. <br />
OVerhauled all graphics and script view layouts to better suit larger colour displays. <br />
Added Button and Icon classes. <br />
Created a Code::Blocks project file for use with MinGW32/64 <br />



From LCDMisc. 4.5.0 <br />
LCDMisc. for the SwitchBlade SBUI by Michael McElligott <br />


From LCDMisc. 4.4.0 <br />
LCDMisc. for the USBD480 <br />
LCDMisc for the USBD480 is a customization of the LCDMisc v4.4.3 scripts; adding an output plugin, touch panel control, full colour and resolution support for the USBD480, along with additional views and graphics. <br />

LCDMisc. 4.4.0 <br />
LCD Misc. is written & (c) by Matt Menke. <br />
Original project page: http://code.google.com/p/lcdmiscellany/ <br />




