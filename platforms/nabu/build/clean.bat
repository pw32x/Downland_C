@echo off
setlocal
rem Deletes everything generated (the out\ folder).  Needs make, sh and rm on the PATH; the compiler is not needed.
pushd "%~dp0.."

where make >nul 2>&1
if errorlevel 1 (
    echo make was not found on the PATH. Install GNU make with sh, find, rm and mkdir and put its folder on the PATH. See readme.MD
    popd
    exit /b 1
)

make clean -f build/Makefile
set "RESULT=%ERRORLEVEL%"
popd
exit /b %RESULT%
