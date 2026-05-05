cmake_minimum_required(VERSION 3.20)
project(CivSim CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(FetchContent)
# SDL3
find_package(SDL3 QUIET CONFIG)
if(NOT SDL3_FOUND)
    message(STATUS "SDL3 not found on system – fetching from GitHub…")
    FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG        release-3.2.0
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(SDL3)
else()
    message(STATUS "Using system SDL3")
endif()
# SDL3_ttf
find_package(SDL3_ttf QUIET CONFIG)
if(NOT SDL3_ttf_FOUND)
    message(STATUS "SDL3_ttf not found on system – fetching from GitHub…")
    set(SDL3TTF_SAMPLES  OFF CACHE BOOL "" FORCE)
    set(SDL3TTF_HARFBUZZ OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        SDL3_ttf
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
        GIT_TAG        release-3.2.0
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(SDL3_ttf)
else()
    message(STATUS "Using system SDL3_ttf")
endif()
# SDL3_image
find_package(SDL3_image QUIET CONFIG)
if(NOT SDL3_image_FOUND)
    message(STATUS "SDL3_image not found on system – fetching from GitHub…")
    set(SDL3IMAGE_SAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        SDL3_image
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
        GIT_TAG        release-3.2.0
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(SDL3_image)
else()
    message(STATUS "Using system SDL3_image")
endif()
add_executable(CivSim
    src/main.cpp
    src/helper.cpp
    src/levelGeneration.cpp
    src/creature.cpp
	src/civ.cpp
	src/pathFinding.cpp
	src/logs.hpp
)
target_include_directories(CivSim PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/vendor
)
target_link_libraries(CivSim PRIVATE
    SDL3::SDL3
    SDL3_ttf::SDL3_ttf
    SDL3_image::SDL3_image
)
