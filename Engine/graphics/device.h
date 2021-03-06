#pragma once

#include <Tempest/AbstractGraphicsApi>
#include <Tempest/CommandBuffer>
#include <Tempest/RenderPass>
#include <Tempest/FrameBuffer>
#include <Tempest/RenderPipeline>
#include <Tempest/Shader>
#include <Tempest/Attachment>
#include <Tempest/Texture2d>
#include <Tempest/Uniforms>
#include <Tempest/VertexBuffer>
#include <Tempest/IndexBuffer>
#include <Tempest/Builtin>
#include <Tempest/Swapchain>
#include <Tempest/UniformBuffer>
#include <Tempest/Except>

#include "videobuffer.h"

#include <memory>
#include <vector>

namespace Tempest {

class Fence;
class Semaphore;

class CommandPool;
class RFile;

class VideoBuffer;
class Pixmap;

class Uniforms;
class UniformsLayout;

class Color;
class RenderState;

class Device {
  public:
    using Props=AbstractGraphicsApi::Props;

    Device(AbstractGraphicsApi& api, uint8_t maxFramesInFlight=2);
    Device(AbstractGraphicsApi& api, const char* name, uint8_t maxFramesInFlight=2);
    Device(const Device&)=delete;
    virtual ~Device();

    uint8_t              maxFramesInFlight() const;
    void                 waitIdle();

    void                 submit(const PrimaryCommandBuffer&  cmd,const Semaphore& wait);
    void                 submit(const PrimaryCommandBuffer&  cmd,Fence& fdone);
    void                 submit(const PrimaryCommandBuffer&  cmd,const Semaphore& wait,Semaphore& done,Fence& fdone);
    void                 submit(const PrimaryCommandBuffer *cmd[], size_t count,
                                const Semaphore* wait[], size_t waitCnt,
                                Semaphore* done[], size_t doneCnt,
                                Fence* fdone);
    void                 present(Swapchain& sw, uint32_t img, const Semaphore& wait);

    Swapchain            swapchain(SystemApi::Window* w) const;

    Shader               loadShader(RFile&          file);
    Shader               loadShader(const char*     filename);
    Shader               loadShader(const char16_t* filename);
    Shader               shader    (const void* source, const size_t length);

    const Props&         properties() const;

    template<class T>
    VertexBuffer<T>      vbo(const T* arr,size_t arrSize);

    template<class T>
    VertexBuffer<T>      vbo(const std::vector<T>& arr){
      return vbo(arr.data(),arr.size());
      }

    template<class T>
    VertexBufferDyn<T>   vboDyn(const T* arr,size_t arrSize);

    template<class T>
    VertexBufferDyn<T>   vboDyn(const std::vector<T>& arr){
      return vboDyn(arr.data(),arr.size());
      }

    template<class T>
    IndexBuffer<T>       ibo(const T* arr,size_t arrSize);

    template<class T>
    IndexBuffer<T>       ibo(const std::vector<T>& arr){
      return ibo(arr.data(),arr.size());
      }

    template<class T>
    UniformBuffer<T>     ubo(const T* data, size_t size);

    template<class T>
    UniformBuffer<T>     ubo(const T& data);

    Uniforms             uniforms(const UniformsLayout &owner);

    Attachment           attachment (TextureFormat frm, const uint32_t w, const uint32_t h, const bool mips = false);
    Texture2d            loadTexture(const Pixmap& pm,bool mips=true);
    Pixmap               readPixels (const Texture2d&  t);
    Pixmap               readPixels (const Attachment& t);

    FrameBuffer          frameBuffer(Attachment& out);
    FrameBuffer          frameBuffer(Attachment& out, Attachment& zbuf);

    RenderPass           pass(const FboMode& color);
    RenderPass           pass(const FboMode& color,const FboMode& depth);

    template<class Vertex>
    RenderPipeline       pipeline(Topology tp,const RenderState& st,
                                  const UniformsLayout& ulay,const Shader &vs,const Shader &fs);

    Fence                fence();
    Semaphore            semaphore();

    PrimaryCommandBuffer commandBuffer();
    CommandBuffer        commandSecondaryBuffer(const FrameBufferLayout &lay);
    CommandBuffer        commandSecondaryBuffer(const FrameBuffer &fbo);

    const Builtin&       builtin() const;
    const char*          renderer() const;

