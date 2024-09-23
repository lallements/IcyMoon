#include "loggers.h"

#include <gmock/gmock.h>

using namespace im3e;
using namespace std;
using namespace testing;

TEST(TerminalLoggerTest, canCreateTerminalLogger)
{
    auto pLogger = createTerminalLogger();
    ASSERT_THAT(pLogger, NotNull());
}
