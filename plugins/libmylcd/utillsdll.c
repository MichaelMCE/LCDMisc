



#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <mylcd.h>
#include <psapi.h>
#include "utillsdll.h"
#include "plugin.h"
#include "scriptval.h"



#define EXPORT					__declspec(dllexport)
//#define LIBMYLCDVLCSTREAM		L"Software\\libmylcd\\Vlcstream\\"
#define LIBMYLCDVLCSTREAM		L"SOFTWARE\\Wow6432Node\\libmylcd\\Vlcstream"
#define VLCFILENAME				L"\\vlc.exe"
#define VLCFILENAME3			L"\\vlc3.exe"


/*
int UTF16ToUTF8 (const wchar_t *in, const size_t ilen, char *out, size_t olen)
{
	LPSTR abuf = NULL;
	int len = WideCharToMultiByte(CP_UTF8, 0, in, ilen, NULL, 0,  0, 0);
	if (len > 0)
		abuf = out;
	else
		return 0;

	int ret = WideCharToMultiByte(CP_UTF8, 0, in, ilen, abuf, len,  0, 0);
	if (ret > 0 && ret <= olen){
		out[ret] = 0;
	}	

	return (ret > 0);
}

static inline char *UTF16ToUTF8Alloc (const wchar_t *in, const size_t ilen)
{
	const size_t olen = 8 * (ilen+2);
	char *out = malloc(1+olen+sizeof(char));
	if (out){
		if (UTF16ToUTF8(in, ilen, out, olen)){
			//out = realloc(out, strlen(out)+1);
		}else{
			free(out);
			return NULL;
		}
	}
	return out;
}

static inline char *convertto8 (const wchar_t *wide)
{
	char *out = NULL;
	if (wide && *wide){
		out = UTF16ToUTF8Alloc(wide, wcslen(wide));
		if (!out){
			//out = strdup("-");
			//if (!out){
				printf("no memory, bailing\n");
				abort();
			//}
		}
	}
	return out;
}
*/

char *regGetString (const HKEY keyBranch, const wchar_t *key, const char *name, char *buffer, size_t blen)
{
	HKEY hKey = 0;
	DWORD type = REG_SZ;
	char *ret = NULL;
	
	if ((ERROR_SUCCESS == RegOpenKeyExW(keyBranch, key, 0, KEY_READ, &hKey))){
		if ((ERROR_SUCCESS == RegQueryValueEx(hKey, name, 0, &type, (LPBYTE)buffer, (PDWORD)&blen)))
			ret = buffer;
		RegCloseKey(hKey);
	}	
	return ret;
}


static inline int regSetValue (const HKEY keyBranch, const wchar_t *key, DWORD type, const wchar_t *name, const void *value)
{
	int ret = 0;
	HKEY hKey = 0;
	DWORD disp;

	if (RegCreateKeyExW(keyBranch, key, 0, NULL, 0, KEY_SET_VALUE | KEY_QUERY_VALUE, NULL, &hKey, &disp) == ERROR_SUCCESS){
		ret = RegSetValueExW(hKey, name, 0, type, (BYTE*)value, (DWORD)sizeof(int)) == ERROR_SUCCESS;
		RegCloseKey(hKey);
	}
	return ret;
}

static inline uintptr_t regGetValue (const HKEY keyBranch, const wchar_t *key, DWORD type, const wchar_t *name, void *value)
{
	uintptr_t ret = 0;
	HKEY hKey = 0;
	
	size_t blen;
	if (type == REG_SZ){
		blen = wcslen(value);
		*(wchar_t*)value = 0;
	}else{
		blen = sizeof(uintptr_t);
		*(uintptr_t*)value = -2;
	}
	
	if ((ERROR_SUCCESS == RegOpenKeyExW(keyBranch, key, 0, KEY_READ, &hKey))){
		int32_t value32 = *(uintptr_t*)value;
		ret = RegQueryValueExW(hKey, name, 0, &type, (LPBYTE)&value32, (PDWORD)&blen) == ERROR_SUCCESS;
		*(uintptr_t*)value = value32;
		RegCloseKey(hKey);
	}else{
		*(uintptr_t*)value = -3;
	}
	
	if (type == REG_SZ)
		return ret;
	else
		return *(uintptr_t*)value;
}

static inline int64_t regGetValue64 (const HKEY keyBranch, const wchar_t *key, DWORD type, const wchar_t *name, int64_t *value)
{
	int64_t ret = 0;
	HKEY hKey = 0;
	
	size_t blen;
	if (type == REG_SZ){
		blen = wcslen((wchar_t*)value);
		*(wchar_t*)value = 0;
	}else{
		blen = sizeof(int64_t);
		*value = -2;
	}
	
	if ((ERROR_SUCCESS == RegOpenKeyExW(keyBranch, key, 0, KEY_READ, &hKey))){
		ret = RegQueryValueExW(hKey, name, 0, &type, (LPBYTE)value, (PDWORD)&blen) == ERROR_SUCCESS;
		RegCloseKey(hKey);
	}else{
		*value = -3;
	}
	
	if (type == REG_SZ)
		return ret;
	else
		return *value;
}

