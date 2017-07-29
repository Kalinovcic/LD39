#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#if __EMSCRIPTEN__
    #include <emscripten.h>
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_opengles2.h>
#else
    #include <windows.h>
    #include "GL/glew.h"
    #include "GL/gl.h"
    #include "GL/glu.h"
    #include "SDL2/SDL.h"
#endif

#include "AL/al.h"
#include "AL/alc.h"

#include "settings.h"
#include "platform.h"
#include "audio_engine.h"
#include "renderer.h"
#include "shader.h"
#include "logic.h"

#include "data.h"

#include "startup.cpp"
#include "platform_crt.cpp"
#include "platform_sdl2.cpp"
#include "audio_engine.cpp"
#include "renderer.cpp"
#include "shader.cpp"
#include "logic.cpp"
