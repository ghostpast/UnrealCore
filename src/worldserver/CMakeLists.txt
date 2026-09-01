add_executable(worldserver Main.cpp)

target_link_libraries(worldserver PRIVATE common)

add_custom_command(TARGET worldserver POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:libmariadb>
        $<TARGET_FILE:freetds_ct>
        $<TARGET_FILE_DIR:worldserver>
)
