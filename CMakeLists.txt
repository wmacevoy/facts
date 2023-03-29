#
#  https://stackoverflow.com/questions/17511496/how-to-create-a-shared-library-with-cmake
#

cmake_minimum_required(VERSION 3.13)
project(facts VERSION 1.3.0 DESCRIPTION "lightweight and beginner friendly C/C++ testing library")
set(CMAKE_C_FLAGS_RELEASE "-g -O")
set(CMAKE_CXX_FLAGS_RELEASE "-g -O")
include(GNUInstallDirs)
add_library(facts SHARED src/facts.cpp src/facts.c)
set_target_properties(facts PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER include/facts.h)
configure_file(facts.pc.in facts.pc @ONLY)
target_include_directories(facts PRIVATE include)
install(TARGETS facts
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES ${CMAKE_BINARY_DIR}/facts.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)
