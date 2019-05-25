REM @ECHO OFF
set a=%1
set file_name=%a:~0,-2%
set i_ext=.i
set asm_ext=.as
set exe_ext=.exe

cd bin
call precomplier.cmd ..\%1
call scc.exe %file_name%%i_ext%
call link.cmd %file_name%%asm_ext%
cd ..
move bin\%file_name%*.*  .\
pause
cls
call .\%file_name%%exe_ext%
pause
cmd.exe
