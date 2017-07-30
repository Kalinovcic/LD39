
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

bool frequencies[256];

enum Entity_Kind
{
    ENTITY_ROBOT,
    ENTITY_GOAL,
    ENTTIY_DETECTOR,
    ENTITY_BOX,
};

struct Entity
{
    Entity_Kind kind;

    int tile_x;
    int tile_z;
    int angle;
    bool extended;

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

    Mesh* mesh;
    float scale;
};

struct Robot: Entity {};

struct Goal: Entity {};

struct Detector: Entity
{
    int frequency;
    bool pressed;
};

struct Box: Entity {};

struct Tile
{
    int   y;
    int   original_y;
    float display_y;
    v3    color;

    Box* box;

    int frequency;
    bool lower;
};

struct Level
{
    Tile* tiles;
    int count_x;
    int count_z;

    int max_moves;
    int remaining_moves;

    bool is_fading_out;
    float fade;
    float fade_velocity;

    bool first_frame;
    bool is_final;
    bool won;

    Entity* entities[128];
    Entity** next_entity;
};

int current_level_index = 0;
Level level;
Robot* robot;
Goal* goal;

template <typename T>
inline T* add_entity(Entity_Kind kind)
{
    auto e = (T*) malloc(sizeof(T));
    *(level.next_entity++) = e;
    *e = {};
    e->kind = kind;
    return e;
}

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

v3 get_entity_display_position(Entity* e)
{
    float x = e->display_x + 0.5f;
    float z = e->display_z + 0.5f;
    float y = get_tile(e->tile_x, e->tile_z)->display_y;
    return glm::vec3(x, y, z);
}

void place_camera()
{
    camera_vector = glm::vec3(1.0f, -1.6, -1.0f);
    target_camera = get_entity_display_position(robot);
    target_camera.y = get_desired_tile_display_y(get_tile(robot->tile_x, robot->tile_z)->y);
    target_camera -= camera_vector * 3.0f;
    camera += (target_camera - camera) * std::min((float) 1.0f, (float) delta_seconds * 4.0f);
}

void animate_entity(Entity* e)
{
    if (e->animating)
    {
        e->animation_time -= delta_seconds;
        e->animation_reverse_time -= delta_seconds;

        if (e->animation_reverses && e->animation_reverse_time <= 0.0)
        {
            e->velocity_x     = -e->velocity_x;
            e->velocity_z     = -e->velocity_z;
            e->velocity_angle = -e->velocity_angle;
            e->animation_reverses = false;
        }

        e->display_x     += delta_seconds * e->velocity_x;
        e->display_z     += delta_seconds * e->velocity_z;
        e->display_angle += delta_seconds * e->velocity_angle;

        if (e->animation_time <= 0.0)
        {
            e->animating = false;
        }
    }

    if (!e->animating)
    {
        e->animation_reverses = false;

        e->display_x     = e->tile_x;
        e->display_z     = e->tile_z;
        e->display_angle = e->angle;

        e->velocity_x     = 0;
        e->velocity_z     = 0;
        e->velocity_angle = 0;
    }
}

static int DX[] = { -1,  0,  1,  0 };
static int DZ[] = {  0,  1,  0, -1 };

int correct_angle(int angle)
{
    if (angle < 0)
        return 360 - (-angle % 360);
    else
        return angle % 360;
}

bool can_move_to_tile(int x, int z, bool extension, int vx, int vz, float vv)
{
    if (x < 0 || x >= level.count_x || z < 0 || z >= level.count_z)
        return extension;
    auto tile = get_tile(x, z);
    auto robot_tile = get_tile(robot->tile_x, robot->tile_z);
    int y0 = robot_tile->y;
    int y1 = tile->y;
    if (y0 > y1)
        return extension;
    if (y0 < y1)
        return false;
    if (tile->box)
    {
        int dx = x + vx;
        int dz = z + vz;
        if (dx < 0 || dx >= level.count_x || dz < 0 || dz >= level.count_z)
            return false;
        auto displacement_tile = get_tile(dx, dz);
        if (y0 != displacement_tile->y)
            return false;
        if (displacement_tile->box)
            return false;

        auto box = tile->box;
        box->tile_x = dx;
        box->tile_z = dz;
        box->animating = true;
        box->animation_time = vv;
        box->display_x = box->tile_x - vx;
        box->display_z = box->tile_z - vz;
        box->velocity_x = vx / vv;
        box->velocity_z = vz / vv;

        tile->box = NULL;
        displacement_tile->box = box;
        return true;
    }
    return true;
}

