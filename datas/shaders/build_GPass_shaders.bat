::echo Current directory : %cd%
::echo Launch mingw32-make.exe...

cd /D %cd%

C:\VulkanSDK\1.4.357.0\Bin\glslang.exe -V -g G_pass_v.vert -o G_pass_v.spv
C:\VulkanSDK\1.4.357.0\Bin\glslang.exe -V -g G_pass_f.frag -o G_pass_f.spv

pause