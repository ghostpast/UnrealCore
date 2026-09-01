add_library(common STATIC
        database/DatabaseFactory.cpp
        database/mariadb/MariaDbConnection.cpp
        database/mariadb/MariaDbResult.cpp
        database/mariadb/MariaDbStatement.cpp
        database/mariadb/MariaDbStmtResult.cpp
)

# --- MariaDB Connector/C: динамическая сборка ---
set(WITH_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(WITH_MYSQLCOMPAT OFF CACHE BOOL "" FORCE)
set(WITH_CURL OFF CACHE BOOL "" FORCE)
set(WITH_ICONV OFF CACHE BOOL "" FORCE)
set(WITH_DOCS OFF CACHE BOOL "" FORCE)
set(WITH_SSL "SCHANNEL" CACHE STRING "" FORCE)
set(CLIENT_PLUGIN_DIALOG OFF CACHE STRING "" FORCE)
set(CLIENT_PLUGIN_AUTH_GSSAPI_CLIENT OFF CACHE STRING "" FORCE)
set(CLIENT_PLUGIN_CLEARTEXT OFF CACHE STRING "" FORCE)
set(CLIENT_PLUGIN_SHA256_PASSWORD DYNAMIC CACHE STRING "" FORCE)
set(CLIENT_PLUGIN_CACHING_SHA2_PASSWORD DYNAMIC CACHE STRING "" FORCE)
set(CLIENT_PLUGIN_MYSQL_CLEAR_PASSWORD OFF CACHE STRING "" FORCE)
set(CLIENT_PLUGIN_REMOTE_IO OFF CACHE STRING "" FORCE)
set(INSTALL_PLUGINDIR "lib/mariadb/plugin" CACHE STRING "" FORCE)

add_subdirectory(${CMAKE_SOURCE_DIR}/src/deps/mariadb-connector-c
        ${CMAKE_BINARY_DIR}/src/deps/mariadb-connector-c
        EXCLUDE_FROM_ALL)

target_include_directories(common PUBLIC
        ${CMAKE_BINARY_DIR}/src/common
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/deps/asio/include
        ${CMAKE_SOURCE_DIR}/src/deps/mariadb-connector-c/include
        ${CMAKE_BINARY_DIR}/src/deps/mariadb-connector-c/include
)

target_compile_definitions(common PUBLIC
        ASIO_STANDALONE
        ASIO_NO_DEPRECATED
)

target_link_libraries(common PUBLIC libmariadb PostgreSQL::PostgreSQL)

if(WIN32)
    target_link_libraries(common PUBLIC ws2_32 wsock32)
else()
    find_package(Threads REQUIRED)
    target_link_libraries(common PUBLIC Threads::Threads)
endif()
