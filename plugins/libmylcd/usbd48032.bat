@echo off

rem call gccpath.bat
del *.o

gcc -m32 -Ilibmylcd/include/ -Wall -c usbd480.c -O2
dllwrap -m32 --Llibmylcd/libs/x32/ usbd480.o -o usbd480_32.dll -lgdi32  -lmylcddll -lwinmm -lsetupapi -lhid -k --add-stdcall-alias
strip usbd480_32.dll

copy "usbd480.dll" "C:\Program Files\lcdmiscb\dll\usbd480.dll" /y
copy "usbd480.dll" "C:\Program Files\lcdmisc\dll\usbd480.dll" /y