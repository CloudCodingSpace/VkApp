@echo off
SetLocal EnableDelayedExpansion

pushd ..

for /R %%f in (*.filters) do (
    del %%f
)

for /R %%f in (*.user) do (
    del %%f
)

for /R %%f in (*.vcxproj) do (
    del %%f
)

for /R %%f in (*.sln) do (
    del %%f
)

for /R %%f in (Makefile) do (
    del %%f
)

popd
