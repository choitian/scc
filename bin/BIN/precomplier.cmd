@ECHO OFF
call "C:\Program Files\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86
cl -nologo -u -D_WIN32 -D_M_IX86 -D__STDC__ -X -I ENV\INCLUDE\ -P %1
rem -nologo
rem -u :remove remove all predefined macros
rem -D_WIN32 -D_M_IX86 -D__STDC__:define marcos
rem -I_INCLUDE : add to include search path
rem -x : ignore "standard places"

rem -P : only precomplie