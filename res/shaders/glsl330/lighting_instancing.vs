#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;      // Not required

in mat4 instanceTransform;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matNormal;

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;

// NOTE: Add your custom variables here

void main()
{
    // Send vertex attributes to fragment shader
    mat4 itrm = instanceTransform;
    vec4 instanceColor = vec4(itrm[0][3], itrm[1][3], itrm[2][3], itrm[3][3]);
    itrm[0][3] = 0.0;
    itrm[1][3] = 0.0;
    itrm[2][3] = 0.0;
    itrm[3][3] = 1.0;
    fragPosition = vec3(itrm*vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor * instanceColor;
    fragNormal = normalize(vec3(matNormal*vec4(vertexNormal, 1.0)));

    // Calculate final vertex position, note that we multiply mvp by instanceTransform
    gl_Position = mvp*itrm*vec4(vertexPosition, 1.0);
}
