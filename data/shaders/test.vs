precision mediump float;

attribute vec3 position;
attribute vec3 color;

varying vec3 fragment_color;

void main()
{
    gl_Position = vec4(position, 0.5);
    fragment_color = color;
}
