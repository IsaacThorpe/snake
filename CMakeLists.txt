cmake_minimum_required(VERSION 3.16)
project(snake_app CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------
# Paths to SFML
# -----------------------------
set(SFML_STATIC_LIBRARIES TRUE)
set(SFML_DIR "/usr/local/lib/cmake/SFML")
# For EVP
find_package(OpenSSL REQUIRED)

# -----------------------------
# Source files
# -----------------------------
file(GLOB_RECURSE SOURCES
    src/*.cpp
)

# -----------------------------
# Include dirs
# -----------------------------
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    /usr/local/include             # SFML include directory
)

# -----------------------------
# Executable
# -----------------------------
add_executable(snake_app ${SOURCES})

# -----------------------------
# SFML libraries (static)
# -----------------------------
target_link_libraries(snake_app PRIVATE
    sfml-graphics-s
    sfml-window-s
    sfml-system-s
    GL
    X11
    Xrandr
    Xi
    Xxf86vm
    Xcursor
    udev
    freetype
    png
    jpeg
    z
    pthread
    dl
    rt
    OpenSSL::Crypto
)

# -----------------------------
# Library search path
# -----------------------------
link_directories(
    /usr/local/lib
)

