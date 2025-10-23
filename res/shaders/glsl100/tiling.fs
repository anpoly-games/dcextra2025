#version 100

precision mediump float;

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform vec2 textureSize;

void main()
{
    vec2 texcoord = fragTexCoord - vec2(floor(fragTexCoord.x), floor(fragTexCoord.y));
    vec4 texel = texture2D(texture0, texcoord);   // Get texel color

    gl_FragColor = texel;
}

