cmake_minimum_required(VERSION 3.20)
project(CivSim CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

find_package(SDL3 QUIET CONFIG)
if(NOT SDL3_FOUND)
	message(STATUS "SDL3 not found on system – fetching from GitHub…")
	FetchContent_Declare(
		SDL3
		GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
		GIT_TAG        main
		GIT_SHALLOW    TRUE
	)
	FetchContent_MakeAvailable(SDL3)
else()
	message(STATUS "Using system SDL3")
endif()

message(STATUS "Fetching SDL3_ttf from GitHub…")
set(SDL3TTF_SAMPLES  OFF CACHE BOOL "" FORCE)
set(SDL3TTF_HARFBUZZ OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
	SDL3_ttf
	GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
	GIT_TAG        main
	GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(SDL3_ttf)

add_executable(CivSim
	src/main.cpp
	src/helper.cpp
	src/levelGeneration.cpp
)

target_include_directories(CivSim PRIVATE
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/src/vendor
)

target_link_libraries(CivSim PRIVATE
	SDL3::SDL3
	SDL3_ttf::SDL3_ttf
)
