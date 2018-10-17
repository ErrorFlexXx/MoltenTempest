#include "vfence.h"

#include "vdevice.h"

using namespace Tempest::Detail;

VFence::VFence(VDevice &device)
  :device(device.device) {
  VkFenceCreateInfo fenceInfo = {};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  if(vkCreateFence(device.device,&fenceInfo,nullptr,&impl) != VK_SUCCESS)
    throw std::runtime_error("failed to create synchronization objects for a frame!");
  }

VFence::VFence(VFence &&other) {
  std::swap(device,other.device);
  std::swap(impl,other.impl);
  }

VFence::~VFence() {
  if(device==nullptr)
    return;
  vkDeviceWaitIdle(device);
  vkDestroyFence(device,impl,nullptr);
  }

void VFence::operator=(VFence &&other) {
  std::swap(device,other.device);
  std::swap(impl,other.impl);
  }

void VFence::wait() {
  vkWaitForFences(device,1,&impl,VK_TRUE,std::numeric_limits<uint64_t>::max());
  }

void VFence::reset() {
  vkResetFences(device,1,&impl);
  }