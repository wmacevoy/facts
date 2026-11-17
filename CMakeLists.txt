cmake_minimum_required(VERSION 3.14)
project(facts VERSION 2.4.0 LANGUAGES C CXX
  DESCRIPTION "lightweight and beginner friendly C/C++ testing library")

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

# Export all symbols from shared libraries on Windows (avoids __declspec)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

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

  # facts_expect(<test name> <target> <exit code> <text the output must contain> [arguments...])
  # Each test requires the exact exit code and the text, so a crash cannot pass
  # for an intended failure (REQ1/PK-2).  The same cases, and more, are in
  # test/check.sh for `make check`.
  function(facts_expect name target code says)
    add_test(NAME ${name}
      COMMAND ${CMAKE_COMMAND} -DPROGRAM=$<TARGET_FILE:${target}> "-DARGS=${ARGN}"
              -DEXIT=${code} "-DSAYS=${says}" -P ${CMAKE_CURRENT_SOURCE_DIR}/test/expect.cmake)
    set_tests_properties(${name} PROPERTIES TIMEOUT 120)
  endfunction()

  function(facts_test_program target source library)
    add_executable(${target} ${source})
    target_link_libraries(${target} PRIVATE ${library})
    # --facts_find looks groups up by symbol, so the program must export them.
    set_target_properties(${target} PROPERTIES ENABLE_EXPORTS ON)
  endfunction()

  facts_test_program(testfacts_c        test/testfacts.c        facts_c_static)
  facts_test_program(testfacts_cpp      test/testfacts.cpp      facts_cpp_static)
  facts_test_program(testfacts_if_c     test/testfacts_if.c     facts_c_static)
  facts_test_program(testfacts_if_cpp   test/testfacts_if.cpp   facts_cpp_static)
  facts_test_program(testfacts_expr_cpp test/testfacts_expr.cpp facts_cpp_static)
  facts_test_program(testfacts_find_c   test/testfacts_find.c   facts_c_static)
  facts_test_program(testfacts_once_c   test/testfacts_once.c   facts_c_static)
  facts_test_program(testfacts_once_cpp test/testfacts_once.cpp facts_cpp_static)
  facts_test_program(testfacts_bad_c    test/testfacts_bad.c    facts_c_static)
  facts_test_program(testfacts_err_c    test/testfacts_err.c    facts_c_static)
  facts_test_program(testfacts_interval_cpp test/testfacts_interval.cpp facts_cpp_static)
  facts_test_program(testfacts_show_cpp test/testfacts_show.cpp facts_cpp_static)
  facts_test_program(testfacts_current_c   test/testfacts_current.c   facts_c_static)
  facts_test_program(testfacts_current_cpp test/testfacts_current.cpp facts_cpp_static)
  # C groups and C++ groups in one program.
  facts_test_program(testfacts_leave_cpp "test/testfacts_leave.cpp;test/testfacts_leave.c" facts_cpp_static)
  # The same, linked with the C library.
  facts_test_program(testfacts_leave_clib "test/testfacts_leave.cpp;test/testfacts_leave.c" facts_c_static)

  set(three_fictions "0 (0.0%) truths and 3 (100.0%) fictions checked.")

  # REQ1/CK-1, REQ2/CK-3: the report's last line, and exit code 1 for a fiction.
  facts_expect(testfacts_c          testfacts_c   1 "${three_fictions}")
  facts_expect(testfacts_cpp        testfacts_cpp 1 "${three_fictions}")
  # REQ1/SX-3: the same in this process.
  facts_expect(testfacts_c_unsafe   testfacts_c   1 "${three_fictions}" --facts_unsafe)
  # REQ1/CK-4: the program's own main without the switch, the facts with it.
  facts_expect(testfacts_if_c       testfacts_if_c   0 "main")
  facts_expect(testfacts_if_cpp     testfacts_if_cpp 0 "main")
  facts_expect(testfacts_if_c_facts   testfacts_if_c   1 "${three_fictions}" --facts)
  facts_expect(testfacts_if_cpp_facts testfacts_if_cpp 1 "${three_fictions}" --facts)
  # REQ1/WR-5: the C++ expression form.
  facts_expect(testfacts_expr_cpp   testfacts_expr_cpp 1 "22 (71.0%) truths and 9 (29.0%) fictions checked.")
  # REQ1/WR-6: once in C++ everywhere; in C once with gcc and clang, twice and warned about elsewhere.
  facts_expect(testfacts_once_cpp   testfacts_once_cpp 1 "3 (75.0%) truths and 1 (25.0%) fictions checked.")
  if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    facts_expect(testfacts_once_c   testfacts_once_c 1 "3 (75.0%) truths and 1 (25.0%) fictions checked.")
  else()
    facts_expect(testfacts_once_c   testfacts_once_c 1 "warning: 2 of those fictions evaluated each side twice")
  endif()
  # REQ1/WR-4: the error helpers; every fact is a truth.
  facts_expect(testfacts_err_c      testfacts_err_c 0 "19 (100.0%) truths and 0 (0.0%) fictions checked.")
  # REQ3/WR-7: facts::interval.
  facts_expect(testfacts_interval_cpp testfacts_interval_cpp 1 "38 (88.4%) truths and 5 (11.6%) fictions checked.")
  # REQ4/WR-10: FactsCurrent() in a helper or a method with no Facts parameter.
  facts_expect(testfacts_current_c   testfacts_current_c   1 "2 (66.7%) truths and 1 (33.3%) fictions checked.")
  facts_expect(testfacts_current_cpp testfacts_current_cpp 1 "a square with 5 sides")
  # REQ4/WR-9: any value is shown; a container's elements, a pair, or "?".
  facts_expect(testfacts_show_cpp testfacts_show_cpp 1 "x == y {? == ?} is fiction")
  # REQ4/WR-8: a fiction in a C++ helper or lambda ends its group; FACT_BY_RETURN and C leave by return.
  facts_expect(testfacts_leave_cpp testfacts_leave_cpp 1 "7 (46.7%) truths and 8 (53.3%) fictions checked.")
  facts_expect(testfacts_leave_clib testfacts_leave_clib 1 "7 (46.7%) truths and 8 (53.3%) fictions checked.")
  facts_expect(testfacts_leave_cpp_return testfacts_leave_cpp 1 "reached: ReturnHelperGoesOn went on after its helper returned")
  # REQ1/RG-2, REQ2/CK-3: find registers what a source file declares; without it this program knows no group.
  facts_expect(testfacts_find_c       testfacts_find_c 0 "2 (100.0%) truths and 0 (0.0%) fictions checked."
               --facts_find ${CMAKE_CURRENT_SOURCE_DIR}/test/testfacts_find.c)
  facts_expect(testfacts_find_c_none  testfacts_find_c 2 "facts: no facts were checked")
  # REQ2/CK-5: a reserved argument facts does not know.
  facts_expect(testfacts_c_reserved   testfacts_c 2 "facts: unknown option --facts_inclde=Out" --facts_inclde=Out)
  # REQ1/SX-1, REQ1/SX-2, REQ2/CK-3: a crash and a hang are reported, with exit code 2.
  facts_expect(testfacts_bad_c_fine   testfacts_bad_c 0 "1 (100.0%) truths and 0 (0.0%) fictions checked.")
  if(WIN32)
    facts_expect(testfacts_bad_c_crash testfacts_bad_c 2 "facts: child crashed" --facts_include=Crashes)
  else()
    facts_expect(testfacts_bad_c_crash testfacts_bad_c 2 "facts: child killed by signal 11 (SIGSEGV)" --facts_include=Crashes)
  endif()
  facts_expect(testfacts_bad_c_sleeps testfacts_bad_c 2 "facts: TIMEOUT after 1 seconds" --facts_timeout=1 --facts_include=Sleeps)
  facts_expect(testfacts_bad_c_spins  testfacts_bad_c 2 "facts: TIMEOUT after 1 seconds" --facts_timeout=1 --facts_include=Spins)
endif()
