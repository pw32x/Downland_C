pushd ..

@SET COMPILER_DIR=%SRL_SDK_ROOT%/../Compiler
@SET PATH=%COMPILER_DIR%\Other Utilities;%COMPILER_DIR%\msys2\usr\bin;%COMPILER_DIR%\sh2eb-elf\bin;%PATH%

"%SRL_SDK_ROOT_WIN32%\..\Compiler\msys2\usr\bin\make.exe" -f build\makefile
popd