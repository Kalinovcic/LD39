
v3 camera;
v3 camera_vector;

v3 light_direction;
m4 perspective_view;
m4 perspective;
m4 view;

bool input_left;
bool input_right;
bool input_up;

void create_level(int count_x, int count_z);

void frame();
