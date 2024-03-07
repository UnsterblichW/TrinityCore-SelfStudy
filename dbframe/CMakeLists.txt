# sudo apt-get install libmysqlclient-dev
# mysql.h 头文件以及动态库和静态库

cmake_minimum_required(VERSION 3.23)

project(dbframe)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(fmt)

add_subdirectory(dbimpl)

add_subdirectory(example/sync)
add_subdirectory(example/async)

# 这条命令可以让cmake在生成的时候，生成出一个 compile_commands.json 文件出来，然后clangd会检测到这个文件
# 之后就会发现原本会找不到头文件的地方已经没有错误提示了
# 但是这个命令在msvc系列的编译器的生成下是没法生成出来compile_commands.json的
# 所以假如你的windows电脑还没装GCC的话，就只能在当前工作区的 .vscode 里面加个settings.json，在里面配置上 clangd.fallbackFlags
# clangd.fallbackFlags 里面手动加上需要包含的头文件或者目录
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 