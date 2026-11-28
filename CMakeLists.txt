cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 17)

project(FingerFlicks)

#prefer static SDL3
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)

#prefer static SDL3_image
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_VENDORED OFF CACHE BOOl "" FORCE)


add_subdirectory(thirdparty/SDL3-3.4.20)
add_subdirectory(thirdparty/SDL3_image-3.4.8)


add_executable(FingerFlicks "src/main.cpp" "src/config.h" "src/player.h" "src/player.cpp" "src/draw.h" "src/draw.cpp" "src/effects.h" "src/effects.cpp" "src/enemy.h" "src/enemy.cpp" "src/game.h" "src/game.cpp"  "src/assets.h" "src/assets.cpp")
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE $<CONFIG:Release>)

if(PRODUCTION_BUILD)
	#SETUP the assets_path macro to be in the root folder of your exe
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="./resources/")
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION_BUILD=1)
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC DEVELOPMENT_BUILD=0)
else()
	#this is useful to get an ASSETS_PATH in your IDE during development
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION_BUILD=0)
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC DEVELOPMENT_BUILD=1)
endif()

target_link_libraries("${CMAKE_PROJECT_NAME}" PRIVATE SDL3::SDL3 SDL3_image::SDL3_image)