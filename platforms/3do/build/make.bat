call build\activate-env.bat
make.exe %* -f build\makefile
set arg0=%0
if [%arg0:~2,1%]==[:] pause
