# VkApp
VkApp is just a graphics application in C++ that uses Vulkan as the graphics API.
It is written because I was feeling bored as I wasn't doing anything.

# Build instructions (Windows)
Simply go to the scripts directory and then run either the **GenMakefileProj.bat** (For generating makefiles) or **GenVS2022Proj.bat** (For generating Visual Studio 2022 Solution Files)
After generating the project files build it. 
**NOTE: Before running the app, make sure to compile the shaders by running the CompileShaders.bat from the project root folder**

While running the working directory of the app executable must be inside the **VkApp** folder that contains the **assets** folder

# Prerequisites
 - A machine that supports Vulkan 1.2 (Minimum)
 - A compiler supporting latest C/C++ standards (gcc/clang/MSVC/etc.)
 - Must have the Vulkan SDK and the Vulkan SDK runtime installed and the environment variable **VULKAN_SDK** must be defined

# External Libraries
 - GLFW
 - GLM
 - STB
 - TinyOBJ
 - ImGui
 - Vulkan SDK
