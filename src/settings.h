#pragma once

#if __EMSCRIPTEN__
#define WEB 1
#endif

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

typedef glm::vec3 v3;
typedef glm::vec4 v4;
typedef glm::mat4 m4;

struct String
{
    char* data;
    int count;
};

const int WINDOW_WIDTH  = 16 * 60;
const int WINDOW_HEIGHT = 9 * 60;
const char* WINDOW_TITLE = "LD39";

void critical(const char* format, ...)
{
    va_list args;
    va_start(args, format);
#if WEB
    vfprintf(stderr, format, args);
    EM_ASM(
        Module.horrible_error();
    );
#else
    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), format, args);
    MessageBox(NULL, buffer, NULL, MB_ICONERROR);
#endif
    va_end(args);
    exit(0);
}

char* copy_string(char* str)
{
    int len = strlen(str);
    char* new_str = (char*) malloc(len + 1);
    memcpy(new_str, str, len + 1);
    return new_str;
}
