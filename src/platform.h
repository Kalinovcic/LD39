#pragma once

int  window_width;
int  window_height;
bool window_requests_close = false;

void create_the_window();
void destroy_the_window();
void process_events();
void swap_buffers();

String read_all_bytes_from_file(char* path, bool null_terminated);
String read_all_bytes_from_file(const char* path, bool null_terminated);
