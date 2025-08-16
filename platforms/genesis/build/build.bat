pushd ..
SET "GDK_WIN=..\..\..\..\..\GenDev\SGDK"
SET "GDK=../../../../../GenDev/SGDK"
tools\bin\DLExporter.exe
%GDK_WIN%\bin\make -f build/makefile.gen
popd