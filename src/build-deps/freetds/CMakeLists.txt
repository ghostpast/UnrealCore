include(ExternalProject)

set(FREETDS_SRC_DIR "${CMAKE_SOURCE_DIR}/src/deps/freetds")
set(FREETDS_PREFIX  "${CMAKE_CURRENT_BINARY_DIR}/ep")
set(FREETDS_INSTALL "${CMAKE_CURRENT_BINARY_DIR}/install")

if(NOT EXISTS "${FREETDS_SRC_DIR}/configure.ac")
    message(FATAL_ERROR
            "FreeTDS submodule не инициализирован: git submodule update --init --recursive")
endif()

# --- Perl: нужен для генерации src/replacements/iconv_charsets.h ---
if(WIN32)
    get_filename_component(_git_root
            "[HKEY_LOCAL_MACHINE\\SOFTWARE\\GitForWindows;InstallPath]"
            ABSOLUTE CACHE)
    set(_perl_hints
            "${_git_root}/usr/bin"
            "$ENV{ProgramFiles}/Git/usr/bin"
            "$ENV{LOCALAPPDATA}/Programs/Git/usr/bin"
            "C:/Strawberry/perl/bin"
            "D:/Strawberry/perl/bin")
endif()

find_program(FREETDS_PERL NAMES perl HINTS ${_perl_hints})

if(NOT FREETDS_PERL)
    message(FATAL_ERROR
            "Perl не найден — он нужен FreeTDS для генерации iconv_charsets.h.\n"
            "  Windows: установить Strawberry Perl (strawberryperl.com) либо Git for Windows.\n"
            "  Linux:   apt install perl / dnf install perl\n"
            "Либо задать вручную: -DFREETDS_PERL=<путь к perl>")
endif()
message(STATUS "FreeTDS: perl = ${FREETDS_PERL}")

find_program(FREETDS_GPERF NAMES gperf)
if(NOT FREETDS_GPERF)
    message(FATAL_ERROR "gperf не найден. Windows: choco install gperf. Linux: apt install gperf")
endif()

get_property(_freetds_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(_freetds_multi_config)
    set(_freetds_build_cfg "$<CONFIG>")
    set(_freetds_cfg_dir   "/$<CONFIG>")   # VS/Xcode кладут артефакты в подкаталог конфигурации
else()
    set(_freetds_build_cfg "${CMAKE_BUILD_TYPE}")
    if(NOT _freetds_build_cfg)
        set(_freetds_build_cfg "Release")
    endif()
    set(_freetds_cfg_dir "")
endif()

# --- артефакты, о которых должен знать генератор сборки ---
set(_freetds_byproducts
        "<BINARY_DIR>/src/ctlib${_freetds_cfg_dir}/${CMAKE_SHARED_LIBRARY_PREFIX}ct${CMAKE_SHARED_LIBRARY_SUFFIX}")
if(WIN32)
    list(APPEND _freetds_byproducts
            "<BINARY_DIR>/src/ctlib${_freetds_cfg_dir}/${CMAKE_IMPORT_LIBRARY_PREFIX}ct${CMAKE_IMPORT_LIBRARY_SUFFIX}")
endif()

ExternalProject_Add(freetds_ep
        SOURCE_DIR        "${FREETDS_SRC_DIR}"
        PREFIX            "${FREETDS_PREFIX}"
        INSTALL_DIR       "${FREETDS_INSTALL}"
        DOWNLOAD_COMMAND  ""
        UPDATE_COMMAND    ""
        PATCH_COMMAND     ""
        CMAKE_GENERATOR   "${CMAKE_GENERATOR}"
        CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DCMAKE_BUILD_TYPE=${_freetds_build_cfg}
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DCMAKE_MSVC_RUNTIME_LIBRARY=${CMAKE_MSVC_RUNTIME_LIBRARY}
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5
        -DPERL_EXECUTABLE=${FREETDS_PERL}
        -DGPERF=${FREETDS_GPERF}
        -DWITH_OPENSSL=OFF          # включить после добавления OpenSSL
        -DENABLE_ODBC_WIDE=OFF
        -DENABLE_ODBC_MARS=OFF
        -DENABLE_KRB5=OFF
        -DENABLE_MSDBLIB=OFF
        BUILD_COMMAND
        ${CMAKE_COMMAND} --build <BINARY_DIR> --config ${_freetds_build_cfg} --target ct
        INSTALL_COMMAND   ""
        BUILD_BYPRODUCTS  ${_freetds_byproducts}
        LOG_CONFIGURE     OFF
        LOG_BUILD         OFF
)

ExternalProject_Get_Property(freetds_ep BINARY_DIR)

set(_freetds_ct_dir "${BINARY_DIR}/src/ctlib${_freetds_cfg_dir}")

set(FREETDS_CT_RUNTIME
        "${_freetds_ct_dir}/${CMAKE_SHARED_LIBRARY_PREFIX}ct${CMAKE_SHARED_LIBRARY_SUFFIX}")

add_library(freetds_ct SHARED IMPORTED GLOBAL)
add_dependencies(freetds_ct freetds_ep)

set_target_properties(freetds_ct PROPERTIES
        IMPORTED_LOCATION             "${FREETDS_CT_RUNTIME}"
        INTERFACE_INCLUDE_DIRECTORIES "${FREETDS_SRC_DIR}/include;${BINARY_DIR}/include"
)

if(WIN32)
    # на Windows линкуемся через импортную библиотеку
    set_target_properties(freetds_ct PROPERTIES
            IMPORTED_IMPLIB "${_freetds_ct_dir}/${CMAKE_IMPORT_LIBRARY_PREFIX}ct${CMAKE_IMPORT_LIBRARY_SUFFIX}")
else()
    # чтобы .so находился в рантайме без установки
    set_target_properties(freetds_ct PROPERTIES
            IMPORTED_NO_SONAME TRUE)
    set(CMAKE_BUILD_RPATH "${CMAKE_BUILD_RPATH};${_freetds_ct_dir}" PARENT_SCOPE)
endif()

# каталоги должны существовать на этапе конфигурации
file(MAKE_DIRECTORY "${FREETDS_SRC_DIR}/include" "${BINARY_DIR}/include")

add_library(freetds ALIAS freetds_ct)
