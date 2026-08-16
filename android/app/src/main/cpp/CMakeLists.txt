cmake_minimum_required(VERSION 3.22.1)
project(droyscript)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(droyengine SHARED
    droy_jni.cpp
    engine/src/lexer.cpp
    engine/src/interpreter.cpp
)

target_include_directories(droyengine PRIVATE
    engine/include
)

find_library(log-lib log)
target_link_libraries(droyengine ${log-lib})
