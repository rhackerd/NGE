#pragma once

#include "core.h"
#include "system.h"
#include "types.h"
#include "vulkan/vulkan.hpp"
#include <SDL3/SDL_log.h>
#include <cassert>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <unordered_set>

// TODO: modernize
// ========================================
// Metadata
//
// State    : #legacy@nge3
// Origin   : @nge2
//
// Desc     : Manager of vulkan descriptors using bindless and descriptor buffer
// Info     : Practically okay but still needs revision
// ========================================

namespace Nova::GE {

    struct SetHandle {
        usize baseOffset = 0;
        vk::DescriptorSetLayout layout = VK_NULL_HANDLE;
        u32 setIndex = 0;
    };

    struct DescriptorSetLayout {
        vk::DescriptorSetLayout layout = VK_NULL_HANDLE;
        u32 setIndex = 0;
    };

    inline usize align(usize offset, usize alignment) {
        return (offset + alignment - 1) & ~(alignment - 1);
    }

    class DescriptorMan {
    public:
        DescriptorMan()  = default;
        ~DescriptorMan() = default;

        void init(VmaAllocator allocator, DevicePackage device, vk::PhysicalDeviceDescriptorBufferPropertiesEXT decsProps, size_t size = 1024 * 1024);
        void shutdown(VmaAllocator allocator);

        usize getCurrentOffset() const { return m_offset; }
        vk::Buffer buffer()      const { return m_buffer; }

        usize getSetSize(vk::DescriptorSetLayout setLayout) {
            vk::DeviceSize size = 0;
            m_device.getDescriptorSetLayoutSizeEXT(setLayout, &size, m_dld);
            return size;
        }

        usize getBindingOffset(vk::DescriptorSetLayout setLayout, u32 binding) {
            vk::DeviceSize offset = 0;
            m_device.getDescriptorSetLayoutBindingOffsetEXT(setLayout, binding, &offset, m_dld);
            return offset;
        }

        SetHandle& getBindlessSet() {
            return m_bindlessSet;
        }

        u32 allocateTextureSlot() {
            std::lock_guard lock(m_textureMutex);
            if (m_freeTextureSlots.empty()) printf("DescMan allocateTextureSlot(): error, no empty free slots\n");
            u32 slot = m_freeTextureSlots.back();
            m_freeTextureSlots.pop_back();
            return slot;
        }
        
        void freeTextureSlot(u32 slot) {
            std::lock_guard lock(m_textureMutex);
            m_freeTextureSlots.push_back(slot);
        }

        void writeTextureSlot(u32 slot, vk::ImageView view, vk::ImageLayout layout) {
            usize bindingOff = getBindingOffset(m_bindlessSet.layout, m_bindlessBinding);
            usize elemOff = bindingOff + slot * m_descProps.sampledImageDescriptorSize;
            _writeSampledImage(view, layout, m_bindlessSet.baseOffset + elemOff);
        }

        // Reserve space for a full descriptor set, returns base offset
        SetHandle allocateSet(vk::DescriptorSetLayout setLayout, u32 setIndex);

        void writeUBO(const SetHandle& set, u32 binding, vk::Buffer ubo, usize size) {
            usize bindingOff = getBindingOffset(set.layout, binding);
            _writeUBO(ubo, size, set.baseOffset + bindingOff);
        };

        void writeSampler(const SetHandle& set, u32 binding, vk::Sampler sampler) {
            usize bindingOff = getBindingOffset(set.layout, binding);
            _writeSampler(sampler, set.baseOffset + bindingOff);
        }

        void writeSampledImage(const SetHandle& set, u32 binding, vk::ImageView view, vk::ImageLayout layout) {
            usize bindingOff = getBindingOffset(set.layout, binding);
            _writeSampledImage(view, layout, set.baseOffset + bindingOff);
        }
        void freeSet(const SetHandle& set);

    private:
        void _writeSampler(vk::Sampler sampler, usize atOffset);
        void _writeSampledImage(vk::ImageView view, vk::ImageLayout layout, usize atOffset);
        void _writeUBO(vk::Buffer ubo, usize size, usize atOffset);
        void _initBindless(u32 binding, u32 maxTextures);

    private:
        void* ptr(usize offset) { return static_cast<char*>(m_ptr) + offset; }

        vk::Buffer     m_buffer     = VK_NULL_HANDLE;
        VmaAllocation  m_allocation = {};
        VmaAllocationInfo m_info    = {};
        void*          m_ptr        = nullptr;
        usize          m_offset     = 0;
        vk::Device     m_device;
        vk::detail::DispatchLoaderDynamic m_dld;
        vk::PhysicalDeviceDescriptorBufferPropertiesEXT m_descProps{};
        std::unordered_map<VkDescriptorSetLayout, std::vector<usize>> m_freeLists;
        usize m_capacity = 0;
        std::mutex m_mutex;
        vk::DescriptorSetLayout m_bindlessLayout = VK_NULL_HANDLE;
        #ifndef NDEBUG
        std::unordered_set<usize> m_liveOffsets; // fixed from unordered_map
        #endif

        // bindless texture array state
        SetHandle m_bindlessSet{};
        u32 m_bindlessBinding = 0;
        u32 m_textureCapacity = 0;
        std::vector<u32> m_freeTextureSlots;
        std::mutex m_textureMutex;
    };
}