void move_robot(int direction)
{
    if (robot->animating) return;

    if (robot->angle < 0)
        robot->angle = 360 - (-robot->angle % 360);
    else
        robot->angle = robot->angle % 360;

    static int DX[] = { -1,  0,  1,  0 };
    static int DZ[] = {  0,  1,  0, -1 };

    int orientation = robot->angle / 90;
    int dx = DX[orientation] * direction;
    int dz = DZ[orientation] * direction;

    int new_x = robot->tile_x + dx;
    int new_z = robot->tile_z + dz;

    bool move = can_move_to_tile(new_x, new_z, false, dx, dz, 0.5f) &&
                can_move_to_tile(new_x - DX[orientation], new_z - DZ[orientation], true, dx, dz, 0.5f);

    float delta_move = 1.0f;
    if (move)
    {
        play_sound(&sound_robot_move[next_move_sound = (next_move_sound + 1) % 3]);
        robot->tile_x = new_x;
        robot->tile_z = new_z;
        robot->animation_time = 0.5;

        level.remaining_moves--;
    }
    else
    {
        play_sound(&sound_robot_hit);
        robot->animation_reverses = true;
        robot->animation_reverse_time = 0.1;
        robot->animation_time = 0.2;
        delta_move = 0.5;
    }

    robot->animating = true;
    robot->velocity_x = dx * delta_move / robot->animation_time;
    robot->velocity_z = dz * delta_move / robot->animation_time;
}

void rotate_robot(int plus_minus)
{
    if (robot->animating) return;

    static int DX[] = { -1,  0,  1,  0 };
    static int DZ[] = {  0,  1,  0, -1 };

    int orientation = robot->angle / 90;
    int dx = DX[orientation];
    int dz = DZ[orientation];

    int new_angle = correct_angle(robot->angle + 90 * plus_minus);

    int orientation2 = new_angle / 90;
    int dx2 = DX[orientation2];
    int dz2 = DZ[orientation2];

    float animation_angle = 90;
    robot->animation_time = 0.3;

    bool turn = can_move_to_tile(robot->tile_x - dx2 - dx, robot->tile_z - dz2 - dz, true, -dx2, -dz2, 0.3f);
    if (!turn)
    {
        animation_angle = 40;
        robot->animation_time *= 40.0 / 90.0;
    }
    else
    {
        turn = can_move_to_tile(robot->tile_x - dx2, robot->tile_z - dz2, true, dx, dz, 0.3f);
        if (!turn)
        {
            animation_angle = 90;
            robot->animation_time *= 90.0 / 90.0;
        }
    }

    if (turn)
    {
        play_sound(&sound_robot_turn);
        robot->angle = new_angle;
    }
    else
    {
        play_sound(&sound_robot_hit);
        robot->animation_reverses = true;
        robot->animation_reverse_time = robot->animation_time / 2;
    }

    robot->animating = true;
    robot->velocity_angle = plus_minus * animation_angle * 1.0 / robot->animation_time;
}

