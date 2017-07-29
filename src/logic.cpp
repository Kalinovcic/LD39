
void frame()
{
    process_events();

    clear_screen();
    render_test();

    swap_buffers();
}
