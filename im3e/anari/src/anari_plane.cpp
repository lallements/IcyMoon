#include "anari_plane.h"

#include "anari_utils.h"

#include <im3e/utils/properties/properties.h>

#include <fmt/format.h>

using namespace im3e;

namespace {

auto createPlaneGeometry(const ILogger& rLogger, ANARIDevice anDevice)
{
    auto anGeometry = anariNewGeometry(anDevice, "triangle");
    UniquePtrWithDeleter<anari::api::Geometry> pGeometry(anGeometry, [anDevice, pLogger = &rLogger](auto anGeometry) {
        anariRelease(anDevice, anGeometry);
        pLogger->debug("Released plane geometry");
    });

    // Vertex positions:
    {
        const float halfWidth = 1000.0F;
        const std::vector<glm::vec3> vertices{
            glm::vec3{-halfWidth, 0.0F, halfWidth},
            glm::vec3{-halfWidth, 0.0F, -halfWidth},
            glm::vec3{halfWidth, 0.0F, halfWidth},
            glm::vec3{halfWidth, 0.0F, -halfWidth},
        };
        auto anArray = anariNewArray1D(anDevice, vertices.data(), nullptr, nullptr, ANARI_FLOAT32_VEC3,
                                       vertices.size());
        anariCommitParameters(anDevice, anArray);
        anariSetParameter(anDevice, anGeometry, "vertex.position", ANARI_ARRAY1D, &anArray);
        anariRelease(anDevice, anArray);
    }

    // Vertex normals:
    {
        const std::vector<glm::vec3> normals{
            glm::vec3(0.0F, 1.0F, 0.0F),
            glm::vec3(0.0F, 1.0F, 0.0F),
            glm::vec3(0.0F, 1.0F, 0.0F),
            glm::vec3(0.0F, 1.0F, 0.0F),
        };
        auto anArray = anariNewArray1D(anDevice, normals.data(), nullptr, nullptr, ANARI_FLOAT32_VEC3, normals.size());
        anariCommitParameters(anDevice, anArray);
        anariSetParameter(anDevice, anGeometry, "vertex.normal", ANARI_ARRAY1D, &anArray);
        anariRelease(anDevice, anArray);
    }

    // Vertex colors:
    {
        const std::vector<glm::vec4> colors{
            glm::vec4{1.0F, 0.0F, 0.0F, 1.0F},
            glm::vec4{0.0F, 1.0F, 0.0F, 1.0F},
            glm::vec4{0.0F, 0.0F, 1.0F, 1.0F},
            glm::vec4{1.0F, 1.0F, 1.0F, 1.0F},
        };
        auto anArray = anariNewArray1D(anDevice, colors.data(), nullptr, nullptr, ANARI_FLOAT32_VEC4, colors.size());
        anariCommitParameters(anDevice, anArray);
        anariSetParameter(anDevice, anGeometry, "vertex.color", ANARI_ARRAY1D, &anArray);
        anariRelease(anDevice, anArray);
    }

    // Vertex indices
    {
        const std::vector<glm::u32vec3> vertexIndices{
            glm::u32vec3{0U, 1U, 2U},
            glm::u32vec3{1U, 2U, 3U},
        };
        auto anArray = anariNewArray1D(anDevice, vertexIndices.data(), nullptr, nullptr, ANARI_UINT32_VEC3,
                                       vertexIndices.size());
        anariCommitParameters(anDevice, anArray);
        anariSetParameter(anDevice, anGeometry, "primitive.index", ANARI_ARRAY1D, &anArray);
        anariRelease(anDevice, anArray);
    }

    anariCommitParameters(anDevice, anGeometry);
    rLogger.debug("Created plane geometry");
    return pGeometry;
}

auto createPlaneMaterial(const ILogger& rLogger, ANARIDevice anDevice)
{
    auto anMaterial = anariNewMaterial(anDevice, "matte");

    // glm::vec3 materialColor{0.06F, 0.1F, 0.13F};
    // anariSetParameter(anDevice, anMaterial, "color", ANARI_FLOAT32_VEC3, &materialColor);

    anariSetParameter(anDevice, anMaterial, "color", ANARI_STRING, "color");
    anariCommitParameters(anDevice, anMaterial);

    rLogger.debug("Created plane material");
    return UniquePtrWithDeleter<anari::api::Material>(anMaterial, [anDevice, pLogger = &rLogger](auto anMaterial) {
        anariRelease(anDevice, anMaterial);
        pLogger->debug("Released plane material");
    });
}

auto createPlaneSurface(const ILogger& rLogger, ANARIDevice anDevice, ANARIGeometry anGeometry,
                        ANARIMaterial anMaterial)
{
    auto anSurface = anariNewSurface(anDevice);
    anariSetParameter(anDevice, anSurface, "geometry", ANARI_GEOMETRY, &anGeometry);
    anariSetParameter(anDevice, anSurface, "material", ANARI_MATERIAL, &anMaterial);
    anariCommitParameters(anDevice, anSurface);

    rLogger.debug("Created plane surface");
    return UniquePtrWithDeleter<anari::api::Surface>(anSurface, [anDevice, pLogger = &rLogger](auto anSurface) {
        anariRelease(anDevice, anSurface);
        pLogger->debug("Released plane surface");
    });
}

auto createPlaneGroup(const ILogger& rLogger, AnariDevice& rDevice, ANARISurface anSurface)
{
    auto anDevice = rDevice.getHandle();
    auto anGroup = anariNewGroup(anDevice);
    {
        auto pAnSurfaces = rDevice.createArray1d(std::vector<ANARISurface>{anSurface}, ANARI_SURFACE);
        auto anSurfaces = pAnSurfaces.get();
        anariSetParameter(anDevice, anGroup, "surface", ANARI_ARRAY1D, &anSurfaces);
    }
    anariCommitParameters(anDevice, anGroup);
    rLogger.debug("Created group");
    return UniquePtrWithDeleter<anari::api::Group>(anGroup, [anDevice, pLogger = &rLogger](auto anGroup) {
        anariRelease(anDevice, anGroup);
        pLogger->debug("Released group");
    });
}

auto createPlaneInstance(const ILogger& rLogger, ANARIDevice anDevice, ANARIGroup anGroup)
{
    auto anInstance = anariNewInstance(anDevice, "transform");
    anariSetParameter(anDevice, anInstance, "group", ANARI_GROUP, &anGroup);
    anariCommitParameters(anDevice, anInstance);

    rLogger.debug("Created instance");
    return std::shared_ptr<anari::api::Instance>(anInstance, [anDevice, pLogger = &rLogger](auto anInstance) {
        anariRelease(anDevice, anInstance);
        pLogger->debug("Released instance");
    });
}

}  // namespace

