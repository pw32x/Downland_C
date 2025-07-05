set FOLDER=build_vs

if not exist "%FOLDER%\" mkdir %FOLDER%

pushd %FOLDER%

"C:\Program Files\CMake\bin\cmake.exe" ..

popd