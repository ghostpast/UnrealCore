cmake_minimum_required(VERSION 4.4.2)
project(UnrealCore)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "Only x64 (64-bit) builds are supported!")
endif()

if(MSVC)
    add_compile_options(/utf-8)
endif()

if(WIN32)
    add_compile_definitions(WIN32_LEAN_AND_MEAN NOMINMAX)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-finput-charset=UTF-8)
endif()

# --- Генерация revision.h ---
find_package(Git QUIET)

if(GIT_EXECUTABLE)
    execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            OUTPUT_VARIABLE REVISION_ID
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
    )
    execute_process(
            COMMAND ${GIT_EXECUTABLE} show --quiet --date=iso-strict --format=%ad ${REVISION_ID}
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            OUTPUT_VARIABLE REVISION_DATE
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
    )
    execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            OUTPUT_VARIABLE REVISION_BRANCH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
    )
endif()

if(NOT REVISION_ID)
    set(REVISION_ID "Git not found")
endif()
if(NOT REVISION_DATE)
    set(REVISION_DATE "0000-00-00T00:00:00+00:00")
endif()
if(NOT REVISION_BRANCH)
    set(REVISION_BRANCH "unknown")
endif()

string(TIMESTAMP REVISION_TIME "%Y-%m-%d %H:%M:%S")

configure_file(
        ${CMAKE_SOURCE_DIR}/src/common/revision.h.in
        ${CMAKE_BINARY_DIR}/src/common/revision.h
        @ONLY
)
# --- конец генерации ---

add_subdirectory(src/common)
add_subdirectory(src/authserver)
add_subdirectory(src/worldserver)
