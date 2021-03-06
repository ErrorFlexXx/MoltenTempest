#if defined(TEMPEST_BUILD_DIRECTX12)

#include "dxbuffer.h"
#include "dxcommandbuffer.h"
#include "dxdevice.h"
#include "dxframebuffer.h"
#include "dxpipeline.h"
#include "dxswapchain.h"

#include "guid.h"

#include <cassert>

using namespace Tempest;
using namespace Tempest::Detail;

struct DxCommandBuffer::ImgState {
  ID3D12Resource*       img=nullptr;
  D3D12_RESOURCE_STATES lay;
  D3D12_RESOURCE_STATES last;
  bool                  outdated;
  };

DxCommandBuffer::DxCommandBuffer(DxDevice& d)
  : dev(d) {
  dxAssert(d.device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, dev.cmdMain.get(), nullptr,
                                       uuid<ID3D12GraphicsCommandList>(), reinterpret_cast<void**>(&impl)));
  impl->Close();
  }

void DxCommandBuffer::begin() {
  dxAssert(impl->Reset(dev.cmdMain.get(),nullptr));
  recording = true;
  }

void DxCommandBuffer::end() {
  for(auto& i:imgState)
    i.outdated = true;
  flushLayout();
  imgState.clear();

  dxAssert(impl->Close());
  recording = false;
  }

bool DxCommandBuffer::isRecording() const {
  return recording;
  }

void DxCommandBuffer::beginRenderPass(AbstractGraphicsApi::Fbo*  f,
                                      AbstractGraphicsApi::Pass* /*p*/,
                                      uint32_t w, uint32_t h) {
  auto& fbo = *reinterpret_cast<DxFramebuffer*>(f);

  for(auto& i:imgState)
    i.outdated = true;

  for(size_t i=0;i<fbo.viewsCount;++i) {
    D3D12_RESOURCE_STATES lay = D3D12_RESOURCE_STATE_RENDER_TARGET;
    setLayout(fbo.views[i].get(),lay);
    }

  flushLayout();

  auto  desc = fbo.rtvHeap->GetCPUDescriptorHandleForHeapStart();
  impl->OMSetRenderTargets(fbo.viewsCount,&desc,TRUE,
                           nullptr);
  const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
  impl->ClearRenderTargetView(desc, clearColor, 0, nullptr);

  D3D12_VIEWPORT vp={};
  vp.TopLeftX = float(0.f);
  vp.TopLeftY = float(0.f);
  vp.Width    = float(w);
  vp.Height   = float(h);
  vp.MinDepth = 0.f;
  vp.MaxDepth = 1.f;
  impl->RSSetViewports   (1, &vp);

  D3D12_RECT sr={};
  sr.left   = 0;
  sr.top    = 0;
  sr.right  = LONG(w);
  sr.bottom = LONG(h);
  impl->RSSetScissorRects(1, &sr);
  }

void DxCommandBuffer::endRenderPass() {
  }

void Tempest::Detail::DxCommandBuffer::setPipeline(Tempest::AbstractGraphicsApi::Pipeline& p,
                                                   uint32_t w, uint32_t h) {
  DxPipeline& px = reinterpret_cast<DxPipeline&>(p);
  vboStride = px.stride;

  impl->SetPipelineState(px.impl.get());
  impl->SetGraphicsRootSignature(px.emptySign.get());
  impl->IASetPrimitiveTopology(px.topology);
  }

void DxCommandBuffer::setViewport(const Rect& r) {
  D3D12_VIEWPORT vp={};
  vp.TopLeftX = float(r.x);
  vp.TopLeftY = float(r.y);
  vp.Width    = float(r.w);
  vp.Height   = float(r.h);
  vp.MinDepth = 0.f;
  vp.MaxDepth = 1.f;

  impl->RSSetViewports(1,&vp);
  }

void DxCommandBuffer::setUniforms(AbstractGraphicsApi::Pipeline& p, AbstractGraphicsApi::Desc& u, size_t offc, const uint32_t* offv) {
  DxUniformsLay& lay = reinterpret_cast<DxUniformsLay&>(p);
  assert(0);
  }

