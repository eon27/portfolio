#version 330

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

out vec4  fColor;

uniform vec3 camPos;

uniform vec3 lightPos;
uniform vec3 ambientColor;
uniform vec3 lightColor;

uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float materialShininess;

uniform bool useTexture;
uniform sampler2D textureMap;

void
main()
{
    // Ambient light calculation
    vec3 ambient = ambientColor * materialAmbient;

    // Lightsource position
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float lightDist = pow(length(lightPos - FragPos), 2)/2;

    // Diffuse light calculation
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * materialDiffuse * lightColor / lightDist;

    vec3 specular;
    if (length(diffuse) > 0) {
        // Specular light calculation
        vec3 camDir = normalize(camPos - FragPos);
        vec3 reflectionDir = reflect(-lightDir, norm);
        float reflection = max(dot(camDir, reflectionDir), 0.0);
        specular = pow(reflection, materialShininess) * materialSpecular * lightColor / lightDist;
    } else {
        specular = vec3(0,0,0);
    }

    vec4 tex;
    if (useTexture) {
        tex = texture(textureMap, TexCoord);
    } else {
        tex = vec4(1,1,1,1);
    }

    vec3 color = (ambient + diffuse + specular) * tex.xyz;

    fColor = vec4(color, tex.w);
}
