cmake_minimum_required(VERSION 3.15)

project(xsettingsd VERSION 1.0.0)

include(GNUInstallDirs)

find_package(X11 REQUIRED)

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
