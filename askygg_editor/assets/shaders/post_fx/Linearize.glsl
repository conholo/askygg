#type compute
#version 450 core

layout(binding = 0, rgba8) restrict readonly uniform image2D i_Image;
layout(binding = 1, rgba32f) restrict writeonly uniform image2D o_Image;

vec4 GammaToLinear(vec4 inColor, float gamma)
{
    return pow(inColor, vec4(gamma));
}

layout(local_size_x = 4, local_size_y = 4) in;
void main()
{
    ivec2 invocID = ivec2(gl_GlobalInvocationID);
    vec4 pixel = imageLoad(i_Image, invocID);
    vec4 linear = GammaToLinear(pixel, 2.2);
    imageStore(o_Image, invocID, vec4(linear.rgb, 1.0));
}