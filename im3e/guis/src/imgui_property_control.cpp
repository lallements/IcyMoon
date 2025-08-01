#include "imgui_property_control.h"

#include <im3e/utils/imgui_utils.h>

#include <imgui.h>
#include <imgui_stdlib.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace im3e;
using namespace std;

namespace {

template <class T>
auto typeIndex()
{
    return type_index(typeid(T));
}

class ImguiBasePropertyControl : public IImguiPropertyControl
{
public:
    ImguiBasePropertyControl(shared_ptr<IPropertyValue> pProperty)
      : m_pProperty(move(pProperty))
    {
    }

    auto getName() const -> string override { return string{m_pProperty->getName()}; }

protected:
    shared_ptr<IPropertyValue> m_pProperty;
};

struct ImguiVoidPropertyControl : public ImguiBasePropertyControl
{
    ImguiVoidPropertyControl(shared_ptr<IPropertyValue> pProperty)
      : ImguiBasePropertyControl(move(pProperty))
    {
    }

    void draw() override {}
};

struct ImguiUnknownPropertyControl : public ImguiBasePropertyControl
{
    ImguiUnknownPropertyControl(shared_ptr<IPropertyValue> pProperty)
      : ImguiBasePropertyControl(move(pProperty))
    {
    }

    void draw() override
    {
        ImGui::TextColored(ImVec4(1.0F, 0.0F, 0.0F, 1.0F), "Unknown property type: %s", m_pProperty->getType().name());
    }
};

class ImguiStringPropertyControl : public ImguiBasePropertyControl
{
public:
    ImguiStringPropertyControl(shared_ptr<IPropertyValue> pProperty)
      : ImguiBasePropertyControl(move(pProperty))
      , m_value(any_cast<string>(m_pProperty->getAnyValue()))
    {
    }

    void draw() override
    {
        if (ImGui::InputText(fmt::format("##{}", m_pProperty->getName()).c_str(), &m_value))
        {
            m_pProperty->setAnyValue(m_value);
        }
        m_value = any_cast<string>(m_pProperty->getAnyValue());
    }

private:
    string m_value;
};

class ImguiBoolPropertyControl : public ImguiBasePropertyControl
{
public:
    ImguiBoolPropertyControl(shared_ptr<IPropertyValue> pProperty)
      : ImguiBasePropertyControl(move(pProperty))
      , m_value(any_cast<bool>(m_pProperty->getAnyValue()))
    {
    }

    void draw() override
    {
        if (ImGui::Checkbox(fmt::format("##{}", m_pProperty->getName()).c_str(), &m_value))
        {
            m_pProperty->setAnyValue(m_value);
        }
        m_value = any_cast<bool>(m_pProperty->getAnyValue());
    }

private:
    bool m_value;
};

class ImguiInt32PropertyControl : public ImguiBasePropertyControl
{
public:
    ImguiInt32PropertyControl(shared_ptr<IPropertyValue> pProperty)
      : ImguiBasePropertyControl(move(pProperty))
      , m_value(any_cast<int32_t>(m_pProperty->getAnyValue()))
    {
    }

    void draw() override
    {
        const auto inputId = fmt::format("##{}", m_pProperty->getName());
        if (ImGui::InputInt(inputId.c_str(), &m_value))
        {
            m_pProperty->setAnyValue(static_cast<int32_t>(m_value));
        }
        m_value = any_cast<int32_t>(m_pProperty->getAnyValue());
    }

private:
    int m_value{};
};

class ImguiUint32PropertyControl : public ImguiBasePropertyControl
{
public:
    ImguiUint32PropertyControl(shared_ptr<IPropertyValue> pProperty)
      : ImguiBasePropertyControl(move(pProperty))
      , m_value(any_cast<uint32_t>(m_pProperty->getAnyValue()))
    {
    }

    void draw() override
    {
        const auto inputId = fmt::format("##{}", m_pProperty->getName());
        if (ImGui::InputInt(inputId.c_str(), &m_value))
        {
            m_value = max(m_value, 0);
            m_pProperty->setAnyValue(static_cast<uint32_t>(m_value));
        }
        m_value = any_cast<uint32_t>(m_pProperty->getAnyValue());
    }

private:
    int m_value{};
};

class ImguiFloatPropertyControl : public ImguiBasePropertyControl
{
public:
    ImguiFloatPropertyControl(shared_ptr<IPropertyValue> pProperty)
      : ImguiBasePropertyControl(move(pProperty))
      , m_value(any_cast<float>(m_pProperty->getAnyValue()))
    {
    }

