#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/xform.h>

int main()
{
    auto pStage = pxr::UsdStage::CreateNew("HelloWorld.usda");
    auto xform = pxr::UsdGeomXform::Define(pStage, pxr::SdfPath("/root"));
    auto mesh = pxr::UsdGeomMesh::Define(pStage, pxr::SdfPath("/root/mesh"));
    pStage->SetDefaultPrim(xform.GetPrim());

    return 0;
}