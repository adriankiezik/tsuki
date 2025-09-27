cmake_minimum_required(VERSION 3.20)
project(Tsuki VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)

# Find SDL3 (required)
pkg_check_modules(SDL3 REQUIRED sdl3)

# Find Lua (required for scripting)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LUA REQUIRED lua5.4)

# Also find the lua library explicitly
find_library(LUA_LIBRARY NAMES lua5.4 lua)
find_path(LUA_INCLUDE_DIR lua.h
    PATH_SUFFIXES lua5.4 lua
    PATHS
        /opt/homebrew/include
        /opt/homebrew/include/lua@5.4
        /usr/local/include
        /usr/local/include/lua@5.4
        /usr/include
)

if(NOT LUA_LIBRARY)
    message(FATAL_ERROR "Lua library not found")
endif()

message(STATUS "Found Lua library: ${LUA_LIBRARY}")
message(STATUS "Found Lua include: ${LUA_INCLUDE_DIR}")
message(STATUS "Lua pkg-config libraries: ${LUA_LIBRARIES}")
message(STATUS "Lua pkg-config include dirs: ${LUA_INCLUDE_DIRS}")


# Find libzip for .tsuki file support
pkg_check_modules(LIBZIP REQUIRED libzip)

# Find optional SDL3 extensions
pkg_check_modules(SDL3_IMAGE SDL3_image)
pkg_check_modules(SDL3_MIXER SDL3_mixer)
pkg_check_modules(SDL3_TTF SDL3_ttf)

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