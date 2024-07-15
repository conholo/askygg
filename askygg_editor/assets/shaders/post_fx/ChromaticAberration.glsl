#type compute
#version 450 core

layout(binding = 0, rgba32f) restrict writeonly uniform image2D o_Image;

uniform sampler2D u_Texture;
uniform float u_Strength;


layout(local_size_x = 4, local_size_y = 4) in;
void main()
{
    vec2 imgSize = vec2(imageSize(o_Image));
    ivec2 invocID = ivec2(gl_GlobalInvocationID);
    vec2 texCoords = vec2(float(invocID.x) / imgSize.x, float(invocID.y) / imgSize.y);
    texCoords += (1.0f / imgSize) * 0.5f;

    vec2 offsetRed = vec2(u_Strength, 0.0);
    vec2 offsetBlue = -vec2(u_Strength, 0.0);

    // Sample each color channel separately with different offsets
    float r = texture(u_Texture, texCoords + offsetRed).r;
    float g = texture(u_Texture, texCoords).g;
    float b = texture(u_Texture, texCoords + offsetBlue).b;

    vec3 color = vec3(r, g, b);

    imageStore(o_Image, invocID, vec4(color, 1.0));
}