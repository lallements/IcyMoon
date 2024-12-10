#include <im3e/test_utils/test_utils.h>

#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/xform.h>

TEST(Im3eUsdTest, canUseOpenUsd)
{
    auto pStage = pxr::UsdStage::CreateNew("HelloWorld.usda");
    auto xform = pxr::UsdGeomXform::Define(pStage, pxr::SdfPath("/root"));
    auto mesh = pxr::UsdGeomMesh::Define(pStage, pxr::SdfPath("/root/mesh"));
    pStage->SetDefaultPrim(xform.GetPrim());
}