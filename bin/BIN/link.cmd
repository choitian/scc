@ECHO OFF
call "C:\Program Files\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86
rem ml  %1 /link -subsystem:console /link LIBCMT.LIB kernel32.lib oldnames.lib
rem ml  %1 /link -subsystem:console /link MSVCRT.LIB /link -entry:mainCRTStartup
ml  %1 /link -subsystem:console /link LIBCMT.LIB kernel32.lib oldnames.lib