


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <mylcd.h>
#include "plugin.h"



#define DTITLE				"LCDMisc"
#define EXPORT	__declspec(dllexport)



const char *device[] = {"DDRAW"};
#define DTOTAL			1
#define DWIDTH			480
#define DHEIGHT			272
#define DBPP			LFRM_BPP_32




static THWD *hw = NULL;
static TFRAME *frame = NULL;
static TRECT display;
static lDISPLAY did;
static LcdCallbacks *LMC = NULL;
static int isShuttingdown = 0;
static int initialized = 0;
static LcdInfo windows;



static inline void resetCurrentDirectory ()
{
	wchar_t drive[MAX_PATH+1];
	wchar_t dir[MAX_PATH+1];
	wchar_t szPath[MAX_PATH+1];
	GetModuleFileNameW(NULL, szPath, MAX_PATH);
	
	_wsplitpath(szPath, drive, dir, NULL, NULL);
	swprintf(szPath, L"%s%s", drive, dir);
	
	//wprintf(L"resetCurrentDirectory: '%s'\n", szPath);
	SetCurrentDirectoryW(szPath);
}


static inline int initLibrary ()
{
    if (!(hw=lOpen(NULL, NULL))){
    	return 0;
    }
   	return 1;
}

static inline int libmylcd_init ()
{
	if (!initLibrary())
		return 0;

	display.left = 0;
	display.top = 0;
	display.right = DWIDTH-1;
	display.btm = DHEIGHT-1;

	did = lSelectDevice(hw, device[0], "NULL", DWIDTH, DHEIGHT, DBPP, 0, &display);

	frame = lNewFrame(hw, DWIDTH, DHEIGHT, DBPP);
    return (did && frame != NULL);
}

static inline void libmylcd_shutdown ()
{
	//printf("libmylcd_shutdown %i\n", did);
	
	isShuttingdown = 1;
	lCloseDevice(hw, did);
	lDeleteFrame(frame);
//	lClose(hw);
	
	initialized = 0;
	did = 0;
	frame = NULL;
	hw = NULL;
}

EXPORT CALLBACK int Init ()
{
	return 1 /*did*/;
}

void CALLBACK Update (LcdInfo *info, BitmapInfo *bmp)
{
	//memcpy(frame->pixels, bmp->bitmap, frame->frameSize);

	//lSaveImage(frame, L"lcdmisc.bmp", IMG_BMP, 0, 0);
	//lRefreshAsync(frame, 1);
	if (!isShuttingdown && initialized)
		lUpdate(hw, bmp->bitmap, frame->frameSize);
}

void CALLBACK Destroy (LcdInfo *info)
{
	isShuttingdown = 1;
	initialized = 0;
	LMC = NULL;
}

void InitWindow (LcdInfo *win, const char *name)
{	
	memset(win, 0, sizeof(LcdInfo));
	win->bpp = 32;
	win->refreshRate = 0;
	win->id = name;
	win->width = DWIDTH;
	win->height = DHEIGHT;
	win->Update = Update;
	win->Destroy = Destroy;
	initialized = libmylcd_init();
}

EXPORT CALLBACK int lcdInit (LcdCallbacks *LCDCallbacks)
{
	LMC = LCDCallbacks;
	InitWindow(&windows, DTITLE);
	resetCurrentDirectory();
	return 1;
}

EXPORT CALLBACK void lcdUninit ()
{
	initialized = 0;
	libmylcd_shutdown();
}

EXPORT CALLBACK LcdInfo * lcdEnum ()
{
	if (initialized == 1){
		initialized = 2;
		return &windows;
	}
	return NULL;
}


#if 1
EXPORT BOOL WINAPI DllMain (HINSTANCE hInstance, DWORD fdwReason, void *lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hInstance);
	}else if (fdwReason == DLL_PROCESS_DETACH) {
		//lcdUninit();
		(void)lpvReserved;
	}
    return 1;
}
#endif


