@echo off
setlocal
pushd "%~dp0.."
set PATH=%CDI_SDK_PATH%\dos\bin;%PATH%
make -f build\Makefile
popd
endlocal