  private:
    struct Impl {
      Impl(AbstractGraphicsApi& api, const char* name, uint8_t maxFramesInFlight);
      ~Impl();

      AbstractGraphicsApi&            api;
      AbstractGraphicsApi::Device*    dev=nullptr;
      uint8_t                         maxFramesInFlight=1;
      };

    AbstractGraphicsApi&            api;
    Impl                            impl;
    AbstractGraphicsApi::Device*    dev=nullptr;
    Props                           devProps;
    Tempest::Builtin                builtins;

    VideoBuffer createVideoBuffer(const void* data, size_t count, size_t size, size_t alignedSz, MemUsage usage, BufferFlags flg);
    RenderPipeline
                implPipeline(const RenderState &st, const UniformsLayout& ulay,
                             const Shader &vs, const Shader &fs,
                             const Decl::ComponentType *decl, size_t declSize,
                             size_t stride, Topology tp);
    void        implSubmit(const Tempest::PrimaryCommandBuffer *cmd[], AbstractGraphicsApi::CommandBuffer* hcmd[],  size_t count,
                           const Semaphore* wait[], AbstractGraphicsApi::Semaphore*     hwait[], size_t waitCnt,
                           Semaphore*       done[], AbstractGraphicsApi::Semaphore*     hdone[], size_t doneCnt,
                           AbstractGraphicsApi::Fence*         fdone);
    void        createInstance(const UniformsLayout &ulay);

    static TextureFormat formatOf(const Attachment& a);

  friend class RenderPipeline;
  friend class RenderPass;
  friend class FrameBuffer;
  friend class Painter;
  friend class Shader;
  friend class CommandPool;
  friend class CommandBuffer;
  friend class VideoBuffer;
  friend class Uniforms;

  template<class T>
  friend class VertexBuffer;
  template<class T>
  friend class VertexBufferDyn;

  friend class Texture2d;
  };

template<class T>
inline VertexBuffer<T> Device::vbo(const T* arr, size_t arrSize) {
  if(arrSize==0)
    return VertexBuffer<T>();
  VideoBuffer     data=createVideoBuffer(arr,arrSize,sizeof(T),sizeof(T),MemUsage::VertexBuffer,BufferFlags::Static);
  VertexBuffer<T> vbo(std::move(data),arrSize);
  return vbo;
  }

template<class T>
inline VertexBufferDyn<T> Device::vboDyn(const T *arr, size_t arrSize) {
  if(arrSize==0)
    return VertexBufferDyn<T>();
  VideoBuffer        data=createVideoBuffer(arr,arrSize,sizeof(T),sizeof(T),MemUsage::VertexBuffer,BufferFlags::Dynamic);
  VertexBufferDyn<T> vbo(std::move(data),arrSize);
  return vbo;
  }

template<class T>
inline IndexBuffer<T> Device::ibo(const T* arr, size_t arrSize) {
  if(arrSize==0)
    return IndexBuffer<T>();
  VideoBuffer     data=createVideoBuffer(arr,arrSize,sizeof(T),sizeof(T),MemUsage::IndexBuffer,BufferFlags::Static);
  IndexBuffer<T>  ibo(std::move(data),arrSize);
  return ibo;
  }

template<class T>
inline UniformBuffer<T> Device::ubo(const T *mem, size_t size) {
  if(size==0)
    return UniformBuffer<T>();
  const size_t align   = devProps.ubo.offsetAlign;
  const size_t eltSize = ((sizeof(T)+align-1)/align)*align;

  if(sizeof(T)>devProps.ubo.maxRange)
    throw std::system_error(Tempest::GraphicsErrc::TooLardgeUbo);
  VideoBuffer      data=createVideoBuffer(mem,size,sizeof(T),eltSize,MemUsage::UniformBit,BufferFlags::Dynamic);
  UniformBuffer<T> ubo(std::move(data),eltSize);
  return ubo;
  }

template<class T>
inline UniformBuffer<T> Device::ubo(const T& mem) {
  return ubo(&mem,1);
  }

template<class Vertex>
RenderPipeline Device::pipeline(Topology tp, const RenderState &st,
                                const UniformsLayout& ulay, const Shader &vs, const Shader &fs) {
  static const auto decl=Tempest::vertexBufferDecl<Vertex>();
  return implPipeline(st,ulay,vs,fs,decl.data.data(),decl.data.size(),sizeof(Vertex),tp);
  }

}

