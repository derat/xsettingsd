cmake_minimum_required(VERSION 3.15)

project(xsettingsd VERSION 1.0.0)

include(GNUInstallDirs)

find_package(X11 REQUIRED)

set(libxsettingsd_SRCS
    common.cc
    config_parser.cc
    data_reader.cc
    data_writer.cc
    setting.cc
    settings_manager.cc
)

add_library(libxsettingsd STATIC ${libxsettingsd_SRCS})

add_executable(xsettingsd xsettingsd.cc)
add_executable(dump_xsettings dump_xsettings.cc)

target_include_directories(xsettingsd PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(dump_xsettings PUBLIC "${PROJECT_BINARY_DIR}")

target_link_libraries(xsettingsd PUBLIC libxsettingsd X11::X11)
target_link_libraries(dump_xsettings PUBLIC libxsettingsd X11::X11)

install(TARGETS xsettingsd DESTINATION ${CMAKE_INSTALL_BINDIR})
install(TARGETS dump_xsettings DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES xsettingsd.1 dump_xsettings.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
