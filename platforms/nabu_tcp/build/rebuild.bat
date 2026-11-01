@echo off
setlocal
rem Cleans, then builds everything from scratch.
call "%~dp0clean.bat" || exit /b 1
call "%~dp0build.bat" %*
exit /b %ERRORLEVEL%
