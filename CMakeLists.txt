cmake_minimum_required(VERSION 3.16)

# Set the project name and version
project(toggl-cli VERSION 1.0 LANGUAGES CXX)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set default char type to unsigned
add_compile_definitions(CHAR_UNSIGNED)

# Add the main executable target
add_executable(toggl-cli
    src/cli.cpp
)

find_package(curl CONFIG REQUIRED)
find_package(glaze CONFIG REQUIRED)

find_path(CPPCODEC_INCLUDE_DIRS "cppcodec/base32_crockford.hpp")
target_include_directories(toggl-cli PRIVATE ${CPPCODEC_INCLUDE_DIRS})

target_link_libraries(toggl-cli  PRIVATE CURL::libcurl glaze::glaze)

# Precompiled headers for Windows/MSVC
if (MSVC)
    target_compile_options(toggl-cli PRIVATE /W4 /WX /MP /Zi /Od /bigobj)
    target_link_options(toggl-cli PRIVATE /SUBSYSTEM:CONSOLE /INCREMENTAL)
endif()
