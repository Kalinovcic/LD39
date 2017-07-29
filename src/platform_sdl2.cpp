
struct Window
{
    SDL_Window*     handle;
    SDL_GLContext   gl;
};

Window the_window;

void create_the_window()
{
    /*SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);*/
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetSwapInterval(1);

    the_window.handle = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    if (!the_window.handle)
    {
        critical("SDL2 failed to create a window!");
    }

    SDL_ShowWindow(the_window.handle);

    the_window.gl = SDL_GL_CreateContext(the_window.handle);
    if (!the_window.gl)
    {
        critical("Failed to create a OpenGL context!");
    }

#if !WEB
    // glewExperimental = GL_TRUE;
    glewInit();
#endif
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
        case SDL_KEYDOWN:
        {
            auto scan = event.key.keysym.scancode;
            if (scan == SDL_SCANCODE_LEFT  || scan == SDL_SCANCODE_A) input_left  = true;
            if (scan == SDL_SCANCODE_RIGHT || scan == SDL_SCANCODE_D) input_right = true;
            if (scan == SDL_SCANCODE_UP    || scan == SDL_SCANCODE_W) input_up    = true;
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
