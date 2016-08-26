
find_package(GMock REQUIRED)
find_package(Qt5Test REQUIRED)

pkg_check_modules(TEST_DEPENDENCIES
  libqtdbustest-1 REQUIRED
  libqtdbusmock-1 REQUIRED
  libgmenuharness REQUIRED
)

include_directories(
  ${Qt5Test_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${GMOCK_INCLUDE_DIRS}
  ${GTEST_INCLUDE_DIRS}
  ${TEST_DEPENDENCIES_INCLUDE_DIRS}
)

add_subdirectory(integration)
add_subdirectory(unit)
add_subdirectory(utils)
