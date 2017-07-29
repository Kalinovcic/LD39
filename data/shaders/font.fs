precision mediump float;

uniform vec3 color;
uniform sampler2D font;

varying vec2 fragment_texcoord;

void main()
{
    float alpha = texture2D(font, fragment_texcoord).a;
    gl_FragColor = vec4(color, alpha);
}
