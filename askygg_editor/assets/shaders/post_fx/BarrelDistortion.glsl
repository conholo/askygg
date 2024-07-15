#type compute
#version 450 core

layout(binding = 0, rgba32f) restrict writeonly uniform image2D o_Image;

uniform vec2 u_Distortion; // Distortion strength, use small values like 0.02
uniform sampler2D u_Texture;

layout(local_size_x = 4, local_size_y = 4) in;
void main()
{
    vec2 texCoords = vec2(gl_GlobalInvocationID.xy) / vec2(imageSize(o_Image));
    // Map texCoords to [-1, 1]
    vec2 p = texCoords * 2.0 - vec2(1.0);

    // Radial distance of current pixel
    float r = length(p);
    if (r < 1.0)
    {
        // Current pixel * radial distortion
        // Quadratic function of r if u_Distortion.y is zero, and a quartic function of r if both u_Distortion.x
        // and u_Distortion.y are nonzero. This creates the barrel or pincushion distortion effect.
        vec2 dp = p * (1.0 + r * (u_Distortion.x + u_Distortion.y * r));
        texCoords = dp * 0.5 + vec2(0.5);
    }

    vec4 color = texture(u_Texture, texCoords);
    imageStore(o_Image, ivec2(gl_GlobalInvocationID.xy), color);
}
