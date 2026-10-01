cmake_minimum_required(VERSION 3.24)
project(snfiletools_bridge_tests LANGUAGES C CXX)
include(CTest)

# Exercise the same authored CMake integration used by the generated runtime.
set(SUPERNOTE_MODULE_TARGET snfiletools_bridge)
add_library(${SUPERNOTE_MODULE_TARGET} STATIC)
add_subdirectory(.. module)

set(SNFILETOOLS_TEST_FILES_DIR "" CACHE PATH
  "Directory containing the supplied notes/ and pdf_marks/ fixtures")
if(NOT IS_DIRECTORY "${SNFILETOOLS_TEST_FILES_DIR}/notes" OR
   NOT IS_DIRECTORY "${SNFILETOOLS_TEST_FILES_DIR}/pdf_marks")
  message(FATAL_ERROR "Set SNFILETOOLS_TEST_FILES_DIR to the supplied fixture directory")
endif()

add_executable(snfiletools_bridge_tests test_bridge.cpp)
target_compile_features(snfiletools_bridge_tests PRIVATE cxx_std_23)
target_include_directories(snfiletools_bridge_tests PRIVATE third_party ../SNfiletools/include)
target_compile_definitions(snfiletools_bridge_tests PRIVATE
  SN_TEST_FILES_DIR="${SNFILETOOLS_TEST_FILES_DIR}"
  SN_BITMAP_TEST_PATTERN="${CMAKE_CURRENT_SOURCE_DIR}/fixtures/bitmap-test.rle")
target_link_libraries(snfiletools_bridge_tests PRIVATE ${SUPERNOTE_MODULE_TARGET})
add_test(NAME snfiletools_bridge_tests COMMAND snfiletools_bridge_tests)
