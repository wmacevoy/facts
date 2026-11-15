# Run a program; require its exact exit code and a piece of text in its output.
# REQ1/PK-2: this is how ctest checks the test programs, on every platform and
# in particular on Windows, where test/check.sh does not run.  A crash, or any
# exit code but the expected one, fails; so does a run that says the wrong thing.
#
#   cmake -DPROGRAM=<path> "-DARGS=<a;b;c>" -DEXIT=<code> "-DSAYS=<text>" -P expect.cmake

execute_process(COMMAND "${PROGRAM}" ${ARGS}
  RESULT_VARIABLE code
  OUTPUT_VARIABLE out
  ERROR_VARIABLE err
  TIMEOUT 60)

set(all "${out}\n${err}")
string(FIND "${all}" "${SAYS}" at)

if(NOT "${code}" STREQUAL "${EXIT}")
  message(FATAL_ERROR "exit code ${code}, expected ${EXIT}\n--- output ---\n${all}")
endif()
if(at EQUAL -1)
  message(FATAL_ERROR "did not print: ${SAYS}\n--- output ---\n${all}")
endif()
