
void consume_spaces(char** remaining)
{
    while (isspace(**remaining))
        (*remaining)++;
}

char* consume_line(char** remaining)
{
    while (true)
    {
        consume_spaces(remaining);
        if (!**remaining)
            return NULL;
        char* line = *remaining;
        char* line_end = NULL;
        while (true)
        {
            char* here = *remaining;
            char c = *here;
            if (c != 0) (*remaining)++;
            if (c == '#' || c == 0 || c == '\n' || c == '\r')
            {
                if (!line_end) line_end = here;
                if (c != '#') break;
            }
        }
        assert(line_end);
        if (line == line_end)
            continue;
        *line_end = 0;
        return line;
    }
}

char* consume_token(char** remaining)
{
    consume_spaces(remaining);
    char* token = *remaining;
    while (**remaining && !isspace(**remaining))
        (*remaining)++;
    if (**remaining)
        *((*remaining)++) = 0;
    return token;
}

u32 load_materials_from_mtl(char* path, Material* destination)
{
    auto bytes = read_all_bytes_from_file(path, true);
    char* file = bytes.data;

    Material* material;
    u32 material_count = 0;
    while (true)
    {
        char* line = consume_line(&file);
        if (!line) break;

        char* what = consume_token(&line);
        if (!strcmp(what, "newmtl"))
        {
            char* name = consume_token(&line);
            if (material_count >= MAX_MATERIALS)
                critical("Too many materials in \"%s\".", path);

            material = destination++;
            material->name = copy_string(name);
            material_count++;
        }
        else if (!strcmp(what, "Kd"))
        {
            sscanf(line, "%f%f%f", &material->diffuse.x, &material->diffuse.y, &material->diffuse.z);
        }
        else if (!strcmp(what, "Ks"))
        {
            sscanf(line, "%f%f%f", &material->specular.x, &material->specular.y, &material->specular.z);
        }
        else if (!strcmp(what, "Ns"))
        {
            sscanf(line, "%f", &material->shininess);
        }
    }

    free(bytes.data);
    return material_count;
}

void load_mesh_from_obj(char* path, Mesh* mesh)
{
    auto path_length = strlen(path);
    auto mtl_path = (char*) malloc(path_length + 1);
    memcpy(mtl_path, path, path_length + 1);
    mtl_path[path_length - 3] = 'm';
    mtl_path[path_length - 2] = 't';
    mtl_path[path_length - 1] = 'l';
    mesh->num_materials = load_materials_from_mtl(mtl_path, mesh->materials);
    free(mtl_path);

    auto bytes = read_all_bytes_from_file(path, true);
    char* file = bytes.data;

    std::vector<v3> vertices;
    std::vector<v3> normals;

    u8 material;
    while (true)
    {
        char* line = consume_line(&file);
        if (!line) break;

        char* what = consume_token(&line);
        if (!strcmp(what, "v"))
        {
            v3 vertex;
            sscanf(line, "%f%f%f", &vertex.x, &vertex.y, &vertex.z);
            vertices.push_back(vertex);
        }
        else if (!strcmp(what, "vn"))
        {
            v3 normal;
            sscanf(line, "%f%f%f", &normal.x, &normal.y, &normal.z);
            normals.push_back(normal);
        }
        else if (!strcmp(what, "f"))
        {
            std::vector<Mesh_Vertex> face;
            while (*line)
            {
                char* token = consume_token(&line);
                int vertex_index, normal_index;
                sscanf(token, "%d//%d", &vertex_index, &normal_index);

                v3 vertex = vertices[vertex_index - 1];
                v3 normal = glm::normalize(normals[normal_index - 1]);
                face.push_back({ vertex, normal, (float) material });
            }

            for (int i = 1; i < face.size() - 1; i++)
            {
                mesh->vertices.push_back(face[0]);
                mesh->vertices.push_back(face[i]);
                mesh->vertices.push_back(face[i + 1]);
            }
        }
        else if (!strcmp(what, "usemtl"))
        {
            char* name = consume_token(&line);
            for (int i = 0; i < mesh->num_materials; i++)
            {
                if (!strcmp(mesh->materials[i].name, name))
                {
                    material = i;
                    goto found;
                }
            }
            critical("Couldn't find material %s in %s\n", name, path);
            found:;
        }
    }

    free(bytes.data);
}

