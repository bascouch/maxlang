
cmake_minimum_required(VERSION 3.19)

if (APPLE)
	set(CMAKE_OSX_ARCHITECTURES x86_64;arm64)
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

include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-posttarget.cmake)

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../externals/${${PROJECT_NAME}_EXTERN_OUTPUT_NAME}.mxo
                ${CMAKE_CURRENT_SOURCE_DIR}/${${PROJECT_NAME}_EXTERN_OUTPUT_NAME}.mxo)
