cmake_minimum_required(VERSION 3.5.0)
project(game1 VERSION 0.1.0 LANGUAGES C CXX)
# 查找SDL2包
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)

add_executable(game1 main.cpp)

# 添加 SDL2 头文件路径
target_include_directories(game1 PRIVATE ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS})

# 链接 SDL2 库
target_link_libraries(game1 ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES})



