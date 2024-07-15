#type compute
#version 450 core

layout(binding = 0, rgba32f) restrict writeonly uniform image2D o_Image;

uniform float u_Strength;
uniform float u_Threshold;
uniform sampler2D u_Texture;

layout(local_size_x = 4, local_size_y = 4) in;
void main()
{
    const float Kx[9] = float[9]
    (
        -1, 0, 1,
        -2, 0, 2,
        -1, 0, 1
    );

    const float Ky[9] = float[9]
    (
        -1, -2, -1,
        0,  0,  0,
        1,  2,  1
    );


    vec2 imgSize = vec2(imageSize(o_Image));
    ivec2 invocID = ivec2(gl_GlobalInvocationID);
    vec2 texCoords = vec2(float(invocID.x) / imgSize.x, float(invocID.y) / imgSize.y);
    texCoords += (1.0f / imgSize) * 0.5f;

    vec2 texelSize = 1.0 / textureSize(u_Texture, 0);
    float sobelX = 0.0;
    float sobelY = 0.0;
    int kernelIndex = 0;

    vec2 clampedCoords = clamp(texCoords, texelSize, 1.0 - texelSize);

    for(int y = -1; y <= 1; ++y)
    {
        for(int x = -1; x <= 1; ++x)
        {
            vec3 color = texture(u_Texture, clampedCoords + vec2(x, y) * texelSize).rgb;
            float I = 0.21*color.r + 0.72*color.g + 0.07*color.b;
            sobelX += I * Kx[kernelIndex];
            sobelY += I * Ky[kernelIndex];
            kernelIndex++;
        }
    }
    float sobel = sqrt(sobelX * sobelX + sobelY * sobelY);
    sobel *= step(u_Threshold, sobel);
    vec4 originalColor = texture(u_Texture, texCoords);
    vec4 finalColor = originalColor + vec4(vec3(sobel * u_Strength), 0.0);
    imageStore(o_Image, invocID, finalColor);
}