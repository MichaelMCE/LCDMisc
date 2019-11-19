

#include <windows.h>
#include <stdio.h>
#include <mylcd.h>
#include "plugin.h"




#define DTITLE		"LCDMisc"



const char *device[] = {"G19", "DDRAW"};
#define DTOTAL		2
#define DWIDTH		320
#define DHEIGHT		240
#define DBPP		LFRM_BPP_32



#define EXPORT	__declspec(dllexport) 

#define G15_LEFT		  0x01
#define G15_RIGHT		  0x02
#define G15_OK			  0x04
#define G15_CANCEL		  0x08
#define G15_UP			  0x10
#define G15_DOWN		  0x20
#define G15_MENU		  0x40



static THWD *hw = NULL;
static TFRAME *frame = NULL;
static TRECT display;
static lDISPLAY did;
static LcdCallbacks *LMC = NULL;
static int isShuttingdown = 0;
static int initialized = 0;
static LcdInfo windows;
static uint32_t lastDt = 0;

int libmylcd_init ();
void libmylcd_shutdown ();
int initLibrary ();

/*
static int RED;
static int GREEN;
static int BLUE;


typedef struct{
	TLPOINTEX rt;
	char *eventName;
	int eventArg;
}TBUTTON;

#define _W  (DWIDTH-1)
#define _H  (DHEIGHT-1)

static TBUTTON buttons[] = {
	{{2, 90, 150, 180}, "G15ButtonDown", G15_LEFT},
	{{330, 90, _W-2, 180}, "G15ButtonDown", G15_RIGHT},
	{{160, 2, 320, 80}, "G15ButtonDown", G15_UP},
	{{160, 190, 320, _H-2}, "G15ButtonDown", G15_DOWN},
	{{165, 95, 315, 175}, "G15ButtonDown", G15_OK},
	{{2, _H-60, 140, _H-2}, "G15ButtonDown", G15_CANCEL},
	{{340, _H-60, _W-2, _H-2}, "G15ButtonDown", G15_MENU},
	{{0, 0, 0, 0}, NULL, 0}
};

void triggerCallback (LcdCallbacks *LMC, TBUTTON *button, TTOUCHCOORD *pos)
{
	char eventArg[32];
	sprintf(eventArg, "%i", button->eventArg);
	LMC->TriggerEvent(LMC->id, button->eventName, eventArg);
}
*/




void CALLBACK Update (LcdInfo *info, BitmapInfo *bmp)
{
	//memcpy(frame->pixels, bmp->bitmap, frame->frameSize);
	
#if 0
	TBUTTON *button = buttons;
	while (button->eventName){
		lDrawRectangleDotted(frame, button->rt.x1, button->rt.y1, button->rt.x2, button->rt.y2, RED|GREEN);
		button++;
	}
#endif

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

int initLibrary ()
{
    if (!(hw=lOpen(NULL, NULL))){
    	return 0;
    }
   	return 1;
}

int libmylcd_init ()
{

	if (!initLibrary())
		return 0;

	display.left = 0;
	display.top = 0;
	display.right = DWIDTH-1;
	display.btm = DHEIGHT-1;
	did = 0;

	for (int i = 0; i < DTOTAL; i++){
		//printf("%i: '%s'\n", i, device[i]);
		did = lSelectDevice(hw, device[i], "NULL", DWIDTH, DHEIGHT, DBPP, 0, &display);
		if (!did)
			did = lSelectDevice(hw, device[i], "NULL", DWIDTH, DHEIGHT, DBPP, 1, &display);
		if (did) break;
	}
		
	if (did){
		
	}


	if (!did)
		did = lSelectDevice(hw, "DDRAW", "NULL", DWIDTH, DHEIGHT, DBPP, 0, &display);

	frame = lNewFrame(hw, DWIDTH, DHEIGHT, DBPP);
/*	RED = lGetRGBMask(frame, LMASK_RED);
	GREEN = lGetRGBMask(frame, LMASK_GREEN);
	BLUE = lGetRGBMask(frame, LMASK_BLUE);*/
	
    return (did && frame != NULL);
}

void libmylcd_shutdown ()
{
	isShuttingdown = 1;
	//printf("libmylcd_shutdown %i\n",did);


	lCloseDevice(hw, did);
	lDeleteFrame(frame);
}

#if 0
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
