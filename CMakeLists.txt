cmake_minimum_required(VERSION 3.15)

project(xsettingsd
  VERSION 1.0.0
  DESCRIPTION "Provides settings to X11 applications via the XSETTINGS specification"
  HOMEPAGE_URL "https://github.com/derat/xsettingsd"
  LANGUAGES CXX
)

include(GNUInstallDirs)
include(CTest)

find_package(X11 REQUIRED)
find_package(GTest)

add_library(libxsettingsd STATIC
  common.cc
  config_parser.cc
  data_reader.cc
  data_writer.cc
  setting.cc
  settings_manager.cc
)

add_executable(xsettingsd xsettingsd.cc)
target_link_libraries(xsettingsd PRIVATE libxsettingsd X11::X11)

add_executable(dump_xsettings dump_xsettings.cc)
target_link_libraries(dump_xsettings PRIVATE libxsettingsd X11::X11)

install(TARGETS xsettingsd dump_xsettings DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES xsettingsd.1 dump_xsettings.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)

if(GTEST_FOUND AND BUILD_TESTING)
  include(GoogleTest)
   
  add_executable(common_test common_test.cc)
  target_link_libraries(common_test PRIVATE libxsettingsd GTest::GTest)
  gtest_discover_tests(common_test)
  
  add_executable(config_parser_test config_parser_test.cc)
  target_link_libraries(config_parser_test PRIVATE libxsettingsd GTest::GTest)
  target_compile_definitions(config_parser_test PRIVATE __TESTING)
  gtest_discover_tests(config_parser_test)
  
  add_executable(setting_test setting_test.cc)
  target_link_libraries(setting_test PRIVATE libxsettingsd GTest::GTest)
  target_compile_options(setting_test PRIVATE -Wno-narrowing)
  gtest_discover_tests(setting_test)
endif()

add_custom_target(uninstall COMMAND xargs rm -v < ${CMAKE_BINARY_DIR}/install_manifest.txt)
