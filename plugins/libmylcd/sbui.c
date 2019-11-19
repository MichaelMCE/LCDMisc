


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <mylcd.h>
#include "plugin.h"



#define DTITLE				"LCDMisc"




#include "libmylcd/sbui20/sbuicb.h"
//#include "libmylcd/sbui153/sbuicb.h"



const char *device[] = {"SBUI20", "SBUI153"};
#define DTOTAL		1
#define DWIDTH		800
#define DHEIGHT		480
#define DBPP		LFRM_BPP_32

#define EXPORT	__declspec(dllexport)


static THWD *hw = NULL;
static TFRAME *frame = NULL;
static TRECT display;
static lDISPLAY did;
static LcdCallbacks *LMC = NULL;
static int isShuttingdown = 0;
static int initialized = 0;
static LcdInfo windows;
static uint32_t lastDt = 0;

static inline int libmylcd_init ();
static inline void libmylcd_shutdown ();
static inline int initLibrary ();





static inline lDISPLAY getDriverId ()
{
	lDISPLAY Did = lDriverNameToID(hw, "sbui20", LDRV_DISPLAY);
	if (!Did) Did = lDriverNameToID(hw, "sbui153", LDRV_DISPLAY);
	return Did;
}

static inline void resetCurrentDirectory ()
{
	wchar_t drive[MAX_PATH+1];
	wchar_t dir[MAX_PATH+1];
	wchar_t szPath[MAX_PATH+1];
	GetModuleFileNameW(NULL, szPath, MAX_PATH);
	
	_wsplitpath(szPath, drive, dir, NULL, NULL);
	__mingw_swprintf(szPath, L"%s%s", drive, dir);
	
	//wprintf(L"resetCurrentDirectory: '%s'\n", szPath);
	SetCurrentDirectoryW(szPath);
}


EXPORT CALLBACK int sbuiDynamicKeySetImage (const int key, const int state, wchar_t *path)
{
	//wprintf(L"sbuiDynamicKeySetImage: %i %i '%s'\n", key, state, path);
	if (!did || !hw) return 0;
	
	if (state == SBUI_DK_UP || state == SBUI_DK_DOWN){
		
		wchar_t drive[MAX_PATH+1];
		wchar_t dir[MAX_PATH+1];
		wchar_t szPath[MAX_PATH+1];
		GetModuleFileNameW(NULL, szPath, MAX_PATH);
		_wsplitpath(szPath, drive, dir, NULL, NULL);
		__mingw_swprintf(szPath, L"%s%s\\%s", drive, dir, path);
		
		TSBGESTURESETDK sbdk;
		sbdk.size = sizeof(TSBGESTURESETDK);
		sbdk.dk = key;
		sbdk.state = state;
		sbdk.path = szPath;
		int ret = lSetDisplayOption(hw, did, lOPT_SBUI_SETDK, (intptr_t*)&sbdk);
		lSleep(1);
		//printf("		lSetDisplayOption returned %i\n", ret);
		return ret;
	}
	return 0;
}

EXPORT CALLBACK int sbuiSetGestureCfg (const int op, const int gesture, const int state)
{
	int ret = 0;

	if (!did) did = getDriverId();
	if (did){
		TSBGESTURECBCFG sbcfg;
		sbcfg.op = op;
		sbcfg.gesture = gesture;
		sbcfg.state = state;
		ret = lSetDisplayOption(hw, did, lOPT_SBUI_GESTURECFG, (intptr_t*)&sbcfg);
	}
	return ret;
}

EXPORT CALLBACK int sbuiReconnect ()
{
	return lSetDisplayOption(hw, did, lOPT_SBUI_RECONNECT, NULL);
}

EXPORT CALLBACK int Init ()
{
	return did;
}

void CALLBACK Update (LcdInfo *info, BitmapInfo *bmp)
{
	//memcpy(frame->pixels, bmp->bitmap, frame->frameSize);

	//lSaveImage(frame, L"lcdmisc.bmp", IMG_BMP, 0, 0);
	//lRefreshAsync(frame, 1);
	if (initialized)
		lUpdate(hw, bmp->bitmap, frame->frameSize);
}

