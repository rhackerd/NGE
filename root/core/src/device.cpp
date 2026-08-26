#include "device.h"
#include <cglm/vec2.h>
#include <vulkan/vulkan_to_string.hpp>

namespace Nova::GE {

    uint16_t GPUIndex = 0;
    bool Device::init(CreateInfo::Device& createInfo) {
        if (!createInfo.mPhysicalDevice) {
            printf("device init(): invalid physical device\n");
            return false;
        }

        mPhysicalDevice = createInfo.mPhysicalDevice;
        system = createInfo.system;

        this->devId = GPUIndex;
        printGPUInfo(createInfo.extensions);
        printf("device(%i) init(): initiating device\n", devId);

        // ── Features chain ────────────────────────────────────────────
        vk::PhysicalDeviceDynamicRenderingFeatures dynamicRendering{};
        dynamicRendering.setDynamicRendering(true);

        vk::PhysicalDeviceSynchronization2Features sync2{};
        sync2.setSynchronization2(true);
        sync2.setPNext(&dynamicRendering);

        vk::PhysicalDeviceBufferDeviceAddressFeatures bda{};
        bda.setBufferDeviceAddress(true);
        bda.setPNext(&sync2);

        vk::PhysicalDeviceDescriptorBufferFeaturesEXT descBuffer{};
        descBuffer.setDescriptorBuffer(true);
        descBuffer.setPNext(&bda);

        vk::PhysicalDeviceFeatures2 deviceFeatures{};
        deviceFeatures.setPNext(&descBuffer); // head of chain

        // ── Queues ────────────────────────────────────────────────────
        // NOVA_INFO(*log, "Preparing for device creation");
        printf("device(%i) init(): preparing for device creation\n", devId);
        FillFamilyIndices(createInfo.surface);
        // NOVA_INFO(*log, "Filled Family Indices");
        printf("device(%i) init(): filled family indices\n", devId);

        // ── Device ───────────────────────────────────────────────────
        auto queueInfos = getQueueCreateInfos();
        vk::DeviceCreateInfo dcreateInfo{};
        dcreateInfo
            .setQueueCreateInfos(queueInfos)
            .setPEnabledFeatures(nullptr)  // must be null when using Features2
            .setPNext(&deviceFeatures)
            .setPEnabledExtensionNames(createInfo.extensions);

        // NOVA_INFO(*log, "Making device create info");
        printf("device(%i) init(): making device create info\n", devId);
        mLogicalDevice = mPhysicalDevice.createDevice(dcreateInfo, nullptr, system->getDld());
        if (!mLogicalDevice) {
            // NOVA_INFO(*log, "Failed to create device");
            printf("device(%i) init(): failed to create device\n", devId);
            return false;
        }
        // NOVA_INFO(*log, "Created device");
        printf("device(%i) init(): device created\n", devId);

        dld.init(system->getInstance(), vkGetInstanceProcAddr);
        dld.init(mLogicalDevice);
        setupQueues();
        GPUIndex++;

        // ── Allocator ─────────────────────────────────────────────────
        VmaAllocatorCreateInfo allocCI{};
        allocCI.vulkanApiVersion = VK_API_VERSION_1_4;
        allocCI.physicalDevice   = mPhysicalDevice;
        allocCI.device           = mLogicalDevice;
        allocCI.instance         = system->getInstance();
        allocCI.flags           |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

        if (vmaCreateAllocator(&allocCI, &mAllocator) != VK_SUCCESS) {
            // NOVA_INFO(*log, "Failed to create allocator");
            printf("device(%i) init(): failed to create allocator\n", devId);
            return false;
        }

        // ── Command pool ──────────────────────────────────────────────
        //NOVA_INFO(*log, "Making command pool.");
        printf("device(%i) init(): making command pool\n", devId);
        vk::CommandPoolCreateInfo poolCI{};
        poolCI.setQueueFamilyIndex(indices.graphicsFamily.value())
            .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer); // ← fix
        commandPool = mLogicalDevice.createCommandPool(poolCI);
        // NOVA_INFO(*log, "Created command pool.");
        printf("device(%i) init(): crated command pool\n", devId);

        // ── Descriptor sizing ─────────────────────────────────────────
        // NOVA_INFO(*log, "Querying compatibility");
        printf("device(%i) init(): querying compatibility\n", devId);
        vk::PhysicalDeviceDescriptorBufferPropertiesEXT descProps{};
        vk::PhysicalDeviceProperties2 props2{};
        props2.pNext = &descProps;
        mPhysicalDevice.getProperties2(&props2);

        uboDescsSize   = descProps.uniformBufferDescriptorSize;
        samplerDescSize = descProps.samplerDescriptorSize;
        ssboDescSize   = descProps.storageBufferDescriptorSize;
        imageDescSize = descProps.sampledImageDescriptorSize;
        // NOVA_INFO(*log, "Done querying compatibility");
        printf("device(%i) init(): done querying compatibility\n", devId);

