#include "device.h"

#include <im3e/devices/devices.h>

using namespace im3e;
using namespace std;

auto im3e::createDevice() -> shared_ptr<IDevice>
{
    return make_shared<Device>();
}