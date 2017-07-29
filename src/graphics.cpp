
static GLuint create_opengl_shader(const char* shader_name, GLenum type)
{
    auto source = read_all_bytes_from_file(shader_name, true).data;

    GLuint shader = glCreateShader(type);
    if (!shader)
    {
        critical("OpenGL failed to create a shader object!");
    }

    glShaderSource(shader, 1, (const GLchar**) &source, NULL);
    glCompileShader(shader);

    free(source);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status)
    {
        GLint error_length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &error_length);

        char* error = (char*) malloc(sizeof(char) * (error_length + 1));
        glGetShaderInfoLog(shader, error_length, NULL, error);
        error[error_length] = 0;

        glDeleteShader(shader);
        critical("Error compiling shader \"%s\":\n%s", shader_name, error);
        free(error);
    }

    return shader;
}

GLuint create_opengl_shader(const char* vertex, const char* fragment)
{
    GLuint vertex_shader = create_opengl_shader(vertex, GL_VERTEX_SHADER);
    GLuint fragment_shader = create_opengl_shader(fragment, GL_FRAGMENT_SHADER);

    GLuint program = glCreateProgram();
    if (!program)
    {
        critical("OpenGL failed to create a program object!");
    }

    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);

    glLinkProgram(program);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (!status)
    {
        GLint error_length = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &error_length);

        char* error = (char*) malloc(sizeof(char) * (error_length + 1));
        glGetProgramInfoLog(program, error_length, NULL, error);
        error[error_length] = 0;

        glDeleteProgram(program);
        critical("Error linking program \"%s\" \"%s\":\n%s", vertex, fragment, error);
        free(error);
    }

    return program;
}

void load_image(const char* path, Image* image, int channels)
{
    auto source = read_all_bytes_from_file(path, false);

    int file_channels;
    image->data = stbi_load_from_memory((stbi_uc const *) source.data, source.count, &image->width, &image->height, &file_channels, channels);
    image->channels = channels;
}

void free_image(Image* image)
{
    stbi_image_free(image->data);
}
