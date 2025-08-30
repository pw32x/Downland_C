pushd ..
powershell.exe /c C:\Windows\System32\wsl.exe make clean -f build/Makefile
powershell.exe /c C:\Windows\System32\wsl.exe make -f build/Makefile
popd