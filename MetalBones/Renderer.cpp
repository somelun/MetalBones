//
//  Renderer.cpp
//  MetalBones
//
//  Created by Sasha on 01/08/2024.
//

#include "Renderer.hpp"

#include <fstream>
#include <sstream>
#include <simd/simd.h>

Renderer::Renderer(MTL::Device* device) 
    : device(device->retain())
{
    commandQueue = device->newCommandQueue();
    buildShaders();
    buildBuffers();
}

Renderer::~Renderer() {
    argBuffer->release();
    vertexPositionsBuffer->release();
    vertexColorsBuffer->release();
    renderPipelineState->release();
    shaderLibrary->release();
    commandQueue->release();
    device->release();
}

void Renderer::buildShaders() {
    using NS::StringEncoding::UTF8StringEncoding;
    
    std::ifstream file;
    file.open("shaders/general.metal");
    std::stringstream reader;
    reader << file.rdbuf();
    std::string rawString = reader.str();
    NS::String* shaderSrc = NS::String::string(rawString.c_str(), UTF8StringEncoding);
    
    NS::Error* error = nullptr;
    MTL::CompileOptions* options = nullptr;
    shaderLibrary = device->newLibrary(shaderSrc, options, &error);
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
    constexpr size_t numVertices = 3;

    simd::float3 positions[numVertices] = {
        {-0.8f,  0.8f, 0.0f},
        { 0.0f, -0.8f, 0.0f},
        { 0.8f,  0.8f, 0.0f}
    };

    simd::float3 colors[numVertices] = {
        {1.0f, 0.3f, 0.2f},
        {0.8f, 1.0f, 0.0f},
        {0.8f, 0.0f, 1.0f}
    };
    
    const size_t positionsDataSize = numVertices * sizeof(simd::float3);
    const size_t colorDataSize = numVertices * sizeof(simd::float3);

    vertexPositionsBuffer = device->newBuffer(positionsDataSize, MTL::ResourceStorageModeManaged);
    vertexColorsBuffer = device->newBuffer(colorDataSize, MTL::ResourceStorageModeManaged);

    memcpy(vertexPositionsBuffer->contents(), positions, positionsDataSize);
    memcpy(vertexColorsBuffer->contents(), colors, colorDataSize);

    vertexPositionsBuffer->didModifyRange(NS::Range::Make(0, vertexPositionsBuffer->length()));
    vertexColorsBuffer->didModifyRange(NS::Range::Make(0, vertexColorsBuffer->length()));

    using NS::StringEncoding::UTF8StringEncoding;
    assert(shaderLibrary);

    MTL::Function* vertexFn = shaderLibrary->newFunction(NS::String::string("vertexMain", UTF8StringEncoding));
    MTL::ArgumentEncoder* argEncoder = vertexFn->newArgumentEncoder(0);

    argBuffer = device->newBuffer(argEncoder->encodedLength(), MTL::ResourceStorageModeManaged);

    argEncoder->setArgumentBuffer(argBuffer, 0);

    argEncoder->setBuffer(vertexPositionsBuffer, 0, 0);
    argEncoder->setBuffer(vertexColorsBuffer, 0, 1);

    argBuffer->didModifyRange(NS::Range::Make(0, argBuffer->length()));

    vertexFn->release();
    argEncoder->release();
}

void Renderer::draw(MTK::View* view) {
    NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
    
    MTL::CommandBuffer* commandBuffer = commandQueue->commandBuffer();
    MTL::RenderPassDescriptor* renderPass = view->currentRenderPassDescriptor();
    MTL::RenderCommandEncoder* encoder = commandBuffer->renderCommandEncoder(renderPass);
    
    encoder->setRenderPipelineState(renderPipelineState);
    encoder->setVertexBuffer(argBuffer, 0, 0);
    encoder->useResource(vertexPositionsBuffer, MTL::ResourceUsageRead);
    encoder->useResource(vertexColorsBuffer, MTL::ResourceUsageRead);
    encoder->drawPrimitives(
        MTL::PrimitiveType::PrimitiveTypeTriangle,
        NS::UInteger(0),
        NS::UInteger(3)
    );
    
    encoder->endEncoding();
    commandBuffer->presentDrawable(view->currentDrawable());
    commandBuffer->commit();
    
    pool->release();
}
