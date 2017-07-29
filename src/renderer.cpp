
GLfloat data[] = {   0,  1, 0, 1, 0, 0,
                     1, -1, 0, 0, 1, 0,
                    -1, -1, 0, 0, 0, 1 };

GLuint buffer;
GLuint program;

void init_opengl()
{
    glClearColor(1.0f, 0.5f, 0.0f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    glClearDepthf(1.0f);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(true);

    load_mesh_shader();

    program = create_opengl_shader("data/shaders/test.vs", "data/shaders/test.fs");
    glBindAttribLocation(program, 0, "position");
    glBindAttribLocation(program, 1, "color");

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
}

void clear_screen()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void render_test()
{
    /*
    glUseProgram(program);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(0 * sizeof(float)));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    */

    render_mesh(&penguin);
}
