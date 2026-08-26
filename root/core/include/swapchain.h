#pragma once


#include "Image.h"
#include "core.h"
#include "system.h"
#include "vulkan/vulkan.hpp"
#include <cglm/vec2.h>
#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
#include <SDL3/SDL.h>
#include "device.h"

// TODO: modernize
// ========================================
// Metadata
//
// State    : #legacy@nge3
// Origin   : @nge2
//
// Desc     : Swapchain implementation with MSAA support.
// Info     : Is a mess in includes and needs to be moved to be per window and not per device.
// ========================================

namespace Nova::GE {
    

    namespace CreateInfo {
        struct Swapchain {
            Nova::GE::Device* device;
            vec2 extent;
            SDL_Window* window;
            vk::SampleCountFlagBits Samples = vk::SampleCountFlagBits::e4;
            vk::SurfaceKHR surface;
            
            class Builder {
                private: Swapchain* info;
                public: Builder() : info(new Swapchain()) {}
                        ~Builder() {if (info != nullptr) delete info;}

                    Builder& setDevice(Nova::GE::Device& device) {
                        info->device = &device;
                        return *this;
                    }

                    Builder& setExtent(vec2& extent) {
                        glm_vec2_copy(extent, info->extent);
                        return *this;
                    }


                    Builder& setExtent(float w, float h) {
                        info->extent[0] = w;
                        info->extent[1] = h;
                        return *this;
                    }

                    Builder& assignWindow(SDL_Window& win) {
                        info->window = &win;
                        return *this;
                    }

                    Builder& assignSurface(vk::SurfaceKHR surface) {
                        info->surface = surface;
                        return *this;
                    }

                    Builder& setSampling(vk::SampleCountFlagBits samples) {
                        info->Samples = samples;
                        return *this;
                    }
                    
                    Swapchain build() {
                        Swapchain result = *info;
                        delete info;
                        info = nullptr;
                        return result;
                    }
            };
        };
    };

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
    class Swapchain {
        public:
            Swapchain(CreateInfo::Swapchain& createInfo) {
                init(createInfo);
            }
            Swapchain() {};
            ~Swapchain() { shutdown(); };

        public:
            // Manual Control
            bool init(CreateInfo::Swapchain& createInfo);
            void shutdown();

            bool recreate(); // For example on win resize - will shutdown and recreate optimally

        public:
            bool advanceFrame();
            uint32_t getCurrentImageIndex() const { return imageIndex; }
            vk::Image getCurrentImage() const { return images[imageIndex]; }
            vk::ImageView getCurrentImageView() const { return imageViews[imageIndex]; }
            vk::Image getCurrentDepthImage() const { return m_depthImages[imageIndex]; }
            vk::ImageView getCurrentDepthImageView() const { return m_depthImagesViews[imageIndex]; }



        public:
            vk::SwapchainKHR getSwapchain() const { return swapchain; }
            vk::Extent2D getExtent() const { return {static_cast<uint32_t>(extent[0]), static_cast<uint32_t>(extent[1])}; }
            vk::Format getFormat() const { return m_format; }
            std::vector<vk::Image> getImages() const { return images; }
            std::vector<vk::ImageView> getImageViews() const { return imageViews; }
            std::vector<vk::Image> getDepthImages() const { return m_depthImages; }
            uint32_t getDepthImageCount() const { return static_cast<uint32_t>(m_depthImages.size()); }
            std::vector<vk::ImageView> getDepthImageViews() const { return m_depthImagesViews; }
            vk::Semaphore getImageAvailableSemaphore() const { return imageAvailable; }
            uint32_t getImageIndex() const { return imageIndex; }
            vk::Format getDepthFormat() const { return m_depthFormat; }
            inline vk::SampleCountFlagBits getSampleCount() const { return m_sampleCount; }

            inline vk::ImageView getCurrentMSColorImageView() const {
                if (m_sampleCount == vk::SampleCountFlagBits::e1 || m_msColorImageViews.empty()) { return nullptr; }
                return m_msColorImageViews[imageIndex];
            }

            inline vk::Image getCurrentMSColorImage() const {
                if (m_sampleCount == vk::SampleCountFlagBits::e1 || m_msColorImagePtrs.empty()) { return nullptr; }
                if (auto locked = m_msColorImagePtrs[imageIndex].lock()) { return locked->getImage(); }
                return nullptr;
            }

        private:
            vk::SurfaceFormat2KHR chooseFormat();
            uint32_t              chooseOptimalImgCount();
            void                  setupQueueSharing(vk::SwapchainCreateInfoKHR& createInfo);
            vk::PresentModeKHR    choosePresentMode();
            void                  updateExtent();
    

            void retrieveSwapchainImgs();

            void setupColorBuffer();
            void setupDepthBuffer();  

            void setupMSColorBuffer();

        public: void handleRecreation();
            
        private:
            vk::SwapchainKHR swapchain;

            // Color buffer
            std::vector<vk::Image> images;
            std::vector<vk::ImageView> imageViews;
            vk::Format m_format;

            // Depth buffer
            std::vector<vk::Image> m_depthImages;
            std::vector<ImagePtr> m_depthImagesPtrs;
            std::vector<vk::ImageView> m_depthImagesViews;
            std::vector<VmaAllocation> m_depthAllocations;
            vk::Format m_depthFormat;

            uint32_t imageIndex = 0;
            uint32_t imageCount;

            vk::Semaphore imageAvailable;

            CI::Swapchain m_createInfo;
            
            vec2 extent;
            Nova::GE::Device* device;
            SDL_Window* window;

            vk::SampleCountFlagBits m_sampleCount = vk::SampleCountFlagBits::e1;
            std::vector<weakRef<Image>> m_msColorImagePtrs;
            std::vector<vk::ImageView> m_msColorImageViews;
    };  
};