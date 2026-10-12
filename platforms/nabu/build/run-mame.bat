@echo off
cd /d "%NABU_EMULATORS%\nabu-mame"
mame.exe nabupc -window -hcca hcca_local -npz "%~dp0..\out\Downland.nabu" -skip_gameinfo
