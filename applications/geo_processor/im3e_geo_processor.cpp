#include <im3e/geo/geo.h>
#include <im3e/utils/loggers.h>
#include <im3e/utils/throw_utils.h>

#include <fmt/format.h>
#include <fmt/std.h>

#include <filesystem>

using namespace im3e;
using namespace std;

int main(int argc, char** argv)
{
    auto pLogger = createTerminalLogger();

    filesystem::path appRelativePath{argv[0]};
    pLogger->info(fmt::format("Application: {}", appRelativePath.filename()));

    constexpr auto ExpectedArgc = 3U;
    throwIfFalse<invalid_argument>(
        argc == ExpectedArgc, fmt::format("Invalid number of arguments passed to application: expected {}, got {}.\n\n"
                                          "Expected Usage:\n"
                                          "\t{} srcPath dstPath\n"
                                          "with:\n"
                                          " - srcPath: path to the source GeoTIFF file\n"
                                          " - dstPath: path to the destimation GeoTIFF file\n",
                                          ExpectedArgc - 1U, argc - 1U, appRelativePath.filename()));

    filesystem::path srcPath{argv[1]};
    throwIfFalse<invalid_argument>(filesystem::exists(srcPath), fmt::format("File not found: \"{}\"", srcPath));
    pLogger->info(fmt::format("Src file: {}", srcPath));

    filesystem::path dstPath{argv[2]};
    if (filesystem::exists(dstPath))
        pLogger->info(fmt::format("Dst file: {}", dstPath));
    {
        pLogger->info(fmt::format("Deleting existing dst file at \"{}\"", dstPath));
        filesystem::remove(dstPath);
    }

    pLogger->info(fmt::format("Copying \"{}\" to \"{}\"", srcPath, dstPath));
    filesystem::copy(srcPath, dstPath);
    pLogger->info("Copy complete");

    auto pHeightMap = loadHeightMapFromFile(*pLogger, HeightMapFileConfig{
                                                          .path = dstPath,
                                                          .readOnly = true,
                                                      });

    return 0;
}