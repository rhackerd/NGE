#include "descMan.h"

#include <fmt/format.h>

namespace Nova::GE {

    void DescriptorMan::_initBindless(u32 binding, u32 maxTextures) {
        vk::DescriptorSetLayoutBinding bind{};
        bind.binding = binding;
        bind.descriptorType = vk::DescriptorType::eSampledImage;
        bind.descriptorCount = maxTextures;
        bind.stageFlags = vk::ShaderStageFlagBits::eFragment;

        vk::DescriptorSetLayoutCreateInfo layoutCI{};
        layoutCI.setBindings(bind);
        layoutCI.flags = vk::DescriptorSetLayoutCreateFlagBits::eDescriptorBufferEXT;

        m_bindlessLayout = m_device.createDescriptorSetLayout(layoutCI, nullptr, m_dld);  // ← add m_dld, and while here, actually store it (see below)
        m_bindlessSet = allocateSet(m_bindlessLayout, 0);

        m_bindlessBinding = binding;
        m_textureCapacity = maxTextures;

        m_freeTextureSlots.reserve(maxTextures);
        for (u32 i = maxTextures; i-- > 0; ) m_freeTextureSlots.push_back(i);
    }

    void DescriptorMan::init(VmaAllocator allocator, DevicePackage device, vk::PhysicalDeviceDescriptorBufferPropertiesEXT descProps, size_t size) {
        VkBufferCreateInfo bufCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size  = size,
            .usage = VK_BUFFER_USAGE_2_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT |
                     VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        };
        VmaAllocationCreateInfo allocCI{
            .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT |
                     VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO,
        };

        m_capacity = size;

        VkBuffer buffer;
        vmaCreateBuffer(allocator, &bufCI, &allocCI, &buffer, &m_allocation, &m_info);
        m_buffer = buffer;
        m_ptr    = m_info.pMappedData;

        m_device         = device.device;
        m_dld            = device.dld;
        m_descProps      = descProps;
        m_offset         = 0;

        _initBindless(6, 256);
    }

    void DescriptorMan::shutdown(VmaAllocator allocator) {
        if (m_bindlessLayout) {
            m_device.destroyDescriptorSetLayout(m_bindlessLayout);
            m_bindlessLayout = VK_NULL_HANDLE;
        }
        if (m_buffer != VK_NULL_HANDLE) {
            vmaDestroyBuffer(allocator, m_buffer, m_allocation);
            m_buffer     = VK_NULL_HANDLE;
            m_allocation = VK_NULL_HANDLE;
            m_ptr        = nullptr;
            m_offset     = 0;
        }
    }
    
    void DescriptorMan::_writeSampledImage(vk::ImageView view, vk::ImageLayout layout, usize atOffset) {
        vk::DescriptorImageInfo imageInfo{};
        imageInfo.setImageView(view).setImageLayout(layout);

        vk::DescriptorDataEXT data{};
        data.pSampledImage = &imageInfo;

        vk::DescriptorGetInfoEXT getInfo{};
        getInfo.setType(vk::DescriptorType::eSampledImage).setData(data);

        m_device.getDescriptorEXT(&getInfo, m_descProps.sampledImageDescriptorSize,
        ptr(atOffset), m_dld);
    }

    void DescriptorMan::_writeSampler(vk::Sampler sampler, usize atOffset) {
        vk::DescriptorDataEXT data{};
        data.pSampler = &sampler;

        vk::DescriptorGetInfoEXT getInfo{};
        getInfo.setType(vk::DescriptorType::eSampler).setData(data);

        m_device.getDescriptorEXT(&getInfo, m_descProps.samplerDescriptorSize,
            ptr(atOffset), m_dld);
    }

    void DescriptorMan::_writeUBO(vk::Buffer ubo, usize size, usize atOffset) {
        vk::BufferDeviceAddressInfo addrInfo{ubo};
        auto addr = m_device.getBufferAddress(addrInfo);

        vk::DescriptorAddressInfoEXT bufInfo{};
        bufInfo.setAddress(addr).setRange(size).setFormat(vk::Format::eUndefined);

        vk::DescriptorDataEXT data{};
        data.pUniformBuffer = &bufInfo;

        vk::DescriptorGetInfoEXT getInfo{};
        getInfo.setType(vk::DescriptorType::eUniformBuffer).setData(data);

        // m_descProps.uniformBufferDescriptorSize // UBO size

        m_device.getDescriptorEXT(&getInfo, m_descProps.uniformBufferDescriptorSize,
            ptr(atOffset), m_dld);
    }

    SetHandle DescriptorMan::allocateSet(vk::DescriptorSetLayout setLayout, u32 setIndex) {
        std::lock_guard lock(m_mutex);

        auto& freeList = m_freeLists[setLayout];
        usize base;
        if (!freeList.empty()) {
            base = freeList.back();
            freeList.pop_back();
        } else {
            usize size = getSetSize(setLayout);
            base = align(m_offset, m_descProps.descriptorBufferOffsetAlignment);
            SDL_Log("allocateSet: layout=%p size=%zu base=%zu -> new m_offset=%zu",
                    (void*)VkDescriptorSetLayout(setLayout), size, base, base + size);
            if (base + size > m_capacity) {
                printf("%s", fmt::format("Descriptor buffer arena exhausted ({} / {} bytes)", base + size, m_capacity).c_str());
            }
            m_offset = base + size;
        }

        #ifndef NDEBUG
        m_liveOffsets.insert(base);
        #endif

        return {base, setLayout, setIndex};
    }

    void DescriptorMan::freeSet(const SetHandle& set) {
        std::lock_guard lock(m_mutex);

        #ifndef NDEBUG
        auto it = m_liveOffsets.find(set.baseOffset);
        // NOVA_ASSERT(it != m_liveOffsets.end(), "freeSet called on an offset that isn't currently allocated (double-free?)");
        m_liveOffsets.erase(it);
        #endif

        m_freeLists[set.layout].push_back(set.baseOffset);
    }
}