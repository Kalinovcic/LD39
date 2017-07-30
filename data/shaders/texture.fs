precision mediump float;

uniform vec3 color;
uniform sampler2D tex;

varying vec2 fragment_texcoord;

void main()
{
    gl_FragColor = texture2D(tex, fragment_texcoord) * vec4(color, 1.0);
}
