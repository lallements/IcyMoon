#pragma once

#include <im3e/api/image.h>

#include <gmock/gmock.h>

namespace im3e {

class MockImageFactory : public IImageFactory
{
public:
    MockImageFactory();
    ~MockImageFactory() override;

    MOCK_METHOD(std::unique_ptr<IImage>, createImage, (ImageConfig config), (const, override));
    MOCK_METHOD(std::unique_ptr<IHostVisibleImage>, createHostVisibleImage, (ImageConfig config), (const, override));

    auto createMockProxy() -> std::unique_ptr<IImageFactory>;
};

}  // namespace im3e