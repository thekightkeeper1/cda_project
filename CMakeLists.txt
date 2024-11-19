cmake_minimum_required(VERSION 3.29)
project(cda_project C)

set(CMAKE_C_STANDARD 11)


add_executable(cda_project spimcore.h
        spimcore.c
        project.c)
