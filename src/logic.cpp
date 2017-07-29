
u64 counter_frequency = SDL_GetPerformanceFrequency();
u64 previous_counter;
bool has_previous_counter = false;

double delta_seconds;
double seconds_since_start = 0.0;

v3 target_camera;

struct Tile
{
    int   y;
    float display_y;
    v3    color;
};

struct Level
{
    Tile* tiles;
    int count_x;
    int count_z;
};

struct Robot
{
    int tile_x;
    int tile_z;
    int angle;

    float display_x;
    float display_z;
    float display_angle;

    bool animating;
    bool animation_reverses;
    float animation_time;
    float animation_reverse_time;
    float velocity_x;
    float velocity_z;
    float velocity_angle;
};

Level level;
Robot robot;

inline Tile* get_tile(int x, int z)
{
    return &level.tiles[z * level.count_x + x];
}

v3 get_robot_display_position()
{
    float x = robot.display_x + 0.5f;
    float z = robot.display_z - 0.5f;
    float y = get_tile(robot.tile_x, robot.tile_z)->display_y;
    return glm::vec3(x, y, z);
}

void place_camera()
{
    camera_vector = glm::vec3(1.0f, -2.0, -1.0f);
    target_camera = get_robot_display_position() - camera_vector * 3.0f;
    camera += (target_camera - camera) * std::min((float) 1.0f, (float) delta_seconds * 4.0f);
}

void animate_robot()
{
    if (robot.animating)
    {
        robot.animation_time -= delta_seconds;
        robot.animation_reverse_time -= delta_seconds;

        if (robot.animation_reverses && robot.animation_reverse_time <= 0.0)
        {
            robot.velocity_x     = -robot.velocity_x;
            robot.velocity_z     = -robot.velocity_z;
            robot.velocity_angle = -robot.velocity_angle;
            robot.animation_reverses = false;
        }

        robot.display_x     += delta_seconds * robot.velocity_x;
        robot.display_z     += delta_seconds * robot.velocity_z;
        robot.display_angle += delta_seconds * robot.velocity_angle;

        if (robot.animation_time <= 0.0)
        {
            robot.animating = false;
        }
    }

    if (!robot.animating)
    {
        robot.animation_reverses = false;

        robot.display_x     = robot.tile_x;
        robot.display_z     = robot.tile_z;
        robot.display_angle = robot.angle;

        robot.velocity_x     = 0;
        robot.velocity_z     = 0;
        robot.velocity_angle = 0;
    }
}

void move_robot()
{
    if (robot.animating) return;

    if (robot.angle < 0)
        robot.angle = 360 - (-robot.angle % 360);
    else
        robot.angle = robot.angle % 360;

    static int DX[] = { -1,  0,  1,  0 };
    static int DZ[] = {  0,  1,  0, -1 };

    int orientation = robot.angle / 90;
    int dx = DX[orientation];
    int dz = DZ[orientation];

    int new_x = robot.tile_x + dx;
    int new_z = robot.tile_z + dz;

    if (new_x < 0 || new_x >= level.count_x || new_z < 0 || new_z >= level.count_z ||
        get_tile(robot.tile_x, robot.tile_z)->y != get_tile(new_x, new_z)->y)
    {
        play_sound(&sound_robot_hit, 1.0);

        robot.animating = true;
        robot.animation_reverses = true;
        robot.animation_time = 0.2;
        robot.animation_reverse_time = 0.1;
        robot.velocity_x = dx * 0.5 / robot.animation_time;
        robot.velocity_z = dz * 0.5 / robot.animation_time;

        return;
    }

    robot.tile_x = new_x;
    robot.tile_z = new_z;

    robot.animating = true;
    robot.animation_time = 0.5;
    robot.velocity_x = dx * 1.0 / robot.animation_time;
    robot.velocity_z = dz * 1.0 / robot.animation_time;

    play_sound(&sound_robot_move[next_move_sound = (next_move_sound + 1) % 3], 1.0);
}

