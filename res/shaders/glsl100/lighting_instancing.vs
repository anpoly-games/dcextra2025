#version 100

// Input vertex attributes
attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord;
attribute vec3 vertexNormal;
attribute vec4 vertexColor;

attribute mat4 instanceTransform;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matNormal;

// Output vertex attributes (to fragment shader)
varying vec3 fragPosition;
varying vec2 fragTexCoord;
varying vec4 fragColor;
varying vec3 fragNormal;

// NOTE: Add your custom variables here

void main()
{
    // Compute MVP for current instance
    mat4 itrm = instanceTransform;
    vec4 instanceColor = vec4(itrm[0][3], itrm[1][3], itrm[2][3], itrm[3][3]);
    itrm[0][3] = 0.0;
    itrm[1][3] = 0.0;
    itrm[2][3] = 0.0;
    itrm[3][3] = 1.0;
    mat4 mvpi = mvp*itrm;

    // Send vertex attributes to fragment shader
    fragPosition = vec3(mvpi*vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor * instanceColor;
    fragNormal = normalize(vec3(matNormal*vec4(vertexNormal, 1.0)));

    // Calculate final vertex position
    gl_Position = mvpi*vec4(vertexPosition, 1.0);
}
