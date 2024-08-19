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

VertexOutput vertex vertexMain(VertexInput vertexInput [[stage_in]], const constant float &t [[buffer(3)]]) {
    float4x4 rotX = float4x4(1.0f);
    rotX[1][1] = cos(t);
    rotX[1][2] = sin(t);
    rotX[2][1] = -sin(t);
    rotX[2][2] = cos(t);
    
    float4x4 rotY = float4x4(1.0f);
    rotY[0][0] = cos(t);
    rotY[0][2] = -sin(t);
    rotY[2][0] = sin(t);
    rotY[2][2] = cos(t);
    
    float4x4 normZ = float4x4(1.0f);
    const float zNear = 0.01f;
    const float zFar = 100.0f;
    normZ[2][2] = (1 - zNear) / zFar;
    normZ[3][2] = zNear;
    
    VertexOutput o;
    o.position = normZ * rotX * rotY * float4(vertexInput.position, 1.0);
    o.color = half3(vertexInput.color);
    return o;
}

half4 fragment fragmentMain(VertexOutput in [[stage_in]]) {
    return half4(in.color, 1.0);
}
