#pragma once

#include <Tempest/AbstractGraphicsApi>
#include "vulkan_sdk.h"

#include "vuniformslay.h"

namespace Tempest {
namespace Detail {

class VUniformsLay;

class VDescriptorArray : public AbstractGraphicsApi::Desc {
  public:
    VkDevice              device=nullptr;
    Detail::DSharedPtr<VUniformsLay*> lay;
    VkDescriptorSet       desc=VK_NULL_HANDLE;

    VDescriptorArray(VkDevice device,const UniformsLayout& lay, VUniformsLay& vlay);
    ~VDescriptorArray() override;

    void                     set   (size_t id, AbstractGraphicsApi::Texture *tex) override;
    void                     set   (size_t id, AbstractGraphicsApi::Buffer* buf, size_t offset, size_t size, size_t align) override;

    size_t  multV[64]={};
    size_t* multiplier=nullptr;

  private:
    Detail::VUniformsLay::Pool* pool=nullptr;

    VkDescriptorPool         allocPool(const UniformsLayout& lay, size_t size);
    bool                     allocDescSet(VkDescriptorPool pool, VkDescriptorSetLayout lay);
    static void              addPoolSize(VkDescriptorPoolSize* p, size_t& sz, VkDescriptorType elt);
  };

}}
