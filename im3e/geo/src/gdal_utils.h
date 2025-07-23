#pragma once

#include <fmt/format.h>
#include <gdal.h>
#include <gdal_priv.h>

#include <algorithm>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace im3e {

inline auto convertGdalDataTypeToString(GDALDataType dataType) -> std::string_view
{
    switch (dataType)
    {
        case GDT_Unknown: return "Unknown";
        case GDT_Byte: return "Byte";
        case GDT_Int8: return "Int8";
        case GDT_UInt16: return "UInt16";
        case GDT_Int16: return "Int16";
        case GDT_UInt32: return "UInt32";
        case GDT_Int32: return "Int32";
        case GDT_UInt64: return "UInt64";
        case GDT_Int64: return "Int64";
        case GDT_Float32: return "Float32";
        case GDT_Float64: return "Float64";
        case GDT_CInt16: return "CInt16";
        case GDT_CInt32: return "CInt32";
        case GDT_CFloat32: return "CFloat32";
        case GDT_CFloat64: return "CFloat64";
        default: break;
    }
    return "Undefined";
}

inline auto convertGdalMaskFlagsToString(int maskFlags) -> std::string
{
    std::vector<std::string> flags;

    auto emplaceIfFound = [&](int flag, std::string_view name) {
        if ((maskFlags & flag) == flag)
        {
            flags.emplace_back(name);
        }
    };
    emplaceIfFound(GMF_ALL_VALID, "GMF_ALL_VALID");
    emplaceIfFound(GMF_PER_DATASET, "GMF_PER_DATASET");
    emplaceIfFound(GMF_ALPHA, "GMF_ALPHA");
    emplaceIfFound(GMF_NODATA, "GMF_NODATA");

    std::string result;
    std::ranges::for_each(flags, [&result, isFirst = true](auto& flagName) mutable {
        if (!isFirst)
        {
            result += " | ";
        }
        isFirst = false;
        result += flagName;
    });
    return result;
}

inline auto convertGdalSuggestedBlockAccessPatternToString(GDALSuggestedBlockAccessPattern pattern) -> std::string_view
{
    switch (pattern)
    {
        case GSBAP_UNKNOWN: return "GSBAP_UNKNOWN";
        case GSBAP_RANDOM: return "GSBAP_RANDOM";
        case GSBAP_TOP_TO_BOTTOM: return "GSBAP_TOP_TO_BOTTOM";
        case GSBAP_BOTTOM_TO_TOP: return "GSBAP_BOTTOM_TO_TOP";
        case GSBAP_LARGEST_CHUNK_POSSIBLE: return "GSBAP_LARGEST_CHUNK_POSSIBLE";
        default: break;
    }
    throw std::runtime_error(fmt::format("Failed to convert GDALSuggestedBlockAccessPattern: uknown value {}",
                                         static_cast<int32_t>(pattern)));
}

}  // namespace im3e