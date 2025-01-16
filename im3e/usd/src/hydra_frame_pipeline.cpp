#include "usd.h"

#include <im3e/utils/throw_utils.h>

#include <pxr/imaging/hd/sceneIndexAdapterSceneDelegate.h>
#include <pxr/imaging/hdx/renderTask.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usdImaging/usdImaging/sceneIndices.h>

using namespace im3e;
using namespace std;

namespace {

constexpr auto RenderTaskName = "/renderTask";

auto createRenderIndex(pxr::HdRenderDelegate* pRenderDelegate)
{
    shared_ptr<pxr::HdRenderIndex> pRenderIndex(pxr::HdRenderIndex::New(pRenderDelegate, {}));
    throwIfNull<runtime_error>(pRenderIndex, "Failed to create Hydra render index");
    return pRenderIndex;
}

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
    HydraFramePipeline(std::shared_ptr<const IDevice> pDevice, unique_ptr<pxr::HdRenderDelegate> pRenderDelegate,
                       pxr::UsdStageRefPtr pUsdStage)
      : m_pDevice(throwIfArgNull(move(pDevice), "Hydra Frame Pipeline requires a device"))
      , m_pRenderDelegate(throwIfArgNull(move(pRenderDelegate), "Hydra Frame Pipeline requires a render delegate"))
      , m_pUsdStage(throwIfArgNull(move(pUsdStage), "Hydra Frame Pipeline requires a USD stage"))

      , m_pRenderIndex(createRenderIndex(m_pRenderDelegate.get()))
      , m_renderTaskId(RenderTaskName)
      , m_renderPrim(m_pUsdStage->DefinePrim(m_renderTaskId, pxr::HdTokens->collection))

      , m_pSceneIndex(createSceneIndex(m_pUsdStage))
      , m_sceneDelegateId(pxr::SdfPath::AbsoluteRootPath())
      , m_pSceneDelegateAdapter(
            make_unique<pxr::HdSceneIndexAdapterSceneDelegate>(m_pSceneIndex, m_pRenderIndex.get(), m_sceneDelegateId))
    {
        m_pRenderIndex->InsertTask<pxr::HdxRenderTask>(m_pSceneDelegateAdapter.get(), m_renderTaskId);
    }

    void prepareExecution(const ICommandBuffer&, std::shared_ptr<IImage>) override {}

    void resize(const VkExtent2D&, uint32_t) override {}

    auto getDevice() const -> std::shared_ptr<const IDevice> override { return m_pDevice; }

private:
    shared_ptr<const IDevice> m_pDevice;
    unique_ptr<pxr::HdRenderDelegate> m_pRenderDelegate;
    pxr::UsdStageRefPtr m_pUsdStage;

    shared_ptr<pxr::HdRenderIndex> m_pRenderIndex;
    pxr::SdfPath m_renderTaskId;
    pxr::UsdPrim m_renderPrim;

    pxr::HdSceneIndexBaseRefPtr m_pSceneIndex;
    pxr::SdfPath m_sceneDelegateId;
    unique_ptr<pxr::HdSceneIndexAdapterSceneDelegate> m_pSceneDelegateAdapter;
};

}  // namespace

auto im3e::createHydraFramePipeline(shared_ptr<const IDevice> pDevice,
                                    unique_ptr<pxr::HdRenderDelegate> pRenderDelegate, pxr::UsdStageRefPtr pUsdStage)
    -> unique_ptr<IFramePipeline>
{
    return make_unique<HydraFramePipeline>(move(pDevice), move(pRenderDelegate), move(pUsdStage));
}