//
//  main.cpp
//  MetalBones
//
//  Created by Sasha on 21/07/2024.
//

#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define MTK_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATIO

#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>

#include "Renderer.hpp"

class MTKViewDelegate : public MTK::ViewDelegate {
public:
    MTKViewDelegate(MTL::Device* device);
    virtual ~MTKViewDelegate() override;

    // TODO: called 60 times per second internally, maybe switch to manual loop?
    virtual void drawInMTKView(MTK::View* view) override;

private:
    Renderer* renderer;
};

class AppDelegate : public NS::ApplicationDelegate {
public:
    ~AppDelegate();
    
    NS::Menu* createMenuBar();

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

NS::Menu* AppDelegate::createMenuBar() {
    using NS::StringEncoding::UTF8StringEncoding;
    
    NS::Menu* mainMenu = NS::Menu::alloc()->init();
    NS::MenuItem* appMenuItem = NS::MenuItem::alloc()->init();
    NS::Menu* appMenu = NS::Menu::alloc()->init(NS::String::string("Appname", UTF8StringEncoding));

    NS::String* appName = NS::RunningApplication::currentApplication()->localizedName();
    NS::String* quitItemName = NS::String::string("Quit ", UTF8StringEncoding )->stringByAppendingString(appName);
    SEL quitCallback = NS::MenuItem::registerActionCallback( "appQuit", [](void*, SEL, const NS::Object* sender) {
        auto application = NS::Application::sharedApplication();
        application->terminate(sender);
    });
    
    NS::MenuItem* appQuitItem = appMenu->addItem(quitItemName, quitCallback, NS::String::string("q", UTF8StringEncoding));
    appQuitItem->setKeyEquivalentModifierMask(NS::EventModifierFlagCommand);
    appMenuItem->setSubmenu(appMenu);
    
    mainMenu->addItem(appMenuItem);
    
    appMenuItem->release();
    appMenu->release();
    
    return mainMenu->autorelease();
}

void AppDelegate::applicationWillFinishLaunching(NS::Notification* notification) {
    NS::Menu* menu = createMenuBar();
    NS::Application* application = reinterpret_cast<NS::Application*>(notification->object());
    application->setMainMenu(menu);
    application->setActivationPolicy(NS::ActivationPolicy::ActivationPolicyRegular);
}

void AppDelegate::applicationDidFinishLaunching(NS::Notification* notification) {
    CGRect frame = (CGRect){
        {400.0, 250.0},
        {512.0, 512.0}
    };

    window = NS::Window::alloc()->init(
        frame,
        NS::WindowStyleMaskClosable|NS::WindowStyleMaskTitled,
        NS::BackingStoreBuffered,
        false
    );

    device = MTL::CreateSystemDefaultDevice();

    metalKitView = MTK::View::alloc()->init(frame, device);
    metalKitView->setColorPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB);
    metalKitView->setClearColor(MTL::ClearColor::Make(1.0, 1.0, 0.6, 1.0));

    viewDelegate = new MTKViewDelegate(device);
    metalKitView->setDelegate(viewDelegate);

    window->setContentView(metalKitView);
    window->setTitle(NS::String::string("Metal Bones", NS::StringEncoding::UTF8StringEncoding));
    window->makeKeyAndOrderFront(nullptr);

    NS::Application* Application = reinterpret_cast<NS::Application*>(notification->object());
    Application->activateIgnoringOtherApps(true);
}

bool AppDelegate::applicationShouldTerminateAfterLastWindowClosed(NS::Application* sender) {
    return true;
}

MTKViewDelegate::MTKViewDelegate(MTL::Device* device)
    : MTK::ViewDelegate()
    , renderer(new Renderer(device))
{
}

MTKViewDelegate::~MTKViewDelegate() {
    delete renderer;
}

void MTKViewDelegate::drawInMTKView(MTK::View* view) {
    renderer->draw(view);
}
