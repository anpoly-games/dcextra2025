#version 100

precision mediump float;

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform vec2 textureSize;
uniform vec4 overlayColor;

void main()
{
    vec4 texel = texture2D(texture0, fragTexCoord);   // Get texel color

    vec4 color = mix(vec4(overlayColor.x, overlayColor.y, overlayColor.z, 0.0), overlayColor, texel.a * fragColor.a);
    gl_FragColor = color;
}

