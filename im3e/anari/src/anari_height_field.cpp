#include "anari_height_field.h"

#include <im3e/utils/core/throw_utils.h>

#include <fmt/format.h>

using namespace im3e;

AnariHeightField::AnariHeightField(std::shared_ptr<AnariDevice> pAnDevice, AnariInstanceSet& rInstanceSet,
                                   std::unique_ptr<IHeightMap> pHeightMap)
  : m_pAnDevice(throwIfArgNull(std::move(pAnDevice), "ANARI Height Field requires a device"))
  , m_rInstanceSet(rInstanceSet)
  , m_pHeightMap(throwIfArgNull(std::move(pHeightMap), "ANARI Height Map requires a height map"))
  , m_pLogger(m_pAnDevice->createLogger(fmt::format("ANARI Height Field - {}", m_pHeightMap->getName())))

  , m_pAnGroup(m_pAnDevice->createGroup())
  , m_pAnInstance(m_pAnDevice->createInstance(m_pAnGroup.get()))

  , m_pProperties(createPropertyGroup(m_pHeightMap->getName(), {}))
{
}

void AnariHeightField::commitChanges() {}
