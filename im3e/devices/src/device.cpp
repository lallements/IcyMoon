#include "device.h"

#include <im3e/devices/devices.h>
#include <im3e/utils/throw_utils.h>

using namespace im3e;
using namespace std;

Device::Device() {}

auto im3e::createDevice() -> shared_ptr<IDevice>
{
    return make_shared<Device>();
}