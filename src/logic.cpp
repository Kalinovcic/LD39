
int ticks = 0;

void frame()
{
    ticks++;

    if (input_up)
    {
        play_sound(&sound_robot_hit, 1.0);
        input_up = false;
    }
    if (input_left)
    {
        play_sound(&sound_robot_turn, 0.5);
        input_left = false;
    }
    if (input_right)
    {
        play_sound(&sound_robot_move, 0.5);
        input_right = false;
    }

    begin_mesh(&block);

    srand(0);
    for (int z = -5; z < 5; z++)
        for (int x = -5; x < 5; x++)
        {
            float y = rand() % 16 / -16.0;
            render_mesh(&block, glm::vec3((float) x, y, (float) z), 0.0f);
        }
    end_mesh();

    camera = glm::vec3(sin(ticks * 0.01) * 2.0f, 3.0f, 4.0f);
    camera_vector = glm::vec3(0.0f, -1.0, -1.0f);

    light_direction = glm::normalize(-camera);//glm::vec3(0.1, -1.0f, 0.1f));

    perspective = glm::perspective(glm::radians(60.0f), (float) window_width / (float) window_height, 0.1f, 100.0f);
    view = glm::lookAt(camera, camera + camera_vector, glm::vec3(0.0f, 1.0f, 0.0f));
    perspective_view = perspective * view;

    begin_mesh(&robot);
    render_mesh(&robot, glm::vec3(-1.5f, 0.0f, 0.0f), (ticks +  0) / 30.0f);
    render_mesh(&robot, glm::vec3( 0.0f, 0.0f, 0.0f), (ticks + 40) / 30.0f);
    render_mesh(&robot, glm::vec3( 1.5f, 0.0f, 0.0f), (ticks + 80) / 30.0f);
    end_mesh();
}
