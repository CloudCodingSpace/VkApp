@echo off

pushd ..
libs\premake\premake5.exe --cc=clang gmake2
popd
pause