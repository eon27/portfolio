#version 330

in vec3 vPosition;
in vec3 vNormal;
in vec2 vTexture;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

out vec3 Normal;
out vec3 FragPos; 
out vec2 TexCoord;

void
main()
{   
    gl_Position = P*V*M*vec4(vPosition, 1.0);
    Normal = (M*vec4(vNormal, 0.0)).xyz;
    FragPos = vec3(M * vec4(vPosition, 1.0));
    TexCoord = vTexture;
}
