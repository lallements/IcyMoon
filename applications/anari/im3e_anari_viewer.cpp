#include "anari_dem_surface_generator.h"

#include <im3e/anari/anari.h>
#include <im3e/devices/devices.h>
#include <im3e/geo/geo.h>
#include <im3e/guis/guis.h>
#include <im3e/utils/core/throw_utils.h>
#include <im3e/utils/loggers.h>
#include <im3e/utils/properties/properties.h>
#include <im3e/utils/types.h>

#include <anari/anari.h>
#include <fmt/format.h>

#include <iostream>
#include <memory>
#include <stdexcept>

using namespace im3e;
using namespace std;

namespace {

/*
auto createWorld(const ILogger& rLogger, ANARIDevice anDevice)
{
    auto anWorld = anariNewWorld(anDevice);
    rLogger.debug("Created new world");
    auto pWorld = shared_ptr<anari::api::World>(anWorld, [anDevice, pLogger = &rLogger](auto* anWorld) {
        anariRelease(anDevice, anWorld);
        pLogger->debug("Destroyed world");
    });

    // Surfaces of world:
    {
        auto pDemSurface = AnariDemSurfaceGenerator::generate(rLogger, anDevice);
        auto pGroundSurface = createGroundSurface(rLogger, anDevice);

        vector<ANARISurface> surfaces{
            pDemSurface.get(),
            pGroundSurface.get(),
        };

        auto anArray = anariNewArray1D(anDevice, surfaces.data(), nullptr, nullptr, ANARI_SURFACE, surfaces.size());
        anariCommitParameters(anDevice, anArray);
        anariSetParameter(anDevice, anWorld, "surface", ANARI_ARRAY1D, &anArray);
        anariRelease(anDevice, anArray);
        rLogger.debug("Added surface to world");
    }

    anariCommitParameters(anDevice, anWorld);
    rLogger.debug("Created world");

    // Print out world bounds
    array<float, 6U> worldBounds;
    if (anariGetProperty(anDevice, anWorld, "bounds", ANARI_FLOAT32_BOX3, worldBounds.data(), sizeof(worldBounds),
                         ANARI_WAIT))
    {
        rLogger.info(fmt::format("World bounds: ({}, {}, {}), ({}, {}, {})", worldBounds[0], worldBounds[1],
                                 worldBounds[2], worldBounds[3], worldBounds[4], worldBounds[5]));
    }
    else
    {
        rLogger.warning("world bounds not returned");
    }

    return pWorld;
}*/

}  // namespace

int main()
{
    auto pLogger = createTerminalLogger();
    pLogger->setLevelFilter(LogLevel::Verbose);
    pLogger->debug("ANARI App");

    auto pApp = createGlfwWindowApplication(*pLogger, WindowApplicationConfig{
                                                          .name = "ANARI Viewer",
                                                          .isDebugEnabled = false,
                                                      });
    auto pDevice = pApp->getDevice();
    auto pAnEngine = createAnariEngine(*pLogger, pDevice);
    auto pFramePipeline = pAnEngine->createFramePipeline();

    auto pWorld = pFramePipeline->getWorld();
    auto pPlane = pWorld->addPlane("Ground");

    auto pHeightMap = loadHeightMapFromFile(*pLogger,
                                            HeightMapFileConfig{
                                                .path = "/mnt/data/dev/assets/lidar_bc/bc_092g064_xli1m_utm10_2020.tif",
                                                .readOnly = true,
                                            });
    auto pHeightField = pWorld->addHeightField(std::move(pHeightMap));

    auto pGuiWorkspace = createImguiWorkspace("ANARI");

    // Properties Panel
    {
        vector<shared_ptr<IProperty>> pProperties{
            pFramePipeline->createRendererProperties(),
            pPlane->getProperties(),
            pHeightField->getProperties(),
        };

        static constexpr PropertyValueTConfig<uint32_t> LevelOfDetails{
            .name = "Level of Details",
            .description = "Index of the current level of details of the terrain being rendered",
            .defaultValue = 0U,
        };

        auto pLodProperty = make_shared<PropertyValueT<LevelOfDetails>>();
        auto pTestParameterGroup = createPropertyGroup("Test Parameters", {pLodProperty});
        pProperties.emplace_back(pTestParameterGroup);

        auto pPropertyGroup = createPropertyGroup("Parameters", pProperties);
        auto pParametersPanel = createImguiPropertyPanel(pPropertyGroup);
        pGuiWorkspace->addPanel(IGuiWorkspace::Location::Left, pParametersPanel);
    }

    // Render Panel
    {
        auto pCameraListener = pFramePipeline->getCameraListener();
        auto pRenderPanel = createImguiRenderPanel("Renderer", std::move(pFramePipeline), std::move(pCameraListener));
        pGuiWorkspace->addPanel(IGuiWorkspace::Location::Center, std::move(pRenderPanel));
    }

    // Stats Panel
    {
        auto pStatsPanel = createImguiStatsPanel("Stats", pDevice->getStatsProvider());
        pGuiWorkspace->addPanel(IGuiWorkspace::Location::Bottom, std::move(pStatsPanel));
    }

    pApp->createWindow(WindowConfig{}, pGuiWorkspace);

    pApp->run();
    return 0;
}