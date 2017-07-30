
v3 camera;
v3 camera_vector;

v3 light_direction;
m4 perspective_view;
m4 perspective;
m4 view;
m4 ortho;

bool input_left;
bool input_right;
bool input_up;
bool input_down;

bool input_escape;
bool input_tab;
bool input_space;
bool input_reset;
bool input_previous_level;
bool input_next_level;

int  input_mouse_x;
int  input_mouse_y;
bool input_mouse_click;

void create_level(int index);

void frame();
