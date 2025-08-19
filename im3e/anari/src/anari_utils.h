#pragma once

#include <glm/glm.hpp>

#include <array>

namespace im3e {

/// @brief Converts a GLM matrix to std::array compatible with ANARI
/// ANARI matrixes are stored in column-major order but GLM matrixes are in row-major order.
inline auto toAnMatrix(const glm::mat4& rGlmMat)
{
    return std::array<std::array<float, 4U>, 4U>{
        std::array<float, 4U>{rGlmMat[0][0], rGlmMat[1][0], rGlmMat[2][0], rGlmMat[3][0]},
        std::array<float, 4U>{rGlmMat[0][1], rGlmMat[1][1], rGlmMat[2][1], rGlmMat[3][1]},
        std::array<float, 4U>{rGlmMat[0][2], rGlmMat[1][2], rGlmMat[2][2], rGlmMat[3][2]},
        std::array<float, 4U>{rGlmMat[0][3], rGlmMat[1][3], rGlmMat[2][3], rGlmMat[3][3]},
    };
}

}  // namespace im3e