cmake_minimum_required(VERSION 3.24)
cmake_policy(SET CMP0131 NEW)
if(NOT DEFINED SUPERNOTE_MODULE_TARGET OR
   NOT TARGET "${SUPERNOTE_MODULE_TARGET}")
  message(FATAL_ERROR
    "SUPERNOTE_MODULE_TARGET must name the generated feature target")
endif()

target_sources(${SUPERNOTE_MODULE_TARGET} PRIVATE
  android/src/main/cpp/feature.cpp
  tests/device_edit.cpp)

# The library and its portable I/O objects are linked into the shared runtime.
add_subdirectory(SNfiletools)
set_target_properties(snfiletools snfiletools_portable_file_io PROPERTIES
  POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${SUPERNOTE_MODULE_TARGET} PRIVATE
  SNfiletools::SNfiletools)
