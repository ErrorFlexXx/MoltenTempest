#pragma once

#include "vimage.h"

#include <Tempest/AbstractGraphicsApi>
#include <vulkan/vulkan.hpp>

namespace Tempest {

class Semaphore;

namespace Detail {

class VDevice;

class VSwapchain : public AbstractGraphicsApi::Swapchain {
  public:
    VSwapchain()=default;
    VSwapchain(VDevice& device,uint32_t w,uint32_t h);
    VSwapchain(VSwapchain&& other);
    ~VSwapchain();

    void operator=(VSwapchain&& other);

    VkFormat                 format() const          { return swapChainImageFormat;   }
    uint32_t                 w()      const override { return swapChainExtent.width;  }
    uint32_t                 h()      const override { return swapChainExtent.height; }

    uint32_t                 imageCount() const override { return uint32_t(swapChainImageViews.size()); }
    uint32_t                 nextImage(AbstractGraphicsApi::Semaphore* onReady) override;
    AbstractGraphicsApi::Image* getImage (uint32_t id) override;

    VkSwapchainKHR           swapChain=VK_NULL_HANDLE;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VImage>      images;

  private:
    VkDevice                 device=nullptr;
    VkFormat                 swapChainImageFormat=VK_FORMAT_UNDEFINED;
    VkExtent2D               swapChainExtent={};

    std::vector<VkImage>     swapChainImages;

    VkSurfaceFormatKHR       chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR         chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D               chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, uint32_t w, uint32_t h);

    void                     createImageViews(VDevice &device);
  };

}}
