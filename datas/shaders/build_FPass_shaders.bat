::echo Current directory : %cd%
::echo Launch mingw32-make.exe...

cd /D %cd%

C:\VulkanSDK\1.4.357.0\Bin\glslang.exe -V -g F_pass_v.vert -o F_pass_v.spv
C:\VulkanSDK\1.4.357.0\Bin\glslang.exe -V -g F_pass_f.frag -o F_pass_f.spv

pause