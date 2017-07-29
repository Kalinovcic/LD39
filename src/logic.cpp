
int ticks = 0;

void frame()
{
    ticks++;

    camera = glm::vec3(0.0f, sin(ticks * 0.01) * 0.5 + 0.5, 3.0f);
    camera_vector = glm::vec3(0.0f, 0.0f, -1.0f);

    light_direction = glm::normalize(glm::vec3(0.1, -1.0f, 0.1f));

    perspective = glm::perspective(glm::radians(60.0f), (float) window_width / (float) window_height, 0.1f, 100.0f);
    view = glm::lookAt(camera, camera + camera_vector, glm::vec3(0.0f, 1.0f, 0.0f));
    perspective_view = perspective * view;

    begin_mesh(&penguin);
    render_mesh(&penguin, glm::vec3(-1.5f, 0.0f, 0.0f), (ticks +  0) / 30.0f);
    render_mesh(&penguin, glm::vec3( 0.0f, 0.0f, 0.0f), (ticks + 40) / 30.0f);
    render_mesh(&penguin, glm::vec3( 1.5f, 0.0f, 0.0f), (ticks + 80) / 30.0f);
    end_mesh();
}
