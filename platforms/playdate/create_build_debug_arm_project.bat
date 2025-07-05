set FOLDER=build_arm\debug

if not exist "%FOLDER%\" mkdir %FOLDER%

pushd %FOLDER%

set PATH="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.38.33130\bin\Hostx86\x86\";%PATH%

"C:\Program Files\CMake\bin\cmake.exe" ..\.. -G "NMake Makefiles" --toolchain=%PLAYDATE_SDK_PATH%/C_API/buildsupport/arm.cmake

nmake

popd