#pragma once

#include "descMan.h"
#include "sampledImage.h"
#include <Nova/Core/log.h>
#include <Nova/Core/macros.h>
#include <string>
#include <vulkan/vulkan_core.h>
#include "stb_image.h"
#include "vulkan/vulkan.hpp"

// TODO: modernize
// ========================================
// Metadata
//
// State    : #modern@nge3
// Origin   : @nge2
//
// Desc     : Wrapper around SampledImage with TexSlot being stored (bindless)
// Info     : Stores ID of the texture in the bindless buffer
// ========================================

namespace Nova::GE {
    // Too high level for createInfo

    class Texture : public SampledImage {
        public:
            Texture() {};
            ~Texture() {shutdown();};

        public:
            bool init(const std::string path, Device& device, vk::Sampler sampler);
            void shutdown();

            u32 getSlot() const { return m_slot; }

        private:
            u32 m_slot = 0;
            DescriptorMan* m_c_dman;
    };
};