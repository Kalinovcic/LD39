#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#include <algorithm>  // ughh
#include <vector>  // ughh

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

#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"

#include "AL/al.h"
#include "AL/alc.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "settings.h"
#include "platform.h"
#include "audio_engine.h"
#include "graphics.h"
#include "logic.h"
#include "mesh.h"

#include "data.h"

#include "startup.cpp"
#include "platform_crt.cpp"
#include "platform_sdl2.cpp"
#include "audio_engine.cpp"
#include "graphics.cpp"
#include "logic.cpp"
#include "mesh.cpp"
