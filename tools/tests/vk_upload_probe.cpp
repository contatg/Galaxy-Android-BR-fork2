// Host-side lifecycle checks against the real renderer, using a fake Vulkan
// device. These check synchronization/ownership, not GPU output or frame rate.
#include <bits/stdc++.h>
#define private public
#include "gx/vk_renderer.h"
#undef private

using namespace gpu;
static uintptr_t nextHandle = 1;
template<class T> T handle() { return (T)nextHandle++; }
static std::unordered_map<VkBuffer, size_t> buffers;
static std::unordered_map<VkDeviceMemory, std::vector<unsigned char>> memory;
static unsigned maps = 0, unmaps = 0, submits = 0, waits = 0, copies = 0;
static bool failMap = false, failSubmit = false;
static bool inFlight = false;
extern "C" {
int __android_log_print(int, const char*, const char*, ...) { return 0; }
void port_log(const char*, ...) {}
PFN_vkVoidFunction vkGetDeviceProcAddr(VkDevice, const char*) { return nullptr; }
void vkGetPhysicalDeviceMemoryProperties(VkPhysicalDevice, VkPhysicalDeviceMemoryProperties* p) {
    *p = {}; p->memoryTypeCount = 1;
    p->memoryTypes[0].propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
}
VkResult vkCreateBuffer(VkDevice, const VkBufferCreateInfo* i, const VkAllocationCallbacks*, VkBuffer* b) {
    *b = handle<VkBuffer>(); buffers[*b] = i->size; return VK_SUCCESS;
}
void vkGetBufferMemoryRequirements(VkDevice, VkBuffer b, VkMemoryRequirements* r) { *r = {buffers.at(b), 1, 1}; }
VkResult vkAllocateMemory(VkDevice, const VkMemoryAllocateInfo* i, const VkAllocationCallbacks*, VkDeviceMemory* m) {
    *m = handle<VkDeviceMemory>(); memory[*m].resize(i->allocationSize); return VK_SUCCESS;
}
VkResult vkBindBufferMemory(VkDevice, VkBuffer, VkDeviceMemory, VkDeviceSize) { return VK_SUCCESS; }
void vkDestroyBuffer(VkDevice, VkBuffer b, const VkAllocationCallbacks*) { assert(!inFlight); assert(buffers.erase(b) == 1); }
void vkFreeMemory(VkDevice, VkDeviceMemory m, const VkAllocationCallbacks*) { assert(!inFlight); assert(memory.erase(m) == 1); }
VkResult vkMapMemory(VkDevice, VkDeviceMemory m, VkDeviceSize, VkDeviceSize, VkMemoryMapFlags, void** p) {
    if (failMap) return VK_ERROR_MEMORY_MAP_FAILED;
    maps++; *p = memory.at(m).data(); return VK_SUCCESS;
}
void vkUnmapMemory(VkDevice, VkDeviceMemory) { unmaps++; }
VkResult vkCreateImage(VkDevice, const VkImageCreateInfo*, const VkAllocationCallbacks*, VkImage* i) { *i = handle<VkImage>(); return VK_SUCCESS; }
void vkGetImageMemoryRequirements(VkDevice, VkImage, VkMemoryRequirements* r) { *r = {64, 1, 1}; }
VkResult vkBindImageMemory(VkDevice, VkImage, VkDeviceMemory, VkDeviceSize) { return VK_SUCCESS; }
VkResult vkCreateImageView(VkDevice, const VkImageViewCreateInfo*, const VkAllocationCallbacks*, VkImageView* v) { *v = handle<VkImageView>(); return VK_SUCCESS; }
void vkDestroyImage(VkDevice, VkImage, const VkAllocationCallbacks*) { assert(!inFlight); }
void vkDestroyImageView(VkDevice, VkImageView, const VkAllocationCallbacks*) { assert(!inFlight); }
VkResult vkResetCommandBuffer(VkCommandBuffer, VkCommandBufferResetFlags) { assert(!inFlight); return VK_SUCCESS; }
VkResult vkBeginCommandBuffer(VkCommandBuffer, const VkCommandBufferBeginInfo*) { return VK_SUCCESS; }
VkResult vkEndCommandBuffer(VkCommandBuffer) { return VK_SUCCESS; }
void vkCmdPipelineBarrier(VkCommandBuffer, VkPipelineStageFlags, VkPipelineStageFlags, VkDependencyFlags, uint32_t, const VkMemoryBarrier*, uint32_t, const VkBufferMemoryBarrier*, uint32_t, const VkImageMemoryBarrier*) {}
void vkCmdCopyBufferToImage(VkCommandBuffer, VkBuffer b, VkImage, VkImageLayout, uint32_t count, const VkBufferImageCopy* regions) {
    copies++; assert(count > 0);
    for (uint32_t i = 0; i < count; ++i)
        assert(regions[i].bufferOffset + regions[i].imageExtent.width * regions[i].imageExtent.height * 4 <= buffers.at(b));
}
VkResult vkQueueSubmit(VkQueue, uint32_t, const VkSubmitInfo*, VkFence) {
    submits++; if (failSubmit) return VK_ERROR_DEVICE_LOST;
    inFlight = true; return VK_SUCCESS;
}
VkResult vkQueueWaitIdle(VkQueue) { waits++; inFlight = false; return VK_SUCCESS; }
}
extern "C" {
void vkDestroyPipeline(VkDevice, VkPipeline, const VkAllocationCallbacks*) {}
void vkDestroyPipelineLayout(VkDevice, VkPipelineLayout, const VkAllocationCallbacks*) {}
void vkDestroyDescriptorSetLayout(VkDevice, VkDescriptorSetLayout, const VkAllocationCallbacks*) {}
void vkDestroySampler(VkDevice, VkSampler, const VkAllocationCallbacks*) {}
void vkDestroyFence(VkDevice, VkFence, const VkAllocationCallbacks*) {}
void vkDestroySemaphore(VkDevice, VkSemaphore, const VkAllocationCallbacks*) {}
void vkDestroyPipelineCache(VkDevice, VkPipelineCache, const VkAllocationCallbacks*) {}
void vkDestroyDescriptorPool(VkDevice, VkDescriptorPool, const VkAllocationCallbacks*) {}
void vkDestroyCommandPool(VkDevice, VkCommandPool, const VkAllocationCallbacks*) {}
void vkDestroyFramebuffer(VkDevice, VkFramebuffer, const VkAllocationCallbacks*) {}
void vkDestroyRenderPass(VkDevice, VkRenderPass, const VkAllocationCallbacks*) {}
void vkDestroySwapchainKHR(VkDevice, VkSwapchainKHR, const VkAllocationCallbacks*) {}
void vkDestroyDevice(VkDevice, const VkAllocationCallbacks*) {}
void vkDestroySurfaceKHR(VkInstance, VkSurfaceKHR, const VkAllocationCallbacks*) {}
void vkDestroyInstance(VkInstance, const VkAllocationCallbacks*) {}
VkResult vkDeviceWaitIdle(VkDevice) { return VK_SUCCESS; }
VkResult vkGetPipelineCacheData(VkDevice, VkPipelineCache, size_t* size, void*) { *size = 0; return VK_SUCCESS; }
}
namespace gpu { TexImage::~TexImage() {} }

