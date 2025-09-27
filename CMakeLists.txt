cmake_minimum_required(VERSION 3.20)
project(Tsuki VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build SDL3 from vendor directory (completely self-contained)
set(SDL3_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/sdl3)
message(STATUS "Building SDL3 from vendor directory: ${SDL3_SOURCE_DIR}")

# Configure SDL3 build options - minimal for our needs
set(SDL_SHARED OFF CACHE BOOL "Build SDL3 as shared library")
set(SDL_STATIC ON CACHE BOOL "Build SDL3 as static library")
set(SDL_TEST OFF CACHE BOOL "Build SDL3 test programs")
set(SDL_TESTS OFF CACHE BOOL "Build SDL3 tests")

# Add SDL3 subdirectory to build it
add_subdirectory(${SDL3_SOURCE_DIR} ${CMAKE_BINARY_DIR}/vendor/sdl3)

# Set SDL3 variables for linking
set(SDL3_LIBRARIES SDL3::SDL3-static)
set(SDL3_INCLUDE_DIRS ${SDL3_SOURCE_DIR}/include)

# Build Lua from vendor directory (no external dependencies needed)
set(LUA_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/lua)

# Lua source files (core library)
set(LUA_CORE_SOURCES
    ${LUA_SOURCE_DIR}/lapi.c
    ${LUA_SOURCE_DIR}/lcode.c
    ${LUA_SOURCE_DIR}/lctype.c
    ${LUA_SOURCE_DIR}/ldebug.c
    ${LUA_SOURCE_DIR}/ldo.c
    ${LUA_SOURCE_DIR}/ldump.c
    ${LUA_SOURCE_DIR}/lfunc.c
    ${LUA_SOURCE_DIR}/lgc.c
    ${LUA_SOURCE_DIR}/llex.c
    ${LUA_SOURCE_DIR}/lmem.c
    ${LUA_SOURCE_DIR}/lobject.c
    ${LUA_SOURCE_DIR}/lopcodes.c
    ${LUA_SOURCE_DIR}/lparser.c
    ${LUA_SOURCE_DIR}/lstate.c
    ${LUA_SOURCE_DIR}/lstring.c
    ${LUA_SOURCE_DIR}/ltable.c
    ${LUA_SOURCE_DIR}/ltm.c
    ${LUA_SOURCE_DIR}/lundump.c
    ${LUA_SOURCE_DIR}/lvm.c
    ${LUA_SOURCE_DIR}/lzio.c
)

# Lua library sources (standard libraries)
set(LUA_LIB_SOURCES
    ${LUA_SOURCE_DIR}/lauxlib.c
    ${LUA_SOURCE_DIR}/lbaselib.c
    ${LUA_SOURCE_DIR}/lcorolib.c
    ${LUA_SOURCE_DIR}/ldblib.c
    ${LUA_SOURCE_DIR}/liolib.c
    ${LUA_SOURCE_DIR}/lmathlib.c
    ${LUA_SOURCE_DIR}/loslib.c
    ${LUA_SOURCE_DIR}/lstrlib.c
    ${LUA_SOURCE_DIR}/ltablib.c
    ${LUA_SOURCE_DIR}/lutf8lib.c
    ${LUA_SOURCE_DIR}/loadlib.c
    ${LUA_SOURCE_DIR}/linit.c
)

# Create Lua static library
add_library(lua STATIC ${LUA_CORE_SOURCES} ${LUA_LIB_SOURCES})

# Explicitly set Lua as C library
set_target_properties(lua PROPERTIES LINKER_LANGUAGE C)

# Set Lua include directory
target_include_directories(lua PUBLIC ${LUA_SOURCE_DIR})

# Set platform-specific compile definitions
if(WIN32)
    target_compile_definitions(lua PRIVATE LUA_BUILD_AS_DLL=0)
endif()

# Link math library on Unix systems
if(UNIX)
    target_link_libraries(lua m)
endif()

# Set variables for the rest of the build system
set(LUA_LIBRARIES lua)
set(LUA_INCLUDE_DIRS ${LUA_SOURCE_DIR})
set(LUA_INCLUDE_DIR ${LUA_SOURCE_DIR})

message(STATUS "Building Lua from vendor directory: ${LUA_SOURCE_DIR}")
message(STATUS "Lua include directory: ${LUA_INCLUDE_DIR}")


# Build libzip from vendor directory (completely self-contained)
set(LIBZIP_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/libzip)
message(STATUS "Building libzip from vendor directory: ${LIBZIP_SOURCE_DIR}")

# Configure libzip build options - minimal for our needs
set(ENABLE_COMMONCRYPTO OFF CACHE BOOL "Enable use of CommonCrypto")
set(ENABLE_GNUTLS OFF CACHE BOOL "Enable use of GnuTLS")
set(ENABLE_MBEDTLS OFF CACHE BOOL "Enable use of mbed TLS")
set(ENABLE_OPENSSL OFF CACHE BOOL "Enable use of OpenSSL")
set(ENABLE_WINDOWS_CRYPTO OFF CACHE BOOL "Enable use of Windows cryptography libraries")
set(ENABLE_BZIP2 OFF CACHE BOOL "Enable use of BZip2")
set(ENABLE_LZMA OFF CACHE BOOL "Enable use of LZMA")
set(ENABLE_ZSTD OFF CACHE BOOL "Enable use of Zstandard")
set(BUILD_TOOLS OFF CACHE BOOL "Build tools in the src directory")
set(BUILD_REGRESS OFF CACHE BOOL "Build regression tests")
set(BUILD_EXAMPLES OFF CACHE BOOL "Build examples")
set(BUILD_DOC OFF CACHE BOOL "Build documentation")

# Add libzip subdirectory to build it
add_subdirectory(${LIBZIP_SOURCE_DIR} ${CMAKE_BINARY_DIR}/vendor/libzip)

# Set libzip variables for linking
set(LIBZIP_LIBRARIES zip)
set(LIBZIP_INCLUDE_DIRS ${LIBZIP_SOURCE_DIR}/lib)
set(LIBZIP_LINK_LIBS zip)

# Note: SDL3 extensions (image, mixer, ttf) are omitted for minimal build
# These can be added later as additional submodules if needed

# Source files
file(GLOB_RECURSE TSUKI_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

# Create the library
add_library(libtsuki STATIC ${TSUKI_SOURCES})

# Target-specific include directories (modern CMake approach)
target_include_directories(libtsuki
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${SDL3_INCLUDE_DIRS}
        ${LUA_INCLUDE_DIRS}
        ${LUA_INCLUDE_DIR}
        ${LIBZIP_INCLUDE_DIRS}
)

# SDL3 extensions removed for minimal build

# Link libraries - all built from submodules
target_link_libraries(libtsuki ${SDL3_LIBRARIES} ${LUA_LIBRARIES} ${LIBZIP_LINK_LIBS})

# Link math library on Unix systems
if(UNIX)
    target_link_libraries(libtsuki m)
endif()

add_executable(tsuki src/runtime/main.cpp)
target_link_libraries(tsuki libtsuki ${LIBZIP_LINK_LIBS})

# Example executable
add_executable(tsuki_example examples/main.cpp)
target_link_libraries(tsuki_example libtsuki)

# Compiler flags
target_compile_options(libtsuki PRIVATE
    -Wall -Wextra -Wpedantic
)

# Install targets
install(TARGETS libtsuki
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/ DESTINATION include)