EXPORT CALLBACK uintptr_t hrmGetValue (const wchar_t *name)
{
	uintptr_t value = 0;
	regGetValue(HKEY_LOCAL_MACHINE, LIBMYLCDVLCSTREAM, REG_DWORD, name, &value);
	return value;
}

EXPORT CALLBACK int64_t hrmGetValueQ (const wchar_t *name)
{
	int64_t value = 0;
	regGetValue64(HKEY_LOCAL_MACHINE, LIBMYLCDVLCSTREAM, REG_QWORD, name, &value);
	return value;
}


EXPORT CALLBACK void hrmGetInfo (ScriptVal *in, ScriptVal *out)
{

	MakeListValue(out);
	ScriptList *list = &out->listValue;
	
	ScriptListPushBackInt64(list, hrmGetValue(L"HRM_bpm"));
	ScriptListPushBackInt64(list, hrmGetValue(L"HRM_average"));
	ScriptListPushBackInt64(list, hrmGetValue(L"HRM_mode"));
	ScriptListPushBackInt64(list, hrmGetValue(L"HRM_low"));
	ScriptListPushBackInt64(list, hrmGetValue(L"HRM_high"));
	ScriptListPushBackInt64(list, hrmGetValue(L"HRM_bpmPrevious"));
	ScriptListPushBackInt64(list, hrmGetValueQ(L"HRM_bpmTime"));
}
/*
static inline int CALLBACK enumWindowsProc_ghk (HWND hwnd, LPARAM lParam)
{
    HWND *wnd = (HWND*)lParam;
    char name[1024];
    
    GetWindowText(hwnd, name, sizeof(name)-1);
    if (!strncmp(name, "VLC ghk ", 8)){
   	    *wnd = hwnd;
   	    return 0;
    }
    return 1;
}

static inline HWND com_getVLCHandle ()
{
	HWND hwnd = 0;
	EnumWindows(enumWindowsProc_ghk, (LPARAM)&hwnd);
	return hwnd;
}
*/
// 'Default IME'
// 'vlc'
// 'vlc ghk'
// 'MSCTFIME UI'
// 'Playlist'
// 'Adjustments and Effects'
// 'Messages'
// 'Program Guide'
// ''

static inline uintptr_t getWindowHandle (const wchar_t *window, const wchar_t *filename)
{
	HWND hWnd = FindWindowExW(0, 0, window, NULL);
	const int vlen = wcslen(filename);
	//printf("\n");

	while(hWnd){
		DWORD pid = 0;
		GetWindowThreadProcessId(hWnd, &pid);
		if (pid){
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    		if (hProcess){
				wchar_t path[MAX_PATH+1] = {0};
				GetProcessImageFileNameW(hProcess, path, MAX_PATH);
				CloseHandle(hProcess);
				if (!*path) continue;
				
				int plen = wcslen(path);
				wchar_t *file = &path[plen - vlen];

				if (!_wcsicmp(file, filename)){
					wchar_t title[2048] = {0};
					GetWindowTextW(hWnd, title, 2047);
						
					//wprintf(L"window: '%s'  %i\n", title, wcsstr(title, L" - VLC media player"));
						
					if (*title){
						wchar_t *found = wcsstr(title, L" - VLC media player");
						if (!found)
							found = wcsstr(title, L"VLC media player");
						if (found)
							return (uintptr_t)hWnd;
					}
				}
			}
		}
		hWnd = GetWindow(hWnd, GW_HWNDNEXT);
	}

	return 0;
}

EXPORT CALLBACK uintptr_t vlcGetWindowHandle ()
{
	uintptr_t hwin = getWindowHandle(L"QWidget", VLCFILENAME);
	if (!hwin)
		hwin = getWindowHandle(L"Qt5QWindowIcon", VLCFILENAME3);
	return hwin;
}

EXPORT CALLBACK intptr_t trackGetMetaInt (const intptr_t meta)
{
	intptr_t value = -1;

	switch (meta){
	case MTAG_TrackNumber: regGetValue(HKEY_LOCAL_MACHINE, LIBMYLCDVLCSTREAM, REG_DWORD, L"track_number", &value); break;
	case MTAG_ADDTIME: regGetValue(HKEY_LOCAL_MACHINE, LIBMYLCDVLCSTREAM, REG_DWORD, L"track_added", &value); break;
	};
	return value;
}