void DxCommandBuffer::exec(const CommandBundle& buf) {
  assert(0);
  //impl->ExecuteBundle(); //BUNDLE!
  }

void DxCommandBuffer::changeLayout(AbstractGraphicsApi::Swapchain& s, uint32_t id, TextureFormat /*frm*/,
                                   TextureLayout prev, TextureLayout next) {
  DxSwapchain&    sw  = reinterpret_cast<DxSwapchain&>(s);
  ID3D12Resource* res = sw.views[id].get();
  static const D3D12_RESOURCE_STATES layouts[] = {
    D3D12_RESOURCE_STATE_COMMON,
    D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
    D3D12_RESOURCE_STATE_RENDER_TARGET,
    D3D12_RESOURCE_STATE_DEPTH_WRITE,
    D3D12_RESOURCE_STATE_PRESENT,
    };
  implChangeLayout(res,layouts[int(prev)],layouts[int(next)]);
  }

void DxCommandBuffer::changeLayout(AbstractGraphicsApi::Texture& t, TextureFormat frm, TextureLayout prev, TextureLayout next) {
  DxTexture& tex = reinterpret_cast<DxTexture&>(t);
  /*
  D3D12_RESOURCE_BARRIER barrier = {};
  barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  barrier.Transition.pResource   = tex.;
  barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
  barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PRESENT;
  barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  */
  assert(0);
  }

void DxCommandBuffer::setVbo(const AbstractGraphicsApi::Buffer& b) {
  const DxBuffer& bx = reinterpret_cast<const DxBuffer&>(b);

  D3D12_VERTEX_BUFFER_VIEW view;
  view.BufferLocation = bx.impl.get()->GetGPUVirtualAddress();
  view.SizeInBytes    = bx.size;
  view.StrideInBytes  = vboStride;
  impl->IASetVertexBuffers(0,1,&view);
  }

void DxCommandBuffer::setIbo(const AbstractGraphicsApi::Buffer* b, IndexClass cls) {
  assert(0);
  }

void DxCommandBuffer::draw(size_t offset, size_t vertexCount) {
  impl->DrawInstanced(UINT(vertexCount),1,UINT(offset),0);
  }

void DxCommandBuffer::drawIndexed(size_t ioffset, size_t isize, size_t voffset) {
  impl->DrawIndexedInstanced(UINT(isize),1,UINT(ioffset),INT(voffset),0);
  }

void DxCommandBuffer::setLayout(ID3D12Resource* res, D3D12_RESOURCE_STATES lay) {
  ImgState* img;
  /*
  if(a.tex!=nullptr) {
    img = &findImg(a.tex->impl,D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    } else {
    img = &findImg(a.sw->images[a.id],D3D12_RESOURCE_STATE_PRESENT);
    }
  */
  img = &findImg(res,D3D12_RESOURCE_STATE_PRESENT);
  implChangeLayout(img->img,img->lay,lay);
  img->outdated = false;
  img->lay      = lay;
  }

void DxCommandBuffer::implChangeLayout(ID3D12Resource* res, D3D12_RESOURCE_STATES prev, D3D12_RESOURCE_STATES lay) {
  D3D12_RESOURCE_BARRIER barrier = {};
  barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  barrier.Transition.pResource   = res;
  barrier.Transition.StateBefore = prev;
  barrier.Transition.StateAfter  = lay;
  barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

  impl->ResourceBarrier(1,&barrier);
  }

DxCommandBuffer::ImgState& DxCommandBuffer::findImg(ID3D12Resource* img, D3D12_RESOURCE_STATES last) {
  for(auto& i:imgState) {
    if(i.img==img)
      return i;
    }
  ImgState s={};
  s.img     = img;
  s.lay     = D3D12_RESOURCE_STATE_COMMON;
  s.last    = last;
  imgState.push_back(s);
  return imgState.back();
  }

void DxCommandBuffer::flushLayout() {
  for(auto& i:imgState) {
    if(!i.outdated)
      continue;
    //if(Detail::nativeIsDepthFormat(i.frm))
    //  continue; // no readable depth for now
    implChangeLayout(i.img,i.lay,i.last);
    i.lay = i.last;
    }
  }

#endif
