#include "gx/vk_draw_state.h"
#include <cassert>
#include <cstdio>
static unsigned pipelines = 0, textures = 0, viewports = 0, scissors = 0;
static VkViewport lastViewport{};
static VkRect2D lastScissor{};
extern "C" {
void vkCmdBindPipeline(VkCommandBuffer, VkPipelineBindPoint, VkPipeline) { ++pipelines; }
void vkCmdBindDescriptorSets(VkCommandBuffer, VkPipelineBindPoint, VkPipelineLayout, uint32_t first, uint32_t count, const VkDescriptorSet*, uint32_t, const uint32_t*) {
    assert(first == 1 && count == 1); ++textures;
}
void vkCmdSetViewport(VkCommandBuffer, uint32_t, uint32_t count, const VkViewport* p) { assert(count == 1); lastViewport = *p; ++viewports; }
void vkCmdSetScissor(VkCommandBuffer, uint32_t, uint32_t count, const VkRect2D* p) { assert(count == 1); lastScissor = *p; ++scissors; }
}
int main() {
    gpu::VkDrawState cache(VK_NULL_HANDLE, VK_NULL_HANDLE);
    VkViewport viewport{0, 0, 640, 360, 0, 1};
    VkRect2D scissor{{0, 0}, {640, 360}};
    auto apply = [&] {
        cache.bindPipeline((VkPipeline)1); cache.bindTextures((VkDescriptorSet)1);
        cache.setViewport(viewport); cache.setScissor(scissor);
    };
    for (int i = 0; i < 100; ++i) apply();
    assert(pipelines == 1 && textures == 1 && viewports == 1 && scissors == 1);
    cache.bindPipeline((VkPipeline)2); cache.bindTextures((VkDescriptorSet)2);
    viewport.width = 426; scissor.extent.width = 426;
    cache.setViewport(viewport); cache.setScissor(scissor);
    assert(pipelines == 2 && textures == 2 && viewports == 2 && scissors == 2);
    assert(lastViewport.width == 426 && lastScissor.extent.width == 426);
    cache.reset();
    cache.bindPipeline((VkPipeline)2); cache.bindTextures((VkDescriptorSet)2);
    cache.setViewport(viewport); cache.setScissor(scissor);
    assert(pipelines == 3 && textures == 3 && viewports == 3 && scissors == 3);
    gpu::VkDrawState nextFrame(VK_NULL_HANDLE, VK_NULL_HANDLE);
    nextFrame.bindPipeline((VkPipeline)2); nextFrame.bindTextures((VkDescriptorSet)2);
    assert(pipelines == 4 && textures == 4);
    std::puts("PASS: unchanged draws skip driver state writes, changed state/copy boundaries/new frames rebind");
}
