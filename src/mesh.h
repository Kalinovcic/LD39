#pragma once

const int MAX_MATERIALS = 8;

struct Material
{
    char* name;
    v3    diffuse;
    v3    specular;
    float shininess;
};

struct Mesh_Vertex
{
    v3    position;
    v3    normal;
    float material_id;
};

struct Mesh
{
    Material materials[MAX_MATERIALS];
    u32      num_materials;

    std::vector<Mesh_Vertex> vertices;

    GLuint vbo;
};

u32 load_materials_from_mtl(char* path, Material* destination);
void load_mesh_from_obj(char* path, Mesh* mesh);
void create_mesh(const char* path, Mesh* mesh);

void load_mesh_shader();
void begin_mesh(Mesh* mesh);
void end_mesh();
void set_mesh_color_multiplier(v3 color_multiplier);
void render_mesh(Mesh* mesh, m4& model);
void render_mesh(Mesh* mesh, v3 position, float orientation);
