cmake_minimum_required(VERSION 3.13)

# Inputs (passed via -D):
# - TCODE_REPO_ROOT
# - TCODE_BUILD_INFO_IN
# - TCODE_BUILD_INFO_OUT

if(NOT DEFINED TCODE_REPO_ROOT OR TCODE_REPO_ROOT STREQUAL "")
  message(FATAL_ERROR "TCODE_REPO_ROOT not set")
endif()
if(NOT DEFINED TCODE_BUILD_INFO_IN OR TCODE_BUILD_INFO_IN STREQUAL "")
  message(FATAL_ERROR "TCODE_BUILD_INFO_IN not set")
endif()
if(NOT DEFINED TCODE_BUILD_INFO_OUT OR TCODE_BUILD_INFO_OUT STREQUAL "")
  message(FATAL_ERROR "TCODE_BUILD_INFO_OUT not set")
endif()

find_program(GIT_EXECUTABLE git)

set(TCODE_BUILD_GIT_DESCRIBE "unknown")
set(TCODE_BUILD_BUILDER "unknown")
set(TCODE_BUILD_DATE_UNIX "0")
if(GIT_EXECUTABLE)
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" -C "${TCODE_REPO_ROOT}" describe --long --always --dirty
    OUTPUT_VARIABLE _git_describe
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    RESULT_VARIABLE _git_res
  )
  if(_git_res EQUAL 0 AND NOT _git_describe STREQUAL "")
    set(TCODE_BUILD_GIT_DESCRIBE "${_git_describe}")
  endif()

  execute_process(
    COMMAND "${GIT_EXECUTABLE}" -C "${TCODE_REPO_ROOT}" log -1 --format=%an
    OUTPUT_VARIABLE _git_author
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    RESULT_VARIABLE _git_author_res
  )
  if(_git_author_res EQUAL 0 AND NOT _git_author STREQUAL "")
    set(TCODE_BUILD_BUILDER "${_git_author}")
  endif()

  execute_process(
    COMMAND "${GIT_EXECUTABLE}" -C "${TCODE_REPO_ROOT}" log -1 --format=%ct
    OUTPUT_VARIABLE _git_ct
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    RESULT_VARIABLE _git_ct_res
  )
  if(_git_ct_res EQUAL 0 AND NOT _git_ct STREQUAL "")
    set(TCODE_BUILD_DATE_UNIX "${_git_ct}")
  endif()
endif()

get_filename_component(_out_dir "${TCODE_BUILD_INFO_OUT}" DIRECTORY)
file(MAKE_DIRECTORY "${_out_dir}")

configure_file("${TCODE_BUILD_INFO_IN}" "${TCODE_BUILD_INFO_OUT}" @ONLY)

