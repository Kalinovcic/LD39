precision mediump float;

uniform mat4 mvp;

attribute vec3 position;
attribute vec3 color;

varying vec3 fragment_color;

void main()
{
    gl_Position = mvp * vec4(position, 1.0);
    fragment_color = color;
}
