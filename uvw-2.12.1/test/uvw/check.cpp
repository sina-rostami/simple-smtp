#include <gtest/gtest.h>
#include <uvw/check.h>

TEST(Check, StartAndStop) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::CheckHandle>();

    bool checkCheckEvent = false;

    handle->on<uvw::ErrorEvent>([](auto &&...) { FAIL(); });

    handle->on<uvw::CheckEvent>([&checkCheckEvent](const auto &, auto &hndl) {
        ASSERT_FALSE(checkCheckEvent);
        checkCheckEvent = true;
        hndl.stop();
        hndl.close();
        ASSERT_TRUE(hndl.closing());
    });

    handle->start();

    ASSERT_TRUE(handle->active());
    ASSERT_FALSE(handle->closing());

    loop->run<uvw::Loop::Mode::NOWAIT>();

    ASSERT_TRUE(checkCheckEvent);
}

TEST(Check, Fake) {
    auto loop = uvw::Loop::getDefault();
    auto handle = loop->resource<uvw::CheckHandle>();

    handle->on<uvw::ErrorEvent>([](auto &&...) { FAIL(); });
    handle->on<uvw::CheckEvent>([](auto &&...) { FAIL(); });

    handle->start();
    handle->close();

    ASSERT_FALSE(handle->active());
    ASSERT_TRUE(handle->closing());

    loop->run();
}
