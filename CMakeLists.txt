cmake_minimum_required(VERSION 3.10)
project(cartographer)

set(CMAKE_CXX_STANDARD 17)


find_package(OpenCV REQUIRED)


file(GLOB_RECURSE main_source src/*.cpp)
file(GLOB_RECURSE main_header src/*.h)
file(GLOB_RECURSE module_source modules/**.cpp)
file(GLOB_RECURSE module_header modules/**.h)

message(STATUS "MY_VARIABLE=${module_source}")


include_directories(include)
include_directories(modules)

add_executable(${PROJECT_NAME} ${main_source} ${module_source})

target_link_libraries(${PROJECT_NAME}
	seekcamera
    ${OpenCV_LIBS}
)


#--------------------------------------------------------------------------------------------------------------------------#
#Runtime dependencies
#--------------------------------------------------------------------------------------------------------------------------#
if(MSVC)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		$<TARGET_FILE_DIR:seekcamera>/seekcamera.dll
		$<TARGET_FILE_DIR:${PROJECT_NAME}>/seekcamera.dll
	)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		$<TARGET_FILE_DIR:SDL2>/SDL2.dll
		$<TARGET_FILE_DIR:${PROJECT_NAME}>/SDL2.dll
	)
endif()
