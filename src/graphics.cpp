
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

GLuint font_program;
GLuint font_u_ortho;
GLuint font_u_color;
GLuint font_u_font;

GLuint color_program;
GLuint color_u_ortho;
GLuint color_u_color;

void load_font_shader()
{
    color_program = create_opengl_shader("data/shaders/color.vs", "data/shaders/color.fs");
    glBindAttribLocation(color_program, 0, "vertex_position");

    color_u_ortho = glGetUniformLocation(color_program, "ortho");
    color_u_color = glGetUniformLocation(color_program, "color");

    font_program = create_opengl_shader("data/shaders/font.vs", "data/shaders/font.fs");
    glBindAttribLocation(font_program, 0, "vertex_position");
    glBindAttribLocation(font_program, 1, "vertex_texcoord");

    font_u_ortho = glGetUniformLocation(font_program, "ortho");
    font_u_color = glGetUniformLocation(font_program, "color");
    font_u_font  = glGetUniformLocation(font_program, "font");
}

void load_font(const char* path, Font* font)
{
    auto source = read_all_bytes_from_file(path, false).data;

    char* bitmap = (char*) malloc(512 * 512);
    stbtt_BakeFontBitmap((const u8*) source, 0, 32.0, (u8*) bitmap, 512, 512, 32, 96, font->chars);
    free(source);

    glGenTextures(1, &font->texture);
    glBindTexture(GL_TEXTURE_2D, font->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512, 512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    free(bitmap);
}

struct Font_Vertex
{
    v2 position;
    v2 texcoord;
};

static void get_string_vertices(std::vector<Font_Vertex>& vertices, v2* min, v2* max, Font* font, float x, float y, float scale, char* string)
{
    float cx = 0.0;
    float cy = 0.0;
    while (*string)
    {
        if (*string == '\n')
        {
            cx = 0;
            cy += 32.0 * 0.9;
        }
        if ((u8) *string >= 32 && (u8) *string < 128)
        {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(font->chars, 512, 512, *string - 32, &cx, &cy, &q, 1);

            Font_Vertex a = { { x + q.x0 * scale, y - q.y0 * scale }, { q.s0, q.t0 } };
            Font_Vertex b = { { x + q.x1 * scale, y - q.y0 * scale }, { q.s1, q.t0 } };
            Font_Vertex c = { { x + q.x1 * scale, y - q.y1 * scale }, { q.s1, q.t1 } };
            Font_Vertex d = { { x + q.x0 * scale, y - q.y1 * scale }, { q.s0, q.t1 } };

            vertices.push_back(a);
            vertices.push_back(b);
            vertices.push_back(c);
            vertices.push_back(c);
            vertices.push_back(d);
            vertices.push_back(a);
        }
        string++;
    }

    min->x = 10000;
    min->y = 10000;
    max->x = -10000;
    max->y = -10000;
    for (auto& v : vertices)
    {
        min->x = std::min(min->x, v.position.x);
        min->y = std::min(min->y, v.position.y);
        max->x = std::max(max->x, v.position.x);
        max->y = std::max(max->y, v.position.y);
    }
}

void render_string(Font* font, float x, float y, float scale, float align, char* string, v3 color)
{
    v2 min, max;
    std::vector<Font_Vertex> vertices;
    get_string_vertices(vertices, &min, &max, font, x, y, scale, string);

    float width = max.x - min.x;
    float adjust = align * width;
    for (auto& v : vertices)
        v.position.x -= adjust;
    min.x -= adjust;
    max.x -= adjust;

    min -= 10.0f;
    max += 10.0f;

    v2 box_vertices[] = {
        glm::vec2(min.x, min.y), glm::vec2(max.x, min.y), glm::vec2(max.x, max.y),
        glm::vec2(max.x, max.y), glm::vec2(min.x, max.y), glm::vec2(min.x, min.y),
        glm::vec2(min.x, min.y), glm::vec2(max.x, min.y), glm::vec2(max.x, max.y), glm::vec2(min.x, max.y)
    };

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font->texture);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    v4 box_color         = glm::vec4(0.0f, 0.0f, 0.0f, 0.5f);
    v4 box_outline_color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    glUseProgram(color_program);
    glUniformMatrix4fv(color_u_ortho, 1, GL_FALSE, (GLfloat*) &ortho);

    glBufferData(GL_ARRAY_BUFFER, 10 * sizeof(v2), box_vertices, GL_STATIC_DRAW);
    {
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(v2), NULL);
        glEnableVertexAttribArray(0);

        glUniform4fv(color_u_color, 1, (GLfloat*) &box_color);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glUniform4fv(color_u_color, 1, (GLfloat*) &box_outline_color);
        glDrawArrays(GL_LINE_LOOP, 6, 4);

        glDisableVertexAttribArray(0);
    }

    glUseProgram(font_program);
    glUniformMatrix4fv(font_u_ortho, 1, GL_FALSE, (GLfloat*) &ortho);
    glUniform3fv(font_u_color, 1, (GLfloat*) &color);
    glUniform1f(font_u_font, 0);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Font_Vertex), &vertices[0], GL_STATIC_DRAW);
    {
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Font_Vertex), (void*)(0 * sizeof(float)));
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Font_Vertex), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glDrawArrays(GL_TRIANGLES, 0, vertices.size());

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
    }

    glDeleteBuffers(1, &vbo);

    glDisable(GL_BLEND);
}