void CALLBACK Destroy (LcdInfo *info)
{
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
	lastDt = GetTickCount();
}

EXPORT CALLBACK LcdInfo * lcdEnum ()
{
	if (initialized == 1){
		initialized = 2;
		return &windows;
	}
	return NULL;
}

static inline int initLibrary ()
{
    if (!(hw=lOpen(NULL, NULL))){
    	return 0;
    }
   	return 1;
}

static int sbuiGestureCB (const TSBGESTURE *sbg, void *ptr)
{
	if (isShuttingdown || !sbg) return 0;

	if (sbg->type != SBUICB_GESTURE_PRESS)
		return 0;

	TTOUCHCOORD pos;
	pos.x = sbg->x;
	pos.y = sbg->y;
	pos.z1 = sbg->z;
	pos.z2 = sbg->z;
	pos.time = sbg->time;
	pos.dt = sbg->dt;
	pos.count = sbg->ct;
	pos.id = sbg->id;
	pos.pen = 1 & ~sbg->params;
	pos.pressure = 100;

	char eventArg[64];
	__mingw_snprintf(eventArg, 64, "%i,%i,%i,%i", pos.x, pos.y, (int)pos.dt, (int)pos.time);


	if (sbg->type == SBUICB_GESTURE_PRESS){
		if (!pos.pen){
			if (pos.dt > 80){
				LMC->TriggerEvent(LMC->id, "touchDown", eventArg);
			}else{
				LMC->TriggerEvent(LMC->id, "touchMove", eventArg);
			}
		}else{
			LMC->TriggerEvent(LMC->id, "touchUp", eventArg);
		}
	}

	return 1;
}

int sbuiDKCB (const int key, const int state, void *ptr)
{
	if (isShuttingdown) return 0;
	
	//printf("sbuiDKCB: key %i, state %i, %p\n", key, state, ptr);


	if (key == SBUI_DK_CLOSE){
		LMC->TriggerEvent(LMC->id, "sbuiClose", "");
		
	}else if (key == SBUI_DK_EXIT){
		LMC->TriggerEvent(LMC->id, "sbuiExit", "");
		
	}else if (key == SBUI_DK_ACTIVATE){
		LMC->TriggerEvent(LMC->id, "sbuiActivate", "");
		//LMC->TriggerEvent(LMC->id, "quit", "");

	}else if (key == SBUI_DK_DEACTIVATE){
		LMC->TriggerEvent(LMC->id, "sbuiDeactivate", "");
		
	}else if ((key >= SBUI_DK_1 && key <= SBUI_DK_10) && (state == SBUI_DK_UP || state == SBUI_DK_DOWN)){

		char eventArg[64];
		snprintf(eventArg, 64, "%i,%i", key, state);
		LMC->TriggerEvent(LMC->id, "sbuiDK", eventArg);
		
		snprintf(eventArg, 64, "%i", key);
		
		if (state == SBUI_DK_DOWN)
			LMC->TriggerEvent(LMC->id, "sbuiDKDown", eventArg);
		else if (state == SBUI_DK_UP)
			LMC->TriggerEvent(LMC->id, "sbuiDKUp", eventArg);
	}
	
	return 1;
}

static int sbuiDKCBEnable ()
{
	if (!did) did = getDriverId();

	if (did){
		if (lSetDisplayOption(hw, did, lOPT_SBUI_UDATAPTR, (intptr_t*)NULL)){
			if (lSetDisplayOption(hw, did, lOPT_SBUI_DKCB, (intptr_t*)sbuiDKCB)){
				return 1;
			}
		}
	}
	return 0;
}

