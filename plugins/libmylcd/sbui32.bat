@echo off

rem call gccpath.bat
del *.o
del sbui_32.dll

gcc -m32 -Ilibmylcd/include/ -Wall -std=gnu99 -c sbui.c -O2
dllwrap -mdll -m32 -Llibmylcd/libs/x32/ sbui.o -o sbui_32.dll -lgdi32 -lmylcddll -lwinmm -lsetupapi -lhid --output-def sbui.def -k --machine=i386 --target=i686
strip sbui_32.dll


rem -k --add-stdcall-alias 

rem copy "sbui_32.dll" "C:\Program Files\lcdmiscsbui\dll\sbui_32.dll" /y

rem copy "sbui.dll" "C:\Program Files\lcdmiscb\dll\sbui.dll" /y
rem copy "sbui.dll" "C:\Program Files\lcdmisc\dll\sbui.dll" /y


K:\code\LCDMisc\plugins\libmylcd>
