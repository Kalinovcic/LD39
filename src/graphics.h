
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

void load_color_and_texture_shaders();

void render_colored_quad(float x0, float y0, float x1, float y1, v4 color);

GLuint load_texture(const char* path);
void render_texture(GLuint texture, float x0, float y0, float x1, float y1, float s0, float t0, float s1, float t1, v3 color);

void load_font(const char* path, Font* font);
void render_string(Font* font, float x, float y, float scale, float align, char* string, v3 color, bool render_box = true, float line_spacing = 0.95f);
