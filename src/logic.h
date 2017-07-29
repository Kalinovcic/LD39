
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

void create_level(int index);

void frame();
