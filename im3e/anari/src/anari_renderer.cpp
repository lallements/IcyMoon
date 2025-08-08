#include "anari_renderer.h"

#include <fmt/format.h>

#include <stdexcept>

using namespace im3e;

namespace {

auto chooseAnRendererSubtype(const ILogger& rLogger, ANARIDevice pDevice)
{
    const auto** pSubtypes = anariGetObjectSubtypes(pDevice, ANARI_RENDERER);
    throwIfNull<std::runtime_error>(pSubtypes, "Failed to retrieve renderer subtypes");

    std::string subtypesMsg = "[";
    for (const auto** pSubtype = pSubtypes; *pSubtype != nullptr; pSubtype++)
    {
        if (pSubtype != pSubtypes)
        {
            subtypesMsg += ", ";
        }
        subtypesMsg += fmt::format("{}", *pSubtype);
    }
    subtypesMsg += "]";

    const std::string chosenSubtype = *pSubtypes;
    rLogger.info(fmt::format("Available renderer subtypes: {}. Choosing: {}", subtypesMsg, chosenSubtype));
    return chosenSubtype;
}

auto createAnRenderer(const ILogger& rLogger, ANARIDevice anDevice, std::string_view anSubtype)
{
    auto anRenderer = anariNewRenderer(anDevice, anSubtype.data());
    throwIfNull<std::runtime_error>(anRenderer, fmt::format("Failed to create renderer with subtype {}", anSubtype));

    constexpr std::array<float, 4U> BackgroundColor{0.3F, 0.3F, 0.4F, 1.0F};
    anariSetParameter(anDevice, anRenderer, "background", ANARI_FLOAT32_VEC4, BackgroundColor.data());

    anariCommitParameters(anDevice, anRenderer);
    rLogger.debug(fmt::format("Created renderer with subtype {}", anSubtype));

    return std::shared_ptr<anari::api::Renderer>(
        anRenderer, [anDevice, pLogger = &rLogger, anSubtype](auto* anRenderer) {
            anariRelease(anDevice, anRenderer);
            pLogger->debug(fmt::format("Destroyed renderer with subtype {}", anSubtype));
        });
}

template <typename T>
auto getRenderParamInfo(ANARIDevice anDevice, std::string_view rendererSubtype, const ANARIParameter* pAnParam,
                        std::string_view attribute, ANARIDataType attributeType)
{
    return static_cast<const T*>(anariGetParameterInfo(anDevice, ANARI_RENDERER, rendererSubtype.data(), pAnParam->name,
                                                       pAnParam->type, attribute.data(), attributeType));
}

template <typename T>
auto createPropertyValue(ANARIDevice anDevice, ANARIRenderer anRenderer, std::string_view rendererSubtype,
                         const ANARIParameter* pAnParam)
{
    const auto* description = getRenderParamInfo<char>(anDevice, rendererSubtype, pAnParam, "description",
                                                       ANARI_STRING);
    const auto* defaultValue = getRenderParamInfo<T>(anDevice, rendererSubtype, pAnParam, "default", pAnParam->type);

    PropertyValueConfig<T> config{
        .name = pAnParam->name,
        .description = description,
        .onChange =
            [anDevice, anRenderer, pAnParam](T newValue) {
                anariSetParameter(anDevice, anRenderer, pAnParam->name, pAnParam->type, &newValue);
            },
    };
    if (defaultValue)
    {
        config.defaultValue = *defaultValue;
    }
    return std::make_shared<PropertyValue<T>>(std::move(config));
}

auto createPropertyValueFromAnParameter(ANARIDevice anDevice, ANARIRenderer anRenderer,
                                        std::string_view rendererSubtype, const ANARIParameter* pAnParam)
    -> std::shared_ptr<IPropertyValue>
{
    switch (static_cast<int>(pAnParam->type))
    {
        case ANARI_BOOL: return createPropertyValue<bool>(anDevice, anRenderer, rendererSubtype, pAnParam);
        case ANARI_INT32: return createPropertyValue<int32_t>(anDevice, anRenderer, rendererSubtype, pAnParam);
        case ANARI_FLOAT32: return createPropertyValue<float>(anDevice, anRenderer, rendererSubtype, pAnParam);
        case ANARI_STRING: return createPropertyValue<std::string>(anDevice, anRenderer, rendererSubtype, pAnParam);
        case ANARI_FLOAT32_VEC3: return createPropertyValue<glm::vec3>(anDevice, anRenderer, rendererSubtype, pAnParam);
        case ANARI_FLOAT32_VEC4: return createPropertyValue<glm::vec4>(anDevice, anRenderer, rendererSubtype, pAnParam);
        case ANARI_ARRAY2D: return createPropertyValue<void*>(anDevice, anRenderer, rendererSubtype, pAnParam);
        case ANARI_UNKNOWN: throw std::runtime_error(fmt::format("Unsupported UNKNOWN ANARI data type"));
        default: break;
    }
    throw std::runtime_error(fmt::format("Unsupported ANARI data type: {}", static_cast<int>(pAnParam->type)));
}

}  // namespace

AnariRenderer::AnariRenderer(const ILogger& rLogger, ANARIDevice anDevice)
  : m_pLogger(rLogger.createChild("ANARI Renderer"))
  , m_anDevice(throwIfArgNull(anDevice, "ANARI Renderer requires an ANARI device"))
  , m_anRendererSubtype(chooseAnRendererSubtype(*m_pLogger, m_anDevice))
  , m_pAnRenderer(createAnRenderer(*m_pLogger, m_anDevice, m_anRendererSubtype))
{
}

void AnariRenderer::commitChanges()
{
    anariCommitParameters(m_anDevice, m_pAnRenderer.get());
}

auto AnariRenderer::createRendererProperties() -> std::shared_ptr<IPropertyGroup>
{
    const auto* pAnParams = static_cast<const ANARIParameter*>(
        anariGetObjectInfo(m_anDevice, ANARI_RENDERER, m_anRendererSubtype.data(), "parameter", ANARI_PARAMETER_LIST));

    std::vector<std::shared_ptr<IProperty>> pProperties;
    for (const auto* pAnParam = pAnParams; pAnParam->name != nullptr; pAnParam++)
    {
        pProperties.emplace_back(
            createPropertyValueFromAnParameter(m_anDevice, m_pAnRenderer.get(), m_anRendererSubtype, pAnParam));
    }
    return createPropertyGroup(fmt::format("Renderer: \"{}\"", m_anRendererSubtype), pProperties);
}