AnariPlane::AnariPlane(std::string_view name, std::shared_ptr<AnariDevice> pAnDevice, AnariInstanceSet& rInstanceSet)
  : m_name(name)
  , m_pAnDevice(throwIfArgNull(std::move(pAnDevice), "ANARI Plane requires an ANARI device"))
  , m_rInstanceSet(rInstanceSet)

  , m_pLogger(m_pAnDevice->createLogger(fmt::format("ANARI Plane - {}", m_name)))

  , m_pAnGeometry(createPlaneGeometry(*m_pLogger, m_pAnDevice->getHandle()))
  , m_pAnMaterial(createPlaneMaterial(*m_pLogger, m_pAnDevice->getHandle()))
  , m_pAnSurface(createPlaneSurface(*m_pLogger, m_pAnDevice->getHandle(), m_pAnGeometry.get(), m_pAnMaterial.get()))
  , m_pAnGroup(createPlaneGroup(*m_pLogger, *m_pAnDevice, m_pAnSurface.get()))
  , m_pAnInstance(createPlaneInstance(*m_pLogger, m_pAnDevice->getHandle(), m_pAnGroup.get()))

  , m_pScaleProp(std::make_shared<PropertyValue<glm::vec3>>(PropertyValueConfig<glm::vec3>{
        .name = "Scale",
        .description = "Applies a scale to the plane",
        .defaultValue = m_transform.getScale(),
        .onChange =
            [this](auto scale) {
                m_transform.setScale(scale);
                m_transformChanged = true;
            },
    }))
  , m_pProperties(createPropertyGroup(m_name, {m_pScaleProp}))
{
    m_transformChanged = true;
    m_rInstanceSet.insert(m_pAnInstance);
}

AnariPlane::~AnariPlane()
{
    m_rInstanceSet.remove(m_pAnInstance);
}

void AnariPlane::commitChanges()
{
    if (m_transformChanged)
    {
        const auto transformMat = toAnMatrix(m_transform.toMatrix());
        anariSetParameter(m_pAnDevice->getHandle(), m_pAnInstance.get(), "transform", ANARI_FLOAT32_MAT4,
                          transformMat.data());
        anariCommitParameters(m_pAnDevice->getHandle(), m_pAnInstance.get());
        m_transformChanged = false;
    }
}
