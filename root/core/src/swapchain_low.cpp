#include "device.h"
#include "swapchain.h"
#include "vulkan/vulkan.hpp"
#include <SDL3/SDL_video.h>
#include <limits>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace Nova::GE {
    vk::SurfaceFormat2KHR Swapchain::chooseFormat() {
        // First query available formats
        std::vector<vk::SurfaceFormat2KHR> available;
        available = device->getPhysicalDevice().getSurfaceFormats2KHR(m_createInfo.surface);
        // NINFO("Getting available formats");
        static const std::vector<vk::Format> FORMAT_PRIORITY = {
            vk::Format::eB8G8R8A8Srgb,
            vk::Format::eR8G8B8A8Srgb,
            vk::Format::eR8G8B8A8Unorm,
            vk::Format::eA2R10G10B10UnormPack32,
        };
        // Return the first (best) supported format, with its matching color space
        for (const auto& preffered : FORMAT_PRIORITY) {
            for (const auto& avail : available) {
                if (avail.surfaceFormat.format == preffered &&
                    avail.surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                    return avail;
                }
        }}
        // Nothing preferred, any supported format is better than eUndefined
        return available.front();
    };

    uint32_t Swapchain::chooseOptimalImgCount() {
        vk::SurfaceCapabilities2KHR caps = device->getPhysicalDevice().getSurfaceCapabilities2KHR(m_createInfo.surface, device->getDld());
        uint32_t imgCount = caps.surfaceCapabilities.minImageCount;
        if (caps.surfaceCapabilities.maxImageCount > 0 && imgCount > caps.surfaceCapabilities.maxImageCount) imgCount = caps.surfaceCapabilities.maxImageCount;
        // NINFO("{} image count", imgCount);
        return imgCount;
    }

    void Swapchain::setupQueueSharing(vk::SwapchainCreateInfoKHR& CreateInfo, uint32_t (&queueFamilies)[2]) {
      QueueFamilyIndices indices = device->getIndices();
      if (indices.graphicsFamily.value() == indices.presentFamily.value()) {
        CreateInfo.setImageSharingMode(vk::SharingMode::eExclusive);
        CreateInfo.setQueueFamilyIndices(nullptr);
        CreateInfo.setQueueFamilyIndexCount(0);
        // NINFO("Queue sharing is disabled");
      }else {
        // Stored in caller owned storage, the create info only keeps a pointer
        queueFamilies[0] = indices.graphicsFamily.value();
        queueFamilies[1] = indices.presentFamily.value();

        CreateInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
        CreateInfo.setQueueFamilyIndexCount(2);
        CreateInfo.setPQueueFamilyIndices(queueFamilies);
        // NINFO("Queue sharing is enabled");
      };
    };

    vk::PresentModeKHR Swapchain::choosePresentMode() {
        const auto& availModes = device->getPhysicalDevice().getSurfacePresentModesKHR(m_createInfo.surface, device->getDld());
        for (const auto& modes : availModes) {
            if (modes == vk::PresentModeKHR::eMailbox) {
                // NINFO("Enabling MailBox present mode");
                return modes;            
            }
        }
        // NINFO("Faillbacking into Fifo present modes");
        return vk::PresentModeKHR::eFifo;
    }

    void Swapchain::updateExtent() {
        vk::SurfaceCapabilities2KHR caps = device->getPhysicalDevice().getSurfaceCapabilities2KHR(m_createInfo.surface, device->getDld());
        if (caps.surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            extent[0] = caps.surfaceCapabilities.currentExtent.width;
            extent[1] = caps.surfaceCapabilities.currentExtent.height;
        }else {
            int width, height;
            SDL_GetWindowSize(window, &width, &height); // This should be later integrated to Nova Desktop

            extent[0] = width;
            extent[1] = height;
        }
    };


    
    
}
