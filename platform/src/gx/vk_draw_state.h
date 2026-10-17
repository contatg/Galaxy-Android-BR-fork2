#pragma once
#include <vulkan/vulkan.h>
#include <cstring>

namespace gpu {
// Scoped to one command buffer. Copy/render-pass transitions explicitly reset
// the cache; no bindings survive into another frame or another command buffer.
class VkDrawState {
public:
    VkDrawState(VkCommandBuffer command, VkPipelineLayout layout) : command(command), layout(layout) {}
    void reset() {
        pipeline = VK_NULL_HANDLE;
        textures = VK_NULL_HANDLE;
        viewportValid = scissorValid = false;
    }
    void setViewport(const VkViewport& value) {
        if (!viewportValid || std::memcmp(&viewport, &value, sizeof(value)) != 0) {
            vkCmdSetViewport(command, 0, 1, &value);
            viewport = value; viewportValid = true;
        }
    }
    void setScissor(const VkRect2D& value) {
        if (!scissorValid || std::memcmp(&scissor, &value, sizeof(value)) != 0) {
            vkCmdSetScissor(command, 0, 1, &value);
            scissor = value; scissorValid = true;
        }
    }
    void bindPipeline(VkPipeline value) {
        if (pipeline != value) {
            vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, value);
            pipeline = value;
        }
    }
    void bindTextures(VkDescriptorSet value) {
        if (textures != value) {
            vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 1, 1, &value, 0, nullptr);
            textures = value;
        }
    }
private:
    VkCommandBuffer command;
    VkPipelineLayout layout;
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkDescriptorSet textures = VK_NULL_HANDLE;
    VkViewport viewport{};
    VkRect2D scissor{};
    bool viewportValid = false, scissorValid = false;
};
} // namespace gpu
