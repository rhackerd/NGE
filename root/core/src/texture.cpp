#include "texture.h"
#include "Image.h"
#include "commandBuffer.h"
#include "sampledImage.h"
#include "vulkan/vulkan.hpp"
#include <filesystem>
#include <memory>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

namespace Nova::GE {
    bool Texture::init(const std::string path, Device& device, vk::Sampler sampler) {
        int w,h,ch;
        printf("texture init(): loading %s\n", path.c_str());
        u8* pixels = stbi_load(path.c_str(), &w, &h, &ch, STBI_rgb_alpha);
        if (!pixels) {
            printf("Texture init: failed to load %s\n", stbi_failure_reason());
            return false;
        }

        SampledImage::Init(device, w, h, vk::Format::eR8G8B8A8Srgb,
                   vk::ImageUsageFlagBits::eTransferDst |
                   vk::ImageUsageFlagBits::eSampled,
                   sampler);

        device.uploadToImage(getImage().lock(), pixels);
        stbi_image_free(pixels);

        auto& descMan = device.getDescriptorManager();

        const u32 slot = descMan.allocateTextureSlot();


        descMan.writeTextureSlot(slot, getImageView(), vk::ImageLayout::eReadOnlyOptimal);
    
        m_c_dman =  &descMan;
            
        return true;
    }

    void Texture::shutdown() {
        m_c_dman->freeTextureSlot(m_slot);
        m_slot = 0;
    }
};