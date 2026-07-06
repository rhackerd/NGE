# Nova Graphics Engine

> [!Note]
> Formerly known as Nova Graphics Engine 2

This is a low level wrapper around Vulkan, as well as a high level library around the low level wrapper.

## Key Features
* **Lightweight:** The project should be clear and lightweight, so no raytracing nor pathtracing this should be raw rasterization renderer, thus being lightweight. More [here](#lightweight)
* **Create Info's:** `Builder`'s and CreateInfo are used for the creation of resources which can be edited raw as a struct by also has a Builder which uses normal functions to construct the struct. More [here](#createinfos)
* **Pipeline Layout:** Without the use of traditional JSON files to define the pipeline layout I want to use straight out C++, so I built a clean bake system for pipeline layouts. More [here](#pipeline-creation)

## Who is this library for ?
### Yes
* **Game Developers:** game developers can use this library as raylib, almost no dependency (except for VMA) but modern and no OpenGL
* **Inspiration:** This project can be used for inspiration and to learn vulkan.
* **Engine Development:** This project is not a direct competitor to big libraries like bgfx or wgpu, yet, but because it is lightweight can be used for indie engines.
### No
* **AAA grade engines:** This project is not AAA grade and probably will not be in the future, it shall be only indie graphics engine which can be used for people to for example prototypes systems for their games.

## Examples
### CreateInfo's
```cpp
    auto sCI = Nova::GE::CreateInfo::System::Builder()
        .setAppName("ExampleApp")
        .setAppVersion(VK_MAKE_VERSION(1, 0, 0))
        .enableValidation()
        .addRecommendedValidationFeatures()
        .addExtensions(man.getExtensions())
        .build();
    if (!m_system.init(sCI)) return false;
```
*Code from [graphics.cpp](./root/core/src/graphics.cpp)*

### Pipeline creation
```cpp
auto vertex = m_device->createShader("shaders/vert.spv", vk::ShaderStageFlagBits::eVertex);
auto frag   = m_device->createShader("shaders/frag.spv", vk::ShaderStageFlagBits::eFragment);

// First layout
Nova::GE::ShaderSet cameraSet;
cameraSet.addUniformBuffer(0, vk::ShaderStageFlagBits::eVertex);

Nova::GE::ShaderSet texSet;
texSet.addSampler(0, vk::ShaderStageFlagBits::eFragment)
    .addSampledImage(1, vk::ShaderStageFlagBits::eFragment);

auto cameraSetLayout = cameraSet.bake(m_device->getDevice(), m_device->getDld(), 0);
auto texSetLayout    = texSet.bake(m_device->getDevice(), m_device->getDld(), 1);

Nova::GE::ShaderLayout shaderLayout;
shaderLayout.addPushConstant(sizeof(Nova::Core::Mat4), vk::ShaderStageFlagBits::eVertex);
auto bakedLayout = shaderLayout.bake({cameraSetLayout, texSetLayout});

// Then pipeline
Nova::GE::CreateInfo::Pipeline pipelineCI = Nova::GE::CreateInfo::Pipeline::Builder()
    .prepareDefault()
    .addShader(vertex)
    .addShader(frag)
    .setLayout(bakedLayout)
    .setSwapchainFormat(graphics.getSwapchain().getFormat())
    .setDepthFormat(graphics.getSwapchain().getDepthFormat())
    .setMSAA(graphics.getSwapchain().getSampleCount())
    .build();
m_pipeline = m_device->createPipeline(pipelineCI);
m_pipelineLayout = m_pipeline.lock()->getLayout();
```
*Code from [renderer.cpp](./root/core/src/renderer.cpp)*

### Rendering
> [!IMPORTANT]
> Synchronization isn't implemented yet, thus needs to be managed manually. Example can be shown in the same function as this code how to manage that.

> [!NOTE]
> Commented code from original file is removed for readability.
```cpp
m_lastSlot = m_cpuf.batchSubmit(m_renderTarget,
    {
        [&](Nova::GE::Render::Cmd cmd) {
            cmd.setSwapchain(*m_swapchain);
            cmd.bindDescriptorBuffer();
            cmd.bindPipeline(m_pipeline.lock());

            auto camera = scene.camera.lock();
            cmd.bindSet(camera->getHandle());

            for (auto& objRef : scene.objects) {
                auto obj = objRef.lock();
                
                auto& texSet = obj->getTexSet();
                cmd.bindSet(texSet);
                cmd.pushConstant(obj->getTransform(), m_pipelineLayout);
                cmd.drawMesh(obj->getMesh());
            }
        }
    },{}
    );
```

*Code from [renderer.cpp](./root/core/src/renderer.cpp)*

#### Description
the last bracket (which should be a std::function) will get executed after the first function is executed. But can be left empty if not needed.


### Swapchain recreation
this is a special case because that needs to be handled manually through SDL_Event.
```cpp
bool running = true;
while (running) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT) {
            running = false;
        }else if(e.type == SDL_EVENT_WINDOW_RESIZED) {
            printf("Window was resized\n");
            auto swap = renderer.getSwapchain();
            swap->handleRecreation();   
            camera.lock()->setAspect(swap->getExtent().width / (float)swap->getExtent().height);
        }
    }

    // camera.lock()->move({0,0.001,0});

    renderer.step(scene);
};
```

*Code from [main.cpp](./root/app/src/main.cpp)*

## Roadmap
* **Multi-threading:** Command buffer recording can be multi threaded, but creation of objects isn't possible at the time while multi thread, because of thread-safety
* **Remove AI trash:** This old project was using AI, but my view of AI has changed, thus I will remove any parts of code written by AI. In the future only examples pieces of code will be written by AI only for testing or placeholders an will be in a branch on itself.
* **Modernizing legacy code:** This project was formerly me learning vulkan but turned out cleaner than I though, and by the legacy name is the second iteration so I knew a lot about vulkan. More details in [Tags](./FileFlags.md)



## Lightweight
To try to explain by what I mean by this, I don't mean as other project to attract more people this is actually a lightweight library like vulkan, everyone can use it but it's not AAA grade graphics engine. for example it lacks these things:
- no ECS
- no scene Graph
- no editor
- no physics
- no asset pipeline