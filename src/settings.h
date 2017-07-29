#pragma once

#if __EMSCRIPTEN__
#define WEB 1
#else
#define WEB 0
#endif

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;

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
#else
    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), format, args);
    MessageBox(NULL, buffer, NULL, MB_ICONERROR);
#endif
    va_end(args);
    exit(0);
}
