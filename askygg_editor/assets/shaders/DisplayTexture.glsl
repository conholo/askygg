#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Binormal;
layout(location = 4) in vec2 a_TexCoord;

out vec2 v_TexCoord;

void main()
{
    v_TexCoord = a_TexCoord;
    gl_Position = vec4(a_Position.xy, 0.0, 1.0);
}

#type fragment
#version 450

layout(location = 0) out vec4 o_Color;

uniform sampler2D u_Texture;
in vec2 v_TexCoord;

void main()
{
    vec3 color = texture(u_Texture, v_TexCoord).rgb;
    o_Color = vec4(color, 1.0);
}