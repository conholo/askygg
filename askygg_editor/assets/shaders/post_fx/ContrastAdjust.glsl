#type compute
#version 450 core

layout(binding = 0, rgba32f) restrict writeonly uniform image2D o_Image;

uniform float u_ContrastStrength;
uniform float u_Brightness;
uniform sampler2D u_Texture;

layout(local_size_x = 4, local_size_y = 4) in;
void main()
{
    vec2 imgSize = vec2(imageSize(o_Image));
    ivec2 invocID = ivec2(gl_GlobalInvocationID);
    vec2 texCoords = vec2(float(invocID.x) / imgSize.x, float(invocID.y) / imgSize.y);
    texCoords += (1.0f / imgSize) * 0.5f;

    vec4 originalColor = textureLod(u_Texture, texCoords, 0.0);

    // Apply contrast and then brightness
    vec4 adjustedColor = vec4((originalColor.rgb - 0.5) * u_ContrastStrength + 0.5, originalColor.a);
    adjustedColor.rgb += u_Brightness;
    imageStore(o_Image, invocID, adjustedColor);
}
