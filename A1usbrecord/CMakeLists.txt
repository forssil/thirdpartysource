cmake_minimum_required(VERSION 3.10)

project(A1usbrecord LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

add_executable(av_virtual
    src/main.cpp
    src/pcm_device.cpp
    src/audio_muxer.cpp
    src/pcm_chunk_queue.cpp
)

target_include_directories(av_virtual
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(av_virtual PRIVATE Threads::Threads)

find_path(TINYALSA_INCLUDE_DIR tinyalsa/asoundlib.h)
find_library(TINYALSA_LIB tinyalsa)

if(TINYALSA_INCLUDE_DIR AND TINYALSA_LIB)
    target_include_directories(av_virtual PRIVATE ${TINYALSA_INCLUDE_DIR})
    target_link_libraries(av_virtual PRIVATE ${TINYALSA_LIB})
    target_compile_definitions(av_virtual PRIVATE A1USBRECORD_HAS_TINYALSA=1)
else()
    target_compile_definitions(av_virtual PRIVATE A1USBRECORD_HAS_TINYALSA=0)
    message(WARNING "tinyalsa headers or library were not found. PCM access will be built as a dry-run stub.")
endif()
