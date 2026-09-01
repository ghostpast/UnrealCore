cmake_minimum_required(VERSION 4.4.2)
project(libpq_build C)

set(PG_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/libpq)
set(PG_SRC  ${PG_ROOT}/src)
set(GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/include)

if(NOT EXISTS ${PG_SRC}/interfaces/libpq/fe-connect.c)
    message(FATAL_ERROR "submodule src/deps/libpq не инициализирован")
endif()

# ---- определение платформы ----
if(WIN32)
    set(PG_OS_HEADER win32.h)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(PG_OS_HEADER linux.h)
else()
    message(FATAL_ERROR "неподдерживаемая платформа: ${CMAKE_SYSTEM_NAME}")
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "поддерживается только 64-bit сборка")
endif()

# ---- проверки конфигурации ----
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/PgConfigChecks.cmake)

# ---- сгенерированные заголовки ----
file(MAKE_DIRECTORY ${GEN_DIR})
configure_file(config/pg_config.h.in       ${GEN_DIR}/pg_config.h)
configure_file(config/pg_config_ext.h.in   ${GEN_DIR}/pg_config_ext.h)
configure_file(config/pg_config_os.h.in    ${GEN_DIR}/pg_config_os.h)
configure_file(config/pg_config_paths.h.in ${GEN_DIR}/pg_config_paths.h)

# ---- исходники libpq (Makefile, REL_18_6, без SSL/GSS) --------------------
set(LIBPQ_SRC
        fe-auth.c fe-auth-scram.c fe-auth-oauth.c fe-cancel.c fe-connect.c
        fe-exec.c fe-lobj.c fe-misc.c fe-print.c fe-protocol3.c fe-secure.c
        fe-trace.c legacy-pqsignal.c libpq-events.c pqexpbuffer.c)
if(WIN32)
    list(APPEND LIBPQ_SRC pthread-win32.c win32.c)
endif()
list(TRANSFORM LIBPQ_SRC PREPEND ${PG_SRC}/interfaces/libpq/)

# ---- pgport / pgcommon (стартовый набор, дополняем по undefined refs) -----
set(PGPORT_SRC
        noblock.c path.c pg_strong_random.c pgsleep.c pgstrcasecmp.c pqsignal.c
        snprintf.c strerror.c strlcpy.c strlcat.c inet_net_ntop.c
        chklocale.c pg_bitutils.c
        explicit_bzero.c timingsafe_bcmp.c win32gettimeofday.c strsep.c)

if(WIN32)
    list(APPEND PGPORT_SRC
            getpeereid.c open.c dirmod.c system.c
            win32common.c win32error.c win32ntdll.c win32security.c win32setlocale.c
            win32stat.c win32gai_strerror.c)
endif()
list(TRANSFORM PGPORT_SRC PREPEND ${PG_SRC}/port/)

set(PGCOMMON_SRC
        base64.c cryptohash.c encnames.c hmac.c ip.c link-canary.c md5.c
        md5_common.c percentrepl.c pg_prng.c saslprep.c scram-common.c sha1.c
        sha2.c string.c stringinfo.c unicode_norm.c wchar.c
        jsonapi.c fe_memutils.c psprintf.c logging.c)

list(TRANSFORM PGCOMMON_SRC PREPEND ${PG_SRC}/common/)

# ---- цель -----------------------------------------------------------------
# отсеиваем файлы, отсутствующие в текущей версии postgres
set(_all_src ${PGPORT_SRC} ${PGCOMMON_SRC} ${LIBPQ_SRC})
set(_missing "")
foreach(_f IN LISTS _all_src)
    if(NOT EXISTS "${_f}")
        list(APPEND _missing "${_f}")
    endif()
endforeach()
if(_missing)
    message(WARNING "libpq: отсутствуют исходники, исключены:\n  ${_missing}")
    list(REMOVE_ITEM PGPORT_SRC   ${_missing})
    list(REMOVE_ITEM PGCOMMON_SRC ${_missing})
    list(REMOVE_ITEM LIBPQ_SRC    ${_missing})
endif()

add_library(pq SHARED ${LIBPQ_SRC} ${PGPORT_SRC} ${PGCOMMON_SRC})
add_library(libpq ALIAS pq)

target_include_directories(pq
        PRIVATE
        ${GEN_DIR}
        ${PG_SRC}/include
        ${PG_SRC}/port
        ${PG_SRC}/interfaces/libpq
        $<$<BOOL:${MSVC}>:${PG_SRC}/include/port/win32_msvc>
        $<$<BOOL:${WIN32}>:${PG_SRC}/include/port/win32>
        INTERFACE
        $<BUILD_INTERFACE:${GEN_DIR}>
        $<BUILD_INTERFACE:${PG_SRC}/include>
        $<BUILD_INTERFACE:${PG_SRC}/interfaces/libpq>)

target_compile_definitions(pq PRIVATE FRONTEND SO_MAJOR_VERSION=5)
set_target_properties(pq PROPERTIES
        C_STANDARD 11 OUTPUT_NAME $<IF:$<BOOL:${WIN32}>,libpq,pq>
        VERSION 5.18 SOVERSION 5 POSITION_INDEPENDENT_CODE ON)

if(WIN32)
    target_compile_definitions(pq PRIVATE WIN32_LEAN_AND_MEAN _CRT_SECURE_NO_WARNINGS)
    target_link_libraries(pq PRIVATE ws2_32 secur32 shell32 advapi32)
    # экспорт по exports.txt -> .def
    set(DEF_FILE ${CMAKE_CURRENT_BINARY_DIR}/libpq.def)
    add_custom_command(OUTPUT ${DEF_FILE}
            COMMAND ${CMAKE_COMMAND}
            -DIN=${PG_SRC}/interfaces/libpq/exports.txt -DOUT=${DEF_FILE}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenDef.cmake
            DEPENDS ${PG_SRC}/interfaces/libpq/exports.txt VERBATIM)
    add_custom_target(pq_def DEPENDS ${DEF_FILE})
    add_dependencies(pq pq_def)
    target_sources(pq PRIVATE ${DEF_FILE})
else()
    target_link_libraries(pq PRIVATE pthread m dl)
    set_target_properties(pq PROPERTIES C_VISIBILITY_PRESET hidden)

    # exports.txt -> version script libpq.map
    set(MAP_FILE ${CMAKE_CURRENT_BINARY_DIR}/libpq.map)
    add_custom_command(OUTPUT ${MAP_FILE}
            COMMAND ${CMAKE_COMMAND}
            -DIN=${PG_SRC}/interfaces/libpq/exports.txt -DOUT=${MAP_FILE}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenMap.cmake
            DEPENDS ${PG_SRC}/interfaces/libpq/exports.txt VERBATIM)
    add_custom_target(pq_map DEPENDS ${MAP_FILE})
    add_dependencies(pq pq_map)

    target_link_options(pq PRIVATE
            "LINKER:--version-script=${MAP_FILE}"
            "LINKER:--no-undefined")
    target_compile_options(pq PRIVATE
            -Wno-unused-parameter -Wno-sign-compare -Wno-missing-field-initializers)
endif()
