#pragma once

#include "types.h"
#include "vulkan/vulkan.hpp"
#include <memory>

// TODO: modernize
// ========================================
// Metadata
//
// State    : #legacy@nge3
// Origin   : #legacy@nge3
//
// Desc     : Wrapper around commandBuffer
// Info     : in first look looks okay but i will still rewrite some things
// ========================================

namespace Nova::GE {
    namespace CreateInfo {
        struct CommandBuffer {
            u32 CommandBufferCount = 8;
            bool secondary = false;
            bool manual = false;
            vk::Device device;


            class Builder;
        };
        class CommandBuffer::Builder {
            private: CommandBuffer* info;
            public:
                Builder() {
                    info = new CommandBuffer();
                    info->CommandBufferCount = 1;
                }
                Builder& setCommandBufferCount(u32 count) { info->CommandBufferCount = count; return *this; }
                Builder& setSecondary(bool secondary) { info->secondary = secondary; return *this; }
                Builder& setPrimary() { info->secondary = false; return *this; }
                Builder& setManual(bool manual) { info->manual = manual; return *this; }
                CommandBuffer build() {
                    CommandBuffer result = *info;
                    delete info;
                    info = nullptr;
                    return result;
                }
        };
    }


    class CommandBuffer {
    public:
        CommandBuffer(CreateInfo::CommandBuffer& createInfo);
        CommandBuffer() {}
        ~CommandBuffer() { shutdown(); }
    public:
        // Manual Control
        bool init(CreateInfo::CommandBuffer& createInfo);
        void shutdown();
        vk::CommandBuffer getCommandBuffer() { return commandBuffer; }
        bool isSecondary() { return secondary; }
        void set(vk::CommandBuffer commandBuffer) { this->commandBuffer = commandBuffer; }
    private:
        vk::CommandBuffer commandBuffer;
        vk::Device device;
        bool secondary;
    };

    using CommandBufferPtr = std::shared_ptr<CommandBuffer>;
};