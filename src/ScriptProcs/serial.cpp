#include "../global.h"
#include <wchar.h>
#include "../ScriptObjects/ScriptObjects.h"
#include "../util.h"
#include "../unicode.h"
#include "../malloc.h"

/*
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>*/

#include <windows.h>
//#include <commctrl.h>


//#include "serial.h"


#define SERIAL_DEFAULT_Com			"com1"
#define SERIAL_DEFAULT_Baud			1200
#define SERIAL_DEFAULT_Bitsize		8
#define SERIAL_DEFAULT_StopBits		ONESTOPBIT
#define SERIAL_DEFAULT_Parity		NOPARITY


int ComPortType;


HANDLE SetupUart (const char *Port, const int baud, const int Bitsize, const int StopBits, const int Parity)
{
	int STOPBITS;

	if (StopBits == 10) STOPBITS = ONESTOPBIT;
	else if (StopBits == 15) STOPBITS = ONE5STOPBITS;
	else if (StopBits == 20) STOPBITS = TWOSTOPBITS;
	else STOPBITS = SERIAL_DEFAULT_StopBits;

	// Open the serial port.
	HANDLE hPort = CreateFileA(Port,						// Name of the port
						GENERIC_READ | GENERIC_WRITE,   // Access (read-write) mode
						0,
						NULL,
						OPEN_EXISTING,
						0,
						NULL);

	// If it fails to open the port, return 0.
	if (hPort == INVALID_HANDLE_VALUE) return NULL;

	//Get the default port setting information.
	DCB PortDCB;
	GetCommState(hPort, &PortDCB);
	// Change the settings.
	PortDCB.BaudRate = baud;              // BAUD Rate
	PortDCB.ByteSize = Bitsize;           // Number of bits/byte, 5-8
	PortDCB.Parity = Parity;              // 0-4=no,odd,even,mark,space
	PortDCB.StopBits = STOPBITS;          // StopBits
	PortDCB.fNull = 0;					  // Allow NULL Receive bytes
	PortDCB.fDtrControl = DTR_CONTROL_HANDSHAKE;
	//PortDCB.fRtsControl = RTS_CONTROL_DISABLE;


	// Re-configure the port with the new DCB structure.
	if (!SetCommState(hPort, &PortDCB)){
		CloseHandle(hPort);
		return NULL;
	}

	// Retrieve the time-out parameters for all read and write operations on the port.
	COMMTIMEOUTS CommTimeouts;
	GetCommTimeouts(hPort, &CommTimeouts);
	memset(&CommTimeouts, 0, sizeof(CommTimeouts));
	CommTimeouts.ReadIntervalTimeout = -1;
	CommTimeouts.ReadTotalTimeoutConstant = 0;
	CommTimeouts.WriteTotalTimeoutConstant = 5000;

	// Set the time-out parameters for all read and write operations on the port.
	if (!SetCommTimeouts(hPort, &CommTimeouts)){
		CloseHandle(hPort);
		return NULL;
	}

  	unsigned long comError;
	COMSTAT comstat;
	ClearCommError(hPort, &comError, &comstat);
	//SetCommMask(hPort, EV_RXCHAR |EV_TXEMPTY |EV_CTS |EV_DSR |EV_RLSD |EV_BREAK |EV_ERR |EV_RING);

	// Clear the port of any existing data.
	if (!PurgeComm(hPort, PURGE_TXCLEAR | PURGE_RXCLEAR)){
		CloseHandle(hPort);
		return NULL;
	}else{
		return hPort;
	}
}


int WriteUart (HANDLE hPort, unsigned char *buf, int len)
{
	DWORD dwNumBytesWritten = 0;
	WriteFile(hPort,				// Port handle
		       buf,					// Pointer to the data to write
			   len,					// Number of bytes to write
			   &dwNumBytesWritten,	// Pointer to the number of bytes written
			   NULL);				// Must be NULL

	long dwMask;
	WaitCommEvent(hPort, &dwMask, NULL);

	if (dwNumBytesWritten > 0)
		return dwNumBytesWritten;
	else
		return 0;
}

