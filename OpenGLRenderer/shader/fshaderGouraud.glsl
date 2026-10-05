#version 330

in vec4 fColor;
in vec2 TexCoord;

uniform bool useTexture;
uniform sampler2D textureMap;

out vec4 outColor;

void
main()
{
    vec4 tex;
    if (useTexture) {
        tex = texture(textureMap, TexCoord);
    } else {
        tex = vec4(1,1,1,1);
    }

    outColor = fColor * tex;
}
