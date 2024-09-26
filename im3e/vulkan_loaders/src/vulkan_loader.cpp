#include "vulkan_loader.h"

#if defined(__linux__)
#include "vulkan_library_loader_linux.h"
#else
static_assert(false, "Vulkan Loader does not support this platform at the moment");
#endif

using namespace im3e;
using namespace std;

#define LOAD_GLOBAL_FCT(fctName) .fctName = reinterpret_cast<PFN_##fctName>(m_vkGetInstanceProcAddr(nullptr, #fctName))
#define LOAD_INST_FCT(fctName) .fctName = reinterpret_cast<PFN_##fctName>(m_vkGetInstanceProcAddr(vkInstance, #fctName))
#define LOAD_DEVICE_FCT(fctName) .fctName = reinterpret_cast<PFN_##fctName>(m_vkGetDeviceProcAddr(vkDevice, #fctName))

VulkanLoader::VulkanLoader(VulkanLoaderConfig config, UniquePtrWithDeleter<void> pLibrary,
                           PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr, PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr)
  : m_config(move(config))
  , m_pLibrary(throwIfArgNull(move(pLibrary), "Vulkan loader requires a library pointer"))
  , m_vkGetInstanceProcAddr(throwIfArgNull(vkGetInstanceProcAddr, "Vulkan loader requires vkGetInstanceProcAddr"))
  , m_vkGetDeviceProcAddr(throwIfArgNull(vkGetDeviceProcAddr, "Vulkan loader requires vkGetDeviceProcAddr"))
{
}

auto VulkanLoader::loadGlobalFcts() -> VulkanGlobalFcts
{
    return VulkanGlobalFcts{
        LOAD_GLOBAL_FCT(vkEnumerateInstanceVersion),
        LOAD_GLOBAL_FCT(vkEnumerateInstanceExtensionProperties),
        LOAD_GLOBAL_FCT(vkEnumerateInstanceLayerProperties),
        LOAD_GLOBAL_FCT(vkCreateInstance),
    };
}
auto VulkanLoader::loadInstanceFcts(VkInstance vkInstance) -> VulkanInstanceFcts
{
    throwIfArgNull(vkInstance, "Cannot load Vulkan instance functions without an instance");

    VulkanInstanceFcts fcts{
        LOAD_INST_FCT(vkDestroyInstance),
        LOAD_INST_FCT(vkCreateDevice),
    };
    if (m_config.isDebugEnabled)
    {
        fcts LOAD_INST_FCT(vkCreateDebugUtilsMessengerEXT);
        fcts LOAD_INST_FCT(vkDestroyDebugUtilsMessengerEXT);
    }
    return fcts;
}
auto VulkanLoader::loadDeviceFcts(VkDevice vkDevice) -> VulkanDeviceFcts
{
    throwIfArgNull(vkDevice, "Cannot load Vulkan device functions without a device");

    return VulkanDeviceFcts{
        LOAD_DEVICE_FCT(vkDestroyDevice),
    };
}

auto im3e::createVulkanLoader(VulkanLoaderConfig config) -> unique_ptr<IVulkanLoader>
{
    vulkan_loader::VulkanLibraryLoader libraryLoader;
    return make_unique<VulkanLoader>(move(config), move(libraryLoader.pLibrary), libraryLoader.vkGetInstanceProcAddr,
                                     libraryLoader.vkGetDeviceProcAddr);
}