void create_mesh(const char* path, Mesh* mesh)
{
    load_mesh_from_obj((char*) path, mesh);

    glGenBuffers(1, &mesh->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh->vertices.size() * sizeof(Mesh_Vertex), &mesh->vertices[0], GL_STATIC_DRAW);
}

GLuint mesh_program;
GLuint mesh_u_perspective_view;
GLuint mesh_u_model;
GLuint mesh_u_camera;
GLuint mesh_u_material_diffuse;
GLuint mesh_u_material_specular;
GLuint mesh_u_material_shininess;
GLuint mesh_u_light_direction;
GLuint mesh_u_color_multiplier;

void load_mesh_shader()
{
    mesh_program = create_opengl_shader("data/shaders/mesh.vs", "data/shaders/mesh.fs");
    glBindAttribLocation(mesh_program, 0, "vertex_position");
    glBindAttribLocation(mesh_program, 1, "vertex_normal");
    glBindAttribLocation(mesh_program, 2, "vertex_material");

    mesh_u_perspective_view   = glGetUniformLocation(mesh_program, "perspective_view");
    mesh_u_model              = glGetUniformLocation(mesh_program, "model");
    mesh_u_camera             = glGetUniformLocation(mesh_program, "camera");
    mesh_u_material_diffuse   = glGetUniformLocation(mesh_program, "material_diffuse");
    mesh_u_material_specular  = glGetUniformLocation(mesh_program, "material_specular");
    mesh_u_material_shininess = glGetUniformLocation(mesh_program, "material_shininess");
    mesh_u_light_direction    = glGetUniformLocation(mesh_program, "light_direction");
    mesh_u_color_multiplier   = glGetUniformLocation(mesh_program, "color_multiplier");
}

void begin_mesh(Mesh* mesh)
{
    glUseProgram(mesh_program);

    v3    materials_diffuse  [MAX_MATERIALS];
    v3    materials_specular [MAX_MATERIALS];
    float materials_shininess[MAX_MATERIALS];

    int material_count = mesh->num_materials;
    for (int i = 0; i < material_count; i++)
    {
        materials_diffuse  [i] = mesh->materials[i].diffuse;
        materials_specular [i] = mesh->materials[i].specular;
        materials_shininess[i] = mesh->materials[i].shininess * 0.3;
    }

    glUniformMatrix4fv(mesh_u_perspective_view, 1, GL_FALSE, (GLfloat*) &perspective_view);
    glUniform3fv(mesh_u_camera,             1,              (GLfloat*) &camera);
    glUniform3fv(mesh_u_material_diffuse,   material_count, (GLfloat*) materials_diffuse);
    glUniform3fv(mesh_u_material_specular,  material_count, (GLfloat*) materials_specular);
    glUniform1fv(mesh_u_material_shininess, material_count, (GLfloat*) materials_shininess);
    glUniform3fv(mesh_u_light_direction,    1,              (GLfloat*) &light_direction);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh_Vertex), (void*)(0 * sizeof(float)));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh_Vertex), (void*)(3 * sizeof(float)));
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Mesh_Vertex), (void*)(6 * sizeof(float)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
}

void end_mesh()
{
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}

void set_mesh_color_multiplier(v3 color_multiplier)
{
    glUniform3fv(mesh_u_color_multiplier, 1, (GLfloat*) &color_multiplier);
}

void render_mesh(Mesh* mesh, m4& model)
{
    glUniformMatrix4fv(mesh_u_model, 1, GL_FALSE, (GLfloat*) &model);
    glDrawArrays(GL_TRIANGLES, 0, mesh->vertices.size());
}

void render_mesh(Mesh* mesh, v3 position, float orientation)
{
    auto model_matrix = glm::translate(position) * glm::rotate(orientation, glm::vec3(0.0f, 1.0f, 0.0f));
    render_mesh(mesh, model_matrix);
}
