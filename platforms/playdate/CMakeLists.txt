cmake_minimum_required(VERSION 3.14)
set(CMAKE_C_STANDARD 11)

# Get SDK path from environment or config
set(ENVSDK $ENV{PLAYDATE_SDK_PATH})

if (NOT ${ENVSDK} STREQUAL "")
    file(TO_CMAKE_PATH ${ENVSDK} SDK)
else()
    execute_process(
        COMMAND bash -c "egrep '^\\s*SDKRoot' $HOME/.Playdate/config"
        COMMAND head -n 1
        COMMAND cut -c9-
        OUTPUT_VARIABLE SDK
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()

if (NOT EXISTS ${SDK})
    message(FATAL_ERROR "SDK Path not found; set ENV value PLAYDATE_SDK_PATH")
    return()
endif()

set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
set(CMAKE_XCODE_GENERATE_SCHEME TRUE)

# Game name customization
set(PLAYDATE_GAME_NAME Downland)
set(PLAYDATE_GAME_DEVICE Downland_DEVICE)

project(${PLAYDATE_GAME_NAME} C ASM)

# Collect source and header files
file(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)
file(GLOB_RECURSE GAME_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/../../game/*.c"
)
file(GLOB_RECURSE ROOMS_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/../../game/rooms/*.c"
)

file(GLOB_RECURSE SRC_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)
file(GLOB_RECURSE GAME_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/../../game/*.h"
)
file(GLOB_RECURSE ROOMS_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/../../game/rooms/*.h"
)

set(ALL_SOURCES
    ${SRC_FILES}
    ${GAME_FILES}
    ${ROOMS_FILES}
    ${SRC_HEADERS}
    ${GAME_HEADERS}
    ${ROOMS_HEADERS}
)

# Group files in Visual Studio
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "src" FILES ${SRC_FILES} ${SRC_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/../../game" PREFIX "game" FILES ${GAME_FILES} ${GAME_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/../../game/rooms" PREFIX "game/rooms" FILES ${ROOMS_FILES} ${ROOMS_HEADERS})

# Build target based on toolchain
if (TOOLCHAIN STREQUAL "armgcc")
    set(CURRENT_TARGET ${PLAYDATE_GAME_DEVICE})
    add_executable(${CURRENT_TARGET} src/main.c)
else()
    set(CURRENT_TARGET ${PLAYDATE_GAME_NAME})
    add_library(${CURRENT_TARGET} SHARED ${ALL_SOURCES})
endif()

# Set include directories on the correct target
target_include_directories(${CURRENT_TARGET} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../game"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../game/rooms"
)

# Playdate SDK integration
include(${SDK}/C_API/buildsupport/playdate_game.cmake)