void create_level(int index)
{
    if (level.tiles) free(level.tiles);

    Entity** to_delete = &level.entities[0];
    while (to_delete < level.next_entity)
        free(*(to_delete++));
    level.next_entity = &level.entities[0];

    char path[128];
    Image level_layout, level_decor;

    sprintf(path, "data/levels/%dlayout.png", index);
    load_image(path, &level_layout);
    sprintf(path, "data/levels/%ddecor.png", index);
    load_image(path, &level_decor);
    sprintf(path, "data/levels/%dsettings.txt", index);
    auto settings = read_all_bytes_from_file(path, true).data;

    assert(level_layout.width  == level_decor.width);
    assert(level_layout.height == level_decor.height);

    level.is_fading_out = false;

    level.count_x = level_layout.width;
    level.count_z = level_layout.height;
    level.tiles = (Tile*) malloc(level.count_x * level.count_z * sizeof(Tile));

    robot = add_entity<Robot>(ENTITY_ROBOT);
    robot->mesh = &mesh_robot;
    robot->scale = 0.58f;

    for (int z = 0; z < level.count_z; z++)
        for (int x = 0; x < level.count_x; x++)
        {
            int i = z * level.count_x + x;
            auto tile = &level.tiles[i];

            int y         = level_layout.data[i * 3] / 64;
            int frequency = level_layout.data[i * 3 + 1];
            int kind      = level_layout.data[i * 3 + 2];

            tile->y          = y;
            tile->original_y = tile->y;
            tile->display_y  = get_desired_tile_display_y(tile->y) - (rand() % 100 / 100.0 * 2.0 - 1.0) * 40.0;
            tile->color      = glm::vec3(level_decor.data[i*3+0]/255.0f, level_decor.data[i*3+1]/255.0f, level_decor.data[i*3+2]/255.0f);
            tile->frequency = frequency;
            tile->lower     = kind != 200;

            if (kind == 64)
            {
                goal = add_entity<Goal>(ENTITY_GOAL);
                goal->tile_x = x;
                goal->tile_z = z;
                tile->color = glm::vec3(1.0f, 1.0f, 1.0f);
            }

            if (kind == 128)
            {
                auto detector = add_entity<Detector>(ENTTIY_DETECTOR);
                detector->mesh = &mesh_detector;
                detector->scale = 0.4f;
                detector->display_x = detector->tile_x = x;
                detector->display_z = detector->tile_z = z;
                detector->frequency = tile->frequency;
                tile->frequency = 0;
            }

            tile->box = NULL;
            if (kind == 255)
            {
                auto box = add_entity<Box>(ENTITY_BOX);
                box->mesh = &mesh_box;
                box->scale = 0.3f;
                box->display_x = box->tile_x = x;
                box->display_z = box->tile_z = z;
                tile->box = box;
            }

            if (kind == 192)
            {
                robot->tile_x = x;
                robot->tile_z = z;
            }
        }

    int is_final;
    sscanf(settings, "%d %d %d", &level.max_moves, &robot->angle, &is_final);
    level.remaining_moves = level.max_moves;
    level.is_final = (bool) is_final;

    free_image(&level_layout);
    free_image(&level_decor);
    free(settings);

    level.first_frame = true;
}

bool queue_left;
bool queue_right;
bool queue_up;
bool queue_down;

void update_entity(Entity* e)
{
    switch (e->kind)
    {

    case ENTITY_ROBOT:
    {
        if (robot->animating)
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

        input_left = input_right = input_up = input_down = false;

        if (level.remaining_moves == 0)
        {
            level.is_fading_out = true;
            level.fade = 0;
            level.fade_velocity = 1.0f;
            level.won = false;
        }
    } break;

    case ENTITY_GOAL:
    {
        if (robot->tile_x == e->tile_x && robot->tile_z == e->tile_z)
        {
            level.is_fading_out = true;
            level.fade = 0;
            level.fade_velocity = 1.0f;
            level.won = true;
        }
    } break;

    case ENTTIY_DETECTOR:
    {
        auto detector = (Detector*) e;
        auto tile = get_tile(e->tile_x, e->tile_z);

        bool pressed = tile->box || (robot->tile_x == e->tile_x && robot->tile_z == e->tile_z);
        if ( pressed && !detector->pressed) play_sound(&sound_detector_on);
        if (!pressed &&  detector->pressed) play_sound(&sound_detector_off);
        detector->pressed = pressed;

        if (pressed)
        {
            frequencies[detector->frequency] = true;
        }
    } break;

    default: break;

    }
}

void update_level()
{
    memset(frequencies, 0, sizeof(frequencies));

    auto to_update = &level.entities[0];
    while (to_update < level.next_entity)
    {
        auto e = *(to_update++);
        update_entity(e);
    }

    for (int i = 0; i < level.count_x * level.count_z; i++)
    {
        auto tile = &level.tiles[i];
        if (!tile->frequency) continue;

        tile->y = tile->original_y;
        if (frequencies[tile->frequency])
        {
            tile->y += (tile->lower) ? -1 : 1;
        }
    }
}

void render_level()
{
    begin_mesh(&mesh_block);
    for (int x = 0; x < level.count_x; x++)
        for (int z = 0; z < level.count_z; z++)
        {
            auto tile = get_tile(x, z);

            float desired = get_desired_tile_display_y(tile->y);
            tile->display_y += (desired - tile->display_y) * std::min(1.0f, (float)(delta_seconds * 6.0f));

            set_mesh_color_multiplier(tile->color);
            render_mesh(&mesh_block, glm::vec3((float) x, tile->display_y, (float) z), 0.0f);
        }
    end_mesh();

    set_mesh_color_multiplier(glm::vec3(1.0f, 1.0f, 1.0f));

    auto to_render = &level.entities[0];
    while (to_render < level.next_entity)
    {
        auto e = *(to_render++);
        if (!e->mesh) continue;

        animate_entity(e);

        begin_mesh(e->mesh);
        render_mesh(e->mesh, get_entity_display_position(e), e->display_angle, e->scale);
        end_mesh();
    }
}

