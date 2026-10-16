#include "high-level/graphics.h"
#include "device.h"
#include "system.h"
#include "vulkan/vulkan.hpp"
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan_core.h>

namespace Nova::Graphics {
    bool Graphics::init(SDL_Window* window) {

        Uint32 count = 0;
        const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&count);
        if (!extensions) {
            printf("Graphics init(): failed to query SDL vk extensions (%s)\n", SDL_GetError());
            return false;
        }

        std::vector<const char*> extensionNames(extensions, extensions + count);

        auto sCI = Nova::GE::CreateInfo::System::Builder()
            .setAppName("ExampleApp")
            .setAppVersion(VK_MAKE_VERSION(1, 0, 0))
            .enableValidation()
            .addRecommendedValidationFeatures()
            .addExtensions(extensionNames)
            .build();
        if (!m_system.init(sCI)) return false;

        VkSurfaceKHR surface;
        if (!SDL_Vulkan_CreateSurface(window, m_system.getInstance(), nullptr, &surface)) {
            printf("Graphics init(): failed to create vulkan surface (%s)\n", SDL_GetError());
            return false;
        }

        auto dCI = Nova::GE::CreateInfo::Device::Builder()
            .setSystem(&m_system)
            .setSurface(surface)
            .pickPhysicalDevice()
            .enableBindless()
            .enableDefaultExt()
            .enableDescriptorBuffer()
            .build();
        if(!m_device.init(dCI)) return false;

        auto scCI = Nova::GE::CreateInfo::Swapchain::Builder()
            .assignWindow(*window)
            .setDevice(m_device)
            .setExtent(800, 600) // TODO: In window add getSize and much more getters
            .setSampling(vk::SampleCountFlagBits::e1)
            .assignSurface(surface)
            .build();
        if(!m_swapchain.init(scCI)) return false;

        m_window  = window;
        m_surface = surface;

        return true;
    }

    void Graphics::shutdown() {
        if (m_window == nullptr) return; // not initialized or already shut down (dtor after manual shutdown)
        m_swapchain.shutdown();
        m_device.shutdown();
        // VkSurfaceKHR must be destroyed after the swapchain and before the instance
        // (SDL_DestroyWindowSurface is for SDL's software surface, not Vulkan)
        if (m_surface != VK_NULL_HANDLE) {
            SDL_Vulkan_DestroySurface(m_system.getInstance(), m_surface, nullptr);
            m_surface = VK_NULL_HANDLE;
        }
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
        m_system.shutdown();
    };
};