//
//  main.cpp
//  MetalBones
//
//  Created by Sasha on 21/07/2024.
//

#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define MTK_PRIVATE_IMPLEMENTATION

#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>

#include "Renderer.hpp"

class MTKViewDelegate : public MTK::ViewDelegate {
public:
    MTKViewDelegate(MTL::Device* device);
    virtual ~MTKViewDelegate() override;
    virtual void drawInMTKView(MTK::View* view) override;
};

class AppDelegate : public NS::ApplicationDelegate {
public:
    ~AppDelegate();

    virtual void applicationWillFinishLaunching(NS::Notification* notification) override;
    virtual void applicationDidFinishLaunching(NS::Notification* notification) override;
    virtual bool applicationShouldTerminateAfterLastWindowClosed(NS::Application* sender) override;

private:
    NS::Window* window;
    MTK::View* metalKitView;
    MTL::Device* device;
    MTKViewDelegate* viewDelegate = nullptr;
};

int main(int argc, const char* argv[argc + 1]) {
    NS::AutoreleasePool* autoreleasePool = NS::AutoreleasePool::alloc()->init();

    AppDelegate appDelegate;

    NS::Application* sharedApplication = NS::Application::sharedApplication();
    sharedApplication->setDelegate(&appDelegate);
    sharedApplication->run();

    autoreleasePool->release();

    return 0;
}

AppDelegate::~AppDelegate() {
    metalKitView->release();
    window->release();
    device->release();
    delete viewDelegate;
}

void AppDelegate::applicationWillFinishLaunching(NS::Notification* notification) {
}

void AppDelegate::applicationDidFinishLaunching(NS::Notification* notification) {
    CGRect frame = (CGRect){ {100.0, 100.0}, {512.0, 512.0} };

    window = NS::Window::alloc()->init(
        frame,
        NS::WindowStyleMaskClosable|NS::WindowStyleMaskTitled,
        NS::BackingStoreBuffered,
        false
    );

    device = MTL::CreateSystemDefaultDevice();

    metalKitView = MTK::View::alloc()->init(frame, device);
    metalKitView->setColorPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB);
    metalKitView->setClearColor(MTL::ClearColor::Make(1.0, 0.0, 0.0, 1.0));

    viewDelegate = new MTKViewDelegate(device);
    metalKitView->setDelegate(viewDelegate);

    window->setContentView(metalKitView);
    window->setTitle(NS::String::string( "Metal Bones", NS::StringEncoding::UTF8StringEncoding));

    window->makeKeyAndOrderFront(nullptr);

    NS::Application* Application = reinterpret_cast<NS::Application*>(notification->object());
    Application->activateIgnoringOtherApps(true);
}

bool AppDelegate::applicationShouldTerminateAfterLastWindowClosed(NS::Application* sender) {
    return true;
}

MTKViewDelegate::MTKViewDelegate(MTL::Device* device)
    : MTK::ViewDelegate() {
}

MTKViewDelegate::~MTKViewDelegate() {
}

void MTKViewDelegate::drawInMTKView(MTK::View* view) {
}
