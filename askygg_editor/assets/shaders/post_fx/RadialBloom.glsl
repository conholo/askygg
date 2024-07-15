#type compute
#version 450 core

layout(binding = 0, rgba32f) restrict readonly uniform image2D i_Input;
layout(binding = 1, rgba32f) restrict writeonly uniform image2D o_Output;

uniform int u_Mode;
uniform int u_BloomPixelRadius;
uniform float u_Amplitude;
uniform float u_SigmaScaleFactor;
uniform float u_LuminanceThreshold;
uniform float u_BlurColorWeight;

#define EXTRACT 0
#define HORIZONTAL 1
#define VERTICAL 2

float GaussianFn(float x, float amplitude, float sigma)
{
    return amplitude * exp(-0.5 * pow(x / sigma, 2.0));
}

// https://www.shadertoy.com/view/XltBzr
vec3 RGBToYCbCr(in vec3 c)
{
    return vec3(0.0, 0.5, 0.5) +  transpose(mat3( 0.2990, 0.5870, 0.1140,
    -0.1687, -0.3313, 0.5000,
    0.5000,-0.4187,-0.0813)) * c;
}

// https://www.shadertoy.com/view/XltBzr
vec3 YCbCrToRGB(in vec3 c)
{
    return transpose(mat3(1.0000, 0.0000, 1.4020,
    1.0000, -0.3441, -0.7141,
    1.0000, 1.7720, 0.0000)) * (c - vec3(0.0, 0.5, 0.5));
}

layout(local_size_x = 4, local_size_y = 4) in;
void main()
{
    ivec2 invocID = ivec2(gl_GlobalInvocationID.xy);
    vec4 inputColor = imageLoad(i_Input, invocID);
    vec4 result = vec4(0.0);

    if(u_Mode == EXTRACT)
    {
        float luminance = dot(inputColor.rgb, vec3(0.299, 0.587, 0.114));
        result = step(u_LuminanceThreshold, luminance) * inputColor;
    }
    else
    {
        float sigmaScaleFactor = max(0.0001, u_SigmaScaleFactor);
        float sigma = float(u_BloomPixelRadius) / sigmaScaleFactor;

        ivec2 offset = u_Mode == HORIZONTAL ? ivec2(1, 0) : ivec2(0, 1);

        vec4 sum = vec4(0.0);

        for(int i = -u_BloomPixelRadius; i <= u_BloomPixelRadius; ++i)
        {
            ivec2 neighborID = invocID + offset * i;
            vec4 neighborColor = imageLoad(i_Input, neighborID);
            float weight = GaussianFn(float(i), u_Amplitude, sigma);
            sum += neighborColor * weight;
        }

        vec3 originalYCbCr = RGBToYCbCr(inputColor.rgb);
        vec3 blurYCbCr = RGBToYCbCr(sum.rgb);

        vec3 blendedYCbCr;
        blendedYCbCr.x = blurYCbCr.x; // Luminance from blur
        blendedYCbCr.yz = originalYCbCr.yz + u_BlurColorWeight * (blurYCbCr.yz - originalYCbCr.yz);
        result.rgb = YCbCrToRGB(blendedYCbCr);
    }

    imageStore(o_Output, invocID, result);
}
