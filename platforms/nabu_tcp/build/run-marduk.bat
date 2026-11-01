@echo off
call "%NABU_ADAPTERS%\start-adapter.bat" "%~dp0.." "%~dp0..\out"
REM cd /d "%NABU_EMULATORS%\marduk"
REM marduk.exe -S 127.0.0.1 -P 5816

cd /d "%NABU_EMULATORS%\marduk-src\marduk\build"
marduk.exe -S 127.0.0.1 -T -P 5816
