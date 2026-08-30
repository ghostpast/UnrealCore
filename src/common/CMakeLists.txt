add_library(common STATIC
        database.cpp
)

target_include_directories(common PUBLIC
        ${CMAKE_BINARY_DIR}/src/common
        ${CMAKE_CURRENT_SOURCE_DIR}
)