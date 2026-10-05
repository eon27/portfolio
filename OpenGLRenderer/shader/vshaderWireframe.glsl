#version 330

in vec3 vPosition;
in vec3 vNormal;
in vec2 vTexture;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

void
main()
{   
    gl_Position = P*V*M*vec4(vPosition, 1.0);
}
