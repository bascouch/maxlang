
cmake_minimum_required(VERSION 3.19)

project(bufgranulp)

if (APPLE)
	set(CMAKE_OSX_ARCHITECTURES x86_64;arm64)
	set(SAF_PERFORMANCE_LIB "SAF_USE_APPLE_ACCELERATE")
endif ()

include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-pretarget.cmake)

#############################################################
# MAX EXTERNAL
#############################################################

include_directories(
	"${MAX_SDK_INCLUDES}"
	"${MAX_SDK_MSP_INCLUDES}"
	"${MAX_SDK_JIT_INCLUDES}"
	"${CMAKE_CURRENT_SOURCE_DIR}/../maxlang"
	"${CMAKE_CURRENT_SOURCE_DIR}/../maxlang/include"
)

file(GLOB PROJECT_SRC
	"${CMAKE_CURRENT_SOURCE_DIR}/../maxlang/*.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../maxlang/commonsyms.c"
     "*.h"
	 "*.hpp"
	 "*.c"
     "*.cpp"
)

add_library(
	${PROJECT_NAME}
	MODULE
	${PROJECT_SRC}
)

add_subdirectory(../libraries/Spatial_Audio_Framework saf)
target_link_libraries(${PROJECT_NAME} PRIVATE saf)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-posttarget.cmake)

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../externals/bufgranulp~.mxo
                ${CMAKE_CURRENT_SOURCE_DIR}/bufgranulp~.mxo)
