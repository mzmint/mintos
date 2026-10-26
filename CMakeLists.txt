
cmake_minimum_required(VERSION 3.20)
project(MintOS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
        SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 3.1.0
        GIT_SHALLOW TRUE
)

set(SFML_USE_SYSTEM_DEPS OFF)
set(BUILD_SHARED_LIBS ON)

FetchContent_MakeAvailable(SFML)

add_executable(MintOS main.cpp
        MintLibClock.h
        MintLibClock.cpp
        MintLibSystem.h
        MintLibSystem.cpp
        MintLibFile.h
        MintLibFile.cpp
        app.cpp
        app.h)

target_link_libraries(MintOS PRIVATE SFML::Graphics)

if(WIN32)
    add_custom_command(TARGET MintOS POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy -t
            $<TARGET_FILE_DIR:MintOS>
            $<TARGET_RUNTIME_DLLS:MintOS>
            COMMAND_EXPAND_LISTS
    )
endif()
