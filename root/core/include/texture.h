#pragma once

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
// Desc     : Better named `SampledImage` 
// Info     : I don't think additional description is needed
// ========================================

namespace Nova::GE {
    // Too high level for createInfo

    class Texture : public SampledImage {
        public:
            Texture() {};
            ~Texture() {};

        public:
            bool init(const std::string path, Device& device, vk::Sampler sampler);
    };
};