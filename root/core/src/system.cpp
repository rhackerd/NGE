#include "system.h"
#include "vulkan/vulkan.hpp"
#include <fmt/format.h>

namespace Nova::GE {
    bool System::init(CreateInfo::System& createInfo) {
        // NINFO("Initializing {} v{}.{}.{}", createInfo.appName, VK_VERSION_MAJOR(createInfo.appVersion), VK_VERSION_MINOR(createInfo.appVersion), VK_VERSION_PATCH(createInfo.appVersion));
        // NINFO("Initiating System");
        // NINFO("Initiating Dynamic DIspatcher");

        vk::detail::DynamicLoader dl;
        PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");

        dld.init(vkGetInstanceProcAddr);


        createInfo.extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        createInfo.extensions.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);

        // Make instance
        if (createInfo.validation) {
                createInfo.extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                // validation is NOT an extension, it's a layer
            }

            const char* validationLayer = "VK_LAYER_KHRONOS_validation";

    vk::ApplicationInfo appInfo = vk::ApplicationInfo()
        .setApiVersion(createInfo.apiVersion)
        .setApplicationVersion(createInfo.appVersion)
        .setEngineVersion(createInfo.engineVersion)
        .setPApplicationName(createInfo.appName.c_str());

        vk::InstanceCreateInfo instanceCreateInfo{};
        instanceCreateInfo
            .setPEnabledExtensionNames(createInfo.extensions)
            .setPApplicationInfo(
                &appInfo
            );

        instanceCreateInfo
            .setPEnabledExtensionNames(createInfo.extensions)
            .setPApplicationInfo(&appInfo);

        if (createInfo.validation) {
            instanceCreateInfo.setPEnabledLayerNames(validationLayer); // ← separate call
        }


        instance = vk::createInstance(instanceCreateInfo, nullptr, dld);

        if (instance == nullptr) {
            printf("system init(): Failed to create instance\n");
            return false;
        }

        printf("System has been initiated\n");
        printf("%s\n", fmt::format(" ├▶ API Version: {}.{}.{}", VK_VERSION_MAJOR(createInfo.apiVersion), VK_VERSION_MINOR(createInfo.apiVersion), VK_VERSION_PATCH(createInfo.apiVersion)).c_str());
        printf("%s\n", fmt::format(" ├▶ App Version: {}.{}.{}", VK_VERSION_MAJOR(createInfo.appVersion), VK_VERSION_MINOR(createInfo.appVersion), VK_VERSION_PATCH(createInfo.appVersion)).c_str());
        printf("%s\n", fmt::format(" ├▶ Engine Version: {}.{}.{}", VK_VERSION_MAJOR(createInfo.engineVersion), VK_VERSION_MINOR(createInfo.engineVersion), VK_VERSION_PATCH(createInfo.engineVersion)).c_str());
        printf("%s\n", fmt::format(" ├▶ App Name: {}", createInfo.appName).c_str());
        printf("%s\n", fmt::format(" └▶ Extensions: ").c_str());
        for (const auto& extension : createInfo.extensions) {
            // Check if last
            if (extension == createInfo.extensions.back()) {
                printf("  └─➤ %s\n", extension);
            }else {
                printf("  ├─➤ %s\n", extension);
            }
        }

        // NINFO("Stepping up dispatcher for instance");
        dld.init(instance);
        // NINFO("Stepped");

        return true;
    }

    void System::shutdown() {
        if (!instance) return;
        printf("system shutdown(): Shutting down System\n");
        instance.destroy();
        instance = nullptr;
        printf("system shutdown(): Shutting down\n");
    }
};