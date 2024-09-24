#include "loggers.h"

#include <im3e/test_utils/test_utils.h>

using namespace im3e;
using namespace std;

TEST(TerminalLoggerTest, canCreateTerminalLogger)
{
    auto pLogger = createTerminalLogger();
    ASSERT_THAT(pLogger, NotNull());
}
