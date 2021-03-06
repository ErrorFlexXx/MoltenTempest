#pragma once

#include <Tempest/AbstractGraphicsApi>
#include <d3d12.h>

namespace Tempest {
namespace Detail {

class DxDevice;
class DxBuffer;
class DxTexture;

class DxAllocator {
  public:
    DxAllocator();

    void setDevice(DxDevice& device);

    DxBuffer  alloc(const void *mem,  size_t size, MemUsage usage, BufferFlags bufFlg);
    //DxTexture alloc(const Pixmap &pm, uint32_t mip, VkFormat format);
    //DxTexture alloc(const uint32_t w, const uint32_t h, const uint32_t mip, TextureFormat frm);
    void     free(DxBuffer&  buf);
    //void     free(DTexture& buf);

  private:
    ID3D12Device* device=nullptr;
  };

}
}