        // ── Descriptor manager ────────────────────────────────────────
        // NOVA_INFO(*log, "Making descriptor manager");
        printf("device(%i) init(): making descriptor manager\n", devId);
        descriptorManager.init(mAllocator, {mLogicalDevice, getDld()}, descProps);
        // NOVA_INFO(*log, "Created descriptor manager");
        printf("device(%i) init(): created descriptor manager\n", devId);

        return true;
    }

    void Device::shutdown() {
        if (!mLogicalDevice) return;
        // NOVA_INFO(*log, "Shutting down Device");
        printf("device(%i) shutdown(): shutting down device\n", devId);

        for (auto& image : images) {
            image->shutdown();
        }
        images.clear();

        for (auto& shader : shaders) {
            shader->shutdown();
        }
        shaders.clear();

        // Command Buffers
        for (auto& commandBuffer : commandBuffers) {
            commandBuffer->shutdown();
        }
        commandBuffers.clear();

        // Pipelines
        for (auto& pl : pipelines) {
            pl->shutdown();
        }
        pipelines.clear();

        // UBOs
        for (auto& ubo : ubos) {
            ubo->shutdown();
        }
        ubos.clear();

        // DescMan
        descriptorManager.shutdown(mAllocator);

        // Command pool
        if (commandPool != VK_NULL_HANDLE) {
            mLogicalDevice.destroyCommandPool(commandPool);
            commandPool = VK_NULL_HANDLE;
        }

        // Destroy allocator
        vmaDestroyAllocator(mAllocator);
        mAllocator = nullptr;

        mLogicalDevice.destroy();
        mLogicalDevice = nullptr;
        // NOVA_INFO(*log, "Shut down Device");
        printf("device(%i) shutdown(): device destroyed\n", devId);
    }





    ImagePtr Device::createImage(CreateInfo::Image& createInfo) {
        createInfo.device = this->mLogicalDevice;
        createInfo.allocator = &this->mAllocator;

        auto image = std::make_shared<Image>();  // Create shared_ptr directly
        
        if (!image->init(createInfo)) {
        printf("device(%i) createImage(): Failed to create image\n", devId);
            return nullptr;
        }
        
        printf("device(%i) createImage(): Created image\n", devId);
        images.push_back(image);  // Store the shared_ptr
        return image;
    }

    void Device::removeImage(ImagePtr image) {
    printf("device(%i) removeImage(): Removing image\n", devId);
        image->shutdown();
        images.erase(std::remove(images.begin(), images.end(), image), images.end());
    }

    std::vector<ref<CommandBuffer>> Device::createCommandBuffers(CreateInfo::CommandBuffer& createInfo) {
        createInfo.device = mLogicalDevice;

        vk::CommandBufferAllocateInfo allocInfo{};
        allocInfo.setCommandBufferCount(static_cast<uint32_t>(createInfo.CommandBufferCount));
        allocInfo.setCommandPool(commandPool);
        allocInfo.setLevel(createInfo.secondary 
            ? vk::CommandBufferLevel::eSecondary 
            : vk::CommandBufferLevel::ePrimary);

        auto allocated = mLogicalDevice.allocateCommandBuffers(allocInfo);

        // NOVA_INFO(*log, "Allocating {} {} command buffers", 
        //    createInfo.CommandBufferCount, 
        //     createInfo.secondary ? "secondary" : "primary");

        std::vector<ref<CommandBuffer>> result;
        result.reserve(createInfo.CommandBufferCount);

        for (auto& cb : allocated) {
            auto commandBuffer = std::make_shared<CommandBuffer>();
            if (!commandBuffer->init(createInfo)) {
            printf("device(%i) createCommandBuffers(): Failed to init command buffer\n", devId);
                return {};  // empty vector on failure
            }
            commandBuffer->set(cb);
            commandBuffers.push_back(commandBuffer);
            result.push_back(commandBuffer);
        }

        printf("device(%i) createCommandBuffers(): Done allocating command buffers\n", devId);
        return result;
    }

    weakRef<Shader> Device::createShader(const std::string& path, vk::ShaderStageFlagBits stage) {
        // NOVA_INFO(*log, "Making {} shader from {}", vk::to_string(stage), path);
        printf("device(%i) createShader(): making %s shader from %s\n",devId, vk::to_string(stage).c_str(), path.c_str());
        auto shader = makeRef<Shader>(path, stage, &mLogicalDevice);

        // NOVA_INFO(*log, "Created {} shader from {}", vk::to_string(stage), path);
        printf("device(%i) createShader(): done\n");
        shaders.push_back(shader);
        return shader;
    }

    weakRef<Pipeline> Device::createPipeline(CreateInfo::Pipeline& createInfo) {
        printf("device(%i) createPipeline(): Making pipeline\n", devId);
        auto pipeline = makeRef<Pipeline>(mLogicalDevice, getDld(), createInfo);
        printf("device(%i) createPipeline(): Made a pipeline \n", devId);
        pipelines.push_back(pipeline);
        return pipeline;
    }

    // weakRef<Texture> Device::createTexture(const std::string& path) {
    //     NOVA_INFO(*log, "Making texture from {}", path);
    //     auto texture = makeRef<Texture>();

    //     auto cbci = CreateInfo::CommandBuffer{};
    //     cbci.CommandBufferCount = 1;
    //     cbci.secondary = false;

    //     auto cb = this->createCommandBuffers(cbci).front();

    //     if (!texture->init(path, mAllocator, mLogicalDevice, cb->getCommandBuffer(), getGraphicsQueue())) {
    //         cb->shutdown();
    //         NOVA_INFO(*log, "Failed to make texture from {}", path);
    //         return {};
    //     }
    //     mLogicalDevice.waitIdle();
    //     cb->shutdown();
        
    //     textures.push_back(texture);
    //     NOVA_INFO(*log, "Made a texture from {}", path);
    //     return texture;
    // }

    void Device::uploadToImage(ref<Image> image, void* pixels) {
        vec2 out;
        image->getExtent(out);
        vk::DeviceSize size = out[0] * out[1] * getFormatSize(image->getFormat());

        auto stagingCI = CreateInfo::Buffer::Builder()
            .setSize(size)
            .asStaging()
            .setAllocator(mAllocator)
            .build();

        Buffer staging;
        staging.init(stagingCI);
        staging.upload(pixels, size);

        auto cbci = CreateInfo::CommandBuffer::Builder()
            .setCommandBufferCount(1)
            .setPrimary()
            .build();
        auto _cb = this->createCommandBuffers(cbci).front();

        vk::CommandBuffer cmd = _cb->getCommandBuffer();

        cmd.begin(vk::CommandBufferBeginInfo{}
            .setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

        // Undefined → TransferDst
        vk::ImageMemoryBarrier2 toTransfer{};
        toTransfer
            .setOldLayout(vk::ImageLayout::eUndefined)
            .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
            .setImage(image->getImage())
            .setSrcStageMask(vk::PipelineStageFlagBits2::eTopOfPipe)
            .setSrcAccessMask(vk::AccessFlagBits2::eNone)
            .setDstStageMask(vk::PipelineStageFlagBits2::eTransfer)
            .setDstAccessMask(vk::AccessFlagBits2::eTransferWrite);
        toTransfer.subresourceRange
            .setAspectMask(vk::ImageAspectFlagBits::eColor)
            .setLevelCount(1).setLayerCount(1);
        cmd.pipelineBarrier2(vk::DependencyInfo{}.setImageMemoryBarriers(toTransfer));

        // Copy
        image->getExtent(out);

        vk::BufferImageCopy region{};
        region.imageSubresource
            .setAspectMask(vk::ImageAspectFlagBits::eColor)
            .setMipLevel(0).setBaseArrayLayer(0).setLayerCount(1);
        region
            .setImageOffset({0,0,0})
            .setImageExtent({static_cast<uint32_t>(out[0]), static_cast<uint32_t>(out[1]), 1});

        cmd.copyBufferToImage(staging.getBuffer(), image->getImage(),
            vk::ImageLayout::eTransferDstOptimal, region);

        // TransferDst → ShaderReadOnly
        vk::ImageMemoryBarrier2 toShader{};
        toShader
            .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
            .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
            .setImage(image->getImage())
            .setSrcStageMask(vk::PipelineStageFlagBits2::eTransfer)
            .setSrcAccessMask(vk::AccessFlagBits2::eTransferWrite)
            .setDstStageMask(vk::PipelineStageFlagBits2::eFragmentShader)
            .setDstAccessMask(vk::AccessFlagBits2::eShaderRead);
        toShader.subresourceRange
            .setAspectMask(vk::ImageAspectFlagBits::eColor)
            .setLevelCount(1).setLayerCount(1);
        cmd.pipelineBarrier2(vk::DependencyInfo{}.setImageMemoryBarriers(toShader));

        cmd.end();

        vk::FenceCreateInfo fenceInfo{};
        auto fence = mLogicalDevice.createFence(fenceInfo);
        
        vk::SubmitInfo submitInfo{};
        submitInfo.setCommandBuffers(cmd);
        getGraphicsQueue().submit(submitInfo, fence);
        mLogicalDevice.waitForFences(fence, true, UINT64_MAX);
        mLogicalDevice.destroyFence(fence);
        staging.shutdown();
        fence = VK_NULL_HANDLE;
    };

};
