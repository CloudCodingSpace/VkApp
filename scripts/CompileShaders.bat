@echo off
SetLocal EnableDelayedExpansion

pushd ..

for /R %%f in (*.vert) do (
	glslc %%f -o %%f.spv
)

for /R %%f in (*.frag) do (
	glslc %%f -o %%f.spv
)

popd

pause