    void draw() override
    {
        const auto inputId = fmt::format("##{}", m_pProperty->getName());
        if (ImGui::InputFloat(inputId.c_str(), &m_value, 0.0F, 0.0F))
        {
            m_pProperty->setAnyValue(m_value);
        }
        m_value = any_cast<float>(m_pProperty->getAnyValue());
    }

private:
    float m_value{};
};

class ImguiVec3PropertyControl : public ImguiBasePropertyControl
{
public:
    ImguiVec3PropertyControl(shared_ptr<IPropertyValue> pProperty)
      : ImguiBasePropertyControl(move(pProperty))
      , m_value(any_cast<glm::vec3>(m_pProperty->getAnyValue()))
    {
    }

    void draw() override
    {
        if (ImGui::InputFloat3(fmt::format("##{}", m_pProperty->getName()).c_str(), glm::value_ptr(m_value)))
        {
            m_pProperty->setAnyValue(m_value);
        }
        m_value = any_cast<glm::vec3>(m_pProperty->getAnyValue());
    }

private:
    glm::vec3 m_value;
};

class ImguiVec4PropertyControl : public ImguiBasePropertyControl
{
public:
    ImguiVec4PropertyControl(shared_ptr<IPropertyValue> pProperty)
      : ImguiBasePropertyControl(move(pProperty))
      , m_value(any_cast<glm::vec4>(m_pProperty->getAnyValue()))
    {
    }

    void draw() override
    {
        if (ImGui::InputFloat4(fmt::format("##{}", m_pProperty->getName()).c_str(), glm::value_ptr(m_value)))
        {
            m_pProperty->setAnyValue(m_value);
        }
        m_value = any_cast<glm::vec4>(m_pProperty->getAnyValue());
    }

private:
    glm::vec4 m_value;
};

class ImguiQuatPropertyControl : public ImguiBasePropertyControl
{
public:
    ImguiQuatPropertyControl(shared_ptr<IPropertyValue> pProperty)
      : ImguiBasePropertyControl(move(pProperty))
      , m_value(any_cast<glm::quat>(m_pProperty->getAnyValue()))
    {
    }

    void draw() override
    {
        vector<float> value{m_value.w, m_value.x, m_value.y, m_value.z};

        if (ImGui::InputFloat4(fmt::format("##{}", m_pProperty->getName()).c_str(), value.data()))
        {
            m_pProperty->setAnyValue(glm::quat(value[0], value[1], value[2], value[3]));
        }
        m_value = any_cast<glm::quat>(m_pProperty->getAnyValue());
    }

    auto getName() const -> string override { return fmt::format("{} [w, x, y, z]", m_pProperty->getName()); }

private:
    glm::quat m_value;
};

using ControlFactoryFct = function<unique_ptr<IImguiPropertyControl>(shared_ptr<IPropertyValue>)>;

template <typename T, typename ControlType>
auto createControlFactory() -> pair<type_index, ControlFactoryFct>
{
    return {typeIndex<T>(), [](auto pProperty) { return make_unique<ControlType>(move(pProperty)); }};
}

}  // namespace

auto im3e::createImguiPropertyControl(shared_ptr<IProperty> pProperty) -> unique_ptr<IImguiPropertyControl>
{
    static const unordered_map<type_index, ControlFactoryFct> typeToValueControlFactory{
        createControlFactory<void, ImguiVoidPropertyControl>(),
        createControlFactory<string, ImguiStringPropertyControl>(),
        createControlFactory<bool, ImguiBoolPropertyControl>(),
        createControlFactory<int32_t, ImguiInt32PropertyControl>(),
        createControlFactory<uint32_t, ImguiUint32PropertyControl>(),
        createControlFactory<float, ImguiFloatPropertyControl>(),
        createControlFactory<glm::vec3, ImguiVec3PropertyControl>(),
        createControlFactory<glm::vec4, ImguiVec4PropertyControl>(),
        createControlFactory<glm::quat, ImguiQuatPropertyControl>(),
    };

    if (auto pValueProperty = dynamic_pointer_cast<IPropertyValue>(pProperty))
    {
        auto itFind = typeToValueControlFactory.find(pValueProperty->getType());
        if (itFind != typeToValueControlFactory.end())
        {
            return itFind->second(move(pValueProperty));
        }
        return make_unique<ImguiUnknownPropertyControl>(move(pValueProperty));
    }

    throw runtime_error("createImguiPropertyControl only supports IPropertyValue at the moment");
}
