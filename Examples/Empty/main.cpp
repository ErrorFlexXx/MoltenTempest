#include <Tempest/Application>
#include <Tempest/VulkanApi>

#include <Tempest/Device>

#include "game.h"

std::unique_ptr<Tempest::AbstractGraphicsApi> mkApi(const char* av) {
  //if(std::strcmp(av,"dx12"))
  //  return std::unique_ptr<Tempest::AbstractGraphicsApi>(new Tempest::Directx12Api{Tempest::ApiFlags::Validation});
  return std::unique_ptr<Tempest::AbstractGraphicsApi>(new Tempest::VulkanApi{Tempest::ApiFlags::Validation});
  }

int main(int argc,const char** argv) {
  Tempest::Application app;
  auto api = mkApi(argc>1 ? argv[1] : "");

  Tempest::Device device{*api};
  Game            wx(device);

  return app.exec();
  }