static std::shared_ptr<TexImage> image(unsigned width, unsigned height) {
    auto p = std::make_shared<TexImage>();
    p->width = width; p->height = height; p->levels = 1; p->version = 1;
    p->rgba.resize(size_t(width) * height, 0x12345678); return p;
}
static void release(VkRenderer& r, const std::shared_ptr<TexImage>& p) {
    if (p->vkView) vkDestroyImageView(r.mDevice, (VkImageView)p->vkView, nullptr);
    if (p->vkImage) vkDestroyImage(r.mDevice, (VkImage)p->vkImage, nullptr);
    if (p->vkMemory) vkFreeMemory(r.mDevice, (VkDeviceMemory)p->vkMemory, nullptr);
    p->vkView = p->vkImage = p->vkMemory = 0;
}
int main() {
    // Do not initialize a real Android device or call its teardown.
    auto& r = VkRenderer::get();
    VkBuffer b{}; VkDeviceMemory mem{}; size_t cap = 0; void* mapped = nullptr;
    uint32_t data[] = {1, 2, 3, 4};
    assert(r.uploadDynamicBuffer(b, mem, cap, mapped, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, data, sizeof(data)));
    auto old = b; auto oldMem = mem; auto oldMapped = mapped;
    assert(maps == 1 && unmaps == 0);
    data[0] = 9;
    assert(r.uploadDynamicBuffer(b, mem, cap, mapped, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, data, sizeof(data)));
    assert(maps == 1 && unmaps == 0 && ((uint32_t*)mapped)[0] == 9);
    failMap = true;
    assert(!r.uploadDynamicBuffer(b, mem, cap, mapped, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, nullptr, cap + 1));
    assert(b == old && mem == oldMem && mapped == oldMapped);
    failMap = false;
    assert(r.uploadDynamicBuffer(b, mem, cap, mapped, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, nullptr, cap + 1));
    assert(maps == 2 && unmaps == 1 && b != old);
    vkUnmapMemory(r.mDevice, mem); vkDestroyBuffer(r.mDevice, b, nullptr); vkFreeMemory(r.mDevice, mem, nullptr);
    assert(buffers.empty() && memory.empty());

    std::vector<std::shared_ptr<TexImage>> textures;
    for (int i = 0; i < 100; ++i) textures.push_back(image(4, 4));
    assert(r.uploadTextures(textures)); assert(submits == 1 && waits == 1 && copies == 100);
    assert(r.uploadTextures(textures)); assert(submits == 1); // unchanged cache hit
    textures[0]->version++;
    assert(r.uploadTextures(textures)); assert(submits == 2 && copies == 101);
    for (auto& p : textures) release(r, p);
    textures.clear();
    assert(buffers.empty() && memory.empty());
    for (int i = 0; i < 3; ++i) textures.push_back(image(1024, 1024));
    assert(r.uploadTextures(textures)); assert(submits == 4 && waits == 4); // 8 MiB + 4 MiB
    for (auto& p : textures) release(r, p);
    textures = {image(4, 4)};
    failMap = true;
    assert(!r.uploadTextures(textures)); assert(textures[0]->vkImage == 0);
    failMap = false;
    assert(buffers.empty() && memory.empty());
    failSubmit = true;
    assert(!r.uploadTextures(textures)); assert(r.hasFailed());
    for (auto& p : textures) release(r, p);
    assert(buffers.empty() && memory.empty());
    std::puts("PASS: persistent mapping, growth rollback, texture batching, budget, cache and failure cleanup");
}
