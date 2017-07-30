@echo off
rem cl /nologo /O2 /DEBUG /Fo"ld39.obj" /Fe"ld39.exe" /MT /EHsc /DCONSOLE=1 src/unity_build.cpp /link /nologo /subsystem:console kernel32.lib user32.lib gdi32.lib opengl32.lib advapi32.lib lib/SDL2.lib lib/glew32.lib lib/OpenAL32.lib
cl /nologo /O2 /DEBUG /Fo"ld39.obj" /Fe"ld39.exe" /MT /EHsc src/unity_build.cpp /link /nologo /subsystem:windows kernel32.lib user32.lib gdi32.lib opengl32.lib advapi32.lib lib/SDL2.lib lib/glew32.lib lib/OpenAL32.lib
