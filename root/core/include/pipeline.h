#pragma once
#include "core.h"
#include "layout.h"
#include "shader.h"
#include "vulkan/vulkan.hpp"
#include <array>
#include <concepts>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

// TODO: modernize
// ========================================
// Metadata
//
// State    : #modern@nge3
// Origin   : @nge2
//
// Desc     : Pipeline vulkan wrapper
// Info     : is pretty much modern and okay, even includes are okay.
// ========================================

namespace Nova::GE {

    namespace CreateInfo {
        struct Pipeline {
            std::vector<vk::PipelineShaderStageCreateInfo> shaders;
            BakedLayout layout;
            vk::PipelineVertexInputStateCreateInfo vertexLayout;
            vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
            vk::PipelineViewportStateCreateInfo viewportState;
            vk::PipelineRasterizationStateCreateInfo rasterizer;
            vk::PipelineMultisampleStateCreateInfo multisampling;
            vk::PipelineColorBlendAttachmentState colorBlendAttachment;
            vk::PipelineColorBlendStateCreateInfo colorBlending;
            vk::PipelineDepthStencilStateCreateInfo depthStencil;
            std::vector<vk::DynamicState> dynamicStates;
            vk::VertexInputBindingDescription binding;
            std::vector<vk::VertexInputAttributeDescription> vertexAttribs;
            vk::Format format;
            vk::Format depthFormat;
            vk::SampleCountFlagBits samples;
            class Builder;
        };

        template<typename T>
        concept HasVertexLayout = requires {
            {T::binding()} -> std::convertible_to<vk::VertexInputBindingDescription>;
            {T::attributes()} -> std::convertible_to<std::vector<vk::VertexInputAttributeDescription>>;      
        };

