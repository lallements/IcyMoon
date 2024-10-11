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

    auto startCommandRecording(CommandExecutionType executionType) -> unique_ptr<ICommandBuffer> override
    {
        return m_rMock.startCommandRecording(executionType);
    }

private:
    MockCommandQueue& m_rMock;
};

}  // namespace

MockCommandQueue::MockCommandQueue()
{
    ON_CALL(*this, startCommandRecording(_)).WillByDefault(Invoke([this](Unused) {
        return m_mockCommandBuffer.createMockProxy();
    }));
}

MockCommandQueue::~MockCommandQueue() = default;

auto MockCommandQueue::createMockProxy() -> unique_ptr<ICommandQueue>
{
    return make_unique<MockProxyCommandQueue>(*this);
}