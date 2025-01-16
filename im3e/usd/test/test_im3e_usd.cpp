#include <im3e/test_utils/test_utils.h>

#include <im3e/resources/resources.h>

#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/xform.h>

using namespace im3e;
using namespace std;

TEST(Im3eUsdTest, canCreateNewUsdStage)
{
    auto pStage = pxr::UsdStage::CreateNew("HelloWorld.usda");
    auto xform = pxr::UsdGeomXform::Define(pStage, pxr::SdfPath("/root"));
    auto mesh = pxr::UsdGeomMesh::Define(pStage, pxr::SdfPath("/root/mesh"));
    pStage->SetDefaultPrim(xform.GetPrim());
}

TEST(Im3eUsdTest, canLoadTestAsset)
{
    auto testAssetPath = getTestAssetsPath() / "usd_wg_assets" / "full_assets" / "CarbonFrameBike" /
                         "CarbonFrameBike.usdz";
    auto pStage = pxr::UsdStage::Open(testAssetPath.string());
    ASSERT_THAT(pStage, NotNull());
}