@echo off

rem call gccpath.bat
del *.o

gcc -m64 -Ilibmylcd/include/ -Wall -c usbd480.c -O2 -std=c99
dllwrap -m64 -Llibmylcd/libs/x64/ usbd480.o -o usbd480_64.dll -lgdi32 -lmylcddll -lwinmm -lsetupapi -lhid
strip usbd480_64.dll

rem copy "usbd480.dll" "C:\Program Files\lcdmiscb\dll\usbd480.dll" /y
rem copy "usbd480.dll" "C:\Program Files\lcdmisc\dll\usbd480.dll" /y