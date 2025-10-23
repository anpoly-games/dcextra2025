#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform vec2 textureSize;

// Output fragment color
out vec4 finalColor;

void main()
{
    vec2 texcoord = fragTexCoord - vec2(floor(fragTexCoord.x), floor(fragTexCoord.y));
    vec4 texel = texture(texture0, texcoord);   // Get texel color

    finalColor = texel;
}

