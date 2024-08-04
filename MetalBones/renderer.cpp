//
//  Renderer.cpp
//  MetalBones
//
//  Created by Sasha on 01/08/2024.
//

#include "renderer.hpp"
#include <fstream>
#include <sstream>

Renderer::Renderer(MTL::Device* device) 
    : device(device->retain())
{
    commandQueue = device->newCommandQueue();
    buildShaders();
}

Renderer::~Renderer() {
    commandQueue->release();
    device->release();
}

void Renderer::buildShaders() {
    using NS::StringEncoding::UTF8StringEncoding;
    
    std::ifstream file;
    file.open("shaders/triangle.metal");
    std::stringstream reader;
    reader << file.rdbuf();
    std::string rawString = reader.str();
    NS::String* shaderSrc = NS::String::string(rawString.c_str(), UTF8StringEncoding);
    
    NS::Error* error = nullptr;
    MTL::CompileOptions* options = nullptr;
    MTL::Library* library = device->newLibrary(shaderSrc, options, &error);
    if (!library) {
        __builtin_printf("%s", error->localizedDescription()->utf8String());
        assert(false);
    }
    
    MTL::Function* vertexFn = library->newFunction(NS::String::string("vertexMain", UTF8StringEncoding));
    MTL::Function* fragmentFn = library->newFunction(NS::String::string("fragmentMain", UTF8StringEncoding));
    
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
    library->release();
}

void Renderer::draw(MTK::View* view) {
    NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
    
    MTL::CommandBuffer* commandBuffer = commandQueue->commandBuffer();
    MTL::RenderPassDescriptor* renderPass = view->currentRenderPassDescriptor();
    MTL::RenderCommandEncoder* encoder = commandBuffer->renderCommandEncoder(renderPass);
    
    encoder->setRenderPipelineState(renderPipelineState);
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
