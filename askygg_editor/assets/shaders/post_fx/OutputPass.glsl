#type compute
#version 450

layout(binding = 0, rgba32f) restrict readonly uniform image2D i_EnhancedImage;
layout(binding = 1, rgba8) restrict writeonly uniform image2D o_Composite;


uniform sampler2D u_SensorNoisePatchTexture;
uniform float u_NoiseAlpha;
uniform float u_NoiseBeta;
uniform float u_NoiseGamma;

uniform float u_NoiseAmplitude;
uniform float u_NoiseFrequency;
uniform float u_Time;

uniform float u_Exposure;
uniform int u_BloomEnabled;
uniform int u_Tonemapper;
uniform float u_WhitePoint;

uniform int u_BloomType;
uniform sampler2D u_BloomTexture;
uniform sampler2D u_BloomDirtTexture;

uniform float u_BloomDirtIntensity;
uniform float u_BloomIntensity;
uniform float u_UpsampleRadius;
uniform float u_UpsampleTightenFactor;

#define NO_TONEMAPPER           0
#define ACES_TONEMAPPER         1
#define REINHARD_TONEMAPPER     2
#define REINHARD_EXT_TONEMAPPER 3

#define NONE                 0
#define RADIAL_BLOOM        -1
#define MULTI_PASS_BLOOM    -2

vec3 ACESTonemap(vec3 color)
{
    mat3 m1 = mat3(
    0.59719, 0.07600, 0.02840,
    0.35458, 0.90834, 0.13383,
    0.04823, 0.01566, 0.83777
    );

    mat3 m2 = mat3(
    1.60475, -0.10208, -0.00327,
    -0.53108, 1.10813, -0.07276,
    -0.07367, -0.00605, 1.07602
    );

    vec3 v = m1 * color;
    vec3 a = v * (v + vec3(0.0245786)) - vec3(0.000090537);
    vec3 b = v * (vec3(0.983729) * v + vec3(0.4329510)) + vec3(0.238081);
    vec3 c = m2 * (a / b);
    vec3 result = clamp(c, 0.0, 1.0);

    return result;
}

vec3 ReinhardTonemap(vec3 color, float whitePoint)
{
    return color / (color + whitePoint);
}

vec3 ReinhardExtendedTonemap(vec3 color, float whitePoint)
{
    vec3 numerator = color * (1.0f + (color / vec3(whitePoint * whitePoint)));
    return numerator / (1.0f + color);
}

vec3 LinearToGamma(vec3 color, float gamma)
{
    return pow(color, vec3(1.0f / gamma));
}

// Hash functions by Dave_Hoskins
vec2 hash21(float p)
{
    vec3 p3 = fract(vec3(p) * vec3(.1031, .1030, .0973));
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.xx+p3.yz)*p3.zy);

}

vec3 UpsampleTent9(sampler2D tex, float lod, vec2 uv, vec2 texelSize, float radius)
{
    vec4 offset = texelSize.xyxy * vec4(1.0f, 1.0f, -1.0f, 0.0f) * radius;

    // Center
    vec3 result = textureLod(tex, uv, lod).rgb * 4.0f;

    result += textureLod(tex, uv - offset.xy, lod).rgb;
    result += textureLod(tex, uv - offset.wy, lod).rgb * 2.0;
    result += textureLod(tex, uv - offset.zy, lod).rgb;

    result += textureLod(tex, uv + offset.zw, lod).rgb * 2.0;
    result += textureLod(tex, uv + offset.xw, lod).rgb * 2.0;

    result += textureLod(tex, uv + offset.zy, lod).rgb;
    result += textureLod(tex, uv + offset.wy, lod).rgb * 2.0;
    result += textureLod(tex, uv + offset.xy, lod).rgb;

    return result * (1.0f / u_UpsampleTightenFactor);
}

layout(local_size_x = 4, local_size_y = 4) in;
void main()
{
    const float Gamma = 2.2;

    ivec2 invocID = ivec2(gl_GlobalInvocationID.xy);
    vec2 targetSize = vec2(imageSize(i_EnhancedImage));
    vec2 texCoords = vec2(invocID) / targetSize;

    vec2 texelSize = 1.0f / targetSize;
    texCoords += texelSize * 0.5f;

    vec3 result = imageLoad(i_EnhancedImage, invocID).rgb;

    if(u_BloomEnabled == 1)
    {
        vec3 bloomColor = vec3(0.0);
        vec3 bloomDirt = texture(u_BloomDirtTexture, texCoords).rgb * u_BloomDirtIntensity;

        if(u_BloomType == RADIAL_BLOOM)
        {
            bloomColor = texture(u_BloomTexture, texCoords).rgb;
            result = mix(result, result + bloomColor, u_BloomIntensity);
        }
        else if(u_BloomType == MULTI_PASS_BLOOM)
        {
            vec2 fBloomTexSize = vec2(textureSize(u_BloomTexture, 0));

            bloomColor = UpsampleTent9(u_BloomTexture, 0, texCoords, 1.0f / fBloomTexSize, u_UpsampleRadius) * u_BloomIntensity;
            result += bloomColor + bloomColor * bloomDirt;
        }
    }

    result = result * u_Exposure;

    if(u_Tonemapper == ACES_TONEMAPPER)
    {
        result = ACESTonemap(result);
    }
    else if(u_Tonemapper == REINHARD_TONEMAPPER)
    {
        result = ReinhardTonemap(result, u_WhitePoint);
    }
    else if(u_Tonemapper == REINHARD_EXT_TONEMAPPER)
    {
        result = ReinhardExtendedTonemap(result, u_WhitePoint);
    }

    vec2 noiseTexCoords = vec2(float(invocID.x) / targetSize.x, float(invocID.y) / targetSize.y);
    vec2 perturbedUV = (noiseTexCoords + hash21(u_Time * u_NoiseFrequency)) * u_NoiseAmplitude;

    ivec2 noisePatchSize = textureSize(u_SensorNoisePatchTexture, 0);
    vec2 noiseUV = perturbedUV * (targetSize / vec2(noisePatchSize));
    vec3 tiledNoise = texture(u_SensorNoisePatchTexture, noiseUV).rgb;
    vec3 resultBlendWithNoise = u_NoiseAlpha * result + u_NoiseBeta * tiledNoise + vec3(u_NoiseGamma);

    result = LinearToGamma(resultBlendWithNoise, Gamma);
    imageStore(o_Composite, invocID, vec4(result, 1.0));
}