int ReadUart (HANDLE hPort, unsigned char *buf, int len)
{
	BOOL ret;
	unsigned long dwNumBytesRead = 0;

	long dwMask;
	WaitCommEvent(hPort, &dwMask, NULL);

	memset(buf,0,len);
	ret = ReadFile(hPort,		// handle of file to read
					buf,		// pointer to buffer that receives data
					len,		// number of bytes to read
					&dwNumBytesRead,	// pointer to number of bytes read
					NULL		// pointer to structure for data
					);

	//printf("read:-%s-\n",buf);

	if (dwNumBytesRead > 0){					//If we have data
		return (int)dwNumBytesRead;		//return the length
	}else{
		//printf("uart read error: %i\n",(int)ret);
		return 0;				//else no data has been read
	}
 }

int CloseUart (HANDLE hPort)
{
	FlushFileBuffers(hPort);
	CloseHandle(hPort);
	return 1;
}

#if 0
int Uart_SetBaud (int sbaud)
{
	int STOPBITS = 0;

	if (StopBits == 10)	STOPBITS = ONESTOPBIT;
	if (StopBits == 15) STOPBITS = ONE5STOPBITS;
	if (StopBits == 20)	STOPBITS = TWOSTOPBITS;

	DCB PortDCB;
	GetCommState (hPort, &PortDCB);
	PortDCB.BaudRate = sbaud;             // BAUD Rate
	PortDCB.ByteSize = Bitsize;           // Number of bits/byte, 5-8
	PortDCB.Parity = Parity;              // 0-4=no,odd,even,mark,space
	PortDCB.StopBits = STOPBITS;          // StopBits
	PortDCB.fNull = 0;					  // Allow NULL Receive bytes
	PortDCB.fRtsControl = RTS_CONTROL_DISABLE;
	//PortDCB.fDtrControl = DTR_CONTROL_HANDSHAKE; //DTR_CONTROL_ENABLE;

	// Re-configure the port with the new DCB structure.
	return SetCommState (hPort, &PortDCB);
}

#endif

#define objGetPortHandle(o)		((HANDLE*)((o)->values[0].intVal))
#define objGetPortName(o)		((unsigned char*)((o)->values[1].stringVal->value))
#define objGetBaud(o)			((int)((o)->values[2].i32))
#define objGetBitsize(o)		((int)((o)->values[3].i32))
#define objGetStopbits(o)		((int)((o)->values[4].i32))
#define objGetParity(o)			((int)((o)->values[5].i32))


#define objSetPortHandle(o,n)	{((o)->values[0].intVal=(int64_t)(n));((o)->values[0].type == SCRIPT_INT);}
#define objSetBaud(o,n)			{((o)->values[2].intVal=(n));((o)->values[2].type == SCRIPT_INT);}
#define objSetBitsize(o,n)		{((o)->values[3].intVal=(n));((o)->values[3].type == SCRIPT_INT);}
#define objSetStopBits(o,n)		{((o)->values[4].intVal=(n));((o)->values[4].type == SCRIPT_INT);}
#define objSetParity(o,n)		{((o)->values[5].intVal=(n));((o)->values[5].type == SCRIPT_INT);}


