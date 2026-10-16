cmake_minimum_required(VERSION 3.15)
project(AntiMrBeast LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(AntiMrBeast main.cpp)

if(MSVC)
    target_compile_options(AntiMrBeast PRIVATE /W4 /EHsc /O2)
else()
    target_compile_options(AntiMrBeast PRIVATE -Wall -Wextra -O2)
endif()
