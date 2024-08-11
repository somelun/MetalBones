//
//  Triangle.metal
//  MetalBones
//
//  Created by Sasha on 03/08/2024.
//

#include <metal_stdlib>
using namespace metal;

struct VertexInput {
    float3 position [[attribute(0)]];
    float3 color [[attribute(1)]];
};

struct VertexOutput {
    float4 position [[position]];
    half3 color;
};

VertexOutput vertex vertexMain(VertexInput vertexInput [[stage_in]]) {
    VertexOutput o;
    o.position = float4(vertexInput.position, 1.0);
    o.color = half3(vertexInput.color);
    return o;
}

half4 fragment fragmentMain(VertexOutput in [[stage_in]]) {
    return half4(in.color, 1.0);
}