static int sbuiGestureCBEnable ()
{
	if (!did) did = getDriverId();
	
	if (did){
		//if (lSetDisplayOption(hw, did, lOPT_SBUI_UDATAPTR, (intptr_t*)vp)){
			if (lSetDisplayOption(hw, did, lOPT_SBUI_GESTURECB, (intptr_t*)sbuiGestureCB)){
				
				sbuiSetGestureCfg(SBUICB_OP_GestureEnable, SBUICB_GESTURE_PRESS, SBUICB_STATE_ENABLED);
				//sbuiSetGestureCfg(SBUICB_OP_GestureEnable, SBUICB_GESTURE_TAP, SBUICB_STATE_ENABLED);
				//sbuiSetGestureCfg(SBUICB_OP_GestureEnable, SBUICB_GESTURE_FLICK, SBUICB_STATE_ENABLED);
				//sbuiSetGestureCfg(SBUICB_OP_GestureEnable, SBUICB_GESTURE_ROTATE, SBUICB_STATE_DISABLED);
				//sbuiSetGestureCfg(SBUICB_OP_GestureEnable, SBUICB_GESTURE_ZOOM, SBUICB_STATE_DISABLED);

				//sbuiSetGestureCfg(SBUICB_OP_GestureSetNotification, SBUICB_GESTURE_FLICK, SBUICB_STATE_ENABLED);
				//sbuiSetGestureCfg(SBUICB_OP_GestureSetNotification, SBUICB_GESTURE_TAP, SBUICB_STATE_ENABLED);
				//sbuiSetGestureCfg(SBUICB_OP_GestureSetNotification, SBUICB_GESTURE_PRESS, SBUICB_STATE_ENABLED);

				//sbuiSetGestureCfg(SBUICB_OP_GestureSetOSNotification, SBUICB_GESTURE_ROTATE, SBUICB_STATE_DISABLED);
				//sbuiSetGestureCfg(SBUICB_OP_GestureSetOSNotification, SBUICB_GESTURE_FLICK, SBUICB_STATE_DISABLED);
				//sbuiSetGestureCfg(SBUICB_OP_GestureSetOSNotification, SBUICB_GESTURE_TAP, SBUICB_STATE_DISABLED);
				//sbuiSetGestureCfg(SBUICB_OP_GestureSetOSNotification, SBUICB_GESTURE_ZOOM, SBUICB_STATE_DISABLED);
				//sbuiSetGestureCfg(SBUICB_OP_GestureSetOSNotification, SBUICB_GESTURE_PRESS, SBUICB_STATE_DISABLED);
				
				return 1;
			}
		//}
	}else{
		// printf("SwitchBladeUI not found\n");
	}
	return 0;
}

static inline void sbuiGestureCBDisable ()
{
	if (!did) did = getDriverId();

	if (did){
		sbuiSetGestureCfg(SBUICB_OP_GestureSetNotification, SBUICB_GESTURE_PRESS, SBUICB_STATE_DISABLED);
		sbuiSetGestureCfg(SBUICB_OP_GestureEnable, SBUICB_GESTURE_PRESS, SBUICB_STATE_DISABLED);
		lSetDisplayOption(hw, did, lOPT_SBUI_GESTURECB, 0);
	}
}

static inline int libmylcd_init ()
{
	if (!initLibrary())
		return 0;

	display.left = 0;
	display.top = 0;
	display.right = DWIDTH-1;
	display.btm = DHEIGHT-1;
	did = 0;
	
	for (int i = 0; i < DTOTAL; i++){
		did = lSelectDevice(hw, device[i], "NULL", DWIDTH, DHEIGHT, DBPP, 0, &display);
		if (!did)
			did = lSelectDevice(hw, device[i], "NULL", DWIDTH, DHEIGHT, DBPP, 1, &display);
		if (did) break;
	}

	if (did){
		sbuiGestureCBEnable();
		sbuiDKCBEnable();
	}

	if (!did)
		did = lSelectDevice(hw, "DDRAW", "NULL", DWIDTH, DHEIGHT, DBPP, 0, &display);

	frame = lNewFrame(hw, DWIDTH, DHEIGHT, DBPP);
    return (did && frame != NULL);
}

static inline void libmylcd_shutdown ()
{
	//printf("libmylcd_shutdown %i\n", did);
	
	isShuttingdown = 1;
	sbuiGestureCBDisable();
	lCloseDevice(hw, did);
	lDeleteFrame(frame);
	lClose(hw);
	
	initialized = 0;
	did = 0;
	frame = NULL;
	hw = NULL;
	lastDt = 0;
}

#if 0
EXPORT CALLBACK BOOL WINAPI DllMain (HINSTANCE hInstance, DWORD fdwReason, void *lpvReserved)
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



