@echo off

rem call gccpath.bat
del *.o

gcc -m64 -Ilibmylcd/include/ -Wall -c g19.c -O2 -std=c99
dllwrap -m64 -Llibmylcd/libs/x64/ g19.o -o g19_64.dll -lgdi32 -lmylcddll -lwinmm -lsetupapi -lhid
strip g19_64.dll

rem copy "g19.dll" "C:\Program Files\lcdmiscb\dll\g19.dll" /y
rem copy "g19.dll" "C:\Program Files\lcdmisc\dll\g19.dll" /y