@echo off

rem Z:\MinGW64_491
rem call gccpath64.bat
del *.o
del utills_64.dll

gcc -m64  -Ilibmylcd/include/ -Wall -std=gnu99 -c utillsdll.c scriptval.c -O2 -D_WIN32_WINNT=0x0601
rem -k --add-stdcall-alias

dllwrap -m64 -Llibmylcd/libs/x64/ utillsdll.o scriptval.o -o utills_64.dll -lgdi32 -lwinmm -lpsapi -lsetupapi -lhid --output-def utills.def -k --add-stdcall-alias
strip utills_64.dll


rem -k --add-stdcall-alias 

rem copy "utills_64.dll" "C:\Program Files\lcdmiscc\dll\utills_64.dll" /y

