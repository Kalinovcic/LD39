precision mediump float;

uniform vec3 color;
uniform sampler2D tex;

varying vec2 fragment_texcoord;

void main()
{
    float alpha = texture2D(tex, fragment_texcoord).a;
    gl_FragColor = vec4(color, alpha);
}
