#include "platform_utils.h"

#include <whereami.h>

#include <string>

using namespace im3e;
using namespace std;
using namespace std::filesystem;

auto im3e::getCurrentExecutableFolder() -> path
{
    auto length = wai_getExecutablePath(nullptr, 0, nullptr);

    string executablePath(length, ' ');
    wai_getExecutablePath(executablePath.data(), length, &length);
    return path{executablePath}.parent_path();
}