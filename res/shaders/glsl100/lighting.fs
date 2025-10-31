#version 300 es

precision mediump float;

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;
uniform sampler2D emissiveMap;
uniform vec4 colDiffuse;
uniform vec4 fogParams = vec4(0.0, 0.0, 0.0, 0.0);

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

    vec2 texelSz = vec2(12.0);
    vec2 tcCenter = (floor(fragTexCoord * texelSz)) / texelSz + 0.5 / texelSz;
    vec2 fragCoord = tcCenter - fragTexCoord;
    vec2 dtc_dx = dFdx(fragTexCoord);
    vec2 dtc_dy = dFdy(fragTexCoord);
    mat2x2 dinv = mat2x2(dtc_dy.y, -dtc_dy.x, -dtc_dx.y, dtc_dx.x) * (1.0 / (dtc_dx.x * dtc_dy.y - dtc_dy.x * dtc_dx.y));
    vec2 dst = fragCoord * dinv;
    vec3 dp_dx = dFdx(fragPosition);
    vec3 dp_dy = dFdy(fragPosition);
    vec3 dp = clamp(dp_dx * dst.x + dp_dy * dst.y, -1.0, 1.0);
    vec3 texelPosition = fragPosition + dp;

    vec4 fogColor = vec4(fogParams.x, fogParams.y, fogParams.z, 1.0);
    float fogStr = clamp(length(texelPosition - viewPos) * fogParams.w, 0.0, 1.0);
    texelColor = mix(texelColor, fogColor, fogStr);
    texelEmission = mix(texelEmission, fogColor, fogStr);

    vec3 lightDot = vec3(0.0);
    vec3 normal = normalize(fragNormal);
    vec3 viewD = normalize(viewPos - fragPosition);
    vec3 specular = vec3(0.0);

    vec4 tint = colDiffuse * fragColor;

    // NOTE: Implement here your fragment shader code

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
                mag2 = 1.0;
            }

            if (lights[i].type == LIGHT_POINT)
            {
                light = normalize(diff);
            }

            float NdotL = max(dot(normal, light), 0.0);
            lightDot += lights[i].color.rgb*(NdotL + 1.0) * 0.5 * lights[i].color.a / mag2;
        }
    }

    lightDot = floor(sqrt(lightDot) * 4.0) / 4.0;
    lightDot *= lightDot;

    finalColor = (texelColor*(tint*vec4(min(lightDot, vec3(1.0, 1.0, 1.0)), 1.0))) + texelEmission * min(lightDot.r + 0.5, 1.0);
    finalColor = vec4(finalColor.rgb, texelColor.a);

    // Gamma correction
    //finalColor = finalColor;
}
