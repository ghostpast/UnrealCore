add_executable(authserver main.cpp)

target_link_libraries(authserver PRIVATE common)

add_custom_command(TARGET authserver POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:libmariadb>
        $<TARGET_FILE:freetds_ct>
        $<TARGET_FILE_DIR:authserver>
)
