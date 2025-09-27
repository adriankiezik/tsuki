cmake_minimum_required(VERSION 3.20)
project(Tsuki VERSION 1.0.0 LANGUAGES C CXX)

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


# Find libzip for .tsuki file support
if(NOT WIN32)
    pkg_check_modules(LIBZIP REQUIRED libzip)
    # Use the full link libraries for proper linking
    set(LIBZIP_LINK_LIBS ${LIBZIP_LINK_LIBRARIES})
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

    set(LIBZIP_LINK_LIBS ${LIBZIP_LIBRARIES})
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
target_link_libraries(libtsuki ${SDL3_LIBRARIES} ${LUA_LIBRARIES} ${LIBZIP_LINK_LIBS})

# Link math library on Unix systems
if(UNIX)
    target_link_libraries(libtsuki m)
endif()

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
target_link_libraries(tsuki libtsuki ${LIBZIP_LINK_LIBS})

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