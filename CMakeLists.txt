cmake_minimum_required(VERSION 3.20)
project(MyPrintf C)

set(CMAKE_C_STANDARD 99)

add_executable(MyPrintf main.c my_printf.h my_printf.c)
