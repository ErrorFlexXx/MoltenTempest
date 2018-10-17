#pragma once

#include <Tempest/AbstractGraphicsApi>
#include <vulkan/vulkan.hpp>

#include "vallocator.h"
#include "vulkanapi.h"

namespace Tempest {
namespace Detail{

class VSwapchain;
class VCommandBuffer;

class VFence;
class VSemaphore;

class VDevice : public AbstractGraphicsApi::Device {
  public:
    struct QueueFamilyIndices {
      uint32_t graphicsFamily=uint32_t(-1);
      uint32_t presentFamily =uint32_t(-1);

      bool isComplete() {
        return graphicsFamily!=std::numeric_limits<uint32_t>::max() &&
               presentFamily !=std::numeric_limits<uint32_t>::max();
        }
      };

    struct SwapChainSupportDetails {
      VkSurfaceCapabilitiesKHR        capabilities={};
      std::vector<VkSurfaceFormatKHR> formats;
      std::vector<VkPresentModeKHR>   presentModes;
      };

    VDevice(VulkanApi& api,void* hwnd);
    ~VDevice();

    VkSurfaceKHR            surface       =VK_NULL_HANDLE;
    VkPhysicalDevice        physicalDevice=nullptr;
    VkDevice                device        =nullptr;

    VkQueue                 graphicsQueue =nullptr;
    VkQueue                 presentQueue  =nullptr;

    VAllocator              allocator;

    VkResult                nextImg(VSwapchain& sw,uint32_t& imageId,VSemaphore& onReady);
    VkResult                present(VSwapchain& sw,const VSemaphore *wait,size_t wSize,uint32_t imageId);

    void                    draw(VCommandBuffer& cmd, VSemaphore& wait, VSemaphore& onReady, VFence& onReadyCpu);

    SwapChainSupportDetails querySwapChainSupport() { return querySwapChainSupport(physicalDevice); }
    QueueFamilyIndices      findQueueFamilies    () { return findQueueFamilies(physicalDevice);     }
    uint32_t                memoryTypeIndex(uint32_t typeBits) const;

  private:
    VkInstance             instance;
    VkPhysicalDeviceMemoryProperties memoryProperties;

    void                    pickPhysicalDevice();
    bool                    isDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices      findQueueFamilies(VkPhysicalDevice device);
    bool                    checkDeviceExtensionSupport(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    void createSurface(VulkanApi &api,void* hwnd);
    void createLogicalDevice(VulkanApi &api);
  };

}}