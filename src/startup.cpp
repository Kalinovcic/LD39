
void initialize_backend()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
    {
        critical("SDL2 failed to initialize!");
    }
}

void init_opengl()
{
    glClearColor(135.0f/255.0f, 206.0f/255.0f, 250.0f/255.0f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    glClearDepthf(1.0f);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(true);

    load_mesh_shader();
}

void main_loop(void* unused)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    process_events();

    frame();

    swap_buffers();

#if WEB
    if (window_requests_close)
    {
        emscripten_cancel_main_loop();
    }
#endif
}

#if WEB
int main()
#elif CONSOLE
int wmain()
#else
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#endif
{
    initialize_backend();
    create_the_window();
    init_opengl();
    init_audio_system();

    load_data();
    create_level(0);

#if WEB
    emscripten_set_main_loop_arg(main_loop, NULL, -1, 1);
#else
    while (!window_requests_close)
        main_loop(NULL);
#endif

    uninit_audio_system();
    destroy_the_window();
    return 0;
}
