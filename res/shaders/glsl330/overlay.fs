#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform vec2 textureSize;
uniform vec4 overlayColor;

// Output fragment color
out vec4 finalColor;

void main()
{
    vec4 texel = texture(texture0, fragTexCoord);   // Get texel color

    vec4 color = mix(vec4(overlayColor.x, overlayColor.y, overlayColor.z, 0.0), overlayColor, texel.a * fragColor.a);
    finalColor = color;
}

