#include <string>
#include <gtest/gtest.h>
#include <scene/Log.hpp>

#ifdef _WIN32
#pragma comment (lib, "WSock32")
#endif

int main(int argc, char **argv)
{
    Scene::initLogger( &argc, argv );
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
