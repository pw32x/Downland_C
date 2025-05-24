@echo off

set OUTPUT_DIR=%cd%\builds\alpha

REM Save the current directory and change to the solution folder

REM Set up the Visual Studio 2022 environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"

REM Build the solution using MSBuild
msbuild ..\platfforms\win32\Downland_C.sln /p:Configuration=Release /p:Platform=x86 /p:OutDir=%OUTPUT_DIR%\ /p:DownlandDefines=DEV_MODE

echo Build completed.

pushd builds\alpha
del ..\Downland_C.alpha.zip /q
"C:\Program Files\7-Zip\7z.exe" a -tzip ..\Downland_C.alpha.zip "*" -xr!*.pdb -xr!*.bin -xr!*.ccc ..\..\..\LICENSE ..\..\..\README.md
popd