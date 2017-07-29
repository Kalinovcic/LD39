
struct Image
{
    int width;
    int height;
    int channels;
    u8* data;
};

GLuint create_opengl_shader(const char* vertex, const char* fragment);

void load_image(const char* path, Image* image, int channels = 3);
void free_image(Image* image);
