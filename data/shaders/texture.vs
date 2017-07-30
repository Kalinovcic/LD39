precision mediump float;

uniform mat4 ortho;

attribute vec2 vertex_position;
attribute vec2 vertex_texcoord;

varying vec2 fragment_texcoord;

void main()
{
    gl_Position = ortho * vec4(vertex_position, 0.0, 1.0);
    fragment_texcoord = vertex_texcoord;
}
