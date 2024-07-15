#type compute
#version 450 core

layout(binding = 0, rgba32f) restrict writeonly uniform image2D o_Image;

uniform float u_BlurStrength;
uniform int u_BlurSamples;
uniform vec2 u_BlurDirection;
uniform sampler2D u_Texture;

layout(local_size_x = 4, local_size_y = 4) in;
void main()
{
    vec2 imgSize = vec2(imageSize(o_Image));
    ivec2 invocID = ivec2(gl_GlobalInvocationID);
    vec2 texCoords = vec2(float(invocID.x) / imgSize.x, float(invocID.y) / imgSize.y);
    texCoords += (1.0f / imgSize) * 0.5f;
    vec4 originalColor = textureLod(u_Texture, texCoords, 0.0);

    vec4 blurColor = vec4(0.0);
    const int numBlurSamples = max(0, u_BlurSamples);
    for (int i = 0; i < u_BlurSamples; ++i)
    {
        float t = float(i) / float(numBlurSamples - 1);
        vec2 offset = (t - 0.5) * u_BlurDirection * u_BlurStrength;
        blurColor += textureLod(u_Texture, texCoords + offset, 0.0);
    }
    blurColor /= numBlurSamples;

    vec4 finalColor = mix(originalColor, blurColor, u_BlurStrength);

    imageStore(o_Image, ivec2(gl_GlobalInvocationID.xy), finalColor);
}
