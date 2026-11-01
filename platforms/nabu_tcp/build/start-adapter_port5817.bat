@echo off
setlocal
rem Makes sure the NABU Internet Adapter is listening on TCP 5817 and serving the given project.
rem Usage: start-adapter_port5817.bat <project folder> [<folder holding the .nabu files>]

set "S=%SystemRoot%\System32"
set "NIA=%~dp0nabu-internet-adapter"
set "MARK=%NIA%\.project_5817"
if "%~2"=="" (set "SERVE=%~1\build") else (set "SERVE=%~2")
for %%I in ("%SERVE%") do set "PROJ=%%~fI"

if not exist "%PROJ%\" (
    echo start-adapter: folder with the .nabu files not found: %PROJ%
    echo Build the project first.
    exit /b 1
)

call :listening && goto ready

:start
echo Starting the NABU Internet Adapter on port 5817 for %PROJ% ...
>"%MARK%" echo %PROJ%
start "NABU Internet Adapter (5817)" /d "%PROJ%" "%NIA%\NABU-Internet-Adapter-84.exe" -c "%NIA%\config_port5817.xml"
echo Waiting for port 5817...

:wait
set /a tries=0
:loop
call :listening && goto ready
set /a tries+=1
if %tries% geq 120 (
    echo Timed out after 2 minutes waiting for the adapter on port 5817.
    exit /b 1
)
"%S%\ping.exe" -n 2 127.0.0.1 >nul
goto loop

:ready
echo Adapter on port 5817 is listening.
exit /b 0

:listening
"%S%\netstat.exe" -ano | "%S%\findstr.exe" /c:":5817 " | "%S%\findstr.exe" /c:"LISTENING" >nul
exit /b %errorlevel%