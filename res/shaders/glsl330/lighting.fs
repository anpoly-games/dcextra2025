#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;
in vec3 fragBinormal;
in vec3 fragTangent;

// Input uniform values
uniform sampler2D texture0;
uniform sampler2D emissiveMap;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

// NOTE: Add your custom variables here

#define     MAX_LIGHTS              4
#define     LIGHT_DIRECTIONAL       0
#define     LIGHT_POINT             1

struct Light {
    int enabled;
    int type;
    vec3 position;
    vec3 target;
    vec4 color;
};

// Input lighting values
uniform Light lights[MAX_LIGHTS];
uniform vec4 ambient;
uniform vec3 viewPos;

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);
    if (texelColor.a == 0.0)
      discard;
    vec4 texelEmission = texture(emissiveMap, fragTexCoord);
    // pixel position
    //vec2 fragCoord = (floor(fragTexCoord * 12.0) + 0.5) / 12.0 - fragTexCoord;
    //vec3 texelPosition = fragPosition + fragCoord.x * fragBinormal - fragCoord.y * fragTangent;
    vec3 texelPosition = fragPosition;
    vec3 lightDot = vec3(0.0);
    vec3 normal = normalize(fragNormal);
    vec3 specular = vec3(0.0);

    vec4 tint = colDiffuse * fragColor;

    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (lights[i].enabled == 1)
        {
            vec3 light = vec3(0.0);

            vec3 diff = lights[i].position - texelPosition;
            float mag2 = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
            //float mag = sqrt(mag2);

            if (lights[i].type == LIGHT_DIRECTIONAL)
            {
                light = -normalize(lights[i].target - lights[i].position);
            }

            if (lights[i].type == LIGHT_POINT)
            {
                light = normalize(diff);
            }

            float NdotL = max(dot(normal, light), 0.0);
            lightDot += lights[i].color.rgb*(NdotL + 1.0) * 0.5 * lights[i].color.a / mag2;
        }
    }

    //lightDot = floor(lightDot * 4.0) / 4.0;

    finalColor = (texelColor*(tint*max(vec4(min(lightDot, vec3(1.0, 1.0, 1.0)), 1.0), ambient))) + texelEmission * min(lightDot.r + 0.5, 1.0);
    finalColor = vec4(finalColor.rgb, texelColor.a);

    // Gamma correction
    //finalColor = pow(finalColor, vec4(1.0/2.2));
}
