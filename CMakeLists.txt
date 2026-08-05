cmake_minimum_required(VERSION 3.28)

project(MyGame)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)

find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)
set(SFML_DIR "C:/Libraries/SFML-3.1.0/lib/cmake/SFML")
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)


add_executable(MyGame
    src/main.cpp
)

add_subdirectory(external/imgui-sfml)

target_link_libraries(MyGame
    PRIVATE
        SFML::Graphics
        SFML::Window
        SFML::System
        ImGui-SFML
)

