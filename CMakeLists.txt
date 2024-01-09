cmake_minimum_required(VERSION 3.5)
project(GamejamGame)

# Find the SDL2 library
find_package(SDL2 REQUIRED)

# Include SDL2 include directories
include_directories(${SDL2_INCLUDE_DIRS})

# Define the executable
add_executable(MySDLApp
    src/main.cpp
    src/Polygon.cpp
    src/util.cpp
    inc/imgui_impl_sdlrenderer2.cpp
    inc/imgui_impl_sdl2.cpp
)

# Link SDL2 with the executable
target_link_libraries(MySDLApp ${SDL2_LIBRARIES} imgui m)
