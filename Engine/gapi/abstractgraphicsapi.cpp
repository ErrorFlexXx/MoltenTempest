#include "abstractgraphicsapi.h"

using namespace Tempest;

bool AbstractGraphicsApi::Props::hasSamplerFormat(TextureFormat f) const {
  uint64_t  m = uint64_t(1) << uint64_t(f);
  return smpFormat&m;
  }

bool AbstractGraphicsApi::Props::hasAttachFormat(TextureFormat f) const {
  uint64_t  m = uint64_t(1) << uint64_t(f);
  return attFormat&m;
  }

bool AbstractGraphicsApi::Props::hasDepthFormat(TextureFormat f) const {
  uint64_t  m = uint64_t(1) << uint64_t(f);
  return dattFormat&m;
  }