EXPORT CALLBACK char *trackGetMetaStr (const intptr_t meta)
{
	static char buffer[8192];
	*buffer = 0;
	const size_t blen = sizeof(buffer)-1;

	
	switch (meta){
	case MTAG_Title: regGetString(HKEY_LOCAL_MACHINE, LIBMYLCDVLCSTREAM, "track_title", buffer, blen); break;
	case MTAG_Album: regGetString(HKEY_LOCAL_MACHINE, LIBMYLCDVLCSTREAM, "track_album", buffer, blen); break;
	case MTAG_LENGTH: regGetString(HKEY_LOCAL_MACHINE, LIBMYLCDVLCSTREAM, "track_length", buffer, blen); break;
	case MTAG_ArtworkPath: regGetString(HKEY_LOCAL_MACHINE, LIBMYLCDVLCSTREAM, "track_artwork", buffer, blen); break;
	case MTAG_Artist: regGetString(HKEY_LOCAL_MACHINE, LIBMYLCDVLCSTREAM, "track_artist", buffer, blen); break;
	case MTAG_Genre: regGetString(HKEY_LOCAL_MACHINE, LIBMYLCDVLCSTREAM, "track_genre", buffer, blen); break;
	case MTAG_Description: regGetString(HKEY_LOCAL_MACHINE, LIBMYLCDVLCSTREAM, "track_description", buffer, blen); break;
	case MTAG_NowPlaying: regGetString(HKEY_LOCAL_MACHINE, LIBMYLCDVLCSTREAM, "track_nowPlaying", buffer, blen); break;
	case MTAG_Date: regGetString(HKEY_LOCAL_MACHINE, LIBMYLCDVLCSTREAM, "track_date", buffer, blen); break;
	case MTAG_FILENAME: regGetString(HKEY_LOCAL_MACHINE, LIBMYLCDVLCSTREAM, "track_filename", buffer, blen); break;
	case MTAG_PATH: regGetString(HKEY_LOCAL_MACHINE, LIBMYLCDVLCSTREAM, "track_path", buffer, blen); break;
	};
	
	return buffer;
}

EXPORT CALLBACK void trackGetMetaInfo (ScriptVal *in, ScriptVal *out)
{
	ScriptVal sv;
	MakeListValue(out);
	ScriptList *list = &out->listValue;


	// MTAG_Title
	MakeStringValue(&sv, trackGetMetaStr(MTAG_Title)); ScriptListPushBackSV(list, &sv);
	
	// MTAG_Artist
	MakeStringValue(&sv, trackGetMetaStr(MTAG_Artist)); ScriptListPushBackSV(list, &sv);
	
	// MTAG_Genre
	MakeStringValue(&sv, trackGetMetaStr(MTAG_Genre)); ScriptListPushBackSV(list, &sv);
	
	// MTAG_Copyright
	MakeStringValue(&sv, ""); ScriptListPushBackSV(list, &sv);
	
	// MTAG_Album
	MakeStringValue(&sv, trackGetMetaStr(MTAG_Album)); ScriptListPushBackSV(list, &sv);
	
	// MTAG_TrackNumber
	ScriptListPushBackInt64(list, trackGetMetaInt(MTAG_TrackNumber));

	// MTAG_Description
	MakeStringValue(&sv, trackGetMetaStr(MTAG_Description)); ScriptListPushBackSV(list, &sv);
	
	// MTAG_Rating
	ScriptListPushBackInt64(list, 0);	
	
	// MTAG_Date	
	MakeStringValue(&sv, trackGetMetaStr(MTAG_Date)); ScriptListPushBackSV(list, &sv);
	
	// MTAG_Setting	
	MakeStringValue(&sv, ""); ScriptListPushBackSV(list, &sv);
	
	// MTAG_URL		
	MakeStringValue(&sv, ""); ScriptListPushBackSV(list, &sv);
	
	// MTAG_Language
	MakeStringValue(&sv, ""); ScriptListPushBackSV(list, &sv);
	
	// MTAG_NowPlaying
	MakeStringValue(&sv, trackGetMetaStr(MTAG_NowPlaying)); ScriptListPushBackSV(list, &sv);
	
	// MTAG_Publisher
	MakeStringValue(&sv, ""); ScriptListPushBackSV(list, &sv);
	
	// MTAG_EncodedBy
	MakeStringValue(&sv, ""); ScriptListPushBackSV(list, &sv);
	
	// MTAG_ArtworkPath
	MakeStringValue(&sv, trackGetMetaStr(MTAG_ArtworkPath)); ScriptListPushBackSV(list, &sv);
	
	// MTAG_TrackID		
	MakeStringValue(&sv, ""); ScriptListPushBackSV(list, &sv);
	
	// MTAG_LENGTH		
	MakeStringValue(&sv, trackGetMetaStr(MTAG_LENGTH)); ScriptListPushBackSV(list, &sv);
	
	// MTAG_FILENAME	
	MakeStringValue(&sv, trackGetMetaStr(MTAG_FILENAME)); ScriptListPushBackSV(list, &sv);
	
	// MTAG_POSITION	
	ScriptListPushBackInt64(list, 0);
	
	// MTAG_PATH		
	MakeStringValue(&sv, trackGetMetaStr(MTAG_PATH)); ScriptListPushBackSV(list, &sv);
	
	// MTAG_ADDTIME		
	ScriptListPushBackInt64(list, trackGetMetaInt(MTAG_ADDTIME));	
}


EXPORT CALLBACK void Init (AppInfo *in, DllInfo *out)
{
	if (in->size < sizeof(AppInfo) || in->maxDllVersion < 1 || out->size < sizeof(DllInfo))
		return;
	
	out->free = free;
	out->dllVersion = 1;
}


BOOL WINAPI DllMain (HINSTANCE hInstance, DWORD fdwReason, void* lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH){
		DisableThreadLibraryCalls(hInstance);
	}

    return 1;
}

