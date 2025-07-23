#include "gdal_instance.h"

#include <gdal.h>

#include <mutex>

using namespace im3e;
using namespace std;

namespace {

class GdalInstance : public IGdalInstance
{
public:
    GdalInstance(const ILogger& rLogger)
      : m_pLogger(rLogger.createChild("GDAL"))
    {
        GDALAllRegister();
        m_pLogger->debug("Successfully initialized");
    }

    ~GdalInstance() override { m_pLogger->debug("Successfully deinitialized"); }

private:
    unique_ptr<ILogger> m_pLogger;
};

mutex g_mutex;
weak_ptr<GdalInstance> g_pInstance;

}  // namespace

auto im3e::getGdalInstance(const ILogger& rLogger) -> shared_ptr<IGdalInstance>
{
    lock_guard lock(g_mutex);

    if (auto pInstance = g_pInstance.lock())
    {
        return pInstance;
    }

    auto pInstance = make_shared<GdalInstance>(rLogger);
    g_pInstance = pInstance;
    return pInstance;
}