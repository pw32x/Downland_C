@echo off
setlocal
rem Builds the platform into out\ (out\Downland.nabu).  Extra arguments go to make, e.g. build.bat PROGNAME=my_game
rem
rem Environment variable (see readme.MD):
rem   NABU_COMPILER   required   the z88dk folder (holds bin\ and lib\)
rem GNU make, and the shell tools it uses (sh, find, rm, mkdir), must be on your PATH.
if not defined NABU_COMPILER (
    echo NABU_COMPILER is not set. Set it to your z88dk folder, the one holding bin\ and lib\. See readme.MD
    exit /b 1
)
if not exist "%NABU_COMPILER%\bin\zcc.exe" (
    echo NABU_COMPILER does not point at z88dk: "%NABU_COMPILER%\bin\zcc.exe" was not found.
    exit /b 1
)

pushd "%~dp0.."
set "ZCCCFG=%NABU_COMPILER%\lib\config"
set "PATH=%NABU_COMPILER%\bin;%PATH%"

where make >nul 2>&1
if errorlevel 1 (
    echo make was not found on the PATH. Install GNU make with sh, find, rm and mkdir and put its folder on the PATH. See readme.MD
    popd
    exit /b 1
)

make -f build/Makefile %*
set "RESULT=%ERRORLEVEL%"
popd
exit /b %RESULT%