void rotate_robot(int plus_minus)
{
    if (robot.animating) return;
    robot.animating = true;

    robot.angle += 90 * plus_minus;

    if (robot.angle < 0)
        robot.angle = 360 - (-robot.angle % 360);
    else
        robot.angle = robot.angle % 360;

    robot.animation_time = 0.5;
    robot.velocity_angle = plus_minus * 90 * 1.0 / robot.animation_time;

    play_sound(&sound_robot_turn, 0.5);
}

bool queue_left;
bool queue_right;
bool queue_up;

void update_robot()
{
    if (robot.animating)
    {
        if (input_left)  { queue_left = true;  queue_right = false; queue_up = false; }
        if (input_right) { queue_left = false; queue_right = true;  queue_up = false; }
        if (input_up)    { queue_left = false; queue_right = false; queue_up = true;  }
    }
    else
    {
        if (queue_left)  { input_left  = true; queue_left  = false; }
        if (queue_right) { input_right = true; queue_right = false; }
        if (queue_up)    { input_up    = true; queue_up    = false; }
    }

    if (input_left)  rotate_robot( 1);
    if (input_right) rotate_robot(-1);
    if (input_up)    move_robot();

    animate_robot();

    input_left = input_right = input_up = false;
}

void render_robot()
{
    begin_mesh(&mesh_robot);

    m4 robot_model_matrix;
    robot_model_matrix  = glm::translate(get_robot_display_position());
    robot_model_matrix *= glm::scale(glm::vec3(0.58f));
    robot_model_matrix *= glm::rotate(glm::radians(robot.display_angle), glm::vec3(0.0f, 1.0f, 0.0f));
    set_mesh_color_multiplier(glm::vec3(1.0f, 1.0f, 1.0f));
    render_mesh(&mesh_robot, robot_model_matrix);

    end_mesh();
}

void create_level(int count_x, int count_z)
{
    if (level.tiles) free(level.tiles);

    level.count_x = count_x;
    level.count_z = count_z;
    level.tiles = (Tile*) malloc(level.count_x * level.count_z * sizeof(Tile));

    for (int x = 0; x < count_x; x++)
        for (int z = 0; z < count_z; z++)
        {
            auto tile = get_tile(x, z);
            tile->y         = (z == count_z / 2) ? 0 : (x % 5 == 0 ? 1 : 0);
            tile->display_y = (float) tile->y * 0.75f;
            tile->color     = glm::vec3(0.5f, z / (float) count_z, x / (float) count_x);
        }

    robot.tile_x = count_x / 2;
    robot.tile_z = count_z / 2;
    robot.angle = 180;
    animate_robot();

    place_camera();
    camera = target_camera;
}

void render_level()
{
    begin_mesh(&mesh_block);
    for (int x = 0; x < level.count_x; x++)
        for (int z = 0; z < level.count_z; z++)
        {
            auto tile = get_tile(x, z);
            float y = tile->display_y;
            set_mesh_color_multiplier(tile->color);
            render_mesh(&mesh_block, glm::vec3((float) x, y, (float) z), 0.0f);
        }
    end_mesh();
}

void render_scene()
{
    place_camera();
    light_direction = glm::normalize(glm::vec3(0.2f, -1.0f, 0.2f));

    perspective = glm::perspective(glm::radians(60.0f), (float) window_width / (float) window_height, 0.1f, 100.0f);
    view = glm::lookAt(camera, camera + camera_vector, glm::vec3(0.0f, 1.0f, 0.0f));
    perspective_view = perspective * view;

    render_level();
    render_robot();
}

void frame()
{
    u64 this_counter = SDL_GetPerformanceCounter();
    u64 delta = 0;
    if (has_previous_counter)
        delta = this_counter - previous_counter;
    previous_counter = this_counter;
    has_previous_counter = true;

    delta_seconds = (double) delta / (double) counter_frequency;
    seconds_since_start += delta_seconds;

    update_robot();
    render_scene();
}
