cmake_minimum_required(VERSION 3.20...4.1)
project(DTM) # DeltaMask
add_compile_options(-fdiagnostics-color=always)

set(CMAKE_CXX_STANDARD 23)

# ソースファイル
file(GLOB SOURCES src/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

# ライブラリをリンク
target_link_libraries(${PROJECT_NAME} PRIVATE
)