// (const char *Port, const int baud, const int Bitsize, const int StopBits, const int Parity)
void ComPortOpen (ScriptValue &s, ScriptValue *args)
{
	if (!CreateObjectValue(s, ComPortType))
		return;

	ObjectValue *obj = s.objectVal;

	objSetPortHandle(obj, NULL);
	CreateStringValue(obj->values[1], (const unsigned char*)SERIAL_DEFAULT_Com);
	objSetBaud(obj, SERIAL_DEFAULT_Baud);
	objSetBitsize(obj, SERIAL_DEFAULT_Bitsize);
	objSetStopBits(obj, SERIAL_DEFAULT_StopBits);
	objSetParity(obj, SERIAL_DEFAULT_Parity);


	if (args[0].stringVal->value){
		args[0].stringVal->AddRef();
		obj->values[1] = args[0];
	}
	if (args[1].i32) objSetBaud(obj, args[1].i32);
	if (args[2].i32) objSetBitsize(obj, args[2].i32);
	if (args[3].i32) objSetStopBits(obj, args[3].i32);
	if (args[4].i32) objSetParity(obj, args[4].i32);



	HANDLE hPort = SetupUart(objGetPortName(obj), objGetBaud(obj), objGetBitsize(obj), objGetStopbits(obj), objGetParity(obj));
	if (hPort){
		objSetPortHandle(obj, hPort);
		return;
	}

	obj->Release();
	CreateNullValue(s);
}

static inline int ComPortWriteVal (HANDLE *hPort, ScriptValue *args, const int svSrc)
{
	int ret = 0;

	if (args[0].type == SCRIPT_STRING){
		unsigned char *buffer = args[0].stringVal->value;
		int blen = strlen(buffer);
		if (blen < 1) return -1;

		int len = 0;
		if (svSrc == 1)
			len = min(args[1].i32, blen);
		if (len < 1) len = blen;

		//printf("str: %i %s\n", len, buffer);
		ret = WriteUart(hPort, buffer, len);

	}else if (args[0].type == SCRIPT_INT){
		unsigned char val8 = args[0].i32&0xFF;

		//printf("int: %i %c\n", val8, val8);
		ret = WriteUart(hPort, &val8, 1);

	}else if (args[0].type == SCRIPT_DOUBLE){
		unsigned char *dbl = (unsigned char*)&args[0].doubleVal;

		//printf("double: %f %f\n", *(double*)dbl, args[0].doubleVal);
		ret = WriteUart(hPort, dbl, sizeof(args[0].doubleVal));

	}else if (args[0].type == SCRIPT_LIST){
		ListValue *list = args[0].listVal;
		//printf("list: %i\n", list->numVals);

		for (int i = 0; i < list->numVals; i++){
			int written = ComPortWriteVal(hPort, &list->vals[i], 2);
			//printf("writtent: %i\n", written);
			if (written > 0) ret += written;
		}
	//}else{
	//	return ret;
	}

	return ret;
}

void ComPortWrite (ScriptValue &s, ScriptValue *args)
{
	ObjectValue *obj = s.objectVal;
	CreateIntValue(s, -1);

	HANDLE *hPort = objGetPortHandle(obj);
	if (!hPort) return;

	int ret = ComPortWriteVal(hPort, args, 1);
	if (ret >= 0){
		//printf("WriteUart %i\n", ret);
		CreateIntValue(s, ret);
	}
}

//	(int len)
void ComPortRead (ScriptValue &s, ScriptValue *args)
{
	ObjectValue *obj = s.objectVal;

	HANDLE *hPort = objGetPortHandle(obj);
	int len = args[0].i32;
	if (len < 1) len = 1;

	unsigned char *buffer = malloc((1+len) * sizeof(unsigned char));
	if (buffer){
		int bytesRead = ReadUart(hPort, buffer, len);
		if (bytesRead > 0){
			buffer[bytesRead] = 0;
			CreateStringValue(s, buffer, bytesRead);
			free(buffer);
			return;
		}
		free(buffer);
	}

	CreateNullValue(s);
}

void ComPortClose (ScriptValue &s, ScriptValue *args)
{
	ObjectValue *obj = s.objectVal;
	CreateNullValue(s);

	HANDLE *hPort = objGetPortHandle(obj);
	if (hPort){
		CloseUart(hPort);
		objSetPortHandle(obj, NULL);
	}
}

void CleanupComPort (ScriptValue &s, ScriptValue *args)
{
	ComPortClose(s, args);
}

