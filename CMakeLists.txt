cmake_minimum_required(VERSION 3.20)
project(Tsuki VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find SDL3 (required)
if(NOT WIN32)
    # Use pkg-config on Unix systems
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL3 REQUIRED sdl3)
else()
    # Manual detection for Windows
    find_path(SDL3_INCLUDE_DIRS SDL3/SDL.h
        PATHS
            ${CMAKE_PREFIX_PATH}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/../SDL3/include
            ${CMAKE_CURRENT_SOURCE_DIR}/SDL3/include
    )
    find_library(SDL3_LIBRARIES NAMES SDL3 SDL3-static
        PATHS
            ${CMAKE_PREFIX_PATH}/lib
            ${CMAKE_CURRENT_SOURCE_DIR}/../SDL3/lib
            ${CMAKE_CURRENT_SOURCE_DIR}/SDL3/lib
    )

    if(NOT SDL3_INCLUDE_DIRS OR NOT SDL3_LIBRARIES)
        message(FATAL_ERROR "SDL3 not found. Please set CMAKE_PREFIX_PATH to SDL3 installation directory.")
    endif()

    message(STATUS "Found SDL3 include: ${SDL3_INCLUDE_DIRS}")
    message(STATUS "Found SDL3 library: ${SDL3_LIBRARIES}")
endif()

# Find Lua (required for scripting)
if(NOT WIN32)
    # Use pkg-config on Unix systems
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LUA REQUIRED lua5.4)
else()
    # Manual detection for Windows
    find_package(Lua REQUIRED)
    if(LUA_FOUND)
        set(LUA_LIBRARIES ${LUA_LIBRARIES})
        set(LUA_INCLUDE_DIRS ${LUA_INCLUDE_DIR})
    endif()
endif()

# Also find the lua library explicitly as fallback
find_library(LUA_LIBRARY NAMES lua5.4 lua lua54)
find_path(LUA_INCLUDE_DIR lua.h
    PATH_SUFFIXES lua5.4 lua lua54 include
    PATHS
        ${LUA_INCLUDE_DIRS}
        /opt/homebrew/include
        /opt/homebrew/include/lua@5.4
        /usr/local/include
        /usr/local/include/lua@5.4
        /usr/local/include/lua5.4
        /usr/include
        /usr/include/lua5.4
        ${LUA_INCLUDE_DIR}
    NO_DEFAULT_PATH
)

# Fallback: try default paths if not found
if(NOT LUA_INCLUDE_DIR)
    find_path(LUA_INCLUDE_DIR lua.h
        PATH_SUFFIXES lua5.4 lua lua54 include
    )
endif()

if(NOT LUA_LIBRARY)
    message(FATAL_ERROR "Lua library not found")
endif()

if(NOT LUA_INCLUDE_DIR)
    message(FATAL_ERROR "Lua headers not found")
endif()

message(STATUS "Found Lua library: ${LUA_LIBRARY}")
message(STATUS "Found Lua include: ${LUA_INCLUDE_DIR}")
if(NOT WIN32)
    message(STATUS "Lua pkg-config libraries: ${LUA_LIBRARIES}")
    message(STATUS "Lua pkg-config include dirs: ${LUA_INCLUDE_DIRS}")
endif()


# Find libzip for .tsuki file support
if(NOT WIN32)
    pkg_check_modules(LIBZIP REQUIRED libzip)
else()
    # Manual detection for Windows
    find_path(LIBZIP_INCLUDE_DIRS zip.h
        PATHS
            ${CMAKE_PREFIX_PATH}/include
            ${VCPKG_ROOT}/installed/x64-windows/include
    )
    find_library(LIBZIP_LIBRARIES NAMES zip libzip
        PATHS
            ${CMAKE_PREFIX_PATH}/lib
            ${VCPKG_ROOT}/installed/x64-windows/lib
    )

    if(NOT LIBZIP_INCLUDE_DIRS OR NOT LIBZIP_LIBRARIES)
        message(FATAL_ERROR "libzip not found. Please install via vcpkg or set CMAKE_PREFIX_PATH.")
    endif()

    message(STATUS "Found libzip include: ${LIBZIP_INCLUDE_DIRS}")
    message(STATUS "Found libzip library: ${LIBZIP_LIBRARIES}")
endif()

# Find optional SDL3 extensions (only on Unix for now)
if(NOT WIN32)
    pkg_check_modules(SDL3_IMAGE SDL3_image)
    pkg_check_modules(SDL3_MIXER SDL3_mixer)
    pkg_check_modules(SDL3_TTF SDL3_ttf)
endif()

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

if(SDL3_IMAGE_FOUND)
    target_include_directories(libtsuki PUBLIC ${SDL3_IMAGE_INCLUDE_DIRS})
    target_compile_definitions(libtsuki PUBLIC TSUKI_HAS_SDL_IMAGE)
endif()

if(SDL3_MIXER_FOUND)
    target_include_directories(libtsuki PUBLIC ${SDL3_MIXER_INCLUDE_DIRS})
    target_compile_definitions(libtsuki PUBLIC TSUKI_HAS_SDL_MIXER)
endif()

if(SDL3_TTF_FOUND)
    target_include_directories(libtsuki PUBLIC ${SDL3_TTF_INCLUDE_DIRS})
    target_compile_definitions(libtsuki PUBLIC TSUKI_HAS_SDL_TTF)
endif()

# Link libraries
target_link_libraries(libtsuki ${SDL3_LIBRARIES} ${LUA_LIBRARY} ${LUA_LIBRARIES} ${LIBZIP_LIBRARIES} m)

if(SDL3_IMAGE_FOUND)
    target_link_libraries(libtsuki ${SDL3_IMAGE_LIBRARIES})
endif()

if(SDL3_MIXER_FOUND)
    target_link_libraries(libtsuki ${SDL3_MIXER_LIBRARIES})
endif()

if(SDL3_TTF_FOUND)
    target_link_libraries(libtsuki ${SDL3_TTF_LIBRARIES})
endif()

add_executable(tsuki src/runtime/main.cpp)
target_link_libraries(tsuki libtsuki ${LIBZIP_LIBRARIES})

# Example executable
add_executable(tsuki_example examples/main.cpp)
target_link_libraries(tsuki_example libtsuki)

# Compiler flags
target_compile_options(libtsuki PRIVATE
    ${SDL3_CFLAGS_OTHER}
    -Wall -Wextra -Wpedantic
)

if(SDL3_IMAGE_FOUND)
    target_compile_options(libtsuki PRIVATE ${SDL3_IMAGE_CFLAGS_OTHER})
endif()

if(SDL3_MIXER_FOUND)
    target_compile_options(libtsuki PRIVATE ${SDL3_MIXER_CFLAGS_OTHER})
endif()

if(SDL3_TTF_FOUND)
    target_compile_options(libtsuki PRIVATE ${SDL3_TTF_CFLAGS_OTHER})
endif()

# Install targets
install(TARGETS libtsuki
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/ DESTINATION include)