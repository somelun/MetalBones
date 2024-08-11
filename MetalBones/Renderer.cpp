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
    buildBuffers();
}

Renderer::~Renderer() {
    indexBuffer->release();
    vertexBuffer->release();
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

void Renderer::buildBuffers() {
    constexpr size_t numVertices = 4;
    
    struct Vertex {
        simd::float3 position;
        simd::float3 color;
    };
    
    Vertex vertices[numVertices] = {
        {{-0.8f, -0.8f, 0.0f}, {1.0f, 0.3f, 0.2f}},
        {{ 0.8f, -0.8f, 0.0f}, {0.8f, 1.0f, 0.0f}},
        {{ 0.8f,  0.8f, 0.0f}, {0.8f, 0.0f, 1.0f}},
        {{-0.8f,  0.8f, 0.0f}, {0.8f, 1.0f, 0.4f}},
    };
    
    uint16_t indicies[] = {
        0, 1, 2,
        2, 3, 0
    };
    
    vertexBuffer = device->newBuffer(numVertices * sizeof(Vertex), MTL::ResourceStorageModeManaged);
    indexBuffer = device->newBuffer(sizeof(indicies), MTL::ResourceStorageModeManaged);

    memcpy(vertexBuffer->contents(), vertices, numVertices * sizeof(Vertex));
    memcpy(indexBuffer->contents(), indicies, sizeof(indicies));

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
