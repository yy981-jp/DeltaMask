cmake_minimum_required(VERSION 3.20...4.1)
project(DTM) # DeltaMask
add_compile_options(-fdiagnostics-color=always)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)

# ソースファイル
file(GLOB SOURCES src/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/external
)

target_link_libraries(${PROJECT_NAME} PRIVATE
)
