@echo off
setlocal
pushd "%~dp0.."
make clean -f build\Makefile
set PATH=%CDI_SDK_PATH%\dos\bin;%PATH%
make -f build\Makefile
popd
endlocal
