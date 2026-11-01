@echo off
call "%NABU_ADAPTERS%\start-adapter_port5817.bat" "%~dp0.." "%~dp0..\out"
cd /d "%NABU_EMULATORS%\marduk-src\marduk\build"
marduk.exe -S 127.0.0.1 -T -P 5817
