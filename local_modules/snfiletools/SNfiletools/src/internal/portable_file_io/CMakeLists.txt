cmake_minimum_required(VERSION 3.24)

add_library(snfiletools_portable_file_io OBJECT
    portable_file_io.cpp
)

target_include_directories(snfiletools_portable_file_io
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(snfiletools_portable_file_io
    PUBLIC
        cxx_std_20
)

if(NOT WIN32)
    target_compile_definitions(snfiletools_portable_file_io
        PRIVATE
            _FILE_OFFSET_BITS=64
            _POSIX_C_SOURCE=200809L
    )
endif()

target_compile_options(snfiletools_portable_file_io
    PRIVATE
        "$<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wall>"
        "$<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wextra>"
        "$<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wpedantic>"
        "$<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Werror>"
        "$<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wconversion>"
        "$<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wsign-conversion>"
        "$<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wshadow>"
        "$<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wformat=2>"
        "$<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wundef>"
        "$<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wcast-qual>"
        "$<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wcast-align>"
        "$<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wnull-dereference>"
        "$<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wswitch-enum>"
        "$<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wfloat-equal>"
        "$<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wvla>"
        "$<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wdouble-promotion>"
        "$<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wold-style-cast>"
        "$<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Woverloaded-virtual>"
        "$<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wnon-virtual-dtor>"
        "$<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wzero-as-null-pointer-constant>"
        "$<$<CXX_COMPILER_ID:MSVC>:/W4>"
        "$<$<CXX_COMPILER_ID:MSVC>:/WX>"
        "$<$<CXX_COMPILER_ID:MSVC>:/permissive->"
)

set_target_properties(snfiletools_portable_file_io PROPERTIES
    CXX_EXTENSIONS OFF
)