void render_battery()
{
    auto battery_empty_color = glm::vec3(207.0f / 255.0f, 216.0f / 255.0f, 220.0f / 255.0f);
    auto battery_full_color  = glm::vec3(139.0f / 255.0f, 195.0f / 255.0f,  74.0f / 255.0f);

    float a0 = (float) level.remaining_moves / (float) level.max_moves;
    float a1 = 1.0 - a0;

    float x = 0.0;
    float s = 100.0f;

    float y0 = 10.0f;
    float y1 = y0 + a0 * s;

    render_texture(texture_battery, x, y0, x + s, y0 + s * a0, 0,  1, 1, a1, battery_full_color);
    render_texture(texture_battery, x, y1, x + s, y1 + s * a1, 0, a1, 1,  0, battery_empty_color);

    char battery_string[8];
    sprintf(battery_string, "%d", level.remaining_moves);
    render_string(&font, 48.0f, 50.0f, 1.0f, 0.5f, battery_string, glm::vec3(0.0f, 0.0f, 0.0f), false);

    if (current_level_index == 0)
    {
        render_string(&font, 100, 50, 24.0f / 48.0f, 0.0f, (char*) "Some actions drain power.\nDon't let it run out!", glm::vec3(1.0f, 1.0f, 1.0f));
    }
}

void render_ui()
{
    glDisable(GL_DEPTH_TEST);

    if (current_level_index == 0)
    {
        v2 robot_screen = world_to_screen(get_entity_display_position(robot) + glm::vec3(-0.5f, 0.0f, +0.5f));
        render_string(&font, robot_screen.x, robot_screen.y - 26.0f, 24.0f / 48.0f, 0.5f, (char*) "Move with arrows or WASD.", glm::vec3(1.0f, 1.0f, 1.0f));

        v2 white_screen = world_to_screen(get_tile_display_position(goal->tile_x, goal->tile_z) + glm::vec3(0.5f, 0.0f, 0.5f));
        render_string(&font, white_screen.x, white_screen.y - 26.0f, 24.0f / 48.0f, 0.5f, (char*) "Reach this white block!", glm::vec3(1.0f, 1.0f, 1.0f));
    }

    render_string(&font, window_width - 80.00f, 8.0f, 24.0f / 48.0f, 0.5f, (char*) "Press R to restart", glm::vec3(1.0f, 1.0f, 1.0f));

    render_battery();

    if (level.is_fading_out)
    {
        render_colored_quad(0, 0, window_width, window_height, glm::vec4(0.0f, 0.0f, 0.0f, level.fade));
    }

    glEnable(GL_DEPTH_TEST);
}

void render_scene()
{
    if (level.first_frame)
        animate_entity(robot);
    place_camera();
    if (level.first_frame)
        camera = target_camera;

    light_direction = glm::normalize(glm::vec3(0.2f, -1.0f, 0.2f));

    perspective = glm::perspective(glm::radians(60.0f), (float) window_width / (float) window_height, 0.1f, 100.0f);
    view = glm::lookAt(camera, camera + camera_vector, glm::vec3(0.0f, 1.0f, 0.0f));
    perspective_view = perspective * view;

    render_level();
    render_ui();
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

    ortho = glm::ortho(0.0f, (float) window_width, 0.0f, (float) window_height);

    if (input_previous_level)
    {
        current_level_index--;
        input_previous_level = false;
        input_reset = true;
    }

    if (input_next_level)
    {
        if (level.is_final)
        {
            // @Incomplete
        }
        else
        {
            current_level_index++;
        }
        input_next_level = false;
        input_reset = true;
    }

    if (input_reset)
    {
        input_reset = 0;
        create_level(current_level_index);
    }

    if (state == STATE_PLAYING)
    {
        if (!level.is_fading_out)
            update_level();
        render_scene();

        if (level.is_fading_out)
        {
            level.fade += delta_seconds * level.fade_velocity;
            if (level.fade >= 1.0f)
            {
                if (level.won)
                    input_next_level = true;
                else
                    input_reset = true;
            }
        }

        level.first_frame = false;
    }
}
