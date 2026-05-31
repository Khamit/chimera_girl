cmake_minimum_required(VERSION 3.16)
project(ChimeraGirl)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES "arm64")
add_compile_options(-O3)

find_package(raylib REQUIRED)

add_executable(game
    src/main.cpp
    src/Player.cpp
    src/Enemy.cpp
    src/Level.cpp
)

target_link_libraries(game raylib)
target_include_directories(game PRIVATE src)

add_custom_command(TARGET game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:game>/assets
)