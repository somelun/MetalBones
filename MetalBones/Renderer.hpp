//
//  Renderer.hpp
//  MetalBones
//
//  Created by Sasha on 01/08/2024.
//

#pragma once

#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>

class Renderer {
public:
    Renderer(MTL::Device* device);
    ~Renderer();
    
    void buildShaders();
    void buildBuffers();
    
    void draw(MTK::View* view);
    
private:
    MTL::Device* device;
    MTL::CommandQueue* commandQueue;
    MTL::Library* shaderLibrary;
    MTL::RenderPipelineState* renderPipelineState;    
    
    MTL::Buffer* vertexBuffer;
    MTL::Buffer* indexBuffer;
};
