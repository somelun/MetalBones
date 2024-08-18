//
//  Renderer.cpp
//  MetalBones
//
//  Created by Sasha on 01/08/2024.
//

#include "Renderer.hpp"

#include <simd/simd.h>

Renderer::Renderer(MTL::Device* device) 
    : device(device->retain())
{
    commandQueue = device->newCommandQueue();
    buildShaders();
    buildDepthStencilStates();
    buildBuffers();
}

Renderer::~Renderer() {
    indexBuffer->release();
    vertexBuffer->release();
    depthStencilState->release();
    renderPipelineState->release();
    shaderLibrary->release();
    commandQueue->release();
    device->release();
}

void Renderer::buildShaders() {
    using NS::StringEncoding::UTF8StringEncoding;
    
    NS::Error* error = nullptr;
    shaderLibrary = device->newDefaultLibrary();
    if (!shaderLibrary) {
        __builtin_printf("%s", error->localizedDescription()->utf8String());
        assert(false);
    }
    
    MTL::Function* vertexFn = shaderLibrary->newFunction(NS::String::string("vertexMain", UTF8StringEncoding));
    MTL::Function* fragmentFn = shaderLibrary->newFunction(NS::String::string("fragmentMain", UTF8StringEncoding));
    
    MTL::RenderPipelineDescriptor* pipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    pipelineDescriptor->setVertexFunction(vertexFn);
    pipelineDescriptor->setFragmentFunction(fragmentFn);
    pipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB);
//    pipelineDescriptor->setDepthAttachmentPixelFormat(MTL::PixelFormat::PixelFormatDepth16Unorm);
    
    MTL::VertexDescriptor* vertexDescriptor = MTL::VertexDescriptor::alloc()->init();
    MTL::VertexAttributeDescriptorArray* attributes = vertexDescriptor->attributes();
    
    MTL::VertexAttributeDescriptor* positionDescriptor = attributes->object(0);
    positionDescriptor->setFormat(MTL::VertexFormat::VertexFormatFloat3);
    positionDescriptor->setOffset(0);
    positionDescriptor->setBufferIndex(0);
    
    MTL::VertexAttributeDescriptor* colorDescriptor = attributes->object(1);
    colorDescriptor->setFormat(MTL::VertexFormat::VertexFormatFloat3);
    colorDescriptor->setOffset(4 * sizeof(float));
    colorDescriptor->setBufferIndex(0);
    
    MTL::VertexBufferLayoutDescriptor* layoutDescriptor = vertexDescriptor->layouts()->object(0);
    layoutDescriptor->setStride(8 * sizeof(float));
    
    pipelineDescriptor->setVertexDescriptor(vertexDescriptor);
    
    renderPipelineState = device->newRenderPipelineState(pipelineDescriptor, &error);
    if (!renderPipelineState) {
        __builtin_printf("%s", error->localizedDescription()->utf8String());
        assert(false);
    }
    
    fragmentFn->release();
    vertexFn->release();
    pipelineDescriptor->release();
}

void Renderer::buildDepthStencilStates() {
    MTL::DepthStencilDescriptor* depthStencilDescriptor = MTL::DepthStencilDescriptor::alloc()->init();
    depthStencilDescriptor->setDepthCompareFunction(MTL::CompareFunction::CompareFunctionLess);
    depthStencilDescriptor->setDepthWriteEnabled(true);

    depthStencilState = device->newDepthStencilState(depthStencilDescriptor);

    depthStencilDescriptor->release();
}

void Renderer::buildBuffers() {
    constexpr size_t numVertices = 24;
    
    struct Vertex {
        simd::float3 position;
        simd::float3 color;
    };
    
    const float s = 0.5f;
    const simd::float3 Red      = {1.0f, 0.0f, 0.0f};
    const simd::float3 Green    = {0.0f, 1.0f, 0.0f};
    const simd::float3 Blue     = {0.0f, 0.0f, 1.0f};
    const simd::float3 Orange   = {1.0f, 0.4f, 0.0f};
    const simd::float3 Purple   = {1.0f, 0.0f, 1.0f};
    const simd::float3 Cyan     = {0.0f, 1.0f, 1.0f};
    
    Vertex vertices[numVertices] = {
        {{ -s, -s, +s }, Red},
        {{ +s, -s, +s }, Red},
        {{ +s, +s, +s }, Red},
        {{ -s, +s, +s }, Red},

        {{ +s, -s, +s }, Green},
        {{ +s, -s, -s }, Green},
        {{ +s, +s, -s }, Green},
        {{ +s, +s, +s }, Green},

        {{ +s, -s, -s }, Blue},
        {{ -s, -s, -s }, Blue},
        {{ -s, +s, -s }, Blue},
        {{ +s, +s, -s }, Blue},

        {{ -s, -s, -s }, Orange},
        {{ -s, -s, +s }, Orange},
        {{ -s, +s, +s }, Orange},
        {{ -s, +s, -s }, Orange},

        {{ -s, +s, +s }, Purple},
        {{ +s, +s, +s }, Purple},
        {{ +s, +s, -s }, Purple},
        {{ -s, +s, -s }, Purple},

        {{ -s, -s, -s }, Cyan},
        {{ +s, -s, -s }, Cyan},
        {{ +s, -s, +s }, Cyan},
        {{ -s, -s, +s }, Cyan},
    };
    
    uint16_t indices[] = {
        0,  1,  2,  2,  3,  0, /* front */
        4,  5,  6,  6,  7,  4, /* right */
        8,  9, 10, 10, 11,  8, /* back */
       12, 13, 14, 14, 15, 12, /* left */
       16, 17, 18, 18, 19, 16, /* top */
       20, 21, 22, 22, 23, 20, /* bottom */
    };
    
    vertexBuffer = device->newBuffer(numVertices * sizeof(Vertex), MTL::ResourceStorageModeManaged);
    indexBuffer = device->newBuffer(sizeof(indices), MTL::ResourceStorageModeManaged);

    memcpy(vertexBuffer->contents(), vertices, numVertices * sizeof(Vertex));
    memcpy(indexBuffer->contents(), indices, sizeof(indices));

    vertexBuffer->didModifyRange(NS::Range::Make(0, vertexBuffer->length()));
    indexBuffer->didModifyRange(NS::Range::Make(0, indexBuffer->length()));
}

void Renderer::draw(MTK::View* view) {
    NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
    
    MTL::CommandBuffer* commandBuffer = commandQueue->commandBuffer();
    MTL::RenderPassDescriptor* renderPassDescriptor = view->currentRenderPassDescriptor();
    MTL::RenderCommandEncoder* encoder = commandBuffer->renderCommandEncoder(renderPassDescriptor);
    
    encoder->setRenderPipelineState(renderPipelineState);
    encoder->setVertexBuffer(vertexBuffer, 0, 0);
    
    encoder->drawIndexedPrimitives(
        MTL::PrimitiveType::PrimitiveTypeTriangle,
        6, MTL::IndexType::IndexTypeUInt16,
        indexBuffer,
        0, 1
    );
    
    encoder->endEncoding();
    
    commandBuffer->presentDrawable(view->currentDrawable());
    commandBuffer->commit();
    
    pool->release();
}
