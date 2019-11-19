@echo off

rem call gccpath.bat
del *.o
del utills_32.dll

gcc -m32 -Ilibmylcd/include/ -Wall -std=gnu99 -m32 -c utillsdll.c scriptval.c -O2 -D_WIN32_WINNT=0x0601
dllwrap -m32 -Llibmylcd/libs/x32/ utillsdll.o scriptval.o -o utills_32.dll -lgdi32 -lwinmm -lpsapi -lsetupapi -lhid --output-def utills.def -k --add-stdcall-alias
strip utills_32.dll



copy "utills_32.dll" "C:\Program Files\lcdmiscc\dll\utills_32.dll" /y

