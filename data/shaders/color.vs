precision mediump float;

uniform mat4 ortho;

attribute vec2 vertex_position;

void main()
{
    gl_Position = ortho * vec4(vertex_position, 0.0, 1.0);
}
