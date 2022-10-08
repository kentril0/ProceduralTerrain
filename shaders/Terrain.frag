#version 450

// -----------------------------------------------------------------------------
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

// -----------------------------------------------------------------------------
layout(location = 0) out vec4 outColor;

// -----------------------------------------------------------------------------

#define EPSILON 1e-5
#define REGION_MAX_COUNT 8

struct Region
{
    float scale;            ///< texture scale
    int texIndex;           ///< texture index in the texture array
    float blendStrength;
    float startHeight;
    vec4 tint;              ///< rgb: tint, a: tintStrength
};

uniform sampler2DArray texArray;

layout(binding=1) uniform TerrainUBO {
    float minHeight;
    float maxHeight;
    int regionCount;
    float __pad;
    Region regions[REGION_MAX_COUNT];
} terrain;

layout(binding=2) uniform LightingUBO {
    vec4 sunColor;      ///< rgb: sunColor, a: sunItensity
    vec4 sunDir;
    vec4 skyColor;
    vec4 bounceColor;
} lighting;

// -----------------------------------------------------------------------------

float InverseLerp(float a, float b, float x) {
    return clamp( (x-a) / (b-a), 0.0, 1.0);
}

vec3 ComputeLighting(const in vec3 kMaterial)
{
    // Sun contribution
    const vec3 kSunDir = normalize(lighting.sunDir.xyz);
    const float kSunDiff = clamp( dot(inNormal, kSunDir), 0.0, 1.0 );
    const float kSunIntensity = lighting.sunColor.a;
    const vec3 kSunColor = lighting.sunColor.rgb;
    vec3 color = kMaterial * kSunIntensity * kSunColor * kSunDiff;

    // Sky contribution
    const vec3 kDirUp = vec3(0.0, 1.0, 0.0);
    const float kSkyDiff = clamp(0.5 + 0.5 * dot(inNormal, kDirUp), 0.0, 1.0);
    color += kMaterial * lighting.skyColor.rgb * kSkyDiff;

    // Bounce lighting
    const vec3 kDirDown = vec3(0.0, -1.0, 0.0);
    const float kBounceDiff = clamp(0.5 + 0.5 * dot(inNormal, kDirDown),
                                    0.0, 1.0);
    color += kMaterial * lighting.bounceColor.rgb * kBounceDiff;

    return color;
}

vec3 GetTriplanarMapping(const in vec3 kBlendAxis, const in float kScale,
                         const in int kTexIndex)
{
    const vec3 kScaledPos = inPos / kScale;

    return texture(texArray, vec3(kScaledPos.yz, kTexIndex) ).rgb * kBlendAxis.x +
           texture(texArray, vec3(kScaledPos.xz, kTexIndex) ).rgb * kBlendAxis.y +
           texture(texArray, vec3(kScaledPos.xy, kTexIndex) ).rgb * kBlendAxis.z;
}

void main()
{
    vec3 blendAxis = abs(inNormal);
    blendAxis /= blendAxis.x + blendAxis.y + blendAxis.z;

    const float kHeightPercent = InverseLerp(terrain.minHeight, terrain.maxHeight, inPos.y);

    vec3 color = vec3(0);
    const int kRegionCount = min(terrain.regionCount, REGION_MAX_COUNT);

    for (int i = 0; i < kRegionCount; ++i)
    {
        const Region region = terrain.regions[i];

        const vec3 kTextureColor = GetTriplanarMapping(blendAxis,
                                                       region.scale, region.texIndex);
        const vec3 kBaseColor = mix(kTextureColor, region.tint.rgb, region.tint.a);

        const float kBlendStrengthHalf = region.blendStrength/2;
        const float kWeight = InverseLerp(-kBlendStrengthHalf - EPSILON,
                                           kBlendStrengthHalf,
                                           kHeightPercent - region.startHeight);
        color = mix(color, kBaseColor, kWeight);
    }

    color = ComputeLighting(color);

    //color = pow(color, vec3(0.4545));
    outColor = vec4(color, 1.0);
}
