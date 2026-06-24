cmake_minimum_required(VERSION 3.14)
project(kv_store_tests)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/heads/main.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(kv_tests
    tests/test_kv_store.cpp
    tests/test_crc32.cpp
    tests/test_wal.cpp
    src/kv_store.cpp
    src/wal.cpp
    src/crc32.cpp
    src/wal_parser.cpp
    src/wal_serializer.cpp
    src/wal_record.cpp
)

target_include_directories(kv_tests PRIVATE include)

target_link_libraries(kv_tests
    gtest
    gtest_main
)

include(GoogleTest)
gtest_discover_tests(kv_tests)
