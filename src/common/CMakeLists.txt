add_library(common STATIC
        database.cpp
)

target_include_directories(common PUBLIC
        ${CMAKE_BINARY_DIR}/src/common
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/deps/asio/include
)

target_compile_definitions(common PUBLIC
        ASIO_STANDALONE
        ASIO_NO_DEPRECATED
)

if(WIN32)
    target_link_libraries(common PUBLIC ws2_32 wsock32)
else()
    find_package(Threads REQUIRED)
    target_link_libraries(common PUBLIC Threads::Threads)
endif()