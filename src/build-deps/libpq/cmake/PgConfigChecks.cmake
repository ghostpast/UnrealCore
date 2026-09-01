include(CheckIncludeFile)
include(CheckSymbolExists)
include(CheckTypeSize)
include(CheckStructHasMember)
include(CheckCSourceCompiles)
include(TestBigEndian)

set(CMAKE_REQUIRED_QUIET ON)
if(UNIX)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)
    set(CMAKE_REQUIRED_LIBRARIES pthread)
endif()

# ---- версия из подмодуля ----
file(READ ${PG_ROOT}/configure.ac _cfg_ac)
string(REGEX MATCH "AC_INIT\\(\\[PostgreSQL\\], \\[([0-9.]+)\\]" _m "${_cfg_ac}")
set(PG_VERSION ${CMAKE_MATCH_1})
string(REGEX MATCH "^([0-9]+)\\.([0-9]+)" _m "${PG_VERSION}")
set(PG_MAJORVERSION ${CMAKE_MATCH_1})
set(PG_MAJORVERSION_NUM ${CMAKE_MATCH_1})
set(PG_MINORVERSION_NUM ${CMAKE_MATCH_2})
math(EXPR PG_VERSION_NUM "${PG_MAJORVERSION_NUM} * 10000 + ${PG_MINORVERSION_NUM}")

# ---- заголовки ----
foreach(h crypt dlfcn ifaddrs langinfo netinet/tcp poll pwd strings
        sys/epoll sys/event sys/personality sys/prctl sys/resource
        sys/select sys/signalfd sys/ucred sys/un termios ucred unistd)
    string(TOUPPER "HAVE_${h}_H" _v)
    string(REPLACE "/" "_" _v "${_v}")
    check_include_file("${h}.h" ${_v})
endforeach()

# ---- функции ----
set(_hdrs "stdlib.h;string.h;unistd.h;locale.h;pthread.h;time.h;fcntl.h;getopt.h;poll.h;signal.h;sys/socket.h;netdb.h;arpa/inet.h;ifaddrs.h")
foreach(f clock_gettime explicit_bzero getaddrinfo getifaddrs getopt
        getopt_long getpeereid getpeerucred inet_aton inet_pton memset_s
        mkdtemp poll posix_fallocate ppoll pthread_barrier_wait
        pthread_is_threaded_np readlink setsid strchrnul strerror_r
        strlcat strlcpy strnlen strsignal sync_file_range unsetenv uselocale)
    string(TOUPPER "HAVE_${f}" _v)
    check_symbol_exists(${f} "${_hdrs}" ${_v})
endforeach()

# ---- типы, размеры, структуры ----
check_type_size("long long int" HAVE_INT64 LANGUAGE C)
check_type_size("bool"   SIZEOF_BOOL   LANGUAGE C)
check_type_size("long"   SIZEOF_LONG   LANGUAGE C)
check_type_size("off_t"  SIZEOF_OFF_T  LANGUAGE C)
check_type_size("size_t" SIZEOF_SIZE_T LANGUAGE C)
check_type_size("void *" SIZEOF_VOID_P LANGUAGE C)
foreach(_s BOOL LONG OFF_T SIZE_T VOID_P)
    if(NOT SIZEOF_${_s})
        set(SIZEOF_${_s} 0)
    endif()
endforeach()
set(ALIGNOF_LONG ${SIZEOF_LONG})

check_symbol_exists(locale_t "locale.h" HAVE_LOCALE_T)
if(WIN32)
    set(HAVE_STRUCT_ADDRINFO 1)
    set(HAVE_STRUCT_SOCKADDR_STORAGE 1)
    set(HAVE_STRUCT_SOCKADDR_STORAGE_SS_FAMILY 1)
else()
    check_struct_has_member("struct addrinfo" ai_family "netdb.h" HAVE_STRUCT_ADDRINFO)
    check_struct_has_member("struct sockaddr_storage" ss_family "sys/socket.h" HAVE_STRUCT_SOCKADDR_STORAGE_SS_FAMILY)
    set(HAVE_STRUCT_SOCKADDR_STORAGE ${HAVE_STRUCT_SOCKADDR_STORAGE_SS_FAMILY})
    check_struct_has_member("struct sockaddr_un" sun_family "sys/un.h" HAVE_STRUCT_SOCKADDR_UN)
endif()
check_struct_has_member("struct option" name "getopt.h" HAVE_STRUCT_OPTION)

check_c_source_compiles("int main(void){__int128 x=0;return (int)x;}" HAVE_INT128)
if(HAVE_INT128)
    set(PG_INT128_TYPE "PG_INT128_TYPE __int128")
endif()
test_big_endian(WORDS_BIGENDIAN)

# ---- атрибуты компилятора ----
check_c_source_compiles("int main(void){return __builtin_bswap16(1);}"  HAVE_BUILTIN_BSWAP16)
check_c_source_compiles("int main(void){return __builtin_bswap32(1);}"  HAVE_BUILTIN_BSWAP32)
check_c_source_compiles("int main(void){return (int)__builtin_bswap64(1);}" HAVE_BUILTIN_BSWAP64)
check_c_source_compiles("int main(void){return __builtin_clz(1u);}"     HAVE_BUILTIN_CLZ)
check_c_source_compiles("int main(void){return __builtin_ctz(1u);}"     HAVE_BUILTIN_CTZ)
check_c_source_compiles("int main(void){return __builtin_popcount(1u);}" HAVE_BUILTIN_POPCOUNT)
check_c_source_compiles("int main(void){__builtin_unreachable();}"      HAVE_BUILTIN_UNREACHABLE)
check_c_source_compiles("int main(void){void *l[]={&&a};goto *l[0];a:return 0;}" HAVE_COMPUTED_GOTO)
check_c_source_compiles("int main(void){const char *s=__func__;return !s;}"     HAVE_FUNCNAME__FUNC)
check_c_source_compiles("int main(void){const char *s=__FUNCTION__;return !s;}" HAVE_FUNCNAME__FUNCTION)
check_c_source_compiles("_Static_assert(1,\"ok\"); int main(void){return 0;}"   HAVE__STATIC_ASSERT)
check_c_source_compiles("
  extern __attribute__((visibility(\"default\"))) int f(void);
  int f(void){return 0;} int main(void){return f();}" HAVE_VISIBILITY_ATTRIBUTE)

if(NOT MSVC)
    set(PG_PRINTF_ATTRIBUTE "PG_PRINTF_ATTRIBUTE printf")
endif()

unset(CMAKE_REQUIRED_QUIET)
unset(CMAKE_REQUIRED_LIBRARIES)
