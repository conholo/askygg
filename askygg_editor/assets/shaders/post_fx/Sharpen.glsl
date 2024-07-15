#type compute
#version 450 core

layout(binding = 0, rgba32f) restrict writeonly uniform image2D o_Image;

uniform float u_SharpenStrength;
uniform sampler2D u_Texture;

const mat3 kernel = mat3(
    -1, -1, -1,
    -1,  9, -1,
    -1, -1, -1
);


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
    vec2 imgSize = vec2(imageSize(o_Image));
    ivec2 invocID = ivec2(gl_GlobalInvocationID);
    vec2 texCoords = vec2(float(invocID.x) / imgSize.x, float(invocID.y) / imgSize.y);
    texCoords += (1.0f / imgSize) * 0.5f;
    vec2 texelSize = 1.0 / imgSize;

    vec3 color = vec3(0.0);
    float yAccum = 0.0;
    float yOriginal = 0.0;

    for (int j = -1; j <= 1; ++j)
    {
        for (int i = -1; i <= 1; ++i)
        {
            vec3 texelColor = texture(u_Texture, texCoords + vec2(i, j) * texelSize).rgb;
            vec3 converted = RGBToYCbCr(texelColor);

            yAccum += converted.x * kernel[j+1][i+1];
            color += texelColor;

            if (i == 0 && j == 0)
            {
                yOriginal = converted.x;
            }
        }
    }

    vec3 avgColor = color / 9.0;  // divide by the number of kernel elements
    vec3 avgYCbCr = RGBToYCbCr(avgColor);

    // Mix the original and sharpened Y components based on the slider
    float finalY = mix(yOriginal, yAccum, u_SharpenStrength);

    avgYCbCr.x = finalY;
    vec3 finalColor = YCbCrToRGB(avgYCbCr);

    imageStore(o_Image, invocID, vec4(finalColor, 1.0));
}