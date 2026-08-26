#pragma once



#include "system.h"
#include "vulkan/vulkan.hpp"
#include <cglm/types.h>
#include <cglm/vec2.h>
#include <cstdint>
#include <memory>
#include <vulkan/vulkan_core.h>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>


// TODO: modernize
// ========================================
// Metadata
//
// State    : #modern@nge3
// Origin   : @nge2
//
// Desc     : Image wrapper and ImageView handler
// Info     : NPretty much modern, only needs a revision and remove some includes.
// ========================================

namespace Nova::GE {


    namespace CreateInfo {
        struct Image {
            vk::ImageCreateInfo info;
            VmaAllocator * allocator;
            vk::Device device;
            vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1;

            class Builder;
        };

        enum class ImageType {
            e1D = static_cast<int>(vk::ImageType::e1D),
            e2D = static_cast<int>(vk::ImageType::e2D),
            e3D = static_cast<int>(vk::ImageType::e3D)
        };


        class Image::Builder {
            private: Image* info;
            public:
                Builder() {
                    info = new Image();
                    info->info.usage = vk::ImageUsageFlagBits::eSampled;
                    info->allocator = nullptr;
                }

                Builder& setAllocator(VmaAllocator * allocator) {
                    info->allocator = allocator;
                    return *this;
                }

                Builder& setDevice(vk::Device device) {
                    info->device = device;
                    return *this;
                }

                Builder& setType(ImageType type) {
                    info->info.imageType = static_cast<vk::ImageType>(type);
                    return *this;
                }

                Builder& setFormat(vk::Format format) {
                    info->info.format = format;
                    return *this;
                }

                Builder& setInitialLayout(vk::ImageLayout layout) {
                    info->info.initialLayout = layout;
                    return *this;
                }

                Builder& setSampling(vk::SampleCountFlagBits sampling) {
                    info->samples = sampling;
                    return *this;
                }

                Builder& setExtent(float w, float h, uint32_t depth = 1) {
                    info->info.extent = vk::Extent3D{
                        static_cast<uint32_t>(w),
                        static_cast<uint32_t>(h),
                        depth
                    };
                    return *this;
                }

                Builder& setExtent(vec2 size, uint32_t depth = 1) {
                    info->info.extent = vk::Extent3D{
                        static_cast<uint32_t>(size[0]), 
                        static_cast<uint32_t>(size[1]), 
                        depth
                    };
                    return *this;
                }

                Image build() {
                    Image result = *info;
                    delete info;
                    info = nullptr;
                    return result;
                }
        };
    };

    
    struct ImageViewHandle {
        uint32_t index = UINT32_MAX;
        uint32_t generation = 0;

        explicit operator bool() const {
            return index != UINT32_MAX;
        }
    };

    struct StoredImageView  {
        vk::ImageView view = VK_NULL_HANDLE;
        uint32_t generation = 0;
        vk::ImageAspectFlags aspect{};
    };

    class Image;

    using ImagePtr = std::shared_ptr<Image>;

    /**
     * @class Image
     * @brief Image is a high level wrapper around Vk::Image and Vk::ImageView
     */
    class Image {
        public:
            Image(CreateInfo::Image& createInfo) {
                init(createInfo);
            }
            Image() {};
            ~Image() { shutdown(); };

        public:
            // Manual Control
            bool init(CreateInfo::Image& createInfo);
            void shutdown();

            vk::Image getImage() { return image; }
            vk::Format getFormat() { return format; }
            [[nodiscard]]
            ImageViewHandle createView(vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor);
            [[nodiscard]]
            vk::ImageView resolve(ImageViewHandle handle);

        public:
            // Getters
            void getExtent(vec2 out) { glm_vec2_copy(extent, out); }
            Image *getThis() { return this; }
            VmaAllocation getAlloc() { return alloc; }

            
        private:
            
        private:
            vk::Image image;
            vk::Format format;
            VmaAllocation alloc;
            vk::ImageCreateInfo info;
            
            vec2 extent;
            VmaAllocator allocator;
            vk::Device device;

            std::vector<StoredImageView> views;
            uint32_t currentGeneration = 1;
    };

    
};
