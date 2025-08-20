cmake_minimum_required(VERSION 3.20)

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# For MSVC: avoid overriding parent’s runtime flags
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)
enable_testing()

add_executable(compiler_tests QueryProcessorTest.cpp)
target_link_libraries(compiler_tests
  PRIVATE
    dblib
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(compiler_tests)