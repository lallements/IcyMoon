#include "usd.h"

#include <im3e/utils/throw_utils.h>

#include <fmt/format.h>
#include <pxr/imaging/hd/sceneIndexAdapterSceneDelegate.h>
#include <pxr/imaging/hdx/renderTask.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usdImaging/usdImaging/sceneIndices.h>

using namespace im3e;
using namespace std;

namespace {

constexpr auto RenderTaskName = "/renderTask";

auto createSceneIndex(pxr::UsdStageRefPtr pUsdStage) -> pxr::HdSceneIndexBaseRefPtr
{
    const pxr::UsdImagingCreateSceneIndicesInfo sceneIndicesCreateInfo{
        .stage = move(pUsdStage),
    };
    auto sceneIndices = pxr::UsdImagingCreateSceneIndices(sceneIndicesCreateInfo);
    throwIfNull<runtime_error>(sceneIndices.finalSceneIndex, "Failed to create scene index from USD stage");
    return sceneIndices.finalSceneIndex;
}

class HydraFramePipeline : public IFramePipeline
{
public:
    HydraFramePipeline(std::shared_ptr<const IDevice> pDevice, shared_ptr<IHydraRenderer> pRenderer,
                       pxr::UsdStageRefPtr pUsdStage)
      : m_pDevice(throwIfArgNull(move(pDevice), "Hydra Frame Pipeline requires a device"))
      , m_pRenderer(throwIfArgNull(move(pRenderer), "Hydra Frame Pipeline requires a renderer"))
      , m_pUsdStage(throwIfArgNull(move(pUsdStage), "Hydra Frame Pipeline requires a USD stage"))

      , m_renderTaskId(RenderTaskName)
      , m_renderPrim(m_pUsdStage->DefinePrim(m_renderTaskId, pxr::HdTokens->collection))

      , m_pSceneIndex(createSceneIndex(m_pUsdStage))
      , m_sceneDelegateId(pxr::SdfPath::AbsoluteRootPath())
      , m_pSceneDelegateAdapter(make_unique<pxr::HdSceneIndexAdapterSceneDelegate>(
            m_pSceneIndex, &m_pRenderer->getRenderIndex(), m_sceneDelegateId))
    {
        m_pRenderer->getRenderIndex().InsertTask<pxr::HdxRenderTask>(m_pSceneDelegateAdapter.get(), m_renderTaskId);
    }

    void prepareExecution(const ICommandBuffer&, std::shared_ptr<IImage>) override {}

    void resize(const VkExtent2D&, uint32_t) override {}

    auto getDevice() const -> std::shared_ptr<const IDevice> override { return m_pDevice; }

private:
    shared_ptr<const IDevice> m_pDevice;
    shared_ptr<IHydraRenderer> m_pRenderer;
    pxr::UsdStageRefPtr m_pUsdStage;

    pxr::SdfPath m_renderTaskId;
    pxr::UsdPrim m_renderPrim;

    pxr::HdSceneIndexBaseRefPtr m_pSceneIndex;
    pxr::SdfPath m_sceneDelegateId;
    unique_ptr<pxr::HdSceneIndexAdapterSceneDelegate> m_pSceneDelegateAdapter;
};

}  // namespace

auto im3e::createHydraFramePipeline(shared_ptr<const IDevice> pDevice, shared_ptr<IHydraRenderer> pRenderer,
                                    pxr::UsdStageRefPtr pUsdStage) -> unique_ptr<IFramePipeline>
{
    return make_unique<HydraFramePipeline>(move(pDevice), move(pRenderer), move(pUsdStage));
}