//
//  Renderer.cpp
//  MetalBones
//
//  Created by Sasha on 01/08/2024.
//

#include "Renderer.hpp"

Renderer::Renderer(MTL::Device* device) 
    : device(device->retain()) {
    commandQueue = device->newCommandQueue();
}

Renderer::~Renderer() {
    commandQueue->release();
    device->release();
}

void Renderer::draw(MTK::View* view) {
    NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
    
    MTL::CommandBuffer* commandBuffer = commandQueue->commandBuffer();
    MTL::RenderPassDescriptor* renderPass = view->currentRenderPassDescriptor();
    MTL::RenderCommandEncoder* encoder = commandBuffer->renderCommandEncoder(renderPass);
    
    encoder->endEncoding();
    commandBuffer->presentDrawable(view->currentDrawable());
    commandBuffer->commit();
    
    pool->release();
}
