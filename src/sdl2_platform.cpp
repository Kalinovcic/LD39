
struct Window
{
    SDL_Window*     handle;
    SDL_GLContext   gl;
};

Window the_window;

void create_the_window()
{
    /* @Reconsider
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);*/
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetSwapInterval(1);

    auto window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    the_window.handle = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, window_flags);
    if (!the_window.handle)
    {
        printf("SDL2 failed to create a window!");
    }

    SDL_ShowWindow(the_window.handle);

    the_window.gl = SDL_GL_CreateContext(the_window.handle);
    if (!the_window.gl)
    {
        printf("Failed to create a OpenGL context!");
    }
}

void destroy_the_window()
{
    SDL_GL_DeleteContext(the_window.gl);
    SDL_DestroyWindow(the_window.handle);
}

void process_events()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
        {
            window_requests_close = true;
        } break;
        }
    }

    SDL_GL_GetDrawableSize(the_window.handle, &window_width, &window_height);
    glViewport(0, 0, window_width, window_height);
}

void swap_buffers()
{
    SDL_GL_SwapWindow(the_window.handle);
}
