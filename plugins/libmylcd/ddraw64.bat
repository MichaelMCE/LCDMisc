@echo off

rem call gccpath.bat
del *.o

gcc -m64 -Ilibmylcd/include/ -Wall -c ddraw.c -O2 -std=gnu99
dllwrap -m64 -Llibmylcd/lib/x64/ ddraw.o -o ddraw_64.dll -lmylcddll -k --add-stdcall-alias
strip ddraw_64.dll

rem copy "ddraw.dll" "C:\Program Files\lcdmiscb\dll\ddraw.dll" /y
rem copy "ddraw.dll" "C:\Program Files\lcdmisc\dll\ddraw.dll" /y