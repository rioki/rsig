cmake_minimum_required(VERSION 3.8)
project(rsig CXX)

option(ENABLE_UNIT_TESTS "Enable building unit tests." OFF)

set(CMAKE_SHARED_LIBRARY_PREFIX)
set(CMAKE_STATIC_LIBRARY_PREFIX)

if(ENABLE_UNIT_TESTS)
  find_package(GTest CONFIG REQUIRED)
endif()

# rsig
set(HEADERS_RSIG
  rsig/rsig.h
)

# rsig-test 
if(ENABLE_UNIT_TESTS)

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

endif()

# install
target_include_directories(rsig PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/rsig>
    $<INSTALL_INTERFACE:include/rsig>)

install(TARGETS ${PROJECT_NAME}
    EXPORT litehtmlTargets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include/rsig
)
