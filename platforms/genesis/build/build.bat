pushd ..
SET "GDK_WIN=..\..\..\..\..\GenDev\SGDK"
SET "GDK=../../../../../GenDev/SGDK"
%GDK_WIN%\bin\make -f build/makefile.gen
popd