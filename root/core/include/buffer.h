#pragma once

#include "system.h"
#include <vulkan/vulkan_core.h>
#include <cassert>

// TODO: modernize
// ========================================
// Metadata
//
// State    : #legacy@nge3
// Origin   : @nge2
//
// Desc     : Wrapper around vk::buffer
// Info     : cover faults and move into modern spec.
// ========================================

namespace Nova::GE {
    namespace CreateInfo {
        struct Buffer {
            VmaAllocator allocator;
            vk::DeviceSize size = 0;
            vk::BufferUsageFlags usage = vk::BufferUsageFlagBits{};
            VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_GPU_ONLY;

            class Builder;
        };

        class Buffer::Builder {
            private: Buffer* info;
            public:
                Builder() {
                    info = new Buffer();
                }
                Builder& setAllocator(VmaAllocator allocator) { info->allocator = allocator; return *this; }
                Builder& setSize(vk::DeviceSize size) { info->size = size; return *this; }
                Builder& setUsage(vk::BufferUsageFlags usage) { info->usage = usage; return *this; }
                Builder& setMemUsage(VmaMemoryUsage memUsage) { info->memUsage = memUsage; return *this; }

                // presets
                Builder& asVertex() { return setUsage(vk::BufferUsageFlagBits::eVertexBuffer).setMemUsage(VMA_MEMORY_USAGE_CPU_TO_GPU); }
                Builder& asIndex()  { return setUsage(vk::BufferUsageFlagBits::eIndexBuffer).setMemUsage(VMA_MEMORY_USAGE_CPU_TO_GPU);  }
                Builder& asUniform(){ 
                    return 
                    setUsage(vk::BufferUsageFlagBits::eUniformBuffer
                    | vk::BufferUsageFlagBits::eShaderDeviceAddress)
                    .setMemUsage(VMA_MEMORY_USAGE_CPU_TO_GPU);
                }
                Builder& asStaging(){ return setUsage(vk::BufferUsageFlagBits::eTransferSrc).setMemUsage(VMA_MEMORY_USAGE_CPU_TO_GPU);  }

                Buffer build() {
                    Buffer result = *info;
                    delete info;
                    info = nullptr;
                    return result;
                }
        };
    };

    // This is maybe the best code i have written, or atleast the most clean one.

    // This class doesn't have a device create function
    // Because it should be managed by the Engine or class that uses it
    // For high level usage use things like UBO, Sample2D, etc...
    class Buffer {
        public:
            Buffer() = default;
            Buffer(CreateInfo::Buffer& createInfo) { init(createInfo); }
            ~Buffer() = default;
        public:
            // by usual order
            void init(CreateInfo::Buffer& createInfo);
            void upload(const void* data, vk::DeviceSize size, vk::DeviceSize offset = 0);
            void shutdown();
        public:
            vk::Buffer      getBuffer() const   { return m_buffer;  }
            vk::DeviceSize  getSize()   const   { return m_size;    }
            VmaAllocation   getAlloc()  const   { return m_alloc;   }
            bool            isValid()   const   { return m_buffer != VK_NULL_HANDLE;}
        private:
            vk::Buffer          m_buffer        = VK_NULL_HANDLE;
            VmaAllocation       m_alloc         = VK_NULL_HANDLE;
            VmaAllocationInfo   m_allocInfo     = {};
            vk::DeviceSize      m_size          = 0;
            VmaAllocator        m_allocator     = nullptr;
    };
};