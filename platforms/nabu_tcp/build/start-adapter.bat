@echo off
setlocal
rem Makes sure the NABU Internet Adapter is listening on TCP 5816 and serving the given project.
rem Usage: start-adapter.bat <project folder> [<folder holding the .nabu files>]

set "S=%SystemRoot%\System32"
set "NIA=%~dp0nabu-internet-adapter"
set "MARK=%NIA%\.project"
if "%~2"=="" (set "SERVE=%~1\build") else (set "SERVE=%~2")
for %%I in ("%SERVE%") do set "PROJ=%%~fI"

if not exist "%PROJ%\" (
    echo start-adapter: folder with the .nabu files not found: %PROJ%
    echo Build the project first.
    exit /b 1
)

rem Check if already listening on port 5816
call :listening && goto ready

goto start

:start
echo Starting the NABU Internet Adapter for %PROJ% ...
>"%MARK%" echo %PROJ%
start "NABU Internet Adapter" /d "%PROJ%" "%NIA%\NABU-Internet-Adapter-84.exe" -c "%NIA%\config.xml"
echo Waiting for port 5816...

:wait
set /a tries=0
:loop
call :listening && goto ready
set /a tries+=1
if %tries% geq 120 (
    echo Timed out after 2 minutes waiting for the adapter on port 5816.
    exit /b 1
)
"%S%\ping.exe" -n 2 127.0.0.1 >nul
goto loop

:ready
echo Adapter is listening.
exit /b 0

:listening
"%S%\netstat.exe" -ano | "%S%\findstr.exe" /c:":5816 " | "%S%\findstr.exe" /c:"LISTENING" >nul
exit /b %errorlevel%
