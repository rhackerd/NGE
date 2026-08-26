#pragma once


#include "Image.h"
#include "buffer.h"
#include "commandBuffer.h"
#include "core.h"
#include "descMan.h"
#include "pipeline.h"
#include "system.h"
#include "uniformBuffer.h"
#include "vulkan/vulkan.hpp"
#include <fmt/format.h>
#include <optional>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include "shader.h"
<<<<<<< Updated upstream
#include <Nova/Core/macros.h>

// TODO: modernize
// ========================================
// Metadata
//
// State    : #legacy@nge3
// Origin   : @nge2
//
// Desc     : Overall per device manger
// Info     : This file only works, does not look good and is very bad, maybe full rewrite and a PR of itself.
// ========================================

namespace Nova::Core {
    [[noreturn]] inline void assertFail(const char* type, const char* msg, 
                                         std::source_location loc = std::source_location::current()) {
        // use your existing logger or just stderr
        fprintf(stderr, "[%s] %s — %s:%d\n", type, msg, loc.file_name(), loc.line());
        std::abort();
    }
}
=======
>>>>>>> Stashed changes

namespace Nova::GE {
    struct DeviceCaps {
        bool bindless;
        bool meshShading;
        bool rayTracing = false;
        bool multiview = false;
        bool descriptorBuffer = true;
    };


    namespace CreateInfo {
        struct Device {
            vk::PhysicalDevice mPhysicalDevice;
            DeviceCaps dCaps;
            Nova::GE::System* system = nullptr;
            std::vector<const char*> extensions = {};
            vk::SurfaceKHR surface = VK_NULL_HANDLE;
            bool presentQueue = true; // -> for example VR does not need presentQueue



            class Builder {
                private:
                    Device* info;

                public:
                    Builder() : info(new Device()) {}

                    ~Builder() {
                        if (info != nullptr) delete info;
                    }

                    Builder& setSystem(Nova::GE::System* system) {
                        info->system = system;
                        return *this;
                    }

                    Builder& pickPhysicalDevice(bool discreteRequired = false, bool integratedRequired = false) {
                        // For now choose first device
                        if (info->system == nullptr) {
                            throw std::runtime_error(fmt::format("System is not set in Device Builder! Please use .setSystem(*System) first! ({}:{})", __PRETTY_FUNCTION__, __LINE__));

                            return *this;
                        }
                        info->mPhysicalDevice  = info->system->getInstance().enumeratePhysicalDevices().front();
                        return *this;
                    };

                    Builder& setPhysicalDevice(vk::PhysicalDevice device) {
                        info->mPhysicalDevice = device;
                        return *this;
                    }

                    Builder& enableBindless() {
                        info->dCaps.bindless = true;
                        return *this;
                    }

                    Builder& enableMeshShading() {
                        info->dCaps.meshShading = true;
                        return *this;
                    }

                    Builder& enableRayTracing() {
                        info->dCaps.rayTracing = true;
                        return *this;
                    }

                    Builder& disablePresentQueue() {
                        info->presentQueue = false;
                        return *this;
                    }

                    Builder& setSurface(vk::SurfaceKHR surface) {
                        info->surface = surface;
                        return *this;
                    }

                    Builder& addExtension(const std::string& extension) {
                        info->extensions.push_back(extension.c_str());
                        return *this;
                    }

                    Builder& addExtensions(const std::vector<const char*>& extensions) {
                        info->extensions.insert(info->extensions.end(), extensions.begin(), extensions.end());
                        return *this;
                    }

                    Builder& enableMultiview() {
                        info->dCaps.multiview = true;
                        return *this;
                    }

                    Builder& enableDefaultExt() {
                        info->extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
                        return *this;
                    }

