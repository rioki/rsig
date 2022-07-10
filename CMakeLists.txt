cmake_minimum_required(VERSION 3.8)
project(rsig CXX)

set(CMAKE_SHARED_LIBRARY_PREFIX)
set(CMAKE_STATIC_LIBRARY_PREFIX)

find_package(GTest CONFIG REQUIRED)

set(HEADERS_RSIG
  rsig/rsig.h
)
 
enable_testing()

set(SOURCES_RSIG_TEST
  rsig-test/main.cpp
  rsig-test/signal_test.cpp.cpp
)

include_directories(.)
add_executable(rsig-test ${SOURCES_RSIG_TEST})
set_target_properties(rsig-test PROPERTIES
  CXX_STANDARD 20
)
target_link_libraries(rsig-test PRIVATE rsig GTest::gtest)
add_test(rsig-test rsig-test)
