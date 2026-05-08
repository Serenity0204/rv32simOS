cmake_minimum_required(VERSION 3.10)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
project(rv32umos CXX ASM)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra)

include_directories(
    core/
    core/common
    core/disk
    core/filesystem
    core/hardware
    core/interrupt
    core/kernel
    core/memory
    core/process
    core/scheduler
    core/sync
    core/syscall
    core/thread
)

file(GLOB_RECURSE SOURCES 
    "core/RV32UMOS.cpp"
    "core/main.cpp"
    "core/*.cpp"
    "core/*.S"
)

add_executable(rv32umos ${SOURCES})

file(GLOB PROGRAM_SOURCES "programs/*.c")
set(PROGRAM_BINS "")

foreach(SOURCE_FILE ${PROGRAM_SOURCES})
    get_filename_component(NAME_WE ${SOURCE_FILE} NAME_WE)
    set(OUTPUT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/programs/${NAME_WE}.bin")
    
    add_custom_command(
        OUTPUT ${OUTPUT_FILE}
        COMMAND ${CMAKE_SOURCE_DIR}/scripts/compile.sh ${SOURCE_FILE}
        DEPENDS ${SOURCE_FILE}
        COMMENT "[PROGRAM] Compiling ${NAME_WE}.c"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    
    list(APPEND PROGRAM_BINS ${OUTPUT_FILE})
endforeach()

add_custom_target(programs ALL DEPENDS ${PROGRAM_BINS})
set_directory_properties(PROPERTIES ADDITIONAL_CLEAN_FILES "${PROGRAM_BINS}")