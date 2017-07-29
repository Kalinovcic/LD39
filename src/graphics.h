
struct Image
{
    int width;
    int height;
    int channels;
    u8* data;
};

struct Font
{
    GLuint texture;
    stbtt_bakedchar chars[96];
};

GLuint create_opengl_shader(const char* vertex, const char* fragment);

void load_image(const char* path, Image* image, int channels = 3);
void free_image(Image* image);

void load_font_shader();
void load_font(const char* path, Font* font);
void render_string(Font* font, float x, float y, float scale, float align, char* string, v3 color);
