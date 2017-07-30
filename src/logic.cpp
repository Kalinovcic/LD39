
u64 counter_frequency = SDL_GetPerformanceFrequency();
u64 previous_counter;
bool has_previous_counter = false;

double delta_seconds;
double seconds_since_start = 0.0;

v3 target_camera;

enum State
{
    STATE_PLAYING,
};

State state;
double state_time;

bool previous_detector_states[256];
bool detector_states[256];

struct Tile
{
    int   y;
    int   original_y;
    float display_y;
    v3    color;

    bool box_on_top;
    float box_delta_x;
    float box_delta_z;

    bool detector_on_top;
    int lower_on_state;
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

inline float get_desired_tile_display_y(int y)
{
    return (float) y * 0.75f;
}

inline v3 get_tile_display_position(int x, int z)
{
    return glm::vec3((float) x, get_tile(x, z)->display_y, (float) z);
}

inline v2 world_to_screen(v3 world)
{
    v4 homo = perspective_view * glm::vec4(world, 1.0);
    v2 window = glm::vec2((GLfloat) window_width, (GLfloat) window_height);
    return (glm::vec2((float) homo.x, (float) homo.y) / homo.w * 0.5f + 0.5f) * window;
}

v3 get_robot_display_position()
{
    float x = robot.display_x + 0.5f;
    float z = robot.display_z + 0.5f;
    float y = get_tile(x, z)->display_y;
    return glm::vec3(x, y, z);
}

void place_camera(bool use_desired = false)
{
    camera_vector = glm::vec3(1.0f, -1.3, -1.0f);
    target_camera = get_robot_display_position();
    if (use_desired)
        target_camera.y = get_desired_tile_display_y(get_tile(robot.tile_x, robot.tile_z)->y);
    target_camera -= camera_vector * 3.0f;
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

bool can_move_to_tile(int x, int z, bool extension, int vx, int vz)
{
    if (x < 0 || x >= level.count_x || z < 0 || z >= level.count_z)
        return extension;
    auto tile = get_tile(x, z);
    auto robot_tile = get_tile(robot.tile_x, robot.tile_z);
    int y0 = robot_tile->y;
    int y1 = tile->y;
    if (y0 > y1)
        return extension;
    if (y0 < y1)
        return false;
    if (tile->box_on_top)
    {
        int dx = x + vx;
        int dz = z + vz;
        if (dx < 0 || dx >= level.count_x || dz < 0 || dz >= level.count_z)
            return false;
        auto displacement_tile = get_tile(dx, dz);
        if (y0 != displacement_tile->y)
            return false;
        if (displacement_tile->box_on_top)
            return false;
        tile->box_on_top = false;
        displacement_tile->box_on_top = true;
        displacement_tile->box_delta_x = -vx;
        displacement_tile->box_delta_z = -vz;
        return true;
    }
    return true;
}

void move_robot(int direction)
{
    if (robot.animating) return;

    if (robot.angle < 0)
        robot.angle = 360 - (-robot.angle % 360);
    else
        robot.angle = robot.angle % 360;

    static int DX[] = { -1,  0,  1,  0 };
    static int DZ[] = {  0,  1,  0, -1 };

    int orientation = robot.angle / 90;
    int dx = DX[orientation] * direction;
    int dz = DZ[orientation] * direction;

    int new_x = robot.tile_x + dx;
    int new_z = robot.tile_z + dz;

    bool move = can_move_to_tile(new_x, new_z, false, dx, dz) &&
                can_move_to_tile(new_x - DX[orientation], new_z - DZ[orientation], true, dx, dz);;

    float delta_move = 1.0f;
    if (move)
    {
        play_sound(&sound_robot_move[next_move_sound = (next_move_sound + 1) % 3]);
        robot.tile_x = new_x;
        robot.tile_z = new_z;
        robot.animation_time = 0.5;
    }
    else
    {
        play_sound(&sound_robot_hit);
        robot.animation_reverses = true;
        robot.animation_reverse_time = 0.1;
        robot.animation_time = 0.2;
        delta_move = 0.5;
    }

    robot.animating = true;
    robot.velocity_x = dx * delta_move / robot.animation_time;
    robot.velocity_z = dz * delta_move / robot.animation_time;
}

float correct_angle(int angle)
{
    if (angle < 0)
        return 360 - (-angle % 360);
    else
        return angle % 360;
}

void rotate_robot(int plus_minus)
{
    if (robot.animating) return;

    static int DX[] = { -1,  0,  1,  0 };
    static int DZ[] = {  0,  1,  0, -1 };

    int orientation = robot.angle / 90;
    int dx = DX[orientation];
    int dz = DZ[orientation];

    int new_angle = correct_angle(robot.angle + 90 * plus_minus);

    int orientation2 = new_angle / 90;
    int dx2 = DX[orientation2];
    int dz2 = DZ[orientation2];

    float animation_angle = 90;
    robot.animation_time = 0.3;

    bool turn = can_move_to_tile(robot.tile_x - dx2 - dx, robot.tile_z - dz2 - dz, true, -dx2, -dz2);
    if (!turn)
    {
        animation_angle = 40;
        robot.animation_time *= 40.0 / 90.0;
    }
    else
    {
        turn = can_move_to_tile(robot.tile_x - dx2, robot.tile_z - dz2, true, dx, dz);
        if (!turn)
        {
            animation_angle = 110;
            robot.animation_time *= 110.0 / 90.0;
        }
    }

    if (turn)
    {
        play_sound(&sound_robot_turn);
        robot.angle = new_angle;
    }
    else
    {
        play_sound(&sound_robot_hit);
        robot.animation_reverses = true;
        robot.animation_reverse_time = robot.animation_time / 2;
    }

    robot.animating = true;
    robot.velocity_angle = plus_minus * animation_angle * 1.0 / robot.animation_time;
}

bool queue_left;
bool queue_right;
bool queue_up;
bool queue_down;

void update_robot()
{
    if (robot.animating)
    {
        if (input_left)  { queue_left = true;  queue_right = false; queue_up = false; queue_down = false; }
        if (input_right) { queue_left = false; queue_right = true;  queue_up = false; queue_down = false; }
        if (input_up)    { queue_left = false; queue_right = false; queue_up = true;  queue_down = false; }
        if (input_down)  { queue_left = false; queue_right = false; queue_up = false; queue_down = true;  }
    }
    else
    {
        if (queue_left)  { input_left  = true; queue_left  = false; }
        if (queue_right) { input_right = true; queue_right = false; }
        if (queue_up)    { input_up    = true; queue_up    = false; }
        if (queue_down)  { input_down  = true; queue_down  = false; }
    }

    if (input_left)  rotate_robot( 1);
    if (input_right) rotate_robot(-1);
    if (input_up)    move_robot( 1);
    if (input_down)  move_robot(-1);

    animate_robot();

    input_left = input_right = input_up = input_down = false;
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

void create_level(int index)
{
    if (level.tiles) free(level.tiles);

    char path[128];
    Image level_layout, level_decor;

    sprintf(path, "data/levels/%dlayout.png", index);
    load_image(path, &level_layout);
    sprintf(path, "data/levels/%ddecor.png", index);
    load_image(path, &level_decor);

    level.count_x = level_layout.width;
    level.count_z = level_layout.height;
    level.tiles = (Tile*) malloc(level.count_x * level.count_z * sizeof(Tile));

    for (int i = 0; i < level.count_x * level.count_z; i++)
    {
        auto tile = &level.tiles[i];
        tile->y               = level_layout.data[i * 3] / 64;
        tile->original_y      = tile->y;
        tile->display_y       = get_desired_tile_display_y(tile->y) - (rand() % 100 / 100.0 * 2.0 - 1.0) * 40.0;
        tile->color           = glm::vec3(level_decor.data[i*3+0]/255.0f, level_decor.data[i*3+1]/255.0f, level_decor.data[i*3+2]/255.0f);
        tile->box_on_top      = level_layout.data[i * 3 + 2] == 255;
        tile->box_delta_x     = 0.0f;
        tile->box_delta_z     = 0.0f;
        tile->detector_on_top = level_layout.data[i * 3 + 2] == 128;
        tile->lower_on_state  = level_layout.data[i * 3 + 1];
    }

    free_image(&level_layout);
    free_image(&level_decor);

    robot.tile_x = level.count_x / 2;
    robot.tile_z = level.count_z / 2;
    robot.angle = 180;
    animate_robot();

    place_camera(true);
    camera = target_camera;
}

void approach_zero(float* var, float velocity)
{
    if (*var < 0.0f)
    {
        *var += delta_seconds * velocity;
        if (*var >= 0.0f)
            *var = 0.0;
    }
    if (*var > 0.0f)
    {
        *var -= delta_seconds * velocity;
        if (*var <= 0.0f)
            *var = 0.0;
    }
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

    set_mesh_color_multiplier(glm::vec3(1.0f, 1.0f, 1.0f));

    begin_mesh(&mesh_box);
    for (int x = 0; x < level.count_x; x++)
        for (int z = 0; z < level.count_z; z++)
        {
            auto tile = get_tile(x, z);
            if (!tile->box_on_top) continue;

            approach_zero(&tile->box_delta_x, 3.0f);
            approach_zero(&tile->box_delta_z, 3.0f);

            float xx = x + 0.5f + tile->box_delta_x;
            float zz = z + 0.5f + tile->box_delta_z;
            float yy = tile->display_y;
            render_mesh(&mesh_box, glm::vec3(xx, yy, zz), 0.0f, 0.3f);
        }
    end_mesh();

    begin_mesh(&mesh_detector);
    for (int x = 0; x < level.count_x; x++)
        for (int z = 0; z < level.count_z; z++)
        {
            auto tile = get_tile(x, z);
            if (!tile->detector_on_top) continue;

            float xx = x + 0.5f;
            float zz = z + 0.5f;
            float yy = tile->display_y;
            render_mesh(&mesh_detector, glm::vec3(xx, yy, zz), 0.0f, 0.4f);
        }
    end_mesh();
}

void render_scene()
{
    light_direction = glm::normalize(glm::vec3(0.2f, -1.0f, 0.2f));

    perspective = glm::perspective(glm::radians(60.0f), (float) window_width / (float) window_height, 0.1f, 100.0f);
    view = glm::lookAt(camera, camera + camera_vector, glm::vec3(0.0f, 1.0f, 0.0f));
    perspective_view = perspective * view;
    ortho = glm::ortho(0.0f, (float) window_width, 0.0f, (float) window_height);

    render_level();
    render_robot();

    glDisable(GL_DEPTH_TEST);

    v2 robot_screen = world_to_screen(get_robot_display_position() + glm::vec3(-0.5f, 0.0f, +0.5f));
    render_string(&font, robot_screen.x, robot_screen.y - 26.0f, 24.0f / 32.0f, 0.5f, (char*) "Move with arrows or WASD.", glm::vec3(1.0f, 1.0f, 1.0f));

    v2 white_screen = world_to_screen(get_tile_display_position(12, 3) + glm::vec3(0.5f, 0.0f, 0.5f));
    render_string(&font, white_screen.x, white_screen.y - 26.0f, 24.0f / 32.0f, 0.5f, (char*) "Reach this white block!", glm::vec3(1.0f, 1.0f, 1.0f));

    render_string(&font, 100, 100, 24.0f / 32.0f, 0.0f, (char*) "Some actions drain power.\nDon't let it run out!", glm::vec3(1.0f, 1.0f, 1.0f));

    glEnable(GL_DEPTH_TEST);
}

void update_tiles()
{
    bool previous_detector_states[256];
    memcpy(previous_detector_states, detector_states, sizeof(detector_states));
    memset(detector_states, 0, sizeof(detector_states));

    for (int z = 0; z < level.count_z; z++)
        for (int x = 0; x < level.count_x; x++)
        {
            auto tile = get_tile(x, z);
            if (!tile->detector_on_top) continue;
            {
                if (tile->box_on_top || (robot.tile_x == x && robot.tile_z == z))
                {
                    detector_states[tile->lower_on_state] = true;
                }
            }
        }

    for (int i = 0; i < level.count_x * level.count_z; i++)
    {
        auto tile = &level.tiles[i];
        if (!tile->detector_on_top)
        {
            tile->y = tile->original_y;
            if (detector_states[tile->lower_on_state])
                tile->y--;
        }
        float desired = get_desired_tile_display_y(tile->y);
        tile->display_y += (desired - tile->display_y) * std::min(1.0f, (float)(delta_seconds * 6.0f));
    }

    bool play_on = false;
    bool play_off = false;
    for (int i = 0; i < sizeof(detector_states) / sizeof(detector_states[0]); i++)
    {
        play_on = detector_states[i] && !previous_detector_states[i];
        play_off = !detector_states[i] && previous_detector_states[i];
    }
    if (play_on)  play_sound(&sound_detector_on);
    if (play_off) play_sound(&sound_detector_off);
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

    if (state == STATE_PLAYING)
    {
        update_robot();
        update_tiles();

        place_camera(true);
        render_scene();
    }
}
