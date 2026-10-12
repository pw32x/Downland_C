@echo off
call "%NABU_ADAPTERS%\start-adapter.bat" "%~dp0.." "%~dp0..\out"
cd /d "%NABU_EMULATORS%\marduk"
marduk.exe -S 127.0.0.1 -P 5816
