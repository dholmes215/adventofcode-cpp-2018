cmake_minimum_required(VERSION 3.9 FATAL_ERROR)

project(adventofcode-cpp-2018 LANGUAGES CXX)

if(MSVC)
  add_compile_options(/W4 /WX)
else(MSVC)
  add_compile_options(-Wall -Wextra -Werror)
endif(MSVC)

add_subdirectory(src)
