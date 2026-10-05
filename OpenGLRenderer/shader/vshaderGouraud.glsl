#version 330

in vec3 vPosition;
in vec3 vNormal;
in vec2 vTexture;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

uniform vec3 camPos;

uniform vec3 lightPos;
uniform vec3 ambientColor;
uniform vec3 lightColor;

uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float materialShininess;

out vec4 fColor;
out vec2 TexCoord;

vec3 Normal;
vec3 vertexPos;

void
main()
{   
    gl_Position = P*V*M*vec4(vPosition, 1.0);
    Normal = (M*vec4(vNormal, 0.0)).xyz;
    vertexPos = vec3(M * vec4(vPosition, 1.0));

	// Ambient light calculation
    vec3 ambient = ambientColor * materialAmbient;

    // Lightsource position
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - vertexPos);
    float lightDist = pow(length(lightPos - vertexPos), 2)/2;

    // Diffuse light calculation
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * materialDiffuse * lightColor / lightDist;

    vec3 specular;
    if (length(diffuse) > 0) {
        // Specular light calculation
        vec3 camDir = normalize(camPos - vertexPos);
        vec3 reflectionDir = reflect(-lightDir, norm);
        float reflection = max(dot(camDir, reflectionDir), 0.0);
        specular = pow(reflection, materialShininess) * materialSpecular * lightColor / lightDist;
    } else {
        specular = vec3(0,0,0);
    }

    fColor = vec4((ambient + diffuse + specular), 1);
    TexCoord = vTexture;
}
