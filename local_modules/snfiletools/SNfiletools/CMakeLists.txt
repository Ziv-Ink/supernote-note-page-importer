cmake_minimum_required(VERSION 3.24)

project(SNfiletools LANGUAGES C CXX)

if(PROJECT_IS_TOP_LEVEL)
    include(CTest)
endif()

option(SNFILETOOLS_UNITY_BUILD
       "Build SNfiletools using unity compilation"
       ON)

option(SNFILETOOLS_TEST_PARALLEL_READS
       "Enable the Android parallel-read path on POSIX hosts for tests"
       OFF)
if(SNFILETOOLS_TEST_PARALLEL_READS AND WIN32)
    message(FATAL_ERROR "SNFILETOOLS_TEST_PARALLEL_READS requires positioned POSIX reads")
endif()

option(SNFILETOOLS_SANITIZERS "Instrument the library and its consumers with ASan and UBSan" OFF)
add_library(snfiletools_sanitizers INTERFACE)
if(SNFILETOOLS_SANITIZERS)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        message(FATAL_ERROR "SNFILETOOLS_SANITIZERS requires GCC or Clang")
    endif()
    target_compile_options(snfiletools_sanitizers INTERFACE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(snfiletools_sanitizers INTERFACE -fsanitize=address,undefined)
endif()

add_subdirectory(src/internal/portable_file_io)
find_package(Threads REQUIRED)

# Consumer-specific instrumentation belongs here, not in the independent library.
target_link_libraries(snfiletools_portable_file_io PRIVATE snfiletools_sanitizers)

add_library(snfiletools STATIC
    src/snfiletools.cpp
    src/internal/crc32.cpp
    src/internal/snfile.cpp
    src/internal/snfile.h
    src/internal/block_index.cpp
    src/internal/block_write_plan.cpp
    src/internal/undo_journal.cpp
)

add_library(SNfiletools::SNfiletools ALIAS snfiletools)

if(SNFILETOOLS_TEST_PARALLEL_READS)
    target_compile_definitions(snfiletools PRIVATE SNFILETOOLS_TEST_PARALLEL_READS=1)
endif()

target_include_directories(snfiletools
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_compile_features(snfiletools
    PUBLIC
        cxx_std_23
)

target_link_libraries(snfiletools
    PUBLIC snfiletools_sanitizers
    PRIVATE
        Threads::Threads
        snfiletools_portable_file_io
)

if(SNFILETOOLS_UNITY_BUILD)
    set_target_properties(snfiletools PROPERTIES
        UNITY_BUILD ON
        UNITY_BUILD_BATCH_SIZE 8
    )
endif()

set_target_properties(snfiletools PROPERTIES
    CXX_EXTENSIONS OFF
)

target_compile_options(snfiletools
    PRIVATE
        "$<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wall>"
        "$<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wextra>"
        "$<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wpedantic>"
        "$<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Werror>"
        "$<$<CXX_COMPILER_ID:MSVC>:/W4>"
        "$<$<CXX_COMPILER_ID:MSVC>:/WX>"
        "$<$<CXX_COMPILER_ID:MSVC>:/permissive->"
        "$<$<CXX_COMPILER_ID:MSVC>:/wd4100>"
        "$<$<CXX_COMPILER_ID:MSVC>:/wd4101>"
        "$<$<CXX_COMPILER_ID:MSVC>:/wd4189>"
        "$<$<CXX_COMPILER_ID:MSVC>:/wd4505>"
)
