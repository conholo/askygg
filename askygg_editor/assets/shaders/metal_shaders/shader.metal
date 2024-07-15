#include <metal_stdlib>
using namespace metal;

struct VertexOut
{
    float4 position [[position]];
    float4 color;
};

VertexOut vertex vertexMain(uint vertexId [[vertex_id]],
                            device const float3* positions [[buffer(0)]],
                            device const float3* colors [[buffer(1)]])
{
    VertexOut out;
    out.position = float4(positions[vertexId], 1.0);
    out.color = float4(colors[vertexId], 1.0);
    return out;
}

float4 fragment fragmentMain(VertexOut in [[stage_in]])
{
    return in.color;
}