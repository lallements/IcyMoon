#include "loggers.h"

#include "stream_logger.h"

#include <fstream>
#include <iostream>

using namespace im3e;
using namespace std;
using namespace std::filesystem;

namespace {

constexpr auto RootName = "Root"sv;

}

unique_ptr<ILogger> im3e::createTerminalLogger()
{
    return make_unique<StreamLogger>(RootName, shared_ptr<ostream>(&cout, [](auto*) {}));
}

unique_ptr<ILogger> im3e::createFileLogger(const path& rFilePath)
{
    return make_unique<StreamLogger>(RootName, make_shared<ofstream>(rFilePath, ios::trunc));
}
