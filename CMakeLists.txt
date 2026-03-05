cmake_minimum_required(VERSION 3.14)
project(facts VERSION 1.3.0 LANGUAGES C CXX
  DESCRIPTION "lightweight and beginner friendly C/C++ testing library")

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

# --- C library (libfacts) ---
add_library(facts_c_static STATIC src/facts.c)
target_include_directories(facts_c_static PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
set_target_properties(facts_c_static PROPERTIES OUTPUT_NAME facts)

add_library(facts_c_shared SHARED src/facts.c)
target_include_directories(facts_c_shared PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
set_target_properties(facts_c_shared PROPERTIES
  OUTPUT_NAME facts
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR})

# --- C++ library (libfacts++) ---
# facts.cpp includes facts.c, so it contains the full C core + C++ extras.
add_library(facts_cpp_static STATIC src/facts.cpp)
target_include_directories(facts_cpp_static PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
set_target_properties(facts_cpp_static PROPERTIES OUTPUT_NAME facts++)

add_library(facts_cpp_shared SHARED src/facts.cpp)
target_include_directories(facts_cpp_shared PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
set_target_properties(facts_cpp_shared PROPERTIES
  OUTPUT_NAME facts++
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR})

# --- Install ---
include(GNUInstallDirs)

install(TARGETS facts_c_static facts_c_shared facts_cpp_static facts_cpp_shared
  EXPORT factsTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES include/facts.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT factsTargets
  FILE factsTargets.cmake
  NAMESPACE facts::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/facts)

# --- CMake package config ---
include(CMakePackageConfigHelpers)

configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/factsConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/factsConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/facts)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/factsConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/factsConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/factsConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/facts)

# --- pkg-config ---
set(PREFIX ${CMAKE_INSTALL_PREFIX})
set(LIBDIR ${CMAKE_INSTALL_FULL_LIBDIR})
set(INCLUDEDIR ${CMAKE_INSTALL_FULL_INCLUDEDIR})
configure_file(facts.pc.in facts.pc @ONLY)
configure_file(facts++.pc.in facts++.pc @ONLY)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/facts.pc
  ${CMAKE_CURRENT_BINARY_DIR}/facts++.pc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

# --- Tests (optional) ---
option(FACTS_BUILD_TESTS "Build test binaries" OFF)
if(FACTS_BUILD_TESTS)
  enable_testing()

  add_executable(testfacts_c test/testfacts.c)
  target_link_libraries(testfacts_c PRIVATE facts_c_static)
  add_test(NAME testfacts_c COMMAND testfacts_c)
  set_tests_properties(testfacts_c PROPERTIES WILL_FAIL TRUE)

  add_executable(testfacts_cpp test/testfacts.cpp)
  target_link_libraries(testfacts_cpp PRIVATE facts_cpp_static)
  add_test(NAME testfacts_cpp COMMAND testfacts_cpp)
  set_tests_properties(testfacts_cpp PROPERTIES WILL_FAIL TRUE)

  add_executable(testfacts_if_c test/testfacts_if.c)
  target_link_libraries(testfacts_if_c PRIVATE facts_c_static)
  add_test(NAME testfacts_if_c COMMAND testfacts_if_c)

  add_executable(testfacts_if_cpp test/testfacts_if.cpp)
  target_link_libraries(testfacts_if_cpp PRIVATE facts_cpp_static)
  add_test(NAME testfacts_if_cpp COMMAND testfacts_if_cpp)
endif()
