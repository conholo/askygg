#type compute
#version 450 core

layout(binding = 0, rgba32f) restrict writeonly uniform image2D o_Image;

uniform sampler2D u_Texture;
uniform float u_HueShift;  // Ranges from -1.0 to 1.0
uniform float u_SaturationBoost; // Can be any value, but useful range is typically around -1.0 to 1.0
uniform float u_ValueBoost;      // Can be any value, but useful range is typically around -1.0 to 1.0

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

layout(local_size_x = 4, local_size_y = 4) in;
void main()
{
    vec2 imgSize = vec2(imageSize(o_Image));
    ivec2 invocID = ivec2(gl_GlobalInvocationID);
    vec2 texCoords = vec2(float(invocID.x) / imgSize.x, float(invocID.y) / imgSize.y);
    texCoords += (1.0f / imgSize) * 0.5f;
    vec3 color = texture(u_Texture, texCoords).rgb;

    // Convert the RGB color to HSV
    vec3 hsv = rgb2hsv(color);

    // Modify the hue, saturation, and value
    hsv.x = fract(hsv.x + clamp(u_HueShift, -1.0, 1.0));
    hsv.y = clamp(hsv.y + u_SaturationBoost, 0.0, 1.0);
    hsv.z = clamp(hsv.z + u_ValueBoost, 0.0, 1.0);

    // Convert the modified HSV color back to RGB
    color = hsv2rgb(hsv);
    imageStore(o_Image, ivec2(gl_GlobalInvocationID.xy), vec4(color, 1.0));
}