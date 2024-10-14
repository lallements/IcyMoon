#include "mock_command_buffer.h"

using namespace im3e;
using namespace std;

namespace {

class MockProxyCommandBuffer : public ICommandBuffer
{
public:
    MockProxyCommandBuffer(MockCommandBuffer& rMock)
      : m_rMock(rMock)
    {
    }

    auto getVkCommandBuffer() const -> VkCommandBuffer override { return m_rMock.getVkCommandBuffer(); }

private:
    MockCommandBuffer& m_rMock;
};

}  // namespace

MockCommandBuffer::MockCommandBuffer()
{
    ON_CALL(*this, getVkCommandBuffer()).WillByDefault(Return(m_vkCommandBuffer));
}

MockCommandBuffer::~MockCommandBuffer() = default;

auto MockCommandBuffer::createMockProxy() -> unique_ptr<ICommandBuffer>
{
    return make_unique<MockProxyCommandBuffer>(*this);
}

namespace {

class MockProxyCommandQueue : public ICommandQueue
{
public:
    MockProxyCommandQueue(MockCommandQueue& rMock)
      : m_rMock(rMock)
    {
    }

    auto startScopedCommand(string_view name, CommandExecutionType executionType)
        -> UniquePtrWithDeleter<ICommandBuffer> override
    {
        return m_rMock.startScopedCommand(name, executionType);
    }

    auto getVkQueue() const -> VkQueue override { return m_rMock.getVkQueue(); }

private:
    MockCommandQueue& m_rMock;
};

}  // namespace

MockCommandQueue::MockCommandQueue()
{
    ON_CALL(*this, startScopedCommand(_, _)).WillByDefault(Invoke([this](Unused, Unused) {
        return m_mockCommandBuffer.createMockProxy();
    }));
    ON_CALL(*this, getVkQueue()).WillByDefault(Return(m_vkQueue));
}

MockCommandQueue::~MockCommandQueue() = default;

auto MockCommandQueue::createMockProxy() -> unique_ptr<ICommandQueue>
{
    return make_unique<MockProxyCommandQueue>(*this);
}