@echo off
rem Builds gamedata\SFX.BIN and src\sfx_table.h from the Downland WAV files.
rem Uses psxavenc from https://github.com/WonderfulToolchain/psxavenc

set "SOUNDS=C:\Dropbox\DownlandDev\Downland_C\Downland_C\sounds"
set "PSXAVENC=C:\development\tools\psxavenc-windows\bin\psxavenc.exe"
set "GAIN=1.5"
set "RATE=18900"

python "%~dp0build_sfx.py" --sounds "%SOUNDS%" --psxavenc "%PSXAVENC%" --gain %GAIN% --rate %RATE%
if errorlevel 1 pause
