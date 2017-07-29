
void initialize_backend()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
    {
        critical("SDL2 failed to initialize!");
    }
}

void main_loop(void* unused)
{
    frame();

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
