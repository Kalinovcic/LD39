precision mediump float;

uniform mat4 perspective_view;
uniform mat4 model;
uniform vec3 camera;

attribute vec3  vertex_position;
attribute vec3  vertex_normal;
attribute float vertex_material;

varying vec3  fragment_to_camera;
varying vec3  fragment_normal;
varying float fragment_material;

void main()
{
    vec4 world_position = model * vec4(vertex_position, 1.0);
    gl_Position = perspective_view * world_position;

    fragment_to_camera = normalize(camera - world_position.xyz);
    fragment_normal    = normalize((model * vec4(vertex_normal, 0.0)).xyz);
    fragment_material  = vertex_material;
}
