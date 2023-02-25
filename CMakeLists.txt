cmake_minimum_required(VERSION 3.10)

project(raytracing)

include_directories(${CMAKE_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 17)

set(SFML_DIR "$DEV/lib/cmake/SFML")
find_package(SFML 2.0 COMPONENTS graphics REQUIRED)

add_executable(ray common/main.cpp)

add_subdirectory(common/hot_reload_draw)

target_link_libraries(ray sfml-graphics)
add_dependencies(ray hot_reload_draw)

target_compile_definitions(ray
		PRIVATE
			HOT_RELOAD_LIB_FULL_PATH="$<TARGET_FILE:hot_reload_draw>"
			HOT_RELOAD_LIB_COPY_PATH="$<TARGET_FILE_PREFIX:hot_reload_draw>$<TARGET_FILE_BASE_NAME:hot_reload_draw>_copy$<TARGET_FILE_SUFFIX:hot_reload_draw>")


if (APPLE)
	target_link_options(ray
			PRIVATE -rpath $ENV{DEV}/lib)
	target_sources(ray
			PRIVATE platform/unix.cpp)
endif()

