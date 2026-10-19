::echo Current directory : %cd%
::echo Launch mingw32-make.exe...

cd /D %cd%

C:\VulkanSDK\1.4.357.0\Bin\glslang.exe -V -g Shadow_pass_v_omni.vert -o Shadow_pass_v_omni.spv
C:\VulkanSDK\1.4.357.0\Bin\glslang.exe -V -g Shadow_pass_g_omni.geom -o Shadow_pass_g_omni.spv
C:\VulkanSDK\1.4.357.0\Bin\glslang.exe -V -g Shadow_pass_f_omni.frag -o Shadow_pass_f_omni.spv

pause