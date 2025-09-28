cmake_minimum_required(VERSION 3.20)
project(Tsuki VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build options

# Generate version header from template
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/tsuki/version.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/tsuki/version.hpp"
    @ONLY
)

# SDL3 - minimal static build
set(SDL_SHARED OFF CACHE BOOL "")
set(SDL_STATIC ON CACHE BOOL "")
set(SDL_TEST OFF CACHE BOOL "")
set(SDL_TESTS OFF CACHE BOOL "")
add_subdirectory(vendor/sdl3)

# Lua - build from source files
file(GLOB LUA_SOURCES vendor/lua/*.c)
list(REMOVE_ITEM LUA_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/lua/lua.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/lua/luac.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/lua/onelua.c)
add_library(lua STATIC ${LUA_SOURCES})
set_target_properties(lua PROPERTIES LINKER_LANGUAGE C)
target_include_directories(lua PUBLIC vendor/lua)
if(UNIX)
    target_link_libraries(lua m)
endif()

# zlib - build from source
add_subdirectory(vendor/zlib)

# libzip - minimal build with our zlib
set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
set(ENABLE_COMMONCRYPTO OFF CACHE BOOL "")
set(ENABLE_GNUTLS OFF CACHE BOOL "")
set(ENABLE_MBEDTLS OFF CACHE BOOL "")
set(ENABLE_OPENSSL OFF CACHE BOOL "")
set(ENABLE_WINDOWS_CRYPTO OFF CACHE BOOL "")
set(ENABLE_BZIP2 OFF CACHE BOOL "")
set(ENABLE_LZMA OFF CACHE BOOL "")
set(ENABLE_ZSTD OFF CACHE BOOL "")
set(BUILD_TOOLS OFF CACHE BOOL "")
set(BUILD_REGRESS OFF CACHE BOOL "")
set(BUILD_EXAMPLES OFF CACHE BOOL "")
set(BUILD_DOC OFF CACHE BOOL "")
set(LIBZIP_DO_INSTALL OFF CACHE BOOL "")
set(ZLIB_LIBRARY zlibstatic)
set(ZLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/zlib)
set(ZLIB_FOUND TRUE)
add_subdirectory(vendor/libzip)

# Main library
file(GLOB_RECURSE TSUKI_SOURCES src/*.cpp src/*.hpp)
add_library(libtsuki STATIC ${TSUKI_SOURCES})
target_include_directories(libtsuki
    PUBLIC
        include
        ${CMAKE_CURRENT_BINARY_DIR}/include
    PRIVATE
        vendor/lua
        vendor/libzip/lib
        vendor/zlib
)
target_link_libraries(libtsuki SDL3::SDL3-static lua zip zlibstatic)
if(UNIX)
    target_link_libraries(libtsuki m)
endif()
if(MSVC)
    target_compile_options(libtsuki PRIVATE /W4)
else()
    target_compile_options(libtsuki PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Main executable
add_executable(tsuki src/cli/main.cpp)
target_link_libraries(tsuki libtsuki)


# Install
install(TARGETS libtsuki ARCHIVE DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)