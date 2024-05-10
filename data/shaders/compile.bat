@echo off

set CODE_PATH=%~dp0/../../code
set GLSLC_EXE=%CODE_PATH%/Extern/VulkanSDK/1.3.275.0/bin/win64/glslc.exe

echo ----------------------
echo --- Vertex Shaders ---
echo ----------------------
for %%f in (*.vert) do (
	echo %%~nf
	%GLSLC_EXE% %%~nf.vert -o %%~nf_vert.spv
)

echo ------------------------
echo --- Fragment Shaders ---
echo ------------------------
for %%f in (*.frag) do (
	echo %%~nf
	%GLSLC_EXE% %%~nf.frag -o %%~nf_frag.spv
)

pause