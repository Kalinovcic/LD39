precision mediump float;

const int   MAX_MATERIALS = 8;

uniform vec3  material_diffuse  [MAX_MATERIALS];
uniform vec3  material_specular [MAX_MATERIALS];
uniform float material_shininess[MAX_MATERIALS];
uniform vec3  light_direction;

varying vec3  fragment_to_camera;
varying vec3  fragment_normal;
varying float fragment_material;

void main()
{
    int index = int(fragment_material + 0.5);

    vec3  diffuse;
    vec3  specular;
    float shininess;
         if (index == 0) { diffuse = material_diffuse[0]; specular = material_specular[0]; shininess = material_shininess[0]; }
    else if (index == 1) { diffuse = material_diffuse[1]; specular = material_specular[1]; shininess = material_shininess[1]; }
    else if (index == 2) { diffuse = material_diffuse[2]; specular = material_specular[2]; shininess = material_shininess[2]; }
    else if (index == 3) { diffuse = material_diffuse[3]; specular = material_specular[3]; shininess = material_shininess[3]; }
    else if (index == 4) { diffuse = material_diffuse[4]; specular = material_specular[4]; shininess = material_shininess[4]; }
    else if (index == 5) { diffuse = material_diffuse[5]; specular = material_specular[5]; shininess = material_shininess[5]; }
    else if (index == 6) { diffuse = material_diffuse[6]; specular = material_specular[6]; shininess = material_shininess[6]; }
    else if (index == 7) { diffuse = material_diffuse[7]; specular = material_specular[7]; shininess = material_shininess[7]; }

    float brightness = dot(fragment_normal, -light_direction);
    if (brightness < 0.0) brightness = 0.0;
    brightness = brightness * 0.5 + 0.5;
    diffuse = diffuse * brightness;

    float specular_factor = dot(reflect(light_direction, fragment_normal), fragment_to_camera);
    if (specular_factor < 0.0) specular_factor = 0.0;
    specular_factor = pow(specular_factor, shininess);

    gl_FragColor = vec4(diffuse + specular * specular_factor, 1);
}