                    Builder& enableDescriptorBuffer() {
                        info->extensions.push_back(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
                        return *this;
                    }

                    Device build() {
                        Device result = *info;
                        delete info;
                        info = nullptr;
                        return result;
                    }
            };
        };
    };

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        std::optional<uint32_t> transferFamily;
        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
        }
    };

    extern uint16_t GPUIndex;

    static vk::DeviceSize getFormatSize(vk::Format format) {
        switch (format) {
            case vk::Format::eR8G8B8A8Srgb:
            case vk::Format::eR8G8B8A8Unorm:  return 4;
            case vk::Format::eR16G16B16A16Sfloat: return 8;
            case vk::Format::eR32G32B32A32Sfloat: return 16;
            case vk::Format::eR8Unorm:         return 1;
            case vk::Format::eR16Unorm:        return 2;
            // add as needed
            default:
                printf("device getFormatSize(): unknown format size");
                return 0;
        }
    }

    // TODO: God .h file, move to elsewhere later
    struct UploadHandle {
        vk::CommandBuffer cmd;
        vk::Fence fence;
        Buffer buffer;
    };

    // TODO: Update metadata
    /**
     * @class GfxEngine
     * @brief Main class for NGE Graphics Engine
     *
     * 
     * This class manages memory allocators, and other main graphics resources. 
     *
     * 
     * Usage:
     *    - Instantiate one GfxEngine per application instance
     *    - Create createInfo either using internal builder, or manually
     *    - Shutdown is managed manually by the Engine
     *
     * Responsibilities:
     *    - Memory allocators
     *    - Device creation
     *    - Device manager
     *    - Debugging utils
     *
     */
    class Device {
        public:
            Device(CreateInfo::Device& createInfo) {
                init(createInfo);
            }
            Device() {};
            ~Device() { shutdown(); };

        public:
            // Manual Control
            bool init(CreateInfo::Device& createInfo);
            void shutdown();

        public:
            // TODO: split device.h into factory.h and device.h
            // Object makers
            [[nodiscard]]
            ImagePtr createImage(CreateInfo::Image& createInfo);
            void removeImage(ImagePtr image);
            [[nodiscard]]
            std::vector<ref<CommandBuffer>> createCommandBuffers(CreateInfo::CommandBuffer& createInfo);
            [[nodiscard]]
            weakRef<Shader> createShader(const std::string& path, vk::ShaderStageFlagBits stage);
            [[nodiscard]]
            weakRef<Pipeline> createPipeline(CreateInfo::Pipeline& createInfo);
            template<typename T>
            [[nodiscard]]
            weakRef<UniformBuffer> createUniformBuffer(CreateInfo::UniformBuffer createInfo) {
                auto buffer = makeRef<UniformBuffer>(mLogicalDevice, createInfo, mAllocator);
                // NOVA_INFO(*log, "Made a uniform buffer");
                return buffer;
            }

            // TODO: also probably move to somewhere else for example Image could be candidate
            void uploadToImage(ref<Image>, void* pixels);
            // void signTexture(weakRef<Texture> texture, vk::DescriptorSetLayout setLayout, Camera& camera) {
            //     auto tex = texture.lock();
            //     if (!tex) return;

            //     usize base = descriptorManager.allocateSet(setLayout);

            //     // binding 0 = camera UBO
            //     descriptorManager.writeUBO(
            //         camera.getBuffer(),  // however you access camera buffer
            //         sizeof(CameraData),
            //         base + descriptorManager.getBindingOffset(setLayout, 0)
            //     );

            //     // binding 1 = sampler
            //     descriptorManager.writeSampler(
            //         tex->getSampler(),
            //         base + descriptorManager.getBindingOffset(setLayout, 1)
            //     );

            //     // binding 2 = image
            //     descriptorManager.writeSampledImage(
            //         tex->getImageView(),
            //         vk::ImageLayout::eShaderReadOnlyOptimal,
            //         base + descriptorManager.getBindingOffset(setLayout, 2)
            //     );

            //     tex->setBase(base);
            // }

        public:
            vk::Device getDevice() { return mLogicalDevice; };
            vk::PhysicalDevice getPhysicalDevice() { return mPhysicalDevice; };
            vk::Queue getGraphicsQueue() { return mGraphicsQueue; };
            vk::Queue getPresentQueue() { return mPresentQueue; };
            vk::Queue getTransferQueue() { return mTransferQueue; };
            vk::detail::DispatchLoaderDynamic& getDld() { return dld; };
            QueueFamilyIndices& getIndices() {return indices;};
            VmaAllocator& getAllocator() { return mAllocator; };
            usize getUBOSize() { return uboDescsSize; }
            usize getSSBOSize() { return ssboDescSize; }
            usize getSamplerSize() { return samplerDescSize; }
            usize getSampledImageSize() { return imageDescSize; }
            DescriptorMan& getDescriptorManager() { return descriptorManager; }
        private:
            // TODO: Should be init phases, can be mistaken for actual functions.
            void setupQueues();
            void FillFamilyIndices(std::optional<vk::SurfaceKHR> surface);
            std::vector<vk::DeviceQueueCreateInfo> getQueueCreateInfos();

            // TODO: 
            void printGPUInfo(std::vector<const char*>);

        private:
            vk::Device mLogicalDevice;
            vk::PhysicalDevice mPhysicalDevice;

            vk::Queue mGraphicsQueue;
            vk::Queue mPresentQueue;
            vk::Queue mTransferQueue;
            QueueFamilyIndices indices;

            VmaAllocator mAllocator = VK_NULL_HANDLE;

            Nova::GE::System* system = nullptr;

            vk::detail::DispatchLoaderDynamic dld;

            // TODO: As noted before move this somewhere else (factory.h)
            std::vector<ref<Image>> images;
            std::vector<ref<CommandBuffer>> commandBuffers;
            std::vector<ref<Shader>> shaders;
            std::vector<ref<Pipeline>> pipelines;
            std::vector<ref<UniformBuffer>> ubos;

            // TODO: Remove global CmdPool, should be managed by CPUF.h
            vk::CommandPool commandPool = VK_NULL_HANDLE;

<<<<<<< Updated upstream
            // TODO: Duplicate Logger
            NOVA_LOG_DEF("Device");
            std::unique_ptr<Nova::Core::Logger> log;
=======
            // std::unique_ptr<Nova::Core::Logger> log;
            u16 devId = 0;
>>>>>>> Stashed changes

            Nova::GE::DescriptorMan descriptorManager;


            usize uboDescsSize = 0;
            usize samplerDescSize = 0;
            usize ssboDescSize = 0;
            usize imageDescSize = 0;

            float m_queuePriority = 1.0f;

            // TODO: as well should be moved to CPUF.h
            ref<CommandBuffer> transferBuffer;
            bool transferBufferReady = false;
    };
    // TODO: move to core.h for example, bloats this file
    namespace CI = CreateInfo;
    
};