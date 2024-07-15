#type compute
#version 450 core
layout(binding = 0, rgba32f) restrict writeonly uniform image2D o_Image;

uniform sampler2D u_Texture;
uniform float u_Radius = 0.75;  // Controls the size of the vignette. Smaller values create a larger dark border.
uniform float u_Softness = 0.45;  // Controls the falloff of the vignette effect. Larger values create a softer border.

layout(local_size_x = 4, local_size_y = 4) in;
void main()
{
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    vec2 imgSize = vec2(imageSize(o_Image));
    vec2 texCoords = vec2(pixelCoords) / imgSize;

    float dist = distance(texCoords, vec2(0.5));  // Distance from the center of the image
    float vignette = smoothstep(u_Radius, u_Radius - u_Softness, dist);  // Calculate the vignette effect

    vec4 color = texture(u_Texture, texCoords);
    color.rgb *= vignette;
    imageStore(o_Image, pixelCoords, color);
}
