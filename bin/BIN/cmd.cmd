@ECHO OFF
call "C:\Program Files\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86
copy "C:\Users\choi\documents\visual studio 2010\Projects\SCC\Release\SCC.exe" scc.exe
cmd.exe