        class Pipeline::Builder {
            private: Pipeline* info;
            public:
                Builder() {
                    info = new Pipeline();
                }
                Builder& addShader(weakRef<Nova::GE::Shader> shader) {
                    info->shaders.push_back(shader.lock()->getStageInfo());
                    return *this;
                }
                template<typename T> requires HasVertexLayout<T>
                [[deprecated("Not recommended, and not supported anymore. Will not affect anything.")]]
                Builder& setVertexLayoutCustom() {
                    auto binding = T::binding();
                    auto attrs = T::attributes();
                    info->vertexLayout.setVertexBindingDescriptionCount(1)
                    .setPVertexBindingDescriptions(&binding)
                    .setVertexAttributeDescriptionCount(attrs.size())
                    .setPVertexAttributeDescriptions(attrs.data());
                    return *this;
                }
                Builder& setLayout(BakedLayout layout) {
                    info->layout = layout;
                    return *this;
                }
                Builder& addDynamicState(vk::DynamicState state) {
                    info->dynamicStates.push_back(state);
                    return *this;
                };
                Builder& setViewportState(u32 viewportCount, u32 scissorCount) {
                    info->viewportState.viewportCount = viewportCount;
                    info->viewportState.scissorCount = scissorCount;
                    return *this;
                }
                Builder& prepareDefaultVertexLayout() {
                    info->vertexAttribs.clear();
                    info->binding = Vertex::binding();
                    
                    auto attrs = Vertex::attributes();
                    info->vertexAttribs.insert(
                        info->vertexAttribs.end(), 
                        attrs.begin(), 
                        attrs.end()
                    );
                    
                    info->vertexLayout
                        .setVertexBindingDescriptionCount(1)
                        .setPVertexBindingDescriptions(&info->binding)
                        .setVertexAttributeDescriptionCount(info->vertexAttribs.size())
                        .setPVertexAttributeDescriptions(info->vertexAttribs.data());
                    return *this;
                }
                Builder& prepareDefaultInputAssembly() {
                    info->inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
                    info->inputAssembly.primitiveRestartEnable = false;
                    return *this;
                };
                Builder& prepareDefaultDynamicState(u32 viewportCount = 1, u32 scissorCount = 1) {
                    info->dynamicStates.push_back(vk::DynamicState::eViewport);
                    info->dynamicStates.push_back(vk::DynamicState::eScissor);
                    this->setViewportState(viewportCount, scissorCount);
                    return *this;  
                };
                Builder& prepareDefaultRasterizer() {
                    info->rasterizer.depthClampEnable = false;
                    info->rasterizer.rasterizerDiscardEnable = false;
                    info->rasterizer.polygonMode = vk::PolygonMode::eFill;
                    info->rasterizer.lineWidth = 1.0f;
                    info->rasterizer.cullMode = vk::CullModeFlagBits::eBack;
                    info->rasterizer.frontFace = vk::FrontFace::eCounterClockwise;
                    info->rasterizer.depthBiasEnable = false;
                    return *this;
                }
                Builder& prepareDefaultMultisampling() {
                    info->multisampling.sampleShadingEnable = false;
                    info->multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
                    info->multisampling.minSampleShading = 1.0f;
                    info->multisampling.pSampleMask = nullptr;
                    info->multisampling.alphaToCoverageEnable = false;
                    info->multisampling.alphaToOneEnable = false;
                    return *this;
                }
                Builder& setMSAA(vk::SampleCountFlagBits samples) {
                    info->multisampling.sampleShadingEnable = false;
                    info->multisampling.rasterizationSamples = samples;
                    info->multisampling.minSampleShading = 1.0f;
                    info->multisampling.pSampleMask = nullptr;
                    info->multisampling.alphaToCoverageEnable = false;
                    info->multisampling.alphaToOneEnable = false;
                    return *this;
                };
                Builder& prepareDefaultColorBlending() {
                    this->prepareDefaultBlendOp();
                    info->colorBlending.attachmentCount = 1;
                    info->colorBlending.pAttachments = &info->colorBlendAttachment;
                    info->colorBlending.blendConstants[0] = 0.0f;
                    info->colorBlending.blendConstants[1] = 0.0f;
                    info->colorBlending.blendConstants[2] = 0.0f;
                    info->colorBlending.blendConstants[3] = 0.0f;
                    return *this;
                }
                Builder& setSampleCount(vk::SampleCountFlagBits samples) {
                    info->samples = samples;
                    return *this;
                }
                Builder& prepareDefaultDepthStencil() {
                    info->depthStencil.depthTestEnable = true;
                    info->depthStencil.depthWriteEnable = true;
                    info->depthStencil.depthCompareOp = vk::CompareOp::eLess;
                    info->depthStencil.depthBoundsTestEnable = false;
                    info->depthStencil.minDepthBounds = 0.0f;
                    info->depthStencil.maxDepthBounds = 1.0f;
                    info->depthStencil.stencilTestEnable = false;
                    return *this;
                }
                Builder& prepareDefaultBlendOp() {
                    info->colorBlendAttachment.blendEnable = false;
                    info->colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
                    info->colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eOne;
                    info->colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eZero;
                    info->colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
                    info->colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
                    info->colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
                    info->colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;
                    return *this;
                }

                Builder& prepareDefault() {
                    this->prepareDefaultVertexLayout();
                    this->prepareDefaultDynamicState();
                    this->prepareDefaultRasterizer();
                    this->prepareDefaultInputAssembly();
                    this->prepareDefaultMultisampling();
                    this->prepareDefaultColorBlending();
                    this->prepareDefaultDepthStencil();
                    return *this;
                }
                Builder& setSwapchainFormat(vk::Format format) {
                    info->format = format;
                    return *this;
                }
                Builder& setDepthFormat(vk::Format format) {
                    info->depthFormat = format;
                    return *this;
                }
                Pipeline build() {
                    Pipeline result = *info;
                    delete info;
                    info = nullptr;
                    return result;
                }
        };
    };

    class Pipeline {
        public:
            Pipeline(vk::Device device, vk::detail::DispatchLoaderDynamic& dld, CreateInfo::Pipeline& ci) {
                init(device, dld, ci);
            };
            ~Pipeline() {shutdown();};
        public:
            void init(vk::Device device, vk::detail::DispatchLoaderDynamic& dld, CreateInfo::Pipeline& ci);
            void shutdown();

        public:
            vk::Pipeline get() { return m_pipeline; }
            vk::PipelineLayout getLayout() { return layout; }
            vk::DescriptorSetLayout getSetLayout() { return setLayout; }
            
        private:
            vk::Pipeline m_pipeline = VK_NULL_HANDLE;
            vk::PipelineLayout layout = VK_NULL_HANDLE;
            vk::Device device = VK_NULL_HANDLE;
            vk::DescriptorSetLayout setLayout = VK_NULL_HANDLE;
    };
};